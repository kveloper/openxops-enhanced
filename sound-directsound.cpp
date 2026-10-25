//! @file sound-directsound.cpp
//! @brief Definition of SoundControl class (DirectSound version)

//--------------------------------------------------------------------------------
// 
// OpenXOPS
// Copyright (c) 2014-2023, OpenXOPS Project / [-_-;](mikan) All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, 
//   this list of conditions and the following disclaimer in the documentation 
//   and/or other materials provided with the distribution.
// * Neither the name of the OpenXOPS Project nor the names of its contributors 
//   may be used to endorse or promote products derived from this software 
//   without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenXOPS Project BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//--------------------------------------------------------------------------------

#include "sound.h"

#if (SOUND_ENGINE == 1 || SOUND_ENGINE == 2)

//! @brief Constructor
SoundControl::SoundControl()
{
	pDSound = NULL;
	for(int i=0;i<MAX_LOADSOUND; i++){
		for(int j=0; j<MAX_SOUNDLISTS; j++){
			pDSBuffer[i][j] = NULL;
		}
	}
	p3DListener = NULL;
	mastervolume = 1.0f;
	volume_dBper = 1.0f;
}

//! @brief Destructor
SoundControl::~SoundControl()
{
	DestroySound();
}

//! @brief Initialize
//! @param WindowCtrl WindowControl class pointer
//! @return Success:0 failure:1
int SoundControl::InitSound(WindowControl *WindowCtrl)
{
	if( WindowCtrl == NULL ){ return 1; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_INIT, "Sound", "DirectSound");
#endif

	//Directrate DirectSound objects
	if( FAILED( DirectSoundCreate8(NULL, &pDSound, NULL) ) ){
		return 1;
	}

	//Configuring Cooperative Levels
	if( FAILED( pDSound->SetCooperativeLevel(WindowCtrl->GethWnd(), DSSCL_EXCLUSIVE) ) ){
		return 1;
	}

	//Create a primary buffer and get a listener interface
	LPDIRECTSOUNDBUFFER pPrimary;
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
	if( SUCCEEDED(pDSound->CreateSoundBuffer(&dsbd, &pPrimary, NULL)) ){
		pPrimary->QueryInterface(IID_IDirectSound3DListener8, (LPVOID *)&p3DListener);
		pPrimary->Release();
	}

	//Disable the Doppler effect.
	p3DListener->SetDopplerFactor(DS3D_MINDOPPLERFACTOR, DS3D_IMMEDIATE);

	//Roll-off setting
	p3DListener->SetRolloffFactor(0.05f, DS3D_IMMEDIATE);

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif

	return 0;
}

//! @brief Release resources.
//! @attention This function is automatically called, but it can be called explicitly.
void SoundControl::DestroySound()
{
	if( pDSound == NULL ){ return; }

	for(int i=0;i<MAX_LOADSOUND; i++){
		for(int j=0; j<MAX_SOUNDLISTS; j++){
			if( pDSBuffer[i][j] != NULL ){
				pDSBuffer[i][j]->Release();
				pDSBuffer[i][j] = NULL;
			}
		}
	}
	if( pDSound != NULL ){
		pDSound->Release();
		pDSound = NULL;
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Sound", "DirectSound");
#endif
}

//! @brief Set the volume of playback
//! @param volume Volume (0.0=non-sound 1.0=100%)
//! @attention Values below 0.01 (1%) are treated as 0.00 (0%).
void SoundControl::SetVolume(float volume)
{
	mastervolume = volume;
	if( mastervolume < 0.01f ){ mastervolume = 0.0f; }
	if( mastervolume > 1.00f ){ mastervolume = 1.0f; }

	//Determines maximum volume in consideration of dB
	if( mastervolume == 0.0f ){ volume_dBper = 0.0f; return; }
	if( mastervolume == 1.0f ){ volume_dBper = 1.0f; return; }
	volume_dBper = 1.0f - log10f(mastervolume)*-0.2f;
}

//! @brief Set the coordinate and angle of the camera
//! @param x Camera X-coordinate
//! @param y Camera Y coordinates
//! @param z Camera Z coordinates
//! @param rx X-axis angle of camera (reservation)
//! @warning Set the latest camera coordinates (application) by calling each frame.
void SoundControl::SetCamera(float x, float y, float z, float rx)
{
	p3DListener->SetPosition(x, y, z, DS3D_IMMEDIATE);
	p3DListener->SetOrientation(cosf(rx), 0.0f, sinf(rx), 0.0f, 1.0f, 0.0f, DS3D_IMMEDIATE);
}

//! @brief Read Sound
//! @param filename File Name
//! @return Success: data ID of 0 or greater; failure: -1.
//! @attention If two-channel (stereo) data is specified, the data on the right is only obtained and the data on the left side is ignored.
int SoundControl::LoadSound(const char* filename)
{
	if( pDSound == NULL ){ return -1; }
	if( filename == NULL ){ return -1; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "Sound", filename);
#endif

	//Find open numbers
	int id=0;
	for(id=0; id<MAX_LOADSOUND; id++){
		if( pDSBuffer[id][0] == NULL ){ break; }
	}
	if( id == MAX_LOADSOUND ){ return -1; }

	WAVEFORMATEX* pwfex;
	int WavSize = 0;
	int Wavoffset = 0;
	bool d2channels = false;

	//Get information about Wave files
	if( CheckSoundFile(filename, &WavSize, &Wavoffset, &pwfex) == true ){
		return -1;
	}
	pwfex->cbSize = 0;

	//Unsupported format fails
	if( (pwfex->wFormatTag != WAVE_FORMAT_PCM)&&(pwfex->wFormatTag != WAVE_FORMAT_IEEE_FLOAT) ){ return -1; }
	if( (pwfex->nChannels != 1)&&(pwfex->nChannels != 2) ){ return -1; }
	if( (pwfex->wBitsPerSample != 8)&&(pwfex->wBitsPerSample != 16)
		&&(pwfex->wBitsPerSample != 24)&&(pwfex->wBitsPerSample != 32) ){ return -1; }

	//If you are a stereo data
	if( pwfex->nChannels == 2 ){
		d2channels = true;		//Set Flags
		WavSize /= 2;			//Half size

		//Forced recal d as mono
		pwfex->nChannels = 1;
		pwfex->nAvgBytesPerSec /= 2;
		pwfex->nBlockAlign /= 2;
	}

	// Create a DirectSound secondary buffer.
	DSBUFFERDESC dsbd;  
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = WavSize;
	dsbd.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
	dsbd.lpwfxFormat = pwfex;
	if( FAILED( pDSound->CreateSoundBuffer(&dsbd, &pDSBuffer[id][0], NULL) ) ){
		return -1;	//Secondary buffer creation failure
	}

	//Lock
	VOID* pBuffer = NULL;
	DWORD dwBufferSize = 0; 
	if( FAILED(pDSBuffer[id][0]->Lock(0, WavSize, &pBuffer, &dwBufferSize, NULL, NULL, 0) ) ){
		return -1;	//lock failure
	}

	//Secure temporary space
	BYTE* pWavData;
	int dwSize = dwBufferSize;
	if( dwSize > WavSize ){ dwSize = WavSize; }
	if( d2channels == true ){
		dwSize = dwSize * 2;
	}
	pWavData = new BYTE[dwSize];
	if( pWavData == NULL ){
		//I can't secure memory that reads WAVE files
		return -1;
	}

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	filename = ChangePathDelimiter(filename);
#endif

	//Capture waveform data
	FILE* fp;
	fp = fopen(filename, "rb");
	fseek(fp, Wavoffset, SEEK_SET);
	fread(pWavData, 1, dwSize, fp);
	fclose(fp);

	//Copy from temporary area to secondary buffer
	if( d2channels == false ){
		//Copy mono data by one byte
		for(int i=0; i<dwSize; i++){
			((BYTE*)pBuffer)[i] = pWavData[i];
		}
	}
	else{
		//Get the number of bytes per sample.
		int samplingbits = pwfex->wBitsPerSample;

		if( samplingbits == 8 ){
			int byte = 0;
			for(int i=0; i<dwSize; i+=2){
				/*
				int mix = (pWavData[i] + pWavData[i + 1])/2;
				((BYTE*)pBuffer)[byte] = (BYTE)mix;
				*/

				//For stereo data, store only the left channel.
				((BYTE*)pBuffer)[byte] = (BYTE)pWavData[i];
				byte += 1;
			}
		}
		if( samplingbits == 16 ){
			int byte = 0;
			for(int i=0; i<dwSize; i+=4){
				/*
				//int mix = (int)(Get2ByteBigEndian(&(pWavData[i + 0])) + Get2ByteBigEndian(&(pWavData[i + 2])))/2;
				int mix = (int)Get2ByteBigEndian(&(pWavData[i + 0]));
				((BYTE*)pBuffer)[byte+0] = (BYTE)((mix>>8)&0x00FF);
				((BYTE*)pBuffer)[byte+1] = (BYTE)(mix)&0x00FF;
				byte += 2;
				*/

				//For stereo data, store only the left channel.
				((BYTE*)pBuffer)[byte+0] = (BYTE)pWavData[i + 0];
				((BYTE*)pBuffer)[byte+1] = (BYTE)pWavData[i + 1];
				byte += 2;
			}
		}
		if( samplingbits == 24 ){
			int byte = 0;
			for(int i=0; i<dwSize; i+=6){
				//For stereo data, store only the left channel.
				((BYTE*)pBuffer)[byte+0] = (BYTE)pWavData[i + 0];
				((BYTE*)pBuffer)[byte+1] = (BYTE)pWavData[i + 1];
				((BYTE*)pBuffer)[byte+2] = (BYTE)pWavData[i + 2];
				byte += 3;
			}
		}
		if( samplingbits == 32 ){
			int byte = 0;
			for(int i=0; i<dwSize; i+=8){
				//For stereo data, store only the left channel.
				((BYTE*)pBuffer)[byte+0] = (BYTE)pWavData[i + 0];
				((BYTE*)pBuffer)[byte+1] = (BYTE)pWavData[i + 1];
				((BYTE*)pBuffer)[byte+2] = (BYTE)pWavData[i + 2];
				((BYTE*)pBuffer)[byte+3] = (BYTE)pWavData[i + 3];
				byte += 4;
			}
		}
	}

	//Unlock
	pDSBuffer[id][0]->Unlock(pBuffer, dwBufferSize, NULL, 0);

	//Free temporary space
	delete [] pWavData;

	//Format information release
	delete pwfex;

	//Create a copy of the secondary buffer
	for(int i=1; i<MAX_SOUNDLISTS; i++){
		if( pDSound->DuplicateSoundBuffer(pDSBuffer[id][0], &(pDSBuffer[id][i])) != DS_OK ){
			CleanupSound(id);
			return -1;
		}
	}

	//Play at least once in volume
	pDSBuffer[id][0]->SetVolume(DSBVOLUME_MIN);
	pDSBuffer[id][0]->Play(NULL, 0, NULL);

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", id);
#endif

	return id;
}

//! @brief Play sound (non-3D playback)
//! @param id Data Number
//! @param volume Play Volume
//! @param pan <Disabled>
//! @return Success:1 failure:0
//! @note Use: Sounds that sound evenly throughout the game space, and BGMs directly from the player themselves
int SoundControl::PlaySound(int id, int volume, int pan)
{
	if( pDSound == NULL ){ return 0; }
	if( (id < 0)||(MAX_LOADSOUND <= id) ){ return 0; }
	if( pDSBuffer[id][0] == NULL ){ return 0; }

	DWORD status = 0;

	for(int i=0; i<MAX_SOUNDLISTS; i++){
		//Get playback status
		pDSBuffer[id][i]->GetStatus(&status);

		//If the sound is stopped
		if( (status & DSBSTATUS_PLAYING) == 0x00 ){
			LPDIRECTSOUND3DBUFFER pDS3DBuffer;
			if( FAILED(pDSBuffer[id][i]->QueryInterface(IID_IDirectSound3DBuffer8, (VOID**)&pDS3DBuffer)) ){
				//Cannot obtain IDirectSound3DBuffer8.
				return 0;
			}

			//Disable 3D playback
			pDS3DBuffer->SetMode(DS3DMODE_DISABLE, DS3D_IMMEDIATE);

			//Set and play volume
			pDSBuffer[id][i]->SetVolume( GetDSVolume(volume) );
			pDSBuffer[id][i]->Play(NULL, 0, NULL);
			return 1;
		}
	}

	return 0;
}

//! @brief Play sound (3D playback)
//! @param id Data Number
//! @param x X-coordinate of sound source
//! @param y Y coordinates of sound source
//! @param z Z coordinate of sound source
//! @param volume Play Volume
//! @return Success:1 failure:0
//! @note Application: General sound effect, which has absolute position and is affected by distance.
int SoundControl::Play3DSound(int id, float x, float y, float z, int volume)
{
	if( pDSound == NULL ){ return 0; }
	if( (id < 0)||(MAX_LOADSOUND <= id) ){ return 0; }
	if( pDSBuffer[id][0] == NULL ){ return 0; }

#if SOUND_ENGINE == 1

	if( volume < -100 ){ volume = -100; }
	if( volume > 100 ){ volume = 100; }

	float dist;
	int playvolume;
	int pan = 0;
	
	//Determination of the volume of sound by distance
	if( CheckSourceDist(x, y, z, false, &dist) == false ){
		return 0;
	}
	playvolume = CalculationVolume(volume, dist, false);

	return PlaySound(id, playvolume, pan);

#else	//#if SOUND_ENGINE

	DWORD status = 0;

	for(int i=0; i<MAX_SOUNDLISTS; i++){
		//Get playback status
		pDSBuffer[id][i]->GetStatus(&status);

		//If the sound is stopped
		if( (status & DSBSTATUS_PLAYING) == 0x00 ){

			LPDIRECTSOUND3DBUFFER pDS3DBuffer;
			if( FAILED(pDSBuffer[id][i]->QueryInterface(IID_IDirectSound3DBuffer8, (VOID**)&pDS3DBuffer)) ){
				//Cannot obtain IDirectSound3DBuffer8.
				return 0;
			}

			//Minimum distance and maximum distance setting
			pDS3DBuffer->SetMinDistance(1, DS3D_IMMEDIATE);
			pDS3DBuffer->SetMaxDistance(MAX_SOUNDDIST, DS3D_IMMEDIATE);

			//Set the coordinates of the sound source
			pDS3DBuffer->SetPosition(x, y, z, DS3D_IMMEDIATE);

			//Enable 3D playback
			pDS3DBuffer->SetMode(DS3DMODE_NORMAL, DS3D_IMMEDIATE);

			//Set and play volume
			pDSBuffer[id][i]->SetVolume( GetDSVolume(volume) );
			pDSBuffer[id][i]->Play(NULL, 0, NULL);
			return 1;
		}
	}

	return 0;

#endif	//#if SOUND_ENGINE
}

//! @brief Get the read sound count
//! @return Number of Sounds
int SoundControl::GetTotalSounds()
{
	int cnt = 0;

	//Counting elements in use
	for(int i=0; i<MAX_LOADSOUND; i++){
		if( pDSBuffer[i][0] != NULL ){ cnt += 1; }
	}

	return cnt;
}

//! @brief Free Sound
//! @param id Data Number
void SoundControl::CleanupSound(int id)
{
	if( pDSound == NULL ){ return; }
	if( (id < 0)||(MAX_LOADSOUND <= id) ){ return; }
	if( pDSBuffer[id][0] == NULL ){ return; }

	DWORD status = 0;

	for(int i=0; i<MAX_SOUNDLISTS; i++){
		//Get playback status
		pDSBuffer[id][i]->GetStatus(&status);

		//Stop if the sound is playing
		if( (status & DSBSTATUS_PLAYING) != 0x00 ){
			pDSBuffer[id][i]->Stop();
		}

		//Release the target secondary buffer
		if( pDSBuffer[id][i] != NULL ){ pDSBuffer[id][i]->Release(); }
		pDSBuffer[id][i] = NULL;
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Sound", id);
#endif
}

//! @brief Find information about Wave files
//! @param filename File Name
//! @param filesize Pointer to receive file size
//! @param fileoffset Pointer to receive data offset
//! @param pwfex Double pointer to receive WAVEFORMATEX type
//! @return Success:false Fail:true
bool SoundControl::CheckSoundFile(const char* filename, int *filesize, int *fileoffset, WAVEFORMATEX** pwfex)
{
	HMMIO hMmio = NULL;
	MMCKINFO ckInfo;
	MMCKINFO riffckInfo;
	PCMWAVEFORMAT pcmWaveFormat; 
	bool errorflag;

	hMmio = mmioOpen((LPSTR)filename, NULL, MMIO_ALLOCBUF | MMIO_READ);
	if( hMmio == NULL ){
		//No files
		return true;
	}

	ZeroMemory(&riffckInfo, sizeof(MMCKINFO));

	//WAVE file check
	errorflag = true;
	if( mmioDescend( hMmio, &riffckInfo, NULL, 0 ) == MMSYSERR_NOERROR ){
		if( (mmioFOURCC('R','I','F','F') == riffckInfo.ckid) && (mmioFOURCC('W','A','V','E') == riffckInfo.fccType) ){
			errorflag = false;
		}
	}
	if( errorflag == true ){
		mmioClose(hMmio, MMIO_FHOPEN);
		return true;	//not WAVE file
	}

	//Get format information
	*pwfex = NULL;
	ckInfo.ckid = mmioFOURCC('f','m','t',' ');
	if( mmioDescend(hMmio, &ckInfo, &riffckInfo, MMIO_FINDCHUNK) == MMSYSERR_NOERROR ){
		if( mmioRead(hMmio, (HPSTR) &pcmWaveFormat, sizeof(pcmWaveFormat)) == sizeof(pcmWaveFormat) ){
			if( (pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)||(pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ){
				if( (pcmWaveFormat.wf.nChannels == 1)||(pcmWaveFormat.wf.nChannels == 2) ){
					*pwfex = new WAVEFORMATEX;
					if( *pwfex != NULL ){
						memcpy( *pwfex, &pcmWaveFormat, sizeof(pcmWaveFormat) );
						//pwfex->cbSize = 0;
					}
				}
			}
		}
	}
	if( *pwfex == NULL ){
		mmioClose(hMmio, MMIO_FHOPEN);
		return true;	//File format is not supported
	}
	if( mmioAscend(hMmio, &ckInfo, 0) != MMSYSERR_NOERROR ){
		delete pwfex;
		mmioClose(hMmio, MMIO_FHOPEN);
		return true;	//chunk control failure
	}

	//Finding Data Area
	ckInfo.ckid = mmioFOURCC('d','a','t','a');
	if( mmioDescend(hMmio, &ckInfo, &riffckInfo, MMIO_FINDCHUNK) != MMSYSERR_NOERROR ){
		mmioClose(hMmio, MMIO_FHOPEN);
		return true;	//File format is not supported
	}

	mmioClose(hMmio, MMIO_FHOPEN);

	*filesize = ckInfo.cksize;
	*fileoffset = ckInfo.dwDataOffset;
	return false;
}

//! @brief 1/100 dB for SetVolume()
//! @param volume Volume(-100 to 100)
//! @return 1/100 dB (dB).
int SoundControl::GetDSVolume(int volume)
{
	if( volume <= -100 ){ return DSBVOLUME_MIN; }
	if( volume >=  100 ){ return DSBVOLUME_MAX; }

	//float volume2 = 1.0f/200 * (volume + 100);
	float volume2 = (1.0f/200 * (volume + 100)) * volume_dBper;
	int retn = (int)((DSBVOLUME_MIN-DSBVOLUME_MAX) * (1.0f - volume2));
	return retn;
}

#if SOUND_ENGINE == 1
//! @brief Check the distance with the sound source
//! @param x X-coordinate of sound source
//! @param y Y coordinates of sound source
//! @param z Z coordinate of sound source
//! @param snear Nearest Sound Sources
//! @param out_dist Distance.
//! @return Enabled (inside): true disabled (outside): false
bool SoundControl::CheckSourceDist(float x, float y, float z, bool snear, float *out_dist)
{
	D3DXVECTOR3 camera;
	float dx, dy, dz, dist;
	int max_dist;

	p3DListener->GetPosition(&camera);

	if( snear == false ){
		max_dist = MAX_SOUNDDIST;
	}
	else{
		max_dist = 30;
	}

	dx = camera.x - x;
	dy = camera.y - y;
	dz = camera.z - z;
	dist = dx*dx + dy*dy + dz*dz;
	if( dist > max_dist * max_dist ){
		*out_dist = 0.0f;
		return false;
	}

	*out_dist = sqrtf(dist);
	return true;
}

//! @brief Calculate the volume.
//! @param MaxVolume Maximum volume of sound source
//! @param dist Distance to sound source
//! @param snear Nearest Sound Sources
int SoundControl::CalculationVolume(int MaxVolume, float dist, bool snear)
{
	int max_dist;
	if( snear == false ){
		max_dist = MAX_SOUNDDIST;
	}
	else{
		max_dist = 30;
	}
	return (int)( (float)MaxVolume/max_dist*dist*-1 + MaxVolume );
}
#endif

#endif	//SOUND_ENGINE