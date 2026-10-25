//! @file object.cpp
//! @brief Definition of object class

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

#include "object.h"

void object::CaptureRenderState(){
    float pose[]={pos_x,pos_y,pos_z,rotation_x};
    renderHistory.Capture(pose); if(!EnableFlag)renderHistory.valid=false;
}
void object::ApplyRenderState(){
    renderSaved[0]=pos_x;renderSaved[1]=pos_y;renderSaved[2]=pos_z;renderSaved[3]=rotation_x;
    float pose[]={pos_x,pos_y,pos_z,rotation_x};
    if(EnableFlag)renderHistory.Blend(pose,RenderInterpolationAlpha);
    pos_x=pose[0];pos_y=pose[1];pos_z=pose[2];rotation_x=pose[3];renderPoseApplied=true;
}
void object::RestoreRenderState(){
    if(!renderPoseApplied)return;
    pos_x=renderSaved[0];pos_y=renderSaved[1];pos_z=renderSaved[2];rotation_x=renderSaved[3];renderPoseApplied=false;
}

//! @brief Constructor
object::object(class ParameterInfo *in_Param, float x, float y, float z, float rx, float size, bool flag)
{
	Param = in_Param;
	pos_x = x;
	pos_y = y;
	pos_z = z;
	rotation_x = rx;
	EnableFlag = flag;
	model_size = size;

	id_parameter = 0;
	id_model = -1;
	id_texture = -1;
	DarkModelFlag = false;
}

//! @brief Destructor
object::~object()
{}

//! @brief Register a class that manages the setting value
//! @attention to execute before using each function.
void object::SetParameterInfoClass(class ParameterInfo *in_Param)
{
	Param = in_Param;
}

//! @brief Set coordinates and angles
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param rx Horizontal rotation.
void object::SetPosData(float x, float y, float z, float rx)
{
	pos_x = x;
	pos_y = y;
	pos_z = z;
	rotation_x = rx;
}

//! @brief Get coordinates and angles
//! @param x Pointer to receive X coordinates (NULL allowed)
//! @param y Pointer to receive Y coordinates (NULL allowed)
//! @param z Pointer to receive Z coordinates (NULL allowed)
//! @param rx Pointer to receive horizontal axis rotation (NULL allowed)
void object::GetPosData(float *x, float *y, float *z, float *rx)
{
	if( x != NULL ){ *x = pos_x; }
	if( y != NULL ){ *y = pos_y; }
	if( z != NULL ){ *z = pos_z; }
	if( rx != NULL ){ *rx = rotation_x; }
}

//! @brief Configure the enabled flag
//! @param flag Flag to set
void object::SetEnableFlag(bool flag)
{
    if(flag!=EnableFlag)renderHistory.valid=false;
	EnableFlag = flag;
}

//! @brief ate flags
//! @return Current flags
bool object::GetEnableFlag()
{
	return EnableFlag;
}

//! @brief Configure model data
//! @param id Model Data Number
//! @param size Drawing Magnification
void object::SetModel(int id, float size)
{
	id_model = id;
	model_size = size;
}

//! @brief Get model data
//! @param id Pointer to receive the data number of the model (NULL allowed)
//! @param size Pointer to receive draw magnification (NULL allowed)
void object::GetModel(int *id, float *size)
{
	if( id != NULL ){ *id = id_model; }
	if( size != NULL ){ *size = model_size; }
}

//! @brief Set Texture
//! @param id Texture Data Number
void object::SetTexture(int id)
{
	id_texture = id;
}

//! @brief Get Texture
//! @return Texture Data Number
int object::GetTexture()
{
	return id_texture;
}

//! @brief Set the flag to darken the model
//! @param flag Flags
void object::SetDarkModelFlag(bool flag)
{
	DarkModelFlag = flag;
}

//! @brief ute calculation (such as free fall)
int object::ProcessObject()
{
	return 0;
}

//! @brief Render
//! @param d3dg Pointer to D3DGraphics.
//! @param NoModel Disable model drawing
void object::Render(D3DGraphics *d3dg, bool NoModel)
{
	if( d3dg == NULL ){ return; }
	if( EnableFlag == false ){ return; }

	d3dg->SetWorldTransform(pos_x, pos_y, pos_z, rotation_x, 0.0f, model_size);
	d3dg->RenderModel(id_model, id_texture, DarkModelFlag, false, NoModel);
}

//! @brief Constructor
human::human(class ParameterInfo *in_Param, float x, float y, float z, float rx, int id_param, int dataid, signed short int p4, int team, bool flag)
{
	//HumanParameter data;

	Param = in_Param;
	pos_x = x;
	pos_y = y;
	pos_z = z;
	move_x = 0.0f;
	move_y = 0.0f;
	move_z = 0.0f;
	move_y_flag = false;
	move_y_upper = 0;
	move_y_landing = false;
	underblock_id = -1;
	underblock_face = -1;
	rotation_x = rx;
	id_parameter = id_param;
	upmodel_size = 9.4f;
	armmodel_size = 9.0f;
	legmodel_size = 9.0f;
	EnableFlag = flag;
	rotation_y = 0.0f;
	armrotation_y = 0.0f;
	point_dataid = dataid;
	point_p4 = p4;
	teamid = team;

	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		weapon[i] = NULL;
		noweapon_bullets[i] = 0;
		noweapon_loadbullets[i] = 0;
	}
	selectweapon = 1;
	selectweaponcnt = 0;
	changeweaponidcnt = 0;
	weaponshotcnt = 0;
	weaponreloadcnt = 0;
	weaponshotframe = false;
	weaponburstmodecnt = 0;
	//if( Param->GetHuman(id_param, &data) == 0 ){
	//	hp = data.hp;
	//}
	//else{
		hp = 0;
	//}
	deadstate = 0;
	add_ry = 0.0f;
	id_texture = -1;

	move_rx = 0.0f;
	MoveFlag = 0x00;
	MoveFlag_lt = MoveFlag;
	scopemode = 0;
	HitFlag = false;
	Hit_rx = 0.0f;
	totalmove = 0.0f;
	StateGunsightErrorRange = 0;
	ReactionGunsightErrorRange = 0;
	Invincible = false;

	MotionCtrl = new HumanMotionControl;
}

//! @brief Destructor
human::~human()
{
	delete MotionCtrl;
}

//! @brief Register a class that manages the setting value
//! @attention to execute before using each function.
void human::SetParameterInfoClass(class ParameterInfo *in_Param)
{
	Param = in_Param;

	MotionCtrl->SetParameterInfoClass(in_Param);
}

//! @brief Set the setting value
//! @param id_param Character type ID.
//! @param dataid Point Data Number
//! @param p4 4 Parameters
//! @param team Team number
//! @param init Initialize objects
//! @warning To initialize the init argument to true, set the angle with the SetPosData() function first.
void human::SetParamData(int id_param, int dataid, signed short int p4, int team, bool init)
{
	id_parameter = id_param;
	point_dataid = dataid;
	point_p4 = p4;
	teamid = team;

	if( init == true ){
		HumanParameter data;

		move_x = 0.0f;
		move_y = 0.0f;
		move_z = 0.0f;
		move_y_flag = false;
		move_y_upper = 0;
		move_y_landing = false;
		underblock_id = -1;
		underblock_face = -1;
		rotation_y = 0.0f;
		armrotation_y = DegreeToRadian(-30);

		for(int i=0; i<TOTAL_HAVEWEAPON; i++){
			weapon[i] = NULL;
			noweapon_bullets[i] = 0;
			noweapon_loadbullets[i] = 0;
		}
		selectweapon = 1;
		selectweaponcnt = 0;
		changeweaponidcnt = 0;
		weaponshotcnt = 0;
		weaponreloadcnt = 0;
		weaponshotframe = false;
		weaponburstmodecnt = 0;
		if( Param->GetHuman(id_param, &data) == 0 ){
			hp = data.hp;
		}
		else{
			hp = 0;
		}
		deadstate = 0;
		add_ry = 0.0f;
		MoveFlag = 0x00;
		MoveFlag_lt = MoveFlag;
		scopemode = 0;
		HitFlag = false;
		Hit_rx = 0.0f;
		totalmove = 0.0f;
		Invincible = false;

		MotionCtrl->Init(rotation_x);
	}
}

//! @brief Getting the setting value
//! @param id_param Pointer to receive a person's type number (NULL allowed)
//! @param dataid Pointer to receive the data number of points (NULL allowed)
//! @param p4 Pointer to receive the 4th parameter (NULL allowed)
//! @param team Pointer to receive the team number (NULL allowed)
void human::GetParamData(int *id_param, int *dataid, signed short int *p4, int *team)
{
	if( id_param != NULL ){ *id_param = id_parameter; }
	if( dataid != NULL ){ *dataid = point_dataid; }
	if( p4 != NULL ){ *p4 = point_p4; }
	if( team != NULL ){ *team = teamid; }
}

//! @brief Get HP
//! @return HP
int human::GetHP()
{
	return hp;
}

//! @brief Configure HP
//! @param in_hp New HP
//! @return success: true failure: false
//! @attention Fails unless the character has at least 1 HP.
bool human::SetHP(int in_hp)
{
	if( hp > 0 ){
		hp = in_hp;
		return true;
	}
	return false;
}


//! @brief Determining whether or not the carcass
//! @return Dead: true; alive: false.
//! @warning Determines the state that is completely stationary as "death". It is not possible that hp <= 0 is judged as "death".
bool human::GetDeadFlag()
{
	if( deadstate == 5 ){ return true; }
	return false;
}

//! @brief Determining whether or not death motion
//! @return In motion: true Not in motion: false
//! @attention Determines when the body falls or when the surface is pulled. It does not judge during fall.
bool human::GetDeadMotionFlag()
{
	if( deadstate == 1 ){ return true; }
	if( deadstate == 3 ){ return true; }
	return false;
}

//! @brief Configure Team Number (Overwrite)
//! @param id New Team Number
void human::SetTeamID(int id)
{
	teamid = id;
}

//! @brief Get Invincible Flags
//! @return Invincible: true; normal: false.
//! @attention Invincibility prevents all damage from bullets, grenade explosions, and falls.
bool human::GetInvincibleFlag()
{
	return Invincible;
}

//! @brief Set Unbeatable Flags
//! @param flag Invincible: true; normal: false.
//! @attention Invincibility prevents all damage from bullets, grenade explosions, and falls.
void human::SetInvincibleFlag(bool flag)
{
	Invincible = flag;
}

//! @brief Get the pre-processing transfer amount
//! @param *x Pointer to get X-axis travel (NULL allowed)
//! @param *y Pointer to acquire Y-axis travel (NULL is possible)
//! @param *z Pointer to obtain Z-axis travel (NULL allowed)
void human::GetMovePos(float *x, float *y, float *z)
{
	if( x != NULL ){ *x = move_x; }
	if( y != NULL ){ *y = move_y; }
	if( z != NULL ){ *z = move_z; }
}

//! @brief Configure model data
//! @param upmodel Model of the upper body
//! @param armmodel[] Array of arm models (array size: TOTAL_ARMMODE).
//! @param legmodel Model of stationary legs.
//! @param walkmodel[] Array of arm models (array size: TOTAL_WALKMODE).
//! @param runmodel[] Array of arm models (array size: TOTAL_RUNMODE).
void human::SetModel(int upmodel, int armmodel[], int legmodel, int walkmodel[], int runmodel[])
{
	MotionCtrl->SetModel(upmodel, armmodel, legmodel, walkmodel, runmodel);
}

//! @brief Set Weapons
//! @param in_weapon[] pointer array of weapon classes to set
//! @param nlnbs[] An int array that sets the number of bullets when the weapon is not in
//! @param nnbs[] Integer array specifying total ammunition when no weapon is held.
//! @warning Normally use PickupWeapon() function
void human::SetWeapon(class weapon *in_weapon[], int nlnbs[], int nnbs[])
{
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		if( in_weapon[i] == NULL ){
			weapon[i] = NULL;
		}
		else{
			//If you pick up your weapon correctly, register as your weapon.
			if( in_weapon[i]->Pickup() == 0 ){
				weapon[i] = in_weapon[i];
			}
		}
		noweapon_loadbullets[i] = nlnbs[i];
		noweapon_bullets[i] = nnbs[i];
	}
}

//! @brief Pick Up Weapons
//! @param in_weapon[] Weapon class pointer to set
//! @return Success:1 failure:0
int human::PickupWeapon(class weapon *in_weapon)
{
	if( in_weapon == NULL ){ return 0; }

	if( weapon[selectweapon] == NULL ){
		//If you pick up your weapon correctly, register as your weapon.
		if( in_weapon->Pickup() == 0 ){
			weapon[selectweapon] = in_weapon;

			//Set arm angle (rebound)
			class weapon *nowweapon;
			int id_param = 0;
			nowweapon = weapon[selectweapon];
			nowweapon->GetParamData(&id_param, NULL, NULL);
			MotionCtrl->PickupWeapon(id_param);

			//Initialization of the number of bullets when not in
			noweapon_bullets[selectweapon] = 0;
			noweapon_loadbullets[selectweapon] = 0;

			//Count of switch completion
			selectweaponcnt = 10;

			return 1;
		}
	}
	return 0;
}

//! @brief Switching weapons (changing)
//! @param id Weapon to switch to (-1 selects the next weapon).
//! @attention Do not call the game directly, and call it from the ObjectManager class.
void human::ChangeHaveWeapon(int id)
{
	//Failure if there is no physical strength
	if( hp <= 0 ){ return; }

	//Failed while reloading
	if( weaponreloadcnt > 0 ){ return; }

	//Failure if you are trying to switch to the same weapon
	if( selectweapon == id ){ return; }

	//Failed to switch weapons
	if( selectweaponcnt > 0 ){ return; }
	if( changeweaponidcnt > 0 ){ return; }

	if( id == -1 ){
		//Select the following weapon number
		selectweapon += 1;
		if( selectweapon == TOTAL_HAVEWEAPON ){
			selectweapon = 0;
		}
	}
	else{
		//Check whether the weapon number is within the range
		if( (id < 0)||(TOTAL_HAVEWEAPON <= id ) ){ return; }
		selectweapon = id;
	}

	//Disable scope display
	SetDisableScope();

	//Get the current weapon class
	class weapon *nowweapon;
	nowweapon = weapon[selectweapon];

	//Set arm angle (rebound)
	if( nowweapon == NULL ){	//Unarmed.
		MotionCtrl->ChangeHaveWeapon(ID_WEAPON_NONE);
	}
	else{
		int id_param = 0;
		nowweapon->GetParamData(&id_param, NULL, NULL);
		MotionCtrl->ChangeHaveWeapon(id_param);
	}

	//Count of switch completion
	selectweaponcnt = 10;

	//Burst mode count
	weaponburstmodecnt = 0;
}

//! @brief Get the weapon toggle count
//! @return Counter (1 or greater while switching).
int human::GetChangeHaveWeaponCnt()
{
	return selectweaponcnt;
}

//! @brief Weapon Type Switching
//! @return Success:0 failure:1
//! @attention Do not call the game directly, and call it from the ObjectManager class.
int human::ChangeWeaponID()
{
	//Failed to switch weapons
	//if( selectweaponcnt > 0 ){ return 1; }
	if( changeweaponidcnt > 0 ){ return 1; }

	//Failed if you don't specify a weapon
	if( weapon[selectweapon] == NULL ){ return 1; }

	//Get the information of the weapon you are equipped
	int param_id, lnbs, nbs;
	weapon[selectweapon]->GetParamData(&param_id, &lnbs, &nbs);

	//Find the type of weapon toggle (new weapon number)
	WeaponParameter ParamData;
	int ChangeWeaponID, ChangeCnt;
	if( Param->GetWeapon(param_id, &ParamData) != 0 ){ return 1; }
	ChangeWeaponID = ParamData.ChangeWeaponID;
	ChangeCnt = ParamData.ChangeWeaponCnt;

	//Check whether the new weapon number is correct
	if( ChangeWeaponID == param_id ){ return 1; }
	if( (ChangeWeaponID < 0)||(TOTAL_PARAMETERINFO_WEAPON <= ChangeWeaponID) ){ return 1; }

	//If the count is reloaded, it is not switched
	if( (ChangeCnt > 0)&&(weaponreloadcnt > 0) ){ return 1; }

	//Apply Settings
	weapon[selectweapon]->SetParamData(ChangeWeaponID, lnbs, nbs, false);

	//If you are using a scope
	if( scopemode != 0 ){
		if( ChangeCnt == 0 ){
			//Get new weapon information
			if( Param->GetWeapon(ChangeWeaponID, &ParamData) != 0 ){ return 1; }

			if( scopemode == ParamData.scopemode ){
				//If the same scope is used, the scope is maintained without any
			}
			else if( ParamData.scopemode == 0 ){
				//Unscoping if scope is not available
				SetDisableScope();
			}
			else{
				//If different scopes are available, reconfigure scopes
				SetEnableScope();
			}
		}
		else{
			//Disable the scope.
			SetDisableScope();
		}
	}

	//Motion execution
	MotionCtrl->ChangeWeaponID(ChangeWeaponID);

	//Count of switch completion
	changeweaponidcnt = ChangeCnt;

	//Burst mode count
	weaponburstmodecnt = 0;

	return 0;
}

//! @brief Get the type switch count of weapons
//! @return Counter (1 or greater while switching).
int human::GetChangeWeaponIDCnt()
{
	return changeweaponidcnt;
}

//! @brief Get weapon reload count
//! @return Count (Reloading: 1 or more)
int human::GetWeaponReloadCnt()
{
	return weaponreloadcnt;
}

//! @brief Get weapons
//! @param out_selectweapon Selected weapon (0 to TOTAL_HAVEWEAPON - 1).
//! @param out_weapon Array of pointers to weapon instances (array size: TOTAL_HAVEWEAPON).
//! @param nlnbs[] Integer array receiving the loaded ammunition count when no weapon is held (size: TOTAL_HAVEWEAPON; NULL allowed).
//! @param nnbs[] Integer array receiving total ammunition when no weapon is held (size: TOTAL_HAVEWEAPON; NULL allowed).
void human::GetWeapon(int *out_selectweapon, class weapon *out_weapon[], int nlnbs[], int nnbs[])
{
	*out_selectweapon = selectweapon;
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		out_weapon[i] = weapon[i];
		if( nlnbs != NULL ){ nlnbs[i] = noweapon_loadbullets[i]; }
		if( nnbs != NULL ){ nnbs[i] = noweapon_bullets[i]; }
	}
}

//! @brief Get the type ID of the currently equipped weapon.
//! @return Weapon type ID (0 to TOTAL_PARAMETERINFO_WEAPON - 1).
//! @attention This is the type number of weapons that you currently have. It is equivalent to using the GetWeapon() function.
int human::GetMainWeaponTypeNO()
{
	if( weapon[selectweapon] == NULL ){
		return ID_WEAPON_NONE;
	}

	int id_param;
	weapon[selectweapon]->GetParamData(&id_param, NULL, NULL);
	return id_param;
}

//! @brief es a resonance setting
//! @return Automatic fire supported: true; automatic fire unsupported: false.
bool human::GetWeaponBlazingmode()
{
	int param_id;
	WeaponParameter data;

	//Failure if you don't specify weapons
	if( weapon[selectweapon] == NULL ){ return false; }

	//Get the type number of weapons you are equipped
	weapon[selectweapon]->GetParamData(&param_id, NULL, NULL);

	//Returns the resonance setting
	Param->GetWeapon(param_id, &data);
	if( data.burstcnt == 1 ){ return false; }
	return true;
}

//! @brief Process firing.
//! @param weapon_paramid Pointer to receive the number of gunned weapons
//! @param GunsightErrorRange Pointer receiving the aiming error when firing.
//! @return success: true failure: false
//! @attention The processing of the bullet object and the playback of the cannon should be done separately.
//! @attention Do not call the game directly, and call it from the ObjectManager class.
bool human::ShotWeapon(int *weapon_paramid, int *GunsightErrorRange)
{
	if( weapon_paramid == NULL ){ return false; }
	if( GunsightErrorRange == NULL ){ return false; }

	int param_id;

	//Failed to switch weapons
	if( selectweaponcnt > 0 ){ return false; }
	if( changeweaponidcnt > 0 ){ return false; }

	//Failure if you don't specify weapons
	if( weapon[selectweapon] == NULL ){ return false; }

	//Accept the firing request and enable the flag.
	weaponshotframe = true;

	//Failure to meet or reload the projection interval
	if( weaponshotcnt > 0 ){ return false; }
	if( weaponreloadcnt > 0 ){ return false; }

	//Get weapon type number
	weapon[selectweapon]->GetParamData(&param_id, NULL, NULL);

	//Get weapon settings
	WeaponParameter ParamData;
	ScopeParameter sparam;
	if( Param->GetWeapon(param_id, &ParamData) != 0 ){ return false; }
	Param->GetScopeParam(ParamData.scopemode, &sparam);

	//Burst mode is enabled, and if the number of hits is exceeded, failure
	if( (ParamData.burstcnt > 0)&&(weaponburstmodecnt >= ParamData.burstcnt) ){ return false; }

	//Performs sampling process.
	if( weapon[selectweapon]->Shot() != 0 ){ return false; }

	//Count as the number of bullets
	weaponburstmodecnt += 1;


	//Set the resonance count
	weaponshotcnt = ParamData.blazings;

	//Set error when not using scope
	if( (scopemode == 0)&&(sparam.WeaponNoScopeReaction > 0) ){
		ReactionGunsightErrorRange = sparam.WeaponNoScopeReaction;
	}


	//Returns weapon type number and error
	*weapon_paramid = param_id;
	*GunsightErrorRange = StateGunsightErrorRange + ReactionGunsightErrorRange;


	//Error Calculator
	if( ( (scopemode == 0)&&(sparam.NoScopeGunsight == true) )||( (scopemode != 0)&&(sparam.ScopeGunsight == 0) ) ){
		//Adds the set error if the tertiary is displayed.
		ReactionGunsightErrorRange += ParamData.reaction;
	}

	//Adding rebounds in a scope
	if( (sparam.WeaponRecoil_ScopeRx != 0.0f)||(sparam.WeaponRecoil_ScopeRyMin != 0.0f)||(sparam.WeaponRecoil_ScopeRyMax != 0.0f) ){
		if( (scopemode != 0)||(sparam.WeaponRecoil_ScopeOnly == false) ){
			int rx_rnds, ry_rnds;
			float rx_degree, ry_degree;

			//Random number range calculation
			rx_rnds = (int)(sparam.WeaponRecoil_ScopeRx*10) * 2;
			ry_rnds = (int)(sparam.WeaponRecoil_ScopeRyMax*10) - (int)(sparam.WeaponRecoil_ScopeRyMin*10);

			//Reaction (angle) decision
			rx_degree = sparam.WeaponRecoil_ScopeRx*-1 + ((float)GetRand(rx_rnds))/10;
			ry_degree = sparam.WeaponRecoil_ScopeRyMin + ((float)GetRand(ry_rnds))/10;

			//Reaction Addition
			rotation_x += DegreeToRadian(rx_degree);
			armrotation_y += DegreeToRadian(ry_degree);
		}
	}

	//Conveying Rebound Arms
	MotionCtrl->ShotWeapon(param_id);

	//If there is no weapon, it will be removed from the equipment. (For grenades)
	if( weapon[selectweapon]->GetEnableFlag() == false ){
		weapon[selectweapon] = NULL;
	}
	return true;
}

//! @brief Reload
//! @return success: true failure: false
//! @attention Do not call the game directly, and call it from the ObjectManager class.
bool human::ReloadWeapon()
{
	//Failed to switch weapons
	if( selectweaponcnt > 0 ){ return false; }
	if( changeweaponidcnt > 0 ){ return false; }

	//If you have specified a weapon of something
	if( weapon[selectweapon] != NULL ){
		//Failed while reloading
		if( weaponreloadcnt > 0 ){ return false; }

		//Started reloading
		if( weapon[selectweapon]->StartReload() != 0 ){ return false; }

		//Unlock scope mode
		SetDisableScope();

		//Motion execution
		class weapon *nowweapon;
		int id_param = 0;
		nowweapon = weapon[selectweapon];
		nowweapon->GetParamData(&id_param, NULL, NULL);
		MotionCtrl->ReloadWeapon(id_param);

		//Get the performance value of the weapon
		int param_id;
		WeaponParameter ParamData;
		weapon[selectweapon]->GetParamData(&param_id, NULL, NULL);
		if( Param->GetWeapon(param_id, &ParamData) != 0 ){ return 1; }

		//Set Reload Count
		weaponreloadcnt = ParamData.reloads + 1;

		//Set count for burst mode
		weaponburstmodecnt = 0;

		return true;
	}
	return false;
}

//! @brief Discarding weapons
//! @return success: true failure: false
//! @attention Do not call the game directly, and call it from the ObjectManager class.
bool human::DumpWeapon()
{
	//Failed to switch weapons
	if( selectweaponcnt > 0 ){ return false; }
	if( changeweaponidcnt > 0 ){ return false; }

	//If you have specified a weapon of something
	if( weapon[selectweapon] != NULL ){
		//Failed while reloading
		if( weaponreloadcnt > 0 ){ return false; }

		//Discarded weapons and removed equipment
		weapon[selectweapon]->Dropoff(pos_x, pos_y, pos_z, rotation_x, 1.63f);
		weapon[selectweapon] = NULL;

		//Initialization of the number of bullets when not in
		noweapon_bullets[selectweapon] = 0;
		noweapon_loadbullets[selectweapon] = 0;

		//Unlock scope mode
		SetDisableScope();

		//Motion execution
		MotionCtrl->DumpWeapon();

		return true;
	}

	return false;
}

//! @brief Set up the advance (run)
//! @attention Do not call the game directly, and call it from the ObjectManager class.
void human::SetMoveForward()
{
	SetFlag(MoveFlag, MOVEFLAG_FORWARD);
}

//! @brief Set backward movement.
//! @attention Do not call the game directly, and call it from the ObjectManager class.
void human::SetMoveBack()
{
	SetFlag(MoveFlag, MOVEFLAG_BACK);
}

//! @brief Set the left run
//! @attention Do not call the game directly, and call it from the ObjectManager class.
void human::SetMoveLeft()
{
	SetFlag(MoveFlag, MOVEFLAG_LEFT);
}

//! @brief Set the right run
//! @attention Do not call the game directly, and call it from the ObjectManager class.
void human::SetMoveRight()
{
	SetFlag(MoveFlag, MOVEFLAG_RIGHT);
}

//! @brief Set up walking
//! @attention Do not call the game directly, and call it from the ObjectManager class.
void human::SetMoveWalk()
{
	SetFlag(MoveFlag, MOVEFLAG_WALK);
}

//! @brief Get a person's moving mode
//! @param nowdata get current value: true Use the previous frame value: false
//! @return Walking: 1; running forward: 2; running backward: 3; running sideways: 4; stationary: 0.
int human::GetMovemode(bool nowdata)
{
	//Return if the object is unused or has not been processed.
	if( EnableFlag == false ){ return 0; }
	if( hp <= 0 ){ return 0; }

	if( nowdata == false ){	//Previous Data
		//Walking 1
		if( GetFlag(MoveFlag_lt, MOVEFLAG_WALK) ){
			return 1;
		}
		//Previous running 2
		if( GetFlag(MoveFlag_lt, MOVEFLAG_FORWARD) ){
			return 2;
		}
		//Backward 3
		if( GetFlag(MoveFlag_lt, MOVEFLAG_BACK) ){
			return 3;
		}
		//Left and right 4
		if( GetFlag(MoveFlag_lt, (MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) ){
			return 4;
		}
	}
	else{					//Current Data
		//Walking 1
		if( GetFlag(MoveFlag, MOVEFLAG_WALK) ){
			return 1;
		}
		//Previous running 2
		if( GetFlag(MoveFlag, MOVEFLAG_FORWARD) ){
			return 2;
		}
		//Backward 3
		if( GetFlag(MoveFlag, MOVEFLAG_BACK) ){
			return 3;
		}
		//Left and right 4
		if( GetFlag(MoveFlag, (MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) ){
			return 4;
		}
	}

	//Not moving
	return 0;
}

//! @brief Get the flags indicating movement direction.
//! @return Configured flags (Human_MoveFlag enumeration).
//! @attention Debugging function. Normally use GetMovemode().
int human::GetMoveFlag(bool nowdata)
{
	if( nowdata == false ){	//Previous Data
		return MoveFlag_lt;
	}
	else{					//Current Data
		return MoveFlag;
	}
}

//! @brief Set scope
//! @return success: true failure: false
bool human::SetEnableScope()
{
	int param_id;
	WeaponParameter data;

	//Nothing fails if you don't specify a weapon
	if( weapon[selectweapon] == NULL ){ return false; }

	//Failed while reloading
	if( weaponreloadcnt > 0 ){ return false; }

	//Get weapon type number
	weapon[selectweapon]->GetParamData(&param_id, NULL, NULL);

	//Get weapon settings
	Param->GetWeapon(param_id, &data);

	//Set scope
	scopemode = data.scopemode;
	return true;
}

//! @brief Unscope
void human::SetDisableScope()
{
	scopemode = 0;
}

//! @brief Get scope settings
int human::GetScopeMode()
{
	return scopemode;
}

//! @brief Get horizontal and vertical axis orientation
//! @param rx Pointer to get a horizontal axis
//! @param ry Pointer to get the vertical axis
void human::GetRxRy(float *rx, float *ry)
{
	if( (rx == NULL)||(ry == NULL) ){ return; }

	*rx = rotation_x;
	*ry = armrotation_y;
}

//! @brief Set the orientation of the horizontal and vertical axis
//! @param rx Horizontal angle to set.
//! @param ry Vertical axis setting
void human::SetRxRy(float rx, float ry)
{
	rotation_x = rx;
	armrotation_y = ry;
}

//! @brief Get the overall rotation angle.
//! @return Pointer to get the vertical axis
//! @warning This is the angle of death and fall. It differs from the value received by the GetRxRy() function.
float human::GetDeadRy()
{
	return rotation_y;
}

//! @brief Jumping
//! @attention Do not call the game directly, and call it from the ObjectManager class.
void human::Jump()
{
	SetFlag(MoveFlag, MOVEFLAG_JUMP);
}

//! @brief Jumping and Landing
//! @param nowdata get current value: true Use the previous frame value: false
//! @return Jumping: 1; landing: 2; otherwise: 0.
//! @attention Landing detection always returns the current state, not the previous frame's state.
//! @note For foot sound playback judgment
int human::GetJumpLanding(bool nowdata)
{
	//Return if the object is unused or has not been processed.
	if( EnableFlag == false ){ return 0; }
	if( hp <= 0 ){ return 0; }

	if( nowdata == false ){	//Previous Data
		if( move_y_landing == true ){ return 2; }
		if( GetFlag(MoveFlag_lt, MOVEFLAG_JUMP) ){ return 1; }
	}
	else{
		if( move_y_landing == true ){ return 2; }
		if( GetFlag(MoveFlag, MOVEFLAG_JUMP) ){ return 1; }
	}

	//Not jumping
	return 0;
}

//! @brief es the original block ID and face number.
//! @param id Pointer to receive the block ID of the foot (NULL allowed)
//! @param face Pointer to receive the face number of the footer block (NULL allowed)
//! @attention If there is no block on the foot, such as in the air, the block ID and the face number will return -1.
void human::GetUnderBlock(int *id, int *face)
{
	if( id != NULL ){ *id = underblock_id; }
	if( face != NULL ){ *face = underblock_face; }
}

//! @brief Pushing and Powering
//! @param rx Horizontal axis.
//! @param ry Vertical axis.
//! @param speed Speed
void human::AddPosOrder(float rx, float ry, float speed)
{
	move_x += cosf(rx) * cosf(ry) * speed;
	move_y += sinf(ry) * speed;
	move_z += sinf(rx) * cosf(ry) * speed;
}

//! @brief hits head
//! @param attacks Bullet damage.
void human::HitBulletHead(int attacks)
{
	if( Invincible == false ){
		hp -= (int)((float)attacks * HUMAN_DAMAGE_HEAD) + GetRand(16);
	}
	ReactionGunsightErrorRange = 15;
}

//! @brief hits the upper body
//! @param attacks Bullet damage.
void human::HitBulletUp(int attacks)
{
	if( Invincible == false ){
		hp -= (int)((float)attacks * HUMAN_DAMAGE_UP) + GetRand(8);
	}
	ReactionGunsightErrorRange = 12;
}

//! @brief Hit the lower body
//! @param attacks Bullet damage.
void human::HitBulletLeg(int attacks)
{
	if( Invincible == false ){
		hp -= (int)((float)attacks * HUMAN_DAMAGE_LEG) + GetRand(6);
	}
	ReactionGunsightErrorRange = 8;
}

//! @brief A zombie attack hit the character.
void human::HitZombieAttack()
{
	if( Invincible == false ){
		hp -= HUMAN_DAMAGE_ZOMBIEU + GetRand(HUMAN_DAMAGE_ZOMBIEA);
	}
	ReactionGunsightErrorRange = 10;
}

//! @brief Hit the grenades
//! @param attacks Blast damage.
//! @attention Please make the calculation by the distance beforehand.
void human::HitGrenadeExplosion(int attacks)
{
	if( Invincible == false ){
		hp -= attacks;
	}
	ReactionGunsightErrorRange = 10;
}

//! @brief Set the bullet flag
//! @param rx Direction from which the hit was received.
void human::SetHitFlag(float rx)
{
	HitFlag = true;
	Hit_rx = rx;
}

//! @brief Check whether you are playing
//! @param rx Pointer to receive the affected direction (NULL allowed)
//! @return Hit received: true; no hit received: false.
//! @attention When executed, the flag is initialized to false.
bool human::CheckHit(float *rx)
{
	bool returnflag = HitFlag;
	if( rx != NULL ){ *rx = Hit_rx; }
	HitFlag = false;
	//Hit_rx = 0.0f;
	return returnflag;
}

//! @brief Get total travel
//! @return Total travel
float human::GetTotalMove()
{
	return totalmove;
}

//! @brief Get the walking/running animation counter.
//! @return Count Value
int human::GetMoveMotionCount()
{
	return MotionCtrl->GetMoveMotionCnt();
}

//! @brief Processing of tertiary state errors
//!	@attention Process this before ControlProcess().
void human::GunsightErrorRange()
{
	//Initialize
	StateGunsightErrorRange = 0;

	//Set error by each operation
	if( GetFlag(MoveFlag, MOVEFLAG_WALK) ){
		StateGunsightErrorRange = 4;
	}
	if( GetFlag(MoveFlag, MOVEFLAG_FORWARD) ){
		StateGunsightErrorRange = 8;
	}
	if( GetFlag(MoveFlag, MOVEFLAG_BACK) ){
		StateGunsightErrorRange = 6;
	}
	if( GetFlag(MoveFlag, (MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) ){
		StateGunsightErrorRange = 7;
	}
	if( move_y_flag == true ){
		StateGunsightErrorRange = 22;
	}
	if( hp < 40 ){
		StateGunsightErrorRange += 3;
	}

	//If you have any weapons
	if( weapon[selectweapon] != NULL ){
		//Get weapon configuration data
		int param;
		WeaponParameter data;
		weapon[selectweapon]->GetParamData(&param, NULL, NULL);
		Param->GetWeapon(param, &data);

		//Reduce errors 1
		ReactionGunsightErrorRange -= 1;

		//Corrects the range of errors
		if( ReactionGunsightErrorRange < 0 ){ ReactionGunsightErrorRange = 0; }
		if( ReactionGunsightErrorRange > data.ErrorRangeMAX ){ ReactionGunsightErrorRange = data.ErrorRangeMAX; }
	}
	else{
		ReactionGunsightErrorRange = 0;
	}
}

//! @brief Death Determination and Processes
//! @return Stationary corpse: 4; just finished falling: 3; falling: 2; starting to fall: 1; no action: 0.
int human::CheckAndProcessDead(class Collision *CollD)
{
	if( CollD == NULL ){ return 0; }

	if( hp <= 0 ){
		//Arm Angle
		if( armrotation_y < 0.0f ){
			armrotation_y -= DegreeToRadian(6);
			if( armrotation_y < DegreeToRadian(-90) ){ armrotation_y = DegreeToRadian(-90); }
		}
		else{
			armrotation_y += DegreeToRadian(6);
			if( armrotation_y > DegreeToRadian(90) ){ armrotation_y = DegreeToRadian(90); }
		}
	}

#ifdef HUMAN_DEADBODY_COLLISION

	//Notes:
	//
	//Status: 0
	//[Do not do anything]
	// HP is 0 or less, if the head is not attached state: 1
	// If HP is 0 or less and the head touches a wall, change to state 2.
	//
	//Status:1
	//[Start tilting]
	// At 135 degrees, change to state 2.
	// If the head hits a wall, change to state 3.
	//
	//State: 2.
	//[Drop]
	// When the foot is attached to the ground State: 4
	//
	//State: 3.
	//[S] the world]
	//  
	//Status: 4
	//[Fixed]

	float check_posx, check_posy, check_posz;

	if( deadstate == 0 ){
		if( hp <= 0 ){		//If your HP is 0 or less (deathed) moment, start to fall
			float tr;

			//Last attacked direction
			tr = Hit_rx - rotation_x;
			tr = AngleNormalization(tr);

			//Body Angle
			if( ((float)M_PI/2*-1 < tr)&&(tr < (float)M_PI/2) ){
				add_ry = HUMAN_DEADADDRY;
			}
			else{
				add_ry = HUMAN_DEADADDRY * -1;
			}

			//+1.0 height so that the carcass is not buried
			pos_y += 1.0f;

			//Discard all of your weapons
			for(int i=0; i<TOTAL_HAVEWEAPON; i++){
				if( weapon[i] != NULL ){
					weapon[i]->Dropoff(pos_x, pos_y, pos_z, DegreeToRadian(10)*GetRand(36), 1.5f);
					weapon[i] = NULL;

					//Initialization of the number of bullets when not in
					noweapon_bullets[i] = 0;
					noweapon_loadbullets[i] = 0;
				}
			}

			//Unlock scope mode
			SetDisableScope();

			//Various count initialization
			selectweaponcnt = 0;
			changeweaponidcnt = 0;
			weaponshotcnt = 0;
			weaponreloadcnt = 0;

			//Get the coordinates of the head of the next frame
			check_posx = pos_x + cosf(rotation_x*-1 - (float)M_PI/2) * sinf(add_ry) * HUMAN_HEIGHT;
			check_posy = pos_y + cosf(add_ry) * HUMAN_HEIGHT;
			check_posz = pos_z + sinf(rotation_x*-1 - (float)M_PI/2) * sinf(add_ry) * HUMAN_HEIGHT;

			if( CollD->CheckALLBlockInside(check_posx, check_posy, check_posz) == true ){
				deadstate = 2;
			}
			else{
				rotation_y += add_ry;
				deadstate = 1;
			}
			return 1;
		}
		return 0;
	}

	if( deadstate == 1 ){
		//If you are defeated over 135 degrees
		if( fabsf(rotation_y) >= DegreeToRadian(135) ){
			deadstate = 2;
			move_y = 0.0f;
			return 2;
		}

		if( pos_y <= (HUMAN_DEADLINE + 10.0f) ){
			//If you fall over 90 degrees
			if( fabsf(rotation_y) >= DegreeToRadian(90) ){
				deadstate = 4;
				return 2;
			}
		}

		//Tip forward or backward.
		if( rotation_y > 0.0f ){		//rotation_y < DegreeToRadian(135)
			add_ry += HUMAN_DEADADDRY;
		}
		else if( rotation_y < 0.0f ){	//rotation_y > DegreeToRadian(-135)
			add_ry -= HUMAN_DEADADDRY;
		}

		if( pos_y <= HUMAN_DEADLINE ){
			rotation_y += add_ry;
		}
		else{
			//Get the coordinates of the head of the next frame
			check_posx = pos_x + cosf(rotation_x*-1 - (float)M_PI/2) * sinf(rotation_y + add_ry) * HUMAN_HEIGHT;
			check_posy = pos_y + cosf(rotation_y + add_ry) * HUMAN_HEIGHT;
			check_posz = pos_z + sinf(rotation_x*-1 - (float)M_PI/2) * sinf(rotation_y + add_ry) * HUMAN_HEIGHT;

			if( CollD->CheckALLBlockInside(check_posx, check_posy, check_posz) == true ){
				deadstate = 3;
			}
			else{
				rotation_y += add_ry;
			}
		}

		return 2;
	}

	if( deadstate == 2 ){
		if( pos_y <= HUMAN_DEADLINE ){
			deadstate = 4;
			return 2;
		}

		if( move_y > -4.0f ){
			move_y -= 0.1f;
		}

		//coordinates of the next frame leg
		check_posx = pos_x;
		check_posy = pos_y - move_y - HUMAN_HEIGHT*0.5f;
		check_posz = pos_z;

		if( CollD->CheckALLBlockInside(check_posx, check_posy, check_posz) == true ){
			deadstate = 4;
		}
		else{
			pos_y += move_y;
		}

		return 2;
	}

	if( deadstate == 3 ){
		//deadstate = 4;

		//If you fall over 90 degrees
		if( fabsf(rotation_y) >= DegreeToRadian(90) ){
			deadstate = 4;
			return 2;
		}

		//Tip forward or backward.
		if( rotation_y > 0.0f ){		//rotation_y < DegreeToRadian(90)
			add_ry += HUMAN_DEADADDRY;
		}
		else if( rotation_y < 0.0f ){	//rotation_y > DegreeToRadian(-90)
			add_ry -= HUMAN_DEADADDRY;
		}

		//Get the coordinates of the foot of the next frame
		check_posx = pos_x - cosf(rotation_x*-1 - (float)M_PI/2) * sinf(rotation_y + add_ry) * HUMAN_HEIGHT;
		check_posy = pos_y + 0.1f;
		check_posz = pos_z - sinf(rotation_x*-1 - (float)M_PI/2) * sinf(rotation_y + add_ry) * HUMAN_HEIGHT;

		if( CollD->CheckALLBlockInside(check_posx, check_posy, check_posz) == true ){
			deadstate = 4;
			return 2;
		}

		//Get the coordinates of the head of the next frame
		check_posx = pos_x - cosf(rotation_x*-1 - (float)M_PI/2) * sinf(rotation_y + add_ry) * HUMAN_HEIGHT;
		check_posy = pos_y + cosf(rotation_y + add_ry) * HUMAN_HEIGHT;
		check_posz = pos_z - sinf(rotation_x*-1 - (float)M_PI/2) * sinf(rotation_y + add_ry) * HUMAN_HEIGHT;

		if( CollD->CheckALLBlockInside(check_posx, check_posy, check_posz) == true ){
			deadstate = 4;
			return 2;
		}

		//Move foot coordinates
		pos_x -= cosf(rotation_x*-1 - (float)M_PI/2) * sinf(add_ry) * HUMAN_HEIGHT;
		pos_z -= sinf(rotation_x*-1 - (float)M_PI/2) * sinf(add_ry) * HUMAN_HEIGHT;

		rotation_y += add_ry;
		return 2;
	}

	if( deadstate == 4 ){
		//Nothing (fixed)
		deadstate = 5;
		return 3;
	}

	if( deadstate == 5 ){
		//Nothing (fixed)
		return 4;
	}

	return 0;
#else
	if( fabsf(rotation_y) >= DegreeToRadian(90) ){
		return 4;
	}
	else if( rotation_y > 0.0f ){		//If you start to fall, you can defeat it as it is.
		add_ry += HUMAN_DEADADDRY;
		rotation_y += add_ry;
		if( rotation_y >= DegreeToRadian(90) ){
			rotation_y = DegreeToRadian(90);
			deadstate = 5;
			return 3;
		}
		return 2;
	}
	else if( rotation_y < 0.0f ){	//If you start to fall, you can defeat it as it is.
		add_ry -= HUMAN_DEADADDRY;
		rotation_y += add_ry;
		if( rotation_y <= DegreeToRadian(-90) ){
			rotation_y = DegreeToRadian(-90);
			deadstate = 5;
			return 3;
		}
		return 2;
	}
	else if( hp <= 0 ){		//If your HP is 0 or less (deathed) moment, start to fall
		float tr;

		//Last attacked direction
		tr = Hit_rx - rotation_x;
		tr = AngleNormalization(tr);

		//Body Angle
		if( ((float)M_PI/2*-1 < tr)&&(tr < (float)M_PI/2) ){
			add_ry = HUMAN_DEADADDRY;
		}
		else{
			add_ry = HUMAN_DEADADDRY * -1;
		}

		rotation_y += add_ry;

		//+1.0 height so that the carcass is not buried
		pos_y += 1.0f;

		//Discard all of your weapons
		for(int i=0; i<TOTAL_HAVEWEAPON; i++){
			if( weapon[i] != NULL ){
				weapon[i]->Dropoff(pos_x, pos_y, pos_z, DegreeToRadian(10)*GetRand(36), 1.5f);
				weapon[i] = NULL;

				//Initialization of the number of bullets when not in
				noweapon_bullets[i] = 0;
				noweapon_loadbullets[i] = 0;
			}
		}

		//Unlock scope mode
		SetDisableScope();

		deadstate = 1;
		return 1;
	}

	return 0;
#endif
}

//! @brief Calculate movement from input controls.
//! @attention When executed, each key flag is initialized to false.
void human::ControlProcess()
{
	//Determine progressive direction and speed
	if( GetFlag(MoveFlag, MOVEFLAG_WALK) ){
		move_rx = DegreeToRadian(0);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_PROGRESSWALK_ACCELERATION);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == MOVEFLAG_FORWARD ){
		move_rx = DegreeToRadian(0);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_PROGRESSRUN_ACCELERATION);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == MOVEFLAG_BACK ){
		move_rx = DegreeToRadian(180);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_REGRESSRUN_ACCELERATION);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == MOVEFLAG_LEFT ){
		move_rx = DegreeToRadian(90);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_SIDEWAYSRUN_ACCELERATION);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == MOVEFLAG_RIGHT ){
		move_rx = DegreeToRadian(-90);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_SIDEWAYSRUN_ACCELERATION);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == (MOVEFLAG_FORWARD | MOVEFLAG_LEFT) ){
		move_rx = DegreeToRadian(45);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_PROGRESSRUN_SIDEWAYSRUN_ACCELERATION);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == (MOVEFLAG_BACK | MOVEFLAG_LEFT) ){
		move_rx = DegreeToRadian(135);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_REGRESSRUN_SIDEWAYSRUN_ACCELERATION);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == (MOVEFLAG_BACK | MOVEFLAG_RIGHT) ){
		move_rx = DegreeToRadian(-135);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_REGRESSRUN_SIDEWAYSRUN_ACCELERATION);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == (MOVEFLAG_FORWARD | MOVEFLAG_RIGHT) ){
		move_rx = DegreeToRadian(-45);
		AddPosOrder(rotation_x*-1 + move_rx + (float)M_PI/2, 0.0f, HUMAN_PROGRESSRUN_SIDEWAYSRUN_ACCELERATION);
	}
	else{
		move_rx = 0.0f;
	}

	//Backup flags
	MoveFlag_lt = MoveFlag;

	//Unlock key flags
	MoveFlag = 0x00;
}

//! @brief Determination of the above
//! @param CollD Pointer of Collision class
//! @param inblockdata Pointer of BlockDataInterface class
//! @param AddCollisionFlag Additional collision-detection flag.
//! @param player the player is eligible
void human::CollisionMap(class Collision *CollD, class BlockDataInterface *inblockdata, bool AddCollisionFlag, bool player)
{
	if( CollD == NULL ){ return; }
	if( inblockdata == NULL ){ return; }

	float pos_x2, pos_y2, pos_z2;
	float dist_x, dist_y, dist_z;
	float box_min_x, box_min_y, box_min_z, box_max_x, box_max_y, box_max_z;
	bool CheckBlockID[MAX_BLOCKS];
	float speed;
	float Dist;
	bool FallFlag;

	//Set the block ID and face number to -1.
	underblock_id = -1;
	underblock_face = -1;

	//Back up current coordinates
	pos_x2 = pos_x;
	pos_y2 = pos_y;
	pos_z2 = pos_z;

	//Apply movement.
	pos_x += move_x;
	pos_z += move_z;

	//Cutting the moving amount of the next frame
	move_x *= HUMAN_ATTENUATION;
	move_z *= HUMAN_ATTENUATION;

	//*In this house XOPS, judgement is performed by people here.
	// In OpenXOPS, the following dist_y is 0.0f, and it is meaningless as the value.

	//Move vector calculation
	dist_x = pos_x - pos_x2;
	dist_y = pos_y - pos_y2;
	dist_z = pos_z - pos_z2;
	speed = sqrtf(dist_x*dist_x + dist_z*dist_z);
	VectorNormalization(&dist_x, &dist_y, &dist_z);

	int bs = inblockdata->GetTotaldatas();

	//Create a person A
	box_min_x = pos_x - HUMAN_MAPCOLLISION_CHECK_MAXDIST - COLLISION_ADDSIZE;
	box_min_y = pos_y - HUMAN_MAPCOLLISION_CHECK_MAXDIST - COLLISION_ADDSIZE;
	box_min_z = pos_z - HUMAN_MAPCOLLISION_CHECK_MAXDIST - COLLISION_ADDSIZE;
	box_max_x = pos_x + HUMAN_MAPCOLLISION_CHECK_MAXDIST + COLLISION_ADDSIZE;
	box_max_y = (pos_y + HUMAN_HEIGHT) + HUMAN_MAPCOLLISION_CHECK_MAXDIST + COLLISION_ADDSIZE;
	box_max_z = pos_z + HUMAN_MAPCOLLISION_CHECK_MAXDIST + COLLISION_ADDSIZE;

	//Use an AABB test to narrow down candidate blocks.
	for(int i=0; i<bs; i++){
		CheckBlockID[i] = CollD->CheckBlockAABB(i, box_min_x, box_min_y, box_min_z, box_max_x, box_max_y, box_max_z);
	}

	if( (speed != 0.0f)||(move_y != 0.0f) ){

		//Determination by head
		for(int i=0; i<bs; i++){
			if( CheckBlockID[i] == true ){
				if( CollisionBlockScratch(CollD, inblockdata, i, &pos_x, &pos_y, &pos_z, pos_x2, pos_y2, pos_z2, pos_x, pos_y + HUMAN_HEIGHT-0.22f, pos_z, 0x01) == true ){
					if( move_y > 0.0f ){ move_y = 0.0f; }
				}
			}
		}

		//Determined by the foot
		for(int i=0; i<bs; i++){
			if( CheckBlockID[i] == true ){
				CollisionBlockScratch(CollD, inblockdata, i, &pos_x, &pos_y, &pos_z, pos_x2, pos_y2, pos_z2, pos_x, pos_y, pos_z, 0x00);
			}
		}

		//Determined at the waist
		for(int i=0; i<bs; i++){
			if( CheckBlockID[i] == true ){
				CollisionBlockScratch(CollD, inblockdata, i, &pos_x, &pos_y, &pos_z, pos_x2, pos_y2, pos_z2, pos_x + dist_x*HUMAN_MAPCOLLISION_R, pos_y + HUMAN_MAPCOLLISION_HEIGHT, pos_z + dist_z*HUMAN_MAPCOLLISION_R, 0x02);
				CollisionBlockScratch(CollD, inblockdata, i, &pos_x, &pos_y, &pos_z, pos_x2, pos_y2, pos_z2, pos_x + dist_z*HUMAN_MAPCOLLISION_R, pos_y + HUMAN_MAPCOLLISION_HEIGHT, pos_z + dist_x*HUMAN_MAPCOLLISION_R, 0x02);
				CollisionBlockScratch(CollD, inblockdata, i, &pos_x, &pos_y, &pos_z, pos_x2, pos_y2, pos_z2, pos_x - dist_z*HUMAN_MAPCOLLISION_R, pos_y + HUMAN_MAPCOLLISION_HEIGHT, pos_z - dist_x*HUMAN_MAPCOLLISION_R, 0x02);
			}
		}

		if( AddCollisionFlag == true ){
			//Additional collision check near the waist.
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					CollisionBlockScratch(CollD, inblockdata, i, &pos_x, &pos_y, &pos_z, pos_x2, pos_y2, pos_z2, pos_x, pos_y + HUMAN_MAPCOLLISION_ADD_HEIGHT_A, pos_z, 0x02);
					CollisionBlockScratch(CollD, inblockdata, i, &pos_x, &pos_y, &pos_z, pos_x2, pos_y2, pos_z2, pos_x, pos_y + HUMAN_MAPCOLLISION_ADD_HEIGHT_B, pos_z, 0x02);
				}
			}
		}

		//Climb the slope
		if( (fabsf(move_x) > 0.2f)||(fabsf(move_z) > 0.2f) ){
			if( move_y_upper == 0 ){
				bool flag = false;
				for(int i=0; i<bs; i++){
					if( CheckBlockID[i] == true ){
						//If you are buried in the block only on the toes of the foot
						if(
							(CollD->CheckBlockInside(i, pos_x + dist_x*2.0f, pos_y, pos_z + dist_z*2.0f, true, NULL) == true)&&
							(CollD->CheckBlockInside(i, pos_x + dist_x*2.0f, pos_y + HUMAN_MAPCOLLISION_CLIMBHEIGHT, pos_z + dist_z*2.0f, true, NULL) == false)
						){
							flag = true;

							int face;
							blockdata bdata;

							//Get the angle of the foot
							// A block is known to exist underfoot, so DummyRay should not be needed.
							if( CollD->CheckBlockIntersectRay(i, pos_x, pos_y, pos_z, 0.0f, -1.0f, 0.0f, &face, &Dist, 1.2f) == true ){

								inblockdata->Getdata(&bdata, i);

								//If standing on the slope, the rise is disabled
								if( acosf(bdata.material[face].vy) > HUMAN_MAPCOLLISION_SLOPEANGLE ){
									flag = false;
									break;
								}
							}
						}
					}
				}

				//Climb
				if( flag == true ){
					pos_y += HUMAN_MAPCOLLISION_CLIMBADDY;
					move_y *= 0.2f;
				}
			}
		}

		//If the destination is in the block, tap the move.
		for(int i=0; i<bs; i++){
			if( CheckBlockID[i] == true ){
				// * The first point of the ray is not judged whether it is inside the block.
				if( CollD->CheckBlockIntersectRay(i, pos_x, pos_y + HUMAN_HEIGHT-2.0f, pos_z, 0.0f, -1.0f, 0.0f, NULL, &Dist, (HUMAN_HEIGHT-2.0f*2)) == true ){
					pos_x = pos_x2;
					pos_z = pos_z2;
				}

				//Cancel movement if both the tested destination and the destination position are embedded in a block.
				if( CollD->CheckBlockInside(i, pos_x, pos_y + HUMAN_HEIGHT-0.6f, pos_z, true, NULL) == true ){
					//Note: DownThis judgment is suspicious. If it is far enough, it is not necessary to be 11 times?
					// The point is beyond the AABB test range, so check all blocks again.
					if( CollD->CheckALLBlockInside(pos_x + move_x*11.0f, pos_y + HUMAN_HEIGHT-0.6f, pos_z + move_z*11.0f) == true ){
						pos_x = pos_x2;
						pos_y = pos_y2;
						pos_z = pos_z2;
						if( move_y > 0.0f ){ move_y = 0.0f; }
					}
				}
			}
		}
	}

	if( move_y_upper > 0 ) { move_y_upper -= 1; }

	//Drop processing
	move_y_flag = false;
	FallFlag = false;
	for(int ycnt=0; ycnt<3; ycnt++){
		int cnt;
		float ang = atan2f(move_z, move_x);
		float x, y, z;

		//Falling
		pos_y += move_y*0.33f;

		if( player == true ){
			y = pos_y + HUMAN_MAPCOLLISION_GROUND_HEIGHT;

			//4-directional judgment No.1
			cnt = 0;
			x = pos_x + cosf(ang)*HUMAN_MAPCOLLISION_GROUND_R1;
			z = pos_z + sinf(ang)*HUMAN_MAPCOLLISION_GROUND_R1;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ cnt += 1; break; }
				}
			}
			x = pos_x - cosf(ang)*HUMAN_MAPCOLLISION_GROUND_R1;
			z = pos_z - sinf(ang)*HUMAN_MAPCOLLISION_GROUND_R1;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ cnt += 1; break; }
				}
			}
			x = pos_x + cosf(ang + (float)M_PI/2)*HUMAN_MAPCOLLISION_GROUND_R1;
			z = pos_z + sinf(ang + (float)M_PI/2)*HUMAN_MAPCOLLISION_GROUND_R1;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ cnt += 1; break; }
				}
			}
			x = pos_x + cosf(ang - (float)M_PI/2)*HUMAN_MAPCOLLISION_GROUND_R1;
			z = pos_z + sinf(ang - (float)M_PI/2)*HUMAN_MAPCOLLISION_GROUND_R1;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ cnt += 1; break; }
				}
			}
			if( cnt == 4 ){ FallFlag = true; break; }

			//4-way judgment No.2
			cnt = 0;
			x = pos_x + cosf(ang)*HUMAN_MAPCOLLISION_GROUND_R2;
			z = pos_z + sinf(ang)*HUMAN_MAPCOLLISION_GROUND_R2;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ cnt += 1; break; }
				}
			}
			x = pos_x - cosf(ang)*HUMAN_MAPCOLLISION_GROUND_R2;
			z = pos_z - sinf(ang)*HUMAN_MAPCOLLISION_GROUND_R2;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ cnt += 1; break; }
				}
			}
			x = pos_x + cosf(ang + (float)M_PI/2)*HUMAN_MAPCOLLISION_GROUND_R2;
			z = pos_z + sinf(ang + (float)M_PI/2)*HUMAN_MAPCOLLISION_GROUND_R2;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ cnt += 1; break; }
				}
			}
			x = pos_x + cosf(ang - (float)M_PI/2)*HUMAN_MAPCOLLISION_GROUND_R2;
			z = pos_z + sinf(ang - (float)M_PI/2)*HUMAN_MAPCOLLISION_GROUND_R2;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ cnt += 1; break; }
				}
			}
			if( cnt == 4 ){ FallFlag = true; break; }
		}
		else{
			y = pos_y + HUMAN_MAPCOLLISION_GROUND_HEIGHT;

			//Check directly below.
			x = pos_x;
			z = pos_z;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ FallFlag = true; break; }
				}
			}
			if( FallFlag == true ){ break; }

			//Determine a little outside
			x = pos_x + cosf(ang)*HUMAN_MAPCOLLISION_GROUND_R3;
			z = pos_z + sinf(ang)*HUMAN_MAPCOLLISION_GROUND_R3;
			for(int i=0; i<bs; i++){
				if( CheckBlockID[i] == true ){
					if( CollD->CheckBlockInside(i, x, y, z, true, NULL) == true ){ FallFlag = true; break; }
				}
			}
			if( FallFlag == true ){ break; }
		}

		//Drop speed addition
		move_y -= HUMAN_DAMAGE_SPEED;
		if( move_y < HUMAN_DAMAGE_MAXSPEED ){ move_y = HUMAN_DAMAGE_MAXSPEED; }

		move_y_flag = true;
	}

	move_y_landing = false;

	//Processing when grounding the ground
	if( FallFlag == true ){
		int id, face;
		blockdata bdata;
		bool flag = true;

		if( move_y < HUMAN_MAPCOLLISION_GROUND_HEIGHT ){ move_y_landing = true; }

		if( Invincible == false ){
			//Fall damage
			if( move_y < HUMAN_DAMAGE_MINSPEED ){
				hp -= GetRand(6) + (int)((float)HUMAN_DAMAGE_MAXFALL / fabsf(HUMAN_DAMAGE_MAXSPEED - (HUMAN_DAMAGE_MINSPEED)) * fabsf(move_y - (HUMAN_DAMAGE_MINSPEED)));
			}
		}

		move_y = 0.0f;

		if( (GetFlag(MoveFlag_lt, MOVEFLAG_JUMP))&&(move_y_upper == 0) ){
			move_y = HUMAN_JUMP_SPEED;

			//Motion execution
			MotionCtrl->Jump();
		}

		if( GetRand(4) == 0 ){
			if( (GetFlag(MoveFlag_lt, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT | MOVEFLAG_WALK)) == 0x00)&&(move_y_upper == 0) ){ flag = false; }
		}

		//es the original block and face number
		id = -1;
		for(int i=0; i<bs; i++){
			if( CheckBlockID[i] == true ){
				if( CollD->CheckBlockIntersectRay(i, pos_x, pos_y + 2.5f, pos_z, 0.0f, -1.0f, 0.0f, &face, &Dist, (2.5f + 1.0f)) == true ){
					id = i;
					break;
				}
			}
		}

		if( id != -1 ){
			//Memory block ID and face number
			underblock_id = id;
			underblock_face = face;

			if( flag == true ){
				inblockdata->Getdata(&bdata, id);

				//the slope if standing on the slope
				if( acosf(bdata.material[face].vy) > HUMAN_MAPCOLLISION_SLOPEANGLE ){
					move_x = bdata.material[face].vx * 1.2f;
					move_y = bdata.material[face].vy * -0.5f;
					move_z = bdata.material[face].vz * 1.2f;

					for(int i=0; i<bs; i++){
						if( CheckBlockID[i] == true ){
							if( CollD->CheckBlockInside(i, pos_x + move_x*3.0f, pos_y + move_y*3.0f, pos_z + move_z*3.0f, true, NULL) == true ){
								move_y = 0.0f;
								if( CollD->CheckBlockInside(i, pos_x + move_x*3.0f, pos_y, pos_z + move_z*3.0f, true, NULL) == true ){
									move_x = 0.0f;
									move_z = 0.0f;
									break;
								}
							}
						}
					}

					move_y_upper = 8;
				}
			}
		}
	}

	//Move vector recalculation
	dist_x = pos_x - pos_x2;
	dist_y = pos_y - pos_y2;
	dist_z = pos_z - pos_z2;

	Dist = sqrtf(dist_x*dist_x + dist_z*dist_z);
	speed = sqrtf((move_x*2)*(move_x*2) + (move_z*2)*(move_z*2));

	//Clearly undo if the amount of movement is wrong
	if( (sqrtf(dist_x*dist_x + dist_y*dist_y + dist_z*dist_z) > HUMAN_MAPCOLLISION_CHECK_MAXDIST)||(Dist - speed > 1.0f) ){
		pos_x = pos_x2;
		pos_y = pos_y2;
		pos_z = pos_z2;
	}

	if( player == true ){
		speed = VectorNormalization(&dist_x, &dist_y, &dist_z);

		//Undo if there is a block between the destination
		// * The first point of the ray is not judged whether it is inside the block.
		if( CollD->CheckALLBlockIntersectRay(pos_x2, pos_y2 + HUMAN_MAPCOLLISION_CHECK_HEIGHT, pos_z2, dist_x, dist_y, dist_z, NULL, NULL, &Dist, speed) == true ){
			pos_x = pos_x2;
			pos_y = pos_y2;
			pos_z = pos_z2;
		}
	}
}

//! @brief Calculate the vector on collision with a block.
//! @param CollD Pointer of Collision class
//! @param inblockdata Pointer of BlockDataInterface class
//! @param blockid Block ID
//! @param px X Coordinate
//! @param py Y Coordinate
//! @param pz Z Coordinates
//! @param px_old X coordinates of the previous frame
//! @param py_old Y coordinates of the previous frame
//! @param pz_old Z coordinates of the previous frame
//! @param in_vx X Coordinate to judge
//! @param in_vy Determine Y coordinates
//! @param in_vz Z coordinates to determine
//! @param mode Coordinate correction mode (0x00: Normally, 0x01: Y-coordinate rise prohibition, 0x02: Y-coordinate fixation)
//! @return success: true failure: false
bool human::CollisionBlockScratch(class Collision *CollD, class BlockDataInterface *inblockdata, int blockid, float *px, float *py, float *pz, float px_old, float py_old, float pz_old, float in_vx, float in_vy, float in_vz, int mode)
{
	if( CollD == NULL ){ return false; }
	if( inblockdata == NULL ){ return false; }
	if( (blockid < 0)||(inblockdata->GetTotaldatas() <= blockid) ){ return false; }
	//if( (*px == px_old)&&(*py == py_old)&&(*pz == pz_old) ){ return false; }

	float px2, py2, pz2;
	float vx, vy, vz;
	float dist;
	int face;
	float face_vx, face_vy, face_vz, face_angle, face_angle_per;
	float temp;

	//Notes:
	//OpenXOPS collision tests treat blocks as slightly larger to absorb numerical error.
	//Therefore, it may be caused by the knot of the block of the foot.
	//In the case of the foot judgment (mode 0x00 normal), correct the Y coordinates to be judged.
	if( mode == 0 ){
		in_vy += COLLISION_ADDSIZE;
	}

	px2 = *px;
	py2 = *py;
	pz2 = *pz;

	//Current move vector and distance calculation
	vx = *px - px_old;
	vy = *py - py_old;
	vz = *pz - pz_old;
	dist = VectorNormalization(&vx, &vy, &vz);

	//Create AABB
	float rmin_x, rmin_y, rmin_z, rmax_x, rmax_y, rmax_z;
	GetAABBRay(in_vx - vx*dist, in_vy - vy*dist, in_vz - vz*dist, vx, vy, vz, dist, &rmin_x, &rmin_y, &rmin_z, &rmax_x, &rmax_y, &rmax_z);

	//Getting the slapped block surface
	face = -1;
	if( CollD->CheckBlockAABB(blockid, rmin_x, rmin_y, rmin_z, rmax_x, rmax_y, rmax_z) == false ){ return false; } 
	if( CollD->CheckBlockIntersectRay(blockid, in_vx - vx*dist, in_vy - vy*dist, in_vz - vz*dist, vx, vy, vz, &face, &temp, dist) == false ){ return false; }

	//Seeking the corners of vectors on the surface of the block
	if( CollD->AngleVector2(blockid, face, vx, vy, vz, &face_angle) == false ){ return false; }

	//Calculate the angle ratio.
	//if( face_angle != 0.0f ){ face_angle_per = 1.0f - face_angle / ((float)M_PI); }
	if( face_angle != 0.0f ){ face_angle_per = ((float)M_PI) / face_angle - 1.0f; }
	else{ face_angle_per = 0.0f; }

	//Getting a line vector on the block surface
	blockdata bdata;
	inblockdata->Getdata(&bdata, blockid);
	face_vx = bdata.material[face].vx;
	face_vy = bdata.material[face].vy;
	face_vz = bdata.material[face].vz;

	//Calculate movement vector plus normal vector.
	float vx2, vy2, vz2;
	vx2 = vx + face_vx;
	vy2 = vy + face_vy;
	vz2 = vz + face_vz;
	VectorNormalization(&vx2, &vy2, &vz2);

	//Calculate the movement vector from the angle between the vectors.
	vx = vx2 * (1.0f-face_angle_per) + vx * face_angle_per;
	vy = vy2 * (1.0f-face_angle_per) + vy * face_angle_per;
	vz = vz2 * (1.0f-face_angle_per) + vz * face_angle_per;
	VectorNormalization(&vx, &vy, &vz);

	//If you do not move horizontally, use a normal vector
	if( (vx == 0.0f)&&(vz == 0.0f) ){
		vx = bdata.material[face].vx;
		vy = bdata.material[face].vy;
		vz = bdata.material[face].vz;
	}

	//Final position
	temp = face_angle_per * dist;
	*px = vx*temp + px_old;
	*py = vy*temp + py_old;
	*pz = vz*temp + pz_old;

	//Disable moving if the final position is inside the block
	//if( CollD->CheckALLBlockInside(*px, *py, *pz) == true ){
	if( CollD->CheckBlockInside(blockid, *px, *py, *pz, true, NULL) == true ){
		*px = px_old;
		*py = py_old;
		*pz = pz_old;
	}

	//Coordinate correction
	if( mode == 0x01 ){
		if( *py > py2 ){ *py = py2; }	//Never move above.
	}
	if( mode == 0x02 ){
		*py = py2;						//Y coordinates are not changed
	}

	return true;
}

//! @brief Run collision calculations.
//! @param CollD Collision Pointer
//! @param inblockdata BlockDataInterface Pointer
//! @param AddCollisionFlag Additional collision-detection flag.
//! @param player the player is eligible
//! @param F5mode flag of the rise function (F5 backing) (valid: true disabled: false)
//! @return No processing: 0; normal: 1; just finished falling after death: 2; stationary corpse: 3; just killed by terrain: 4.
int human::ProcessObject(class Collision *CollD, class BlockDataInterface *inblockdata, bool AddCollisionFlag, bool player, bool F5mode)
{
	if( CollD == NULL ){ return 0; }
	if( inblockdata == NULL ){ return 0; }
	if( EnableFlag == false ){ return 0; }

	//Forced rise if using F5
	if( F5mode == true ){
		move_y = 0.0f;
		pos_y += 5.0f;
	}

	if( deadstate == 5 ){ return 3; }

	int WeaponReloadMotionCnt;
	int WeaponChangeWeaponIDCnt;
	int CheckDead;

	//Weapon Switching Count
	if( selectweaponcnt > 0 ){
		selectweaponcnt -= 1;
	}
	if( changeweaponidcnt > 0 ){
		changeweaponidcnt -= 1;
	}

	//Reduce 1 if the serial count is pulled
	if( weaponshotcnt > 0 ){
		weaponshotcnt -= 1;
	}

	//If you did not trigger this frame, set the burst mode count to 0.
	if( weaponshotframe == false ){ weaponburstmodecnt = 0; }
	weaponshotframe = false;

	//Reduce 1 if the reload count stops, and execute reloading if the reload count is 0
	if( weaponreloadcnt > 0 ){
		weaponreloadcnt -= 1;
		if( weaponreloadcnt == 0 ){
			if( weapon[selectweapon] != NULL ){ weapon[selectweapon]->RunReload(); }
		}
	}

	//Get the reload/weapon-switch counter for animation.
	if( weapon[selectweapon] != NULL ){
		WeaponReloadMotionCnt = weaponreloadcnt;
		WeaponChangeWeaponIDCnt = changeweaponidcnt;
	}
	else{
		WeaponReloadMotionCnt = 0;
		WeaponChangeWeaponIDCnt = 0;
	}

	//Processing of tertiary state errors
	GunsightErrorRange();

	//Death Determination and Processes
	CheckDead = CheckAndProcessDead(CollD);
	if( CheckDead == 3 ){ return 2; }

	//if( CheckDead == 0 ){
	if( (CheckDead == 0)||(deadstate == 1) ){
		int hp_old = hp;

		//Determine progressive direction and speed
		ControlProcess();

		//Determination of the above
		CollisionMap(CollD, inblockdata, AddCollisionFlag, player);

		//If the HP is zero in this map, death by the terrain
		if( (hp_old > 0)&&(hp <= 0) ){
			return 4;
		}

		//Kill the character if this collision check leaves it below -100.0.
		if( CheckDead == 0 ){
			if( pos_y < HUMAN_DEADLINE ){
				//pos_y = HUMAN_DEADLINE;
				hp = 0;
				return 4;
			}
		}
	}


	//Get the current weapon class
	class weapon *nowweapon;
	int weapon_paramid;
	nowweapon = weapon[selectweapon];

	if( nowweapon == NULL ){		//Unarmed.
		weapon_paramid = ID_WEAPON_NONE;
	}
	else{							//Have a weapon
		nowweapon->GetParamData(&weapon_paramid, NULL, NULL);
	}

	//Motion Calculator
	MotionCtrl->ProcessObject(rotation_x, armrotation_y, weapon_paramid, WeaponReloadMotionCnt, WeaponChangeWeaponIDCnt, MoveFlag_lt, hp, player);

	if( CheckDead != 0 ){ return 3; }
	return 1;
}

//! @brief Get standard error
int human::GetGunsightErrorRange()
{
	return StateGunsightErrorRange + ReactionGunsightErrorRange;
}

//! @brief Render
//! @param d3dg Pointer to D3DGraphics.
//! @param Resource ResourceManager pointer
//! @param DrawArmOnly Draw only arms and weapons
//! @param player the player is eligible
//! @param NoModel Disable model drawing
//! @todo Seeking the position of the arm in a matrix
void human::Render(class D3DGraphics *d3dg, class ResourceManager *Resource, bool DrawArmOnly, bool player, bool NoModel)
{
	//Handle unused arguments.
	UNREFERENCED_PARAMETER(player);

	//If it is not correctly initialized, it will not be processed
	if( d3dg == NULL ){ return; }
	if( Resource == NULL ){ return; }
	if( EnableFlag == false ){ return; }

	//Motion acquisition
	int upmodel, armmodel, legmodel;
	float armry, legrx;
	MotionCtrl->GetRenderMotion(&armry, &legrx, &upmodel, &armmodel, &legmodel);

	//Get the current weapon class
	class weapon *nowweapon;
	nowweapon = weapon[selectweapon];

	if( DrawArmOnly == false ){
		float add_x, add_y, add_z;

		//Draw the upper body
		if( rotation_y != 0.0f ){		//dead or start to fall
			add_x = cosf(rotation_x*-1 - (float)M_PI/2)*sinf(rotation_y)*-1.0f;
			add_y = cosf(rotation_y)*-1.0f;
			add_z = sinf(rotation_x*-1 - (float)M_PI/2)*sinf(rotation_y)*-1.0f;
		}
		else{
			add_x = 0.0f;
			add_y = -1.0f;
			add_z = 0.0f;
		}
		d3dg->SetWorldTransform(pos_x + add_x, pos_y + add_y, pos_z + add_z, rotation_x + (float)M_PI, rotation_y, upmodel_size);
		d3dg->RenderModel(upmodel, id_texture, DarkModelFlag, false, NoModel);

		//Render the legs.
		if( rotation_y != 0.0f ){		//dead or start to fall
			add_x = cosf(rotation_x*-1 - (float)M_PI/2)*sinf(rotation_y)*-0.2f;
			add_y = cosf(rotation_y)*-0.2f;
			add_z = sinf(rotation_x*-1 - (float)M_PI/2)*sinf(rotation_y)*-0.2f;
		}
		else{
			add_x = 0.0f;
			add_y = -0.2f;
			add_z = 0.0f;
		}
		d3dg->SetWorldTransform(pos_x + add_x, pos_y + add_y, pos_z + add_z, legrx + (float)M_PI, rotation_y, legmodel_size);
		d3dg->RenderModel(legmodel, id_texture, DarkModelFlag, false, NoModel);
	}

	//Draw arms
	if( rotation_y != 0.0f ){		//dead or start to fall
		float add_x = cosf(rotation_x*-1 - (float)M_PI/2)*sinf(rotation_y)*16.0f;
		float add_y = cosf(rotation_y)*16.0f;
		float add_z = sinf(rotation_x*-1 - (float)M_PI/2)*sinf(rotation_y)*16.0f;
		d3dg->SetWorldTransform(pos_x + add_x, pos_y + add_y, pos_z + add_z, rotation_x + (float)M_PI, armrotation_y + rotation_y, armmodel_size);
		d3dg->RenderModel(armmodel, id_texture, DarkModelFlag, false, NoModel);
	}
	else if( nowweapon == NULL ){	//Unarmed.
		d3dg->SetWorldTransform(pos_x, pos_y + 16.0f, pos_z, rotation_x + (float)M_PI, armry, armmodel_size);
		d3dg->RenderModel(armmodel, id_texture, DarkModelFlag, false, NoModel);
	}
	else{							//Have a weapon
		//Get weapon models and textures
		int id_param;
		WeaponParameter paramdata;
		int model, texture;
		nowweapon->GetParamData(&id_param, NULL, NULL);
		Param->GetWeapon(id_param, &paramdata);
		Resource->GetWeaponModelTexture(id_param, &model, &texture);

		//Draw arms
		d3dg->SetWorldTransform(pos_x, pos_y + 16.0f, pos_z, rotation_x + (float)M_PI, armry, armmodel_size);
		d3dg->RenderModel(armmodel, id_texture, DarkModelFlag, false, NoModel);

		//Draw weapons
		d3dg->SetWorldTransformHumanWeapon(pos_x, pos_y + 16.0f, pos_z, paramdata.mx/10*-1, paramdata.my/10, paramdata.mz/10*-1, rotation_x + (float)M_PI, armry, paramdata.size);
		d3dg->RenderModel(model, texture, DarkModelFlag, false, NoModel);
	}
}

//! @brief Constructor
weapon::weapon(class ParameterInfo *in_Param, float x, float y, float z, float rx, int id_param, int nbs, bool flag)
{
	Param = in_Param;
	pos_x = x;
	pos_y = y;
	pos_z = z;
	move_x = 0.0f;
	move_y = 0.0f;
	move_z = 0.0f;
	rotation_x = rx;
	EnableFlag = flag;

	id_parameter = id_param;
	usingflag = false;
	bullets = nbs;
	Loadbullets = 0;
	motionflag = true;

	if( Param != NULL ){
		WeaponParameter ParamData;
		if( Param->GetWeapon(id_param, &ParamData) == 0 ){
			model_size = ParamData.size;
			//id_model = ParamData.id_model;
			//id_texture = ParamData.id_texture;
		}
	}
	id_model = -1;
	id_texture = -1;
}

//! @brief Destructor
weapon::~weapon()
{}

//! @brief Set coordinates and angles
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param rx Horizontal rotation.
void weapon::SetPosData(float x, float y, float z, float rx)
{
	pos_x = x;
	pos_y = y;
	pos_z = z;
	move_x = 0.0f;
	move_y = 0.0f;
	move_z = 0.0f;
	rotation_x = rx;
}

//! @brief Set the setting value
//! @param id_param Type number of weapons
//! @param lnbs Loaded ammunition count.
//! @param nbs Total ammunition count.
//! @param init Initialize objects
void weapon::SetParamData(int id_param, int lnbs, int nbs, bool init)
{
	id_parameter = id_param;
	bullets = nbs;
	Loadbullets = lnbs;

	if( init == true ){
		usingflag = false;
		motionflag = true;
	}
}

//! @brief Getting the setting value
//! @param id_param Pointer to receive weapon type number (NULL allowed)
//! @param lnbs Pointer to receive the number of bullets (NULL allowed)
//! @param nbs Pointer to receive the total number of bullets (NULL allowed)
void weapon::GetParamData(int *id_param, int *lnbs, int *nbs)
{
	if( id_param != NULL ){ *id_param = id_parameter; }
	if( lnbs != NULL ){ *lnbs = Loadbullets; }
	if( nbs != NULL ){ *nbs = bullets; }
}

//! @brief Acquisition of weapon usage
//! @return In use: true; unused: false.
bool weapon::GetUsingFlag()
{
	return usingflag;
}

//! @brief Pick Up Weapons
//! @return Success:0 failure:1
int weapon::Pickup()
{
	if( usingflag == true ){ return 1; }
	usingflag = true;
	return 0;
}

//! @brief Discarding weapons
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param rx Horizontal rotation.
//! @param speed Initial speed when discarded.
void weapon::Dropoff(float x, float y, float z, float rx, float speed)
{
	//Set the coordinates and angles to draw
	move_x = cosf(rx*-1 + (float)M_PI/2) * speed;
	move_y = 0.0f;
	move_z = sinf(rx*-1 + (float)M_PI/2) * speed;
	pos_x = x + cosf(rx*-1 + (float)M_PI/2) * 5.0f;
	pos_y = y + 16.0f + move_y;
	pos_z = z + sinf(rx*-1 + (float)M_PI/2) * 5.0f;
	rotation_x = rx + (float)M_PI;

	//Mark as unused (unequipped) and enable coordinate movement.
	usingflag = false;
	motionflag = true;
}

//! @brief Fire
//! @return Success:0 failure:1
//! @attention Interaction intervals are also considered.
//! @attention If a function fails, it is one of the conditions. "It is not satisfied with the relapse interval", "reloading", "no bullets", "no bullets" and "the type of invalid weapon is set"
int weapon::Shot()
{
	//Failed if the class is not set
	if( Param == NULL ){ return 1; }

	//Failed if there is no bullet
	if( Loadbullets == 0 ){ return 1; }

	//Getting the setting value
	WeaponParameter ParamData;
	int param_pellet;
	if( Param->GetWeapon(id_parameter, &ParamData) == 1 ){ return 1; }
	param_pellet = ParamData.pellet;

	//Failure if the number of bullets to be fired (pert count) is less than 0
	if( param_pellet <= 0 ){ return 1; }

	//If the weapon is grenades~
	if( id_parameter == ID_WEAPON_GRENADE ){
		//Decrease ammunition.
		bullets -= 1;
		Loadbullets -= 1;

		if( (bullets - Loadbullets) <= 0 ){		//If there is no reload, the weapon will be destroyed.
			EnableFlag = false;
			usingflag = false;
		}
		else if( Loadbullets <= 0 ){			//Automatic reloading
			Loadbullets = 1;
		}
		return 0;
	}

	//Decrease ammunition.
	Loadbullets -= 1;
	bullets -= 1;
	return 0;
}

//! @brief Start reloading
//! @return Success:0 failure:1
//! @attention Reload time is also considered.
//! @attention If a function fails, it is one of the conditions. "No bullets" "The type of invalid weapon is set"
int weapon::StartReload()
{
	//Failed if the class is not set
	if( Param == NULL ){ return 1; }

	//Failed if there is no bullet
	if( (bullets - Loadbullets) == 0 ){ return 1; }
	return 0;
}

//! @brief Run Reload
//! @attention Unlike StartReload(), this replenishes ammunition instantly and ignores reload time.
int weapon::RunReload()
{
	//Failed if the class is not set
	if( Param == NULL ){ return 1; }

	//Failed if there is no bullet
	if( (bullets - Loadbullets) == 0 ){ return 1; }

	//Get the number of bullets to be loaded from the performance value of the weapon
	WeaponParameter ParamData;
	int nbsmax = 0;
	if( Param->GetWeapon(id_parameter, &ParamData) == 0 ){
		nbsmax = ParamData.nbsmax;
	}

	if( (bullets - Loadbullets) < nbsmax ){	//If the number of bullets is higher than the remaining number of bullets,
		bullets = (bullets - Loadbullets);
		Loadbullets = bullets;
	}
	else{									//If the remaining number of bullets is higher
		bullets -= Loadbullets;
		Loadbullets = nbsmax;
	}

	return 0;
}

//! @brief Changes in weapon type and number of bullets
//! @param Resource ResourceManager pointer
//! @param id_param Type number
//! @param lnbs Loaded ammunition count.
//! @param nbs Total ammunition count.
//! @return success: true failure: false
//! @attention Function for the player's F6/F7 cheats. Selecting a grenade automatically replenishes ammunition.
//! @attention This function fails when it executes against un weapon objects.
bool weapon::ResetWeaponParam(class ResourceManager *Resource, int id_param, int lnbs, int nbs)
{
	//failure if not initialized
	if( EnableFlag == false ){ return false; }
	if( Resource == NULL ){ return false; }

	//Overwrite to the specified setting value
	id_parameter = id_param;
	bullets = nbs;
	Loadbullets = lnbs;

	//If the grenades are grenades, auto reload
	if( id_param == ID_WEAPON_GRENADE ){
		if( (bullets > 0)&&(Loadbullets == 0) ){
			Loadbullets = 1;
		}
	}

	//Set the model and texture
	Resource->GetWeaponModelTexture(id_param, &id_model, &id_texture);
	WeaponParameter param;
	if( Param->GetWeapon(id_param, &param) == 0 ){
		model_size = param.size;
	}

	return true;
}

//! @brief ute calculation (free fall)
//! @param CollD Collision Pointer
int weapon::ProcessObject(class Collision *CollD)
{
	//Failed if the class is not set
	if( CollD == NULL ){ return 0; }

	//failure if not initialized
	if( EnableFlag == false ){ return 0; }

	//If no one is used, the moving flag is enabled -
	if( (usingflag == false)&&(motionflag == true) ){
		float Dist;
		float maxDist;

		//Update the moving speed
		move_x *= 0.96f;
		move_z *= 0.96f;
		if( move_y > -1.8f ){
			move_y -= 0.3f;
		}

		//No processing if buried in the block
		if( CollD->CheckALLBlockInside(pos_x, pos_y, pos_z) == true ){
			motionflag = false;
			return 0;
		}

		//Seeking horizontal travel speed
		maxDist = sqrtf(move_x*move_x + move_z*move_z);
		if( maxDist < 0.1f ){
			maxDist = 0.0f;
			move_x = 0.0f;
			move_z = 0.0f;
		}

		//Do not move horizontally if it is attached to the block.
		if( maxDist > 0.0f ){
			if( CollD->CheckALLBlockIntersectDummyRay(pos_x, pos_y, pos_z, move_x/maxDist, 0, move_z/maxDist, NULL, NULL, &Dist, maxDist) == true ){
				//Don't be buried in the block
				Dist -= 0.1f;

				//Move to the coordinates in contact
				pos_x += move_x/maxDist * Dist;
				pos_z += move_z/maxDist * Dist;

				//Move to 0
				move_x = 0.0f;
				move_z = 0.0f;
			}
		}

		//Drop processing
		for(int i=0; i<2; i++){
			//If it is buried in the block, the processing is finished.
			if( CollD->CheckALLBlockInside(pos_x, pos_y + move_y*0.5f, pos_z) == true ){
				motionflag = false;
				return 0;
			}

			//Falling
			pos_y += move_y*0.5f;
		}

		//Apply the horizontal coordinates.
		pos_x += move_x;
		pos_z += move_z;
	}

	return 0;
}

//! @brief Render
//! @param d3dg Pointer to D3DGraphics.
//! @param NoModel Disable model drawing
void weapon::Render(class D3DGraphics *d3dg, bool NoModel)
{
	//Failed if the class is not set
	if( d3dg == NULL ){ return; }

	//Not initialized or used by someone
	if( EnableFlag == false ){ return; }
	if( usingflag == true ){ return; }

	//Draw weapons
	d3dg->SetWorldTransform(pos_x, pos_y, pos_z, rotation_x, 0.0f, (float)M_PI/2, model_size);
	d3dg->RenderModel(id_model, id_texture, DarkModelFlag, false, NoModel);
}

//! @brief Constructor
smallobject::smallobject(class ParameterInfo *in_Param, class MIFInterface *in_MIFdata, float x, float y, float z, float rx, int id_param, signed short int p4, bool flag)
{
	Param = in_Param;
	MIFdata = in_MIFdata;
	pos_x = x;
	pos_y = y;
	pos_z = z;
	rotation_x = rx;
	rotation_y = 0.0f;
	EnableFlag = flag;
	model_size = 5.0f;

	id_parameter = id_param;
	point_p4 = p4;

	hp = 0;		//Provisional.
	jump_rx = 0.0f;
	move_rx = 0.0f;
	add_rx = 0.0f;
	add_ry = 0.0f;
	jump_cnt = 0;

	if( Param != NULL ){
		SmallObjectParameter ParamData;
		if( Param->GetSmallObject(id_param, &ParamData) == 0 ){
			hp = ParamData.hp;
		}
	}
	id_model = -1;
	id_texture = -1;
}

//! @brief Destructor
smallobject::~smallobject()
{}

//! @brief Set up a class to manage MIF data
//! @param in_MIFdata Pointer of MIFInterface class
void smallobject::SetMIFInterfaceClass(class MIFInterface *in_MIFdata)
{
	MIFdata = in_MIFdata;
}

//! @brief Set the setting value
//! @param id_param Type number of accessories
//! @param p4 4 Parameters
//! @param init Initialize objects
void smallobject::SetParamData(int id_param, signed short int p4, bool init)
{
	rotation_y = 0.0f;
	id_parameter = id_param;
	point_p4 = p4;

	if( init == true ){
		hp = 0;		//Provisional.
		jump_rx = 0.0f;
		move_rx = 0.0f;
		add_rx = 0.0f;
		add_ry = 0.0f;
		jump_cnt = 0;

		if( (TOTAL_PARAMETERINFO_SMALLOBJECT <= id_param)&&(id_param <= (TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT -1)) ){
			if( MIFdata != NULL ){
				hp = MIFdata->GetAddSmallobjectHP(id_param - TOTAL_PARAMETERINFO_SMALLOBJECT);
			}
		}
		else{
			if( Param != NULL ){
				SmallObjectParameter ParamData;
				if( Param->GetSmallObject(id_param, &ParamData) == 0 ){
					hp = ParamData.hp;
				}
			}
		}
	}
}

//! @brief Getting the setting value
//! @param id_param Pointer to receive the type number of accessories (NULL is acceptable)
//! @param p4 Pointer to receive the 4th parameter (NULL allowed)
void smallobject::GetParamData(int *id_param, signed short int *p4)
{
	if( id_param != NULL ){ *id_param = id_parameter; }
	if( p4 != NULL ){ *p4 = point_p4; }
}

//! @brief Get health.
//! @return Health value.
int smallobject::GetHP()
{
	return hp;
}

//! @brief Move over the block
//! @param CollD Collision Pointer
//! @return Moving amount from the original coordinate (without moving at 0)
//! @attention Even if it is buried in the block, it will go through to the bottom block.
float smallobject::CollisionMap(class Collision *CollD)
{
	//Failed if the class is not set
	if( CollD == NULL ){ return 0.0f; }

	float Dist;
	SmallObjectParameter ParamData;

	//Check for collisions below.
	if( CollD->CheckALLBlockIntersectRay(pos_x, pos_y + COLLISION_ADDSIZE, pos_z, 0, -1, 0, NULL, NULL, &Dist, 1000.0f) == true ){
		//Get the collision volume size.
		if( (TOTAL_PARAMETERINFO_SMALLOBJECT <= id_parameter)&&(id_parameter <= (TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT -1)) ){
			Dist -= (float)MIFdata->GetAddSmallobjectDecide(id_parameter - TOTAL_PARAMETERINFO_SMALLOBJECT)/10.0f;
		}
		else{
			if( Param->GetSmallObject(id_parameter, &ParamData) == 0 ){
				Dist -= (float)ParamData.decide/10.0f;
			}
		}

		//Move Coordinate
		pos_y -= Dist;
		return Dist;
	}

	return 0.0f;
}

//! @brief Bullet hit
//! @param attacks Bullet damage.
void smallobject::HitBullet(int attacks)
{
	hp -= attacks;
	if( hp <= 0 ){
		Destruction();
	}
}

//! @brief The blast of the grenades hit
//! @param attacks Blast damage.
//! @attention Please make the calculation by the distance beforehand.
void smallobject::HitGrenadeExplosion(int attacks)
{
	hp -= attacks;
	if( hp <= 0 ){
		Destruction();
	}
}

//! @brief Destroy the small object.
//! @attention Normally called automatically from HitBullet() and GrenadeExplosion(); a direct call is unnecessary.
void smallobject::Destruction()
{
	//EnableFlag = false;
	//return;

	int jump;

	//Get knockback strength.
	if( (TOTAL_PARAMETERINFO_SMALLOBJECT <= id_parameter)&&(id_parameter <= (TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT -1)) ){
		jump = MIFdata->GetAddSmallobjectJump(id_parameter - TOTAL_PARAMETERINFO_SMALLOBJECT);
	}
	else{
		SmallObjectParameter paramdata;
		Param->GetSmallObject(id_parameter, &paramdata);
		jump = paramdata.jump;
	}

	//Apply knockback.
	hp = 0;
	jump_cnt = jump;

	//Posture setting
	jump_rx = DegreeToRadian(10) * GetRand(36);
	move_rx = (float)jump * 0.04243f;
	add_rx = DegreeToRadian(1) * GetRand(20);
	add_ry = DegreeToRadian(1) * GetRand(20);
}

//! @brief ute calculation (e.g. moving at breakdown)
int smallobject::ProcessObject()
{
	//If not drawn or the physical strength is not obtained, it will not be processed.
	if( EnableFlag == false ){ return 0; }
	if( hp > 0 ){ return 0; }

	int cnt;
	int jump;

	//Get knockback strength.
	if( (TOTAL_PARAMETERINFO_SMALLOBJECT <= id_parameter)&&(id_parameter <= (TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT -1)) ){
		jump = MIFdata->GetAddSmallobjectJump(id_parameter - TOTAL_PARAMETERINFO_SMALLOBJECT);
	}
	else{
		SmallObjectParameter paramdata;
		Param->GetSmallObject(id_parameter, &paramdata);
		jump = paramdata.jump;
	}
	
	//Calculate the knockback counter.
	cnt = jump - jump_cnt;

	//Coordinate and angle from posture
	pos_x += cosf(jump_rx) * move_rx;
	pos_y += jump_cnt * 0.1f;
	pos_z += sinf(jump_rx) * move_rx;
	rotation_x += add_rx;
	rotation_y += add_ry;

	jump_cnt -= 1;

	//Once you fly for 1 second, the drawing finishes
	if( cnt > (int)GAMEFPS ){
		EnableFlag = false;
		return 2;
	}

	return 1;
}

//! @brief Render
//! @param d3dg Pointer to D3DGraphics.
//! @param NoModel Disable model drawing
void smallobject::Render(D3DGraphics *d3dg, bool NoModel)
{
	//Failed if the class is not set
	if( d3dg == NULL ){ return; }

	//If it is not initialized, it will not be processed.
	if( EnableFlag == false ){ return; }

	//Render
	d3dg->SetWorldTransform(pos_x, pos_y, pos_z, rotation_x, rotation_y, model_size);
	d3dg->RenderModel(id_model, id_texture, DarkModelFlag, false, NoModel);
}

//! @brief Constructor
bullet::bullet(int modelid, int textureid)
{
	model_size = 1.0f;
	id_model = modelid;
	id_texture = textureid;
	EnableFlag = false;

	rotation_y = 0.0f;
	attacks = 0;
	penetration = 0;
	speed = 0.0f;
	teamid = 0;
	humanid = 0;
	ontargetcnt = 0.0f;
	cnt = 0;
}

//! @brief Destructor
bullet::~bullet()
{}

//! @brief Set coordinates and angles
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param rx Horizontal rotation.
//! @param ry Vertical spindle rotation
void bullet::SetPosData(float x, float y, float z, float rx, float ry)
{
	pos_x = x;
	pos_y = y;
	pos_z = z;
	rotation_x = rx;
	rotation_y = ry;
}

//! @brief Set the setting value
//! @param _attacks Attack Force
//! @param _penetration Penetration strength.
//! @param _speed Bullet speed.
//! @param _teamid Team number
//! @param _humanid Data number of people
//! @param _ontargetcnt Number of counts during life
//! @param init Initialize objects
void bullet::SetParamData(int _attacks, int _penetration, float _speed, int _teamid, int _humanid, float _ontargetcnt, bool init)
{
	attacks = _attacks;
	penetration = _penetration;
	speed = _speed;
	teamid = _teamid;
	humanid = _humanid;
	ontargetcnt = _ontargetcnt;

	if( init == true ){
		cnt = 0;
	}
}

//! @brief Get coordinates and angles
//! @param x Pointer to receive X coordinates (NULL allowed)
//! @param y Pointer to receive Y coordinates (NULL allowed)
//! @param z Pointer to receive Z coordinates (NULL allowed)
//! @param rx Pointer to receive horizontal axis rotation (NULL allowed)
//! @param ry Pointer to receive vertical axis rotation (NULL is acceptable)
void bullet::GetPosData(float *x, float *y, float *z, float *rx, float *ry)
{
	if( x != NULL ){ *x = pos_x; }
	if( y != NULL ){ *y = pos_y; }
	if( z != NULL ){ *z = pos_z; }
	if( rx != NULL ){ *rx = rotation_x; }
	if( ry != NULL ){ *ry = rotation_y; }
}

//! @brief Getting the setting value
//! @param _attacks Pointer to receive attack force (NULL allowed)
//! @param _penetration Pointer to receive penetration force (NULL allowed)
//! @param _speed Pointer to receive bullet speed (NULL available)
//! @param _teamid Pointer to receive the team number (NULL allowed)
//! @param _humanid Pointer to receive a person's data number (NULL allowed)
//! @param _ontargetcnt Pointers that receive counts during life (NULL allowed)
void bullet::GetParamData(int *_attacks, int *_penetration, float *_speed, int *_teamid, int *_humanid, float *_ontargetcnt)
{
	if( _attacks != NULL ){ *_attacks = attacks; }
	if( _penetration != NULL ){ *_penetration = penetration; }
	if( _speed != NULL ){ *_speed = speed; }
	if( _teamid != NULL ){ *_teamid = teamid; }
	if( _humanid != NULL ){ *_humanid = humanid; }
	if( _ontargetcnt != NULL ){ *_ontargetcnt = ontargetcnt; }
}

//! @brief Update bullet movement and lifetime expiration.
int bullet::ProcessObject()
{
	//If not initialized, it will not be processed
	if( EnableFlag == false ){ return 0; }

	//Disable the object if it is past the disappearance time
	if( cnt > BULLET_DESTROYFRAME ){
		EnableFlag = false;
		return 0;
	}

	//Process movement.
	pos_x += cosf(rotation_x)*cosf(rotation_y)*speed;
	pos_y += sinf(rotation_y)*speed;
	pos_z += sinf(rotation_x)*cosf(rotation_y)*speed;
	cnt += 1;

	return 0;
}

//! @brief Render
//! @param d3dg Pointer to D3DGraphics.
//! @param NoModel Disable model drawing
void bullet::Render(class D3DGraphics *d3dg, bool NoModel)
{
	//Failed if the class is not set
	if( d3dg == NULL ){ return; }

	//If it is not initialized, it will not be processed.
	if( EnableFlag == false ){ return; }

	//Do not draw if it is before moving the bullet
	// Measures to see bullets from the head
	if( cnt == 0 ){ return; }

	//Render
	d3dg->SetWorldTransform(pos_x, pos_y, pos_z, (rotation_x * -1 - (float)M_PI/2), rotation_y, model_size);
	d3dg->RenderModel(id_model, id_texture, false, false, NoModel);
}

//! @brief Constructor
grenade::grenade(int modelid, int textureid) : bullet(modelid, textureid)
{
	if( Param != NULL ){
		WeaponParameter ParamData;
		if( Param->GetWeapon(ID_WEAPON_GRENADE, &ParamData) == 0 ){
			model_size = ParamData.size;
		}
	}

	move_x = 0.0f;
	move_y = 0.0f;
	move_z = 0.0f;
}

//! @brief Destructor
grenade::~grenade()
{}

//! @brief Set coordinates and information
//! @param speed Initial speed.
//! @param _teamid Team number
//! @param _humanid Data number of people
//! @param _ontargetcnt Number of counts during life
//! @param init Initialize objects
//! @attention Run SetPosData() first.
void grenade::SetParamData(float speed, int _teamid, int _humanid, float _ontargetcnt, bool init)
{
	move_x = cosf(rotation_x) * cosf(rotation_y) * speed;
	move_y = sinf(rotation_y) * speed;
	move_z = sinf(rotation_x) * cosf(rotation_y) * speed;
	teamid = _teamid;
	humanid = _humanid;
	ontargetcnt = _ontargetcnt;

	if( init == true ){
		cnt = 0;
	}
}

//! @brief Getting the setting value
//! @param _speed Pointer to receive speed (NULL allowed)
//! @param _teamid Pointer to receive the team number (NULL allowed)
//! @param _humanid Pointer to receive a person's data number (NULL allowed)
//! @param _ontargetcnt Pointers that receive counts during life (NULL allowed)
void grenade::GetParamData(float *_speed, int *_teamid, int *_humanid, float *_ontargetcnt)
{
	if( _speed != NULL ){ *_speed = GetSpeed(); }
	if( _teamid != NULL ){ *_teamid = teamid; }
	if( _humanid != NULL ){ *_humanid = humanid; }
	if( _ontargetcnt != NULL ){ *_ontargetcnt = ontargetcnt; }
}

//! @brief Get speed
//! @return Speed
float grenade::GetSpeed()
{
	return sqrtf(move_x*move_x + move_y*move_y + move_z*move_z);
}

//! @brief Update grenade movement and explosions.
//! @return Explosion: 2; bounce/reflection: 1; otherwise: 0.
int grenade::ProcessObject(class Collision *CollD)
{
	if( CollD == NULL ){ return 0; }

	//If not initialized, it will not be processed
	if( EnableFlag == false ){ return 0; }

	//If the time passes, the object is disabled and it is displayed as "explosion".
	if( cnt > GRENADE_DESTROYFRAME ){
		EnableFlag = false;
		return 2;
	}

	//Do not process if it is still
	if( (move_x == 0.0f)&&(move_y == 0.0f)&&(move_z == 0.0f) ){
		cnt += 1;
		return 0;
	}

	int id, face;
	float Dist;
	float maxDist = sqrtf(move_x*move_x + move_y*move_y + move_z*move_z);

	//Check collisions against the map.
	if( CollD->CheckALLBlockIntersectRay(pos_x, pos_y, pos_z, move_x/maxDist, move_y/maxDist, move_z/maxDist, &id, &face, &Dist, maxDist) == true ){
		//In case of collision to 

		float Angle, Acceleration;
		float vx, vy, vz;

		//Seeking Angle
		CollD->AngleVector(id, face, move_x, move_y, move_z, &Angle);

		//Seeking Reflective s
		CollD->ReflectVector(id, face, move_x, move_y, move_z, &vx, &vy, &vz);

		//Determine the reduction rate
		Acceleration = Angle*-1 * 0.2546f + 0.7f;

		//Reflect and decelerate.
		move_x = vx * Acceleration;
		move_y = vy * Acceleration;
		move_z = vz * Acceleration;

		// *The collision time (frame) does not move.

		//Moving Speed
		move_x *= 0.98f;
		move_y = (move_y - 0.17f) * 0.98f;
		move_z *= 0.98f;

		cnt += 1;
		return 1;
	}

	//Move Coordinate
	pos_x += move_x;
	pos_y += move_y;
	pos_z += move_z;

	//Moving Speed
	move_x *= 0.98f;
	move_y = (move_y - 0.17f) * 0.98f;
	move_z *= 0.98f;

	cnt += 1;
	return 0;
}

//! @brief Render
//! @param d3dg Pointer to D3DGraphics.
//! @param NoModel Disable model drawing
void grenade::Render(class D3DGraphics *d3dg, bool NoModel)
{
	//Failed if the class is not set
	if( d3dg == NULL ){ return; }

	//If it is not initialized, it will not be processed.
	if( EnableFlag == false ){ return; }

	//Render
	d3dg->SetWorldTransform(pos_x, pos_y, pos_z, (rotation_x * -1 - (float)M_PI/2), 0.0f, (float)M_PI/2, model_size);
	d3dg->RenderModel(id_model, id_texture, DarkModelFlag, false, NoModel);
}

//! @brief Constructor
effect::effect(float x, float y, float z, float size, float rotation, int texture, int count)
{
	pos_x = x;
	pos_y = y;
	pos_z = z;
	move_x = 0.0f;
	move_y = 0.0f;
	move_z = 0.0f;
	addmove_y = 0.0f;
	rotation_x = 0.0f;
	rotation_y = 0.0f;
	rotation_texture = rotation;
	addrotation_texture = 0.0f;
	model_size = size;
	addmodel_size = 0.0f;
	id_texture = texture;
	brightness = 0;
	addbrightness = 0;
	alpha = 1.0f;
	add_alpha = 0.0f;
	cnt = count;
	type = EFFECT_NORMAL;
	if( cnt > 0 ){
		EnableFlag = true;
	}
	else{
		EnableFlag = false;
	}
}

//! @brief Destructor
effect::~effect()
{}

//! @brief Set the setting value
//! @param in_move_x X-axis travel
//! @param in_move_y Y-axis travel
//! @param in_move_z Z-axis travel
//! @param in_addmove_y Amount of Y-axis transfer
//! @param rotation Rotation Angle
//! @param addrotation Rotation Angle
//! @param size Drawing Magnification
//! @param addsize Drawing Magnification
//! @param count Counting
//! @param settype Effect type (combine Effect_Type flags).
void effect::SetParamData(float in_move_x, float in_move_y, float in_move_z, float in_addmove_y, float rotation, float addrotation, float size, float addsize, int count, int settype)
{
	move_x = in_move_x;
	move_y = in_move_y;
	move_z = in_move_z;
	addmove_y = in_addmove_y;
	rotation_x = addrotation;
	rotation_y = 0.0f;
	rotation_texture = rotation;
	addrotation_texture = addrotation;
	model_size = size;
	addmodel_size = addsize;
	cnt = count;
	type = settype;
}

//! @brief Set Texture
//! @param id Texture Data Number
//! @param in_brightness Brightness.
//! @param in_addbrightness Brightness Addition
//! @param in_alpha Transparency
//! @param in_addalpha Transparent Addition
void effect::SetTexture(int id, int in_brightness, int in_addbrightness, float in_alpha, float in_addalpha)
{
	id_texture = id;
	brightness = in_brightness;
	addbrightness = in_addbrightness;
	alpha = in_alpha;
	add_alpha = in_addalpha;
}

//! @brief Set the orientation of the horizontal and vertical axis
//! @param rx Horizontal angle to set.
//! @param ry Vertical axis setting
void effect::SetRxRy(float rx, float ry)
{
	rotation_x = rx;
	rotation_y = ry;
}

//! Get Texture Data Number
//! @return Texture Data Number
int effect::GetTextureID()
{
	return id_texture;
}

//! Get the effect moving amount
//! @param mx Pointer to receive X-axis travel
//! @param my Pointer to receive Y-axis travel
//! @param mz Pointer to receive Z-axis travel
void effect::GetMove(float *mx, float *my, float *mz)
{
	if( (mx == NULL)||(my == NULL)||(mz == NULL) ){ return; }

	*mx = move_x;
	*my = move_y;
	*mz = move_z;
}

//! Get the effect-map collision flag.
//! @return enabled: true disabled: false
bool effect::GetCollideMapFlag()
{
	if( type&EFFECT_COLLIDEMAP ){
		return true;
	}
	//else{
		return false;
	//}
}

//! @brief Update effect movement and lifetime expiration.
//! @return Processing performed: 1; last rendering frame: 2; no processing: 0.
int effect::ProcessObject()
{
	//If not initialized, it will not be processed
	if( EnableFlag == false ){ return 0; }

	//If the count is finished, set to not be processed.
	if( cnt <= 0 ){
		EnableFlag = false;
		return 2;
	}

	//Coordinate Move
	pos_x += move_x;
	pos_y += move_y;
	pos_z += move_z;
	move_y += addmove_y;

	//Rotation
	rotation_texture += addrotation_texture;

	//Enlarge.
	model_size += addmodel_size;

	//Brightness and transparency
	brightness += addbrightness;
	alpha += add_alpha;

	//If the transparency becomes negative, set to not be processed
	if( alpha <= 0.0f ){
		EnableFlag = false;
		cnt = 0;
		return 2;
	}

	//1 draw count if the count is less than 255
	if( cnt < 255 ){
		cnt -= 1;
	}
	return 1;
}

//! @brief Render
//! @param d3dg Pointer to D3DGraphics.
//! @param camera_rx Camera horizontal axis angle
//! @param camera_ry Camera vertical axis angle
//! @param NoModel Disable model drawing
void effect::Render(class D3DGraphics *d3dg, float camera_rx, float camera_ry, bool NoModel)
{
	//Failed if the class is not set
	if( d3dg == NULL ){ return; }

	//If it is not initialized, it will not be processed.
	if( EnableFlag == false ){ return; }

    const bool billboard=(type & EFFECT_NOBILLBOARD)==0;
    const float draw_rx=billboard ? -camera_rx : rotation_x;
    const float draw_ry=billboard ? camera_ry : rotation_y;
	d3dg->SetWorldTransformEffect(pos_x, pos_y, pos_z, draw_rx, draw_ry, rotation_texture, model_size);
	d3dg->RenderBoard(id_texture, alpha, NoModel);
}

//! @brief Constructor
HumanMotionControl::HumanMotionControl(class ParameterInfo *in_Param)
{
	Param = in_Param;

	id_upmodel = -1;
	for(int i=0; i<TOTAL_ARMMODE; i++){
		id_armmodel[i] = -1;
	}
	id_legmodel = -1;
	for(int i=0; i<TOTAL_WALKMODE; i++){
		id_walkmodel[i] = -1;
	}
	for(int i=0; i<TOTAL_RUNMODE; i++){
		id_runmodel[i] = -1;
	}

	reaction_y = 0.0f;
	slowarm = false;
	armmodel_rotation_y = 0.0f;
	legrotation_x = 0.0f;
	walkcnt = 0;
	runcnt = 0;
	armmodelid = -1;
	legmodelid = -1;
}

//! @brief Destructor
HumanMotionControl::~HumanMotionControl()
{}

//! @brief Register a class that manages the setting value
//! @attention to execute before using each function.
void HumanMotionControl::SetParameterInfoClass(class ParameterInfo *in_Param)
{
	Param = in_Param;
}

//! @brief Configure model data
//! @param upmodel Model of the upper body
//! @param armmodel[] Array of arm models (array size: TOTAL_ARMMODE).
//! @param legmodel Model of stationary legs.
//! @param walkmodel[] Array of arm models (array size: TOTAL_WALKMODE).
//! @param runmodel[] Array of arm models (array size: TOTAL_RUNMODE).
void HumanMotionControl::SetModel(int upmodel, int armmodel[], int legmodel, int walkmodel[], int runmodel[])
{
	id_upmodel = upmodel;
	for(int i=0; i<TOTAL_ARMMODE; i++){
		id_armmodel[i] = armmodel[i];
	}
	id_legmodel = legmodel;
	for(int i=0; i<TOTAL_WALKMODE; i++){
		id_walkmodel[i] = walkmodel[i];
	}
	for(int i=0; i<TOTAL_RUNMODE; i++){
		id_runmodel[i] = runmodel[i];
	}
}

//! @brief Initialization function
//! @param rx Rotation Angle
void HumanMotionControl::Init(float rx)
{
	reaction_y = 0.0f;
	slowarm = false;
	armmodel_rotation_y = 0.0f;
	legrotation_x = rx;
	walkcnt = 0;
	runcnt = 0;
	armmodelid = -1;
	legmodelid = -1;
}

//! @brief Pick Up Weapons
//! @param weapon_paramid Type number of weapons
void HumanMotionControl::PickupWeapon(int weapon_paramid)
{
	//Handle unused arguments.
	UNREFERENCED_PARAMETER(weapon_paramid);

	//Set arm angle (rebound)
	reaction_y = DegreeToRadian(-20);
	slowarm = true;
}

//! @brief Switching weapons (changing)
//! @param weapon_paramid Type number of weapons
void HumanMotionControl::ChangeHaveWeapon(int weapon_paramid)
{
	//Handle unused arguments.
	UNREFERENCED_PARAMETER(weapon_paramid);

	//Set arm angle (rebound)
	reaction_y = DegreeToRadian(-20);
	slowarm = true;
}

//! @brief Weapon Type Switching
//! @param weapon_paramid Type number of weapons
//! @attention This function is a dummy function for extension.
void HumanMotionControl::ChangeWeaponID(int weapon_paramid)
{
	//Handle unused arguments.
	UNREFERENCED_PARAMETER(weapon_paramid);
}

//! @brief Process firing.
//! @param weapon_paramid Type number of weapons
void HumanMotionControl::ShotWeapon(int weapon_paramid)
{
	//Conveying Rebound Arms
	if( weapon_paramid == ID_WEAPON_GRENADE ){
		reaction_y = DegreeToRadian(20);
	}
	else{
		//Get weapon settings
		WeaponParameter ParamData;
		ScopeParameter sparam;
		if( Param->GetWeapon(weapon_paramid, &ParamData) != 0 ){ return; }
		Param->GetScopeParam(ParamData.scopemode, &sparam);

		if( sparam.WeaponMotionReactionY >= 0.0f ){
			reaction_y = sparam.WeaponMotionReactionY;
		}
		else{
			reaction_y = DegreeToRadian(0.5f) * ParamData.reaction;
		}
	}
	slowarm = false;
}

//! @brief Reload
//! @param weapon_paramid Type number of weapons
//! @attention This function is a dummy function for extension.
void HumanMotionControl::ReloadWeapon(int weapon_paramid)
{
	//Handle unused arguments.
	UNREFERENCED_PARAMETER(weapon_paramid);
}

//! @brief Discarding weapons
//! @attention This function is a dummy function for extension.
void HumanMotionControl::DumpWeapon()
{
	//
}

//! @brief Jumping
//! @attention This function is a dummy function for extension.
void HumanMotionControl::Jump()
{
	//
}

//! @brief Run motion calculation
//! @attention rotation_x Rotation angle of entire body
//! @attention armrotation_y: arm rotation angle.
//! @attention weapon_paramid Weapon Type Number
//! @attention ReloadCnt weapon reload count
//! @attention ChangeWeaponIDCnt Switching count of weapon ID
//! @attention MoveFlag Flag representing the moving direction
//! @attention hp: health.
//! @attention PlayerFlag Player or not
void HumanMotionControl::ProcessObject(float rotation_x, float armrotation_y, int weapon_paramid, int ReloadCnt, int ChangeWeaponIDCnt, int MoveFlag, int hp, bool PlayerFlag)
{
	int ArmModelID;
	float move_rx;

	if( slowarm == false ){
		//Rebound by guns, etc.
		if( reaction_y > 0.0f ){
			if( reaction_y > DegreeToRadian(0.01f) ){ reaction_y *= 0.5f; }
			else{ reaction_y = 0.0f; }
		}
		if( reaction_y < 0.0f ){
			if( reaction_y < DegreeToRadian(-0.01f) ){ reaction_y *= 0.5f; }
			else{ reaction_y = 0.0f; }
		}
	}
	else{
		if( (DegreeToRadian(-2.0f) < reaction_y)&&(reaction_y < DegreeToRadian(2.0f)) ){
			reaction_y = 0.0f;
			slowarm = false;
		}
		if( reaction_y > 0.0f ){ reaction_y += DegreeToRadian(-2.0f); }
		if( reaction_y < 0.0f ){ reaction_y += DegreeToRadian(2.0f); }
	}

	//Reloading the arm angle again
	if( ReloadCnt > 0 ){
		reaction_y = ARMRAD_RELOADWEAPON;
	}

	//Weapon ID Switching Reset Arm Angle
	if( ChangeWeaponIDCnt > 0 ){
		reaction_y = ARMRAD_RELOADWEAPON;
	}

	if( weapon_paramid == ID_WEAPON_NONE ){	//Unarmed.
		if( PlayerFlag == true ){
			armmodel_rotation_y = ARMRAD_NOWEAPON;
		}
		else{
			armmodel_rotation_y = armrotation_y;
		}
		ArmModelID = 0;
	}
	else{							//Have a weapon
		//Get weapon information
		WeaponParameter paramdata;
		Param->GetWeapon(weapon_paramid, &paramdata);

		armmodel_rotation_y = 0.0f;
		ArmModelID = 0;

		//Determine the shape and angle of the arm
		if( paramdata.WeaponP == 0 ){
			armmodel_rotation_y = armrotation_y + reaction_y;
			ArmModelID = 1;
		}
		if( paramdata.WeaponP == 1 ){
			armmodel_rotation_y = armrotation_y + reaction_y;
			ArmModelID = 2;
		}
		if( paramdata.WeaponP == 2 ){
			armmodel_rotation_y = ARMRAD_NOWEAPON;
			ArmModelID = 0;
		}
	}

	//Determines the direction of progress
	if( GetFlag(MoveFlag, MOVEFLAG_WALK) ){
		move_rx = DegreeToRadian(0);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == MOVEFLAG_FORWARD ){
		move_rx = DegreeToRadian(0);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == MOVEFLAG_BACK ){
		move_rx = DegreeToRadian(180);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == MOVEFLAG_LEFT ){
		move_rx = DegreeToRadian(90);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == MOVEFLAG_RIGHT ){
		move_rx = DegreeToRadian(-90);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == (MOVEFLAG_FORWARD | MOVEFLAG_LEFT) ){
		move_rx = DegreeToRadian(45);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == (MOVEFLAG_BACK | MOVEFLAG_LEFT) ){
		move_rx = DegreeToRadian(135);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == (MOVEFLAG_BACK | MOVEFLAG_RIGHT) ){
		move_rx = DegreeToRadian(-135);
	}
	else if( GetFlag(MoveFlag, (MOVEFLAG_FORWARD | MOVEFLAG_BACK | MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) == (MOVEFLAG_FORWARD | MOVEFLAG_RIGHT) ){
		move_rx = DegreeToRadian(-45);
	}
	else{
		move_rx = 0.0f;
	}

	//Calculate the leg angle.
	if( hp <= 0 ){
		legrotation_x = rotation_x;
	}
	else{
		float move_rx2;

		//Seeking the orientation of the legs
		if( fabsf(move_rx) > DegreeToRadian(90)){
			move_rx2 = move_rx + (float)M_PI;
		}
		else{
			move_rx2 = move_rx;
		}
		move_rx2 = AngleNormalization(move_rx2);

		//Gradually turn in that direction.
		legrotation_x = legrotation_x*0.85f + (rotation_x + move_rx2*-1)*0.15f;		// 3/4 + 1/4
	}

	//Arm Modeling
	armmodelid = id_armmodel[ArmModelID];

	//Foot Modeling
	legmodelid = id_legmodel;	//Standing still.
	if( hp > 0 ){
		if( GetFlag(MoveFlag, MOVEFLAG_WALK) ){
			legmodelid = id_walkmodel[ (walkcnt/3 % TOTAL_WALKMODE) ];	//Walking.
			walkcnt += 1;
			runcnt = 0;
		}
		else{
			if( GetFlag(MoveFlag, (MOVEFLAG_LEFT | MOVEFLAG_RIGHT)) ){
				legmodelid = id_runmodel[ (runcnt/3 % TOTAL_RUNMODE) ];		//Running sideways.
			}
			if( GetFlag(MoveFlag, MOVEFLAG_FORWARD) ){
				legmodelid = id_runmodel[ (runcnt/2 % TOTAL_RUNMODE) ];		//Running forward.
			}
			if( GetFlag(MoveFlag, MOVEFLAG_BACK) ){
				legmodelid = id_runmodel[ (runcnt/4 % TOTAL_RUNMODE) ];		//Running backward.
			}
			walkcnt = 0;
			runcnt += 1;
		}
	}
}

//! @brief Motion acquisition
//! @param arm_rotation_y Pointer to get the angle of arm
//! @param leg_rotation_x Pointer to get the angle of the foot
//! @param upmodel Pointer to get the data number of the upper body model
//! @param armmodel Pointer to get the data number of the arm model
//! @param legmodel Pointer to get the data number of the model of the foot
void HumanMotionControl::GetRenderMotion(float *arm_rotation_y, float *leg_rotation_x, int *upmodel, int *armmodel, int *legmodel)
{
	if( arm_rotation_y != NULL ){ *arm_rotation_y = armmodel_rotation_y; }
	if( leg_rotation_x != NULL ){ *leg_rotation_x = legrotation_x; }

	if( upmodel != NULL ){ *upmodel = id_upmodel; }
	if( armmodel != NULL ){ *armmodel = armmodelid; }
	if( legmodel != NULL ){ *legmodel = legmodelid; }
}

//! @brief Get the walking/running animation counter.
//! @return Count Value
int HumanMotionControl::GetMoveMotionCnt()
{
	if( walkcnt > 0 ){ return walkcnt; }
	return runcnt;
}