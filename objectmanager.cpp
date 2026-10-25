//! @file objectmanager.cpp
//! @brief Definition of ObjectManager class

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

#include "weapon-attachment.h"
#include "objectmanager.h"

//! @brief Constructor
ObjectManager::ObjectManager()
{
	HumanIndex = new human[MAX_HUMAN];
	WeaponIndex = new weapon[MAX_WEAPON];
	SmallObjectIndex = new smallobject[MAX_SMALLOBJECT];
	BulletIndex = new bullet[MAX_BULLET];
	GrenadeIndex = new grenade[MAX_GRENADE];
	EffectIndex = new effect[MAX_EFFECT];

	framecnt = 0;
	Human_ontarget = new float[MAX_HUMAN];
	Human_kill = new int[MAX_HUMAN];
	Human_headshot = new int[MAX_HUMAN];
	Human_ShotFlag = new bool[MAX_HUMAN];
	Human_YakkyouCnt = new int[MAX_HUMAN];
	BulletObj_HumanIndex = new BulletObjectHumanIndex[MAX_BULLET];
	AddCollisionFlag = false;
	FriendlyFire = new bool[MAX_HUMAN];
	Player_HumanID = 0;
	Human_FrameTextureRefresh = new bool[MAX_HUMAN];

	GameParamInfo = NULL;
	d3dg = NULL;
	Resource = NULL;
	BlockData = NULL;
	PointData = NULL;
	CollD = NULL;
	GameSound = NULL;
	MIFdata = NULL;

	ObjectLog = new ObjectManagerLog;
}

//! @brief Destructor
ObjectManager::~ObjectManager()
{
	Cleanup();

	if( HumanIndex != NULL ){ delete [] HumanIndex; }
	if( WeaponIndex != NULL ){ delete [] WeaponIndex; }
	if( SmallObjectIndex != NULL ){ delete [] SmallObjectIndex; }
	if( BulletIndex != NULL ){ delete [] BulletIndex; }
	if( GrenadeIndex != NULL ){ delete [] GrenadeIndex; }
	if( EffectIndex != NULL ){ delete [] EffectIndex; }

	if( Human_ontarget != NULL ){ delete [] Human_ontarget; }
	if( Human_kill != NULL ){ delete [] Human_kill; }
	if( Human_headshot != NULL ){ delete [] Human_headshot; }
	if( Human_ShotFlag != NULL ){ delete [] Human_ShotFlag; }
	if( Human_YakkyouCnt != NULL ){ delete [] Human_YakkyouCnt; }
	if( BulletObj_HumanIndex != NULL ){ delete [] BulletObj_HumanIndex; }
	if( FriendlyFire != NULL ){ delete [] FriendlyFire; }
	if( Human_FrameTextureRefresh != NULL ){ delete [] Human_FrameTextureRefresh; }

	if( ObjectLog != NULL ){ delete ObjectLog; }
}

//! @brief Set the class you want to see
//! @param in_GameParamInfo Game Settings Data Management Class
//! @param in_d3dg Drawing processing class
//! @param in_Resource Resource Management Class
//! @param in_BlockData Block Data Management Class
//! @param in_PointData Point Data Management Class
//! @param in_CollD Percentage processing class
//! @param in_GameSound Sound Playing Class
//! @param in_MIFdata MIF Control Class
//! @attention If you do not set this function, the class itself will not function correctly.
void ObjectManager::SetClass(ParameterInfo *in_GameParamInfo, D3DGraphics *in_d3dg, ResourceManager *in_Resource, BlockDataInterface *in_BlockData, PointDataInterface *in_PointData, Collision *in_CollD, SoundManager *in_GameSound, MIFInterface *in_MIFdata)
{
	GameParamInfo = in_GameParamInfo;
	d3dg = in_d3dg;
	Resource = in_Resource;
	BlockData = in_BlockData;
	PointData = in_PointData;
	CollD = in_CollD;
	GameSound = in_GameSound;
	MIFdata = in_MIFdata;

	for(int i=0; i<MAX_HUMAN; i++){
		HumanIndex[i].SetParameterInfoClass(GameParamInfo);
	}
	for(int i=0; i<MAX_WEAPON; i++){
		WeaponIndex[i].SetParameterInfoClass(GameParamInfo);
	}
	for(int i=0; i<MAX_SMALLOBJECT; i++){
		SmallObjectIndex[i].SetParameterInfoClass(GameParamInfo);
		SmallObjectIndex[i].SetMIFInterfaceClass(MIFdata);
	}
	for(int i=0; i<MAX_GRENADE; i++){
		GrenadeIndex[i].SetParameterInfoClass(GameParamInfo);
	}

	int bulletmodel, bullettexture;
	BulletParameter data;

	if( Resource->GetBulletModelTexture(0, &bulletmodel, &bullettexture) == 0 ){
		if( GameParamInfo->GetBullet(0, &data) == 0 ){
			for(int i=0; i<MAX_BULLET; i++){
				BulletIndex[i].SetModel(bulletmodel, data.size);
				BulletIndex[i].SetTexture(bullettexture);
			}
		}
	}

	if( Resource->GetBulletModelTexture(1, &bulletmodel, &bullettexture) == 0 ){
		if( GameParamInfo->GetBullet(1, &data) == 0 ){
			for(int i=0; i<MAX_GRENADE; i++){
				GrenadeIndex[i].SetModel(bulletmodel, data.size);
				GrenadeIndex[i].SetTexture(bullettexture);
			}
		}
	}

	ObjectLog->SetClass(d3dg);
}

//! @brief Add People
//! @param data Pointdata structure
//! @param infodata Point data of referral information ()
//! @param RandomFlag Random correction method
//! @return Succeed: Data number (0 or more) Failed:-1
//! @attention If the type number of invalid person is specified, it is called "mysterious man". The texture is used for the map texture No. 0, and the HP is specified for 0 (= immortal).
int ObjectManager::AddHumanIndex(pointdata data, pointdata infodata, bool RandomFlag)
{
	int GetHumanFlag;
	HumanParameter HumanParam;
	GetHumanFlag = GameParamInfo->GetHuman(infodata.p2, &HumanParam);
	int Humanindexid = -1;
	int Weaponindexid = -1;
	class weapon *Weapon[TOTAL_HAVEWEAPON];
	int lnbslist[TOTAL_HAVEWEAPON];
	int nbslist[TOTAL_HAVEWEAPON];
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		Weapon[i] = NULL;
		lnbslist[i] = 0;
		nbslist[i] = 0;
	}

#ifndef ENABLE_BUG_HUMANWEAPON
	if( GetHumanFlag != 0 ){
		return -1;
	}
#endif

	//Get the model number of a person
	int upmodel[TOTAL_UPMODE];
	int armmodel[TOTAL_ARMMODE];
	int legmodel;
	int walkmodel[TOTAL_WALKMODE];
	int runmodel[TOTAL_RUNMODE];
	Resource->GetHumanModel(upmodel, armmodel, &legmodel, walkmodel, runmodel);

	float px = data.x;
	float py = data.y;
	float pz = data.z;
	float rx = data.r;
	if( RandomFlag == true ){
		px += 0.001f*GetRand(100*2) - 0.1f;
		pz += 0.001f*GetRand(100*2) - 0.1f;
		rx += DegreeToRadian(0.01f)*GetRand(80*2) - DegreeToRadian(0.8f);
	}

	for(int j=0; j<MAX_HUMAN; j++){
		if( HumanIndex[j].GetEnableFlag() == false ){
			//Initialize
			HumanIndex[j].SetPosData(px, py, pz, rx);
			HumanIndex[j].SetParamData(infodata.p2, data.id, data.p4, infodata.p3, true);
			if( GetHumanFlag == 0 ){
				int id = Resource->GetHumanTexture(infodata.p2);
				if( id == -1 ){
					//If the texture is not available, enable the refresh flag to get each frame
					Human_FrameTextureRefresh[j] = true;
				}

				HumanIndex[j].SetTexture(id);
				HumanIndex[j].SetModel(upmodel[ HumanParam.model ], armmodel, legmodel, walkmodel, runmodel);
			}
			else{
#ifdef ENABLE_BUG_HUMANWEAPON
				HumanIndex[j].SetTexture(d3dg->GetMapTextureID(0));
				HumanIndex[j].SetModel(upmodel[0], armmodel, legmodel, walkmodel, runmodel);
#endif
			}
			HumanIndex[j].SetEnableFlag(true);
			Humanindexid = j;
			break;
		}
	}

	//If it is not possible to initialize, it will be returned as an error
	if( Humanindexid == -1 ){
		return -1;
	}

	//If the data of the person has been read correctly ~
	if( GetHumanFlag == 0 ){
		//Add weapon B as a virtual weapon
		Weaponindexid = AddVisualWeaponIndex(HumanParam.Weapon[0], true);
		if( Weaponindexid != -1 ){
			//Record to array if successful
			Weapon[0] = &WeaponIndex[Weaponindexid];
		}
		if( data.p1 == POINT_P1TYPE_HUMAN ){
			//Added as a virtual weapon of weapon A
			Weaponindexid = AddVisualWeaponIndex(HumanParam.Weapon[1], true);
			if( Weaponindexid != -1 ){
				//Record to array if successful
				Weapon[1] = &WeaponIndex[Weaponindexid];
			}
		}

		//Give to the character.
		HumanIndex[Humanindexid].SetWeapon(Weapon, lnbslist, nbslist);
	}

	//Initialize gun flags
	Human_ShotFlag[Humanindexid] = false;
	Human_YakkyouCnt[Humanindexid] = -1;

	//Register a number if you are a player
	if( ( (data.p1 == POINT_P1TYPE_HUMAN)||(data.p1 == POINT_P1TYPE_HUMAN2) )&&(data.p4 == 0) ){
		Player_HumanID = Humanindexid;
	}

	return Humanindexid;
}

//! @brief Add people (for game)
//! @param px X Coordinate
//! @param py Y Coordinate
//! @param pz Z Coordinates
//! @param rx X-axis orientation
//! @param paramID Type number
//! @param TeamID Team number
//! @param WeaponID Array of weapon type IDs (number of elements: TOTAL_HAVEWEAPON).
//! @param param4 4 Parameters
//! @param RandomFlag Random correction method
//! @return Succeed: Data number (0 or more) Failed:-1
//! @attention If the type number of invalid person is specified, it is called "mysterious man". The texture is used for the map texture No. 0, and the HP is specified for 0 (= immortal).
int ObjectManager::AddHumanIndex(float px, float py, float pz, float rx, int paramID, int TeamID, int WeaponID[], signed short int param4, bool RandomFlag)
{
	int GetHumanFlag;
	HumanParameter HumanParam;
	GetHumanFlag = GameParamInfo->GetHuman(paramID, &HumanParam);
	int Humanindexid = -1;
	int Weaponindexid = -1;
	class weapon *Weapon[TOTAL_HAVEWEAPON];
	int lnbslist[TOTAL_HAVEWEAPON];
	int nbslist[TOTAL_HAVEWEAPON];
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		Weapon[i] = NULL;
		lnbslist[i] = 0;
		nbslist[i] = 0;
	}

#ifndef ENABLE_BUG_HUMANWEAPON
	if( GetHumanFlag != 0 ){
		return -1;
	}
#endif

	//Get the model number of a person
	int upmodel[TOTAL_UPMODE];
	int armmodel[TOTAL_ARMMODE];
	int legmodel;
	int walkmodel[TOTAL_WALKMODE];
	int runmodel[TOTAL_RUNMODE];
	Resource->GetHumanModel(upmodel, armmodel, &legmodel, walkmodel, runmodel);

	if( RandomFlag == true ){
		px += 0.001f*GetRand(100*2) - 0.1f;
		pz += 0.001f*GetRand(100*2) - 0.1f;
		rx += DegreeToRadian(0.01f)*GetRand(80*2) - DegreeToRadian(0.8f);
	}

	for(int i=0; i<MAX_HUMAN; i++){
		if( HumanIndex[i].GetEnableFlag() == false ){
			//Initialize
			HumanIndex[i].SetPosData(px, py, pz, rx);
			HumanIndex[i].SetParamData(paramID, -1, param4, TeamID, true);
			if( GetHumanFlag == 0 ){
				int id = Resource->GetHumanTexture(paramID);
				if( id == -1 ){
					//If the texture is not available, enable the refresh flag to get each frame
					Human_FrameTextureRefresh[i] = true;
				}

				HumanIndex[i].SetTexture(id);
				HumanIndex[i].SetModel(upmodel[ HumanParam.model ], armmodel, legmodel, walkmodel, runmodel);
			}
			else{
#ifdef ENABLE_BUG_HUMANWEAPON
				HumanIndex[i].SetTexture(d3dg->GetMapTextureID(0));
				HumanIndex[i].SetModel(upmodel[0], armmodel, legmodel, walkmodel, runmodel);
#endif
			}
			HumanIndex[i].SetEnableFlag(true);
			Humanindexid = i;
			break;
		}
	}

	//If it is not possible to initialize, it will be returned as an error
	if( Humanindexid == -1 ){
		return -1;
	}

	if( GetHumanFlag == 0 ){
		//Add virtual weapons
		for(int i=0; i<TOTAL_HAVEWEAPON; i++){
			Weaponindexid = AddVisualWeaponIndex(WeaponID[i], true);
			if( Weaponindexid != -1 ){
				//Record to array if successful
				Weapon[i] = &WeaponIndex[Weaponindexid];
			}
		}

		//Give to the character.
		HumanIndex[Humanindexid].SetWeapon(Weapon, lnbslist, nbslist);
	}

	//Log-related processing
	int player_teamid;
	HumanIndex[Player_HumanID].GetParamData(NULL, NULL, NULL, &player_teamid);
	ObjectLog->AddHuman(Humanindexid, TeamID, player_teamid);

	return Humanindexid;
}

//! @brief Add weapons
//! @param data Weapon point data (pointdata structure)
//! @return Succeed: Data number (0 or more) Failed:-1
int ObjectManager::AddWeaponIndex(pointdata data)
{
	//Temporarily register the weapon ID and ammunition count.
	int WeaponID = data.p2;
	int nbs = PointParamToInt(data.p3);

	//Random weapons
	if( data.p1 == POINT_P1TYPE_RAND_WEAPON ){
		WeaponParameter WeaponParam;

		if( GetRand(2) == 0 ){
			if( GameParamInfo->GetWeapon(data.p2, &WeaponParam) == 1 ){ return -1; }
			nbs = (int)WeaponParam.nbsmax * TOTAL_WEAPON_AUTOBULLET;		//Overwrite the ammunition count.
		}
		else{
			if( GameParamInfo->GetWeapon(data.p3, &WeaponParam) == 1 ){ return -1; }
			WeaponID = data.p3;						//Weapon Number Overwrite
			nbs = (int)WeaponParam.nbsmax * TOTAL_WEAPON_AUTOBULLET;		//Overwrite the ammunition count.
		}
	}

	//NONE fails
	if( WeaponID == ID_WEAPON_NONE ){ return -1; }

	//Get a model and texture
	int model, texture;
	if( Resource->GetWeaponModelTexture(WeaponID, &model, &texture) == 1 ){
		return -1;
	}

	//Getting the setting value
	WeaponParameter WeaponParam;
	if( GameParamInfo->GetWeapon(WeaponID, &WeaponParam) == 1 ){ return -1; }

	for(int i=0; i<MAX_WEAPON; i++){
		if( WeaponIndex[i].GetEnableFlag() == false ){
			//Initialize
			WeaponIndex[i].SetPosData(data.x, data.y, data.z, data.r);
			WeaponIndex[i].SetParamData(WeaponID, 0, nbs, true);
			WeaponIndex[i].SetModel(model, WeaponParam.size);
			WeaponIndex[i].SetTexture(texture);
			WeaponIndex[i].SetEnableFlag(true);
			WeaponIndex[i].RunReload();
			return i;
		}
	}
	return -1;
}

//! @brief Add virtual weapons
//! @param WeaponID Type number of weapons
//! @param loadbullet Load bullets
//! @return Succeed: Data number (0 or more) Failed:-1
//! @attention This is a function to add weapons that a person has from the beginning as weapon data (object).
//! @attention Use the AddWeaponIndex() function to place it as a weapon unit.
int ObjectManager::AddVisualWeaponIndex(int WeaponID, bool loadbullet)
{
	//NONE fails
	if( WeaponID == ID_WEAPON_NONE ){ return -1; }

	//Get information
	WeaponParameter WeaponParam;
	if( GameParamInfo->GetWeapon(WeaponID, &WeaponParam) == 1 ){ return -1; }

	//Get a model and texture
	int model, texture;
	if( Resource->GetWeaponModelTexture(WeaponID, &model, &texture) == 1 ){
		return -1;
	}

	for(int i=0; i<MAX_WEAPON; i++){
		if( WeaponIndex[i].GetEnableFlag() == false ){
			//Initialize
			WeaponIndex[i].SetPosData(0.0f, 0.0f, 0.0f, 0.0f);
			if( loadbullet == false ){
				WeaponIndex[i].SetParamData(WeaponID, 0, 0, true);
			}
			else{
				WeaponIndex[i].SetParamData(WeaponID, WeaponParam.nbsmax, WeaponParam.nbsmax * TOTAL_WEAPON_AUTOBULLET, true);
			}
			WeaponIndex[i].SetModel(model, WeaponParam.size);
			WeaponIndex[i].SetTexture(texture);
			WeaponIndex[i].SetEnableFlag(true);
			return i;
		}
	}
	return -1;
}

//! @brief Add small items
//! @param data Point data for small items (pointdata structure)
//! @return Succeed: Data number (0 or more) Failed:-1
int ObjectManager::AddSmallObjectIndex(pointdata data)
{
	//Get a model and texture
	int model, texture;
	if( Resource->GetSmallObjectModelTexture(data.p2, &model, &texture) == 1 ){
		return -1;
	}

	for(int i=0; i<MAX_SMALLOBJECT; i++){
		if( SmallObjectIndex[i].GetEnableFlag() == false ){
			//Initialize
			SmallObjectIndex[i].SetPosData(data.x, data.y, data.z, data.r);
			SmallObjectIndex[i].SetParamData(data.p2, data.p4, true);
			SmallObjectIndex[i].SetModel(model, SMALLOBJECT_SCALE);
			SmallObjectIndex[i].SetTexture(texture);
			SmallObjectIndex[i].SetEnableFlag(true);

			//Map and judgment if the location correction flag is enabled
			if( data.p3 !=0 ){
				SmallObjectIndex[i].CollisionMap(CollD);
			}
			return i;
		}
	}
	return -1;
}

//! @brief Add small items (for game)
//! @param px X Coordinate
//! @param py Y Coordinate
//! @param pz Z Coordinates
//! @param rx X-axis orientation
//! @param paramID Type number
//! @param MapColl Position correction flag
//! @return Succeed: Data number (0 or more) Failed:-1
int ObjectManager::AddSmallObjectIndex(float px, float py, float pz, float rx, int paramID, bool MapColl)
{
	//Get a model and texture
	int model, texture;
	if( Resource->GetSmallObjectModelTexture(paramID, &model, &texture) == 1 ){
		return -1;
	}

	for(int i=0; i<MAX_SMALLOBJECT; i++){
		if( SmallObjectIndex[i].GetEnableFlag() == false ){
			//Initialize
			SmallObjectIndex[i].SetPosData(px, py, pz, rx);
			SmallObjectIndex[i].SetParamData(paramID, 0, true);
			SmallObjectIndex[i].SetModel(model, SMALLOBJECT_SCALE);
			SmallObjectIndex[i].SetTexture(texture);
			SmallObjectIndex[i].SetEnableFlag(true);

			//Map and judgment if the location correction flag is enabled
			if( MapColl == true ){
				SmallObjectIndex[i].CollisionMap(CollD);
			}

			//Log-related processing
			ObjectLog->AddSmallObject(i);

			return i;
		}
	}
	return -1;
}

//! @brief Add Effects
//! @param pos_x X Coordinate
//! @param pos_y Y Coordinate
//! @param pos_z Z Coordinates
//! @param move_x X-axis travel
//! @param move_y Y-axis travel
//! @param move_z Z-axis travel
//! @param addmove_y Amount of Y-axis transfer
//! @param rotation Rotation Angle
//! @param addrotation Rotation Angle
//! @param size Drawing Magnification
//! @param addsize Drawing Magnification
//! @param texture Texture Data Number
//! @param brightness Brightness.
//! @param addbrightness Brightness Addition
//! @param alpha Transparency
//! @param addalpha Transparent Addition
//! @param settype Effect type (combine Effect_Type flags).
//! @param count Number of drawing frames
//! @return Succeed: Data number (0 or more) Failed:-1
int ObjectManager::AddEffect(float pos_x, float pos_y, float pos_z, float move_x, float move_y, float move_z, float addmove_y, float rotation, float addrotation, float size, float addsize, int texture, int brightness, int addbrightness, float alpha, float addalpha, int settype, int count)
{
	for(int i=0; i<MAX_EFFECT; i++){
		if( EffectIndex[i].GetEnableFlag() == false ){
			EffectIndex[i].SetPosData(pos_x, pos_y, pos_z, 0.0f);
			EffectIndex[i].SetParamData(move_x, move_y, move_z, addmove_y, rotation, addrotation, size, addsize, count, settype);
			EffectIndex[i].SetTexture(texture, brightness, addbrightness, alpha, addalpha);
			EffectIndex[i].SetEnableFlag(true);
			return i;
		}
	}

	return -1;
}

//! @brief Add effects (not billboards)
//! @param id Block number
//! @param face Face Number
//! @param pos_x X Coordinate
//! @param pos_y Y Coordinate
//! @param pos_z Z Coordinates
//! @param size Drawing Magnification
//! @param rotation Rotation Angle
//! @param texture Texture Data Number
//! @param brightness Brightness.
//! @param addbrightness Brightness Addition
//! @param alpha Transparency
//! @param addalpha Transparent Addition
//! @param count Number of drawing frames
//! @return Succeed: Data number (0 or more) Failed:-1
int ObjectManager::AddMapEffect(int id, int face, float pos_x, float pos_y, float pos_z, float size, float rotation, int texture, int brightness, int addbrightness, float alpha, float addalpha, int count)
{
	blockdata data;
	float vx, vy, vz;
	float rx, ry;

	//Get information on block surfaces
	BlockData->Getdata(&data, id);
	vx = data.material[face].vx;
	vy = data.material[face].vy;
	vz = data.material[face].vz;

	//Seeking Angle
	rx = atan2f(vz, vx)*-1 + (float)M_PI;
	ry = atan2f(vy, sqrtf(vx*vx + vz*vz))*-1;

	//Create effects
	for(int i=0; i<MAX_EFFECT; i++){
		if( EffectIndex[i].GetEnableFlag() == false ){
			EffectIndex[i].SetPosData(pos_x, pos_y, pos_z, 0.0f);
			EffectIndex[i].SetParamData(0.0f, 0.0f, 0.0f, 0.0f, rotation, 0.0f, size, 0.0f, count, EFFECT_NOBILLBOARD);
			EffectIndex[i].SetRxRy(rx, ry);
			EffectIndex[i].SetTexture(texture, brightness, addbrightness, alpha, addalpha);
			EffectIndex[i].SetEnableFlag(true);
			return i;
		}
	}

	return -1;
}

//! @brief bleeding
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param damage Damage
//! @param flowing With or without blood flow
void ObjectManager::SetHumanBlood(float x, float y, float z, int damage, bool flowing)
{
	int addtype;

	if( flowing == false ){
		addtype = EFFECT_NORMAL;
	}
	else{
		addtype = EFFECT_COLLIDEMAP;
	}

	if( GameConfig.GetBloodFlag() == true ){
		AddEffect(x, y, z, 0.0f, 0.0f, 0.0f, 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, 10.0f, 0.2f, Resource->GetEffectBloodTexture(), 10, 0, 0.5f, -0.04f, addtype, (int)(GAMEFPS * 0.4f));
		if( flowing == true ){
			for(int i=0; i<(damage/10); i++){
				float rx = DegreeToRadian(10)*GetRand(36);
				AddEffect(x + cosf(rx)*1.0f, y + (float)(GetRand(20)-10)/10, z + sinf(rx)*1.0f, cosf(rx)*0.5f, GetRand(5)*0.1f + 0.5f, sinf(rx)*0.5f, -0.09f, DegreeToRadian(10)*GetRand(36), 0.0f, GetRand(40)*0.1f+0.1f, 0.1f, Resource->GetEffectBloodTexture(), 10, 0, 1.0f, 0.0f, addtype, (int)(GAMEFPS * 0.5f));
			}
		}
	}
}

//! @brief Determination per person
//! @param in_humanA Target human object A
//! @param in_humanB Object B
//! @return Colliding: true; not colliding: false.
//! @warning There is no distinction between in_humanA and in_humanB
//! @warning Test each in_humanA/in_humanB pair only once per frame.
//! @attention Both classes automatically push each other using AddPosOrder().
bool ObjectManager::CollideHuman(human *in_humanA, human *in_humanB)
{
	if( in_humanA == NULL ){ return false; }
	if( in_humanB == NULL ){ return false; }

	float h1_x, h1_y, h1_z;
	float h2_x, h2_y, h2_z;
	float angle, length;

	//Do not judge if it is not initialized or die
	if( in_humanA->GetEnableFlag() == false ){ return false; }
	if( in_humanB->GetEnableFlag() == false ){ return false; }
	if( in_humanA->GetHP() <= 0 ){ return false; }
	if( in_humanB->GetHP() <= 0 ){ return false; }

	//Get coordinates of each other
	in_humanA->GetPosData(&h1_x, &h1_y, &h1_z, NULL);
	in_humanB->GetPosData(&h2_x, &h2_y, &h2_z, NULL);

	//Cylinder collision detection.
	if( CollideCylinder(h1_x, h1_y, h1_z, HUMAN_HUMANCOLLISION_R, HUMAN_HEIGHT-0.5f, h2_x, h2_y, h2_z, HUMAN_HUMANCOLLISION_R, HUMAN_HEIGHT-0.5f, &angle, &length) == true ){
		//Push out by the penetration depth.
		in_humanA->AddPosOrder(angle, 0.0f, length/2);
		in_humanB->AddPosOrder(angle + (float)M_PI, 0.0f, length/2);
		return true;
	}

	return false;
}

//! @brief Determination and processing of bullets
//! @param in_bullet Target bullet object
//! @return Hit: true; no hit: false.
//! @attention The target is "map", "person (head, upper body, lower body)", and "small items".
//! @attention We do not only judgement, but consistently perform damage calculations and sound effects playback.
bool ObjectManager::CollideBullet(bullet *in_bullet)
{
	if( in_bullet == NULL ){ return false; }

	//If you are not using it, you can use it without processing.
	if( in_bullet->GetEnableFlag() == false ){ return false; }

	int objectID;
	float bx, by, bz;
	float brx, bry;
	int attacks;
	int penetration;
	float speed;
	int teamid;
	int humanid;
	float ontargetcnt;
	float vx, vy, vz;
	bool CollideFlag = false;
	int mapflag = 0;		//0: no block; 1: block present, untested; 2: block present, tested.

	float Dist;

	//es the data number of bullet objects
	objectID = GetBulletObjectID(in_bullet);

	//coordinates are obtained and the vector is calculated.
	in_bullet->GetPosData(&bx, &by, &bz, &brx, &bry);
	in_bullet->GetParamData(&attacks, &penetration, &speed, &teamid, &humanid, &ontargetcnt);
	vx = cosf(brx)*cosf(bry);
	vy = sinf(bry);
	vz = sinf(brx)*cosf(bry);

	//Check collision with the map (whether a block lies along the bullet trajectory).
	if( CollD->CheckALLBlockIntersectRay(bx, by, bz, vx, vy, vz, NULL, NULL, &Dist, speed) == true ){
		mapflag = 1;
	}

	// BULLET_SPEEDSCALE
	int maxcnt = (int)roundf(speed / BULLET_SPEEDSCALE);

	// BULLET_SPEEDSCALE Constant Resolution
	for(int cnt=0; cnt<maxcnt; cnt++){
		//If the penetration force is not obtained, the bullet will be disabled (Judge 1 )
		if( penetration < 0 ){
			in_bullet->SetEnableFlag(false);
			mapflag = 0;
			break;
		}

		float bvx, bvy, bvz;
		bvx = bx + vx * BULLET_SPEEDSCALE * (cnt+1);
		bvy = by + vy * BULLET_SPEEDSCALE * (cnt+1);
		bvz = bz + vz * BULLET_SPEEDSCALE * (cnt+1);

		//Determination of People
		for(int i=0; i<MAX_HUMAN; i++){
			//If the person himself has a gun, it will not be processed.
			if( i == humanid ){ continue; }

			//not used or treated with carcasses.
			if( HumanIndex[i].GetEnableFlag() == false ){ continue; }
			//if( HumanIndex[i].GetDeadFlag() == true ){ continue; }
			if( HumanIndex[i].GetHP() <= 0 ){ continue; }

			//If you are already present, do not process it.
			if( BulletObj_HumanIndex[objectID].GetIndexFlag(i) == true ){ continue; }

			//Get coordinates
			float ox, oy, oz;
			int h_teamid;
			HumanIndex[i].GetPosData(&ox, &oy, &oz, NULL);
			HumanIndex[i].GetParamData(NULL, NULL, NULL, &h_teamid);

			if( FriendlyFire[i] == false ){
				//If the same team number (ally) is not processed
				if( h_teamid == teamid ){ continue; }
			}

			//Determination per head
			if( CollideCylinderInside(ox, oy + HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H, oz, HUMAN_BULLETCOLLISION_HEAD_R, HUMAN_BULLETCOLLISION_HEAD_H, bvx, bvy, bvz) == true ){
				//Processing by a person
				HitBulletHuman(i, 0, bvx, bvy, bvz, brx, attacks, humanid, ontargetcnt);

				//Set up a list for human judgment (one time initialization, and only one person at the end)
				BulletObj_HumanIndex[objectID].Init();
				BulletObj_HumanIndex[objectID].SetIndexFlag(i);

				//Attack and penetration
				attacks = (int)((float)attacks * 0.5f);
				penetration -= 1;

				CollideFlag = true;
			}

			//Upper body and perception
			if( CollideCylinderInside(ox, oy + HUMAN_BULLETCOLLISION_LEG_H, oz, HUMAN_BULLETCOLLISION_UP_R, HUMAN_BULLETCOLLISION_UP_H, bvx, bvy, bvz) == true ){
				//Processing by a person
				HitBulletHuman(i, 1, bvx, bvy, bvz, brx, attacks, humanid, ontargetcnt);

				//Set up a list for human judgment (one time initialization, and only one person at the end)
				BulletObj_HumanIndex[objectID].Init();
				BulletObj_HumanIndex[objectID].SetIndexFlag(i);

				//Attack and penetration
				attacks = (int)((float)attacks * 0.6f);
				penetration -= 1;

				CollideFlag = true;
			}

			//Foot and Perspective
			if( CollideCylinderInside(ox, oy, oz, HUMAN_BULLETCOLLISION_LEG_R, HUMAN_BULLETCOLLISION_LEG_H, bvx, bvy, bvz) == true ){
				//Processing by a person
				HitBulletHuman(i, 2, bvx, bvy, bvz, brx, attacks, humanid, ontargetcnt);

				//Set up a list for human judgment (one time initialization, and only one person at the end)
				BulletObj_HumanIndex[objectID].Init();
				BulletObj_HumanIndex[objectID].SetIndexFlag(i);

				//Attack and penetration
				attacks = (int)((float)attacks * 0.7f);
				penetration -= 1;

				CollideFlag = true;
			}
		}

		//If the penetration force is not obtained, the bullet will be disabled (determining 2 )
		if( penetration < 0 ){
			in_bullet->SetEnableFlag(false);
			mapflag = 0;
			break;
		}

		//Determination with small items
		for(int i=0; i<MAX_SMALLOBJECT; i++){
			//not processed unless used
			if( SmallObjectIndex[i].GetEnableFlag() == false ){ continue; }

			//Get coordinates
			float ox, oy, oz;
			int id;
			float decide;
			SmallObjectIndex[i].GetPosData(&ox, &oy, &oz, NULL);
			SmallObjectIndex[i].GetParamData(&id, NULL);

			//Get the collision volume size.
			if( (TOTAL_PARAMETERINFO_SMALLOBJECT <= id)&&(id <= (TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT -1)) ){
				decide = (float)MIFdata->GetAddSmallobjectDecide(id - TOTAL_PARAMETERINFO_SMALLOBJECT)*SMALLOBJECT_COLLISIONSCALE;
			}
			else{
				SmallObjectParameter Param;
				GameParamInfo->GetSmallObject(id, &Param);
				decide = (float)Param.decide*SMALLOBJECT_COLLISIONSCALE;
			}

			//Collision detection
			if( CollideSphereInside(ox, oy, oz, decide, bvx, bvy, bvz) == true ){
				//Processing for small items
				int attack2 = (int)(floorf( (float)attacks * 0.25f ));
				HitBulletSmallObject(i, bvx, bvy, bvz, attack2, teamid);

				//Calculate damage.
				attacks = (int)(floorf( (float)attacks * 0.7f ));

				CollideFlag = true;
			}
		}

		if( mapflag > 0 ){
			//Determination of the above
			if( CollD->CheckALLBlockInside(bvx, bvy, bvz) == true ){
				//Processing hitting the bullet map
				HitBulletMap(bx + vx*(Dist-1.0f), by + vy*(Dist-1.0f), bz + vz*(Dist-1.0f), teamid);

				//Attack and penetration
				penetration -= 1;
				if( penetration >= 0 ){
					attacks = (int)((float)attacks * 0.6f);
				}

				mapflag = 2;
				CollideFlag = true;
			}
		}
	}

	//If there is a block on the ballistic, but it is not judged as a block
	// (=There is a thin block)
	if( mapflag == 1 ){
		//Processing hitting the bullet map
		HitBulletMap(bx + vx*(Dist-1.0f), by + vy*(Dist-1.0f), bz + vz*(Dist-1.0f), teamid);

		//Calculate damage.
		if( penetration > 0 ){
			attacks = (int)((float)attacks * 0.75f);
		}
		else{
			attacks = (int)((float)attacks * 0.55f);
		}

		CollideFlag = true;
	}

	//Apply settings (especially attack force and penetration force)
	in_bullet->SetParamData(attacks, penetration, speed, teamid, humanid, ontargetcnt, false);

	return CollideFlag;
}

//! @brief Processing hitting the bullet map
//! @param x X coordinate of impact.
//! @param y Y coordinate of impact.
//! @param z Z coordinate of impact.
//! @param teamID Projector Team Number
void ObjectManager::HitBulletMap(float x, float y, float z, int teamID)
{
	//Draw effects (smoke)
	AddEffect(x, y, z, 0.0f, 0.05f, 0.0f, 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, 3.0f, 0.24f, Resource->GetEffectSmokeTexture(), 20, 0, 0.63f, -0.03f, EFFECT_NORMAL, 20);

	//Play sound effects
	GameSound->HitMap(x, y, z, teamID);
}

//! @brief Processing of bullets to a person
//! @param HitHuman_id Character ID.
//! @param Hit_id Default (Head: 0 Body: 1 Foot: 2)
//! @param x X coordinate of the hit.
//! @param y Y coordinate of the hit.
//! @param z Z coordinate of the hit.
//! @param brx Horizontal angle.
//! @param attacks Attack Force
//! @param Shothuman_id Number of fired persons
//! @param ontargetcnt Number of counts during life
void ObjectManager::HitBulletHuman(int HitHuman_id, int Hit_id, float x, float y, float z, float brx, int attacks, int Shothuman_id, float ontargetcnt)
{
	int Shothuman_TeamID;
	int HitHuman_TeamID;
	int hp_old;
	int damage = 0;
	int paramid;
	HumanParameter Paraminfo;
	bool NotRobot;

	//not used or treated with carcasses.
	if( HumanIndex[HitHuman_id].GetEnableFlag() == false ){ return; }
	if( HumanIndex[HitHuman_id].GetDeadFlag() == true ){ return; }

	//Get the team IDs of the shooter and the target character.
	HumanIndex[Shothuman_id].GetParamData(NULL, NULL, NULL, &Shothuman_TeamID);
	HumanIndex[HitHuman_id].GetParamData(NULL, NULL, NULL, &HitHuman_TeamID);

	//HP acquisition before damage calculation
	hp_old = HumanIndex[HitHuman_id].GetHP();

	//Damage and shock to people
	if( Hit_id == 0 ){ HumanIndex[HitHuman_id].HitBulletHead(attacks); }
	if( Hit_id == 1 ){ HumanIndex[HitHuman_id].HitBulletUp(attacks); }
	if( Hit_id == 2 ){ HumanIndex[HitHuman_id].HitBulletLeg(attacks); }
	HumanIndex[HitHuman_id].AddPosOrder(brx, 0.0f, 1.0f);

#ifdef ENABLE_BUG_TEAMID
	bool flag = true;

	//If the team number is negative and the team number is large, flag is disabled.
	if( (HitHuman_TeamID < 0)&&(Shothuman_TeamID < 0) ){
		if( HitHuman_TeamID < Shothuman_TeamID ){
			flag = false;
		}
	}

	if( flag == true ){
		HumanIndex[HitHuman_id].SetHitFlag(brx*-1 - (float)M_PI/2);
	}
#else
	HumanIndex[HitHuman_id].SetHitFlag(brx*-1 - (float)M_PI/2);
#endif

	//Determining whether the robot is
	HumanIndex[HitHuman_id].GetParamData(&paramid, NULL, NULL, NULL);
	GameParamInfo->GetHuman(paramid, &Paraminfo);
	if( Paraminfo.type == 1 ){
		NotRobot = false;
	}
	else{
		NotRobot = true;
	}

	//Drawing effects (blood)
	if( Hit_id == 0 ){ damage = (int)((float)attacks * HUMAN_DAMAGE_HEAD); }
	if( Hit_id == 1 ){ damage = (int)((float)attacks * HUMAN_DAMAGE_UP); }
	if( Hit_id == 2 ){ damage = (int)((float)attacks * HUMAN_DAMAGE_LEG); }
	SetHumanBlood(x, y, z, damage, NotRobot);

	//Play sound effects
	GameSound->HitHuman(x, y, z, Hit_id, Shothuman_TeamID);

	if( (0 <= Shothuman_id)&&(Shothuman_id < MAX_HUMAN) ){
		//Addition to the results of the person who fired the bullet
		Human_ontarget[Shothuman_id] += ontargetcnt;
		if( Hit_id == 0 ){ Human_headshot[Shothuman_id] += 1; }
		if( (hp_old > 0)&&(HumanIndex[HitHuman_id].GetHP() <= 0) ){
			//If hp <= 0 before the damage calculation and hp <= 0 after the calculation, this time I died.
			Human_kill[Shothuman_id] += 1;
		}
	}

	//Log-related processing
	if( (hp_old > 0)&&(HumanIndex[HitHuman_id].GetHP() <= 0) ){
		//If hp <= 0 before the damage calculation and hp <= 0 after the calculation, this time I died.

		int player_teamid;
		HumanIndex[Player_HumanID].GetParamData(NULL, NULL, NULL, &player_teamid);

		ObjectLog->DiedHuman(Shothuman_id, HitHuman_id, Shothuman_TeamID, HitHuman_TeamID, player_teamid);
	}
}

//! @brief Processing hitting bullets into small items
//! @param HitSmallObject_id Number of accessories
//! @param x X coordinate of impact.
//! @param y Y coordinate of impact.
//! @param z Z coordinate of impact.
//! @param attacks Attack Force
//! @param teamID Projector Team Number
void ObjectManager::HitBulletSmallObject(int HitSmallObject_id, float x, float y, float z, int attacks, int teamID)
{
	int hp;

	//not processed unless used.
	if( SmallObjectIndex[HitSmallObject_id].GetEnableFlag() == false ){ return; }

	//If there is no physical strength, it will not be processed.
	hp = SmallObjectIndex[HitSmallObject_id].GetHP();
	if( hp <= 0 ){ return; }

	//Damage small items
	SmallObjectIndex[HitSmallObject_id].HitBullet(attacks);

	//Draw effects (smoke)
	//AddEffect(x, y, z, 0.01f*(GetRand(101)-50), 0.01f*(GetRand(101)-50), 0.01f*(GetRand(101)-50), 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, 1.0f, 0.1f, Resource->GetEffectSmokeTexture(), 10, 0, 0.625f, -0.047f, EFFECT_NORMAL, 255);
	//AddEffect(x, y, z, 0.01f*(GetRand(101)-50), 0.01f*(GetRand(101)-50), 0.01f*(GetRand(101)-50), 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, 1.0f, 0.1f, Resource->GetEffectSmokeTexture(), 10, 0, 0.625f, -0.047f, EFFECT_NORMAL, 255);
	AddEffect(x, y, z, 0.0f, 0.0f, 0.0f, 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, 1.0f, 0.1f, Resource->GetEffectSmokeTexture(), 10, 0, 0.625f, -0.047f, EFFECT_NORMAL, 255);
	AddEffect(x, y, z, 0.0f, 0.01f*(GetRand(10)-5), 0.0f, 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, 1.0f, 0.1f, Resource->GetEffectSmokeTexture(), 10, 0, 0.625f, -0.047f, EFFECT_NORMAL, 255);

	//Play sound effects
	int id;
	SmallObjectIndex[HitSmallObject_id].GetParamData(&id, NULL);
	GameSound->HitSmallObject(x, y, z, id, teamID);

	//Log-related processing
	hp = SmallObjectIndex[HitSmallObject_id].GetHP();
	if( hp <= 0 ){
		//If hp <= 0 before the damage calculation and hp <= 0 after the calculation, this time it was destroyed.
		ObjectLog->BreakSmallObject(HitSmallObject_id);
	}
}

//! @brief Damage detection and processing of grenades
//! @param in_grenade Target grenades object
//! @return Hit: true; no hit: false.
//! @attention The target is "people" and "small items".
//! @attention We do not only damage judgment but also consistently perform damage calculations and sound effects.
bool ObjectManager::GrenadeExplosion(grenade *in_grenade)
{
	if( in_grenade == NULL ){ return false; }

	bool returnflag = false;

	//Get coordinates
	float gx, gy, gz;
	int teamid, humanid;
	in_grenade->GetPosData(&gx, &gy, &gz, NULL, NULL);
	in_grenade->GetParamData(NULL, &teamid, &humanid, NULL);

	//Check blast collision with characters.
	for(int i=0; i<MAX_HUMAN; i++){
		//Do not process if it is not initialized or carcass.
		if( HumanIndex[i].GetEnableFlag() == false ){ continue; }
		if( HumanIndex[i].GetDeadFlag() == true ){ continue; }

		int HitHuman_TeamID;
		float hx, hy, hz;
		float x, y, z, r;

		//the coordinates of a person and determine the distance
		HumanIndex[i].GetPosData(&hx, &hy, &hz, NULL);
		x = hx - gx;
		y = hy - gy;
		z = hz - gz;
		r = sqrtf(x*x + y*y + z*z);

		//Not count if it is farther than 100.0
		if( r > MAX_DAMAGE_GRENADE_DISTANCE + HUMAN_HEIGHT ){ continue; }

		float dummy = 0.0f;
		int total_damage = 0;
		int damage;

		//Check collisions at the feet.
		y = hy + 2.0f - gy;
		r = sqrtf(x*x + y*y + z*z);
		//If the block is not blocked (should be judged by ray, and if it is not hit)
		if( CollD->CheckALLBlockIntersectRay(gx, gy, gz, x/r, y/r, z/r, NULL, NULL, &dummy, r) == false ){
			//Damage
			damage = HUMAN_DAMAGE_GRENADE_LEG - (int)((float)HUMAN_DAMAGE_GRENADE_LEG/MAX_DAMAGE_GRENADE_DISTANCE * r);
			if( damage > 0 ){
				total_damage += damage;
			}
		}

		//Determination on the head
		y = hy + (HUMAN_HEIGHT-2.0f) - gy;
		r = sqrtf(x*x + y*y + z*z);
		//If the block is not blocked (should be judged by ray, and if it is not hit)
		if( CollD->CheckALLBlockIntersectRay(gx, gy, gz, x/r, y/r, z/r, NULL, NULL, &dummy, r) == false ){
			//Damage
			damage = HUMAN_DAMAGE_GRENADE_HEAD - (int)((float)HUMAN_DAMAGE_GRENADE_HEAD/MAX_DAMAGE_GRENADE_DISTANCE * r);
			if( damage > 0 ){
				total_damage += damage;
			}
		}

		if( total_damage > 0 ){
			int hp_old;

			//HP acquisition before damage calculation
			hp_old = HumanIndex[i].GetHP();

			//Damage
			HumanIndex[i].HitGrenadeExplosion(total_damage);

			HumanIndex[i].GetParamData(NULL, NULL, NULL, &HitHuman_TeamID);

			float y2;
			float arx, ary;

			if( (0 <= humanid)&&(humanid < MAX_HUMAN) ){
				//Add to the result of the fired person if it is defeated
				if( HumanIndex[i].GetHP() <= 0 ){ Human_kill[humanid] += 1; }
			}

			//Drawing effects (blood)
			SetHumanBlood(hx, hy+15.0f, hz, total_damage, false);

			//Calculate the distance between the character and the grenade.
			x = gx - hx;
			y = gy - (hy + 1.0f);
			z = gz - hz;

			//Seeking Angle
			arx = atan2f(z, x);

			if( sinf(atan2f(y, sqrtf(x*x + z*z))) < 0.0f ){		//If you fly in the upper direction, you can draw the angle
				y2 = gy - (hy + HUMAN_HEIGHT);
				ary = atan2f(y2, sqrtf(x*x + z*z)) + (float)M_PI;
			}
			else{		//Cancel the vertical angle when moving downward, to prevent the blast from pushing the character into the ground.
				ary = 0.0f;
			}

#ifdef ENABLE_BUG_TEAMID
			bool flag = true;

			//If the team number is negative and the team number is large, flag is disabled.
			if( (HitHuman_TeamID < 0)&&(teamid < 0) ){
				if( HitHuman_TeamID < teamid ){
					flag = false;
				}
			}

			if( flag == true ){
				HumanIndex[i].SetHitFlag(arx*-1 + (float)M_PI/2);
			}
#else
			HumanIndex[i].SetHitFlag(arx*-1 + (float)M_PI/2);
#endif

			//Wind blowing
			HumanIndex[i].AddPosOrder(arx, ary, 2.2f/MAX_DAMAGE_GRENADE_DISTANCE * (MAX_DAMAGE_GRENADE_DISTANCE - sqrtf(x*x + y*y + z*z)));

			//Log-related processing
			if( (hp_old > 0)&&(HumanIndex[i].GetHP() <= 0) ){
				//If hp <= 0 before the damage calculation and hp <= 0 after the calculation, this time I died.

				int player_teamid;
				HumanIndex[Player_HumanID].GetParamData(NULL, NULL, NULL, &player_teamid);

				ObjectLog->DiedHuman(humanid, i, teamid, HitHuman_TeamID, player_teamid);
			}

			returnflag = true;
		}
	}

	//Check blast collision with small objects.
	for(int i=0; i<MAX_SMALLOBJECT; i++){
		//Do not process if used
		if( SmallObjectIndex[i].GetEnableFlag() == false ){ continue; }

		float sx, sy, sz;
		float x, y, z, r;

		//Get the coordinates of the accessories and determine the distance
		SmallObjectIndex[i].GetPosData(&sx, &sy, &sz, NULL);
		x = sx - gx;
		y = sy - gy;
		z = sz - gz;
		r = sqrtf(x*x + y*y + z*z);

		//Not count if it is farther than 100.0
		if( r > MAX_DAMAGE_GRENADE_DISTANCE ){ continue; }

		float dummy = 0.0f;

		//If the block is not blocked (should be judged by ray, and if it is not hit)
		if( CollD->CheckALLBlockIntersectRay(gx, gy, gz, x/r, y/r, z/r, NULL, NULL, &dummy, r) == false ){
			int id, damage;
			SmallObjectIndex[i].GetParamData(&id, NULL);

			//Calculate and apply the damage.
			damage = SMALLOBJECT_DAMAGE_GRENADE - (int)((float)SMALLOBJECT_DAMAGE_GRENADE/MAX_DAMAGE_GRENADE_DISTANCE * r);
			SmallObjectIndex[i].HitGrenadeExplosion(damage);

			//Emit sound effects from the small object.
			GameSound->HitSmallObject(sx, sy, sz, id, teamid);

			//Log-related processing
			if( SmallObjectIndex[i].GetHP() <= 0 ){
				//If hp <= 0 before the damage calculation and hp <= 0 after the calculation, this time it was destroyed.
				ObjectLog->BreakSmallObject(i);
			}

			returnflag = true;
		}
	}

	//Drawing Effects (Flash)
	AddEffect(gx, gy, gz, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 40.0f, 0.0f, Resource->GetEffectMflashTexture(), 0, 0, 0.63f, 0.0f, EFFECT_NORMAL, 2);

	//Drawing Effects
	AddEffect(gx, gy, gz, 0.0f, 0.1f, 0.0f, 0.0f,
					DegreeToRadian(10)*GetRand(36), 0.0f, 36.0f, 0.2f, Resource->GetEffectSmokeTexture(), 220, 0, 0.6f, -0.01f, EFFECT_NORMAL, 50);
	AddEffect(gx + ((float)GetRand(7)/10-0.3f), gy + ((float)GetRand(5)/10-0.2f), gz + ((float)GetRand(7)/10-0.3f), (float)GetRand(5)/10-0.2f, (float)GetRand(3)/10, (float)GetRand(5)/10-0.2f, 0.0f,
					DegreeToRadian(10)*GetRand(36), 0.0f, ((float)GetRand(11)+25.0f)*2, 0.3f, Resource->GetEffectSmokeTexture(), 0, 0, 0.47f, -0.004f, EFFECT_NORMAL, 120);
	AddEffect(gx + ((float)GetRand(13)/10-0.6f), gy + ((float)GetRand(11)/10-0.5f), gz + ((float)GetRand(13)/10-0.6f), (float)GetRand(7)/10-0.3f, (float)GetRand(7)/10-0.3f, (float)GetRand(7)/10-0.3f, 0.0f,
					DegreeToRadian(10)*GetRand(36), DegreeToRadian(1), ((float)GetRand(21)+20.0f)*2, 0.2f, Resource->GetEffectSmokeTexture(), 0, 0, 0.4f, -0.004f, EFFECT_NORMAL, 100);
	AddEffect(gx + ((float)GetRand(13)/10-0.6f), gy + ((float)GetRand(11)/10-0.5f), gz + ((float)GetRand(13)/10-0.6f), (float)GetRand(7)/10-0.3f, (float)GetRand(7)/10-0.3f, (float)GetRand(7)/10-0.3f, 0.0f,
					DegreeToRadian(10)*GetRand(36), DegreeToRadian(-1), ((float)GetRand(21)+20.0f)*2, 0.2f, Resource->GetEffectSmokeTexture(), 0, 0, 0.4f, -0.004f, EFFECT_NORMAL, 100);

	//Play sound effects
	GameSound->GrenadeExplosion(gx, gy, gz, teamid);

	return returnflag;
}

//! @brief Configuring effects when defeated
//! @param in_human Target human object
void ObjectManager::DeadEffect(human *in_human)
{
	if( in_human == NULL ){ return; }

	int paramid;
	HumanParameter data;
	bool robot;

	in_human->GetParamData(&paramid, NULL, NULL, NULL);

	if( GameParamInfo->GetHuman(paramid, &data) != 0 ){ return; }

	if( data.type == 1 ){
		robot = true;
	}
	else{
		robot = false;
	}

	//If you are a robot
	if( robot == true ){
		float hx, hy, hz, hrx, hry;

		//Get various parameters
		in_human->GetPosData(&hx, &hy, &hz, &hrx);
		hry = in_human->GetDeadRy();

		//Calculate coordinates around the waist.
		hx += cosf(hrx*-1 - (float)M_PI/2) * sinf(hry) * HUMAN_HEIGHT/2;
		hz += sinf(hrx*-1 - (float)M_PI/2) * sinf(hry) * HUMAN_HEIGHT/2;

		//Drawing Effects
		float rnd = DegreeToRadian(10)*GetRand(18);
		AddEffect(hx+1.0f, hy+1.0f, hz+1.0f, 0.1f, 0.05f, 0.1f, 0.0f, rnd, 0, GetRand(30)*0.1f+30.0f, 0.1f, Resource->GetEffectSmokeTexture(), 10, 0, 1.0f, -1.0f / (GAMEFPS * 3), EFFECT_NORMAL, (int)GAMEFPS * 3);
		AddEffect(hx+1.0f, hy-1.0f, hz-1.0f, 0.2f, 0.05f, -0.2f, 0.0f, rnd*-1, 0, GetRand(30)*0.1f+30.0f, 0.1f, Resource->GetEffectSmokeTexture(), 10, 0, 1.0f, -1.0f / (GAMEFPS * 3), EFFECT_NORMAL, (int)GAMEFPS * 3);
		AddEffect(hx-1.0f, hy-1.0f, hz-1.0f, -0.3f, 0.05f, -0.3f, 0.0f, rnd, 0, GetRand(30)*0.1f+30.0f, 0.1f, Resource->GetEffectSmokeTexture(), 10, 0, 1.0f, -1.0f / (GAMEFPS * 3), EFFECT_NORMAL, (int)GAMEFPS * 3);
	}
}

//! Determines whether blood is attached to the pet
//! @param in_effect Object effect object
//! @param id Pointer to receive a sticking block number
//! @param face Pointer to receive the attached face number
//! @param pos_x Pointer to receive X coordinates
//! @param pos_y Pointer to receive Y coordinates
//! @param pos_z Pointer to receive Z coordinates
//! @return Attaches: true; does not attach: false.
bool ObjectManager::CollideBlood(effect *in_effect, int *id, int *face, float *pos_x, float *pos_y, float *pos_z)
{
	if( in_effect == NULL ){ return false; }
	if( (id == NULL)||(face == NULL) ){ return false; }
	if( (pos_x == NULL)||(pos_y == NULL)||(pos_z == NULL) ){ return false; }

	//Do not handle invalid effects
	if( in_effect->GetEnableFlag() == false ){ return false; }
	if( in_effect->GetCollideMapFlag() == false ){ return false; }
	if( in_effect->GetTextureID() != Resource->GetEffectBloodTexture() ){ return false; }

	//If the blood does not appear, it will not be processed.
	if( GameConfig.GetBloodFlag() == false ){ return false; }

	float x, y, z;
	float move_x, move_y, move_z;
	float vx, vy, vz, dist, dist2;

	//Getting effect parameters
	in_effect->GetPosData(&x, &y, &z, NULL);
	in_effect->GetMove(&move_x, &move_y, &move_z);

	//If the effect does not move, it will not be processed.
	if( (move_x == 0.0f)&&(move_y == 0.0f)&&(move_z == 0.0f) ){ return false; }

	//Seeking s
	dist = sqrtf(move_x*move_x + move_y*move_y + move_z*move_z);
	vx = move_x / dist;
	vy = move_y / dist;
	vz = move_z / dist;

	//Determination of lay
	if( CollD->CheckALLBlockIntersectRay(x, y, z, vx, vy, vz, id, face, &dist2, dist) == false ){
		return false;
	}

	//Raise slightly above the map surface.
	dist2 -= 1.0f;

	//Determine the coordinates to be attached
	*pos_x = x + vx*dist2;
	*pos_y = y + vy*dist2;
	*pos_z = z + vz*dist2;

	return true;
}

//! @brief Pick Up Weapons
//! @param in_human Target human object
//! @param in_weapon Target weapon object
//! @attention This function fails for zombie character types.
void ObjectManager::PickupWeapon(human *in_human, weapon *in_weapon)
{
	if( in_human == NULL ){ return; }
	if( in_weapon == NULL ){ return; }

	//Not processed if disabled
	if( in_human->GetEnableFlag() == false ){ return; }
	if( in_human->GetHP() <= 0 ){ return; }

	//Fail if the character type is a zombie.
	int paramid;
	HumanParameter Paraminfo;
	in_human->GetParamData(&paramid, NULL, NULL, NULL);
	GameParamInfo->GetHuman(paramid, &Paraminfo);
	if( Paraminfo.type == 2 ){ return; }

	//Initialized weapons and weapons that no one uses -
	if( (in_weapon->GetEnableFlag() == true)&&(in_weapon->GetUsingFlag() == false) ){
		float human_x, human_y, human_z;
		float weapon_x, weapon_y, weapon_z;
		float x, z;
		float r;

		//Get the coordinates of people and weapons
		in_human->GetPosData(&human_x, &human_y, &human_z, NULL);
		in_weapon->GetPosData(&weapon_x, &weapon_y, &weapon_z, NULL);

		//If the above is within the range
		if( ((human_y + HUMAN_PICKUPWEAPON_L) <= weapon_y)&&(weapon_y <= (human_y + HUMAN_PICKUPWEAPON_H)) ){
			//Calculate the distance.
			x = human_x - weapon_x;
			z = human_z - weapon_z;
			r = x*x + z*z;
			//If the distance is within the range, pick up.
			if( r < (HUMAN_PICKUPWEAPON_R * HUMAN_PICKUPWEAPON_R) ){
				in_human->PickupWeapon(in_weapon);
			}
		}
	}
}

//! @brief Release Objects
void ObjectManager::CleanupPointDataToObject()
{
	for(int i=0; i<MAX_HUMAN; i++){
		HumanIndex[i].SetEnableFlag(false);
	}

	for(int i=0; i<MAX_WEAPON; i++){
		WeaponIndex[i].SetEnableFlag(false);
	}

	for(int i=0; i<MAX_SMALLOBJECT; i++){
		SmallObjectIndex[i].SetEnableFlag(false);
	}

	for(int i=0; i<MAX_BULLET; i++){
		BulletIndex[i].SetEnableFlag(false);
	}

	for(int i=0; i<MAX_GRENADE; i++){
		GrenadeIndex[i].SetEnableFlag(false);
	}

	for(int i=0; i<MAX_EFFECT; i++){
		EffectIndex[i].SetEnableFlag(false);
	}


	if( Resource != NULL ){
		Resource->CleanupHumanTexture();
	}
}

//! @brief Place objects based on point data
void ObjectManager::LoadPointData()
{
	AddCollisionFlag = false;
	Player_HumanID = 0;

	//Initialization of gun flag
	for(int i=0; i<MAX_HUMAN; i++){
		Human_ShotFlag[i] = false;
		Human_YakkyouCnt[i] = -1;
	}

	//FF Enabled flag setting
	for(int i=0; i<MAX_HUMAN; i++){
		FriendlyFire[i] = false;
	}

	//Human Texture Refresh Flag Initialization
	for(int i=0; i<MAX_HUMAN; i++){
		Human_FrameTextureRefresh[i] = false;
	}

	/*
	//Find information points
	for(int i=0; i<PointData->GetTotaldatas(); i++){
		pointdata data;
		PointData->Getdata(&data, i);

		if( data.p1 == POINT_P1TYPE_HUMANINFO ){
			//Register Human Texture
			Resource->AddHumanTexture(data.p2);
		}
	}
	*/

	//Search for people, weapons, and accessories
	for(int i=0; i<PointData->GetTotaldatas(); i++){
		pointdata data;
		PointData->Getdata(&data, i);

		//If a person
		if( (data.p1 == POINT_P1TYPE_HUMAN)||(data.p1 == POINT_P1TYPE_HUMAN2) ){
			pointdata humaninfodata;

			//Find information points
			if( PointData->SearchPointdata(&humaninfodata, 0x01 + 0x08, POINT_P1TYPE_HUMANINFO, 0, 0, data.p2, 0) == 0 ){
				//continue;

				//If no character-information point is found, add "Special Black A" provisionally to reproduce the original bug.

				HumanParameter HumanParam;
				int Weapon[TOTAL_HAVEWEAPON];
				GameParamInfo->GetHuman(0, &HumanParam);
				for(int j=0; j<TOTAL_HAVEWEAPON; j++){
					Weapon[j] = HumanParam.Weapon[j];
				}

				//Register Human Texture
				Resource->AddHumanTexture(0);

				//Add people
				if( data.p1 == POINT_P1TYPE_HUMAN2 ){
					Weapon[1] = ID_WEAPON_NONE;
				}

				//Register a number if you are a player
				if( (data.p4 == 0)&&(Player_HumanID == 0) ){
					Player_HumanID = AddHumanIndex(data.x, data.y, data.z, data.r, 0, 0, Weapon, data.p4, true);
				}
				else{
					AddHumanIndex(data.x, data.y, data.z, data.r, 0, 0, Weapon, data.p4, true);
				}
			}
			else{
				//Register Human Texture
				Resource->AddHumanTexture(humaninfodata.p2);

				//Add as a person
				AddHumanIndex(data, humaninfodata, true);
			}
		}

		//Weapon
		if( (data.p1 == POINT_P1TYPE_WEAPON)||(data.p1 == POINT_P1TYPE_RAND_WEAPON) ){
			AddWeaponIndex(data);
		}

		//For small items
		if( data.p1 == POINT_P1TYPE_SMALLOBJ ){
			AddSmallObjectIndex(data);
		}
	}

	//Log-related processing
	ObjectLog->ClearLog();
	ObjectLog->InfoLog("Set object...");
}

//! @brief Resource Recovery
//! @todo All effects are disabled.
void ObjectManager::Recovery()
{
	//Get the model number of a person
	int upmodel[TOTAL_UPMODE];
	int armmodel[TOTAL_ARMMODE];
	int legmodel;
	int walkmodel[TOTAL_WALKMODE];
	int runmodel[TOTAL_RUNMODE];
	Resource->GetHumanModel(upmodel, armmodel, &legmodel, walkmodel, runmodel);

	//Character.
	int HumanID;
	int GetHumanFlag;
	HumanParameter HumanParam;
	for(int i=0; i<MAX_HUMAN; i++){
		if( HumanIndex[i].GetEnableFlag() == true ){
			HumanIndex[i].GetParamData(&HumanID, NULL, NULL, NULL);

			GetHumanFlag = GameParamInfo->GetHuman(HumanID, &HumanParam);

#ifndef ENABLE_BUG_HUMANWEAPON
			if( GetHumanFlag != 0 ){
				HumanIndex[i].SetEnableFlag(false);
				continue;
			}
#endif

			if( GetHumanFlag == 0 ){
				//Register Human Texture
				Resource->AddHumanTexture(HumanID);

				int id = Resource->GetHumanTexture(HumanID);
				if( id == -1 ){
					//If the texture is not available, enable the refresh flag to get each frame
					Human_FrameTextureRefresh[i] = true;
				}

				HumanIndex[i].SetTexture(id);
				HumanIndex[i].SetModel(upmodel[ HumanParam.model ], armmodel, legmodel, walkmodel, runmodel);
			}
			else{
#ifdef ENABLE_BUG_HUMANWEAPON
				HumanIndex[i].SetTexture(d3dg->GetMapTextureID(0));
				HumanIndex[i].SetModel(upmodel[0], armmodel, legmodel, walkmodel, runmodel);
#endif
			}
		}
	}

	//Weapons
	int WeaponID;
	int Weaponmodel, Weapontexture;
	for(int i=0; i<MAX_WEAPON; i++){
		if( WeaponIndex[i].GetEnableFlag() == true ){
			//Getting the setting value
			WeaponParameter WeaponParam;
			WeaponIndex[i].GetParamData(&WeaponID, NULL, NULL);
			if( Resource->GetWeaponModelTexture(WeaponID, &Weaponmodel, &Weapontexture) == 1 ){ continue; }
			if( GameParamInfo->GetWeapon(WeaponID, &WeaponParam) == 1 ){ continue; }

			//Application
			WeaponIndex[i].SetModel(Weaponmodel, WeaponParam.size);
			WeaponIndex[i].SetTexture(Weapontexture);
		}
	}

	//Small object.
	int SmallObjectID;
	int SmallObjectmodel, SmallObjecttexture;
	for(int i=0; i<MAX_SMALLOBJECT; i++){
		if( SmallObjectIndex[i].GetEnableFlag() == true ){
			//Getting the setting value
			SmallObjectIndex[i].GetParamData(&SmallObjectID, NULL);
			if( Resource->GetSmallObjectModelTexture(SmallObjectID, &SmallObjectmodel, &SmallObjecttexture) == 1 ){ continue; }

			//Application
			SmallObjectIndex[i].SetModel(SmallObjectmodel, SMALLOBJECT_SCALE);
			SmallObjectIndex[i].SetTexture(SmallObjecttexture);
		}
	}

	int bulletmodel, bullettexture;
	BulletParameter data;

	//Application of bullets
	if( Resource->GetBulletModelTexture(0, &bulletmodel, &bullettexture) == 0 ){
		if( GameParamInfo->GetBullet(0, &data) == 0 ){
			for(int i=0; i<MAX_BULLET; i++){
				BulletIndex[i].SetModel(bulletmodel, data.size);
				BulletIndex[i].SetTexture(bullettexture);
			}
		}
	}

	//Hand grenades
	if( Resource->GetBulletModelTexture(1, &bulletmodel, &bullettexture) == 0 ){
		if( GameParamInfo->GetBullet(1, &data) == 0 ){
			for(int i=0; i<MAX_GRENADE; i++){
				GrenadeIndex[i].SetModel(bulletmodel, data.size);
				GrenadeIndex[i].SetTexture(bullettexture);
			}
		}
	}

	//Disable all effects
	for(int i=0; i<MAX_EFFECT; i++){
		if( EffectIndex[i].GetEnableFlag() == true ){
			EffectIndex[i].SetEnableFlag(false);
		}
	}

	//Log-related processing
	ObjectLog->InfoLog("Recovery object...");
}

//! @brief Set the additional collision-detection flag.
//! @param flag Flags
void ObjectManager::SetAddCollisionFlag(bool flag)
{
	AddCollisionFlag = flag;
}

//! @brief FFate FF flags
//! @param id Data Number
//! @return Flags
bool ObjectManager::GetFriendlyFireFlag(int id)
{
	if( (id < 0)||(MAX_HUMAN <= id) ){ return false; }
	return FriendlyFire[id];
}

//! @brief Set the FF enabled flag
//! @param id Data Number
//! @param flag Flags
void ObjectManager::SetFriendlyFireFlag(int id, bool flag)
{
	if( (id < 0)||(MAX_HUMAN <= id) ){ return; }
	FriendlyFire[id] = flag;
}

//! @brief Get Player Number
//! @return Player Data Number
int ObjectManager::GetPlayerID()
{
	return Player_HumanID;
}

//! @brief Set Player Number
//! @param id Player Data Number
void ObjectManager::SetPlayerID(int id)
{
	Player_HumanID = id;
}

//! @brief Get the human class of the specified data number
//! @param id Data Number
//! @return Pointer of a human object (NULL with invalid data number)
human* ObjectManager::GetHumanObject(int id)
{
	if( (id < 0)||(MAX_HUMAN <= id) ){ return NULL; }
	return &(HumanIndex[id]);
}

//! @brief Get the human class of the player
//! @return Pointer of a person object (player)
human* ObjectManager::GetPlayerHumanObject()
{
	return GetHumanObject(Player_HumanID);
}

//! @brief Get the data number of the specified human pointer
//! @param object Pointer of Human Objects
//! @return Data ID (-1 on error).
int ObjectManager::GetHumanObjectID(human* object)
{
	if( object == NULL ){ return -1; }

	for(int i=0; i<MAX_HUMAN; i++){
		if( &(HumanIndex[i]) == object ){
			return i;
		}
	}

	return -1;
}

//! @brief Get the weapon class of the specified data number
//! @param id Data Number
//! @return Pointer of weapon object (NULL with invalid data number)
weapon* ObjectManager::GetWeaponObject(int id)
{
	if( (id < 0)||(MAX_WEAPON <= id) ){ return NULL; }
	return &(WeaponIndex[id]);
}

//! @brief Get the smallobject class of the specified data number
//! @param id Data Number
//! @return Pointer of small object (NULL with invalid data number)
smallobject* ObjectManager::GetSmallObject(int id)
{
	if( (id < 0)||(MAX_SMALLOBJECT <= id) ){ return NULL; }
	return &(SmallObjectIndex[id]);
}

//! @brief Get the bullet class of the specified data number
//! @param id Data Number
//! @return Pointer of bullet object (NULL with invalid data number)
bullet* ObjectManager::GetBulletObject(int id)
{
	if( (id < 0)||(MAX_BULLET <= id) ){ return NULL; }
	return &(BulletIndex[id]);
}

//! @brief Get the data number of the specified Bullet pointer
//! @param object Pointer of bullet object
//! @return Data ID (-1 on error).
int ObjectManager::GetBulletObjectID(bullet* object)
{
	if( object == NULL ){ return -1; }

	for(int i=0; i<MAX_BULLET; i++){
		if( &(BulletIndex[i]) == object ){
			return i;
		}
	}

	return -1;
}

//! @brief Get an unused bullet instance.
//! @return Pointer to an unused bullet object (NULL on failure).
bullet* ObjectManager::GetNewBulletObject()
{
	for(int i=0; i<MAX_BULLET; i++){
		if( BulletIndex[i].GetEnableFlag() == false ){
			return &(BulletIndex[i]);
		}
	}
	return NULL;
}

//! @brief Get an unused grenade instance.
//! @return Pointer to an unused grenade object (NULL on failure).
grenade* ObjectManager::GetNewGrenadeObject()
{
	for(int i=0; i<MAX_GRENADE; i++){
		if( GrenadeIndex[i].GetEnableFlag() == false ){
			return &(GrenadeIndex[i]);
		}
	}
	return NULL;
}

//! @brief Search for people
//! @param p4 Identification number of search target
//! @return Pointer of the corresponding human class (NULL if not found)
//! @attention Returns the first data if multiple data is applied.
human* ObjectManager::SearchHuman(signed short int p4)
{
	signed short int humanp4;

	for(int i=0; i<MAX_HUMAN; i++){
		//Not processed if not used
		if( HumanIndex[i].GetEnableFlag() == false ){ continue; }

		//Get 4 parameters
		HumanIndex[i].GetParamData(NULL, NULL, &humanp4, NULL);

		//if matched with the specified p4
		if( humanp4 == p4 ){
			return &(HumanIndex[i]);
		}
	}
	return NULL;
}

//! @brief Search Accessories
//! @param p4 Identification number of search target
//! @return Pointer of the corresponding smallobject class (NULL if not found)
//! @attention Returns the first data if multiple data is applied.
smallobject* ObjectManager::SearchSmallobject(signed short int p4)
{
	signed short int smallobjectp4;

	for(int i=0; i<MAX_SMALLOBJECT; i++){
		//Not processed if not used
		// [Invalid for determining destroyed objects]
		//if( SmallObjectIndex[i].GetEnableFlag() == false ){ continue; }

		//Get 4 parameters
		SmallObjectIndex[i].GetParamData(NULL, &smallobjectp4);

		//if matched with the specified p4
		if( smallobjectp4 == p4 ){
			return &(SmallObjectIndex[i]);
		}
	}
	return NULL;
}

//! @brief s the number of valid objects
//! @param HumanCnt Pointer to receive the valid number of human objects (NULL allowed)
//! @param WeaponCnt Pointer to receive the effective number of weapon objects (NULL allowed)
//! @param SmallobjectCnt Pointer to receive the valid number of small objects (NULL allowed)
//! @param BulletCnt Pointer to receive the active number of bullet objects (NULL allowed)
//! @param GrenadeCnt Pointer to receive the effective number of grenades objects (NULL allowed)
//! @param EffectCnt Pointer to receive the effective number of effects objects (NULL allowed)
void ObjectManager::GetTotalObjects(int *HumanCnt, int *WeaponCnt, int *SmallobjectCnt, int *BulletCnt, int *GrenadeCnt, int *EffectCnt)
{
	if( HumanCnt != NULL ){
		*HumanCnt = 0;
		for(int i=0; i<MAX_HUMAN; i++){
			if( HumanIndex[i].GetEnableFlag() == true ){ *HumanCnt += 1; }
		}
	}

	if( WeaponCnt != NULL ){
		*WeaponCnt = 0;
		for(int i=0; i<MAX_WEAPON; i++){
			if( WeaponIndex[i].GetEnableFlag() == true ){ *WeaponCnt += 1; }
		}
	}

	if( SmallobjectCnt != NULL ){
		*SmallobjectCnt = 0;
		for(int i=0; i<MAX_SMALLOBJECT; i++){
			if( SmallObjectIndex[i].GetEnableFlag() == true ){ *SmallobjectCnt += 1; }
		}
	}

	if( BulletCnt != NULL ){
		*BulletCnt = 0;
		for(int i=0; i<MAX_BULLET; i++){
			if( BulletIndex[i].GetEnableFlag() == true ){ *BulletCnt += 1; }
		}
	}

	if( GrenadeCnt != NULL ){
		*GrenadeCnt = 0;
		for(int i=0; i<MAX_GRENADE; i++){
			if( GrenadeIndex[i].GetEnableFlag() == true ){ *GrenadeCnt += 1; }
		}
	}

	if( EffectCnt != NULL ){
		*EffectCnt = 0;
		for(int i=0; i<MAX_EFFECT; i++){
			if( EffectIndex[i].GetEnableFlag() == true ){ *EffectCnt += 1; }
		}
	}
}

//! @brief Run forward (run)
//! @param human_id Number of people (0 to MAX_HUMAN-1)
void ObjectManager::MoveForward(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return; }

	//Set the flag to the object
	HumanIndex[human_id].SetMoveForward();
}

//! @brief Execute backward movement.
//! @param human_id Number of people (0 to MAX_HUMAN-1)
void ObjectManager::MoveBack(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return; }

	//Set the flag to the object
	HumanIndex[human_id].SetMoveBack();
}

//! @brief Run left
//! @param human_id Number of people (0 to MAX_HUMAN-1)
void ObjectManager::MoveLeft(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return; }

	//Set the flag to the object
	HumanIndex[human_id].SetMoveLeft();
}

//! @brief Run right
//! @param human_id Number of people (0 to MAX_HUMAN-1)
void ObjectManager::MoveRight(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return; }

	//Set the flag to the object
	HumanIndex[human_id].SetMoveRight();
}

//! @brief Running a walk
//! @param human_id Number of people (0 to MAX_HUMAN-1)
void ObjectManager::MoveWalk(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return; }

	//Set the flag to the object
	HumanIndex[human_id].SetMoveWalk();
}

//! @brief Jumping
//! @param human_id Number of people (0 to MAX_HUMAN-1)
void ObjectManager::MoveJump(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return; }

	HumanIndex[human_id].Jump();
}

//! @brief Fire
//! @param human_id ID of the character firing.
//! @return Normal fired: 1 grenades fired: 2 failure: 0
int ObjectManager::ShotWeapon(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return 0; }

	human *MyHuman = &(HumanIndex[human_id]);

	float pos_x, pos_y, pos_z;
	int teamid;
	float rotation_x, armrotation_y;
	int weapon_paramid;
	int GunsightErrorRange;
	WeaponParameter ParamData;
	bool playerflag;
	bool grenadeflag;

	//es coordinates and angles of people
	MyHuman->GetPosData(&pos_x, &pos_y, &pos_z, NULL);
	MyHuman->GetParamData(NULL, NULL, NULL, &teamid);
	MyHuman->GetRxRy(&rotation_x, &armrotation_y);

	//Determine whether the player is
	if( MyHuman == &(HumanIndex[Player_HumanID]) ){
		playerflag = true;
	}
	else{
		playerflag = false;
	}

	//Requests bullet launch
	if( MyHuman->ShotWeapon(&weapon_paramid, &GunsightErrorRange) == false ){ return 0; }

	//Get weapon information
	if( GameParamInfo->GetWeapon(weapon_paramid, &ParamData) != 0 ){ return 0; }

	//Check whether this is a grenade.
	if( weapon_paramid == ID_WEAPON_GRENADE ){ grenadeflag = true; }
	else{ grenadeflag = false; }

	//Range of Errors
	int ErrorRange;
	ErrorRange = GunsightErrorRange;
	if( ErrorRange < ParamData.ErrorRangeMIN ){ ErrorRange = ParamData.ErrorRangeMIN; }

	//Determines the shooting angle (base of zero error)
	float rx, ry;
	rx = rotation_x*-1 + (float)M_PI/2;
	ry = armrotation_y;

	//Determine error
	if( ErrorRange != 0 ){
		int ErrorRange_x, ErrorRange_y;

		ErrorRange_x = GetRand(ErrorRange*2+1) - ErrorRange;
		ErrorRange_y = GetRand(ErrorRange*2+1) - ErrorRange;

		//Projection Errors Add to Angle
		rx += ErrorRange_x * DegreeToRadian(0.15f);
		ry += ErrorRange_y * DegreeToRadian(0.15f);
	}

	//Without grenades
	if( grenadeflag == false ){
		class bullet* newbullet;

		//A few minutes of shotguns and other bullets (pellets)
		for(int i=0; i<ParamData.pellet; i++){
			int attacks;
			float ontargetcnt;
			float rx2, ry2;

			//If there are multiple shotguns (pellets), etc.
			if( ParamData.pellet > 1 ){
				//Calculate damage per bullet and the hit counter.
				//  Double the whole piece.
				attacks = (int)( (float)ParamData.attacks / ((float)ParamData.pellet/2) );
				ontargetcnt = 1.0f / ((float)ParamData.pellet/2);

				//Increase the error further to spread the shots.
				int len;
				float a = DegreeToRadian(10) * GetRand(36);
				len = GetRand(5)*2+5;
				rx2 = rx + cosf(a)*len * DegreeToRadian(0.15f);
				ry2 = ry + sinf(a)*len * DegreeToRadian(0.15f);
			}
			else{
				//Determines the number of attack and life-time counts.
				attacks = ParamData.attacks;
				ontargetcnt = 1.0f;

				//The erroneous error is
				rx2 = rx;
				ry2 = ry;
			}

			//Get an unused object to fire.
			newbullet = GetNewBulletObject();
			if( newbullet == NULL ){ return 0; }

			//Fire bullets
			newbullet->SetPosData(pos_x, pos_y + WEAPONSHOT_HEIGHT, pos_z, rx2, ry2);
			newbullet->SetParamData(attacks, ParamData.penetration, (float)ParamData.speed * BULLET_SPEEDSCALE, teamid, human_id, ontargetcnt, true);
			newbullet->SetEnableFlag(true);

			//Initialization of the list for human judgment
			BulletObj_HumanIndex[ GetBulletObjectID(newbullet) ].Init();
		}

		//Set gun flag
		Human_ShotFlag[human_id] = true;
		Human_YakkyouCnt[human_id] = ParamData.yakkyou_delay;
	}
	else{
		class grenade* newgrenade;

		//Get an unused object to fire.
		newgrenade = GetNewGrenadeObject();
		if( newgrenade == NULL ){ return 0; }

		//Throw the grenade.
		newgrenade->SetPosData(pos_x, pos_y + WEAPONSHOT_HEIGHT, pos_z, rx, ry);
		newgrenade->SetParamData(8.0f, teamid, human_id, 1.0f, true);
		newgrenade->SetEnableFlag(true);
	}

	if( ParamData.soundvolume > 0 ){
		//Play the firing sound.
		GameSound->ShotWeapon(pos_x, pos_y + WEAPONSHOT_HEIGHT, pos_z, weapon_paramid, teamid, playerflag);
	}

	if( grenadeflag == true ){
		return 2;
	}
	return 1;
}

//! @brief Set the muzzle flash.
//! @param humanid Character ID.
//! @attention If the call timing of this function is incorrect, the muzzle does not match the muzzle.
void ObjectManager::ShotWeaponEffect(int humanid)
{
	//Check the value range
	if( (humanid < 0)||(MAX_HUMAN <= humanid) ){ return; }

	float pos_x, pos_y, pos_z;
	float rotation_x, armrotation_y;
	int weapon_paramid;
	WeaponParameter ParamData;
	float x, y, z;
	float flashsize, smokesize1, smokesize2;
	float rx, emx, emy, emz;

	//es coordinates and angles of people
	HumanIndex[humanid].GetPosData(&pos_x, &pos_y, &pos_z, NULL);
	HumanIndex[humanid].GetRxRy(&rotation_x, &armrotation_y);

	//Get weapon information
	weapon_paramid = HumanIndex[humanid].GetMainWeaponTypeNO();
	if( GameParamInfo->GetWeapon(weapon_paramid, &ParamData) != 0 ){ return; }

	//Decide the size of the smoke with the masul flash
	if( ParamData.silencer == true ){ flashsize = 1.2f; }
	else{ flashsize = 0.06f * ParamData.attacks; }
	smokesize1 = 0.04f * ParamData.attacks;
	smokesize2 = 0.032f * ParamData.attacks;

	//Determines smoke movement vector
	rx = rotation_x*-1 + (float)M_PI/2;
	emx = cosf(rx)*cosf(armrotation_y);
	emy = sinf(armrotation_y);
	emz = sinf(rx)*cosf(armrotation_y);

	//Specify the effect coordinates in the matrix
	WeaponAttachmentPosition(pos_x, pos_y + 16.0f, pos_z, ParamData.flashx/10, ParamData.flashy/10, ParamData.flashz/10, rotation_x, armrotation_y*-1, &x, &y, &z);

	//Mazul Flash Drawing
	AddEffect(x, y, z, 0.0f, 0.0f, 0.0f, 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, flashsize, 0.0f, Resource->GetEffectMflashTexture(), 0, 0, 0.63f, 0.0f, EFFECT_NORMAL, 1);

	//Specify the effect coordinates in the matrix
	WeaponAttachmentPosition(pos_x, pos_y + 16.0f, pos_z, ParamData.flashx/10, ParamData.flashy/10, ParamData.flashz/10 - 0.1f, rotation_x, armrotation_y*-1, &x, &y, &z);

	//Drawing Effects
	AddEffect(x, y, z, emx*0.15f, emy*0.15f + 0.02f, emz*0.15f, 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, smokesize1, 0.3f, Resource->GetEffectSmokeTexture(), 0, 0, 0.47f, -0.047f, EFFECT_NORMAL, 12);
	AddEffect(x, y, z, emx*0.45f, emy*0.45f, emz*0.45f, 0.0f, DegreeToRadian(10)*GetRand(36), 0.0f, smokesize2, 0.36f, Resource->GetEffectSmokeTexture(), 0, 0, 0.47f, -0.055f, EFFECT_NORMAL, 10);
}

//! @brief Configure the casing discharge
//! @param humanid Character ID.
//! @attention If the call timing of this function is incorrect, the position of the shell is not matched to the gun.
void ObjectManager::ShotWeaponYakkyou(int humanid)
{
	//Check the value range
	if( (humanid < 0)||(MAX_HUMAN <= humanid) ){ return; }

	float pos_x, pos_y, pos_z;
	float rotation_x, armrotation_y;
	int weapon_paramid;
	WeaponParameter ParamData;
	float x, y, z;
	float yakkyousize;
	float rx;
	float mx, my, mz;

	//es coordinates and angles of people
	HumanIndex[humanid].GetPosData(&pos_x, &pos_y, &pos_z, NULL);
	HumanIndex[humanid].GetRxRy(&rotation_x, &armrotation_y);

	//Get weapon information
	weapon_paramid = HumanIndex[humanid].GetMainWeaponTypeNO();
	if( GameParamInfo->GetWeapon(weapon_paramid, &ParamData) != 0 ){ return; }

	//Decide the size of the smoke with the masul flash
	if( ParamData.yakkyou_size >= 0.0f ){ yakkyousize = ParamData.yakkyou_size/10; }
	else{ yakkyousize = 0.012f * ParamData.attacks; }

	//Get weapon information
	weapon_paramid = HumanIndex[humanid].GetMainWeaponTypeNO();
	if( GameParamInfo->GetWeapon(weapon_paramid, &ParamData) != 0 ){ return; }

	//Determines rotational volume
	rx = rotation_x*-1 + (float)M_PI/2;

	//Determine shell-casing movement.
	mx = cosf(rx - (float)M_PI/2) * ParamData.yakkyou_sx * 0.12f;
	my = (ParamData.yakkyou_sy + (GetRand(3)-1)) * 0.12f;
	mz = sinf(rx - (float)M_PI/2) * ParamData.yakkyou_sx * 0.12f;

	//Specify the effect coordinates in the matrix
	WeaponAttachmentPosition(pos_x, pos_y + 16.0f, pos_z, ParamData.yakkyou_px/10, ParamData.yakkyou_py/10, ParamData.yakkyou_pz/10, rotation_x, armrotation_y*-1, &x, &y, &z);

	//Render shell casings.
	AddEffect(x, y, z, mx, my, mz, -0.15f, DegreeToRadian(10)*GetRand(36), DegreeToRadian(GetRand(61)-30), yakkyousize, 0.0f, Resource->GetEffectYakkyouTexture(), 0, 0, 1.0f, 0.0f, EFFECT_NORMAL, (int)(GAMEFPS*0.75f));
}

//! @brief Reload weapons
//! @param human_id Target number
//! @return success: true failure: false
bool ObjectManager::ReloadWeapon(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return false; }

	//Not processed if disabled
	if( HumanIndex[human_id].GetEnableFlag() == false ){ return false; }
	if( HumanIndex[human_id].GetHP() <= 0 ){ return false; }

	//Run Reload
	if( HumanIndex[human_id].ReloadWeapon() == true ){
		float x, y, z;
		int id;

		//es coordinates and team numbers of people
		HumanIndex[human_id].GetPosData(&x, &y, &z, NULL);
		HumanIndex[human_id].GetParamData(NULL, NULL, NULL, &id);
		y += 16.0f;

		//Place the sound source
		GameSound->ReloadWeapon(x, y, z, id);

		//Human_YakkyouCnt[human_id] = -1;

		return true;
	}

	return false;
}

//! @brief Switching weapons (changing)
//! @param human_id Number of people (0 to MAX_HUMAN-1)
//! @param id Weapon to switch to (-1 selects the next weapon).
void ObjectManager::ChangeHaveWeapon(int human_id, int id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return; }

	HumanIndex[human_id].ChangeHaveWeapon(id);
	Human_YakkyouCnt[human_id] = -1;
}

//! @brief Weapon Type Switching
//! @param human_id Number of people (0 to MAX_HUMAN-1)
//! @return Success:0 failure:1
int ObjectManager::ChangeWeaponID(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return 1; }

	//When you switch the type of weapon
	if( HumanIndex[human_id].ChangeWeaponID() == 0 ){
		int selectweapon;
		weapon *weapon[TOTAL_HAVEWEAPON];
		int WeaponID;
		int model, texture;
		float WeaponSize;

		//Get current weapons
		HumanIndex[human_id].GetWeapon(&selectweapon, weapon, NULL, NULL);
		weapon[selectweapon]->GetParamData(&WeaponID, NULL, NULL);

		//Get a model and texture
		if( Resource->GetWeaponModelTexture(WeaponID, &model, &texture) == 1 ){
			return 1;
		}

		//Get weapon size
		WeaponParameter ParamData;
		if( GameParamInfo->GetWeapon(WeaponID, &ParamData) != 0 ){ return 1; }
		WeaponSize = ParamData.size;

		//Change the model texture of the weapon object
		weapon[selectweapon]->SetModel(model, WeaponSize);
		weapon[selectweapon]->SetTexture(texture);

		Human_YakkyouCnt[human_id] = -1;

		return 0;
	}
	return 1;
}

//! @brief Discarding weapons
//! @param human_id Number of people (0 to MAX_HUMAN-1)
//! @return success: true failure: false
bool ObjectManager::DumpWeapon(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return false; }

	Human_YakkyouCnt[human_id] = -1;

	return HumanIndex[human_id].DumpWeapon();
}

//! @brief Switch scope mode
//! @param human_id Number of people (0 to MAX_HUMAN-1)
void ObjectManager::ChangeScopeMode(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return; }

	if( HumanIndex[human_id].GetScopeMode() == 0 ){	//Set scopes if not using scopes
		HumanIndex[human_id].SetEnableScope();
	}
	else{											//Disable when used
		HumanIndex[human_id].SetDisableScope();
	}

	//Human_YakkyouCnt[human_id] = -1;
}

//! @brief Cheat: add ammunition to carried weapons.
//! @param human_id Number of people (0 to MAX_HUMAN-1)
//! @return success: true failure: false
bool ObjectManager::CheatAddBullet(int human_id)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return false; }

	int selectweapon;
	weapon *weapon[TOTAL_HAVEWEAPON];
	int id_param, lnbs, nbs;
	WeaponParameter ParamData;

	//Get your own weapons
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		weapon[i] = NULL;
	}
	HumanIndex[human_id].GetWeapon(&selectweapon, weapon, NULL, NULL);

	//If you have any weapons
	if( weapon[selectweapon] != NULL ){
		//es weapon type, number of bullets and weapon settings
		weapon[selectweapon]->GetParamData(&id_param, &lnbs, &nbs);
		if( GameParamInfo->GetWeapon(id_param, &ParamData) == 0 ){
			//Maximum number of bullets added and applied
			nbs += ParamData.nbsmax;
			weapon[selectweapon]->ResetWeaponParam(Resource, id_param, lnbs, nbs);
			return true;
		}
	}

	return false;
}

//! @brief Cheat: change the carried weapon.
//! @param human_id Number of people (0 to MAX_HUMAN-1)
//! @param new_weaponID Type number of newly set weapons
//! @return success: true failure: false
bool ObjectManager::CheatNewWeapon(int human_id, int new_weaponID)
{
	//Check the value range
	if( (human_id < 0)||(MAX_HUMAN <= human_id) ){ return false; }
	if( (new_weaponID < 0)||(TOTAL_PARAMETERINFO_WEAPON <= new_weaponID) ){ return false; }

	human *myHuman = &(HumanIndex[human_id]);

	int selectweapon;
	weapon *weapon[TOTAL_HAVEWEAPON];
	int lnbslist[TOTAL_HAVEWEAPON];
	int nbslist[TOTAL_HAVEWEAPON];
	int lnbs, nbs;

	//Get your own weapons
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		weapon[i] = NULL;
		lnbslist[i] = 0;
		nbslist[i] = 0;
	}
	myHuman->GetWeapon(&selectweapon, weapon, lnbslist, nbslist);

	//If you do not own the weapon now, you must not have a new weapon
	if( (weapon[selectweapon] == NULL)&&(new_weaponID == ID_WEAPON_NONE) ){
		return false;		//Processing itself is meaningless
	}

	if( weapon[selectweapon] == NULL ){		//If you do not own weapons
		int dataid = -1;

		//Place new weapons
		dataid = AddVisualWeaponIndex(new_weaponID, false);

		//If the weapon can be placed
		if( dataid != -1 ){
			//Apply weapon settings (mainly number of bullets)
			weapon[selectweapon] = &(WeaponIndex[dataid]);
			weapon[selectweapon]->ResetWeaponParam(Resource, new_weaponID, lnbslist[selectweapon], nbslist[selectweapon]);

			//Initialization of the number of bullets when not in
			lnbslist[selectweapon] = 0;
			nbslist[selectweapon] = 0;

			//Weapon
			myHuman->SetWeapon(weapon, lnbslist, nbslist);

			Human_YakkyouCnt[human_id] = -1;
			return true;
		}
	}
	else{									//If you own a weapon
		//Get weapon settings (to inherit the number of bullets)
		weapon[selectweapon]->GetParamData(NULL, &lnbs, &nbs);

		if( new_weaponID == ID_WEAPON_NONE ){			//Removing weapons
			//Deleting the weapon
			weapon[selectweapon]->SetEnableFlag(false);

			//Remove weapons and draw bullets
			weapon[selectweapon] = NULL;
			lnbslist[selectweapon] = lnbs;
			nbslist[selectweapon] = nbs;
			myHuman->SetWeapon(weapon, lnbslist, nbslist);

			//Unlock scope mode
			myHuman->SetDisableScope();
		}
		else{											//If you want to change the weapon
			//Apply weapon settings
			weapon[selectweapon]->ResetWeaponParam(Resource, new_weaponID, lnbs, nbs);
		}

		Human_YakkyouCnt[human_id] = -1;
		return true;
	}

	return false;
}

//! @brief Check whether a zombie attack hits.
//! @param MyHuman Pointer to the attacking character object (the zombie).
//! @param EnemyHuman Pointer of an attacked person object
//! @return Satisfied: true; not satisfied: false.
//! @warning MyHuman may also refer to a non-zombie. <b>This function does not check whether it is a zombie</b>; verify the type beforehand.
//! @attention Only perform the test. Call HitZombieAttack() if it succeeds.
bool ObjectManager::CheckZombieAttack(human* MyHuman, human* EnemyHuman)
{
	if( MyHuman == NULL ){ return false; }
	if( EnemyHuman == NULL ){ return false; }

	//If not used or died, it will not be processed.
	if( MyHuman->GetEnableFlag() == false ){ return false; }
	if( MyHuman->GetHP() <= 0 ){ return false; }
	if( EnemyHuman->GetEnableFlag() == false ){ return false; }
	if( EnemyHuman->GetDeadFlag() == true ){ return false; }

	float mx, my, mz, mrx, tx, ty, tz;
	int mteam, tteam;
	float AttackPoint_x, AttackPoint_y, AttackPoint_z;
	float ax, az;

	MyHuman->GetPosData(&mx, &my, &mz, &mrx);
	MyHuman->GetParamData(NULL, NULL, NULL, &mteam);
	my += VIEW_HEIGHT;
	EnemyHuman->GetPosData(&tx, &ty, &tz, NULL);
	EnemyHuman->GetParamData(NULL, NULL, NULL, &tteam);
	ty += VIEW_HEIGHT;

	//If ally, it will not be processed.
	if( mteam == tteam ){ return false; }

	//Seeking Attack Point (Tip of the Arm)
	AttackPoint_x = mx + cosf(mrx*-1 + (float)M_PI/2) * 2.0f;
	AttackPoint_y = my + VIEW_HEIGHT - 0.5f;
	AttackPoint_z = mz + sinf(mrx*-1 + (float)M_PI/2) * 2.0f;
	ax = AttackPoint_x - tx;
	az = AttackPoint_z - tz;

	//If the enemy is affected by the attack point,
	if( (ax*ax + az*az) < 3.3f*3.3f ){
		if( (AttackPoint_y >= ty)&&(AttackPoint_y <= (ty + HUMAN_HEIGHT)) ){
			return true;
		}
	}

	return false;
}

//! @brief Handle a hit from a zombie attack.
//! @param MyHuman Pointer to the attacking character object (the zombie).
//! @param EnemyHuman Pointer of an attacked person object
void ObjectManager::HitZombieAttack(human* MyHuman, human* EnemyHuman)
{
	if( MyHuman == NULL ){ return; }
	if( EnemyHuman == NULL ){ return; }

	//If not used or died, it will not be processed.
	if( EnemyHuman->GetEnableFlag() == false ){ return; }
	if( EnemyHuman->GetDeadFlag() == true ){ return; }

	int MyHuman_dataID, MyHuman_TeamID;
	int EnemyHuman_dataID, EnemyHuman_TeamID;
	float mx, my, mz;
	float tx, ty, tz;
	float arx;
	int paramid;
	HumanParameter Paraminfo;
	bool NotRobot;
	int hp_old;

	//Get a team number of z es and attackers
	MyHuman->GetParamData(NULL, &MyHuman_dataID, NULL, &MyHuman_TeamID);
	EnemyHuman->GetParamData(NULL, &EnemyHuman_dataID, NULL, &EnemyHuman_TeamID);

	MyHuman->GetPosData(&mx, &my, &mz, NULL);
	EnemyHuman->GetPosData(&tx, &ty, &tz, NULL);
	ty += VIEW_HEIGHT;

	//Seeking the angles in which the enemy is attacked
	arx = atan2f(mz - tz, mx - tx);

	//Determining whether the robot is
	EnemyHuman->GetParamData(&paramid, NULL, NULL, NULL);
	GameParamInfo->GetHuman(paramid, &Paraminfo);
	if( Paraminfo.type == 1 ){
		NotRobot = false;
	}
	else{
		NotRobot = true;
	}

	//HP acquisition before damage calculation
	hp_old = EnemyHuman->GetHP();

	//Calculate damage and related values.
	EnemyHuman->HitZombieAttack();

#ifdef ENABLE_BUG_TEAMID
	bool flag = true;

	//If the team number is negative and the team number is large, flag is disabled.
	if( (EnemyHuman_TeamID < 0)&&(MyHuman_TeamID < 0) ){
		if( EnemyHuman_TeamID < MyHuman_TeamID ){
			flag = false;
		}
	}

	if( flag == true ){
		EnemyHuman->SetHitFlag(arx*-1 + (float)M_PI/2);
	}
#else
	EnemyHuman->SetHitFlag(arx*-1 + (float)M_PI/2);
#endif

	//Drawing effects (blood)
	SetHumanBlood(tx, ty, tz, HUMAN_DAMAGE_ZOMBIEU, NotRobot);

	//Play sound effects
	GameSound->HitHuman(tx, ty, tz, 3, MyHuman_TeamID);

	//Log-related processing
	if( (hp_old > 0)&&(EnemyHuman->GetHP() <= 0) ){
		//If hp <= 0 before the damage calculation and hp <= 0 after the calculation, this time I died.

		int player_teamid;
		HumanIndex[Player_HumanID].GetParamData(NULL, NULL, NULL, &player_teamid);

		ObjectLog->DiedHuman(MyHuman_dataID, EnemyHuman_dataID, MyHuman_TeamID, EnemyHuman_TeamID, player_teamid);
	}
}

//! @brief Reviving the Dead
//! @param id Number of people (0 to MAX_HUMAN-1)
//! @return true: success false: failure
//! @warning Revival with your hands
//! @attention This function fails if an invalid person number is specified, if a given person is alive, or if it is executed against a mystery.
bool ObjectManager::HumanResuscitation(int id)
{
	if( (id < 0)||(MAX_HUMAN <= id) ){ return false; }

	//Not treated if not used or survived.
	if( HumanIndex[id].GetEnableFlag() == false ){ return false; }
	if( HumanIndex[id].GetDeadFlag() == false ){ return false; }

	int id_param, dataid, team;
	signed short int p4;
	HumanIndex[id].GetParamData(&id_param, &dataid, &p4, &team);
	if( (id_param < 0)||( TOTAL_PARAMETERINFO_HUMAN <= id_param) ){ return false; }	//If you are a mystery, don't process it.
	HumanIndex[id].SetParamData(id_param, dataid, p4, team, true);

	//Log-related processing
	int player_teamid;
	HumanIndex[Player_HumanID].GetParamData(NULL, NULL, NULL, &player_teamid);
	ObjectLog->ReviveHuman(id, team, player_teamid);

	return true;
}

//! @brief Determination of game clear game overs
//! @return Mission complete: 1; game over: 2; no result: 0.
//! @attention In terms of game clearing and game over, it is judged as "game clear" as in the same way as the original XOPS.
int ObjectManager::CheckGameOverorComplete()
{
	//Notes:
	//
	// It is more efficient to perform game over judgment first, and then implement game clear judgment.
	// However, since XOPS prioritizes game clearing, we have determined the game clear first.
	// If the number of live enemies is counted (total HP), and the number of enemies (total HP) is 0, it is judged as game clear.

	//Game Clear Determination
	int MyTeamid, teamid;
	int TotalEnemyHP = 0;
	HumanIndex[Player_HumanID].GetParamData(NULL, NULL, NULL, &MyTeamid);	//Get player's team number
	for(int i=0; i<MAX_HUMAN; i++){
		//If not initialized, it will not be processed
		if( HumanIndex[i].GetEnableFlag() == false ){ continue; }

		//Get the team number you are looking for
		HumanIndex[i].GetParamData(NULL, NULL, NULL, &teamid);

		//If you have a different team number (= enemy)
		if( teamid != MyTeamid ){
			//If you live, add it as an enemy's HP
			//if( HumanIndex[i].GetDeadFlag() == false ){
			if( HumanIndex[i].GetHP() > 0 ){
				TotalEnemyHP += HumanIndex[i].GetHP();
			}
		}
	}
	if( TotalEnemyHP == 0 ){	//If the HP of all enemies is 0
		return 1;
	}


	//Game Over Judgment
	if( HumanIndex[Player_HumanID].GetEnableFlag() == true ){	//If the controlled character is valid (account for changes made through cheats).
		if( HumanIndex[Player_HumanID].GetDeadFlag() == true ){		//If the player died
			return 2;
		}
	}

	return 0;
}

//! @brief Get object information.
//! @param camera_x Camera X-coordinate
//! @param camera_y Camera Y coordinates
//! @param camera_z Camera Z coordinates
//! @param camera_rx Camera horizontal axis angle
//! @param camera_ry Camera vertical axis angle
//! @param color Pointer to get color
//! @param infostr Pointer to get characters
//! @return Display information available: true No display information: false
bool ObjectManager::GetObjectInfoTag(float camera_x, float camera_y, float camera_z, float camera_rx, float camera_ry, int *color, char *infostr)
{
	if( color == NULL ){ return false; }
	if( infostr == NULL ){ return false; }

	float dist = 50.0f;
	float px, py, pz;
	float rx, ry;
	float r;
	int Player_teamID;

	//Initialize characters
	infostr[0] = '\0';

	//Get player's team number
	HumanIndex[Player_HumanID].GetParamData(NULL, NULL, NULL, &Player_teamID);

	//Character.
	for(int i=0; i<MAX_HUMAN; i++){
		//Players do not process themselves
		if( i == Player_HumanID ){ continue; }

		if( HumanIndex[i].GetEnableFlag() == true ){
			HumanIndex[i].GetPosData(&px, &py, &pz, NULL);

			//Calculate the angle to the target relative to the viewpoint.
			if( CheckTargetAngle(camera_x, camera_y, camera_z, camera_rx, 0.0f, px, py, pz, dist, &rx, NULL, &r) == true ){
				int team;

				//On the angle, if you are in the vision
				if( fabsf(rx) < DegreeToRadian(10) ){
					HumanIndex[i].GetParamData(NULL, NULL, NULL, &team);
					if( team == Player_teamID ){
						*color = d3dg->GetColorCode(0.0f,0.0f,1.0f,1.0f);
						sprintf(infostr, "Human[%d]  HP %d : Friend", i, HumanIndex[i].GetHP());
					}
					else{
						*color = d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f);
						sprintf(infostr, "Human[%d]  HP %d : Enemy", i, HumanIndex[i].GetHP());
					}
					dist = sqrtf(r);
				}
			}
		}
	}

	//Weapons
	for(int i=0; i<MAX_WEAPON; i++){
		if( (WeaponIndex[i].GetEnableFlag() == true)&&(WeaponIndex[i].GetUsingFlag() == false) ){
			int lnbs, nbs;

			WeaponIndex[i].GetPosData(&px, &py, &pz, NULL);
			WeaponIndex[i].GetParamData(NULL, &lnbs, &nbs);

			//Calculate the angle to the target relative to the viewpoint.
			if( CheckTargetAngle(camera_x, camera_y, camera_z, camera_rx, camera_ry, px, py, pz, dist, &rx, &ry, &r) == true ){
				//On the angle, if you are in the vision
				if( (fabsf(rx) < DegreeToRadian(10))&&(fabsf(ry) < DegreeToRadian(10)) ){
					*color = d3dg->GetColorCode(0.0f,1.0f,0.0f,1.0f);
					sprintf(infostr, "Weapon[%d]  %d:%d", i, lnbs, (nbs - lnbs));
					dist = sqrtf(r);
				}
			}
		}
	}

	//Small object.
	for(int i=0; i<MAX_SMALLOBJECT; i++){
		if( SmallObjectIndex[i].GetEnableFlag() == true ){
			SmallObjectIndex[i].GetPosData(&px, &py, &pz, NULL);
			
			//Calculate the angle to the target relative to the viewpoint.
			if( CheckTargetAngle(camera_x, camera_y, camera_z, camera_rx, camera_ry, px, py, pz, dist, &rx, &ry, &r) == true ){
				//On the angle, if you are in the vision
				if( (fabsf(rx) < DegreeToRadian(10))&&(fabsf(ry) < DegreeToRadian(10)) ){
					*color = d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f);
					sprintf(infostr, "SmallObject[%d]  HP %d", i, SmallObjectIndex[i].GetHP());
					dist = sqrtf(r);
				}
			}
		}
	}

	//Determines whether the character is set or not, and whether it has been processed.
	if( infostr[0] == NULL ){
		return false;
	}
	return true;
}

//! @brief Main calculation processing of objects
//! @param cmdF5id Person data number to increase function (F5 backing) (function in -1)
//! @param demomode Demo mode
//! @param screen Dark screen
//! @return Always 0
//! @attention Normally, set cmdF5id to the player ID (from GetPlayerID()) while using the F5 cheat, or -1 otherwise.
//! @attention demomode is mainly used on the menu screen. When enabled, it does not process bullets and grenades.
int ObjectManager::Process(int cmdF5id, bool demomode, bool screen)
{
	//Initialize the history of this frame
	for(int i=0; i<MAX_HUMAN; i++){
		Human_ontarget[i] = 0.0f;
		Human_kill[i] = 0;
		Human_headshot[i] = 0;
	}

	//Processing human objects
	for(int i=0; i<MAX_HUMAN; i++){
		bool cmdF5, player;
		int rtn;
		int block_id = -1;
		int block_face = -1;
		int block_textureid = -1;
		int MoveMode;
		int JumpLandingMode;

		//Determining whether the player is
		if( Player_HumanID == i ){
			player = true;
		}
		else{
			player = false;
		}

		if( i == cmdF5id ){
			cmdF5 = true;
		}
		else{
			cmdF5 = false;
		}

		rtn = HumanIndex[i].ProcessObject(CollD, BlockData, AddCollisionFlag, player, cmdF5);
		if( rtn == 2 ){
			//Death Effects
			DeadEffect(&(HumanIndex[i]));
		}
		if( rtn == 4 ){
			int teamid, player_teamid;
			HumanIndex[i].GetParamData(NULL, NULL, NULL, &teamid);
			HumanIndex[Player_HumanID].GetParamData(NULL, NULL, NULL, &player_teamid);

			//Log-related processing
			ObjectLog->DiedHuman(-1, i, -1, teamid, player_teamid);
		}

		HumanIndex[i].SetDarkModelFlag(screen);

		//Get the texture number of the block at the foot
		HumanIndex[i].GetUnderBlock(&block_id, &block_face);
		if( block_id != -1 ){
			blockdata bdata;
			BlockData->Getdata(&bdata, block_id);
			block_textureid = bdata.material[block_face].textureID;
		}
		else{
			block_textureid = -1;
		}

		MoveMode = HumanIndex[i].GetMovemode(false);
		JumpLandingMode = HumanIndex[i].GetJumpLanding(false);

		//Footsteps.
		if( (1 <= MoveMode)&&(MoveMode <= 4) ){
			//Add footsteps
			// *The texture number of the block at the foot is the type number of the foot.
			float posx, posy, posz;
			int teamid;
			int cnt;
			HumanIndex[i].GetPosData(&posx, &posy, &posz, NULL);
			HumanIndex[i].GetParamData(NULL, NULL, NULL, &teamid);
			cnt = HumanIndex[i].GetMoveMotionCount();
			GameSound->SetFootsteps(posx, posy, posz, block_textureid, MoveMode - 1, teamid, cnt);
		}

		//Sound to jump
		if( JumpLandingMode == 1 ){
			//Add footsteps
			float posx, posy, posz;
			int teamid;
			HumanIndex[i].GetPosData(&posx, &posy, &posz, NULL);
			HumanIndex[i].GetParamData(NULL, NULL, NULL, &teamid);
			GameSound->SetFootsteps(posx, posy, posz, block_textureid, 4, teamid, 0);
		}

		//Landing sound.
		if( JumpLandingMode == 2 ){
			//Add footsteps
			float posx, posy, posz;
			int teamid;
			HumanIndex[i].GetPosData(&posx, &posy, &posz, NULL);
			HumanIndex[i].GetParamData(NULL, NULL, NULL, &teamid);
			GameSound->SetFootsteps(posx, posy, posz, block_textureid, 5, teamid, 0);
		}
	}

	//Handling weapon objects
	for(int i=0; i<MAX_WEAPON; i++){
		WeaponIndex[i].ProcessObject(CollD);
		WeaponIndex[i].SetDarkModelFlag(screen);
	}

	//Processing small objects
	for(int i=0; i<MAX_SMALLOBJECT; i++){
		SmallObjectIndex[i].ProcessObject();
		SmallObjectIndex[i].SetDarkModelFlag(screen);
	}

	if( demomode == false ){
		//Processing of bullet objects
		for(int i=0; i<MAX_BULLET; i++){
			float bx, by, bz, brx, bry, speed;
			int teamid;
			float mx, my, mz;

			CollideBullet(&BulletIndex[i]);		//Run collision detection.
			BulletIndex[i].ProcessObject();		//Main calculation.
			BulletIndex[i].SetDarkModelFlag(screen);

			if( BulletIndex[i].GetEnableFlag() == true ){
				//Get the coordinates and angles of the bullet
				BulletIndex[i].GetParamData(NULL, NULL, &speed, &teamid, NULL, NULL);
				BulletIndex[i].GetPosData(&bx, &by, &bz, &brx, &bry);
				mx = cosf(brx)*cosf(bry)*speed;
				my = sinf(bry)*speed;
				mz = sinf(brx)*cosf(bry)*speed;
				GameSound->PassingBullet(bx, by, bz, mx, my, mz, teamid);
			}
		}
	}
	else{
		//Cancel all bullet objects
		for(int i=0; i<MAX_BULLET; i++){
			if( BulletIndex[i].GetEnableFlag() == true ){
				BulletIndex[i].SetEnableFlag(false);
			}
		}
	}

	//Causes effects when weapons are fired
	for(int i=0; i<MAX_HUMAN; i++){
		if( Human_ShotFlag[i] == true ){
			ShotWeaponEffect(i);
			Human_ShotFlag[i] = false;
		}

		if( Human_YakkyouCnt[i] == 0 ){
			ShotWeaponYakkyou(i);
		}
		if( Human_YakkyouCnt[i] > -1 ){ Human_YakkyouCnt[i] -= 1; }
	}

	//Processing effects objects
	for(int i=0; i<MAX_EFFECT; i++){
		int id, face;
		float pos_x, pos_y, pos_z;
		if( CollideBlood(&(EffectIndex[i]), &id, &face, &pos_x, &pos_y, &pos_z) == true ){
			AddMapEffect(id, face, pos_x, pos_y, pos_z, 10.0f, DegreeToRadian(10)*GetRand(18), Resource->GetEffectBloodTexture(),  0, 0, 1.0f, -0.002f, (int)GAMEFPS * 20);
			EffectIndex[i].SetEnableFlag(false);
		}
		else{
			EffectIndex[i].ProcessObject();
			EffectIndex[i].SetDarkModelFlag(screen);
		}
	}

	if( demomode == false ){
		//Hand grenades processing
		for(int i=0; i<MAX_GRENADE; i++){
			float speed = GrenadeIndex[i].GetSpeed();

			//Main calculation.
			int rcr = GrenadeIndex[i].ProcessObject(CollD);
			GrenadeIndex[i].SetDarkModelFlag(screen);

			//Bound bounce back
			if( rcr == 1 ){
				if( speed > 3.4f ){
					//Get coordinates and play sound effects
					float x, y, z;
					int teamid;
					GrenadeIndex[i].GetPosData(&x, &y, &z, NULL, NULL);
					GrenadeIndex[i].GetParamData(NULL, &teamid, NULL, NULL);
					GameSound->GrenadeBound(x, y, z, teamid);
				}
			}

			//If it exploded.
			if( rcr == 2 ){
				GrenadeExplosion(&(GrenadeIndex[i]));
			}
		}
	}
	else{
		//Remove all grenades
		for(int i=0; i<MAX_GRENADE; i++){
			if( GrenadeIndex[i].GetEnableFlag() == true ){
				GrenadeIndex[i].SetEnableFlag(false);
			}
		}
	}


	//Pick up weapons
	for(int i=0; i<MAX_HUMAN; i++){
		if( HumanIndex[i].GetEnableFlag() == true ){
			//Weapon pickup does not need a full scan every frame, so process each weapon once every two frames.
			//On even frames, process weapons with even data IDs; on odd frames, process weapons with odd IDs.
			for(int j=(framecnt%2); j<MAX_WEAPON; j+=2){
				PickupWeapon(&HumanIndex[i], &WeaponIndex[j]);
			}
		}
	}

	for(int i=0; i<MAX_HUMAN; i++){
		if( HumanIndex[i].GetEnableFlag() == true ){
			for(int j=i+1; j<MAX_HUMAN; j++){
				if( HumanIndex[j].GetEnableFlag() == true ){

					//Character-character collision detection (pushing).
					CollideHuman(&HumanIndex[i], &HumanIndex[j]);

				}
			}
		}
	}

	//Log-related processing
	ObjectLog->Process();

	framecnt += 1;

	return 0;
}

//! @brief Acquisition of the current frame, the number of defeated enemies, and the number of hit shots
//! @param id Data number of the person to acquire
//! @param ontarget Pointer to receive the mid-live count
//! @param kill Pointer to receive the number of defeated enemies
//! @param headshot Pointer to receive the number of lives in the enemy head
//! @return success: true failure: false
bool ObjectManager::GetHumanShotInfo(int id, float *ontarget, int *kill, int *headshot)
{
	if( (id < 0)||(MAX_HUMAN <= id) ){ return false; }
	if( (ontarget == NULL)||(kill == NULL)||(headshot == NULL) ){ return false; }

	*ontarget = Human_ontarget[id];
	*kill = Human_kill[id];
	*headshot = Human_headshot[id];
	return true;
}

//! @brief Sort effects.
//! @param camera_x Camera X-coordinate
//! @param camera_y Camera Y coordinates
//! @param camera_z Camera Z coordinates
//! @param data Array of effectdata structures receiving results (number of elements: MAX_EFFECT).
//! @return Number of effective effects
int ObjectManager::SortEffect(float camera_x, float camera_y, float camera_z, effectdata data[])
{
	int cnt = 0;

	//Count the total number while searching for distance from the camera.
	for(int i=0; i<MAX_EFFECT; i++){
		if( EffectIndex[i].GetEnableFlag() == true ){
			float ex, ey, ez;
			float x, y, z;

			//Get coordinates, register distances, and register data.
			EffectIndex[i].GetPosData(&ex, &ey, &ez, NULL);
			x = ex - camera_x;
			y = ey - camera_y;
			z = ez - camera_z;
			data[cnt].id = i;
			data[cnt].dist = x*x + y*y + z*z;

			//Add count
			cnt += 1;
		}
	}

	//Simple Insert
	effectdata temp;
	for(int i=1; i<cnt; i++) {
		int j;
		temp = data[i];
		for(j=i; j>0 && data[j-1].dist<temp.dist; j--){
			data[j] = data[j-1];
		}
		data[j] = temp;
	}

	return cnt;
}

//! @brief Object drawing processing
//! @param camera_x Camera X-coordinate
//! @param camera_y Camera Y coordinates
//! @param camera_z Camera Z coordinates
//! @param camera_rx Camera horizontal axis angle
//! @param camera_ry Camera vertical axis angle
//! @param HidePlayer Player rendering mode (render: 0; hide: 1; render only arms and weapons: 2).
//! @param NoModel Disable model drawing
void ObjectManager::CaptureRenderState(){
    for(int i=0;i<MAX_HUMAN;i++)HumanIndex[i].CaptureRenderState();
    for(int i=0;i<MAX_WEAPON;i++)WeaponIndex[i].CaptureRenderState();
    for(int i=0;i<MAX_SMALLOBJECT;i++)SmallObjectIndex[i].CaptureRenderState();
    for(int i=0;i<MAX_BULLET;i++)BulletIndex[i].CaptureRenderState();
    for(int i=0;i<MAX_GRENADE;i++)GrenadeIndex[i].CaptureRenderState();
    for(int i=0;i<MAX_EFFECT;i++)EffectIndex[i].CaptureRenderState();
}

void ObjectManager::Render(float camera_x, float camera_y, float camera_z, float camera_rx, float camera_ry, int HidePlayer, bool NoModel)
{
    for(int i=0;i<MAX_HUMAN;i++)HumanIndex[i].ApplyRenderState();
    for(int i=0;i<MAX_WEAPON;i++)WeaponIndex[i].ApplyRenderState();
    for(int i=0;i<MAX_SMALLOBJECT;i++)SmallObjectIndex[i].ApplyRenderState();
    for(int i=0;i<MAX_BULLET;i++)BulletIndex[i].ApplyRenderState();
    for(int i=0;i<MAX_GRENADE;i++)GrenadeIndex[i].ApplyRenderState();
    for(int i=0;i<MAX_EFFECT;i++)EffectIndex[i].ApplyRenderState();

	//World Coordinates
	d3dg->ResetWorldTransform();

	//People Drawing
	for(int i=0; i<MAX_HUMAN; i++){
		bool DrawArm, player;

		//If a person's texture refresh flag is enabled
		if( Human_FrameTextureRefresh[i] == true ){
			if( i > 0 ){
				//Get the weapons that the former has
				int WeaponType = HumanIndex[i-1].GetMainWeaponTypeNO();

				if( WeaponType != ID_WEAPON_NONE ){
					int model, texture;

					//Apply the texture of the weapon if you have a weapon
					Resource->GetWeaponModelTexture(WeaponType, &model, &texture);
					HumanIndex[i].SetTexture(texture);
				}
				else{
					//If you are a hand, reflect the texture of the person as it is
					HumanIndex[i].SetTexture( HumanIndex[i-1].GetTexture() );
				}
			}
		}

		//Arm Drawing
		if( HidePlayer == 0 ){
			DrawArm = false;
		}
		else if( Player_HumanID != i ){	// HidePlayer != 0
			DrawArm = false;
		}
		else if( HidePlayer == 2 ){		// Player_HumanID == i
			DrawArm = true;
		}
		else{							// Player_HumanID == i && HidePlayer == 1
			continue;	//Skip if you don't draw the player itself
		}

		//Determining whether the player is
		if( Player_HumanID == i ){
			player = true;
		}
		else{
			player = false;
		}

		HumanIndex[i].Render(d3dg, Resource, DrawArm, player, NoModel);

		/*
		//Display simplified collision volumes.
		float x, y, z;
		HumanIndex[i].GetPosData(&x, &y, &z, NULL);
		if( HumanIndex[i].GetHP() > 0 ){
			d3dg->SetWorldTransform(x, y, z, 0.0f, 0.0f, 1.0f);
			d3dg->Renderline(HUMAN_BULLETCOLLISION_HEAD_R, HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H + HUMAN_BULLETCOLLISION_HEAD_H, 0.0f, HUMAN_BULLETCOLLISION_HEAD_R * -1, HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H + HUMAN_BULLETCOLLISION_HEAD_H, 0.0f);
			d3dg->Renderline(0.0f, HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H + HUMAN_BULLETCOLLISION_HEAD_H, HUMAN_BULLETCOLLISION_HEAD_R, 0.0f, HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H + HUMAN_BULLETCOLLISION_HEAD_H, HUMAN_BULLETCOLLISION_HEAD_R * -1);
			d3dg->Renderline(HUMAN_BULLETCOLLISION_UP_R, HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H, 0.0f, HUMAN_BULLETCOLLISION_UP_R * -1, HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H, 0.0f);
			d3dg->Renderline(0.0f, HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H, HUMAN_BULLETCOLLISION_UP_R, 0.0f, HUMAN_BULLETCOLLISION_LEG_H + HUMAN_BULLETCOLLISION_UP_H, HUMAN_BULLETCOLLISION_UP_R * -1);
			d3dg->Renderline(HUMAN_BULLETCOLLISION_LEG_R, HUMAN_BULLETCOLLISION_LEG_H, 0.0f, HUMAN_BULLETCOLLISION_LEG_R * -1, HUMAN_BULLETCOLLISION_LEG_H, 0.0f);
			d3dg->Renderline(0.0f, HUMAN_BULLETCOLLISION_LEG_H, HUMAN_BULLETCOLLISION_LEG_R, 0.0f, HUMAN_BULLETCOLLISION_LEG_H, HUMAN_BULLETCOLLISION_LEG_R * -1);
		}
		*/
	}

	//Weapon Drawing
	for(int i=0; i<MAX_WEAPON; i++){
		WeaponIndex[i].Render(d3dg, NoModel);
	}

	//Drawing Accessories
	for(int i=0; i<MAX_SMALLOBJECT; i++){
		SmallObjectIndex[i].Render(d3dg, NoModel);
	}

	//Bullet drawing
	for(int i=0; i<MAX_BULLET; i++){
		BulletIndex[i].Render(d3dg, NoModel);
	}

	//Hand grenades drawing
	for(int i=0; i<MAX_GRENADE; i++){
		GrenadeIndex[i].Render(d3dg, NoModel);
	}

	//Effect drawing
	d3dg->StartEffectRender();
	/*
	for(int i=0; i<MAX_EFFECT; i++){
		EffectIndex[i].Render(d3dg, camera_rx, camera_ry, NoModel);
	}
	*/
	effectdata data[MAX_EFFECT];
	int cnt = SortEffect(camera_x, camera_y, camera_z, data);
	for(int i=0; i<cnt; i++) {
		EffectIndex[ data[i].id ].Render(d3dg, camera_rx, camera_ry, NoModel);
	}
	d3dg->EndEffectRender();
    for(int i=0;i<MAX_HUMAN;i++)HumanIndex[i].RestoreRenderState();
    for(int i=0;i<MAX_WEAPON;i++)WeaponIndex[i].RestoreRenderState();
    for(int i=0;i<MAX_SMALLOBJECT;i++)SmallObjectIndex[i].RestoreRenderState();
    for(int i=0;i<MAX_BULLET;i++)BulletIndex[i].RestoreRenderState();
    for(int i=0;i<MAX_GRENADE;i++)GrenadeIndex[i].RestoreRenderState();
    for(int i=0;i<MAX_EFFECT;i++)EffectIndex[i].RestoreRenderState();
}

//! @brief Drawing Process of Object Log
//! @param x Top left x coordinate
//! @param y Upper left y coordinates
void ObjectManager::RenderLog(int x, int y)
{
	ObjectLog->Render(x, y);
}

//! @brief Data Release
void ObjectManager::Cleanup()
{
	//Point Data Release
	CleanupPointDataToObject();
}

//! @brief Constructor
BulletObjectHumanIndex::BulletObjectHumanIndex()
{
	HumanIndex = new bool[MAX_HUMAN];

	Init();
}

//! @brief Destructor
BulletObjectHumanIndex::~BulletObjectHumanIndex()
{
	if( HumanIndex != NULL ){ delete [] HumanIndex; }
}

//! @brief Initialize
void BulletObjectHumanIndex::Init()
{
	for(int i=0; i<MAX_HUMAN; i++){
		HumanIndex[i] = false;
	}
}

//! @brief Get flags
//! @param id Data number of people
//! @return flag value
bool BulletObjectHumanIndex::GetIndexFlag(int id)
{
	if( (id < 0)||(MAX_HUMAN <= id) ){ return false; }
	return HumanIndex[id];
}

//! @brief Enable flags
//! @param id Data number of people
void BulletObjectHumanIndex::SetIndexFlag(int id)
{
	if( (id < 0)||(MAX_HUMAN <= id) ){ return; }
	HumanIndex[id] = true;
}

//! @brief Constructor
ObjectManagerLog::ObjectManagerLog()
{
	d3dg = NULL;
	for(int i=0; i<MAX_OBJECTMANAGER_LOGLEN; i++){
		TextStr[i] = new char [MAX_OBJECTMANAGER_LOGLINES];
	}
	ClearLog();
}

//! @brief Destructor
ObjectManagerLog::~ObjectManagerLog()
{
	for(int i=0; i<MAX_OBJECTMANAGER_LOGLEN; i++){
		delete [] TextStr[i];
	}
}

//! @brief Set the class you want to see
//! @param in_d3dg Drawing processing class
//! @attention If you do not set this function, the class itself will not function correctly.
void ObjectManagerLog::SetClass(D3DGraphics *in_d3dg)
{
	d3dg = in_d3dg;
}

//! @brief Initialize object
void ObjectManagerLog::ClearLog()
{
	for(int i=0; i<MAX_OBJECTMANAGER_LOGLEN; i++){
		TextCnt[i] = -1;
		TextStr[i][0] = '\0';
		TextColor[i] = 0;
	}
}

//! @brief Add information
//! @param str String (return code: <b> non</b>)
void ObjectManagerLog::InfoLog(const char *str)
{
	if( str == NULL ){ return; }
	AddTextLog(MAX_OBJECTMANAGER_LOGCNT, str, d3dg->GetColorCode(0.0f,1.0f,0.0f,1.0f));
}

//! @brief Add people
//! @param humanID Person ID
//! @param TeamID Team number
//! @param PlayerTeamID Player's Team Number
void ObjectManagerLog::AddHuman(int humanID, int TeamID, int PlayerTeamID)
{
	char str[64];
	int str_color;

	//Determines the color of the character
	if( PlayerTeamID == TeamID ){ str_color = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f); }
	else{ str_color = d3dg->GetColorCode(1.0f,0.5f,0.0f,1.0f); }

	sprintf(str, "Added human[%d]", humanID);

	AddTextLog(MAX_OBJECTMANAGER_LOGCNT, str, str_color);
}

//! @brief A person died
//! @param ShothumanID  ID of the attacker
//! @param HitHumanID ID of the character that was attacked.
//! @param ShothumanTeamID Team number of attackers
//! @param HitHumanTeamID Team number of attackers
//! @param PlayerTeamID Player's Team Number
void ObjectManagerLog::DiedHuman(int ShothumanID, int HitHumanID, int ShothumanTeamID, int HitHumanTeamID, int PlayerTeamID)
{
	char str[64];
	int str_color;

	//Determines the color of the character
	if( PlayerTeamID == HitHumanTeamID ){ str_color = d3dg->GetColorCode(1.0f,0.5f,0.0f,1.0f); }
	else{ str_color = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f); }

	if( (ShothumanID == -1)||(ShothumanID == HitHumanID) ){
		sprintf(str, "human[%d] died", HitHumanID);
	}
	else{
		sprintf(str, "human[%d] killed human[%d]", ShothumanID, HitHumanID);

		//TK Test
		if( ShothumanTeamID == HitHumanTeamID ){
			strcat(str, " (TK)");
		}
	}

	AddTextLog(MAX_OBJECTMANAGER_LOGCNT, str, str_color);
}

//! @brief People revived
//! @param humanID Person ID
//! @param TeamID Team number
//! @param PlayerTeamID Player's Team Number
void ObjectManagerLog::ReviveHuman(int humanID, int TeamID, int PlayerTeamID)
{
	char str[64];
	int str_color;

	//Determines the color of the character
	if( PlayerTeamID == TeamID ){ str_color = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f); }
	else{ str_color = d3dg->GetColorCode(1.0f,0.5f,0.0f,1.0f); }

	sprintf(str, "Revived human[%d]", humanID);

	AddTextLog(MAX_OBJECTMANAGER_LOGCNT, str, str_color);
}

//! @brief Add small items
//! @param objID ID of small items
void ObjectManagerLog::AddSmallObject(int objID)
{
	char str[64];
	sprintf(str, "Added SmallObject[%d]", objID);
	AddTextLog(MAX_OBJECTMANAGER_LOGCNT, str, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
}

//! @brief A small object was destroyed.
//! @param objID ID of small items
void ObjectManagerLog::BreakSmallObject(int objID)
{
	char str[64];
	sprintf(str, "Breaked SmallObject[%d]", objID);
	AddTextLog(MAX_OBJECTMANAGER_LOGCNT, str, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
}

//! @brief Add object log
//! @param cnt Number of drawing frames
//! @param str String (return code: <b> non</b>)
//! @param color Color:
//! @return 1 line overwrite: true Add only: false
bool ObjectManagerLog::AddTextLog(int cnt, const char *str, int color)
{
	if( str == NULL ){ return false; }

	//If there is an empty line, write it on that line and finish
	for(int i=0; i<MAX_OBJECTMANAGER_LOGLEN; i++){
		if( TextCnt[i] == -1 ){
			TextCnt[i] = cnt;
			strcpy(TextStr[i], str);
			TextColor[i] = color;
			return false;
		}
	}

	//If there is no empty line, it is packed by one line.
	for(int i=0; i<MAX_OBJECTMANAGER_LOGLEN-1; i++){
		TextCnt[i] = TextCnt[i+1];
		strcpy(TextStr[i], TextStr[i+1]);
		TextColor[i] = TextColor[i+1];
	}

	//Last line
	TextCnt[MAX_OBJECTMANAGER_LOGLEN-1] = cnt;
	strcpy(TextStr[MAX_OBJECTMANAGER_LOGLEN-1], str);
	TextColor[MAX_OBJECTMANAGER_LOGLEN-1] = color;
	return true;
}

//! @brief Main calculation processing of object log
void ObjectManagerLog::Process()
{
	//Count processing for each line
	for(int i=0; i<MAX_OBJECTMANAGER_LOGLEN; i++){
		if( TextCnt[i] == 0 ){
			TextCnt[i] = -1;
			TextStr[i][0] = '\0';
			TextColor[i] = 0;
		}
		else if( TextCnt[i] > 0 ){
			TextCnt[i] -= 1;
		}
	}

	//Zero counting lines
	for(int i=0; i<MAX_OBJECTMANAGER_LOGLEN-1; i++){
		if( TextCnt[i] == 0 ){
			for(int j=i; j<MAX_OBJECTMANAGER_LOGLEN-1; j++){
				TextCnt[j] = TextCnt[j+1];
				strcpy(TextStr[j], TextStr[j+1]);
				TextColor[j] = TextColor[j+1];
			}

			//The last line is empty
			TextCnt[MAX_OBJECTMANAGER_LOGLEN-1] = -1;
			TextStr[MAX_OBJECTMANAGER_LOGLEN-1][0] = '\0';
			TextColor[MAX_OBJECTMANAGER_LOGLEN-1] = 0;
		}
	}
}

//! @brief Drawing Process of Object Log
//! @param x Top left x coordinate
//! @param y Upper left y coordinates
//! @attention From the top to the reverse order.
void ObjectManagerLog::Render(int x, int y)
{
#ifdef ENABLE_DEBUGCONSOLE
	if( d3dg == NULL ){ return; }

	for(int i=MAX_OBJECTMANAGER_LOGLEN-1; i>=0; i--){
		if( TextCnt[i] >= 0 ){
			d3dg->Draw2DTextureDebugFontText(x+2+1, y+2+1, TextStr[i], d3dg->GetColorCode(0.1f,0.1f,0.1f,1.0f));
			d3dg->Draw2DTextureDebugFontText(x+2, y+2, TextStr[i], TextColor[i]);
		}
		y += 20;
	}
#endif
}