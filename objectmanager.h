//! @file objectmanager.h
//! @brief ObjectManager class declaration

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

#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#define MAX_HUMAN 96			//!< Maximum number of people [Important]: Make '24 multiples' in relation to AI level adjustment!
#define MAX_WEAPON 200			//!< Maximum number of weapons
#define MAX_SMALLOBJECT 40		//!< Maximum number of accessories

#define MAX_BULLET 128			//!< Maximum number of bullets
#define MAX_GRENADE 32			//!< Maximum number of grenades
#define MAX_EFFECT 256			//!< Maximum number of effects

#define WEAPONSHOT_HEIGHT (VIEW_HEIGHT)		//!< Height to fire bullets

#define TOTAL_WEAPON_AUTOBULLET 3			//!< The number of bullets that are automatically supplemented at initialization (how many times the number of bullets)

#define HUMAN_PICKUPWEAPON_L -2.0f			//!< Height to pick up weapons (bottom)
#define HUMAN_PICKUPWEAPON_H 16.0f			//!< Height to pick up weapons (top)
#define HUMAN_PICKUPWEAPON_R 3.8f			//!< Distance to pick up weapons

#define HUMAN_HUMANCOLLISION_R 2.5f			//!< Radius for character-character collision detection.

#define HUMAN_BULLETCOLLISION_HEAD_H 2.9f	//!< Height of the head collision volume.
#define HUMAN_BULLETCOLLISION_HEAD_R 1.5f	//!< Radius of the head collision volume.
#define HUMAN_BULLETCOLLISION_UP_H 7.0f		//!< Height of the upper-body collision volume.
#define HUMAN_BULLETCOLLISION_UP_R 2.4f		//!< Radius of the upper-body collision volume.
#define HUMAN_BULLETCOLLISION_LEG_H 10.5f	//!< Height of the leg collision volume.
#define HUMAN_BULLETCOLLISION_LEG_R 2.6f	//!< Radius of the leg collision volume.

#define SMALLOBJECT_COLLISIONSCALE 0.13f	//!< Small-object collision volume scale.

#define MAX_OBJECTMANAGER_LOGLEN 3						//!< Maximum number of lines of object log
#define MAX_OBJECTMANAGER_LOGLINES 64					//!< Maximum number of characters in object log
#define MAX_OBJECTMANAGER_LOGCNT (int)(5.0f*GAMEFPS)	//!< Number of object log display counts

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 3		//!< Select include file.
#endif
#include "main.h"

//! Structure for effect drawing calculation
typedef struct
{
	int id;		//!< Data Number
	float dist;	//!< Distance.
} effectdata;

//! @brief Object Management Class
//! @details Initialization, calculation, and drawing of each object are managed.
class ObjectManager
{
	class human *HumanIndex;					//!< Human Objects
	class weapon *WeaponIndex;					//!< Weapon object
	class smallobject *SmallObjectIndex;		//!< Small objects
	class bullet *BulletIndex;					//!< Bullet object
	class grenade *GrenadeIndex;				//!< Hand grenades object
	class effect *EffectIndex;					//!< Effect object

	unsigned int framecnt;		//!< Counting Frame Number

	float *Human_ontarget;	//!< Number of hits.
	int *Human_kill;		//!< Number of defeated enemies
	int *Human_headshot;	//!< The number of lives in the enemy head
	bool *Human_ShotFlag;	//!< Firing flag (for muzzle flash).
	int *Human_YakkyouCnt;	//!< Shell-casing counter after firing.
	class BulletObjectHumanIndex *BulletObj_HumanIndex;		//!< Object used for bullet-character collision detection.

	bool AddCollisionFlag;	//!< Additional collision detection.
	bool *FriendlyFire;		//!< Enable friendly fire (FF).

	int Player_HumanID;		//!< ID of a person object that the player operates

	bool *Human_FrameTextureRefresh;	//!< Human Texture Refresh Flag (for Texture Counterfeiter (Mystery))

	ParameterInfo *GameParamInfo;		//!< Game settings
	D3DGraphics *d3dg;					//!< Drawing class
	ResourceManager *Resource;			//!< Resource Management
	BlockDataInterface *BlockData;		//!< Block Data Management Class
	PointDataInterface *PointData;		//!< Point Data Management Class
	Collision *CollD;					//!< Percentage Management Class
	SoundManager *GameSound;			//!< Game effect sound management class
	MIFInterface *MIFdata;				//!< MIF Control Class

	class ObjectManagerLog *ObjectLog;	//!< Object Log Class

	int AddHumanIndex(pointdata data, pointdata infodata, bool RandomFlag);
	int AddWeaponIndex(pointdata data);
	int AddSmallObjectIndex(pointdata data);
	void SetHumanBlood(float x, float y, float z, int damage, bool flowing);
	bool CollideHuman(human *in_humanA, human *in_humanB);
	bool CollideBullet(bullet *in_bullet);
	void HitBulletMap(float x, float y, float z, int teamID);
	void HitBulletHuman(int HitHuman_id, int Hit_id, float x, float y, float z, float brx, int attacks, int humanid, float ontargetcnt);
	void HitBulletSmallObject(int HitSmallObject_id, float x, float y, float z, int attacks, int teamID);
	bool GrenadeExplosion(grenade *in_grenade);
	void DeadEffect(human *in_human);
	bool CollideBlood(effect *in_effect, int *id, int *face, float *pos_x, float *pos_y, float *pos_z);
	void PickupWeapon(human *in_human, weapon *in_weapon);
	void CleanupPointDataToObject();
	int SortEffect(float camera_x, float camera_y, float camera_z, effectdata data[]);
	void ShotWeaponEffect(int humanid);
	void ShotWeaponYakkyou(int humanid);

public:
    void CaptureRenderState();

	ObjectManager();
	~ObjectManager();
	void SetClass(ParameterInfo *in_GameParamInfo, D3DGraphics *in_d3dg, ResourceManager *in_Resource, BlockDataInterface *in_BlockData, PointDataInterface *in_PointData, Collision *in_CollD, SoundManager *in_GameSound, MIFInterface *in_MIFdata);
	int AddHumanIndex(float px, float py, float pz, float rx, int CharacterID, int TeamID, int WeaponID[], signed short int param4, bool RandomFlag);
	int AddVisualWeaponIndex(int WeaponID, bool loadbullet);
	int AddSmallObjectIndex(float px, float py, float pz, float rx, int paramID, bool MapColl);
	int AddEffect(float pos_x, float pos_y, float pos_z, float move_x, float move_y, float move_z, float addmove_y, float rotation, float addrotation, float size, float addsize, int texture, int brightness, int addbrightness, float alpha, float addalpha, int settype, int count);
	int AddMapEffect(int id, int face, float pos_x, float pos_y, float pos_z, float size, float rotation, int texture, int brightness, int addbrightness, float alpha, float addalpha, int count);
	void LoadPointData();
	void Recovery();
	void SetAddCollisionFlag(bool flag);
	bool GetFriendlyFireFlag(int id);
	void SetFriendlyFireFlag(int id, bool flag);
	int GetPlayerID();
	void SetPlayerID(int id);
	human* GetHumanObject(int id);
	human* GetPlayerHumanObject();
	int GetHumanObjectID(human* object);
	weapon* GetWeaponObject(int id);
	smallobject* GetSmallObject(int id);
	bullet* GetBulletObject(int id);
	int GetBulletObjectID(bullet* object);
	bullet* GetNewBulletObject();
	grenade* GetNewGrenadeObject();
	human* SearchHuman(signed short int p4);
	smallobject* SearchSmallobject(signed short int p4);
	void GetTotalObjects(int *HumanCnt, int *WeaponCnt, int *SmallobjectCnt, int *BulletCnt, int *GrenadeCnt, int *EffectCnt);
	void MoveForward(int human_id);
	void MoveBack(int human_id);
	void MoveLeft(int human_id);
	void MoveRight(int human_id);
	void MoveWalk(int human_id);
	void MoveJump(int human_id);
	int ShotWeapon(int human_id);
	bool ReloadWeapon(int human_id);
	void ChangeHaveWeapon(int human_id, int id = -1);
	int ChangeWeaponID(int human_id);
	bool DumpWeapon(int human_id);
	void ChangeScopeMode(int human_id);
	bool CheatAddBullet(int human_id);
	bool CheatNewWeapon(int human_id, int new_weaponID);
	bool CheckZombieAttack(human* MyHuman, human* EnemyHuman);
	void HitZombieAttack(human* MyHuman, human* EnemyHuman);
	bool HumanResuscitation(int id);
	int CheckGameOverorComplete();
	bool GetObjectInfoTag(float camera_x, float camera_y, float camera_z, float camera_rx, float camera_ry, int *color, char *infostr);
	int Process(int cmdF5id, bool demomode, bool screen);
	bool GetHumanShotInfo(int id, float *ontarget, int *kill, int *headshot);
	void Render(float camera_x, float camera_y, float camera_z, float camera_rx, float camera_ry, int HidePlayer, bool NoModel);
	void RenderLog(int x, int y);
	void Cleanup();
};

//! @brief Class for bullet-character collision detection.
//! @details This is a class that manages whether the bullet is judged or not. It is used in the ObjectManager class.
class BulletObjectHumanIndex
{
	bool *HumanIndex;		//!< List.

public:
	BulletObjectHumanIndex();
	~BulletObjectHumanIndex();
	void Init();
	bool GetIndexFlag(int id);
	void SetIndexFlag(int id);
};

//! @brief Object Log Class
//! @details Object management class Log and display of ObjectManager.
class ObjectManagerLog
{
	D3DGraphics *d3dg;							//!< Drawing class

	int TextCnt[MAX_OBJECTMANAGER_LOGLEN];		//!< Number of frames
	char *TextStr[MAX_OBJECTMANAGER_LOGLEN];	//!< String.
	int TextColor[MAX_OBJECTMANAGER_LOGLEN];	//!< String Color

	bool AddTextLog(int cnt, const char *str, int color);

public:
	ObjectManagerLog();
	~ObjectManagerLog();
	void SetClass(D3DGraphics *in_d3dg);
	void ClearLog();
	void InfoLog(const char *str);
	void AddHuman(int humanID, int TeamID, int PlayerTeamID);
	void DiedHuman(int ShothumanID, int HitHumanID, int ShothumanTeamID, int HitHumanTeamID, int PlayerTeamID);
	void ReviveHuman(int humanID, int TeamID, int PlayerTeamID);
	void AddSmallObject(int objID);
	void BreakSmallObject(int objID);
	void Process();
	void Render(int x, int y);
};

#endif