//! @file sound.h
//! @brief SoundControl Class Declaration

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

#ifndef SOUND2_H
#define SOUND2_H

#define MAX_LOADSOUND 24		//!< Maximum number of sound can be read
#define MAX_SOUNDLISTS 100		//!< Maximum number of sound playback (DirectSound)
#define MAX_SOUNDDIST 335		//!< Maximum distance to play sound

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 1		//!< Select include file.
#endif
#include "main.h"
#include <windows.h>

#define SOUND_ENGINE 0		//!< @brief Select sound playback library @ezs ezds.dll:0 DirectSound(2D playback):1 DirectSound(3D playback)

#if SOUND_ENGINE == 0
 typedef int (*FARPROCH)(HWND);					//!< DLL Parameter
 typedef int (*FARPROCCI)(const char*, int);	//!< DLL Parameter
 typedef int (*FARPROCIII)(int, int, int);		//!< DLL Parameter
 typedef int (*FARPROCI)(int);					//!< DLL Parameter

 #define SOUND_CORE "ezds"					//!< Version display information
#else	//#if SOUND_ENGINE
 #include <dsound.h>
 #include <mmsystem.h>
 #pragma comment(lib, "dsound.lib")
 #pragma comment(lib, "dxguid.lib")
 #pragma comment(lib, "winmm.lib")
 #ifndef WAVE_FORMAT_IEEE_FLOAT
  #define WAVE_FORMAT_IEEE_FLOAT 3
 #endif

 #if SOUND_ENGINE == 1
  #define SOUND_CORE "DirectSound(Monaural)"	//!< Version display information
 #else
  #define SOUND_CORE "DirectSound(Stereo)"	//!< Version display information
 #endif
#endif	//#if SOUND_ENGINE

//! @brief Class to play sound
//! @details Manage the sound from reading to playback.
//! @details Reference: ezds.dll file (ver:0.4) analysis information http://openxops.net/filesystem-ezds04.php
class SoundControl
{
#if SOUND_ENGINE == 0
	HINSTANCE lib;		//!< instance of a DLL file
	FARPROC DSver;		//!< DSver()
	FARPROCH DSinit;	//!< DSinit()
	FARPROC DSend;		//!< DSend()
	FARPROCCI DSload;	//!< DSload()
	FARPROCIII DSplay;	//!< DSplay()
	FARPROCI DSrelease;	//!< DSrelease()
	bool useflag[MAX_LOADSOUND];	//!< An array that manages the sound number in use

	float camera_x;			//!< Camera Coordinate
	float camera_y;			//!< Camera Coordinate
	float camera_z;			//!< Camera Coordinate
	float camera_rx;		//!< Camera X-axis angle
	float mastervolume;		//!< Volume (Percent)
	int volume_max;			//!< Maximum volume


	bool CheckSourceDist(float x, float y, float z, bool near, float *out_dist);
	int CalculationVolume(int MaxVolume, float dist, bool near);
#else	//#if SOUND_ENGINE
	LPDIRECTSOUND8 pDSound;					//!< DIRECTSOUND8 Pointer
	LPDIRECTSOUNDBUFFER pDSBuffer[MAX_LOADSOUND][MAX_SOUNDLISTS];	//!< Secondary buffer
	LPDIRECTSOUND3DLISTENER8 p3DListener;	//!< Listener.
	float mastervolume;						//!< Volume (Percent)
	float volume_dBper;						//!< Maximum volume

	bool CheckSoundFile(const char* filename, int *filesize, int *fileoffset, WAVEFORMATEX** pwfex);
	int GetDSVolume(int volume);
#if SOUND_ENGINE == 1
	bool CheckSourceDist(float x, float y, float z, bool near, float *out_dist);
	int CalculationVolume(int MaxVolume, float dist, bool near);
#endif
#endif	//#if SOUND_ENGINE

public:
	SoundControl();
	~SoundControl();
	int InitSound(WindowControl *WindowCtrl);
	void DestroySound();
	void SetVolume(float volume);
	void SetCamera(float x, float y, float z, float rx);
	int LoadSound(const char* filename);
	int PlaySound(int id, int volume, int pan);
	int Play3DSound(int id, float x, float y, float z, int volume);
	int GetTotalSounds();
	void CleanupSound(int id);
};

#endif