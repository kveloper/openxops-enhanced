//! @file main.h
//! @brief Top-level header files

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

#ifndef MAIN_H
#define MAIN_H

#define MAINICON 101		//!< Icon

#ifdef _DEBUG
 #define _CRTDBG_MAP_ALLOC
 #include <crtdbg.h>
#endif

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 3		//!< Select include file.
#endif

#pragma warning(disable:4996)		//VC++ warning prevention
//#pragma warning(disable:4244)

//Constant.
#define GAMENAME "OpenXOPS"				//!< Game Name
#define GAMEVERSION "en01"			//!< Game Version
#define GAMEFRAMEMS 30					//!< Processing time per frame (ms)
#define GAMEFPS (1000.0f/GAMEFRAMEMS)	//!< FPS (frame rate) 1000/30 = 33.333[FPS]
#define DEFAULT_SCREEN_WIDTH 640		//!< Screen Width
#define DEFAULT_SCREEN_HEIGHT 480		//!< Screen Height

#define ADDONDIR "addon"		//!< ADDON directory name
#define MAX_ADDONLIST 128		//!< Maximum number of reading ADDON

#define TOTAL_HAVEWEAPON 2		//!< Number of weapons

//System include files
#define _USE_MATH_DEFINES	 //!< math.h 'M_PI' enable
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <direct.h>

#define ENABLE_BUG_HUMANWEAPON			//!< Reproduce bugs caused by out-of-range character or weapon type IDs (comment out to disable).
#define ENABLE_BUG_TEAMID				//!< Reproduce bugs caused by negative team IDs (comment out to disable).
#define ENABLE_WEAPONNAME_SCALING		//!< Reduce the display when the weapon name is long (invalid by commenting)
#define ENABLE_ADDOBJ_PARAM8BIT			//!< Set the setting value of the additional accessory to char type 8bit (invalid by commenting)
#define ENABLE_DEBUGCONSOLE				//!< Enable the debug console (comment out to disable).
#define ENABLE_DEBUGLOG					//!< Enable debug-log output (comment out to disable).
#define ENABLE_CHECKOPENXOPSEVENT		//!< Enable OpenXOPS action judgment event (unable function by commenting)
#define ENABLE_MENUOPTIONS				//!< Added option screen to menu (unable by commenting)
//#define ENABLE_AUTOCREATECONFIG // Automatically create config.dat if it is missing.
//#define ENABLE_PATH_DELIMITER_SLASH // Convert path separators from '\\' to '/'.
#define ENABLE_AUTOLOADMIF				//!< If .mif is specified as an argument, it will be automatically loaded (function disabled by commenting)
#define ENABLE_ADDCONFIG				//!< Get additional settings with ini files (unable by commenting)
#define ENABLE_CHECKDATADIR				//!< Fixed the existence of data folder at startup (function disabled by commenting)

//If you do not use windows.h
#ifndef _MAX_PATH
 #define _MAX_PATH 260		//!< _MAX_PATH is 260
#endif
#ifndef MAX_PATH
 #define MAX_PATH 260		//!< MAX_PATH is 260
#endif

//BorlandC++ Compiler
#ifdef __BORLANDC__
 //#define sqrtf(df) sqrt(df)
 int _matherr(struct _exception  *__e) { return 1; }
#endif

//Macro for handling unused arguments.
#ifndef MAIN_H
 #define UNREFERENCED_PARAMETER(P) ((P)=(P))
#endif

//arithmetic function of float type prepared
#ifndef _MSC_VER
 #define sinf(x) (float)sin(x)				//!< sinf(x)
 #define cosf(x) (float)cos(x)				//!< cosf(x)
 #define asinf(x) (float)asin(x)			//!< asinf(x)
 #define acosf(x) (float)acos(x)			//!< acosf(x)
 #define atan2f(y,x) (float)atan2(y,x)		//!< atan2f(y,x)
 #define sqrtf(x) (float)sqrt(x)			//!< sqrtf(x)
 #define floorf(x) (float)floor(x)			//!< floorf(x)
 #define ceilf(x) (float)ceil(x)			//!< ceilf(x)
 #define fabsf(x) (float)fabs(x)			//!< fabsf(x)
#endif

#define SetFlag(value, bit) value = value | bit		//!< Flag by bit Settings
#define DelFlag(value, bit) value = value & (~bit)	//!< Clear flag bits.
#define GetFlag(value, bit) (value & bit)			//!< Get flag bits.

#define DegreeToRadian(value) ((float)M_PI/180*value)	//!< Convert degrees to radians.

//Low Layer
#if H_LAYERLEVEL >= 1
 #include "debug.h"
 #include "window.h"
 #include "config.h"
 #include "datafile.h"
 #include "d3dgraphics.h"
 #include "input.h"
 #include "sound.h"
 #include "parameter.h"
#endif

//Middle Layer
#if H_LAYERLEVEL >= 2
 #include "resource.h"
 #include "collision.h"
 #include "object.h"
 #include "soundmanager.h"
 #include "gui-object.h"
#endif

//High Layer
#if H_LAYERLEVEL >= 3
 #include "objectmanager.h"
 #include "scene.h"
 #include "gamemain.h"
 #include "ai.h"
 #include "statemachine.h"
 #include "event.h"
#endif

//Additional libraries
// not .lib

#ifdef ENABLE_DEBUGLOG
 extern class DebugLog OutputLog;
#endif
extern class StateMachine GameState;
extern class Config GameConfig;

bool CheckCommandParameter(const char *param, const char *cmd);
float AngleNormalization(float r);
unsigned int PointParamToInt(signed short int value);
void Set2ByteLittleEndian(unsigned char *outdata, unsigned int value);
void Set4ByteLittleEndian(unsigned char *outdata, unsigned int value);
void Set2ByteBigEndian(unsigned char *outdata, unsigned int value);
void Set4ByteBigEndian(unsigned char *outdata, unsigned int value);
unsigned int Get2ByteLittleEndian(unsigned char *indata);
unsigned int Get4ByteLittleEndian(unsigned char *indata);
unsigned int Get2ByteBigEndian(unsigned char *indata);
unsigned int Get4ByteBigEndian(unsigned char *indata);

#endif