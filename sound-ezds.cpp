//! @file sound-directsound.cpp
//! @brief Definition of SoundControl class (ezds.dll version)

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

#if SOUND_ENGINE == 0

//! @brief Constructor
SoundControl::SoundControl()
{
	lib = NULL;
	DSver = NULL;
	DSinit = NULL;
	DSend = NULL;
	DSload = NULL;
	DSplay = NULL;
	DSrelease = NULL;

	//Initialize used flags
	for(int i=0; i<MAX_LOADSOUND; i++){
		useflag[i] = false;
	}

	camera_x = 0.0f;
	camera_y = 0.0f;
	camera_z = 0.0f;
	camera_rx = 0.0f;
	mastervolume = 1.0f;
	volume_max = 100;
}

//! @brief Destructor
SoundControl::~SoundControl()
{
	DestroySound();
}

//! @brief Initialization @n
//! (Loading DLL and executing initialization functions)
//! @param WindowCtrl WindowControl class pointer
//! @return Success:0 failure:1
int SoundControl::InitSound(WindowControl *WindowCtrl)
{
	if( WindowCtrl == NULL ){ return 1; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_INIT, "Sound", "ezds.dll");
#endif

	if( lib != NULL ){
		return 1;
	}

	//Read DLL
	lib = LoadLibrary("ezds.dll");
	if( lib == NULL ){
		return 1;
	}

	//Assign Functions
	DSver = GetProcAddress(lib, "DSver");
	DSinit = (FARPROCH)GetProcAddress(lib, "DSinit");
	DSend = GetProcAddress(lib, "DSend");
	DSload = (FARPROCCI)GetProcAddress(lib, "DSload");
	DSplay = (FARPROCIII)GetProcAddress(lib, "DSplay");
	DSrelease = (FARPROCI)GetProcAddress(lib, "DSrelease");

	if( DSinit != NULL ){
		//DLL Initialization
		if( DSinit(WindowCtrl->GethWnd()) != 0 ){
#ifdef ENABLE_DEBUGLOG
			//Write to the log
			OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif
			return 0;
		}
	}

	//Release DLL
	FreeLibrary(lib);
	lib = NULL;

	//return 1;
	return 0;
}

//! @brief Release resources.
//! @attention This function is automatically called, but it can be called explicitly.
void SoundControl::DestroySound()
{
	if( lib == NULL ){ return; }

	//Releasing sound data in use
	for(int i=0; i<MAX_LOADSOUND; i++){
		if( useflag[i] == true ){
			if( DSrelease != NULL ){ DSrelease(i); }
			useflag[i] = false;
		}
	}

	//End DLL
	if( DSend != NULL ){ DSend(); }

	//Release DLL
	FreeLibrary(lib);
	lib = NULL;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Sound", "ezds.dll");
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
	if( mastervolume == 0.0f ){ volume_max = -100; return; }
	if( mastervolume == 1.0f ){ volume_max = 100; return; }
	volume_max = 100 - (int)(log10f(mastervolume)*-0.2f*200);
}

//! @brief Set the coordinate and angle of the camera
//! @param x Camera X-coordinate
//! @param y Camera Y coordinates
//! @param z Camera Z coordinates
//! @param rx X-axis angle of camera (reservation)
//! @warning Set the latest camera coordinates (application) by calling each frame.
void SoundControl::SetCamera(float x, float y, float z, float rx)
{
	camera_x = x;
	camera_y = y;
	camera_z = z;
	camera_rx = rx;
}

//! @brief Read Sound
//! @param filename File Name
//! @return Success: data ID of 0 or greater; failure: -1.
int SoundControl::LoadSound(const char* filename)
{
	if( lib == NULL ){ return -1; }
	if( filename == NULL ){ return -1; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "Sound", filename);
#endif

	//Find Data Numbers Not Used
	for(int i=0; i<MAX_LOADSOUND; i++){
		if( useflag[i] == false ){

#ifdef ENABLE_PATH_DELIMITER_SLASH
			//Convert path separators.
			filename = ChangePathDelimiter(filename);
#endif

			//Try to read
			if( DSload == NULL ){ return -1; }
			if( DSload(filename, i) == 0 ){ return -1; }

			//Play once with a small volume
			if( DSplay == NULL ){ return -1; }
			DSplay(i, -100, 0);

			//Sets flags that represent in use
			useflag[i] = true;

#ifdef ENABLE_DEBUGLOG
			//Write to the log
			OutputLog.WriteLog(LOG_COMPLETE, "", i);
#endif
			return i;
		}
	}

	return -1;
}

//! @brief Play sound (called non-3D playback DLL)
//! @param id Data Number
//! @param volume Play Volume
//! @param pan Pan (left/right balance).
//! @return Success:1 to 3 Failure:0
//! @note Use: Sounds that sound evenly throughout the game space, and BGMs directly from the player themselves
int SoundControl::PlaySound(int id, int volume, int pan)
{
	if( lib == NULL ){ return 0; }
	if( (id < 0)||(MAX_LOADSOUND <= id) ){ return 0; }
	if( useflag[id] == false ){ return 0; }

	if( volume < -100 ){ volume = -100; }
	if( volume > 100 ){ volume = 100; }
	if( pan < -100 ){ pan = -100; }
	if( pan > 100 ){ pan = 100; }

	//Play Sound
	if( DSplay == NULL ){ return 0; }
	//return DSplay(id, volume, pan);
	int volume2 = (int)(1.0f/200 * (volume + 100) * (volume_max + 100) - 100);
	return DSplay(id, volume2, pan);
}

//! @brief Play sound (3D playback)
//! @param id Data Number
//! @param x X-coordinate of sound source
//! @param y Y coordinates of sound source
//! @param z Z coordinate of sound source
//! @param volume Play Volume
//! @return Success:1 to 3 Failure:0
//! @note Application: General sound effect, which has absolute position and is affected by distance.
int SoundControl::Play3DSound(int id, float x, float y, float z, int volume)
{
	if( (id < 0)||(MAX_LOADSOUND <= id) ){ return 0; }
	if( useflag[id] == false ){ return 0; }

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

	/*
	//Determine audio pan (left/right playback balance).
	float vx = x - camera_x;
	float vz = z - camera_z;
	float rx = (atan2f(vz, vx) - camera_rx) * -1;
	rx = AngleNormalization(rx);
	pan = (int)((float)10 / M_PI * rx);
	*/

	//DLL Calls
	return PlaySound(id, playvolume, pan);
}

//! @brief Get the read sound count
//! @return Number of Sounds
int SoundControl::GetTotalSounds()
{
	int cnt = 0;

	//Counting elements in use
	for(int i=0; i<MAX_LOADSOUND; i++){
		if( useflag[i] == true ){ cnt += 1; }
	}

	return cnt;
}

//! @brief Free Sound
//! @param id Data Number
void SoundControl::CleanupSound(int id)
{
	if( lib == NULL ){ return; }
	if( (id < 0)||(MAX_LOADSOUND <= id) ){ return; }
	if( useflag[id] == false ){ return; }

	//Releasing sound data in use
	if( DSrelease != NULL ){ DSrelease(id); }

	//Clear the in-use flag.
	useflag[id] = false;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Sound", id);
#endif
}

//! @brief Check the distance with the sound source
//! @param x X-coordinate of sound source
//! @param y Y coordinates of sound source
//! @param z Z coordinate of sound source
//! @param snear Nearest Sound Sources
//! @param out_dist Distance.
//! @return Enabled (inside): true disabled (outside): false
bool SoundControl::CheckSourceDist(float x, float y, float z, bool snear, float *out_dist)
{
	float dx, dy, dz, dist;
	int max_dist;

	if( snear == false ){
		max_dist = MAX_SOUNDDIST;
	}
	else{
		max_dist = 30;
	}

	dx = camera_x - x;
	dy = camera_y - y;
	dz = camera_z - z;
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

#endif	//SOUND_ENGINE