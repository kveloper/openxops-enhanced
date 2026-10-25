//! @file object.h
//! @brief Object class declaration

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

#ifndef OBJECT_H
#define OBJECT_H

// When changing speed, also review the collision-detection constants.
#define HUMAN_PROGRESSRUN_ACCELERATION	0.7f			//!< Human advance (running) acceleration
#define HUMAN_SIDEWAYSRUN_ACCELERATION 0.5f			//!< Man's side driving acceleration
#define HUMAN_REGRESSRUN_ACCELERATION 0.38f			//!< Character acceleration when moving backward.
//! Acceleration when a character moves diagonally forward.
#define HUMAN_PROGRESSRUN_SIDEWAYSRUN_ACCELERATION ((HUMAN_PROGRESSRUN_ACCELERATION + HUMAN_SIDEWAYSRUN_ACCELERATION) / 2)
//! Acceleration in which a person is oblique behind
#define HUMAN_REGRESSRUN_SIDEWAYSRUN_ACCELERATION (HUMAN_REGRESSRUN_ACCELERATION)
#define HUMAN_PROGRESSWALK_ACCELERATION	0.35f		//!< Accelerometers walking by people
#define HUMAN_ATTENUATION			0.5f		//!< Rate per person per frame (0.0 < x < 1.0)

#define HUMAN_JUMP_SPEED 2.2f					//!< Jumping Speed

#define HUMAN_HEIGHT 20.0f			//!< Person Height
#define HUMAN_DAMAGE_HEAD 2.0f		//!< Damage magnification when the bullet hits the head
#define HUMAN_DAMAGE_UP 1.0f		//!< Damage magnification when the bullet hits the upper body
#define HUMAN_DAMAGE_LEG 0.7f		//!< Damage magnification when the bullet hits the lower body
#define HUMAN_DAMAGE_ZOMBIEU 15		//!< Minimum damage received from a zombie attack.
#define HUMAN_DAMAGE_ZOMBIEA 5		//!< Additional damages when z es are attacked
#define HUMAN_DAMAGE_SPEED 0.066f	//!< Drop amount
#define HUMAN_DAMAGE_MINSPEED -3.8f	//!< Speed that can not be damaged due to fall
#define HUMAN_DAMAGE_MAXSPEED -7.0f	//!< Maximum drop speed
#define HUMAN_DAMAGE_MAXFALL 120	//!< Max damage caused by fall
#define MAX_DAMAGE_GRENADE_DISTANCE 80.0f		//!< Maximum distance damaged due to grenades
#define HUMAN_DAMAGE_GRENADE_HEAD 100			//!< Maximum damage to the head due to grenades
#define HUMAN_DAMAGE_GRENADE_LEG 80				//!< Maximum damage to the foot due to grenades
#define SMALLOBJECT_DAMAGE_GRENADE 80			//!< Maximum damage to small items due to hand grenades

#define ARMRAD_NOWEAPON DegreeToRadian((-90 + 20))			//!< Arm rendering angle when unarmed.
#define ARMRAD_RELOADWEAPON DegreeToRadian(-20)				//!< Reloading arm drawing angle

#define HUMAN_MAPCOLLISION_R 2.8f							//!< Radius for character-map collision detection.
#define HUMAN_MAPCOLLISION_HEIGHT 10.0f						//!< Measures to determine the perception of people and maps (Note: Waist)
#define HUMAN_MAPCOLLISION_ADD_HEIGHT_A 9.0f				//!< Determination for people and maps Additions (Note: Conform to standard map SCHOOL)
#define HUMAN_MAPCOLLISION_ADD_HEIGHT_B 13.0f				//!< Determination for people and maps Additions (Note: Conform to standard map SCHOOL)
#define HUMAN_MAPCOLLISION_SLOPEANGLE DegreeToRadian(50)	//!< Determination of people and maps Angle of the slope that cannot be climbed
#define HUMAN_MAPCOLLISION_CLIMBHEIGHT 3.2f					//!< Determination of people and maps Height to climb
#define HUMAN_MAPCOLLISION_CLIMBADDY 0.4f					//!< Climb height per frame for character-map collision detection.
#define HUMAN_MAPCOLLISION_GROUND_HEIGHT -0.5f				//!< Ground-check height for character-map collision detection.
#define HUMAN_MAPCOLLISION_GROUND_R1 0.15f					//!< Ground-check radius for character-map collision detection (1).
#define HUMAN_MAPCOLLISION_GROUND_R2 0.50f					//!< Ground-check radius for character-map collision detection (2).
#define HUMAN_MAPCOLLISION_GROUND_R3 0.30f					//!< Ground-check radius for character-map collision detection (3).
#define HUMAN_MAPCOLLISION_CHECK_MAXDIST 12.0f				//!< Maximum travel distance per frame for character-map collision detection.
#define HUMAN_MAPCOLLISION_CHECK_HEIGHT (HUMAN_HEIGHT-3.0f)	//!< Height used for the final character-map movement check (approximately shoulder height).
#define HUMAN_DEADLINE -100.0f						//!< Y-coordinate (deadline) that a person died
#define HUMAN_DEADADDRY DegreeToRadian(0.75f)		//!< Deadly Acceleration

#define BULLET_SPEEDSCALE 2.5f			//!< Speed magnification
#define BULLET_DESTROYFRAME 40			//!< Number of frames
#define GRENADE_DESTROYFRAME 100		//!< Number of frames until a grenade explodes.

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 2		//!< Select include file.
#endif
#include "main.h"

#define HUMAN_DEADBODY_COLLISION	//!< @brief Check map collisions while a character falls. @details Defined: check collisions; undefined (commented out): simply let the character fall.

//! @brief Object management class (base class)
//! @details It is a base class of the class group that manages all objects that perform coordinate management and drawing in 3D.
#include "render-timing.h"

class object
{
    RenderPoseHistory<4> renderHistory;
    float renderSaved[4]={};
    bool renderPoseApplied=false;

protected:
	class ParameterInfo *Param;	//!< Pointer to a class that manages the setting value
	float pos_x;		//!< X Coordinate
	float pos_y;		//!< Y Coordinate
	float pos_z;		//!< Z Coordinates
	float rotation_x;	//!< Rotation Angle
	float model_size;	//!< Drawing size
	int id_parameter;	//!< Data Types
	int id_model;		//!< Model Data Number
	int id_texture;		//!< Texture Data Number
	bool DarkModelFlag;	//!< Dark Model
	bool EnableFlag;	//!< Enabled flags

public:
    void CaptureRenderState();
    void ApplyRenderState();
    void RestoreRenderState();

	object(class ParameterInfo *in_Param = NULL, float x = 0.0f, float y = 0.0f, float z = 0.0f, float rx = 0.0f, float size = 1.0f, bool flag = true);
	~object();
	virtual void SetParameterInfoClass(class ParameterInfo *in_Param);
	virtual void SetPosData(float x, float y, float z, float rx);
	virtual void GetPosData(float *x, float *y, float *z, float *rx);
	virtual void SetEnableFlag(bool flag);
	virtual bool GetEnableFlag();
	virtual void SetModel(int id, float size);
	virtual void GetModel(int *id, float *size);
	virtual void SetTexture(int id);
	virtual int GetTexture();
	virtual void SetDarkModelFlag(bool flag);
	virtual int ProcessObject();
	virtual void Render(class D3DGraphics *d3dg, bool NoModel);
};

//! @brief Human management class
class human : public object
{
protected:
	int point_dataid;			//!< Point Data Number
	signed short int point_p4;	//!< Identification number of points
	int teamid;					//!< Team number
	float move_x;				//!< X-axis speed
	float move_y;				//!< Y-axis (fall) speed
	float move_z;				//!< Z-axis speed
	bool move_y_flag;			//!< Y-axis moving flag
	int move_y_upper;			//!< Y-axis lifting counter
	bool move_y_landing;		//!< Landing flag.
	int underblock_id;			//!< Footer block ID
	int underblock_face;		//!< Footer Block Face Number
	float rotation_y;			//!< Overall rotation angle
	float armrotation_y;		//!< Arm Rotation Angle
	float upmodel_size;			//!< Upper body drawing size
	float armmodel_size;		//!< Arm Drawing Size
	float legmodel_size;		//!< Foot drawing size
	class weapon *weapon[TOTAL_HAVEWEAPON];			//!< Weapons
	int noweapon_bullets[TOTAL_HAVEWEAPON];			//!< Total number of bullets when weapons are not
	int noweapon_loadbullets[TOTAL_HAVEWEAPON];		//!< Number of bullets when weapons are not in
	int selectweapon;			//!< Select weapon A/B
	int selectweaponcnt;		//!< Weapon-switch counter.
	int changeweaponidcnt;		//!< Weapon Type Switching Count
	int weaponshotcnt;			//!< Weapon Counting
	int weaponreloadcnt;		//!< Weapon Reload Count
	bool weaponshotframe;		//!< Firing flag.
	int weaponburstmodecnt;		//!< Burst Mode Count
	int hp;						//!< Health.
	int deadstate;				//!< Is it dead?
	float add_ry;				//!< Deadly Acceleration
	float move_rx;				//!< Moving Angle
	int MoveFlag;				//!< Flag representing the direction of movement
	int MoveFlag_lt;			//!< flag representing the moving direction (previous)
	int scopemode;				//!< Scope Use Mode
	bool HitFlag;				//!< Flags that represent the bullet
	float Hit_rx;				//!< Direction from which the hit was received.
	float totalmove;			//!< Total travel
	int StateGunsightErrorRange;		//!< Determination status error
	int ReactionGunsightErrorRange;		//!< Rebound Errors
	bool Invincible;			//!< Invincibility flag.

	class HumanMotionControl *MotionCtrl;	//!< Pointer to a class that controls motion

	void GunsightErrorRange();
	int CheckAndProcessDead(class Collision *CollD);
	void ControlProcess();
	void CollisionMap(class Collision *CollD, class BlockDataInterface *inblockdata, bool AddCollisionFlag, bool player);
	bool CollisionBlockScratch(class Collision *CollD, class BlockDataInterface *inblockdata, int blockid, float *px, float *py, float *pz, float px_old, float py_old, float pz_old, float in_vx, float in_vy, float in_vz, int mode);

public:
	human(class ParameterInfo *in_Param = NULL, float x = 0.0f, float y = 0.0f, float z = 0.0f, float rx = 0.0f, int id_param = -1, int dataid = 0, signed short int p4 = 0, int team = 0, bool flag = false);
	~human();
	virtual void SetParameterInfoClass(class ParameterInfo *in_Param);
	virtual void SetParamData(int id_param, int dataid, signed short int p4, int team, bool init);
	virtual void GetParamData(int *id_param, int *dataid, signed short int *p4, int *team);
	virtual void GetMovePos(float *x, float *y, float *z);
	virtual void SetModel(int upmodel, int armmodel[], int legmodel, int walkmodel[], int runmodel[]);
	virtual int GetHP();
	virtual bool SetHP(int in_hp);
	virtual bool GetDeadFlag();
	virtual bool GetDeadMotionFlag();
	virtual void SetTeamID(int id);
	virtual bool GetInvincibleFlag();
	virtual void SetInvincibleFlag(bool flag);
	virtual void SetWeapon(class weapon *in_weapon[], int nlnbs[], int nnbs[]);
	virtual int PickupWeapon(class weapon *in_weapon);
	virtual void ChangeHaveWeapon(int id = -1);
	virtual int GetChangeHaveWeaponCnt();
	virtual int ChangeWeaponID();
	virtual int GetChangeWeaponIDCnt();
	virtual int GetWeaponReloadCnt();
	virtual void GetWeapon(int *out_selectweapon, class weapon *out_weapon[], int nlnbs[], int nnbs[]);
	virtual int GetMainWeaponTypeNO();
	virtual bool GetWeaponBlazingmode();
	virtual bool ShotWeapon(int *weapon_paramid, int *GunsightErrorRange);
	virtual bool ReloadWeapon();
	virtual bool DumpWeapon();
	virtual void SetMoveForward();
	virtual void SetMoveBack();
	virtual void SetMoveLeft();
	virtual void SetMoveRight();
	virtual void SetMoveWalk();
	virtual int GetMovemode(bool nowdata);
	virtual int GetMoveFlag(bool nowdata);
	virtual bool SetEnableScope();
	virtual void SetDisableScope();
	virtual int GetScopeMode();
	virtual void GetRxRy(float *rx, float *ry);
	virtual void SetRxRy(float rx, float ry);
	virtual float GetDeadRy();
	virtual void Jump();
	virtual int GetJumpLanding(bool nowdata);
	virtual void GetUnderBlock(int *id, int *face);
	virtual void AddPosOrder(float rx, float ry, float speed);
	virtual void HitBulletHead(int attacks);
	virtual void HitBulletUp(int attacks);
	virtual void HitBulletLeg(int attacks);
	virtual void HitZombieAttack();
	virtual void HitGrenadeExplosion(int attacks);
	virtual void SetHitFlag(float rx);
	virtual bool CheckHit(float *rx);
	virtual float GetTotalMove();
	virtual int GetMoveMotionCount();
	virtual int ProcessObject(class Collision *CollD, class BlockDataInterface *inblockdata, bool AddCollisionFlag, bool player, bool F5mode);
	virtual int GetGunsightErrorRange();
	virtual void Render(class D3DGraphics *d3dg, class ResourceManager *Resource, bool DrawArmOnly, bool player, bool NoModel);
};

//! @brief Weapon Management Class
class weapon : public object
{
protected:
	float move_x;		//!< X-axis travel
	float move_y;		//!< Y-axis travel
	float move_z;		//!< Z-axis travel
	bool usingflag;		//!< Flags for use
	int bullets;		//!< Total ammunition count.
	int Loadbullets;	//!< Loaded ammunition count.
	bool motionflag;	//!< Flag representing the coordinate moving

public:
	weapon(class ParameterInfo *in_Param = NULL, float x = 0.0f, float y = 0.0f, float z = 0.0f, float rx = 0.0f, int id_param = 0, int nbs = 0, bool flag = false);
	~weapon();
	virtual void SetPosData(float x, float y, float z, float rx);
	virtual void SetParamData(int id_param, int lnbs, int nbs, bool init);
	virtual void GetParamData(int *id_param, int *lnbs, int *nbs);
	virtual bool GetUsingFlag();
	virtual int Pickup();
	virtual void Dropoff(float x, float y, float z, float rx, float speed);
	virtual int Shot();
	virtual int StartReload();
	virtual int RunReload();
	virtual bool ResetWeaponParam(class ResourceManager *Resource, int id_param, int lnbs, int nbs);
	virtual int ProcessObject(class Collision *CollD);
	virtual void Render(class D3DGraphics *d3dg, bool NoModel);
};

//! @brief Small parts management class
class smallobject : public object
{
protected:
	class MIFInterface *MIFdata;		//!< Pointer to a class that manages MIF
	float rotation_y;			//!< Rotation Angle
	signed short int point_p4;	//!< Identification number of points
	int hp;						//!< Health.
	float jump_rx;				//!< Flying horizontal angle
	float move_rx;				//!< Moving amount of horizontal axis
	float add_rx;				//!< Rotation of the horizontal axis
	float add_ry;				//!< Vertical axis rotation speed
	int jump_cnt;				//!< Flying Count

public:
	smallobject(class ParameterInfo *in_Param = NULL, class MIFInterface *in_MIFdata = NULL, float x = 0.0f, float y = 0.0f, float z = 0.0f, float rx = 0.0f, int id_param = 0, signed short int p4 = 0, bool flag = false);
	~smallobject();
	virtual void SetMIFInterfaceClass(class MIFInterface *in_MIFdata);
	virtual void SetParamData(int id_param, signed short int p4, bool init);
	virtual void GetParamData(int *id_param, signed short int *p4);
	virtual int GetHP();
	virtual float CollisionMap(class Collision *CollD);
	virtual void HitBullet(int attacks);
	virtual void HitGrenadeExplosion(int attacks);
	virtual void Destruction();
	virtual int ProcessObject();
	virtual void Render(D3DGraphics *d3dg, bool NoModel);
};

//! @brief Management Class
class bullet : public object
{
protected:
	float rotation_y;	//!< Rotation Angle
	int attacks;		//!< Attack Force
	int penetration;	//!< Penetration strength.
	float speed;		//!< Bullet speed.
	int teamid;			//!< Team number
	int humanid;		//!< Data number of people
	float ontargetcnt;	//!< Number of counts during life
	int cnt;			//!< Counting

public:
	bullet(int modelid = -1, int textureid = -1);
	~bullet();
	virtual void SetPosData(float x, float y, float z, float rx, float ry);
	virtual void SetParamData(int _attacks, int _penetration, float _speed, int _teamid, int _humanid, float _ontargetcnt, bool init);
	virtual void GetPosData(float *x, float *y, float *z, float *rx, float *ry);
	virtual void GetParamData(int *_attacks, int *_penetration, float *_speed, int *_teamid, int *_humanid, float *_ontargetcnt);
	virtual int ProcessObject();
	virtual void Render(class D3DGraphics *d3dg, bool NoModel);
};

//! @brief Hand grenades management class
class grenade : public bullet
{
	float move_x;		//!< X-axis travel
	float move_y;		//!< Y-axis travel
	float move_z;		//!< Y-axis travel

public:
	grenade(int modelid = -1, int textureid = -1);
	~grenade();
	void SetParamData(float speed, int _teamid, int _humanid, float _ontargetcnt, bool init);
	void GetParamData(float *_speed, int *_teamid, int *_humanid, float *_ontargetcnt);
	float GetSpeed();
	int ProcessObject(class Collision *CollD);
	virtual void Render(D3DGraphics *d3dg, bool NoModel);
};

//! @brief Effect Management Class
class effect : public object
{
protected:
	float move_x;		//!< X-axis travel
	float move_y;		//!< Y-axis travel
	float move_z;		//!< Z-axis travel
	float addmove_y;	//!< Amount of Y-axis transfer
	float rotation_y;	//!< Rotation Angle
	float rotation_texture;		//!< Rotation Angle
	float addrotation_texture;	//!< Rotation Angle
	float addmodel_size;		//!< Drawing size
	int brightness;				//!< Brightness.
	int addbrightness;			//!< Brightness Addition
	float alpha;				//!< Transparency
	float add_alpha;			//!< Transparent Addition
	int cnt;			//!< Counting
	int type;			//!< Types

public:
	effect(float x = 0.0f, float y = 0.0f, float z = 0.0f, float size = 1.0f, float rotation = 0.0f, int texture = -1, int count = 0);
	~effect();
	virtual void SetParamData(float in_move_x, float in_move_y, float in_move_z, float in_addmove_y, float rotation, float addrotation, float size, float addsize, int count, int settype);
	virtual void SetTexture(int id, int in_brightness, int in_addbrightness, float in_alpha, float in_addalpha);
	virtual void SetRxRy(float rx, float ry);
	virtual int GetTextureID();
	virtual void GetMove(float *mx, float *my, float *mz);
	virtual bool GetCollideMapFlag();
	virtual int ProcessObject();
	virtual void Render(class D3DGraphics *d3dg, float camera_rx, float camera_ry, bool NoModel);
};

//! @brief Human motion control class
//! @details It is a class that controls human motion (animation). It is used in the human class.
//! @attention In this class, there is no motion that can be defeated when a person died.
class HumanMotionControl
{
	class ParameterInfo *Param;				//!< Pointer to a class that manages the setting value
	int id_upmodel;							//!< Upper body.
	int id_armmodel[TOTAL_ARMMODE];			//!< Arms
	int id_legmodel;						//!< Foot (stationary)
	int id_walkmodel[TOTAL_WALKMODE];		//!< foot (walking)
	int id_runmodel[TOTAL_RUNMODE];			//!< Foot (Run)
	float reaction_y;				//!< Relative angle to raise and lower arms
	bool slowarm;					//!< Slow down arm lifting
	float armmodel_rotation_y;		//!< Arm Rotation Angle
	float legrotation_x;			//!< Foot Rotation Angle
	int walkcnt;					//!< Walking motion counting
	int runcnt;						//!< Running motion count
	int armmodelid;					//!< Model of arm to draw
	int legmodelid;					//!< Model of the legs to draw

public:
	HumanMotionControl(class ParameterInfo *in_Param = NULL);
	~HumanMotionControl();
	void SetParameterInfoClass(class ParameterInfo *in_Param);
	void SetModel(int upmodel, int armmodel[], int legmodel, int walkmodel[], int runmodel[]);
	void Init(float rx);
	void PickupWeapon(int weapon_paramid);
	void ChangeHaveWeapon(int weapon_paramid);
	void ChangeWeaponID(int weapon_paramid);
	void ShotWeapon(int weapon_paramid);
	void ReloadWeapon(int weapon_paramid);
	void DumpWeapon();
	void Jump();
	void ProcessObject(float rotation_x, float armrotation_y, int weapon_paramid, int ReloadCnt, int ChangeWeaponIDCnt, int MoveFlag, int hp, bool PlayerFlag);
	void GetRenderMotion(float *arm_rotation_y, float *leg_rotation_x, int *upmodel, int *armmodel, int *legmodel);
	int GetMoveMotionCnt();
};

//! Constants that show the status of a person's foot
enum Human_LegState
{
	LEG_STOP = 0,
	LEG_WALK,
	LEG_RUN
};

//! Flag representing a person's movement operation
enum Human_MoveFlag
{
	MOVEFLAG_FORWARD = 0x01,
	MOVEFLAG_BACK = 0x02,
	MOVEFLAG_LEFT = 0x04,
	MOVEFLAG_RIGHT = 0x08,
	MOVEFLAG_WALK = 0x10,
	MOVEFLAG_JUMP = 0x20,
};

//! Constants representing the type of effect
enum Effect_Type
{
	EFFECT_NORMAL = 0x00,			//!< Normal
	EFFECT_NOBILLBOARD = 0x01,		//!< Don't make a billboard
	EFFECT_COLLIDEMAP = 0x02		//!< Determination of the above
};

#endif