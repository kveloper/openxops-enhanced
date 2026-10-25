//! @file resource.h
//! @brief ResourceManager class declaration

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

#ifndef RESOURCE_H
#define RESOURCE_H

#define TOTAL_UPMODE 6				//!< Total model of upper body
#define TOTAL_ARMMODE 3				//!< Arm Total Model
#define TOTAL_WALKMODE 8			//!< Total model of foot (walk)
#define TOTAL_RUNMODE 12			//!< Total model of foot (running)

#define MAX_LOADHUMANTEXTURE 10		//!< Maximum number of people loading textures

#define ENABLE_HUMANMODEL_MORPHING	//!< Enable human model morphing (unable by commenting)

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 2		//!< Select include file.
#endif
#include "main.h"

//! @brief Resource Management Class
//! @details Manage the main models, textures, and sounds, with some exceptions.
class ResourceManager
{
	class ParameterInfo *ParamInfo;				//!< Set value information class to be managed
	class D3DGraphics *d3dg;					//!< Manage drawing classes
	class SoundControl *SoundCtrl;				//!< Managed Sound Class

	int human_upmodel[TOTAL_UPMODE];			//!< Model of the upper body
	int human_armmodel[TOTAL_ARMMODE];			//!< Arm Model
	int human_legmodel;							//!< Model of stationary legs.
	int human_walkmodel[TOTAL_WALKMODE];		//!< Model of walking
	int human_runmodel[TOTAL_RUNMODE];			//!< Model of foot (run)
	int human_texture_Param[MAX_LOADHUMANTEXTURE];		//!< Human Texture Correction Number
	int human_texture_d3dg[MAX_LOADHUMANTEXTURE];		//!< Human Texture Data Number
	int weapon_model[TOTAL_PARAMETERINFO_WEAPON];		//!< Weapon Model Data Number
	int weapon_texture[TOTAL_PARAMETERINFO_WEAPON];		//!< Weapon Texture Data Number
	int weapon_sound[TOTAL_PARAMETERINFO_WEAPON];		//!< Weapon Sound Data Number
	int weapon_reloadsound;								//!< Weapon Reload Sound Data Number
	int smallobject_model[TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT];		//!< Data number of small models
	int smallobject_texture[TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT];		//!< Texture data number of small items
	int smallobject_sound[TOTAL_PARAMETERINFO_SMALLOBJECT + MAX_ADDSMALLOBJECT];		//!< Sound data number of accessories
	int bullet_model[TOTAL_PARAMETERINFO_BULLET];		//!< Model of bullets
	int bullet_texture[TOTAL_PARAMETERINFO_BULLET];		//!< Bullet Texture

	int scopetexture;			//!< Texture of scope
	int skymodel;				//!< Sky background model.
	int skytexture;				//!< Sky background texture.
	int bullet_hitsoundA;		//!< Hit Sound (A)
	int bullet_hitsoundB;		//!< Hit Sound(B)
	int bullet_humanhitsound;	//!< Hit Sound (Person)
	int bullet_passingsound;	//!< Sound through the bullet
	int grenade_bang;			//!< Grenade explosion sound.
	int grenade_cco;			//!< Hand grenades bound sound
	int effecttexture_blood;	//!< Effects and Blood Textures
	int effecttexture_mflash;	//!< Flush Texture for Effects
	int effecttexture_smoke;	//!< Effects and smoke textures
	int effecttexture_yakkyou;	//!< Effect Texture

public:
	ResourceManager();
	~ResourceManager();
	void SetParameterInfo(ParameterInfo *_ParamInfo);
	void SetD3DGraphics(D3DGraphics *_d3dg);
	void SetSoundControl(SoundControl *_SoundCtrl);
	void DestroyResource();

	int LoadHumanModel();
	void GetHumanModel(int out_upmodel[], int out_armmodel[], int *legmodel, int out_walkmodel[], int out_runmodel[]);
	void CleanupHumanModel();
	int AddHumanTexture(int id);
	int GetHumanTextures();
	int GetHumanTexture(int id);
	bool GetHumanTextureInfo(int dataid, int *ParamID, int *TextureID);
	void CleanupHumanTexture();
	int LoadWeaponModelTexture();
	int GetWeaponModelTexture(int id, int *model, int *texture);
#ifdef ENABLE_BUG_HUMANWEAPON
	int GetBugWeaponModelTexture(int id, int *model, int *texture);
#endif
	void CleanupWeaponModelTexture();
	int LoadWeaponSound();
	int GetWeaponSound(int id);
	void CleanupWeaponSound();
	int LoadSmallObjectModelTexture();
	int GetSmallObjectModelTexture(int id, int *model, int *texture);
	void CleanupSmallObjectModelTexture();
	int LoadSmallObjectSound();
	int GetSmallObjectSound(int id);
	void CleanupSmallObjectSound();
	int LoadAddSmallObject(int id, char *modelpath, char *texturepath, char *soundpath);
	void CleanupAddSmallObject(int id);
	int LoadBulletModelTexture();
	int GetBulletModelTexture(int id, int *model, int *texture);
	void CleanupBulletModelTexture();

	int LoadScopeTexture();
	int GetScopeTexture();
	void CleanupScopeTexture();
	int LoadSkyModelTexture(int id);
	void GetSkyModelTexture(int *model, int *texture);
	void CleanupSkyModelTexture();
	int LoadBulletSound();
	void GetBulletSound(int *hitsoundA, int *hitsoundB, int *humanhitsound, int *passingsound, int *grenadebang, int *grenadecco);
	void CleanupBulletSound();
	int LoadEffectTexture();
	int GetEffectBloodTexture();
	int GetEffectMflashTexture();
	int GetEffectSmokeTexture();
	int GetEffectYakkyouTexture();
	void CleanupEffectTexture();
};

#endif