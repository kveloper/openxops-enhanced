//! @file parameter.h
//! @brief ParameterInfo class

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

// Check character, weapon, and small-object configuration data.
//  Manda (December 2014)

#ifndef PARAMETER_H
#define PARAMETER_H

#define TOTAL_PARAMETERINFO_HUMAN 43			//!< Number of people
#define TOTAL_HUMANTEXTURE 30					//!< Total number of human textures
#define TOTAL_PARAMETERINFO_WEAPON 23			//!< Number of weapons
#define TOTAL_PARAMETERINFO_SMALLOBJECT 12	//!< Number of small items
#define TOTAL_PARAMETERINFO_BULLET 2		//!< Number of bullet object settings
#define TOTAL_OFFICIALMISSION 58			//!< Number of standard missions
#define TOTAL_PARAMETERINFO_AILEVEL 6		//!< Number of AI Levels
#define TOTAL_PARAMETERINFO_SCOPE 4			//!< Scope setting number

#define ID_WEAPON_NONE 0		//!< Weapon: None Weapon Number
#define ID_WEAPON_GRENADE 13	//!< Weapon number that handles as a grenades
#define ID_WEAPON_CASE 15		//!< Weapon number to handle as a case

#define OFFICIALMISSION_BD1 "temp.bd1"		//!< BD1 file name for standard mission

#define SMALLOBJECT_SCALE 5.0f				//!< Drawing magnification of small items

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 1		//!< Select include file.
#endif
#include "main.h"

//! Structure for human setting
typedef struct
{
	int texture;		//!< Texture number
	int model;			//!< Model Number
	int hp;				//!< Health.
	int AIlevel;		//!< AI Level
	int AItrackability;	//!< Adherence in AI attack (Z)e only)
	int Weapon[TOTAL_HAVEWEAPON];	//!< Weapons
	int type;			//!< Types
} HumanParameter;

//! Structure for weapon setting
typedef struct
{
	const char *name;		//!< Weapon Name
	const char *model;		//!< Model File Name
	const char *texture;	//!< Texture File Name
	int attacks;		//!< Attack Force
	int penetration;	//!< Penetration strength.
	int blazings;		//!< Firing interval.
	int speed;			//!< Bullet speed.
	int nbsmax;			//!< Loaded ammunition count.
	int reloads;		//!< Reload time
	int reaction;		//!< Recoil angle.
	int ErrorRangeMIN;	//!< Ballistic Error Min
	int ErrorRangeMAX;	//!< Ballistic Error Max
	float mx;		//!< Gun Position: X
	float my;		//!< Gun Position:Y
	float mz;		//!< Gun Position: Z
	float flashx;	//!< Position of the mazul flash: X
	float flashy;	//!< Location of Masul Flash: Y
	float flashz;	//!< Position of the mazul flash: Z
	float yakkyou_px;	//!< Shell-casing ejection position: X.
	float yakkyou_py;	//!< Shell-casing ejection position: Y.
	float yakkyou_pz;	//!< Shell-casing ejection position: Z.
	float yakkyou_sx;	//!< Initial shell-casing velocity: X.
	float yakkyou_sy;	//!< Initial shell-casing velocity: Y.
	float yakkyou_size;	//!< Shell-casing rendering scale (negative values derive the scale from damage).
	int yakkyou_delay;	//!< Delay counting
	int burstcnt;		//!< Total number of bullets (0: Full Auto 1: Semi-Auto 2 or higher: Burst mode)
	int scopemode;		//!< Scope mode
	float size;			//!< Drawing Magnification
	int soundid;		//!< Firing-sound ID.
	int soundvolume;	//!< Firing-sound volume.
	bool silencer;		//!< Silencer or not
	int WeaponP;		//!< Equipment method
	int ChangeWeaponID;		//!< Weapons to switch
	int ChangeWeaponCnt;	//!< Weapon Switching Time
	int pellet;			//!< Number of pellets
} WeaponParameter;

//! Structure for small parts setting
typedef struct
{
	const char *model;		//!< Model name
	const char *texture;	//!< Texture Name
	int decide;		//!< Collision volume size.
	int hp;			//!< Durability.
	int sound;		//!< Impact sound-effect ID.
	int jump;		//!< Knockback strength.
} SmallObjectParameter;

//! Structure for bullet objects
typedef struct
{
	const char *model;		//!< Model name
	const char *texture;	//!< Texture Name
	float size;				//!< Drawing Magnification
} BulletParameter;

//! Structure for standard mission setting
typedef struct
{
	const char *name;				//!< Mission identification name
	const char *fullname;			//!< Mission official name
	const char *directory;			//!< Data Directory
	const char *txt;				//!< File Name (.pd1/.txt)
	bool collision;			//!< Flag to indicate additional per-deter
	bool screen;			//!< Flag to darken the screen
} OfficialMissionParameter;

//! Structure for setting (performance value) by AI level
typedef struct
{
	int aiming;			//!< Aiming Ability
	int attack;			//!< Aggressiveness.
	int search;			//!< Enemy detection ability.
	int limitserror;	//!< Tolerance
} AIParameter;

//! Structure for scope setting
typedef struct
{
	float ViewAngle;				//!< Viewing Angle
	float MouseRange;				//!< Operation Sensitivity
	int ScopeGunsight;				//!< Determination display when using scope (0: Normally, 1: Simple, 2: Precision)
	bool NoScopeGunsight;			//!< Normal crosshair display.
	int WeaponNoScopeReaction;		//!< Recoil during normal firing.
	float WeaponMotionReactionY;	//!< Reaction at the time of gunning (motion)
	bool WeaponRecoil_ScopeOnly;	//!< Scope Reaction at the time of gunning Only at scope
	float WeaponRecoil_ScopeRx;		//!< Scope Reaction at the time of the gun horizontal axis Note: Number method 0.1f
	float WeaponRecoil_ScopeRyMin;	//!< Scope Reaction at the time of the gun Minimum of the vertical axis *Number method 0.1f
	float WeaponRecoil_ScopeRyMax;	//!< Maximum vertical recoil while scoped, in increments of 0.1 degrees.
	float AI_ShotAngle;							//!< Angle to capture the enemy when AI is triggered
	float AI_ShotAngleLong;						//!< Angle to capture the enemy at the time of AI gunning (long distance)
	float AI_AddSearchEnemyMaxDist_Normal;		//!< Distance to find an AI enemy Addition (usually)
	float AI_AddSearchEnemyMaxDist_Caution;		//!< AI Distance to find enemies Addition (when wary)
} ScopeParameter;

//! @brief Class to manage set values
//! @details Manage pre-fixed configuration values.
//! @details This is the setting value related to the standard mission from people, weapons, and accessories.
class ParameterInfo
{
	HumanParameter *Human;								//!< Pointer to store information of people
	const char *HumanTexturePath[TOTAL_HUMANTEXTURE];	//!< Pointer to store human textures
	WeaponParameter *Weapon;							//!< Pointer to store weapon information
#ifdef ENABLE_BUG_HUMANWEAPON
	WeaponParameter *BugWeapon;							//!< Pointer to store bug weapon information
#endif
	SmallObjectParameter *SmallObject;					//!< Pointer to store information of small items
	BulletParameter *Bullet;							//!< Pointer to store information of bullet objects
	OfficialMissionParameter *MissionData;				//!< Pointer to store standard mission information
	AIParameter *AIlevel;								//!< Performance value of AI
	ScopeParameter *Scope;								//!< Pointer to store scope settings

public:
	ParameterInfo();
	~ParameterInfo();
	void InitInfo();
	void DestroyInfo();
	int GetHuman(int id, HumanParameter *out_data);
	int GetHumanTexturePath(int id, char *out_str);
	int GetWeapon(int id, WeaponParameter *out_data);
#ifdef ENABLE_BUG_HUMANWEAPON
	int GetBugWeapon(int id, WeaponParameter *out_data);
#endif
	int GetSmallObject(int id, SmallObjectParameter *out_data);
	int GetBullet(int id, BulletParameter *out_data);
	int GetOfficialMission(int id, char *name, char *fullname, char* directory, char *txt, bool *collisionflag, bool *screenflag);
	int GetAIlevel(int level, AIParameter **out_AIlevel);
	int GetScopeParam(int id, ScopeParameter *out_data);
};

#endif