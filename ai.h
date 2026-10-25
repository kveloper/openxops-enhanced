//! @file ai.h
//! @brief AIcontrol Class Declaration

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

#ifndef AI_H
#define AI_H

#define AI_TOTALHUMAN_SCALE (MAX_HUMAN/24)		//!< Maximum character count multiplier, per 24 characters.

#define AI_ADDTURNRAD DegreeToRadian(0.8f)	//!< AI turning ability.
#define AI_SEARCH_RX DegreeToRadian(110)	//!< angle to discover the enemy
#define AI_SEARCH_RY DegreeToRadian(60)		//!< Vertical axis angle to discover the enemy
#define AI_ZOMBIEATTACK_ARMRY DegreeToRadian(-15)		//!< Arm angle during a zombie attack.

#define AI_ARRIVALDIST_PATH 5.0f			//!< Distance to determine if the path is reached
#define AI_ARRIVALDIST_TRACKING 18.0f		//!< Distance to determine that the target is reached
#define AI_ARRIVALDIST_WALKTRACKING 24.0f	//!< Distance approaching to walking (without running)
#define AI_CHECKJUMP_HEIGHT 0.3f			//!< Height to determine whether to block before
#define AI_CHECKJUMP_DIST 2.0f				//!< Distance to determine whether to block before
#define AI_CHECKJUMP2_DIST 10.0f			//!< Distance to determine whether to block before (priority running)
#define AI_CHECKBULLET 20.0f				//!< Distance to determine if the bullet passes near
#define AI_CHECKSHORTATTACK_DIST 200.0f		//!< Distance to judge as a short distance attack

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 3		//!< Select include file.
#endif
#include "main.h"

//! @brief Class for AI control
//! @details We control the AI in general such as moving by path, recognition of the enemy in the field of view, attack, and use of weapons.
class AIcontrol
{
	class ObjectManager *ObjMgr;		//!< Pointer to the ObjectManager class.
	class BlockDataInterface *blocks;	//!< Pointer to a class that manages block data
	class ParameterInfo *Param;		//!< Pointer to a class that manages the setting value
	class Collision *CollD;				//!< Pointer to a class that manages the perception
	class SoundManager *GameSound;		//!< Pointer to the game effect sound playback class

	class AIMoveNavi *MoveNavi;			//!< Pointer of Human Movement Management Class
	class AIObjectDriver *ObjDriver;	//!< Pointer of human movement rotation control class

	int AIlevel;		//!< AI Level
	int battlemode;		//!< Battle Mode
	bool NoFight;		//!< Non-Combating Flags
	float posx;			//!< X Coordinate
	float posy;			//!< Y Coordinate
	float posz;			//!< Z Coordinates
	float rx;			//!< X-axis rotation angle
	float ry;			//!< Y-axis rotation angle
	float cautionback_posx;		//!< X coordinates back after warning
	float cautionback_posz;		//!< Z coordinates back after warning
	bool FaceCaution_flag;		//!< Flags for specific reports during wary
	float FaceCaution_rx;		//!< Directions toward the wary
	float total_move;			//!< Total travel
	int waitcnt;				//!< Time wait count
	int movejumpcnt;			//!< Jumping Count
	int gotocnt;				//!< Moving Count
	int cautioncnt;				//!< Precaution Counting
	int actioncnt;				//!< Attacking Count
	bool longattack;			//!< Close-range/long-range flag.
	bool EventWeaponShot;		//!< Grenade Throw Flag by Events
	AIParameter *LevelParam;	//!< Performance value of AI

	bool CheckTargetPos(bool back);
	void MoveTarget(bool back);
	void MoveTarget2(bool back);
	void MoveRandom();
	void TurnSeen();
	bool StopSeen();
	bool MoveJump();
	void Action();
	bool ActionCancel();
	int HaveWeapon();
	void CancelMoveTurn();
	int ControlWeapon();
	int ThrowGrenade();
	void ArmAngle();
	int SearchEnemy();
	int SearchShortEnemy();
	bool CheckLookEnemy(int id, float search_rx, float search_ry, float maxDist, float *out_minDist);
	bool CheckLookEnemy(class human* thuman, float search_rx, float search_ry, float maxDist, float *out_minDist);
	bool CheckCorpse(int id);
	void MovePath();
	bool ActionMain();
	bool CautionMain();
	bool NormalMain();

	int ctrlid;					//!< Individual number (controlled)
	class human *ctrlhuman;		//!< human class for yourself (controlled)
	class human *enemyhuman;	//!< human class for attack

public:
	AIcontrol(class ObjectManager *in_ObjMgr = NULL, int in_ctrlid = -1, class BlockDataInterface *in_blocks = NULL, class PointDataInterface *in_Points = NULL, class ParameterInfo *in_Param = NULL, class Collision *in_CollD = NULL, class SoundManager *in_GameSound = NULL);
	~AIcontrol();
	void SetClass(class ObjectManager *in_ObjMgr, int in_ctrlid, class BlockDataInterface *in_blocks, class PointDataInterface *in_Points, class ParameterInfo *in_Param, class Collision *in_CollD, class SoundManager *in_GameSound);
	void Init();
	void SetAIlevel(int level);
	int GetAIlevel();
	void SetHoldWait(float px, float pz, float rx);
	void SetHoldTracking(int id);
	void SetCautionMode();
	void SetNoFightFlag(bool flag);
	void GetBattleMode(int *mode, char *modestr);
	int GetEnemyHumanID();
	void GetMoveTargetPos(float *posx, float *posz, int *movemode);
	void GetPathPointData(pointdata *out_data);
	void Process();
};

//! @brief Human mobility management class
//! @details Class that determines a character's destination in normal mode. Used inside AIcontrol.
class AIMoveNavi
{
	class ObjectManager *ObjMgr;		//!< Pointer to the ObjectManager class.
	int ctrlid;							//!< Individual number (controlled)
	class PointDataInterface *Points;	//!< Pointer to a class that manages point data

	int movemode;			//!< Moving mode
	bool hold;				//!< Don't read the moving path
	int path_pointid;		//!< Point data number of path
	int target_humanid;		//!< Data number of targeted persons
	float target_posx;		//!< Target X coordinates
	float target_posz;		//!< Target Z coordinates
	float target_rx;		//!< Target horizontal angle

public:
	AIMoveNavi(class ObjectManager *in_ObjMgr = NULL, int in_ctrlid = -1, class PointDataInterface *in_Points = NULL);
	~AIMoveNavi();
	void SetClass(class ObjectManager *in_ObjMgr, int in_ctrlid, class PointDataInterface *in_Points);
	void Init();
	bool MovePathNowState();
	bool MovePathNextState();
	void SetHoldWait(float px, float pz, float rx);
	void SetHoldTracking(int id);
	int GetMoveMode();
	bool GetRun2();
	int GetTargetHumanID();
	void GetPathPointData(pointdata *out_data);
	void GetTargetPos(float *posx, float *posz, float *rx, int *out_movemode, int *out_pointmode);
};

//! @brief Human Moving Rotation Control Class
//! @details Class that manages character movement and rotation using flags. Used inside AIcontrol.
class AIObjectDriver
{
	class ObjectManager *ObjMgr;		//!< Pointer to the ObjectManager class.
	int ctrlid;				//!< Individual number (controlled)

	int moveturn_mode;		//!< Move direction and rotation direction flags
	float addrx;			//!< X-axis rotation angle acceleration
	float addry;			//!< Y-axis rotation angle acceleration

public:
	AIObjectDriver(class ObjectManager *in_ObjMgr = NULL, int in_ctrlid = -1);
	~AIObjectDriver();
	void SetClass(class ObjectManager *in_ObjMgr, int in_ctrlid);
	void Init();
	void ResetMode();
	void SetModeFlag(int flag);
	void DelModeFlag(int flag);
	bool GetModeFlag(int flag);
	void ControlObject();
};

//! Constant representing AI control mode
//! @warning When changing the constants, also update the string output in GetBattleMode() as needed.
enum AImode
{
	AI_DEAD = 0,	//!< Who has died
	AI_ACTION,		//!< Character in combat.
	AI_CAUTION,		//!< Character on alert.
	AI_NORMAL,		//!< Normal mode

	AI_WALK,		//!< People walking by moving path
	AI_RUN,			//!< People running by moving path
	AI_WAIT,		//!< People waiting by path
	AI_STOP_5SEC,	//!< People waiting for time by path (5 seconds)
	AI_TRACKING,	//!< Pursuing a specific person
	AI_GRENADE,		//!< Throwing grenades
	AI_RUN2,		//!< Priority running
	AI_RANDOM,		//!< Random path processing
	AI_NULL			//!< No path.
};

//! A constant representing AI moving mode
enum AIMoveNaviFlag
{
	AI_NAVI_MOVE_NULL,
	AI_NAVI_MOVE_WALK,
	AI_NAVI_MOVE_RUN,
	AI_NAVI_MOVE_RUN2,
	AI_NAVI_MOVE_TRACKING,

	AI_NAVI_POINT_NULL,
	AI_NAVI_POINT_WAIT,
	AI_NAVI_POINT_STOP_5SEC,
	AI_NAVI_POINT_TRACKING,
	AI_NAVI_POINT_GRENADE
};

//! A constant representing AI operation mode
enum AIcontrolFlag
{
	AI_CTRL_MOVEFORWARD = 0x0001,
	AI_CTRL_MOVEBACKWARD = 0x0002,
	AI_CTRL_MOVELEFT = 0x0004,
	AI_CTRL_MOVERIGHT = 0x0008,
	AI_CTRL_MOVEWALK = 0x0010,
	AI_CTRL_TURNUP = 0x0100,
	AI_CTRL_TURNDOWN = 0x0200,
	AI_CTRL_TURNLEFT = 0x0400,
	AI_CTRL_TURNRIGHT = 0x0800
};

#endif