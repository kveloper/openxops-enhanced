//! @file config.h
//! @brief Declaration of config class

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

#ifndef CONFING_H
#define CONFING_H

#define TOTAL_ControlKey 18		//!< Total number of operation keys
#define MAX_PLAYERNAME 21		//!< Maximum number of characters

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 1		//!< Select include file.
#endif
#include "main.h"

//! Constants for GetKeycode().
enum ControlKey
{
	KEY_TURNUP = 0,
	KEY_TURNDOWN,
	KEY_TURNLEFT,
	KEY_TURNRIGHT,
	KEY_MOVEFORWARD,
	KEY_MOVEBACKWARD,
	KEY_MOVELEFT,
	KEY_MOVERIGHT,
	KEY_WALK,
	KEY_JUMP,
	KEY_RELOAD,
	KEY_DROPWEAPON,
	KEY_ZOOMSCOPE,
	KEY_SHOTMODE,
	KEY_SWITCHWEAPON,
	KEY_WEAPON1,
	KEY_WEAPON2,
	KEY_SHOT
};

//! @brief Class to read configuration files
//! @details Manage file references and values.
//! @details Reference: config.dat file analysis information http://openxops.net/filesystem-config.php
class Config
{
	int Keycode[TOTAL_ControlKey];		//!< Original Key Code
	int MouseSensitivity;				//!< Mouse Sensitivity
	bool FullscreenFlag;				//!< Full-screen enabled
	bool SoundFlag;						//!< Enable sound effects.
	bool BloodFlag;						//!< Enable bleeding.
	int Brightness;						//!< Screen Brightness
	bool InvertMouseFlag;				//!< Mouse Inverted
	bool FrameskipFlag;					//!< Frame skipping.
	bool AnotherGunsightFlag;			//!< Use the alternate crosshair.
	char PlayerName[MAX_PLAYERNAME];	//!< Player Name

	int Language;						//!< Display language
	bool ShowArmFlag;					//!< Arm display flag
	int ScreenWidth;					//!< Screen resolution (width)
	int ScreenHeight;					//!< Screen Resolution (Height)
	bool MouseLimitFlag;				//!< Mouse Move Limit Flag
	float mastervolume;					//!< Volume

public:
	Config();
	~Config();
	int LoadFile(const char *fname);
	int SaveFile(const char *fname);
	void SetDefaultConfig();
	int GetKeycode(int id);
	void SetKeycode(int id, int key);
	int GetMouseSensitivity();
	void SetMouseSensitivity(int param);
	bool GetFullscreenFlag();
	void SetFullscreenFlag(bool flag);
	bool GetSoundFlag();
	void SetSoundFlag(bool flag);
	bool GetBloodFlag();
	void SetBloodFlag(bool flag);
	int GetBrightness();
	void SetBrightness(int param);
	bool GetInvertMouseFlag();
	void SetInvertMouseFlag(bool flag);
	bool GetFrameskipFlag();
	void SetFrameskipFlag(bool flag);
	bool GetAnotherGunsightFlag();
	void SetAnotherGunsightFlag(bool flag);
	int GetPlayerName(char *out_str);
	void SetPlayerName(char *in_str);
	const char* GetOriginalkeycodeString(int code);

	bool CreateExtConfig(const char *fname);
	int LoadExtFile(const char *fname);
	void SetDefaultExtConfig();
	int GetLanguage();
	bool GetShowArmFlag();
	int GetScreenWidth();
	int GetScreenHeight();
	bool GetMouseLimitFlag();
	float GetVolume();
};

#endif