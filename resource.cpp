//! @file resource.cpp
//! @brief Definition of ResourceManager class

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

#include "resource.h"

//! @brief Constructor
ResourceManager::ResourceManager()
{
	ParamInfo = NULL;
	d3dg = NULL;
	SoundCtrl = NULL;

	for(int i=0; i<TOTAL_UPMODE; i++){
		human_upmodel[i] = -1;
	}
	for(int i=0; i<TOTAL_ARMMODE; i++){
		human_armmodel[i] = -1;
	}
	human_legmodel = -1;
	for(int i=0; i<TOTAL_WALKMODE; i++){
		human_walkmodel[i] = -1;
	}
	for(int i=0; i<TOTAL_RUNMODE; i++){
		human_runmodel[i] = -1;
	}
	for(int i=0; i<MAX_LOADHUMANTEXTURE; i++){
		human_texture_Param[i] = -1;
		human_texture_d3dg[i] = -1;
	}
	for(int i=0; i<TOTAL_PARAMETERINFO_WEAPON; i++){
		weapon_model[i] = -1;
		weapon_texture[i] = -1;
		weapon_sound[i] = -1;
	}
	weapon_reloadsound = -1;
	for(int i=0; i<(TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT); i++){
		smallobject_model[i] = -1;
		smallobject_texture[i] = -1;
		smallobject_sound[i] = -1;
	}
	for(int i=0; i<TOTAL_PARAMETERINFO_BULLET; i++){
		bullet_model[i] = -1;
		bullet_texture[i] = -1;
	}

	scopetexture = -1;
	skymodel = -1;
	skytexture = -1;
	bullet_hitsoundA = -1;
	bullet_hitsoundB = -1;
	bullet_humanhitsound = -1;
	bullet_passingsound = -1;
	grenade_bang = -1;
	grenade_cco = -1;
	effecttexture_blood = -1;
	effecttexture_mflash = -1;
	effecttexture_smoke = -1;
	effecttexture_yakkyou = -1;
}

//! @brief Destructor
ResourceManager::~ResourceManager()
{
	DestroyResource();
}

//! @brief Set ParameterInfo class to reference
//! @param *_ParamInfo Pointer of ParameterInfo class
//! @attention If you do not set this function, the class itself will not function correctly.
void ResourceManager::SetParameterInfo(ParameterInfo *_ParamInfo)
{
	ParamInfo = _ParamInfo;
}

//! @brief Set D3DGraphics classes to be referenced
//! @param *_d3dg D3DGraphics class pointer
//! @attention If you do not set this function, the class itself will not function correctly.
void ResourceManager::SetD3DGraphics(D3DGraphics *_d3dg)
{
	d3dg = _d3dg;
}

//! @brief Set the SoundControl class to reference
//! @param *_SoundCtrl SoundControl class pointer
//! @attention If you do not set this function, the class itself will not function correctly.
void ResourceManager::SetSoundControl(SoundControl *_SoundCtrl)
{
	SoundCtrl = _SoundCtrl;
}

//! @brief Release resources.
//! @attention This function is automatically called, but it can be called explicitly.
void ResourceManager::DestroyResource()
{
	CleanupHumanModel();
	CleanupHumanTexture();
	CleanupWeaponModelTexture();
	CleanupWeaponSound();
	CleanupSmallObjectModelTexture();
	CleanupSmallObjectSound();

	CleanupScopeTexture();
	CleanupSkyModelTexture();
	CleanupBulletModelTexture();
	CleanupBulletSound();
	CleanupEffectTexture();
}

//! @brief Load a human model
//! @return Success:0 failure:1
int ResourceManager::LoadHumanModel()
{
	if( d3dg == NULL ){ return 1; }

	human_upmodel[0] = d3dg->LoadModel("data\\model\\up0.x");
	human_upmodel[1] = d3dg->LoadModel("data\\model\\up1.x");
	human_upmodel[2] = d3dg->LoadModel("data\\model\\up2.x");
	human_upmodel[3] = d3dg->LoadModel("data\\model\\up3.x");
	human_upmodel[4] = d3dg->LoadModel("data\\model\\up4.x");
	human_upmodel[5] = d3dg->LoadModel("data\\model\\up5.x");
	human_armmodel[0] = d3dg->LoadModel("data\\model\\arm00.x");
	human_armmodel[1] = d3dg->LoadModel("data\\model\\arm01.x");
	human_armmodel[2] = d3dg->LoadModel("data\\model\\arm02.x");
	human_legmodel = d3dg->LoadModel("data\\model\\leg.x");
	human_walkmodel[0] = d3dg->LoadModel("data\\model\\walk01.x");
	human_walkmodel[2] = d3dg->LoadModel("data\\model\\walk02.x");
	human_walkmodel[4] = d3dg->LoadModel("data\\model\\walk03.x");
	human_walkmodel[6] = d3dg->LoadModel("data\\model\\walk04.x");
	human_runmodel[0] = d3dg->LoadModel("data\\model\\run01.x");
	human_runmodel[2] = d3dg->LoadModel("data\\model\\run02.x");
	human_runmodel[4] = d3dg->LoadModel("data\\model\\run03.x");
	human_runmodel[6] = d3dg->LoadModel("data\\model\\run04.x");
	human_runmodel[8] = d3dg->LoadModel("data\\model\\run05.x");
	human_runmodel[10] = d3dg->LoadModel("data\\model\\run06.x");

#ifdef ENABLE_HUMANMODEL_MORPHING
	//Perform muffing processing
	human_walkmodel[1] = d3dg->MorphingModel(human_walkmodel[0], human_walkmodel[2]);
	human_walkmodel[3] = d3dg->MorphingModel(human_walkmodel[2], human_walkmodel[4]);
	human_walkmodel[5] = d3dg->MorphingModel(human_walkmodel[4], human_walkmodel[6]);
	human_walkmodel[7] = d3dg->MorphingModel(human_walkmodel[6], human_walkmodel[0]);
	human_runmodel[1] = d3dg->MorphingModel(human_runmodel[0], human_runmodel[2]);
	human_runmodel[3] = d3dg->MorphingModel(human_runmodel[2], human_runmodel[4]);
	human_runmodel[5] = d3dg->MorphingModel(human_runmodel[4], human_runmodel[6]);
	human_runmodel[7] = d3dg->MorphingModel(human_runmodel[6], human_runmodel[8]);
	human_runmodel[9] = d3dg->MorphingModel(human_runmodel[8], human_runmodel[10]);
	human_runmodel[11] = d3dg->MorphingModel(human_runmodel[10], human_runmodel[0]);
#else
	//Do not perform muffing processing
	human_walkmodel[1] = human_walkmodel[0];
	human_walkmodel[3] = human_walkmodel[2];
	human_walkmodel[5] = human_walkmodel[4];
	human_walkmodel[7] = human_walkmodel[6];
	human_runmodel[1] = human_runmodel[0];
	human_runmodel[3] = human_runmodel[2];
	human_runmodel[5] = human_runmodel[4];
	human_runmodel[7] = human_runmodel[6];
	human_runmodel[9] = human_runmodel[8];
	human_runmodel[11] = human_runmodel[10];
#endif

	return 0;
}

//! @brief Get the data number of a person's model
//! @param out_upmodel[] Upper-body models (array size: TOTAL_UPMODE).
//! @param out_armmodel[] Arm models (array size: TOTAL_ARMMODE).
//! @param *legmodel Pointer of the model of the foot (static state)
//! @param out_walkmodel[] Walking leg models (array size: TOTAL_WALKMODE).
//! @param out_runmodel[] Running leg models (array size: TOTAL_RUNMODE).
void ResourceManager::GetHumanModel(int out_upmodel[], int out_armmodel[], int *legmodel, int out_walkmodel[], int out_runmodel[])
{
	for(int i=0; i<TOTAL_UPMODE; i++){
		out_upmodel[i] = human_upmodel[i];
	}
	for(int i=0; i<TOTAL_ARMMODE; i++){
		out_armmodel[i] = human_armmodel[i];
	}
	*legmodel = human_legmodel;
	for(int i=0; i<TOTAL_WALKMODE; i++){
		out_walkmodel[i] = human_walkmodel[i];
	}
	for(int i=0; i<TOTAL_RUNMODE; i++){
		out_runmodel[i] = human_runmodel[i];
	}
}

//! @brief releasing human models
void ResourceManager::CleanupHumanModel()
{
	if( d3dg == NULL ){ return; }

	for(int i=0; i<TOTAL_UPMODE; i++){
		d3dg->CleanupModel(human_upmodel[i]);
		human_upmodel[i] = -1;
	}
	for(int i=0; i<TOTAL_ARMMODE; i++){
		d3dg->CleanupModel(human_armmodel[i]);
		human_armmodel[i] = -1;
	}
	d3dg->CleanupModel(human_legmodel);
	for(int i=0; i<TOTAL_WALKMODE; i++){
		d3dg->CleanupModel(human_walkmodel[i]);
		human_walkmodel[i] = -1;
	}
	for(int i=0; i<TOTAL_RUNMODE; i++){
		d3dg->CleanupModel(human_runmodel[i]);
		human_runmodel[i] = -1;
	}
}

//! @brief Read Human Texture
//! @param id Type ID of person
//! @return Success: 0 or more Failure: -1
int ResourceManager::AddHumanTexture(int id)
{
	if( d3dg == NULL ){ return -1; }
	if( (id < 0)||(TOTAL_PARAMETERINFO_HUMAN <= id ) ){ return -1; }

	HumanParameter data;
	char path[MAX_PATH];
	if( ParamInfo->GetHuman(id, &data) == 1 ){ return -1; }

	//Check if you already read the texture
	for(int i=0; i<MAX_LOADHUMANTEXTURE; i++){
		if( human_texture_Param[i] == data.texture ){
			return -1;
		}
	}

	//Read Texture
	for(int i=0; i<MAX_LOADHUMANTEXTURE; i++){
		if( human_texture_Param[i] == -1 ){
			int TexturePathID = data.texture;
			if( ParamInfo->GetHumanTexturePath(TexturePathID, path) == 1 ){ return -1; }
			human_texture_Param[i] = TexturePathID;				//Record the data ID.
			human_texture_d3dg[i] = d3dg->LoadTexture(path, false, false);	//Read Texture
			return i;
		}
	}

	return -1;
}

//! @brief Get the number of textures of the reader
//! @return Number of textures (maximum MAX_LOADHUMANTEXTURE).
int ResourceManager::GetHumanTextures()
{
	int Textures = 0;

	for(int i=0; i<MAX_LOADHUMANTEXTURE; i++){
		if( human_texture_Param[i] != -1 ){
			Textures += 1;
		}
	}

	return Textures;
}

//! @brief Get a human texture number
//! @param id Type ID of person
//! @return Success: texture ID; failure: -1.
int ResourceManager::GetHumanTexture(int id)
{
	if( (id < 0)||(TOTAL_PARAMETERINFO_HUMAN <= id ) ){ return -1; }

	HumanParameter data;
	if( ParamInfo->GetHuman(id, &data) == 1 ){ return -1; }

	for(int i=0; i<MAX_LOADHUMANTEXTURE; i++){
		if( human_texture_Param[i] == data.texture ){
			return human_texture_d3dg[i];
		}
	}

	return -1;
}

//! @brief Get the texture information of the loader
//! @param dataid Data Number
//! @param ParamID Pointer to receive a person's type ID (NULL allowed)
//! @param TextureID Pointer to receive texture ID (NULL allowed)
//! @return success: true failure: false
//! @attention Debugging function; normally unused.
bool ResourceManager::GetHumanTextureInfo(int dataid, int *ParamID, int *TextureID)
{
	if( (dataid < 0)||(MAX_LOADHUMANTEXTURE <= dataid ) ){ return false; }

	if( ParamID != NULL ){ *ParamID = human_texture_Param[dataid]; }
	if( TextureID != NULL ){ *TextureID = human_texture_d3dg[dataid]; }
	return true;
}

//! @brief Free all character textures.
void ResourceManager::CleanupHumanTexture()
{
	if( d3dg == NULL ){ return; }

	for(int i=0; i<MAX_LOADHUMANTEXTURE; i++){
		if( human_texture_Param[i] != -1 ){
			d3dg->CleanupTexture(human_texture_d3dg[i]);
			human_texture_Param[i] = -1;
			human_texture_d3dg[i] = -1;
		}
	}
}

//! @brief Load all weapon models and textures.
//! @return Success: 0 failure: 1 or more
int ResourceManager::LoadWeaponModelTexture()
{
	int cnt = 0;

	if( d3dg == NULL ){ return 1; }

	for(int i=0; i<TOTAL_PARAMETERINFO_WEAPON; i++){
		WeaponParameter data;
		if( ParamInfo->GetWeapon(i, &data) == 0 ){
			//Load the model and texture, and count if an error occurs.
			weapon_model[i] = d3dg->LoadModel(data.model);
			weapon_texture[i] = d3dg->LoadTexture(data.texture, false, false);
			if( weapon_model[i] == -1 ){ cnt += 1; }
			if( weapon_texture[i] == -1 ){ cnt += 1; }
		}
		else{
			//If the configuration data is invalid, report errors for both the model and the texture.
			cnt += 2;
		}
	}

	return cnt;
}

//! @brief Get weapon models and textures
//! @return Success:0 failure:1
int ResourceManager::GetWeaponModelTexture(int id, int *model, int *texture)
{
	if( model == NULL ){ return 1; }
	if( texture == NULL ){ return 1; }

	if( (id < 0)||(TOTAL_PARAMETERINFO_WEAPON <= id ) ){
#ifdef ENABLE_BUG_HUMANWEAPON
		return GetBugWeaponModelTexture(id, model, texture);
#else
		return 1;
#endif
	}

	*model = weapon_model[id];
	*texture = weapon_texture[id];
	return 0;
}

#ifdef ENABLE_BUG_HUMANWEAPON
//! @brief Get the model and texture of the bug weapon
//! @return Success:0 failure:1
//! @warning Use this function only when the GetWeaponModelTexture() function fails to retrieve it.
//! @attention To add a bug weapon, edit the GetBugWeapon() function of the ParameterInfo class.
int ResourceManager::GetBugWeaponModelTexture(int id, int *model, int *texture)
{
	if( d3dg == NULL ){ return 1; }
	if( model == NULL ){ return 1; }
	if( texture == NULL ){ return 1; }

	if( id == 23 ){
		*model = human_upmodel[0];
		*texture = d3dg->GetMapTextureID(0);
		return 0;
	}
	if( id == 24 ){
		*model = human_upmodel[0];
		*texture = d3dg->GetMapTextureID(3);
		return 0;
	}
	if( id == 30 ){
		*model = human_upmodel[0];
		*texture = d3dg->GetMapTextureID(2);
		return 0;
	}
	if( id == 53 ){
		*model = human_upmodel[0];
		*texture = d3dg->GetMapTextureID(8);
		return 0;
	}

	return 1;
}
#endif

//! @brief Free all weapon models and textures.
void ResourceManager::CleanupWeaponModelTexture()
{
	if( d3dg == NULL ){ return; }

	for(int i=0; i<TOTAL_PARAMETERINFO_WEAPON; i++){
		d3dg->CleanupModel(weapon_model[i]);
		d3dg->CleanupTexture(weapon_texture[i]);
		weapon_model[i] = -1;
		weapon_texture[i] = -1;
	}
}

//! @brief Load the sound of the weapon
//! @return Success:0 failure:1
int ResourceManager::LoadWeaponSound()
{
	if( SoundCtrl == NULL ){ return 1; }

	int soundid[6];
	int reloadid;

	//Load sound files together
	soundid[0] = SoundCtrl->LoadSound("data\\sound\\bang1.wav");
	soundid[1] = SoundCtrl->LoadSound("data\\sound\\bang2.wav");
	soundid[2] = SoundCtrl->LoadSound("data\\sound\\bang3.wav");
	soundid[3] = SoundCtrl->LoadSound("data\\sound\\bang4.wav");
	soundid[4] = SoundCtrl->LoadSound("data\\sound\\bang5.wav");
	soundid[5] = SoundCtrl->LoadSound("data\\sound\\ka.wav");
	//reloadid = SoundCtrl->LoadSound("reload.wav");
	reloadid = -1;

	for(int i=0; i<TOTAL_PARAMETERINFO_WEAPON; i++){
		WeaponParameter data;
		if( ParamInfo->GetWeapon(i, &data) == 0 ){
			//Assign sound effects according to the sound number.
			if( data.soundid == 0 ){
				weapon_sound[i] = soundid[0];
			}
			else if( data.soundid == 1 ){
				weapon_sound[i] = soundid[1];
			}
			else if( data.soundid == 2 ){
				weapon_sound[i] = soundid[2];
			}
			else if( data.soundid == 3 ){
				weapon_sound[i] = soundid[3];
			}
			else if( data.soundid == 4 ){
				weapon_sound[i] = soundid[4];
			}
			else if( data.soundid == 13 ){
				weapon_sound[i] = soundid[5];
			}
			else{
				weapon_sound[i] = -1;
			}
		}
	}
	weapon_reloadsound = reloadid;

	//Check whether an error occurred when reading a file
	for(int i=0; i<6; i++){
		if( soundid[i] == -1 ){ return 1; }
	}
	if( reloadid == -1 ){ return 1; }
	return 0;
}

//! @brief Get weapon sound
//! @param id Weapon data number with 0 or higher, -1 reload sound
//! @return Success: data ID; failure: -1.
int ResourceManager::GetWeaponSound(int id)
{
	if( SoundCtrl == NULL ){ return -1; }
	if( id == -1 ){ return weapon_reloadsound; }

	if( (id < 0)||(TOTAL_PARAMETERINFO_WEAPON <= id ) ){ return -1; }
	return weapon_sound[id];
}

//! @brief Free all weapon sounds.
void ResourceManager::CleanupWeaponSound()
{
	if( SoundCtrl == NULL ){ return; }

	for(int i=0; i<TOTAL_PARAMETERINFO_WEAPON; i++){
		SoundCtrl->CleanupSound(weapon_sound[i]);
		weapon_sound[i] = -1;
	}
	SoundCtrl->CleanupSound(weapon_reloadsound);
	weapon_reloadsound = -1;
}

//! @brief Load all small-object models and textures.
//! @return Success: 0 failure: 1 or more
int ResourceManager::LoadSmallObjectModelTexture()
{
	int cnt = 0;

	if( d3dg == NULL ){ return 1; }

	for(int i=0; i<TOTAL_PARAMETERINFO_SMALLOBJECT; i++){
		SmallObjectParameter data;
		if( ParamInfo->GetSmallObject(i, &data) == 0 ){
			//Load the model and texture, and count if an error occurs.
			smallobject_model[i] = d3dg->LoadModel(data.model);
			smallobject_texture[i] = d3dg->LoadTexture(data.texture, false, false);
			if( smallobject_model[i] == -1 ){ cnt += 1; }
			if( smallobject_texture[i] == -1 ){ cnt += 1; }
		}
		else{
			//If the configuration data is invalid, report errors for both the model and the texture.
			cnt += 2;
		}
	}

	return cnt;
}

//! @brief Get a small model and texture
//! @return Success:0 failure:1
int ResourceManager::GetSmallObjectModelTexture(int id, int *model, int *texture)
{
	if( model == NULL ){ return 1; }
	if( texture == NULL ){ return 1; }
	if( (id < 0)||((TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT) <= id ) ){ return 1; }

	*model = smallobject_model[id];
	*texture = smallobject_texture[id];
	return 0;
}

//! @brief Release model and texture of small items in bulk
void ResourceManager::CleanupSmallObjectModelTexture()
{
	if( d3dg == NULL ){ return; }

	for(int i=0; i<(TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT); i++){
		d3dg->CleanupModel(smallobject_model[i]);
		d3dg->CleanupTexture(smallobject_texture[i]);
		smallobject_model[i] = -1;
		smallobject_texture[i] = -1;
	}
}

//! @brief Read the sound of small items
//! @return Success:0 failure:1
int ResourceManager::LoadSmallObjectSound()
{
	if( SoundCtrl == NULL ){ return 1; }

	int soundid[2];

	//Load sound files together
	soundid[0] = SoundCtrl->LoadSound("data\\sound\\can.wav");
	soundid[1] = SoundCtrl->LoadSound("data\\sound\\dan.wav");

	for(int i=0; i<TOTAL_PARAMETERINFO_SMALLOBJECT; i++){
		SmallObjectParameter data;
		if( ParamInfo->GetSmallObject(i, &data) == 0 ){
			//Assign sound effects according to the sound number.
			if( data.sound == 0 ){
				smallobject_sound[i] = soundid[0];
			}
			else if( data.sound == 1 ){
				smallobject_sound[i] = soundid[1];
			}
			else{
				smallobject_sound[i] = -1;
			}
		}
	}

	//Check whether an error occurred when reading a file
	for(int i=0; i<2; i++){
		if( soundid[i] == -1 ){ return 1; }
	}
	return 0;
}

//! @brief Get the sound of small items
//! @param id Data number of small items
//! @return Success: data ID; failure: -1.
int ResourceManager::GetSmallObjectSound(int id)
{
	if( SoundCtrl == NULL ){ return -1; }
	if( (id < 0)||((TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT) <= id ) ){ return -1; }
	return smallobject_sound[id];
}

//! @brief Free all small-object sounds.
void ResourceManager::CleanupSmallObjectSound()
{
	if( SoundCtrl == NULL ){ return; }

	for(int i=0; i<(TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT); i++){
		SoundCtrl->CleanupSound(smallobject_sound[i]);
		smallobject_sound[i] = -1;
	}
}

//! @brief Get a model, texture and sound of additional accessories
//! @param id Data number (usually 0 fixed)
//! @param modelpath Path of model data
//! @param texturepath Texture Data Path
//! @param soundpath Sound Data Path
//! @return Success: 0 failure: 1 or more
int ResourceManager::LoadAddSmallObject(int id, char *modelpath, char *texturepath, char *soundpath)
{
	if( d3dg == NULL ){ return 1; }
	if( SoundCtrl == NULL ){ return 1; }
	if( modelpath == NULL ){ return 1; }
	if( texturepath == NULL ){ return 1; }
	if( soundpath == NULL ){ return 1; }
	if( (id < 0)||(MAX_ADDSMALLOBJECT <= id) ){ return 1; }

	int dataid = TOTAL_PARAMETERINFO_SMALLOBJECT + id;
	int cnt = 0;

	smallobject_model[dataid] = d3dg->LoadModel(modelpath);
	smallobject_texture[dataid] = d3dg->LoadTexture(texturepath, false, false);
	if( smallobject_model[dataid] == -1 ){ cnt += 1; }
	if( smallobject_texture[dataid] == -1 ){ cnt += 1; }
	smallobject_sound[dataid] = SoundCtrl->LoadSound(soundpath);
	if( smallobject_sound[dataid] == -1 ){ cnt += 1; }

	return cnt;
}

//! @brief Releasing the model, texture and sound of additional accessories
//! @param id Data number (usually 0 fixed)
void ResourceManager::CleanupAddSmallObject(int id)
{
	if( d3dg == NULL ){ return; }
	if( SoundCtrl == NULL ){ return; }
	if( (id < 0)||(MAX_ADDSMALLOBJECT <= id) ){ return; }

	int dataid = TOTAL_PARAMETERINFO_SMALLOBJECT + id;

	d3dg->CleanupModel(smallobject_model[dataid]);
	d3dg->CleanupTexture(smallobject_texture[dataid]);
	SoundCtrl->CleanupSound(smallobject_sound[dataid]);
}

//! @brief Load the model and texture of the bullet
//! @return Success: 0 failure: 1 or more
int ResourceManager::LoadBulletModelTexture()
{
	int cnt = 0;

	if( d3dg == NULL ){ return 1; }

	for(int i=0; i<TOTAL_PARAMETERINFO_BULLET; i++){
		BulletParameter data;
		if( ParamInfo->GetBullet(i, &data) == 0 ){
			//Load the model and texture, and count if an error occurs.
			bullet_model[i] = d3dg->LoadModel(data.model);
			bullet_texture[i] = d3dg->LoadTexture(data.texture, false, false);
			if( bullet_model[i] == -1 ){ cnt += 1; }
			if( bullet_texture[i] == -1 ){ cnt += 1; }
		}
		else{
			//If the configuration data is invalid, report errors for both the model and the texture.
			cnt += 2;
		}
	}

	return cnt;
}

//! @brief Get the model and texture of the bullet
//! @return Success:0 failure:1
int ResourceManager::GetBulletModelTexture(int id, int *model, int *texture)
{
	if( model == NULL ){ return 1; }
	if( texture == NULL ){ return 1; }
	if( (id < 0)||(TOTAL_PARAMETERINFO_BULLET <= id ) ){ return 1; }

	*model = bullet_model[id];
	*texture = bullet_texture[id];

	return 0;
}

//! @brief Release the model and texture of the bullet
void ResourceManager::CleanupBulletModelTexture()
{
	if( d3dg == NULL ){ return; }

	for(int i=0; i<TOTAL_PARAMETERINFO_BULLET; i++){
		d3dg->CleanupModel(bullet_model[i]);
		bullet_model[i] = -1;

		d3dg->CleanupTexture(bullet_texture[i]);
		bullet_texture[i] = -1;
	}
}

//! @brief Load scope texture
//! @return Success:0 failure:1
int ResourceManager::LoadScopeTexture()
{
	if( d3dg == NULL ){ return 1; }

	scopetexture = d3dg->LoadTexture("data\\scope.dds", false, false);
	if( scopetexture == -1 ){ return 1; }
	return 0;
}

//! @brief Get a scope texture
//! @return Texture Data Number
int ResourceManager::GetScopeTexture()
{
	return scopetexture;
}

//! @brief Release scope texture
void ResourceManager::CleanupScopeTexture()
{
	if( d3dg == NULL ){ return; }
	if( scopetexture == -1 ){ return; }

	d3dg->CleanupTexture(scopetexture);
	scopetexture = -1;
}

//! @brief Read the background empty texture
//! @param id Empty number(1 to 5)
//! @return Success:0 failure:1
int ResourceManager::LoadSkyModelTexture(int id)
{
	if( d3dg == NULL ){ return 1; }
	if( (id < 1)||(5 < id) ){ return 1; }

	//Load model (fixed)
	skymodel = d3dg->LoadModel("data\\sky\\sky.x");

	//Generate the texture filename and load it.
	char path[MAX_PATH];
	sprintf(path, "data\\sky\\sky%d.bmp", id);
	skytexture = d3dg->LoadTexture(path, false, false);

	if( skymodel == -1 ){ return 1; }
	if( skytexture == -1 ){ return 1; }
	return 0;
}

//! @brief Get background empty models and textures
void ResourceManager::GetSkyModelTexture(int *model, int *texture)
{
	if( model == NULL ){ return; }
	if( texture == NULL ){ return; }

	*model = skymodel;
	*texture = skytexture;
}

//! @brief Free background empty models and textures
void ResourceManager::CleanupSkyModelTexture()
{
	if( d3dg == NULL ){ return; }

	if( skymodel != -1 ){
		d3dg->CleanupModel(skymodel);
		skymodel = -1;
	}
	if( skytexture != -1 ){
		d3dg->CleanupTexture(skytexture);
		skytexture = -1;
	}
}

//! @brief Read the sound of bullets and grenades
//! @return Success:0 failure:1
int ResourceManager::LoadBulletSound()
{
	if( SoundCtrl == NULL ){ return 1; }

	bullet_hitsoundA = SoundCtrl->LoadSound("data\\sound\\hit1.wav");
	bullet_hitsoundB = SoundCtrl->LoadSound("data\\sound\\hit3.wav");
	bullet_humanhitsound = SoundCtrl->LoadSound("data\\sound\\hit2.wav");
	bullet_passingsound = SoundCtrl->LoadSound("data\\sound\\hyu.wav");
	grenade_bang = SoundCtrl->LoadSound("data\\sound\\bang.wav");
	grenade_cco = SoundCtrl->LoadSound("data\\sound\\cco.wav");

	if( bullet_hitsoundA == -1 ){ return 1; }
	if( bullet_hitsoundB == -1 ){ return 1; }
	if( bullet_humanhitsound == -1 ){ return 1; }
	if( bullet_passingsound == -1 ){ return 1; }
	if( grenade_bang == -1 ){ return 1; }
	if( grenade_cco == -1 ){ return 1; }
	return 0;
}

//! @brief Get the sound of bullets and grenades
//! @param hitsoundA Hit Sound: A (NULL OK)
//! @param hitsoundB Hit Sound: B (NULL OK)
//! @param humanhitsound Hit Sound: People (NULL OK)
//! @param passingsound Sound (NULL OK)
//! @param grenadebang Grenade Explosive Sound (NULL allowed)
//! @param grenadecco Grenade Bound Sound (NULL allowed)
void ResourceManager::GetBulletSound(int *hitsoundA, int *hitsoundB, int *humanhitsound, int *passingsound, int *grenadebang, int *grenadecco)
{
	if( SoundCtrl == NULL ){ return; }

	if( hitsoundA != NULL ){ *hitsoundA = bullet_hitsoundA; }
	if( hitsoundB != NULL ){ *hitsoundB = bullet_hitsoundB; }
	if( humanhitsound != NULL ){ *humanhitsound = bullet_humanhitsound; }
	if( passingsound != NULL ){ *passingsound = bullet_passingsound; }
	if( grenadebang != NULL ){ *grenadebang = grenade_bang; }
	if( grenadecco != NULL ){ *grenadecco = grenade_cco; }
}

//! @brief Free all bullet and grenade sounds.
void ResourceManager::CleanupBulletSound()
{
	if( SoundCtrl == NULL ){ return; }

	SoundCtrl->CleanupSound(bullet_hitsoundA);
	SoundCtrl->CleanupSound(bullet_hitsoundB);
	SoundCtrl->CleanupSound(bullet_humanhitsound);
	SoundCtrl->CleanupSound(bullet_passingsound);
	SoundCtrl->CleanupSound(grenade_bang);
	SoundCtrl->CleanupSound(grenade_cco);

	bullet_hitsoundA = -1;
	bullet_hitsoundB = -1;
	bullet_humanhitsound = -1;
	bullet_passingsound = -1;
	grenade_bang = -1;
	grenade_cco = -1;
}

//! @brief Read Effect Textures
//! @return Success:0 failure:1
int ResourceManager::LoadEffectTexture()
{
	if( d3dg == NULL ){ return 1; }

	effecttexture_blood = d3dg->LoadTexture("data\\blood.dds", false, false);
	effecttexture_mflash = d3dg->LoadTexture("data\\mflash.dds", false, false);
	effecttexture_smoke = d3dg->LoadTexture("data\\smoke.dds", false, false);
	effecttexture_yakkyou = d3dg->LoadTexture("data\\yakkyou.dds", false, false);

	if( effecttexture_blood == -1 ){ return 1; }
	if( effecttexture_mflash == -1 ){ return 1; }
	if( effecttexture_smoke == -1 ){ return 1; }
	if( effecttexture_yakkyou == -1 ){ return 1; }
	return 0;
}

//! @brief Blood texture acquisition
//! @return Texture Data Number
int ResourceManager::GetEffectBloodTexture()
{
	return effecttexture_blood;
}

//! @brief Flush Texture
//! @return Texture Data Number
int ResourceManager::GetEffectMflashTexture()
{
	return effecttexture_mflash;
}

//! @brief Smoke texture acquisition
//! @return Texture Data Number
int ResourceManager::GetEffectSmokeTexture()
{
	return effecttexture_smoke;
}

//! @brief Get the shell-casing texture.
//! @return Texture Data Number
int ResourceManager::GetEffectYakkyouTexture()
{
	return effecttexture_yakkyou;
}

//! @brief Free effect texture
void ResourceManager::CleanupEffectTexture()
{
	if( d3dg == NULL ){ return; }

	if( effecttexture_blood != -1 ){
		d3dg->CleanupTexture(effecttexture_blood);
		effecttexture_blood = -1;
	}
	if( effecttexture_mflash != -1 ){
		d3dg->CleanupTexture(effecttexture_mflash);
		effecttexture_mflash = -1;
	}
	if( effecttexture_smoke != -1 ){
		d3dg->CleanupTexture(effecttexture_smoke);
		effecttexture_smoke = -1;
	}
	if( effecttexture_yakkyou != -1 ){
		d3dg->CleanupTexture(effecttexture_yakkyou);
		effecttexture_yakkyou = -1;
	}
}