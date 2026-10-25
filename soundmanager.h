//! @file soundmanager.h
//! @brief SoundManager class declaration

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

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#define MAX_SOUNDMGR_LIST 100		//!< Maximum number of soundlists

#define MAX_SOUNDHITMAP 95			//!< Maximum sound volume
#define MAX_SOUNDHITSMALLOBJ 110	//!< Maximum volume of bullet impacts on small objects.
#define MAX_SOUNDHITHUMAN 75		//!< Maximum volume of sound
#define MAX_SOUNDPASSING 80			//!< Maximum volume of the sound that the bullet crosses
#define MAX_SOUNDCCOGRENADE 100		//!< Maximum grenade bounce/reflection volume.
#define MAX_SOUNDHITGRENADE 120		//!< Maximum grenade explosion volume.

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 2		//!< Select include file.
#endif
#include "main.h"

//! Constant for the sound list "paramid"
enum SoundmgrID
{
	SHOT_WEAPON = 0,		//!< Firing sound.
	SHOT_WEAPON_PLAYER,		//!< Player's own cannon
	HIT_MAP,				//!< Bullet impact sound on the map.
	HIT_HUMAN_HEAD,			//!< Hit sound (head).
	HIT_HUMAN_UP,			//!< Hit sound (upper body).
	HIT_HUMAN_LEG,			//!< Hit sound (lower body).
	HIT_HUMAN_ZOMBIE,		//!< Zombie attack sound.
	HIT_SMALLOBJECT,		//!< Small-object destruction sound.
	BULLET,					//!< Bullet flyby sound.
	GRE_BOUND,				//!< Grenade bounce sound.
	GRE_EXPLOSION,			//!< Grenade explosion sound.
	FOOTSTEPS_WALK,			//!< Footsteps
	FOOTSTEPS_FORWARD,		//!< Footsteps/Running Sounds (Advance)
	FOOTSTEPS_BACK,			//!< Footsteps while running backward.
	FOOTSTEPS_SIDE,			//!< Footsteps and Sounds Running (Left and Right)
	FOOTSTEPS_JUMP,			//!< Footsteps & Jumps
	FOOTSTEPS_LANDING,		//!< Footsteps on landing.
	WEAPON_RELOAD			//!< Reload Sound
};

//! Structure for Sound List
typedef struct
{
	int paramid;	//!< Sound source type number / SoundmgrID constant
	int dataid;		//!< Data Number
	float x;		//!< X Coordinate
	float y;		//!< Y Coordinate
	float z;		//!< Z Coordinates
	float move_x;	//!< X-coordinate moving amount after one frame
	float move_y;	//!< Y-coordinate moving amount after one frame
	float move_z;	//!< Z-coordinate moving amount after one frame
	int teamid;		//!< Team number
	int cnt;		//!< Count Value
} soundlist;

//! @brief Sound Management Class
//! @details Initialize, compute, and play each sound.
class SoundManager
{
	class SoundControl *SoundCtrl;		//!< Sound playback class
	class ResourceManager *Resource;	//!< Resource Management Class
	class ParameterInfo *Param;			//!< Class to manage configuration values
	soundlist *soundlistA;		//!< Soundlist A
	soundlist *soundlistB;		//!< Soundlist B
	bool changeAB;				//!< Reversing the sound list
	int listAdatas;				//!< Sound List A
	int listBdatas;				//!< Number of entries in sound list B.

	bool GetNewList(soundlist **plist);
	int GetTargetList(soundlist **plist);
	bool CheckApproach(soundlist *plist, float camera_x, float camera_y, float camera_z, float *min_x, float *min_y, float *min_z);
	void PlaySound(soundlist *plist, float camera_x, float camera_y, float camera_z, int teamID);

public:
	SoundManager(SoundControl *in_SoundCtrl = NULL, ResourceManager *in_Resource = NULL, ParameterInfo *in_Param = NULL);
	~SoundManager();
	void SetClass(SoundControl *in_SoundCtrl, ResourceManager *in_Resource, ParameterInfo *in_Param);
	void InitWorldSound();
	bool ShotWeapon(float x, float y, float z, int id, int teamID, bool player);
	bool HitMap(float x, float y, float z, int teamID);
	bool HitHuman(float x, float y, float z, int Hit_id, int teamID);
	bool HitSmallObject(float x, float y, float z, int id, int teamID);
	bool PassingBullet(float x, float y, float z, float move_x, float move_y, float move_z, int teamID);
	bool GrenadeBound(float x, float y, float z, int teamID);
	bool GrenadeExplosion(float x, float y, float z, int teamID);
	bool SetFootsteps(float x, float y, float z, int id, int MoveMode, int teamID, int cnt);
	bool ReloadWeapon(float x, float y, float z, int teamID);
	int GetTotalSoundList();
	int GetWorldSound(float pos_x, float pos_y, float pos_z, int teamID, soundlist *psoundlist);
	void PlayWorldSound(float camera_x, float camera_y, float camera_z, float camera_rx, int teamID);
};

#endif