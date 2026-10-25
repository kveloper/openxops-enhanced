//! @file gamemain.cpp
//! @brief Game main processing

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
// The in-menu license text is defined in mainmenu::CreateOptions().

#include "gamemain.h"
#include "menu-license.h"
#include "render-timing.h"

D3DGraphics d3dg;			//!< Drawing class
InputControl inputCtrl;		//!< Input acquisition class
SoundControl SoundCtrl;		//!< Sound playback class

//! Game settings
ParameterInfo GameParamInfo;

//! Resource Management
ResourceManager Resource;

SoundManager GameSound;		//!< Game effect sound playback class

BlockDataInterface BlockData;		//!< Block Data Management Class
PointDataInterface PointData;		//!< Point Data Management Class
MIFInterface MIFdata;				//!< MIF Control
AddonList GameAddon;				//!< addon List
Collision CollD;					//!< Percentage Management Class
ObjectManager ObjMgr;				//!< Object Management Class
AIcontrol HumanAI[MAX_HUMAN];		//!< AI Management Class

GameInfo GameInfoData;				//!< Game status

EventControl Event[TOTAL_EVENTLINE];	//!< Event Control Class


//! @brief Basic initialization
int InitGame(WindowControl *WindowCtrl, int mode, char *MIFpath)
{
	if( WindowCtrl == NULL ){ return 1; }

	//D3DGraphics class initialization
	if( d3dg.InitD3D(WindowCtrl, "data\\char.dds", GameConfig.GetFullscreenFlag(), GameConfig.GetLanguage()) ){
		WindowCtrl->ErrorInfo("Direct3D‚Ìì¬‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
		return 1;
	}

	//InputControl class initialization
	if( inputCtrl.InitInput(WindowCtrl) ){
		WindowCtrl->ErrorInfo("Input initialization error");
		return 1;
	}

	//SoundControl class initialization
	if( SoundCtrl.InitSound(WindowCtrl) ){
		WindowCtrl->ErrorInfo("dll open failed");
		return 1;
	}

	//Initialize the setting value
	GameParamInfo.InitInfo();

	//Initial configuration of resources
	Resource.SetParameterInfo(&GameParamInfo);
	Resource.SetD3DGraphics(&d3dg);
	Resource.SetSoundControl(&SoundCtrl);

	//Initialize resources
	Resource.LoadHumanModel();
	Resource.LoadWeaponModelTexture();
	Resource.LoadWeaponSound();
	Resource.LoadSmallObjectModelTexture();
	Resource.LoadSmallObjectSound();
	Resource.LoadScopeTexture();
	Resource.LoadBulletModelTexture();
	Resource.LoadBulletSound();
	Resource.LoadEffectTexture();

	//Input processing setting
	inputCtrl.SetMouseLimitFlag( GameConfig.GetMouseLimitFlag() );

	//Initialization of sound effects
	float volume;
	if( GameConfig.GetSoundFlag() == false ){
		volume = 0.0f;
	}
	else{
#ifdef ENABLE_ADDCONFIG
		volume = GameConfig.GetVolume();
#else
		volume = 1.0f;
#endif
	}
	GameSound.SetClass(&SoundCtrl, &Resource, &GameParamInfo);
	SoundCtrl.SetVolume(volume);

	//Initialization of object manager
	ObjMgr.SetClass(&GameParamInfo, &d3dg, &Resource, &BlockData, &PointData, &CollD, &GameSound, &MIFdata);

	if( (mode != 0)&&(MIFpath != NULL) ){
		//* If you start the mission directly, the addon list is not necessary.

		//Start Mission Settings
		GameInfoData.selectmission_id = -1;
		strcpy(GameInfoData.mifpath, MIFpath);
		GameState.SetStateMode(mode);
		if( mode == 2 ){
			//Load mif file
			if( MIFdata.LoadFiledata(MIFpath, true) != 0 ){
				WindowCtrl->ErrorInfo("briefing data open failed");
				return 1;
			}
		}
	}
	else{
		//addon List
		char str[16];
		sprintf(str, "%s\\", ADDONDIR);
		GameAddon.LoadFiledata(str);

		GameInfoData.selectmission_id = 0;
		strcpy(GameInfoData.mifpath, "");
	}

	return 0;
}

//! @brief Reset Resources
//! @return Success:0 Fail:-1 Standby:1
//! @attention Normally, please call only if the drawing process fails.
int ResetGame(WindowControl *WindowCtrl)
{
	if( WindowCtrl == NULL ){ return -1; }

	//To release the list correctly, call it in advance.
	Resource.CleanupHumanTexture();

	int ReturnCode = d3dg.ResetD3D(WindowCtrl);

	if( ReturnCode == 0 ){
		//Initialize resources
		Resource.LoadHumanModel();
		Resource.LoadWeaponModelTexture();
		Resource.LoadSmallObjectModelTexture();
		Resource.LoadScopeTexture();
		Resource.LoadBulletModelTexture();
		Resource.LoadEffectTexture();

		//WindowCtrl->ErrorInfo("Recovery...");
		return 0;
	}
	if( ReturnCode == 1 ){
		return 1;
	}
	//if( ReturnCode == 2 ){
		//WindowCtrl->ErrorInfo("Reset failed");
		//WindowCtrl->CloseWindow();
		return -1;
	//}
}

//! @brief Basic liberation processing
void CleanupGame()
{
	//Free resources
	Resource.DestroyResource();

	//Release setting value
	GameParamInfo.DestroyInfo();


	//SoundControl class release
	SoundCtrl.DestroySound();

	//InputControl class release
	inputCtrl.DestroyInput();

	//Release D3DGraphics resources.
	d3dg.DestroyD3D();
}

//! @brief Constructor
opening::opening()
{
	add_camera_x = 0.0f;
	add_camera_y = 0.0f;
	add_camera_z = 0.0f;
	add_camera_rx = 0.0f;
	add_camera_ry = 0.0f;
}

//! @brief Destructor
opening::~opening()
{}

int opening::Create()
{
	int blockflag, pointflag;

	//Block data reading
	blockflag = BlockData.LoadFiledata("data\\map10\\temp.bd1");

	//Point data reading
	pointflag = PointData.LoadFiledata("data\\map10\\op.pd1");

	//Suspended if there is an error in file loading
	if( (blockflag != 0)||(pointflag != 0) ){
		//2bit : point data open failed
		//1bit : block data open failed
		return pointflag*2 + blockflag;
	}

	//Block data initialization
	BlockData.CalculationBlockdata(false);
	d3dg->LoadMapdata(&BlockData, "data\\map10\\");
	CollD.InitCollision(&BlockData);

	//Point data initialization
	ObjMgr.LoadPointData();
	ObjMgr.SetPlayerID(MAX_HUMAN-1);	//Players who do not exist (anti-sound sound playback measures)

	//Configure additional collision detection.
	ObjMgr.SetAddCollisionFlag(false);

	//AI Settings
	for(int i=0; i<MAX_HUMAN; i++){
		HumanAI[i].SetClass(&ObjMgr, i, &BlockData, &PointData, &GameParamInfo, &CollD, GameSound);
		HumanAI[i].Init();
	}

	//Background empty reading
	Resource.LoadSkyModelTexture(1);

	//opening_banner = d3dg->LoadTexture("banner.png", true, false);

	//Sound Initialization
	GameSound->InitWorldSound();

	//center mouse cursor
	inputCtrl->MoveMouseCenter();
	framecnt = 0;

	camera_x = -5.0f;
	camera_y = 58.0f;
	camera_z = 29.0f;
	camera_rx = DegreeToRadian(206);
	camera_ry = DegreeToRadian(12);
	add_camera_x = 0.0f;
	add_camera_y = 0.0f;
	add_camera_z = 0.0f;
	add_camera_rx = 0.0f;
	add_camera_ry = 0.0f;

	GameState->NextState();
	return 0;
}

int opening::Recovery()
{
	//Block data initialization
	d3dg->LoadMapdata(&BlockData, "data\\map10\\");

	//Point data initialization
	ObjMgr.Recovery();

	//Initialize the background sky.
	Resource.CleanupSkyModelTexture();
	Resource.LoadSkyModelTexture(1);

	//opening_banner = d3dg->LoadTexture("banner.png", true, false);

	return 0;
}

void opening::Input()
{
	inputCtrl->GetInputState(false);

	if( inputCtrl->CheckKeyDown(GetEscKeycode()) ){
		GameState->PushBackSpaceKey();
	}
	else if( inputCtrl->CheckMouseButtonUpL() ){
		GameState->PushMouseButton();
	}
	else if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x0F)) ){		// [ENTER]
		//Press [ENTER] to click the mouse.
		GameState->PushMouseButton();
	}
}

//! @todo Smoothing Camera Transfer
void opening::Process()
{
	//ute object manager
	ObjMgr.Process(-1, false, false);

	//Run the AI.
	for(int i=0; i<MAX_HUMAN; i++){
		HumanAI[i].Process();
	}

	//Seeking camera work (coordinate)
	if( framecnt < (int)(4.0f*GAMEFPS) ){
		add_camera_z = 0.0f;
		add_camera_y = 0.0f;
	}
	else if( framecnt < (int)(5.0f*GAMEFPS) ){
		add_camera_z += (0.08f - add_camera_z) / 5.0f;
		add_camera_y += (-0.05f - add_camera_y) / 5.0f;
	}
	else{
		add_camera_z = 0.08f;
		add_camera_y = -0.05f;
	}
	camera_x += add_camera_x;
	camera_y += add_camera_y;
	camera_z += add_camera_z;

	//Seeking Camera Work (Rotating)
	if( framecnt < (int)(2.6f*GAMEFPS) ){
		add_camera_rx = 0.0f;
		add_camera_ry = 0.0f;
	}
	else if( framecnt < (int)(3.6f*GAMEFPS) ){
		add_camera_rx += (DegreeToRadian(0.9f) - add_camera_rx) / 5.0f;
		add_camera_ry += (DegreeToRadian(-0.6f) - add_camera_ry) / 5.0f;
	}
	else if( framecnt < (int)(5.0f*GAMEFPS) ){
		add_camera_rx = DegreeToRadian(0.9f);
		add_camera_ry = DegreeToRadian(-0.6f);
	}
	else{
		add_camera_rx *= 0.8f;
		add_camera_ry *= 0.8f;
	}
	camera_rx += add_camera_rx;
	camera_ry += add_camera_ry;

	//After 16 seconds, finish
	if( framecnt >= 16*((int)GAMEFPS) ){
		GameState->PushMouseButton();
	}

	framecnt += 1;
}

void opening::Render3D()
{
	int skymodel, skytexture;

	//Configure fog and camera
	d3dg->SetFog(true, 1);
	d3dg->SetCamera(camera_x, camera_y, camera_z, camera_rx, camera_ry, VIEWANGLE_NORMAL);

	//Draw the background sky to the camera coordinate
	d3dg->SetWorldTransform(camera_x, camera_y, camera_z, 0.0f, 0.0f, 2.0f);
	Resource.GetSkyModelTexture(&skymodel, &skytexture);
	d3dg->RenderModel(skymodel, skytexture, false, false, false);

	//Initialize Z buffer
	d3dg->ResetZbuffer();

	//Render the map.
	d3dg->ResetWorldTransform();
	d3dg->RenderMapdata(false);

	//Draw an object
	ObjMgr.Render(camera_x, camera_y, camera_z, camera_rx, camera_ry, 0, false);
}

void opening::Render2D()
{
	float effect = 0.0f;

	//float scaling_x = (float)GameConfig.GetScreenWidth() / 640;
	float scaling_y = (float)GameConfig.GetScreenHeight() / 480;
	int swidth = (int)((float)GameConfig.GetScreenWidth() / GameConfig.GetScreenHeight() * 480);

	//Blackout Settings
	if( framecnt < (int)(2.0f*GAMEFPS) ){
		effect = GetEffectAlpha(framecnt, 1.0f, 2.0f, 0.0f, true);
	}
	if( ((int)(2.0f*GAMEFPS) <= framecnt)&&(framecnt < (int)(11.0f*GAMEFPS)) ){
		effect = 0.0f;
	}
	if( ((int)(11.0f*GAMEFPS) <= framecnt)&&(framecnt < (int)(15.0f*GAMEFPS)) ){
		effect = GetEffectAlpha(framecnt, 1.0f, 4.0f, 11.0f, false);
	}
	if( (int)(15.0f*GAMEFPS) <= framecnt ){
		effect = 1.0f;
	}
	d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,effect));

	//Black edge drawing up and down
	d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth(), (int)(scaling_y * 40), d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
	d3dg->Draw2DBox(0, GameConfig.GetScreenHeight() - (int)(scaling_y * 40), GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));

	//Project Name
	if( ((int)(0.5f*GAMEFPS) < framecnt)&&(framecnt < (int)(4.0f*GAMEFPS)) ){
		char str[32];
		float effectA = 1.0f;
		sprintf(str, "%s project", GAMENAME);
		if( framecnt < (int)(1.5f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 0.5f, false); }
		if( framecnt > (int)(3.0f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 3.0f, true); }
		d3dg->Draw2DTextureFontTextCenterScaling(0, 340, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,effectA), 22, 22);
	}

	//Name of staff 1
	if( ((int)(4.5f*GAMEFPS) < framecnt)&&(framecnt < (int)(8.5f*GAMEFPS)) ){
		float effectA = 1.0f;
		if( framecnt < (int)(5.5f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 4.5f, false); }
		if( framecnt > (int)(7.5f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 7.5f, true); }
		d3dg->Draw2DTextureFontTextScaling(60, 150, "ORIGINAL", d3dg->GetColorCode(1.0f,1.0f,1.0f,effectA), 20, 20);
	}
	if( ((int)(5.0f*GAMEFPS) < framecnt)&&(framecnt < (int)(9.0f*GAMEFPS)) ){
		float effectA = 1.0f;
		if( framecnt < (int)(6.0f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 5.0f, false); }
		if( framecnt > (int)(8.0f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 8.0f, true); }
		d3dg->Draw2DTextureFontTextScaling(100, 180, "nine-two", d3dg->GetColorCode(1.0f,1.0f,1.0f,effectA), 20, 20);
		d3dg->Draw2DTextureFontTextScaling(100, 210, "TENNKUU", d3dg->GetColorCode(1.0f,1.0f,1.0f,effectA), 20, 20);
	}

	//Staff names, part 2.
	if( ((int)(7.0f*GAMEFPS) < framecnt)&&(framecnt < (int)(11.0f*GAMEFPS)) ){
		float effectA = 1.0f;
		if( framecnt < (int)(8.0f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 7.0f, false); }
		if( framecnt > (int)(10.0f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 10.0f, true); }
		d3dg->Draw2DTextureFontTextScaling(swidth - 310, 300, "REMAKE", d3dg->GetColorCode(1.0f,1.0f,1.0f,effectA), 20, 20);
	}
	if( ((int)(7.5f*GAMEFPS) < framecnt)&&(framecnt < (int)(11.5f*GAMEFPS)) ){
		float effectA = 1.0f;
		if( framecnt < (int)(8.5f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 7.5f, false); }
		if( framecnt > (int)(10.5f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 10.5f, true); }
		d3dg->Draw2DTextureFontTextScaling(swidth - 270, 330, "[-_-;](mikan)", d3dg->GetColorCode(1.0f,1.0f,1.0f,effectA), 20, 20);
		//d3dg->Draw2DTextureScaling(swidth - 230, 360, opening_banner, 200, 40, effectA);
	}

	//Game Name
	if( (int)(12.0f*GAMEFPS) <= framecnt ){	//framecnt < (int)(16.0f*GAMEFPS)
		char str[32];
		float effectA = 1.0f;
		sprintf(str, GAMENAME);
		if( framecnt < (int)(13.0f*GAMEFPS) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 1.0f, 12.0f, false); }
		if( ((int)(15.0f*GAMEFPS) < framecnt)&&(framecnt < (int)(15.5f*GAMEFPS)) ){ effectA = GetEffectAlpha(framecnt, 1.0f, 0.5f, 15.0f, true); }
		if( framecnt >= (int)(15.5f*GAMEFPS) ){ effectA = 0.0f; }
		d3dg->Draw2DTextureFontTextCenterScaling(0, (480-11)/2, str, d3dg->GetColorCode(1.0f,0.0f,0.0f,effectA), 22, 22);
	}
}

void opening::Destroy()
{
	//Block Data Release
	d3dg->CleanupMapdata();

	//Object Manager Release
	ObjMgr.Cleanup();

	//Free the background sky.
	Resource.CleanupSkyModelTexture();

	//d3dg->CleanupTexture(opening_banner);

	GameState->NextState();
}

//! @brief Constructor
mainmenu::mainmenu()
{
	demopath[0] = '\0';
	modescreen = 0;
	mainmenu_mouseX = 0;
	mainmenu_mouseY = 0;
	mainmenu_mouseY_old = mainmenu_mouseY;
	gametitle = -1;
    cursorTexture = -1;

#ifdef ENABLE_MENUOPTIONS
	for( int i=0; i<OPTIONS_P1_DATAS; i++ ){
		Options_p1LinkTextData[i].enable = false;
	}
	for( int i=0; i<OPTIONS_P2_DATAS; i++ ){
		Options_p2LinkTextData[i].enable = false;
	}
	for( int i=0; i<OPTIONS_P3_DATAS; i++ ){
		Options_p3LinkTextData[i].enable = false;
	}
	Options_p2LinkTextID = -1;
	Options_FullscreenFlag_old = GameConfig.GetFullscreenFlag();
	Options_KeyConfigID = -1;
	Options_KeyConfigCnt = 0;
    for(int i=0;i<OPTIONS_LICENSELINE;i++) licenseinfo[i]="";
#endif
}

//! @brief Destructor
mainmenu::~mainmenu()
{
#ifdef ENABLE_MENUOPTIONS

#endif
}

int mainmenu::Create()
{
	char bdata[MAX_PATH];
	char pdata[MAX_PATH];
	int blockflag, pointflag;

	//Determine and read demos
	switch( GetRand(6) ){
		case 0:
			strcpy(demopath, "data\\map2\\");
			break;
		case 1:
			strcpy(demopath, "data\\map4\\");
			break;
		case 2:
			strcpy(demopath, "data\\map5\\");
			break;
		case 3:
			strcpy(demopath, "data\\map7\\");
			break;
		case 4:
			strcpy(demopath, "data\\map8\\");
			break;
		case 5:
			strcpy(demopath, "data\\map16\\");
			break;
	}
	strcpy(bdata, demopath);
	strcat(bdata, "temp.bd1");
	strcpy(pdata, demopath);
	strcat(pdata, "demo.pd1");

	//Block data reading
	blockflag = BlockData.LoadFiledata(bdata);

	//Point data reading
	pointflag = PointData.LoadFiledata(pdata);

	//Suspended if there is an error in file loading
	if( (blockflag != 0)||(pointflag != 0) ){
		//2bit : point data open failed
		//1bit : block data open failed
		return pointflag*2 + blockflag;
	}

	//Block data initialization
	BlockData.CalculationBlockdata(false);
	d3dg->LoadMapdata(&BlockData, demopath);
	CollD.InitCollision(&BlockData);

	//Point data initialization
	ObjMgr.LoadPointData();

	//Configure additional collision detection.
	ObjMgr.SetAddCollisionFlag(false);

	//AI Settings
	for(int i=0; i<MAX_HUMAN; i++){
		HumanAI[i].SetClass(&ObjMgr, i, &BlockData, &PointData, &GameParamInfo, &CollD, GameSound);
		HumanAI[i].Init();
	}

	modescreen = 0;

	gametitle = d3dg->LoadTexture("data\\title.dds", false, false);
    cursorTexture = d3dg->LoadTexture("data\\cursor.png", false, false);

	//Sound Initialization
	GameSound->InitWorldSound();

	mainmenu_mouseX = GameConfig.GetScreenWidth()/2;
	mainmenu_mouseY = GameConfig.GetScreenHeight()/2;
	mainmenu_mouseY_old = mainmenu_mouseY;

	//Setting the mission scrollbar
	scrollbar_official.Create(MAINMENU_X+341, MAINMENU_Y+1, 19, MAINMENU_H-1-24, TOTAL_OFFICIALMISSION, TOTAL_MENUITEMS, scrollbar_official.GetScrollItem());
	scrollbar_official.SetColor(d3dg->GetColorCode(0.5f,0.5f,0.5f,0.5f), d3dg->GetColorCode(0.6f,0.6f,0.25f,1.0f), d3dg->GetColorCode(0.8f,0.8f,0.25f,1.0f),
		d3dg->GetColorCode(0.4f,0.67f,0.57f,1.0f), d3dg->GetColorCode(0.38f,0.77f,0.64f,1.0f), d3dg->GetColorCode(0.6f,0.3f,0.25f,1.0f), d3dg->GetColorCode(0.8f,0.3f,0.25f,1.0f));
	scrollbar_addon.Create(MAINMENU_X+341, MAINMENU_Y+1, 19, MAINMENU_H-1-24, GameAddon.GetTotaldatas(), TOTAL_MENUITEMS, scrollbar_addon.GetScrollItem());
	scrollbar_addon.SetColor(d3dg->GetColorCode(0.5f,0.5f,0.5f,0.5f), d3dg->GetColorCode(0.6f,0.6f,0.25f,1.0f), d3dg->GetColorCode(0.8f,0.8f,0.25f,1.0f),
		d3dg->GetColorCode(0.4f,0.67f,0.57f,1.0f), d3dg->GetColorCode(0.38f,0.77f,0.64f,1.0f), d3dg->GetColorCode(0.6f,0.3f,0.25f,1.0f), d3dg->GetColorCode(0.8f,0.3f,0.25f,1.0f));

#ifdef ENABLE_MENUOPTIONS
	CreateOptions();
#endif

	inputCtrl->MoveMouseCenter();
	framecnt = 0;

	GameState->NextState();
	return 0;
}

int mainmenu::Recovery()
{
	//Block data initialization
	d3dg->LoadMapdata(&BlockData, demopath);

	//Point data initialization
	ObjMgr.Recovery();

	gametitle = d3dg->LoadTexture("data\\title.dds", false, false);
    cursorTexture = d3dg->LoadTexture("data\\cursor.png", false, false);

	return 0;
}

void mainmenu::Input()
{
	inputCtrl->GetInputState(false);

	//Get mouse coordinates
	inputCtrl->GetMouseMovement(&mainmenu_mouseX, &mainmenu_mouseY);
	//if( mainmenu_mouseX < 0 ){ mainmenu_mouseX = 0; }
	//if( mainmenu_mouseX > GameConfig.GetScreenWidth()-1 ){ mainmenu_mouseX = GameConfig.GetScreenWidth()-1; }
	//if( mainmenu_mouseY < 0 ){ mainmenu_mouseY = 0; }
	//if( mainmenu_mouseY > GameConfig.GetScreenHeight()-1 ){ mainmenu_mouseY = GameConfig.GetScreenHeight()-1; }

	if( modescreen == 0 ){
		//Process the scroll bar and get information
		int scrollitems;
		int totalmission;
		if( GameInfoData.selectmission_id >= 0 ){
			scrollbar_official.Input(mainmenu_mouseX, mainmenu_mouseY, inputCtrl->CheckMouseButtonNowL(), mainmenu_mouseY - mainmenu_mouseY_old);
			scrollitems = scrollbar_official.GetScrollItem();
			totalmission = TOTAL_OFFICIALMISSION;
		}
		else{
			scrollbar_addon.Input(mainmenu_mouseX, mainmenu_mouseY, inputCtrl->CheckMouseButtonNowL(), mainmenu_mouseY - mainmenu_mouseY_old);
			scrollitems = scrollbar_addon.GetScrollItem();
			totalmission = GameAddon.GetTotaldatas();
		}

		//Mission Selection
		if( inputCtrl->CheckMouseButtonUpL() ){
			for(int i=0; i<TOTAL_MENUITEMS; i++){
				char name[32];
				strcpy(name, "");
				if( GameInfoData.selectmission_id >= 0 ){
					GameParamInfo.GetOfficialMission(scrollitems + i, name, NULL, NULL, NULL, NULL, NULL);

					if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+(signed)strlen(name)*20))&&(MAINMENU_Y+30 + i*30 < mainmenu_mouseY)&&(mainmenu_mouseY < MAINMENU_Y+30 + i*30 + 26) ){
						GameInfoData.selectmission_id = scrollitems + i;
						strcpy(GameInfoData.mifpath, "");
						GameState->PushMouseButton();
					}
				}
				else{
					strcpy(name, GameAddon.GetMissionName(scrollitems + i));

					if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+(signed)strlen(name)*20))&&(MAINMENU_Y+30 + i*30 < mainmenu_mouseY)&&(mainmenu_mouseY < MAINMENU_Y+30 + i*30 + 26) ){
						GameInfoData.selectmission_id = -1;
						strcpy(GameInfoData.mifpath, ADDONDIR);
						strcat(GameInfoData.mifpath, "\\");
						strcat(GameInfoData.mifpath, GameAddon.GetFileName(scrollitems + i));
						GameState->PushMouseButton();
					}
				}
			}
		}

		if( inputCtrl->CheckMouseButtonUpL() ){
			// UP
			if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+340))&&(MAINMENU_Y < mainmenu_mouseY)&&(mainmenu_mouseY < MAINMENU_Y+30) ){
				if( GameInfoData.selectmission_id >= 0 ){
					scrollbar_official.ScrollUP();
				}
				else{
					scrollbar_addon.ScrollUP();
				}
			}

			// DOWN
			if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+340))&&((MAINMENU_Y+MAINMENU_H-55) < mainmenu_mouseY)&&(mainmenu_mouseY < (MAINMENU_Y+MAINMENU_H-55+30)) ){
				if( GameInfoData.selectmission_id >= 0 ){
					scrollbar_official.ScrollDOWN();
				}
				else{
					scrollbar_addon.ScrollDOWN();
				}
			}
		}

		//Switching standard missions and add-on lists
		if( inputCtrl->CheckMouseButtonUpL() ){
			if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+340))&&((MAINMENU_Y+MAINMENU_H-25) < mainmenu_mouseY)&&(mainmenu_mouseY < (MAINMENU_Y+MAINMENU_H-2)) ){
				if( GameInfoData.selectmission_id >= 0 ){
					if( GameAddon.GetTotaldatas() > 0 ){
						GameInfoData.selectmission_id = -1;
					}
				}
				else{
					GameInfoData.selectmission_id = 0;
				}
			}
		}

#ifdef ENABLE_MENUOPTIONS
		//Option screen switch
		if( inputCtrl->CheckMouseButtonUpL() ){
			if( (MAINMENU_OPTION_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_OPTION_X+120))&&(MAINMENU_OPTION_Y < mainmenu_mouseY)&&(mainmenu_mouseY < MAINMENU_OPTION_Y+16) ){
				modescreen = 1;
			}
		}
#endif

		//Processing ESC key
		if( inputCtrl->CheckKeyDown(GetEscKeycode()) ){
			GameState->PushBackSpaceKey();
		}
	}
	else{
#ifdef ENABLE_MENUOPTIONS
		InputOptions();
#endif
	}

	mainmenu_mouseY_old = mainmenu_mouseY;
}

void mainmenu::Process()
{
	//ute object manager
	ObjMgr.Process(-1, true, false);

	//Run the AI.
	for(int i=0; i<MAX_HUMAN; i++){
		HumanAI[i].Process();
	}

	//Calculate the camera position.
	human *myHuman = ObjMgr.GetPlayerHumanObject();
	myHuman->GetPosData(&camera_x, &camera_y, &camera_z, NULL);
	camera_x -= 4.0f;
	camera_y += 22.0f;
	camera_z -= 12.0f;
	camera_rx = DegreeToRadian(45);
	camera_ry = DegreeToRadian(-25);

	if( modescreen != 0 ){
#ifdef ENABLE_MENUOPTIONS
		ProcessOptions();
#endif
	}

	framecnt += 1;
}

void mainmenu::Render3D()
{
	//Configure fog and camera
	d3dg->SetFog(true, 0);
	d3dg->SetCamera(camera_x, camera_y, camera_z, camera_rx, camera_ry, VIEWANGLE_NORMAL);

	//Initialize Z buffer
	d3dg->ResetZbuffer();

	//Render the map.
	d3dg->ResetWorldTransform();
	d3dg->RenderMapdata(false);

	//Draw an object
	ObjMgr.Render(camera_x, camera_y, camera_z, camera_rx, camera_ry, 0, false);
}

void mainmenu::Render2D()
{
	int color;
	float effect;

	int swidth = (int)((float)GameConfig.GetScreenWidth() / GameConfig.GetScreenHeight() * 480);

	//Show game version information
	d3dg->Draw2DTextureFontTextScaling(swidth - 118+1, 75+1, GAMEVERSION, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 18, 22);
	d3dg->Draw2DTextureFontTextScaling(swidth - 118, 75, GAMEVERSION, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 18, 22);

	if( modescreen == 0 ){
		//Get scrollbar information
		int scrollitems;
		int totalmission;
		bool scrolling;
		if( GameInfoData.selectmission_id >= 0 ){
			scrollitems = scrollbar_official.GetScrollItem();
			totalmission = TOTAL_OFFICIALMISSION;
			scrolling = scrollbar_official.CheckScrolling();
		}
		else{
			scrollitems = scrollbar_addon.GetScrollItem();
			totalmission = GameAddon.GetTotaldatas();
			scrolling = scrollbar_addon.CheckScrolling();
		}

		//Menu area drawing
		if( GameAddon.GetTotaldatas() > 0 ){	//addon
			d3dg->Draw2DBox(MAINMENU_X-1, MAINMENU_Y, MAINMENU_X+360, MAINMENU_Y+MAINMENU_H+1, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));
		}
		else{
			d3dg->Draw2DBox(MAINMENU_X-1, MAINMENU_Y, MAINMENU_X+360, MAINMENU_Y+MAINMENU_H-24, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));
		}

		//Scroll Bar Drawing
		if( GameInfoData.selectmission_id >= 0 ){
			scrollbar_official.Draw(d3dg);
		}
		else{
			scrollbar_addon.Draw(d3dg);
		}

		//'< UP >' display
		if( scrollitems > 0 ){
			d3dg->Draw2DTextureFontText(MAINMENU_X+1, MAINMENU_Y+1, "<  UP  >", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 25, 26);

			//Set the character color
			if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+340))&&(MAINMENU_Y < mainmenu_mouseY)&&(mainmenu_mouseY < MAINMENU_Y+30)&&(scrolling == false) ){
				color = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f);
			}
			else{
				color = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
			}

			//Show Letters
			d3dg->Draw2DTextureFontText(MAINMENU_X, MAINMENU_Y, "<  UP  >", color, 25, 26);
		}
		else{
			d3dg->Draw2DTextureFontText(MAINMENU_X+1, MAINMENU_Y+1, "<  UP  >", d3dg->GetColorCode(0.6f,0.6f,0.6f,1.0f), 25, 26);
		}

		//'< DOWN >' display
		if( scrollitems < (totalmission - TOTAL_MENUITEMS) ){
			d3dg->Draw2DTextureFontText(MAINMENU_X+1, MAINMENU_Y+MAINMENU_H-55+1, "< DOWN >", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 25, 26);

			//Set the character color
			if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+340))&&((MAINMENU_Y+MAINMENU_H-55) < mainmenu_mouseY)&&(mainmenu_mouseY < (MAINMENU_Y+MAINMENU_H-55+30))&&(scrolling == false) ){
				color = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f);
			}
			else{
				color = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
			}

			//Show Letters
			d3dg->Draw2DTextureFontText(MAINMENU_X, MAINMENU_Y+MAINMENU_H-55, "< DOWN >", color, 25, 26);
		}
		else{
			d3dg->Draw2DTextureFontText(MAINMENU_X+1, MAINMENU_Y+MAINMENU_H-55+1, "< DOWN >", d3dg->GetColorCode(0.6f,0.6f,0.6f,1.0f), 25, 26);
		}

		//Standard mission and addon switch
		if( GameInfoData.selectmission_id >= 0 ){
			//addon
			if( GameAddon.GetTotaldatas() > 0 ){
				//Set the character color
				if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+340))&&((MAINMENU_Y+MAINMENU_H-25) < mainmenu_mouseY)&&(mainmenu_mouseY < (MAINMENU_Y+MAINMENU_H-2))&&(scrolling == false) ){
					color = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f);
				}
				else{
					color = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
				}

				//Show Letters
				d3dg->Draw2DTextureFontText(MAINMENU_X+1, MAINMENU_Y+MAINMENU_H-25+1, "ADD-ON MISSIONS >>", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 17, 22);
				d3dg->Draw2DTextureFontText(MAINMENU_X, MAINMENU_Y+MAINMENU_H-25, "ADD-ON MISSIONS >>", color, 17, 22);
			}
		}
		else{
			//addon
			//if( GameAddon.GetTotaldatas() > 0 ){
				//Set the character color
				if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+340))&&((MAINMENU_Y+MAINMENU_H-25) < mainmenu_mouseY)&&(mainmenu_mouseY < (MAINMENU_Y+MAINMENU_H-2))&&(scrolling == false) ){
					color = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f);
				}
				else{
					color = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
				}

				//Show Letters
				d3dg->Draw2DTextureFontText(MAINMENU_X+1, MAINMENU_Y+MAINMENU_H-25+1, "<< STANDARD MISSIONS", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 17, 22);
				d3dg->Draw2DTextureFontText(MAINMENU_X, MAINMENU_Y+MAINMENU_H-25, "<< STANDARD MISSIONS", color, 17, 22);
			//}
		}

		//Show Mission Name
		for(int i=0; i<TOTAL_MENUITEMS; i++){
			char name[32];
			strcpy(name, "");

			//Get the mission name
			if( GameInfoData.selectmission_id >= 0 ){
				GameParamInfo.GetOfficialMission(scrollitems + i, name, NULL, NULL, NULL, NULL, NULL);
			}
			else{
				strcpy(name, GameAddon.GetMissionName(scrollitems + i));
			}

			//Set the character color
			if( (MAINMENU_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_X+(signed)strlen(name)*20))&&(MAINMENU_Y+30 + i*30 < mainmenu_mouseY)&&(mainmenu_mouseY < MAINMENU_Y+30 + i*30 + 26)&&(scrolling == false) ){
				color = d3dg->GetColorCode(1.0f,0.6f,0.6f,1.0f);
			}
			else{
				color = d3dg->GetColorCode(0.6f,0.6f,1.0f,1.0f);
			}

			//Show Letters
			d3dg->Draw2DTextureFontText(MAINMENU_X+1, MAINMENU_Y+30+1 + i*30, name, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 20, 26);
			d3dg->Draw2DTextureFontText(MAINMENU_X, MAINMENU_Y+30 + i*30, name, color, 20, 26);
		}

#ifdef ENABLE_MENUOPTIONS
		//Optional Item Link
		d3dg->Draw2DBox(MAINMENU_OPTION_X, MAINMENU_OPTION_Y, MAINMENU_OPTION_X+132+2, MAINMENU_OPTION_Y+16+2, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));

		//Set the character color
		if( (MAINMENU_OPTION_X < mainmenu_mouseX)&&(mainmenu_mouseX < (MAINMENU_OPTION_X+132+2))&&(MAINMENU_OPTION_Y < mainmenu_mouseY)&&(mainmenu_mouseY < MAINMENU_OPTION_Y+16)&&(scrolling == false) ){
			color = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f);
		}
		else{
			color = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
		}

		//Show Letters
		d3dg->Draw2DTextureFontText(MAINMENU_OPTION_X+1, MAINMENU_OPTION_Y+1, "< OPTIONS >", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(MAINMENU_OPTION_X, MAINMENU_OPTION_Y, "< OPTIONS >", color, 12, 16);
#endif	//ENABLE_MENUOPTIONS
	}
	else{
#ifdef ENABLE_MENUOPTIONS
		RenderOptions();
#endif
	}


	//Game logo mark drawing
	d3dg->Draw2DTextureScaling(20, 25, gametitle, 480, 80, 1.0f);

	//Blackout Settings
	if( framecnt < (int)(2.0f*GAMEFPS) ){
		effect = GetEffectAlpha(framecnt, 1.0f, 2.0f, 0.0f, true);
	}
	else{
		effect = 0.0f;
	}
	d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,effect));
    // The existing menu hit-test coordinates are the image's center hotspot.
    int cursorWidth=0,cursorHeight=0;
    if(d3dg->GetTextureSize(cursorTexture,&cursorWidth,&cursorHeight)==0){
        d3dg->Draw2DTexture(mainmenu_mouseX-cursorWidth/2, mainmenu_mouseY-cursorHeight/2,
            cursorTexture,cursorWidth,cursorHeight,1.0f);
    }

}

#ifdef ENABLE_MENUOPTIONS
//! @brief Initialization of the optional screen
void mainmenu::CreateOptions()
{
	//Variable Initialization
	Options_FullscreenFlag_old = GameConfig.GetFullscreenFlag();
	Options_KeyConfigID = -1;
	Options_KeyConfigCnt = 0;


	//No.1 Screen Link text
	Options_p1LinkTextData[0].enable = true;
	Options_p1LinkTextData[0].pos_x = 0;
	Options_p1LinkTextData[0].pos_y = 30;
	Options_p1LinkTextData[0].text_w = 20;
	Options_p1LinkTextData[0].text_h = 26;
	Options_p1LinkTextData[0].text_defaultcolor = d3dg->GetColorCode(0.6f,0.6f,1.0f,1.0f);
	Options_p1LinkTextData[0].text_cursorcolor = d3dg->GetColorCode(1.0f,0.6f,0.6f,1.0f);
	strcpy(Options_p1LinkTextData[0].text, "CONFIG");
	Options_p1LinkTextData[1].enable = true;
	Options_p1LinkTextData[1].pos_x = 0;
	Options_p1LinkTextData[1].pos_y = 90;
	Options_p1LinkTextData[1].text_w = 20;
	Options_p1LinkTextData[1].text_h = 26;
	Options_p1LinkTextData[1].text_defaultcolor = d3dg->GetColorCode(0.6f,0.6f,1.0f,1.0f);
	Options_p1LinkTextData[1].text_cursorcolor = d3dg->GetColorCode(1.0f,0.6f,0.6f,1.0f);
	strcpy(Options_p1LinkTextData[1].text, "SOFTWARE LICENSE");
	Options_p1LinkTextData[2].enable = true;
	Options_p1LinkTextData[2].pos_x = 0;
	Options_p1LinkTextData[2].pos_y = 150;
	Options_p1LinkTextData[2].text_w = 20;
	Options_p1LinkTextData[2].text_h = 26;
	Options_p1LinkTextData[2].text_defaultcolor = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
	Options_p1LinkTextData[2].text_cursorcolor = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f);
	strcpy(Options_p1LinkTextData[2].text, "< EXIT >");


	//No.2 screen Link text
	strcpy(Options_p2LinkTextData[0].text, "< DEFAULT >");
	strcpy(Options_p2LinkTextData[1].text, "< SAVE >");
	strcpy(Options_p2LinkTextData[2].text, "< CANCEL >");
	for(int i=0; i<3; i++){
		Options_p2LinkTextData[i].enable = true;
		Options_p2LinkTextData[i].pos_x = 350;
		Options_p2LinkTextData[i].pos_y = 280 + i*30;
		Options_p2LinkTextData[i].text_w = 12;
		Options_p2LinkTextData[i].text_h = 16;
		Options_p2LinkTextData[i].text_defaultcolor = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
		Options_p2LinkTextData[i].text_cursorcolor = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f);
	}

	Options_p2LinkTextData[3].pos_x = 350+20;
	Options_p2LinkTextData[3].pos_y = 20;
	strcpy(Options_p2LinkTextData[3].text, "[<<]");
	Options_p2LinkTextData[4].pos_x = 350+120;
	Options_p2LinkTextData[4].pos_y = 20;
	strcpy(Options_p2LinkTextData[4].text, "[>>]");
	Options_p2LinkTextData[5].pos_x = 350+20;
	Options_p2LinkTextData[5].pos_y = 70;
	strcpy(Options_p2LinkTextData[5].text, "[<<]");
	Options_p2LinkTextData[6].pos_x = 350+120;
	Options_p2LinkTextData[6].pos_y = 70;
	strcpy(Options_p2LinkTextData[6].text, "[>>]");
	for(int i=0; i<4; i++){
		Options_p2LinkTextData[3 + i].enable = true;
		Options_p2LinkTextData[3 + i].text_w = 12;
		Options_p2LinkTextData[3 + i].text_h = 16;
		Options_p2LinkTextData[3 + i].text_defaultcolor = d3dg->GetColorCode(0.6f,0.6f,1.0f,1.0f);
		Options_p2LinkTextData[3 + i].text_cursorcolor = d3dg->GetColorCode(1.0f,0.6f,0.6f,1.0f);
	}

	Options_p2LinkTextData[7].pos_x = 350;
	Options_p2LinkTextData[7].pos_y = 100;
	strcpy(Options_p2LinkTextData[7].text, "[ ] FULL SCREEN");
	Options_p2LinkTextData[8].pos_x = 350;
	Options_p2LinkTextData[8].pos_y = 120;
	strcpy(Options_p2LinkTextData[8].text, "[ ] WINDOW");
	Options_p2LinkTextData[9].pos_x = 350;
	Options_p2LinkTextData[9].pos_y = 160;
	strcpy(Options_p2LinkTextData[9].text, "[ ] ENABLE SOUND");
	Options_p2LinkTextData[10].pos_x = 350;
	Options_p2LinkTextData[10].pos_y = 180;
	strcpy(Options_p2LinkTextData[10].text, "[ ] ENABLE BLOOD");
	Options_p2LinkTextData[11].pos_x = 350;
	Options_p2LinkTextData[11].pos_y = 200;
	strcpy(Options_p2LinkTextData[11].text, "[ ] INVERT MOUSE");
	Options_p2LinkTextData[12].pos_x = 350;
	Options_p2LinkTextData[12].pos_y = 220;
	strcpy(Options_p2LinkTextData[12].text, "[ ] FRAME SKIP");
	Options_p2LinkTextData[13].pos_x = 350;
	Options_p2LinkTextData[13].pos_y = 240;
	strcpy(Options_p2LinkTextData[13].text, "[ ] ANOTHER GUNSIGHT");
	for(int i=0; i<7; i++){
		Options_p2LinkTextData[7 + i].enable = true;
		Options_p2LinkTextData[7 + i].text_w = 12;
		Options_p2LinkTextData[7 + i].text_h = 16;
		Options_p2LinkTextData[7 + i].text_defaultcolor = d3dg->GetColorCode(0.6f,0.6f,1.0f,1.0f);
		Options_p2LinkTextData[7 + i].text_cursorcolor = d3dg->GetColorCode(1.0f,0.6f,0.6f,1.0f);
	}

	for(int i=0; i<18; i++){
		Options_p2LinkTextData[14 + i].enable = true;
		Options_p2LinkTextData[14 + i].pos_x = 170;
		Options_p2LinkTextData[14 + i].pos_y = i*20;
		Options_p2LinkTextData[14 + i].text_w = 12;
		Options_p2LinkTextData[14 + i].text_h = 16;
		Options_p2LinkTextData[14 + i].text_defaultcolor = d3dg->GetColorCode(0.6f,0.6f,1.0f,1.0f);
		Options_p2LinkTextData[14 + i].text_cursorcolor = d3dg->GetColorCode(1.0f,0.6f,0.6f,1.0f);
		strcpy(Options_p2LinkTextData[14 + i].text, "[ ]");
	}

	Options_p3LinkTextData[0].enable = true;
	Options_p3LinkTextData[0].pos_x = (OPTIONS_P3_W - 160)/2;
	Options_p3LinkTextData[0].pos_y = OPTIONS_P3_H - 35;
	Options_p3LinkTextData[0].text_w = 20;
	Options_p3LinkTextData[0].text_h = 26;
	Options_p3LinkTextData[0].text_defaultcolor = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
	Options_p3LinkTextData[0].text_cursorcolor = d3dg->GetColorCode(0.0f,1.0f,1.0f,1.0f);
	strcpy(Options_p3LinkTextData[0].text, "< EXIT >");

	int maxline = 18;

	//Reduce the number of display lines in Korean
	if( GameConfig.GetLanguage() == 1 ){
		maxline = 14;
	}

	scrollbar_license.Create(OPTIONS_P3_X + OPTIONS_P3_W - 20, OPTIONS_P3_Y+55, 20, 18*12, OPTIONS_LICENSELINE, maxline, 0);
	scrollbar_license.SetColor(d3dg->GetColorCode(0.5f,0.5f,0.5f,0.5f), d3dg->GetColorCode(0.6f,0.6f,0.25f,1.0f), d3dg->GetColorCode(0.8f,0.8f,0.25f,1.0f),
		d3dg->GetColorCode(0.4f,0.67f,0.57f,1.0f), d3dg->GetColorCode(0.38f,0.77f,0.64f,1.0f), d3dg->GetColorCode(0.6f,0.3f,0.25f,1.0f), d3dg->GetColorCode(0.8f,0.3f,0.25f,1.0f));

#if (GRAPHIC_ENGINE == 0)||(GRAPHIC_ENGINE == 1)
	//License text
	licenseinfo[0] = "";
	licenseinfo[1] = "OpenXOPS";
	licenseinfo[2] = "Copyright (c) 2014-2023, OpenXOPS Project / [-_-;](mikan) All rights reserved.";
	licenseinfo[3] = "";
	licenseinfo[4] = "Redistribution and use in source and binary forms, with or without";
	licenseinfo[5] = "modification, are permitted provided that the following conditions are met:";
	licenseinfo[6] = "* Redistributions of source code must retain the above copyright notice, ";
	licenseinfo[7] = "  this list of conditions and the following disclaimer.";
	licenseinfo[8] = "* Redistributions in binary form must reproduce the above copyright notice, ";
	licenseinfo[9] = "  this list of conditions and the following disclaimer in the documentation ";
	licenseinfo[10] = "  and/or other materials provided with the distribution.";
	licenseinfo[11] = "* Neither the name of the OpenXOPS Project nor the names of its contributors ";
	licenseinfo[12] = "  may be used to endorse or promote products derived from this software ";
	licenseinfo[13] = "  without specific prior written permission.";
	licenseinfo[14] = "";
	licenseinfo[15] = "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND";
	licenseinfo[16] = "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED";
	licenseinfo[17] = "WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE";
	licenseinfo[18] = "DISCLAIMED. IN NO EVENT SHALL OpenXOPS Project BE LIABLE FOR ANY";
	licenseinfo[19] = "DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES";
	licenseinfo[20] = "(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;";
	licenseinfo[21] = "LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND";
	licenseinfo[22] = "ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT";
	licenseinfo[23] = "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS";
	licenseinfo[24] = "SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
	licenseinfo[25] = "";
	//Update OPTIONS_LICENSELINE when adding lines.
#elif GRAPHIC_ENGINE == 8
	licenseinfo[0] = "";
	licenseinfo[1] = "OpenXOPS";
	licenseinfo[2] = "Copyright (c) 2014-2023, OpenXOPS Project / [-_-;](mikan) All rights reserved.";
	licenseinfo[3] = "";
	licenseinfo[4] = " -- See the document.";
	licenseinfo[5] = "";
	//With OpenGL, displaying enough text to require scrolling can cause flicker.
#endif	//GRAPHIC_ENGINE
}

//! @brief Input processing of option screen
void mainmenu::InputOptions()
{
	Options_p2LinkTextID = -1;

	if( modescreen == 1 ){
		//Link text judgment processing
		int ButtonID = -1;
		for(int i=0; i<OPTIONS_P1_DATAS; i++){
			if( Options_p1LinkTextData[i].enable == true ){
				if( inputCtrl->CheckMouseButtonUpL() ){
					if( ((OPTIONS_P1_X + Options_p1LinkTextData[i].pos_x) < mainmenu_mouseX)&&(mainmenu_mouseX < (OPTIONS_P1_X + Options_p1LinkTextData[i].pos_x + (signed)strlen(Options_p1LinkTextData[i].text)*Options_p1LinkTextData[i].text_w))
						&&((OPTIONS_P1_Y + Options_p1LinkTextData[i].pos_y) < mainmenu_mouseY)&&(mainmenu_mouseY < OPTIONS_P1_Y + Options_p1LinkTextData[i].pos_y + Options_p1LinkTextData[i].text_h)
					){
						ButtonID = i;
					}
				}
			}
		}
		if( ButtonID == 0 ){
			modescreen = 2;
		}
		if( ButtonID == 1 ){
			modescreen = 3;

			//Set up the scrollbar of the license statement at the top
			scrollbar_license.SetScrollItem(0);
			scrollbar_license.Input(0, 0, false, 0);
		}
		if( ButtonID == 2 ){
			modescreen = 0;
		}

		if( inputCtrl->CheckKeyDown(GetEscKeycode()) ){		//Processing ESC key
			modescreen = 0;
		}
	}

	else if( modescreen == 2 ){
		//Input key judgment processing for key setting
		if( (Options_KeyConfigID != -1)&&(Options_KeyConfigCnt > 0) ){
			int keycode = inputCtrl->CheckInputAll();
			if( keycode != -1 ){
				GameConfig.SetKeycode(Options_KeyConfigID, keycode);

				Options_KeyConfigID = -1;
				Options_KeyConfigCnt = 0;

				return;		//Disco ed to react other link text when clicked
			}
		}

		//Link text judgment processing
		for(int i=0; i<OPTIONS_P2_DATAS; i++){
			if( Options_p2LinkTextData[i].enable == true ){
				if( inputCtrl->CheckMouseButtonUpL() ){
					if( ((OPTIONS_P2_X + Options_p2LinkTextData[i].pos_x) < mainmenu_mouseX)&&(mainmenu_mouseX < (OPTIONS_P2_X + Options_p2LinkTextData[i].pos_x + (signed)strlen(Options_p2LinkTextData[i].text)*Options_p2LinkTextData[i].text_w))
						&&((OPTIONS_P2_Y + Options_p2LinkTextData[i].pos_y) < mainmenu_mouseY)&&(mainmenu_mouseY < OPTIONS_P2_Y + Options_p2LinkTextData[i].pos_y + Options_p2LinkTextData[i].text_h)
					){
						Options_p2LinkTextID = i;
					}
				}
			}
		}

		if( inputCtrl->CheckKeyDown(GetEscKeycode()) ){		//Processing ESC key
			//modescreen = 1;
			Options_p2LinkTextID = 2;
		}
	}

	else if( modescreen == 3 ){
		scrollbar_license.Input(mainmenu_mouseX, mainmenu_mouseY, inputCtrl->CheckMouseButtonNowL(), mainmenu_mouseY - mainmenu_mouseY_old);

		//Link text judgment processing
		int ButtonID = -1;
		for(int i=0; i<OPTIONS_P3_DATAS; i++){
			if( Options_p3LinkTextData[i].enable == true ){
				if( inputCtrl->CheckMouseButtonUpL() ){
					if( ((OPTIONS_P3_X + Options_p3LinkTextData[i].pos_x) < mainmenu_mouseX)&&(mainmenu_mouseX < (OPTIONS_P3_X + Options_p3LinkTextData[i].pos_x + (signed)strlen(Options_p3LinkTextData[i].text)*Options_p3LinkTextData[i].text_w))
						&&((OPTIONS_P3_Y + Options_p3LinkTextData[i].pos_y) < mainmenu_mouseY)&&(mainmenu_mouseY < (OPTIONS_P3_Y + Options_p3LinkTextData[i].pos_y + Options_p3LinkTextData[i].text_h))
					){
						ButtonID = i;
					}
				}
			}
		}
		if( ButtonID == 0 ){
			modescreen = 1;
		}

		if( inputCtrl->CheckKeyDown(GetEscKeycode()) ){		//Processing ESC key
			modescreen = 1;
		}
	}
}

//! @brief Main processing of optional screen
void mainmenu::ProcessOptions()
{
	if( modescreen == 2 ){

		//Default
		if( Options_p2LinkTextID == 0 ){
			//Default value read
			GameConfig.SetDefaultConfig();
		}

		//Save
		if( Options_p2LinkTextID == 1 ){
			//Window Full Screen Switching
			if( Options_FullscreenFlag_old != GameConfig.GetFullscreenFlag() ){
				if( ChangeWindowMode(WindowCtrl, d3dg, inputCtrl, this, GameConfig.GetFullscreenFlag()) != 0 ){
					//Failure to reset the settings
					GameConfig.SetFullscreenFlag(Options_FullscreenFlag_old);
				}
				else{
					//Disable settings when successful
					Options_FullscreenFlag_old = GameConfig.GetFullscreenFlag();
				}
			}

			//Sound reconfiguration
			float volume;
			if( GameConfig.GetSoundFlag() == false ){
				volume = 0.0f;
			}
			else{
				volume = 1.0f;
			}
			SoundCtrl.SetVolume(volume);

			//Config file writing
			if( GameConfig.SaveFile("config.dat") == 1 ){
				//MainWindow.ErrorInfo("config data save failed");
				//return 1;
			}

			Options_KeyConfigID = -1;
			Options_KeyConfigCnt = 0;

			modescreen = 1;
		}

		//Cancel
		if( Options_p2LinkTextID == 2 ){
			//Config file reading
			if( GameConfig.LoadFile("config.dat") == 1 ){
				//MainWindow.ErrorInfo("config data open failed");
				//return 1;
			}

			Options_FullscreenFlag_old = GameConfig.GetFullscreenFlag();
			Options_KeyConfigID = -1;
			Options_KeyConfigCnt = 0;

			modescreen = 1;
		}

		if( Options_p2LinkTextID == 3 ){
			int param = GameConfig.GetMouseSensitivity();
			param -= 1;
			if( param < 10 ){ param = 10; }
			GameConfig.SetMouseSensitivity(param);
		}
		if( Options_p2LinkTextID == 4 ){
			int param = GameConfig.GetMouseSensitivity();
			param += 1;
			if( param > 50 ){ param = 50; }
			GameConfig.SetMouseSensitivity(param);
		}
		if( Options_p2LinkTextID == 5 ){
			int param = GameConfig.GetBrightness();
			param -= 1;
			if( param < 0 ){ param = 0; }
			GameConfig.SetBrightness(param);
		}
		if( Options_p2LinkTextID == 6 ){
			int param = GameConfig.GetBrightness();
			param += 1;
			if( param > 10 ){ param = 10; }
			GameConfig.SetBrightness(param);
		}

		if( Options_p2LinkTextID == 7 ){
			GameConfig.SetFullscreenFlag(true);
		}
		if( Options_p2LinkTextID == 8 ){
			GameConfig.SetFullscreenFlag(false);
		}
		if( Options_p2LinkTextID == 9 ){
			if( GameConfig.GetSoundFlag() == true ){ GameConfig.SetSoundFlag(false); }
			else { GameConfig.SetSoundFlag(true); }
		}
		if( Options_p2LinkTextID == 10 ){
			if( GameConfig.GetBloodFlag() == true ){ GameConfig.SetBloodFlag(false); }
			else { GameConfig.SetBloodFlag(true); }
		}
		if( Options_p2LinkTextID == 11 ){
			if( GameConfig.GetInvertMouseFlag() == true ){ GameConfig.SetInvertMouseFlag(false); }
			else { GameConfig.SetInvertMouseFlag(true); }
		}
		if( Options_p2LinkTextID == 12 ){
			if( GameConfig.GetFrameskipFlag() == true ){ GameConfig.SetFrameskipFlag(false); }
			else { GameConfig.SetFrameskipFlag(true); }
		}
		if( Options_p2LinkTextID == 13 ){
			if( GameConfig.GetAnotherGunsightFlag() == true ){ GameConfig.SetAnotherGunsightFlag(false); }
			else { GameConfig.SetAnotherGunsightFlag(true); }
		}

		//Count set for key setting
		for(int i=0; i<18; i++){
			if( Options_p2LinkTextID == (14 + i) ){
				Options_KeyConfigID = i;
				Options_KeyConfigCnt = (int)(GAMEFPS*5);
			}
		}

		//Countdown for key setting
		if( (Options_KeyConfigID != -1)&&(Options_KeyConfigCnt > 0) ){
			Options_KeyConfigCnt -= 1;
		}
		else{
			Options_KeyConfigID = -1;
			Options_KeyConfigCnt = 0;
		}
	}
}

//! @brief Optional screen drawing processing
void mainmenu::RenderOptions()
{
	if( modescreen == 1 ){
		int color;

		//Area drawing
		d3dg->Draw2DBox(OPTIONS_P1_X, OPTIONS_P1_Y, OPTIONS_P1_X + OPTIONS_P1_W, OPTIONS_P1_Y + OPTIONS_P1_H, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));

		for(int i=0; i<OPTIONS_P1_DATAS; i++){
			if( Options_p1LinkTextData[i].enable == true ){
				//Set the character color
				if( ((OPTIONS_P1_X + Options_p1LinkTextData[i].pos_x) < mainmenu_mouseX)&&(mainmenu_mouseX < (OPTIONS_P1_X + Options_p1LinkTextData[i].pos_x + (signed)strlen(Options_p1LinkTextData[i].text)*Options_p1LinkTextData[i].text_w))
					&&((OPTIONS_P1_Y + Options_p1LinkTextData[i].pos_y) < mainmenu_mouseY)&&(mainmenu_mouseY < (OPTIONS_P1_Y + Options_p1LinkTextData[i].pos_y + Options_p1LinkTextData[i].text_h))
				){
					color = Options_p1LinkTextData[i].text_cursorcolor;
				}
				else{
					color = Options_p1LinkTextData[i].text_defaultcolor;
				}

				//Show Letters
				d3dg->Draw2DTextureFontText(OPTIONS_P1_X + Options_p1LinkTextData[i].pos_x +1, OPTIONS_P1_Y + Options_p1LinkTextData[i].pos_y +1, Options_p1LinkTextData[i].text,
					d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), Options_p1LinkTextData[i].text_w, Options_p1LinkTextData[i].text_h);
				d3dg->Draw2DTextureFontText(OPTIONS_P1_X + Options_p1LinkTextData[i].pos_x, OPTIONS_P1_Y + Options_p1LinkTextData[i].pos_y, Options_p1LinkTextData[i].text,
					color, Options_p1LinkTextData[i].text_w, Options_p1LinkTextData[i].text_h);
			}
			else{
				d3dg->Draw2DTextureFontText(OPTIONS_P1_X + Options_p1LinkTextData[i].pos_x, OPTIONS_P1_Y + Options_p1LinkTextData[i].pos_y, Options_p1LinkTextData[i].text,
					d3dg->GetColorCode(0.6f,0.6f,0.6f,1.0f), Options_p1LinkTextData[i].text_w, Options_p1LinkTextData[i].text_h);
			}
		}
	}

	if( modescreen == 2 ){
		int color;
		char str[32];

		//Area drawing
		d3dg->Draw2DBox(OPTIONS_P2_X, OPTIONS_P2_Y, OPTIONS_P2_X + OPTIONS_P2_W, OPTIONS_P2_Y + OPTIONS_P2_H, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));

		//Mouse Sensitivity
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 350 +1, OPTIONS_P2_Y + 0 +1, "MOUSE SENSITIVITY", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 350, OPTIONS_P2_Y + 0, "MOUSE SENSITIVITY", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		sprintf(str, "%02d", GameConfig.GetMouseSensitivity());
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 350 + 85 +1, OPTIONS_P2_Y + 20 +1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 350 + 85, OPTIONS_P2_Y + 20, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);

		//Screen Brightness
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 350 +1, OPTIONS_P2_Y + 50 +1, "BRIGHTNESS", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 350, OPTIONS_P2_Y + 50, "BRIGHTNESS", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		sprintf(str, "%02d", GameConfig.GetBrightness());
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 350 + 85 +1, OPTIONS_P2_Y + 70 +1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 350 + 85, OPTIONS_P2_Y + 70, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);

		//Operation Key List
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 0 +1, "TURN UP", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 0, "TURN UP", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 20 +1, "TURN DOWN", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 20, "TURN DOWN", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 40 +1, "TURN LEFT", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 40, "TURN LEFT", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 60 +1, "TURN RIGHT", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 60, "TURN RIGHT", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 80 +1, "MOVE FORWARD", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 80, "MOVE FORWARD", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 100 +1, "MOVE BACKWARD", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 100, "MOVE BACKWARD", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 120 +1, "MOVE LEFT", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 120, "MOVE LEFT", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 140 +1, "MOVE RIGHT", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 140, "MOVE RIGHT", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 160 +1, "WALK", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 160, "WALK", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 180 +1, "JUMP", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 180, "JUMP", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 200 +1, "RELOAD", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 200, "RELOAD", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 220 +1, "DROP WEAPON", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 220, "DROP WEAPON", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 240 +1, "ZOOM", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 240, "ZOOM", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 260 +1, "FIRE MODE", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 260, "FIRE MODE", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 280 +1, "SWITCH WEAPON", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 280, "SWITCH WEAPON", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 300 +1, "WEAPON 1", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 300, "WEAPON 1", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 320 +1, "WEAPON 2", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 320, "WEAPON 2", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0 +1, OPTIONS_P2_Y + 340 +1, "FIRE", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 12, 16);
		d3dg->Draw2DTextureFontText(OPTIONS_P2_X + 0, OPTIONS_P2_Y + 340, "FIRE", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 12, 16);
		for(int i=0; i<18; i++){
			if( Options_KeyConfigID == i ){
				sprintf(Options_p2LinkTextData[14 + i].text, "[(%d)]", (int)((float)Options_KeyConfigCnt/GAMEFPS)+1 );
			}
			else{
				sprintf(Options_p2LinkTextData[14 + i].text, "[%s]", GameConfig.GetOriginalkeycodeString( GameConfig.GetKeycode(i) ) );
			}
		}

		//Item name setting
		if( GameConfig.GetFullscreenFlag() == true ){
			Options_p2LinkTextData[7].text[1] = '+';
			Options_p2LinkTextData[8].text[1] = ' ';
		}
		else{
			Options_p2LinkTextData[7].text[1] = ' ';
			Options_p2LinkTextData[8].text[1] = '+';
		}
		if( GameConfig.GetSoundFlag() == true ){ Options_p2LinkTextData[9].text[1] = '*'; }
		else{ Options_p2LinkTextData[9].text[1] = ' '; }
		if( GameConfig.GetBloodFlag() == true ){ Options_p2LinkTextData[10].text[1] = '*'; }
		else{ Options_p2LinkTextData[10].text[1] = ' '; }
		if( GameConfig.GetInvertMouseFlag() == true ){ Options_p2LinkTextData[11].text[1] = '*'; }
		else{ Options_p2LinkTextData[11].text[1] = ' '; }
		if( GameConfig.GetFrameskipFlag() == true ){ Options_p2LinkTextData[12].text[1] = '*'; }
		else{ Options_p2LinkTextData[12].text[1] = ' '; }
		if( GameConfig.GetAnotherGunsightFlag() == true ){ Options_p2LinkTextData[13].text[1] = '*'; }
		else{ Options_p2LinkTextData[13].text[1] = ' '; }

		for(int i=0; i<OPTIONS_P2_DATAS; i++){
			if( Options_p2LinkTextData[i].enable == true ){
				//Set the character color
				if( ((OPTIONS_P2_X + Options_p2LinkTextData[i].pos_x) < mainmenu_mouseX)&&(mainmenu_mouseX < (OPTIONS_P2_X + Options_p2LinkTextData[i].pos_x + (signed)strlen(Options_p2LinkTextData[i].text)*Options_p2LinkTextData[i].text_w))
					&&((OPTIONS_P2_Y + Options_p2LinkTextData[i].pos_y) < mainmenu_mouseY)&&(mainmenu_mouseY < (OPTIONS_P2_Y + Options_p2LinkTextData[i].pos_y + Options_p2LinkTextData[i].text_h))
				){
					color = Options_p2LinkTextData[i].text_cursorcolor;
				}
				else{
					color = Options_p2LinkTextData[i].text_defaultcolor;
				}

				//Show Letters
				d3dg->Draw2DTextureFontText(OPTIONS_P2_X + Options_p2LinkTextData[i].pos_x +1, OPTIONS_P2_Y + Options_p2LinkTextData[i].pos_y +1, Options_p2LinkTextData[i].text,
					d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), Options_p2LinkTextData[i].text_w, Options_p2LinkTextData[i].text_h);
				d3dg->Draw2DTextureFontText(OPTIONS_P2_X + Options_p2LinkTextData[i].pos_x, OPTIONS_P2_Y + Options_p2LinkTextData[i].pos_y, Options_p2LinkTextData[i].text,
					color, Options_p2LinkTextData[i].text_w, Options_p2LinkTextData[i].text_h);
			}
			else{
				d3dg->Draw2DTextureFontText(OPTIONS_P2_X + Options_p2LinkTextData[i].pos_x, OPTIONS_P2_Y + Options_p2LinkTextData[i].pos_y, Options_p2LinkTextData[i].text,
					d3dg->GetColorCode(0.6f,0.6f,0.6f,1.0f), Options_p2LinkTextData[i].text_w, Options_p2LinkTextData[i].text_h);
			}
		}
	}

	if( modescreen == 3 ){
		int color;
		int maxline = 18;

		//Reduce the number of display lines in Korean
		if( GameConfig.GetLanguage() == 1 ){
			maxline = 14;
		}

		//Area drawing
		d3dg->Draw2DBox(OPTIONS_P3_X, OPTIONS_P3_Y, OPTIONS_P3_X + OPTIONS_P3_W, OPTIONS_P3_Y + OPTIONS_P3_H, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));

		d3dg->Draw2DTextureFontText(OPTIONS_P3_X + 0 +1, OPTIONS_P3_Y + 15 +1, "SOFTWARE LICENSE", d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 18, 24);
		d3dg->Draw2DTextureFontText(OPTIONS_P3_X + 0, OPTIONS_P3_Y + 15, "SOFTWARE LICENSE", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 18, 24);

		d3dg->Draw2DBox(OPTIONS_P3_X + 0, OPTIONS_P3_Y + 55, OPTIONS_P3_X + OPTIONS_P3_W-20, OPTIONS_P3_Y + 55 + 18*12, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.7f));

		//License writing
        const std::string licenseText=BuildMenuLicenseText(licenseinfo, OPTIONS_LICENSELINE,
            scrollbar_license.GetScrollItem(), maxline);
        d3dg->Draw2DMSSmallFontText(OPTIONS_P3_X + 5, OPTIONS_P3_Y+55, licenseText.c_str(),
            d3dg->GetColorCode(0.8f,0.8f,0.8f,1.0f));

		//Scroll Bar Drawing
		scrollbar_license.Draw(d3dg);

		for(int i=0; i<OPTIONS_P3_DATAS; i++){
			if( Options_p3LinkTextData[i].enable == true ){
				//Set the character color
				if( ((OPTIONS_P3_X + Options_p3LinkTextData[i].pos_x) < mainmenu_mouseX)&&(mainmenu_mouseX < (OPTIONS_P3_X + Options_p3LinkTextData[i].pos_x + (signed)strlen(Options_p3LinkTextData[i].text)*Options_p3LinkTextData[i].text_w))
					&&((OPTIONS_P3_Y + Options_p3LinkTextData[i].pos_y) < mainmenu_mouseY)&&(mainmenu_mouseY < (OPTIONS_P3_Y + Options_p3LinkTextData[i].pos_y + Options_p3LinkTextData[i].text_h))
				){
					color = Options_p3LinkTextData[i].text_cursorcolor;
				}
				else{
					color = Options_p3LinkTextData[i].text_defaultcolor;
				}

				//Show Letters
				d3dg->Draw2DTextureFontText(OPTIONS_P3_X + Options_p3LinkTextData[i].pos_x +1, OPTIONS_P3_Y + Options_p3LinkTextData[i].pos_y +1, Options_p3LinkTextData[i].text,
					d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), Options_p3LinkTextData[i].text_w, Options_p3LinkTextData[i].text_h);
				d3dg->Draw2DTextureFontText(OPTIONS_P3_X + Options_p3LinkTextData[i].pos_x, OPTIONS_P3_Y + Options_p3LinkTextData[i].pos_y, Options_p3LinkTextData[i].text,
					color, Options_p3LinkTextData[i].text_w, Options_p3LinkTextData[i].text_h);
			}
			else{
				d3dg->Draw2DTextureFontText(OPTIONS_P3_X + Options_p3LinkTextData[i].pos_x, OPTIONS_P3_Y + Options_p3LinkTextData[i].pos_y, Options_p3LinkTextData[i].text,
					d3dg->GetColorCode(0.6f,0.6f,0.6f,1.0f), Options_p3LinkTextData[i].text_w, Options_p3LinkTextData[i].text_h);
			}
		}
	}
}
#endif	//ENABLE_MENUOPTIONS

void mainmenu::Destroy()
{
	//Block Data Release
	d3dg->CleanupMapdata();

	//Object Manager Release
	ObjMgr.Cleanup();

	//Free the background sky.
	Resource.CleanupSkyModelTexture();

	d3dg->CleanupTexture(gametitle);
    d3dg->CleanupTexture(cursorTexture);
    cursorTexture = -1;

	GameState->NextState();
}

//! @brief Constructor
briefing::briefing()
{
	TwoTexture = false;
	TextureA = -1;
	TextureB = -1;
}

//! @brief Destructor
briefing::~briefing()
{}

int briefing::Create()
{
	char path[MAX_PATH];
	char pdata[MAX_PATH];
	char PictureA[MAX_PATH];
	char PictureB[MAX_PATH];

	//Get background image
	gametitle = d3dg->LoadTexture("data\\title.dds", false, false);

	//Get mif file path
	if( GameInfoData.selectmission_id >= 0 ){
		GameParamInfo.GetOfficialMission(GameInfoData.selectmission_id, NULL, NULL, path, pdata, NULL, NULL);
		strcat(path, pdata);
		strcat(path, ".txt");
	}
	else{
		strcpy(path, GameInfoData.mifpath);
	}

	//Load mif file
	if( MIFdata.LoadFiledata(path, true) != 0 ){
		//briefing data open failed
		return 1;
	}

	//Bleefing image file path acquisition
	MIFdata.GetPicturefilePath(PictureA, PictureB);

	//Bleefing image loading
	if( strcmp(PictureB, "!") == 0 ){
		TwoTexture = false;
		TextureA = d3dg->LoadTexture(PictureA, true, false);
		TextureB = -1;
	}
	else{
		TwoTexture = true;
		TextureA = d3dg->LoadTexture(PictureA, true, false);
		TextureB = d3dg->LoadTexture(PictureB, true, false);
	}

	//center mouse cursor
	inputCtrl->MoveMouseCenter();
	framecnt = 0;

	GameState->NextState();
	return 0;
}

int briefing::Recovery()
{
	char PictureA[MAX_PATH];
	char PictureB[MAX_PATH];

	//Get background image
	gametitle = d3dg->LoadTexture("data\\title.dds", false, false);

	//Bleefing image file path acquisition
	MIFdata.GetPicturefilePath(PictureA, PictureB);

	//Bleefing image loading
	if( strcmp(PictureB, "!") == 0 ){
		TwoTexture = false;
		TextureA = d3dg->LoadTexture(PictureA, true, false);
		TextureB = -1;
	}
	else{
		TwoTexture = true;
		TextureA = d3dg->LoadTexture(PictureA, true, false);
		TextureB = d3dg->LoadTexture(PictureB, true, false);
	}

	return 0;
}

void briefing::Render2D()
{
	float effectA = GetEffectAlphaLoop(framecnt, 0.8f, 0.7f, true);
	float effectB = GetEffectAlphaLoop(framecnt, 0.8f, 1.0f, true);
	int effectB_sizeW = (int)( (float)(framecnt%((int)(GAMEFPS*1.0f))) * 0.2f + 18 );
	int effectB_sizeH = (int)( (float)(framecnt%((int)(GAMEFPS*1.0f))) * 1.0f + 26 );

	//Note: The background image is automatically drawn.

	//Fixed character display
	d3dg->Draw2DTextureFontTextCenterScaling(0, 30, "BRIEFING", d3dg->GetColorCode(1.0f,1.0f,0.0f,effectA), 60, 42);
	d3dg->Draw2DTextureFontText(GameConfig.GetScreenWidth() - 210 - effectB_sizeW*20/2, GameConfig.GetScreenHeight() - 37 - effectB_sizeH/2,
								"LEFT CLICK TO BEGIN", d3dg->GetColorCode(1.0f,1.0f,1.0f,effectB), effectB_sizeW, effectB_sizeH);
	d3dg->Draw2DTextureFontText(GameConfig.GetScreenWidth() - 210 - 18*20/2, GameConfig.GetScreenHeight() - 37 - 26/2, "LEFT CLICK TO BEGIN", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 18, 26);

	//Bleefing image drawing
	if( TwoTexture == false ){
		if( TextureA == -1 ){ d3dg->Draw2DBoxScaling(40, 180, 40+160, 180+150, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f)); }
		else{ d3dg->Draw2DTextureScaling(40, 180, TextureA, 160, 150, 1.0f); }
	}
	else{
		if( TextureA == -1 ){ d3dg->Draw2DBoxScaling(40, 130, 40+160, 130+150, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f)); }
		else{ d3dg->Draw2DTextureScaling(40, 130, TextureA, 160, 150, 1.0f); }

		if( TextureB == -1 ){ d3dg->Draw2DBoxScaling(40, 300, 40+160, 300+150, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f)); }
		else{ d3dg->Draw2DTextureScaling(40, 300, TextureB, 160, 150, 1.0f); }
	}

	//Get and display the mission name.
	char mname[64];
	if( MIFdata.GetFiletype() == false ){
		GameParamInfo.GetOfficialMission(GameInfoData.selectmission_id, NULL, mname, NULL, NULL, NULL, NULL);
	}
	else{
		strcpy(mname, MIFdata.GetMissionFullname());
	}
	d3dg->Draw2DTextureFontTextCenterScaling(0, 90, mname, d3dg->GetColorCode(1.0f,0.5f,0.0f,1.0f), 18, 25);

	//Show Mission Description
	d3dg->Draw2DMSFontTextScaling(230, 180, MIFdata.GetBriefingText(), d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f));
}

void briefing::Destroy()
{
	//Free the briefing images.
	d3dg->CleanupTexture(TextureA);
	d3dg->CleanupTexture(TextureB);

	//Free background image
	d3dg->CleanupTexture(gametitle);

	GameState->NextState();
}

//! @brief Constructor
maingame::maingame()
{
	SkyNumber = 0;
	DarkScreenFlag = false;
	FogFlag = true;
	mouse_rx = 0.0f;
	mouse_ry = 0.0f;
	view_rx = 0.0f;
	view_ry = 0.0f;
	add_camera_rx = 0.0f;
	add_camera_ry = 0.0f;
	ShowInfo_Debugmode = false;
	Camera_Debugmode = false;
	tag = false;
	radar = false;
	wireframe = false;
	nomodel = false;
	CenterLine = false;
	Camera_Blind = true;
	Camera_F1mode = false;
	Camera_F2mode = 0;
	Camera_HOMEmode = GameConfig.GetShowArmFlag();
	Cmd_F5 = false;
	InvincibleID = -1;
	PlayerAI = false;
	AIstop = false;
	AINoFight = false;
	AIdebuginfoID = -1;
	start_framecnt = 0;
	end_framecnt = 0;
    elapsed_ticks = 0;
    elapsed_stopped = false;
	EventStop = false;
	for(int i=0; i<TOTAL_EVENTLINE; i++){ EventSkip[i] = false; }
	GameSpeed = 1;
	message_id = -1;
	message_cnt = 0;
	redflash_flag = false;
	time = 0;
	time_input = 0;
	time_process_object = 0;
	time_process_ai = 0;
	time_process_event = 0;
	time_sound = 0;
	time_render = 0;
	memcpy(&MainGameInfo, &GameInfoData, sizeof(GameInfo));

#ifdef ENABLE_DEBUGCONSOLE
	Show_Console = false;
	InfoConsoleData = NULL;
	NewCommand[0] = '\0';
	ScreenShot = 0;
#endif
}

//! @brief Destructor
maingame::~maingame()
{}

//! @brief Set the debug-mode flag for displaying coordinates and other information.
void maingame::SetShowInfoFlag(bool flag)
{
	ShowInfo_Debugmode = flag;
}

//! @brief Get the game execution speed.
int maingame::GetGameSpeed()
{
	return GameSpeed;
}

int maingame::Create()
{
	memcpy(&MainGameInfo, &GameInfoData, sizeof(GameInfo));
	char path[MAX_PATH];
	char bdata[MAX_PATH];
	char pdata[MAX_PATH];
	char pdata2[MAX_PATH];
	int blockflag, pointflag;
	bool collisionflag;

	//.bd1 and .pd1 file path
	if( MIFdata.GetFiletype() == false ){
		GameParamInfo.GetOfficialMission(MainGameInfo.selectmission_id, NULL, NULL, path, pdata2, &collisionflag, &DarkScreenFlag);

		strcpy(bdata, path);
		strcat(bdata, OFFICIALMISSION_BD1);
		strcpy(pdata, path);
		strcat(pdata, pdata2);
		strcat(pdata, ".pd1");
	}
	else{
		MIFdata.GetDatafilePath(bdata, pdata);
		collisionflag = MIFdata.GetCollisionFlag();
		DarkScreenFlag = MIFdata.GetScreenFlag();

		GetFileDirectory(bdata, path);
	}

	//Read additional accessories
	for(int i=0; i<MAX_ADDSMALLOBJECT; i++){
		Resource.LoadAddSmallObject(i, MIFdata.GetAddSmallobjectModelPath(i), MIFdata.GetAddSmallobjectTexturePath(i), MIFdata.GetAddSmallobjectSoundPath(i));
	}

	//Block data reading
	blockflag = BlockData.LoadFiledata(bdata);

	//Point data reading
	pointflag = PointData.LoadFiledata(pdata);

	//Suspended if there is an error in file loading
	if( (blockflag != 0)||(pointflag != 0) ){
		//2bit : point data open failed
		//1bit : block data open failed
		return pointflag*2 + blockflag;
	}

	//Block data initialization
	BlockData.CalculationBlockdata(DarkScreenFlag);
	d3dg->LoadMapdata(&BlockData, path);
	CollD.InitCollision(&BlockData);

	//Point data initialization
	ObjMgr.LoadPointData();

	//Configure additional collision detection.
	ObjMgr.SetAddCollisionFlag(collisionflag);

	//AI Settings
	for(int i=0; i<MAX_HUMAN; i++){
		HumanAI[i].SetClass(&ObjMgr, i, &BlockData, &PointData, &GameParamInfo, &CollD, GameSound);
		HumanAI[i].Init();
	}


	//Background empty reading
	SkyNumber = MIFdata.GetSkynumber();
	Resource.LoadSkyModelTexture(SkyNumber);

	//Sound Initialization
	GameSound->InitWorldSound();

	//Event Initialization
	for(int i=0; i<TOTAL_EVENTLINE; i++){
		Event[i].SetClass(&PointData, &ObjMgr);
	}
	Event[0].Reset(TOTAL_EVENTENTRYPOINT_0);
	Event[1].Reset(TOTAL_EVENTENTRYPOINT_1);
	Event[2].Reset(TOTAL_EVENTENTRYPOINT_2);

	//Get player orientation
	ObjMgr.GetPlayerHumanObject()->GetRxRy(&mouse_rx, &mouse_ry);

	FogFlag = true;
	view_rx = 0.0f;
	view_ry = 0.0f;
	add_camera_rx = 0.0f;
	add_camera_ry = 0.0f;
	Camera_Debugmode = false;
	tag = false;
	radar = false;
	wireframe = false;
	nomodel = false;
	CenterLine = false;
	Camera_Blind = true;
	Camera_F1mode = false;
	InvincibleID = -1;
	PlayerAI = false;
	AIstop = false;
	AINoFight = false;
	AIdebuginfoID = -1;
	framecnt = 0;
	start_framecnt = 0;
	end_framecnt = 0;
    elapsed_ticks = 0;
    elapsed_stopped = false;
	EventStop = false;
	for(int i=0; i<TOTAL_EVENTLINE; i++){ EventSkip[i] = false; }
	GameSpeed = 1;
	message_id = -1;
	message_cnt = 0;
	redflash_flag = false;
	MainGameInfo.missioncomplete = false;
	MainGameInfo.fire = 0;
	MainGameInfo.ontarget = 0;
	MainGameInfo.kill = 0;
	MainGameInfo.headshot = 0;

#ifdef ENABLE_DEBUGCONSOLE
	Show_Console = false;
	ScreenShot = 0;

	//Initialization for consoles
	InfoConsoleData = new ConsoleData [MAX_CONSOLELINES];
	for(int i=0; i<MAX_CONSOLELINES; i++){
		InfoConsoleData[i].color = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
		InfoConsoleData[i].textdata[0] = '\0';
	}
	AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Game Debug Console.");
	AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "        Command list >help");
	ConsoleTextBoxControl.InitTextBox("", MAX_CONSOLELEN - (int)strlen(CONSOLE_PROMPT), 0x01|0x04|0x20);
#endif

	//center mouse cursor
	inputCtrl->MoveMouseCenter();

	GameState->NextState();
	return 0;
}

int maingame::Recovery()
{
	char path[MAX_PATH];
	char bdata[MAX_PATH];	//Dummy
	char pdata[MAX_PATH];	//Dummy

	//.bd1 and .pd1 file path
	if( MIFdata.GetFiletype() == false ){
		GameParamInfo.GetOfficialMission(MainGameInfo.selectmission_id, NULL, NULL, path, NULL, NULL, NULL);
	}
	else{
		MIFdata.GetDatafilePath(bdata, pdata);
		GetFileDirectory(bdata, path);
	}

	//Release additional accessories  Load
	for(int i=0; i<MAX_ADDSMALLOBJECT; i++){
		Resource.CleanupAddSmallObject(i);
		Resource.LoadAddSmallObject(i, MIFdata.GetAddSmallobjectModelPath(i), MIFdata.GetAddSmallobjectTexturePath(i), MIFdata.GetAddSmallobjectSoundPath(i));
	}

	//Block data initialization
	d3dg->LoadMapdata(&BlockData, path);

	//Point data initialization
	ObjMgr.Recovery();

	//Initialize the background sky.
	Resource.CleanupSkyModelTexture();
	Resource.LoadSkyModelTexture(SkyNumber);

	return 0;
}

//! @brief Check the input of specific operations
bool maingame::CheckInputControl(int CheckKey, int mode)
{
	int KeyCode = OriginalkeycodeToDinputdef(GameConfig.GetKeycode(CheckKey));
	if( KeyCode == -1 ){
		if( mode == 0 ){
			return inputCtrl->CheckMouseButtonNowL();
		}
		if( mode == 1 ){
			return inputCtrl->CheckMouseButtonDownL();
		}
		//if( mode == 2 ){
			return inputCtrl->CheckMouseButtonUpL();
		//}
	}
	if( KeyCode == -2 ){
		if( mode == 0 ){
			return inputCtrl->CheckMouseButtonNowR();
		}
		if( mode == 1 ){
			return inputCtrl->CheckMouseButtonDownR();
		}
		//if( mode == 2 ){
			return inputCtrl->CheckMouseButtonUpR();
		//}
	}
	if( KeyCode == -3 ){
		int CodeL, CodeR;
		GetDoubleKeyCode(0, &CodeL, &CodeR);
		if( mode == 0 ){
			if( inputCtrl->CheckKeyNow(CodeL) ){
				return true;
			}
			return inputCtrl->CheckKeyNow(CodeR);
		}
		if( mode == 1 ){
			if( inputCtrl->CheckKeyDown(CodeL) ){
				return true;
			}
			return inputCtrl->CheckKeyDown(CodeR);
		}
		//if( mode == 2 ){
			if( inputCtrl->CheckKeyUp(CodeL) ){
				return true;
			}
			return inputCtrl->CheckKeyUp(CodeR);
		//}
	}
	if( KeyCode == -4 ){
		int CodeL, CodeR;
		GetDoubleKeyCode(1, &CodeL, &CodeR);
		if( mode == 0 ){
			if( inputCtrl->CheckKeyNow(CodeL) ){
				return true;
			}
			return inputCtrl->CheckKeyNow(CodeR);
		}
		if( mode == 1 ){
			if( inputCtrl->CheckKeyDown(CodeL) ){
				return true;
			}
			return inputCtrl->CheckKeyDown(CodeR);
		}
		//if( mode == 2 ){
			if( inputCtrl->CheckKeyUp(CodeL) ){
				return true;
			}
			return inputCtrl->CheckKeyUp(CodeR);
		//}
	}

	if( mode == 0 ){
		return inputCtrl->CheckKeyNow( OriginalkeycodeToDinputdef(GameConfig.GetKeycode(CheckKey)) );
	}
	if( mode == 1 ){
		return inputCtrl->CheckKeyDown( OriginalkeycodeToDinputdef(GameConfig.GetKeycode(CheckKey)) );
	}
	//if( mode == 2 ){
		return inputCtrl->CheckKeyUp( OriginalkeycodeToDinputdef(GameConfig.GetKeycode(CheckKey)) );
	//}
}

void maingame::Input()
{
	time = GetTimeMS();

	//Get Player Class
	human *myHuman = ObjMgr.GetPlayerHumanObject();

	//Get key input
	inputCtrl->GetInputState(true);
	inputCtrl->MoveMouseCenter();

	//Mouse Move
	int x, y;
	float MouseSensitivity;
	inputCtrl->GetMouseMovement(&x, &y);

	//Calculate viewpoint movement.
	ScopeParameter sparam;
	GameParamInfo.GetScopeParam(myHuman->GetScopeMode(), &sparam);
	MouseSensitivity = DegreeToRadian(1) * sparam.MouseRange * GameConfig.GetMouseSensitivity();

	//If the mouse invert (option setting) is enabled, flip it.
	if( GameConfig.GetInvertMouseFlag() == true ){
		y *= -1;
	}

	if( inputCtrl->CheckKeyDown(GetEscKeycode()) ){					//Game termination operation or check
		GameState->PushBackSpaceKey();
		GameSpeed = 1;
	}
	else if( inputCtrl->CheckKeyDown( GetFunctionKeycode(12) ) ){	//Reset operation or check
		GameState->PushF12Key();
		GameSpeed = 1;
	}

	//Check whether to change the UI of the screen
	if( inputCtrl->CheckKeyDown( GetFunctionKeycode(2) ) ){
		if( Camera_F2mode == 2 ){
			Camera_F2mode = 0;
		}
		else{
			Camera_F2mode += 1;
		}
	}

#ifdef ENABLE_DEBUGCONSOLE
	if( Show_Console == false ){
#endif

		if( Camera_Debugmode == false ){	//Normal mode
			if( PlayerAI == false ){
				InputPlayer(myHuman, x, y, MouseSensitivity);
			}
		}
		else{								//If debug mode is active.
			InputViewCamera(x, y, MouseSensitivity);
		}

#ifdef ENABLE_DEBUGCONSOLE
	}

	//Check for an operation that toggles the debug console.
	if( inputCtrl->CheckKeyDown( GetFunctionKeycode(11) ) ){
		if( Show_Console == false ){
			Show_Console = true;

			ConsoleTextBoxControl.SetTextBoxStr("");
		}
		else{
			Show_Console = false;
		}
	}

	if( Show_Console == true ){
		InputConsole();
	}
#endif

	time_input = GetTimeMS() - time;
}

//! @brief Process player-control input.
//! @param myHuman Player Class
//! @param mouse_x Mouse X Coordinate
//! @param mouse_y Mouse Y coordinates
//! @param MouseSensitivity Calculate viewpoint movement.
void maingame::InputPlayer(human *myHuman, int mouse_x, int mouse_y, float MouseSensitivity)
{
	if( myHuman == NULL ){ return; }

	int PlayerID = ObjMgr.GetPlayerID();

	if( myHuman->GetHP() > 0 ){

		//Mouse-based orientation
		mouse_rx += mouse_x * MouseSensitivity;
		mouse_ry -= mouse_y * MouseSensitivity;

		//Positioning by key operation
		if(      (CheckInputControl(KEY_TURNUP, 0) == true)&&(CheckInputControl(KEY_TURNDOWN, 0) == false) ){ add_camera_ry += (INPUT_ARROWKEYS_ANGLE - add_camera_ry)*0.2f; }			// Standard:[Up]
		else if( (CheckInputControl(KEY_TURNDOWN, 0) == true)&&(CheckInputControl(KEY_TURNUP, 0) == false) ){ add_camera_ry += (INPUT_ARROWKEYS_ANGLE*-1 - add_camera_ry)*0.2f; }		// Standard:[Down]
		else { add_camera_ry = 0.0f; }
		if(      (CheckInputControl(KEY_TURNLEFT, 0) == true)&&(CheckInputControl(KEY_TURNRIGHT, 0) == false) ){ add_camera_rx += (INPUT_ARROWKEYS_ANGLE*-1 - add_camera_rx)*0.2f; }	// Standard:[<-]
		else if( (CheckInputControl(KEY_TURNRIGHT, 0) == true)&&(CheckInputControl(KEY_TURNLEFT, 0) == false) ){ add_camera_rx += (INPUT_ARROWKEYS_ANGLE - add_camera_rx)*0.2f; }		// Standard:[->]
		else { add_camera_rx = 0.0f; }
		mouse_rx += add_camera_rx;
		mouse_ry += add_camera_ry;

		if( mouse_ry > DegreeToRadian(70) ) mouse_ry = DegreeToRadian(70);
		if( mouse_ry < DegreeToRadian(-70) ) mouse_ry = DegreeToRadian(-70);


		//Set the direction of the player (object)
		myHuman->SetRxRy(mouse_rx, mouse_ry);


		//Move (run) or check back and forth
		if( CheckInputControl(KEY_MOVEFORWARD, 0) ){
			ObjMgr.MoveForward(PlayerID);
		}
		else{
			if( CheckInputControl(KEY_MOVEBACKWARD, 0) ){
				ObjMgr.MoveBack(PlayerID);
			}
		}
		if( CheckInputControl(KEY_MOVELEFT, 0) ){
			ObjMgr.MoveLeft(PlayerID);
		}
		else{
			if( CheckInputControl(KEY_MOVERIGHT, 0) ){
				ObjMgr.MoveRight(PlayerID);
			}
		}

		//Walk operation or check
		if( CheckInputControl(KEY_WALK, 0) ){
			ObjMgr.MoveWalk(PlayerID);
		}

		//Jump operation or check
		if( CheckInputControl(KEY_JUMP, 1) ){
			ObjMgr.MoveJump(PlayerID);
		}

		HumanParameter humandata;
		int id_param;
		bool zombie;
		int keymode;

		//Check whether this is a zombie.
		myHuman->GetParamData(&id_param, NULL, NULL, NULL);
		GameParamInfo.GetHuman(id_param, &humandata);
		if( humandata.type == 2 ){
			zombie = true;
		}
		else{
			zombie = false;
		}

		//es resonance mode
		if( zombie == true ){
			keymode = 1;
		}
		else if( myHuman->GetWeaponBlazingmode() == false ){
			keymode = 1;
		}
		else{
			keymode = 0;
		}

		//Check whether the gun operation or
		if( CheckInputControl(KEY_SHOT, keymode) ){

			if( zombie == false ){
				//If the bullet is successfully fired
				if( ObjMgr.ShotWeapon(PlayerID) == 1 ){
					//Add to Score
					MainGameInfo.fire += 1;

					//Get player orientation
					ObjMgr.GetPlayerHumanObject()->GetRxRy(&mouse_rx, &mouse_ry);
				}
			}
			else{
				for(int i=0; i<MAX_HUMAN; i++){
					human *EnemyHuman = ObjMgr.GetHumanObject(i);
					if( ObjMgr.CheckZombieAttack(myHuman, EnemyHuman) == true ){
						ObjMgr.HitZombieAttack(myHuman, EnemyHuman);
					}
				}
			}

		}

		//Reload operation or check
		if( CheckInputControl(KEY_RELOAD, 1) ){
			ObjMgr.ReloadWeapon(PlayerID);
		}

		//Switching weapons or check
		if( CheckInputControl(KEY_SWITCHWEAPON, 1) ){
			ObjMgr.ChangeHaveWeapon(PlayerID, -1);
		}
		if( CheckInputControl(KEY_WEAPON1, 1) ){
			ObjMgr.ChangeHaveWeapon(PlayerID, 0);
		}
		if( CheckInputControl(KEY_WEAPON2, 1) ){
			ObjMgr.ChangeHaveWeapon(PlayerID, 1);
		}

		//Check for a firing-mode change operation.
		if( CheckInputControl(KEY_SHOTMODE, 1) ){
			ObjMgr.ChangeWeaponID(PlayerID);
		}

		//Weapon disposal operation or check
		if( CheckInputControl(KEY_DROPWEAPON, 1) ){
			ObjMgr.DumpWeapon(PlayerID);
		}

		//Scope operation or check
		if( CheckInputControl(KEY_ZOOMSCOPE, 1) ){
			ObjMgr.ChangeScopeMode(PlayerID);
		}

	}

	//Camera display mode change operation or check
	if( inputCtrl->CheckKeyDown( GetFunctionKeycode(1) ) ){
		if( Camera_F1mode == false ){
			Camera_F1mode = true;
			view_rx = 0.0f;
			view_ry = VIEW_F1MODE_ANGLE;
		}
		else{
			Camera_F1mode = false;
			view_rx = 0.0f;
			view_ry = 0.0f;
		}
	}

	//Camera operation
	if( Camera_F1mode == true ){
		if( inputCtrl->CheckKeyNow( OriginalkeycodeToDinputdef(0x0C) ) ){	//NUM8
			view_ry -= INPUT_F1NUMKEYS_ANGLE;
		}
		if( inputCtrl->CheckKeyNow( OriginalkeycodeToDinputdef(0x09) ) ){	//NUM5
			view_ry += INPUT_F1NUMKEYS_ANGLE;
		}
		if( inputCtrl->CheckKeyNow( OriginalkeycodeToDinputdef(0x08) ) ){	//NUM4
			view_rx -= INPUT_F1NUMKEYS_ANGLE;
		}
		if( inputCtrl->CheckKeyNow( OriginalkeycodeToDinputdef(0x0A) ) ){	//NUM6
			view_rx += INPUT_F1NUMKEYS_ANGLE;
		}
	}

	// Normal operation system until now
	//
	// Cheat controls begin here.

	//Check whether to operate or improve your skills.
	if( (inputCtrl->CheckKeyNow( GetFunctionKeycode(5) ))&&(inputCtrl->CheckKeyNow(OriginalkeycodeToDinputdef(0x0F))) ){	// F5 + [ENTER]
		Cmd_F5 = true;
	}
	else{
		Cmd_F5 = false;
	}

	if( myHuman->GetHP() > 0 ){

		//Check whether to use or check backing and bullets.
		if( inputCtrl->CheckKeyNow( GetFunctionKeycode(6) ) ){
			if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x0F)) ){		// [ENTER]
				ObjMgr.CheatAddBullet(PlayerID);
			}
		}

		//Check whether to operate or change weapons
		if( inputCtrl->CheckKeyNow( GetFunctionKeycode(7) ) ){
			if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x02)) ){		// [<-]
				int id_param = myHuman->GetMainWeaponTypeNO();

				//Calculate the next weapon ID.
				if( id_param >= TOTAL_PARAMETERINFO_WEAPON-1 ){ id_param = 0; }
				else{ id_param += 1; }

				ObjMgr.CheatNewWeapon(PlayerID, id_param);
			}
			if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x03)) ){		// [->]
				int id_param = myHuman->GetMainWeaponTypeNO();

				//Calculate the next weapon ID.
				if( id_param <= 0 ){ id_param = TOTAL_PARAMETERINFO_WEAPON-1; }
				else{ id_param -= 1; }

				ObjMgr.CheatNewWeapon(PlayerID, id_param);
			}
		}

	}

	//Check whether to operate or check backing or changing personnel
	if( inputCtrl->CheckKeyNow( GetFunctionKeycode(8) ) ){
		int Player_HumanID;

		if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x02)) ){		// [<-]
			//Get current player number
			Player_HumanID = ObjMgr.GetPlayerID();

			//Calculate the next character.
			Player_HumanID += 1;
			if( Player_HumanID >= MAX_HUMAN ){ Player_HumanID = 0; }

			//Apply the Player Number
			ObjMgr.SetPlayerID(Player_HumanID);

			//Get player orientation
			ObjMgr.GetPlayerHumanObject()->GetRxRy(&mouse_rx, &mouse_ry);

			//Reset camera orientation when F1 mode
			if( Camera_F1mode == true ){
				camera_rx = DegreeToRadian(90);
			}
		}
		if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x03)) ){		// [->]
			//Get current player number
			Player_HumanID = ObjMgr.GetPlayerID();

			//Calculate the next character.
			Player_HumanID -= 1;
			if( Player_HumanID < 0 ){ Player_HumanID = MAX_HUMAN-1; }

			//Apply the Player Number
			ObjMgr.SetPlayerID(Player_HumanID);

			//Get player orientation
			ObjMgr.GetPlayerHumanObject()->GetRxRy(&mouse_rx, &mouse_ry);

			//Reset camera orientation when F1 mode
			if( Camera_F1mode == true ){
				camera_rx = DegreeToRadian(90);
			}
		}
	}

	//Check whether to operate or check whether or not.
	if( inputCtrl->CheckKeyNow( GetFunctionKeycode(9) ) ){
		if( (inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x00)))||(inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x01))) ){		// [Up] and [Down].
			float x, y, z, r;
			int param, dataid, team; 
			int selectweapon;
			weapon *weapon[TOTAL_HAVEWEAPON];
			int weapon_paramid[TOTAL_HAVEWEAPON];
			for(int i=0; i<TOTAL_HAVEWEAPON; i++){
				weapon[i] = NULL;
				weapon_paramid[i] = 0;
			}
			int id;

			//Get player coordinates and weapons
			myHuman->GetPosData(&x, &y, &z, &r);
			myHuman->GetParamData(&param, &dataid, NULL, &team);
			myHuman->GetWeapon(&selectweapon, weapon, NULL, NULL);
			for(int i=0; i<TOTAL_HAVEWEAPON; i++){
				if( weapon[i] != NULL ){
					weapon[i]->GetParamData(&weapon_paramid[i], NULL, NULL);
				}
			}

			//Get the coordinates in front of the player
			x += cosf(r*-1 + (float)M_PI/2)*10.0f;
			y += 5.0f;
			z += sinf(r*-1 + (float)M_PI/2)*10.0f;

			//Add people
			id = ObjMgr.AddHumanIndex(x, y, z, r, param, team, weapon_paramid, 0, false);
			if( id >= 0 ){
				ObjMgr.ChangeHaveWeapon(id, selectweapon);

				//Configure AI
				HumanAI[id].Init();
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x00)) ){		// [Up]
					HumanAI[id].SetHoldTracking(PlayerID);
				}
				if( inputCtrl->CheckKeyDown(OriginalkeycodeToDinputdef(0x01)) ){		// [Down]
					HumanAI[id].SetHoldWait(x, z, r);
				}
			}
		}
	}

	//Check whether to use or check backing and arm drawing
	if( inputCtrl->CheckKeyDown( GetHomeKeycode() ) ){
		if( Camera_HOMEmode == false ){
			Camera_HOMEmode = true;
		}
		else{
			Camera_HOMEmode = false;
		}
	}
}

//! @brief Process free-camera input.
//! @param mouse_x Mouse X Coordinate
//! @param mouse_y Mouse Y coordinates
//! @param MouseSensitivity Calculate viewpoint movement.
void maingame::InputViewCamera(int mouse_x, int mouse_y, float MouseSensitivity)
{
	//Apply mouse movement as camera orientation
	camera_rx -= mouse_x * MouseSensitivity;
	camera_ry -= mouse_y * MouseSensitivity;

	//Positioning by key operation
	if( CheckInputControl(KEY_TURNUP, 0) ){ camera_ry += INPUT_ARROWKEYS_ANGLE; }			// Standard:[Up]
	if( CheckInputControl(KEY_TURNDOWN, 0) ){ camera_ry -= INPUT_ARROWKEYS_ANGLE; }			// Standard:[Down]
	if( CheckInputControl(KEY_TURNLEFT, 0) ){ camera_rx += INPUT_ARROWKEYS_ANGLE; }			// Standard:[<-]
	if( CheckInputControl(KEY_TURNRIGHT, 0) ){ camera_rx -= INPUT_ARROWKEYS_ANGLE; }		// Standard:[->]

	if( camera_ry > DegreeToRadian(70) ) camera_ry = DegreeToRadian(70);
	if( camera_ry < DegreeToRadian(-70) ) camera_ry = DegreeToRadian(-70);

	//Determination of moving amount
	float dist = VIEW_FREECAMERA_SCALE;
	if( CheckInputControl(KEY_SHOT, 0) ){
		dist *= 2;
	}

	//Move camera coordinates by key operation
	if( CheckInputControl(KEY_MOVEFORWARD, 0) ){
		camera_x += cosf(camera_rx)*cosf(camera_ry)*dist;
		camera_y += sinf(camera_ry)*dist;
		camera_z += sinf(camera_rx)*cosf(camera_ry)*dist;
	}
	if( CheckInputControl(KEY_MOVEBACKWARD, 0) ){
		camera_x -= cosf(camera_rx)*cosf(camera_ry)*dist;
		camera_y -= sinf(camera_ry)*dist;
		camera_z -= sinf(camera_rx)*cosf(camera_ry)*dist;
	}
	if( CheckInputControl(KEY_MOVELEFT, 0) ){
		camera_x += cosf(camera_rx + (float)M_PI/2)*dist;
		camera_z += sinf(camera_rx + (float)M_PI/2)*dist;
	}
	if( CheckInputControl(KEY_MOVERIGHT, 0) ){
		camera_x += cosf(camera_rx - (float)M_PI/2)*dist;
		camera_z += sinf(camera_rx - (float)M_PI/2)*dist;
	}
}

void maingame::Process()
{
	//Get Player Class
	human *myHuman = ObjMgr.GetPlayerHumanObject();

	float ontarget;
	int kill, headshot;

	//-----------------------------------

	time = GetTimeMS();

	//ute object manager
	int cmdF5id;
	if( Cmd_F5 == true ){
		cmdF5id = ObjMgr.GetPlayerID();
	}
	else{
		cmdF5id = -1;
	}
	ObjMgr.Process(cmdF5id, false, DarkScreenFlag);

	//Add Player's War History
	ObjMgr.GetHumanShotInfo( ObjMgr.GetPlayerID(), &ontarget, &kill, &headshot);
	MainGameInfo.ontarget += ontarget;
	MainGameInfo.kill += kill;
	MainGameInfo.headshot += headshot;

	time_process_object = GetTimeMS() - time;

	//-----------------------------------

	time = GetTimeMS();
	if( AIstop == false ){
		int PlayerID = ObjMgr.GetPlayerID();
		for(int i=0; i<MAX_HUMAN; i++){
			if( (PlayerAI == false)&&(i == PlayerID) ){ continue; }
			//Run the AI.
			HumanAI[i].Process();
		}
	}
	time_process_ai = GetTimeMS() - time;

	//-----------------------------------

	//If the mission is not finished,
	if( end_framecnt == 0 ){
		int check = ObjMgr.CheckGameOverorComplete();

		//Game Clear Determination
		if( check == 1 ){
			end_framecnt += 1;
			MainGameInfo.missioncomplete = true;
		}

		//Game Over Judgment
		if( check == 2 ){
			end_framecnt += 1;
			MainGameInfo.missioncomplete = false;
		}
	}

	//-----------------------------------

	time = GetTimeMS();
	if( EventStop == false ){
		//Execute the event.
		for(int i=0; i<TOTAL_EVENTLINE; i++){
			if( end_framecnt == 0 ){
				EventParameter EventParam;

				//Store the event execution state in the structure.
				EventParam.endcnt				 = end_framecnt;
				EventParam.complete				 = MainGameInfo.missioncomplete;
				EventParam.MessageID			 = message_id;
				EventParam.SetMessageID			 = false;

				//Execute an event point.
				Event[i].ProcessEventPoint(EventSkip[i], &EventParam);

				//Back to each variable from the structure
				end_framecnt						 = EventParam.endcnt;
				MainGameInfo.missioncomplete		 = EventParam.complete;
				message_id							 = EventParam.MessageID;

				//Back event skip flags
				EventSkip[i] = false;

				//Once the event message has been reset, the count will be returned.
				if( EventParam.SetMessageID == true ){
					message_cnt = 0;
				}
			}
		}
	}

	if( (message_id != -1)&&(message_cnt < (int)(TOTAL_EVENTENT_SHOWMESSEC*GAMEFPS)) ){
		message_cnt += 1;
	}
	else{
		message_id = -1;
		message_cnt = 0;
	}
	time_process_event = GetTimeMS() - time;

	//-----------------------------------

	float x, y, z;
	myHuman->GetPosData(&x, &y, &z, NULL);

	if( PlayerAI == true ){
		//Get player orientation
		ObjMgr.GetPlayerHumanObject()->GetRxRy(&mouse_rx, &mouse_ry);
	}

	//Seeking Camera Work
	if( Camera_Debugmode == true ){
		//
	}
	else if( myHuman->GetDeadFlag() == true ){
		float camera_ry2 = camera_ry;

		//Normalize camera_ry2 to the range -PI to PI.
		camera_ry2 = AngleNormalization(camera_ry2);

		float ry = camera_ry2*0.95f + DegreeToRadian(-89.0f)*0.05f;						// 19/20 + 1/20
		float r = 3.12f;

		camera_rx += DegreeToRadian(1.0f);
		camera_ry = ry;
		camera_x = x + cosf(camera_rx)*r;
		camera_y = y + 33.3f;
		camera_z = z + sinf(camera_rx)*r;
	}
	else if( myHuman->GetDeadMotionFlag() == true ){	//myHuman->GetDeadFlag() == false
		//Do not change camera coordinates during death motion
	}
	else if( Camera_F1mode == true ){
		float crx, cry;
		float ccx, ccy, ccz;

		//View the camera's point of view
		crx = camera_rx*0.8f + (view_rx + mouse_rx*-1 + (float)M_PI/2)*0.2f;	// 8 : 2
		cry = camera_ry*0.8f + (view_ry + mouse_ry)*0.2f - (float)M_PI/2;		// 8 : 2
		ccx = x - cosf(crx)*cosf(cry)*3.0f;
		ccy = y + HUMAN_HEIGHT-0.5f + sinf(cry*-1)*2.5f;
		ccz = z - sinf(crx)*cosf(cry)*3.0f;

		//From the point of view to the camera
		cry += (float)M_PI/2;
		float dist;
		if( CollD.CheckALLBlockIntersectRay(ccx, ccy, ccz, cosf(crx)*cosf(cry)*-1, sinf(cry*-1), sinf(crx)*cosf(cry)*-1, NULL, NULL, &dist, VIEW_F1MODE_DIST) == true ){
			dist -= 1.0f;
		}
		else{
			dist = VIEW_F1MODE_DIST;
		}

		//Recalculate the camera coordinates.
		camera_x = ccx - cosf(crx)*cosf(cry)*(dist);
		camera_y = ccy + sinf(cry*-1)*dist;
		camera_z = ccz - sinf(crx)*cosf(cry)*(dist);
		camera_rx = crx;
		camera_ry = cry;
	}
	else{
		float crx = view_rx + mouse_rx*-1 + (float)M_PI/2;
		float cry = view_ry + mouse_ry;

		camera_x = x + cosf(crx)*cosf(cry)*VIEW_DIST;
		camera_y = y + VIEW_HEIGHT + sinf(cry)*VIEW_DIST;
		camera_z = z + sinf(crx)*cosf(cry)*VIEW_DIST;
		camera_rx = crx;
		camera_ry = cry;
	}

	//Draw a red flash if damaged
	redflash_flag = myHuman->CheckHit(NULL);

	//-----------------------------------

#ifdef ENABLE_DEBUGCONSOLE
	//If the debug console is visible and the mission-end countdown has not started.
	if( (Show_Console == true)&&(end_framecnt == 0) ){
		//Main processing for the debug console.
		ProcessConsole();
	}
#endif

	//-----------------------------------

	framecnt += 1;
    if(!elapsed_stopped){
        elapsed_ticks = framecnt;
        if(end_framecnt > 0 || myHuman->GetHP() <= 0) elapsed_stopped = true;
    }

	if( start_framecnt < (int)(2.0f*GAMEFPS) ){	//If you are starting a mission
		start_framecnt += 1;
	}
	if( end_framecnt == 1 ){					//If the mission is not completed
		MainGameInfo.framecnt = framecnt;
		memcpy(&GameInfoData, &MainGameInfo, sizeof(GameInfo));		//Copy All
		end_framecnt += 1;
	}
	else if( end_framecnt > 0 ){				//If the mission is finished
		if( end_framecnt < (int)(4.0f*GAMEFPS) ){
			end_framecnt += 1;
		}
		else{
			GameInfoData.fire = MainGameInfo.fire;			//Number of shots fired.
			GameInfoData.ontarget = MainGameInfo.ontarget;	//Number of hits.
			GameInfoData.kill = MainGameInfo.kill;			//Number of defeated enemies
			GameInfoData.headshot = MainGameInfo.headshot;	//The number of lives in the enemy head

			GameState->PushMouseButton();
			GameSpeed = 1;
		}
	}
}

void maingame::Sound()
{
	time = GetTimeMS();

	//Get player's team number
	int teamid;
	human *myHuman = ObjMgr.GetPlayerHumanObject();
	myHuman->GetParamData(NULL, NULL, NULL, &teamid);

	//Play Sound
	GameSound->PlayWorldSound(camera_x, camera_y, camera_z, camera_rx, teamid);

	time_sound = GetTimeMS() - time;
}

void maingame::Render3D()
{
	time = GetTimeMS();


	int skymodel, skytexture;
	human *myHuman = ObjMgr.GetPlayerHumanObject();

	//Configure fog and camera
	d3dg->SetFog(FogFlag, SkyNumber);
	if( (Camera_F1mode == false)&&(Camera_Debugmode == false) ){
		ScopeParameter sparam;
		int scopemode = myHuman->GetScopeMode();
		GameParamInfo.GetScopeParam(scopemode, &sparam);

		d3dg->SetCamera(camera_x, camera_y, camera_z, camera_rx, camera_ry, sparam.ViewAngle);
	}
	else{
		ScopeParameter sparam;
		GameParamInfo.GetScopeParam(0, &sparam);

		d3dg->SetCamera(camera_x, camera_y, camera_z, camera_rx, camera_ry, sparam.ViewAngle);
	}

	//Draw the background sky to the camera coordinate
	d3dg->SetWorldTransform(camera_x, camera_y, camera_z, 0.0f, 0.0f, 2.0f);
	Resource.GetSkyModelTexture(&skymodel, &skytexture);
	d3dg->RenderModel(skymodel, skytexture, DarkScreenFlag, false, false);

	//Initialize Z buffer
	d3dg->ResetZbuffer();

	if( CenterLine == true ){
		//Display center lines for debugging.
		d3dg->RenderCenterline();
	}

	//Render the map.
	d3dg->ResetWorldTransform();
	d3dg->RenderMapdata(wireframe);

	//Determination without player drawing
	int DrawPlayer = 0;
	if( (Camera_F1mode == false)&&(Camera_Debugmode == false)&&(myHuman->GetHP() > 0) ){
		if( Camera_HOMEmode == false ){
			DrawPlayer = 1;
		}
		else{
			DrawPlayer = 2;
		}
	}
	//Draw an object
	ObjMgr.Render(camera_x, camera_y, camera_z, camera_rx, camera_ry, DrawPlayer, nomodel);

	//Display AI debug information.
	if( AIdebuginfoID != -1 ){
		if( (0 <= AIdebuginfoID)&&(AIdebuginfoID < MAX_HUMAN) ){
			float posx, posy, posz, rx;
			int EnemyID;
			float mposx, mposz;
			int movemode;
			ObjMgr.GetHumanObject(AIdebuginfoID)->GetPosData(&posx, &posy, &posz, &rx);
			EnemyID = HumanAI[AIdebuginfoID].GetEnemyHumanID();
			HumanAI[AIdebuginfoID].GetMoveTargetPos(&mposx, &mposz, &movemode);

			d3dg->ResetWorldTransform();

			//The character itself.
			d3dg->Renderline(posx+10.0f, posy, posz, posx-10.0f, posy, posz, d3dg->GetColorCode(0.0f,0.0f,1.0f,1.0f));
			d3dg->Renderline(posx, posy+10.0f, posz, posx, posy-10.0f, posz, d3dg->GetColorCode(0.0f,0.0f,1.0f,1.0f));
			d3dg->Renderline(posx, posy, posz+10.0f, posx, posy, posz-10.0f, d3dg->GetColorCode(0.0f,0.0f,1.0f,1.0f));

			//Destination.
			d3dg->Renderline(mposx+10.0f, posy, mposz, mposx-10.0f, posy, mposz, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
			d3dg->Renderline(mposx, 5000.0f, mposz, mposx, -500.0f,mposz, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
			d3dg->Renderline(mposx, posy, mposz+10.0f, mposx, posy, mposz-10.0f, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));

			if( EnemyID != -1 ){
				ObjMgr.GetHumanObject(EnemyID)->GetPosData(&posx, &posy, &posz, &rx);

				//Attack target
				d3dg->Renderline(posx+3.0f, posy, posz+3.0f, posx+3.0f, posy, posz-3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx+3.0f, posy, posz-3.0f, posx-3.0f, posy, posz-3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx-3.0f, posy, posz-3.0f, posx-3.0f, posy, posz+3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx-3.0f, posy, posz+3.0f, posx+3.0f, posy, posz+3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx+3.0f, posy+HUMAN_HEIGHT, posz+3.0f, posx+3.0f, posy+HUMAN_HEIGHT, posz-3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx+3.0f, posy+HUMAN_HEIGHT, posz-3.0f, posx-3.0f, posy+HUMAN_HEIGHT, posz-3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx-3.0f, posy+HUMAN_HEIGHT, posz-3.0f, posx-3.0f, posy+HUMAN_HEIGHT, posz+3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx-3.0f, posy+HUMAN_HEIGHT, posz+3.0f, posx+3.0f, posy+HUMAN_HEIGHT, posz+3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx+3.0f, posy, posz+3.0f, posx+3.0f, posy+HUMAN_HEIGHT, posz+3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx+3.0f, posy, posz-3.0f, posx+3.0f, posy+HUMAN_HEIGHT, posz-3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx-3.0f, posy, posz-3.0f, posx-3.0f, posy+HUMAN_HEIGHT, posz-3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
				d3dg->Renderline(posx-3.0f, posy, posz+3.0f, posx-3.0f, posy+HUMAN_HEIGHT, posz+3.0f, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
			}
		}
	}
}

void maingame::Render2D()
{
	char str[256];
	unsigned char stru[256];
	float fps = GetFps(10);
	float effect;

	human *myHuman = ObjMgr.GetPlayerHumanObject();

	int selectweapon;
	weapon *weapon[TOTAL_HAVEWEAPON];
	int weapon_paramid[TOTAL_HAVEWEAPON];
	int lnbslist[TOTAL_HAVEWEAPON];
	int nbslist[TOTAL_HAVEWEAPON];
	WeaponParameter weapon_paramdata;
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		weapon[i] = NULL;
		weapon_paramid[i] = 0;
		lnbslist[i] = 0;
		nbslist[i] = 0;
	}
	int lnbs = 0;
	int nbs = 0;
	int reloadcnt = 0;
	int selectweaponcnt = 0;
	int weaponmodel, weapontexture;
	char weaponname[24+1];
	int hp;
	int param_scopemode;
	int param_WeaponP;
	int param_pellet;
	int ErrorRange;

	//Get various settings and game information
	myHuman->GetWeapon(&selectweapon, weapon, lnbslist, nbslist);
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		if( selectweapon == i ){
			//Main weapons
			//If you have a weapon, you will get the number of bullets and the number of bullets when you are not .
			if( weapon[i] != NULL ){
				weapon[i]->GetParamData(&weapon_paramid[i], &lnbs, &nbs);
			}
			else{
				lnbs = lnbslist[i];
				nbs = nbslist[i];
			}
		}
		else{
			//Sub Weapons Sub Weapons Only Weapons
			if( weapon[i] != NULL ){
				weapon[i]->GetParamData(&weapon_paramid[i], NULL, NULL);
			}
		}
	}
	reloadcnt = myHuman->GetWeaponReloadCnt();
	selectweaponcnt = myHuman->GetChangeHaveWeaponCnt() + myHuman->GetChangeWeaponIDCnt();
	GameParamInfo.GetWeapon(weapon_paramid[selectweapon], &weapon_paramdata);
	//strcpy(weaponname, weapon_paramdata.name);
	strcpy(str, weapon_paramdata.name);
	str[sizeof(weaponname)-1] = '\0';
	strcpy(weaponname, str);
	hp = myHuman->GetHP();
	param_scopemode = weapon_paramdata.scopemode;
	param_WeaponP = weapon_paramdata.WeaponP;
	param_pellet = weapon_paramdata.pellet;
	ErrorRange = myHuman->GetGunsightErrorRange();

	float human_x, human_y, human_z, human_rx;
	myHuman->GetPosData(&human_x, &human_y, &human_z, &human_rx);

	//Color Determination by HP
	int statecolor;
	if( hp >= 100 ){
		statecolor = d3dg->GetColorCode(0.0f,1.0f,0.0f,1.0f);
	}
	else if( hp >= 50 ){
		statecolor = d3dg->GetColorCode(1.0f/50*(100-hp),1.0f,0.0f,1.0f);
	}
	else if( hp > 0 ){
		statecolor = d3dg->GetColorCode(1.0f,1.0f/50*hp,0.0f,1.0f);
	}
	else{
		statecolor = d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f);
	}

	//Red Flash Drawing
	if( (redflash_flag == true)&&(Camera_Debugmode == false) ){
		d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(1.0f,0.0f,0.0f,0.5f));
		redflash_flag = false;
	}

	//Scope drawing
	if( (Camera_F1mode == false)&&(Camera_Debugmode == false)&&(myHuman->GetScopeMode() != 0) ){
		ScopeParameter sparam;
		GameParamInfo.GetScopeParam(myHuman->GetScopeMode(), &sparam);

		if( ((float)GameConfig.GetScreenWidth() / GameConfig.GetScreenHeight()) > 1.5f ){
			int swidth = (int)((float)GameConfig.GetScreenHeight() * 1.333f)+1;
			d3dg->Draw2DTexture((GameConfig.GetScreenWidth()-swidth)/2, 0, Resource.GetScopeTexture(), swidth, GameConfig.GetScreenHeight(), 1.0f);
			d3dg->Draw2DBox(0, 0, (GameConfig.GetScreenWidth()-swidth)/2, GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DBox(GameConfig.GetScreenWidth() - (GameConfig.GetScreenWidth()-swidth)/2, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
		}
		else{
			d3dg->Draw2DTexture(0, 0, Resource.GetScopeTexture(), GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), 1.0f);
		}

		if( sparam.ScopeGunsight == 1 ){
			d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2-49, GameConfig.GetScreenHeight()/2, GameConfig.GetScreenWidth()/2-4, GameConfig.GetScreenHeight()/2, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2+4, GameConfig.GetScreenHeight()/2, GameConfig.GetScreenWidth()/2+49, GameConfig.GetScreenHeight()/2, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2-49, GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2-4, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2+4, GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2+49, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DBox(GameConfig.GetScreenWidth()/2-50, GameConfig.GetScreenHeight()/2-1, GameConfig.GetScreenWidth()/2+50, GameConfig.GetScreenHeight()/2+1, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));
			d3dg->Draw2DBox(GameConfig.GetScreenWidth()/2-1, GameConfig.GetScreenHeight()/2-50, GameConfig.GetScreenWidth()/2+1, GameConfig.GetScreenHeight()/2+50, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));
		}
		if( sparam.ScopeGunsight == 2 ){
			int w;
			if( ((float)GameConfig.GetScreenWidth() / GameConfig.GetScreenHeight()) > 1.5f ){
				int swidth = (int)((float)GameConfig.GetScreenHeight() * 1.333f)+1;
				w = 140 + (GameConfig.GetScreenWidth()-swidth)/2;
			}
			else{
				w = 140;
			}
			d3dg->Draw2DLine(w, GameConfig.GetScreenHeight()/2, GameConfig.GetScreenWidth() -w, GameConfig.GetScreenHeight()/2, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2, 60, GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight() -60, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DBox(w, GameConfig.GetScreenHeight()/2-1, GameConfig.GetScreenWidth() -w, GameConfig.GetScreenHeight()/2+1, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));
			d3dg->Draw2DBox(GameConfig.GetScreenWidth()/2-1, 60, GameConfig.GetScreenWidth()/2+1, GameConfig.GetScreenHeight() -60, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.5f));
		}
	}

	//Blindfolded Drawing
	if( (Camera_Blind == true)&&(Camera_Debugmode == false)&&(hp > 0) ){

		int scopemode = myHuman->GetScopeMode();
		float adddist = 1.2f;
		ScopeParameter sparam;

		//Determines the angle of view by scope
		GameParamInfo.GetScopeParam(scopemode, &sparam);
		float addang = sparam.ViewAngle / 4;

		//Up.
		if( CollD.CheckALLBlockInside(camera_x + cosf(camera_rx)*cosf(camera_ry + addang) * adddist, camera_y + sinf(camera_ry + addang) * adddist, camera_z + sinf(camera_rx)*cosf(camera_ry + addang) * adddist) == true ){
			d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight()/2, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
		}

		//Down.
		if( CollD.CheckALLBlockInside(camera_x + cosf(camera_rx)*cosf(camera_ry - addang) * adddist, camera_y + sinf(camera_ry - addang) * adddist, camera_z + sinf(camera_rx)*cosf(camera_ry - addang) * adddist) == true ){
			d3dg->Draw2DBox(0, GameConfig.GetScreenHeight()/2, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
		}

		//Left
		if( CollD.CheckALLBlockInside(camera_x + cosf(camera_rx + addang)*cosf(camera_ry) * adddist, camera_y + sinf(camera_ry) * adddist, camera_z + sinf(camera_rx + addang)*cosf(camera_ry) * adddist) == true ){
			d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
		}

		//Right
		if( CollD.CheckALLBlockInside(camera_x + cosf(camera_rx - addang)*cosf(camera_ry) * adddist, camera_y + sinf(camera_ry) * adddist, camera_z + sinf(camera_rx - addang)*cosf(camera_ry) * adddist) == true ){
			d3dg->Draw2DBox(GameConfig.GetScreenWidth()/2, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
		}
	}

#ifdef ENABLE_DEBUGCONSOLE
	//Display debug and game information.
	if( ShowInfo_Debugmode == true ){
		float move_x, move_y, move_z;
		int keyflag;
		int block_id;
		myHuman->GetMovePos(&move_x, &move_y, &move_z);
		keyflag = myHuman->GetMoveFlag(false);
		myHuman->GetUnderBlock(&block_id, NULL);

		//Display text using the texture font (ASCII letters, numbers, and symbols only).
		sprintf(str, "frame:%d   time %02d:%02d", framecnt, framecnt/(int)GAMEFPS/60, framecnt/(int)GAMEFPS%60);
		d3dg->Draw2DTextureDebugFontText(10+1, 10+1, str, d3dg->GetColorCode(0.1f,0.1f,0.1f,1.0f));
		d3dg->Draw2DTextureDebugFontText(10, 10, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f));
		sprintf(str, "camera x:%.2f y:%.2f z:%.2f rx:%.2f ry:%.2f", camera_x, camera_y, camera_z, camera_rx, camera_ry);
		d3dg->Draw2DTextureDebugFontText(10+1, 30+1, str, d3dg->GetColorCode(0.1f,0.1f,0.1f,1.0f));
		d3dg->Draw2DTextureDebugFontText(10, 30, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f));
		sprintf(str, "human[%d] x:%.2f y:%.2f z:%.2f rx:%.2f HP:%d MoveKey:0x%02X", ObjMgr.GetPlayerID(), human_x, human_y, human_z, human_rx, hp, keyflag);
		d3dg->Draw2DTextureDebugFontText(10+1, 50+1, str, d3dg->GetColorCode(0.1f,0.1f,0.1f,1.0f));
		d3dg->Draw2DTextureDebugFontText(10, 50, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f));
		sprintf(str, "          move_x:%.2f move_y:%.2f move_z:%.2f BlockID:%d", move_x, move_y, move_z, block_id);
		d3dg->Draw2DTextureDebugFontText(10+1, 70+1, str, d3dg->GetColorCode(0.1f,0.1f,0.1f,1.0f));
		d3dg->Draw2DTextureDebugFontText(10, 70, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f));
		sprintf(str, "Input:%02dms Object:%02dms AI:%02dms Event:%02dms Sound:%02dms Render:%02dms", time_input, time_process_object, time_process_ai, time_process_event, time_sound, time_render);
		d3dg->Draw2DTextureDebugFontText(10+1, 90+1, str, d3dg->GetColorCode(0.1f,0.1f,0.1f,1.0f));
		d3dg->Draw2DTextureDebugFontText(10, 90, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f));
	}
#endif

	//Show game execution speed
	//int speed = (int)(fps / (1000.0f/GAMEFRAMEMS) * 100);
	//sprintf(str, "PROCESSING SPEED %d%%", speed);
	sprintf(str, "fps:%.2f", fps);
	d3dg->Draw2DTextureFontText(GameConfig.GetScreenWidth() - strlen(str)*10 - 5 +1, 5+1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 10, 14);
	d3dg->Draw2DTextureFontText(GameConfig.GetScreenWidth() - strlen(str)*10 - 5, 5, str, d3dg->GetColorCode(1.0f,0.5f,0.0f,1.0f), 10, 14);

    // Match the FPS color, font size, and shadow; align to the left game edge.
    const unsigned long long elapsedSeconds = (unsigned long long)elapsed_ticks * GAMEFRAMEMS / 1000;
    sprintf(str, "time:%02llu:%02llu", elapsedSeconds/60, elapsedSeconds%60);
    d3dg->Draw2DTextureFontText(6, 6, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 10, 14);
    d3dg->Draw2DTextureFontText(5, 5, str, d3dg->GetColorCode(1.0f,0.5f,0.0f,1.0f), 10, 14);

	//HUD Display Mode A
	if( Camera_F2mode == 0 ){
		//Lower left area drawing
		//"\xB3\xB4\xB4\xB4\xB4\xB4\xB4\xB5"
		stru[0] = 0xB3;		stru[1] = 0xB4;		stru[2] = 0xB4;		stru[3] = 0xB4;		stru[4] = 0xB4;
		stru[5] = 0xB4;		stru[6] = 0xB4;		stru[7] = 0xB5;		stru[8] = '\0';
		d3dg->Draw2DTextureFontText(15, GameConfig.GetScreenHeight() - 105, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32);
		//"\xC3\xC4\xC4\xC4\xC4\xC4\xC4\xC5"
		for(int i=0; stru[i] != 0x00; i++){ stru[i] += 0x10; }
		d3dg->Draw2DTextureFontText(15, GameConfig.GetScreenHeight() - 105 +32, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32);
		//"\xB3\xB4\xB4\xB6\xB7\xB7\xB7\xB8\xB9"
		stru[0] = 0xB3;		stru[1] = 0xB4;		stru[2] = 0xB4;		stru[3] = 0xB6;		stru[4] = 0xB7;
		stru[5] = 0xB7;		stru[6] = 0xB7;		stru[7] = 0xB8;		stru[8] = 0xB9;		stru[9] = '\0';
		d3dg->Draw2DTextureFontText(15, GameConfig.GetScreenHeight() - 55, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32);
		//"\xC3\xC4\xC4\xC6\xC7\xC7\xC7\xC8\xC9"
		for(int i=0; stru[i] != 0x00; i++){ stru[i] += 0x10; }
		d3dg->Draw2DTextureFontText(15, GameConfig.GetScreenHeight() - 55 +32, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32);

		//Character code setting for the lower right area
		stru[0] = 0xB0;//'\xB0';
		for(int i=1; i<HUDA_WEAPON_SIZEW-1; i++){
			stru[i] = 0xB1;//'\xB1';
		}
		stru[HUDA_WEAPON_SIZEW-1] = 0xB2;//'\xB2';
		stru[HUDA_WEAPON_SIZEW] = '\0';

		//Right-bottom area drawing
		d3dg->Draw2DTextureFontText(HUDA_WEAPON_POSX, HUDA_WEAPON_POSY, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32);
		for(int i=0; i<HUDA_WEAPON_SIZEW; i++){ stru[i] += 0x10; }
		for(int i=1; i<HUDA_WEAPON_SIZEH-1; i++){
			d3dg->Draw2DTextureFontText(HUDA_WEAPON_POSX, HUDA_WEAPON_POSY + 32*i, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32);
		}
		for(int i=0; i<HUDA_WEAPON_SIZEW; i++){ stru[i] += 0x10; }
		d3dg->Draw2DTextureFontText(HUDA_WEAPON_POSX, HUDA_WEAPON_POSY + 32*(HUDA_WEAPON_SIZEH-1), (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32);

		//Number of weapons
		sprintf((char*)stru, "A%d B%d", lnbs, (nbs - lnbs));
		for(int i=0; i<(int)strlen((char*)stru); i++){
			if( stru[i] == 'A' ){ stru[i] = 0xBB; }	//'\xBB'
			if( stru[i] == 'B' ){ stru[i] = 0xBA; }	//'\xBA'
		}
		d3dg->Draw2DTextureFontText(25, GameConfig.GetScreenHeight() - 96, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 23, 24);

		//HP display
		if( hp >= 80 ){
			d3dg->Draw2DTextureFontText(23, GameConfig.GetScreenHeight() - 45, "STATE", statecolor, 18, 24);
			d3dg->Draw2DTextureFontText(155, GameConfig.GetScreenHeight() - 45, "FINE", statecolor, 18, 24);
		}
		else if( hp >= 40 ){
			d3dg->Draw2DTextureFontText(23, GameConfig.GetScreenHeight() - 45, "STATE", statecolor, 18, 24);
			d3dg->Draw2DTextureFontText(135, GameConfig.GetScreenHeight() - 45, "CAUTION", statecolor, 18, 24);
		}
		else if( hp > 0 ){
			d3dg->Draw2DTextureFontText(23, GameConfig.GetScreenHeight() - 45, "STATE", statecolor, 18, 24);
			d3dg->Draw2DTextureFontText(140, GameConfig.GetScreenHeight() - 45, "DANGER", statecolor, 18, 24);
		}
		else{
			d3dg->Draw2DTextureFontText(23, GameConfig.GetScreenHeight() - 45, "STATE", statecolor, 18, 24);
			d3dg->Draw2DTextureFontText(155, GameConfig.GetScreenHeight() - 45, "DEAD", statecolor, 18, 24);
		}

		//Weapon Name Display
#ifdef ENABLE_WEAPONNAME_SCALING
		int w, h;
		d3dg->GetFontTextSize(weaponname, 16, 20, 14, &w, &h);
		d3dg->Draw2DTextureFontText(HUDA_WEAPON_POSX + 9, HUDA_WEAPON_POSY + 4 + (20 - h)/2, weaponname, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), w, h);
#else
		d3dg->Draw2DTextureFontText(HUDA_WEAPON_POSX + 9, HUDA_WEAPON_POSY + 4, weaponname, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 16, 20);
#endif
	}

	//HUD display, mode B.
	if( Camera_F2mode == 1 ){
		//Line around the screen
		d3dg->Draw2DLine(0, 0, GameConfig.GetScreenWidth()-1, 0, statecolor);
		d3dg->Draw2DLine(GameConfig.GetScreenWidth()-1, 0, GameConfig.GetScreenWidth()-1, GameConfig.GetScreenHeight()-1, statecolor);
		d3dg->Draw2DLine(0, 0, 0, GameConfig.GetScreenHeight()-1, statecolor);
		d3dg->Draw2DLine(0, GameConfig.GetScreenHeight()-1, GameConfig.GetScreenWidth()-1, GameConfig.GetScreenHeight()-1, statecolor);

		//Weapon Name Display
#ifdef ENABLE_WEAPONNAME_SCALING
		int w, h;
		d3dg->GetFontTextSize(weaponname, 16, 20, 14, &w, &h);

		d3dg->Draw2DBox(8, GameConfig.GetScreenHeight() - 32, 227, GameConfig.GetScreenHeight() - 7, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.3f));
		d3dg->Draw2DTextureFontText(10, GameConfig.GetScreenHeight() - 30 + (20 - h)/2, weaponname, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), w, h);
#else
		d3dg->Draw2DBox(8, GameConfig.GetScreenHeight() - 32, 227, GameConfig.GetScreenHeight() - 7, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.3f));
		d3dg->Draw2DTextureFontText(10, GameConfig.GetScreenHeight() - 30, weaponname, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 16, 20);
#endif
	}

	//Render the radar.
	if( radar == true ){
		RenderRadar();
	}

	//Event message display
	if( (message_id != -1)&&(message_cnt < (int)(TOTAL_EVENTENT_SHOWMESSEC*GAMEFPS)) ){
		char messtr[MAX_POINTMESSAGEBYTE];
		PointData.GetMessageText(messtr, message_id);
		float effectA = 1.0f;
		if( message_cnt < (int)(0.2f*GAMEFPS) ){ effectA = GetEffectAlpha(message_cnt, 1.0f, 0.2f, 0.0f, false); }
		if( (int)((TOTAL_EVENTENT_SHOWMESSEC-0.2f)*GAMEFPS) < message_cnt ){ effectA = GetEffectAlpha(message_cnt, 1.0f, 0.2f, (TOTAL_EVENTENT_SHOWMESSEC - 0.2f), true); }
		d3dg->Draw2DMSFontTextCenter(1, GameConfig.GetScreenHeight()*3/4 + 1, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight()/4, messtr, d3dg->GetColorCode(0.1f,0.1f,0.1f,effectA));
		d3dg->Draw2DMSFontTextCenter(0, GameConfig.GetScreenHeight()*3/4, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight()/4, messtr, d3dg->GetColorCode(1.0f,1.0f,1.0f,effectA));
	}

#ifdef ENABLE_DEBUGCONSOLE
	//Display debug and game information.
	if( ShowInfo_Debugmode == true ){
		ObjMgr.RenderLog(HUDA_WEAPON_POSX, HUDA_WEAPON_POSY-60);
	}
#endif

	//Reload display
	if( reloadcnt > 0 ){
		d3dg->Draw2DTextureFontTextCenterScaling(3, 300+3, "RELOADING", d3dg->GetColorCode(0.2f,0.2f,0.2f,1.0f), 32, 34);
		d3dg->Draw2DTextureFontTextCenterScaling(0, 300, "RELOADING", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 32, 34);
	}

	//Weapon Switch Display
	if( selectweaponcnt > 0 ){
		d3dg->Draw2DTextureFontTextCenterScaling(3, 300+3, "CHANGING", d3dg->GetColorCode(0.2f,0.2f,0.2f,1.0f), 32, 34);
		d3dg->Draw2DTextureFontTextCenterScaling(0, 300, "CHANGING", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 32, 34);
	}

	//Display the crosshair.
	if( (Camera_F1mode == false)&&(param_WeaponP != 2)&&(param_pellet > 0) ){
		if( (weapon[selectweapon] != NULL) ){
			int scopemode = myHuman->GetScopeMode();
			ScopeParameter sparam;
			GameParamInfo.GetScopeParam(param_scopemode, &sparam);

			if( ( (scopemode == 0)&&(sparam.NoScopeGunsight == true) )||( (scopemode != 0)&&(sparam.ScopeGunsight == 0) ) ){
				if( GameConfig.GetAnotherGunsightFlag() ){	//Optional Type
					//Crosshair opacity.
					float alpha = 1.0f - (float)ErrorRange/40.0f;
					if( alpha < 0.0f ){ alpha = 0.0f; }

					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2, GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2+4, d3dg->GetColorCode(1.0f,0.0f,0.0f,0.5f));
					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2-15, GameConfig.GetScreenHeight()/2+15, GameConfig.GetScreenWidth()/2-19, GameConfig.GetScreenHeight()/2+19, d3dg->GetColorCode(1.0f,0.0f,0.0f,0.5f));
					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2+15, GameConfig.GetScreenHeight()/2+15, GameConfig.GetScreenWidth()/2+19, GameConfig.GetScreenHeight()/2+19, d3dg->GetColorCode(1.0f,0.0f,0.0f,0.5f));
					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2-4, GameConfig.GetScreenHeight()/2+4, GameConfig.GetScreenWidth()/2+4, GameConfig.GetScreenHeight()/2+4, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));

					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2-4 - ErrorRange, GameConfig.GetScreenHeight()/2-4 - ErrorRange/2, GameConfig.GetScreenWidth()/2-4 - ErrorRange, GameConfig.GetScreenHeight()/2+4 + ErrorRange/2, d3dg->GetColorCode(1.0f,0.0f,0.0f,alpha));
					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2+4 + ErrorRange, GameConfig.GetScreenHeight()/2-4 - ErrorRange/2, GameConfig.GetScreenWidth()/2+4 + ErrorRange, GameConfig.GetScreenHeight()/2+4 + ErrorRange/2, d3dg->GetColorCode(1.0f,0.0f,0.0f,alpha));
				}
				else{										//Standard type
					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2-13, GameConfig.GetScreenHeight()/2, GameConfig.GetScreenWidth()/2-3, GameConfig.GetScreenHeight()/2, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2+13, GameConfig.GetScreenHeight()/2, GameConfig.GetScreenWidth()/2+3, GameConfig.GetScreenHeight()/2, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2-13, GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2-3, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));
					d3dg->Draw2DLine(GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2+13, GameConfig.GetScreenWidth()/2, GameConfig.GetScreenHeight()/2+3, d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f));

					stru[0] = 0xBD;		stru[1] = '\0';	//"\xBD"
					d3dg->Draw2DTextureFontTextCenter(ErrorRange * -1, GameConfig.GetScreenHeight()/2 - 16, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32);
					stru[0] = 0xBE;		stru[1] = '\0';	//"\xBE"
					d3dg->Draw2DTextureFontTextCenter(ErrorRange, GameConfig.GetScreenHeight()/2 - 16, (char*)stru, d3dg->GetColorCode(1.0f,1.0f,1.0f,0.5f), 32, 32); 
				}
			}
		}
	}

	if( tag == true ){
		int color;
		if( ObjMgr.GetObjectInfoTag(camera_x, camera_y, camera_z, camera_rx, camera_ry, &color, str) == true ){
			d3dg->Draw2DTextureFontTextCenterScaling(1, 270 +1 , str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f), 14, 18);
			d3dg->Draw2DTextureFontTextCenterScaling(0, 270, str, color, 14, 18);
		}
	}

#ifdef ENABLE_DEBUGCONSOLE
	//Display AI debug information.
	if( AIdebuginfoID != -1 ){
		if( (0 <= AIdebuginfoID)&&(AIdebuginfoID < MAX_HUMAN) ){
			float posx, posy, posz, rx;
			int hp;
			char modestr[32];
			int EnemyID;
			int AIlevel;
			float mposx, mposz;
			int movemode;
			pointdata ppdata;
			ObjMgr.GetHumanObject(AIdebuginfoID)->GetPosData(&posx, &posy, &posz, &rx);
			hp = ObjMgr.GetHumanObject(AIdebuginfoID)->GetHP();
			HumanAI[AIdebuginfoID].GetBattleMode(NULL, modestr);
			EnemyID = HumanAI[AIdebuginfoID].GetEnemyHumanID();
			AIlevel = HumanAI[AIdebuginfoID].GetAIlevel();
			HumanAI[AIdebuginfoID].GetMoveTargetPos(&mposx, &mposz, &movemode);
			HumanAI[AIdebuginfoID].GetPathPointData(&ppdata);

			sprintf(str, "AI debug info [ID:%d]", AIdebuginfoID);
			d3dg->Draw2DTextureDebugFontText(20 +1, 130 +1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DTextureDebugFontText(20, 130, str, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
			sprintf(str, "(X:%.2f Y:%.2f Z:%.2f RX:%.2f HP:%d)", posx, posy, posz, rx, hp);
			d3dg->Draw2DTextureDebugFontText(20 +1, 150 +1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DTextureDebugFontText(20, 150, str, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
			sprintf(str, "Mode:%s  TargetEnemyID:%d  AIlevel:%d", modestr, EnemyID, AIlevel);
			d3dg->Draw2DTextureDebugFontText(20 +1, 170 +1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DTextureDebugFontText(20, 170, str, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
			sprintf(str, "PointPath:[%d][%d][%d][%d]", ppdata.p1, ppdata.p2, ppdata.p3, ppdata.p4);
			d3dg->Draw2DTextureDebugFontText(20 +1, 190 +1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DTextureDebugFontText(20, 190, str, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
			sprintf(str, "MovePosX:%.2f  MovePosZ:%.2f  (MoveMode:%d)", mposx, mposz, movemode);
			d3dg->Draw2DTextureDebugFontText(20 +1, 210 +1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DTextureDebugFontText(20, 210, str, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
		}
	}
#endif


	//-----------------------------------

	//Initialize Z buffer
	d3dg->ResetZbuffer();


	//HUD Display Mode A
	if( Camera_F2mode == 0 ){
		int notselectweapon = selectweapon + 1;
		if( notselectweapon == TOTAL_HAVEWEAPON ){ notselectweapon = 0; }

		//(3D drawing) Interim setting camera coordinates
		ScopeParameter sparam;
		GameParamInfo.GetScopeParam(0, &sparam);
		d3dg->SetCamera(camera_x, camera_y, camera_z, camera_rx, camera_ry, sparam.ViewAngle);

		//Render the carried primary weapon model in 3D.
		GameParamInfo.GetWeapon(weapon_paramid[selectweapon], &weapon_paramdata);
		Resource.GetWeaponModelTexture(weapon_paramid[selectweapon], &weaponmodel, &weapontexture);
		d3dg->SetWorldTransformPlayerWeapon(true, camera_rx, camera_ry, DegreeToRadian(framecnt*2), weapon_paramdata.size);
		d3dg->RenderModel(weaponmodel, weapontexture, false, true, nomodel);

		//Render the carried secondary weapon model in 3D.
		GameParamInfo.GetWeapon(weapon_paramid[notselectweapon], &weapon_paramdata);
		Resource.GetWeaponModelTexture(weapon_paramid[notselectweapon], &weaponmodel, &weapontexture);
		d3dg->SetWorldTransformPlayerWeapon(false, camera_rx, camera_ry, 0.0f, weapon_paramdata.size);
		d3dg->RenderModel(weaponmodel, weapontexture, false, true, nomodel);
	}

	//-----------------------------------

#ifdef ENABLE_DEBUGCONSOLE
	if( Show_Console == true ){
		if( ScreenShot > 0 ){
			ScreenShot += 1;
		}
		else{
			RenderConsole();
		}
	}
#endif

	//-----------------------------------


	//Blackout settings at start and end
	if( start_framecnt < (int)(2.0f*GAMEFPS) ){
		effect = GetEffectAlpha(start_framecnt, 1.0f, 2.0f, 0.0f, true);
	}
	else if( end_framecnt > 0 ){
		effect = GetEffectAlpha(end_framecnt, 1.0f, 3.0f + 0.5f, 0.0f, false);
	}
	else{
		effect = 0.0f;
	}
	d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,effect));

	//Character display at the end
	if( end_framecnt > 0 ){
		if( end_framecnt < (int)(1.0f*GAMEFPS) ){
			effect = GetEffectAlpha(end_framecnt, 1.0f, 1.0f, 0.0f, false);
		}
		else if( end_framecnt > (int)(3.0f*GAMEFPS) ){
			effect = GetEffectAlpha(end_framecnt, 1.0f, 1.0f, 3.0f, true);
		}
		else{
			effect = 1.0f;
		}

		if( GameInfoData.missioncomplete == true ){
			d3dg->Draw2DTextureFontTextCenterScaling(0, 240 + 10, "objective complete", d3dg->GetColorCode(1.0f,0.5f,0.0f,effect), 28, 32);
		}
		else{
			d3dg->Draw2DTextureFontTextCenterScaling(0, 240 + 10, "mission failure", d3dg->GetColorCode(1.0f,0.0f,0.0f,effect), 28, 32);
		}
	}


	time_render = GetTimeMS() - time;
}

//! @brief Converts to coordinates to draw on the command line
//! @param in_x Space X Coordinate
//! @param in_y Space Y Coordinate
//! @param in_z Space Z coordinates
//! @param RadarPosX X coordinate of the radar's upper-left corner.
//! @param RadarPosY Y coordinate of the radar's upper-left corner.
//! @param RadarSize drawing size
//! @param RadarWorldR Distance pointing to the left side
//! @param out_x Pointer to receive 2D X coordinates
//! @param out_y Pointer to receive 2D X coordinates
//! @param local_y Pointer receiving the local Y coordinate (NULL allowed).
//! @param check Default calculation
//! @return success: true failure: false
//! @attention If the check flag is enabled, the calculation will be d when it becomes clear that it is removed from the command line and return false.
bool maingame::GetRadarPos(float in_x, float in_y, float in_z, int RadarPosX, int RadarPosY, int RadarSize, float RadarWorldR, int *out_x, int *out_y, float *local_y, bool check)
{
	bool outf = false;
	float x, y, z, r, rx;
	float x2, z2, r2, rx2;

	//Calculate the distance to the camera.
	x = in_x - camera_x;
	y = in_y - camera_y;
	z = in_z - camera_z;

	//Processing if you are close
	if( (check == false) || ((fabsf(x) < RadarWorldR*2)&&(fabsf(z) < RadarWorldR*2)&&(fabsf(y) < 80.0f)) ){
		//Calculate the angle and distance.
		rx = atan2f(z, x);
		r = sqrtf(x*x + z*z);

		//Recalculate coordinates relative to the camera.
		rx2 = (rx - camera_rx)*-1 - (float)M_PI/2;
		x2 = cosf(rx2) * r;
		z2 = sinf(rx2) * r;

		//Determine whether to fit
		if( (check == false) || ((fabsf(x2) < RadarWorldR)&&(fabsf(z2) < RadarWorldR)) ){
			//Drawing Coordinates
			r2 = r / RadarWorldR * (RadarSize/2);
			*out_x = (int)(RadarPosX+RadarSize/2 + cosf(rx2) * r2);
			*out_y = (int)(RadarPosY+RadarSize/2 + sinf(rx2) * r2);
			if( local_y != NULL ){ *local_y = y; }
			outf = true;
		}
	}

	return outf;
}

//! @brief Simple touch panel display
void maingame::RenderRadar()
{
	int RadarSize = 200;							//drawing size
	int RadarPosX = GameConfig.GetScreenWidth() - RadarSize - 10;	//X coordinate of the radar's upper-left corner.
	int RadarPosY = 110;							//Y coordinate of the radar's upper-left corner.
	float RadarWorldR = 300.0f;						//Distance pointing to the left side

	float ecr = DISTANCE_CHECKPOINT / RadarWorldR * (RadarSize/2);

	//Background and border.
	d3dg->Draw2DBox(RadarPosX, RadarPosY, RadarPosX+RadarSize, RadarPosY+RadarSize, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.6f));
	d3dg->Draw2DLine(RadarPosX, RadarPosY, RadarPosX+RadarSize, RadarPosY, d3dg->GetColorCode(0.0f,0.8f,0.0f,1.0f));
	d3dg->Draw2DLine(RadarPosX+RadarSize, RadarPosY, RadarPosX+RadarSize, RadarPosY+RadarSize, d3dg->GetColorCode(0.0f,0.8f,0.0f,1.0f));
	d3dg->Draw2DLine(RadarPosX+RadarSize, RadarPosY+RadarSize, RadarPosX, RadarPosY+RadarSize, d3dg->GetColorCode(0.0f,0.8f,0.0f,1.0f));
	d3dg->Draw2DLine(RadarPosX, RadarPosY+RadarSize, RadarPosX, RadarPosY, d3dg->GetColorCode(0.0f,0.8f,0.0f,1.0f));

	//Render the map.
	int bs = BlockData.GetTotaldatas();
	for(int i=0; i< bs; i++){
		blockdata bdata;
		int vid[4];
		int bvx[4], bvy[4];

		//Get block data
		BlockData.Getdata(&bdata, i);

		//Detection of block of drawing candidates (coarse cutting)
		if( CollD.CheckBlockAABB(i, camera_x-RadarWorldR*2, camera_y-1.0f, camera_z-RadarWorldR*2, camera_x+RadarWorldR*2, camera_y+1.0f, camera_z+RadarWorldR*2) == true ){

			//Processing for each face
			for(int j=0; j<6; j++){
				//Check for an unclimbable slope, excluding inclines such as ground and stairs.
				float angle = acosf(bdata.material[j].vy);
				if( (HUMAN_MAPCOLLISION_SLOPEANGLE < angle)&&(angle < DegreeToRadian(120)) ){

					//es a block vertex data association
					blockdataface(j, &(vid[0]), NULL);
					
					//Calculate the four vertices.
					for(int k=0; k<4; k++){
						GetRadarPos(bdata.x[ vid[k] ], bdata.y[ vid[k] ], bdata.z[ vid[k] ], RadarPosX, RadarPosY, RadarSize, RadarWorldR, &(bvx[k]), &(bvy[k]), NULL, false);
					}

					int line_x1, line_y1, line_x2, line_y2;

					//Drawing to fit in the square of the curve
					if( Get2DLineInBox(bvx[0], bvy[0], bvx[1], bvy[1], RadarPosX, RadarPosY, RadarPosX+RadarSize, RadarPosY+RadarSize, &line_x1, &line_y1, &line_x2, &line_y2) == true ){
						d3dg->Draw2DLine(line_x1, line_y1, line_x2, line_y2, d3dg->GetColorCode(0.8f,0.8f,0.8f,1.0f));
					}
					if( Get2DLineInBox(bvx[1], bvy[1], bvx[2], bvy[2], RadarPosX, RadarPosY, RadarPosX+RadarSize, RadarPosY+RadarSize, &line_x1, &line_y1, &line_x2, &line_y2) == true ){
						d3dg->Draw2DLine(line_x1, line_y1, line_x2, line_y2, d3dg->GetColorCode(0.8f,0.8f,0.8f,1.0f));
					}
					if( Get2DLineInBox(bvx[2], bvy[2], bvx[3], bvy[3], RadarPosX, RadarPosY, RadarPosX+RadarSize, RadarPosY+RadarSize, &line_x1, &line_y1, &line_x2, &line_y2) == true ){
						d3dg->Draw2DLine(line_x1, line_y1, line_x2, line_y2, d3dg->GetColorCode(0.8f,0.8f,0.8f,1.0f));
					}
					if( Get2DLineInBox(bvx[3], bvy[3], bvx[0], bvy[0], RadarPosX, RadarPosY, RadarPosX+RadarSize, RadarPosY+RadarSize, &line_x1, &line_y1, &line_x2, &line_y2) == true ){
						d3dg->Draw2DLine(line_x1, line_y1, line_x2, line_y2, d3dg->GetColorCode(0.8f,0.8f,0.8f,1.0f));
					}
				}
			}
		}
	}

	//Show event arrival points
	if( EventStop == false ){
		for(int i=0; i<TOTAL_EVENTLINE; i++){
			signed short int p4 = Event[i].GetNextP4();
			pointdata data;
			if( PointData.SearchPointdata(&data, 0x08, 0, 0, 0, p4, 0) != 0 ){
				float y;
				int x_2d, y_2d;
				float alpha;

				if( (data.p1 == 13)||(data.p1 == 16) ){
					data.y += VIEW_HEIGHT;

					if( GetRadarPos(data.x, data.y, data.z, RadarPosX, RadarPosY, RadarSize, RadarWorldR, &x_2d, &y_2d, &y, true) == true ){
						//Transparency by printing
						if( (fabsf(y) < 40.0f) ){
							alpha = 1.0f;
						}
						else{
							alpha = 0.5f;
						}

						//Marker Drawing
						d3dg->Draw2DCycle(x_2d, y_2d, (int)ecr, d3dg->GetColorCode(1.0f,0.5f,0.0f,alpha));
					}
				}
			}
		}
	}

	//Get Player Information
	int PlayerID = ObjMgr.GetPlayerID();
	int myteamid;
	ObjMgr.GetHumanObject(PlayerID)->GetParamData(NULL, NULL, NULL, &myteamid);

	//Draw a person
	for(int i=0; i<MAX_HUMAN; i++){
		human* thuman;
		float tx, ty, tz;
		int tteamid;
		float y;
		int x_2d, y_2d;
		float alpha;
		int color;

		//Get the character object.
		thuman = ObjMgr.GetHumanObject(i);

		//Ignore unused characters and corpses.
		if( thuman->GetEnableFlag() == false ){ continue; }
		if( thuman->GetDeadFlag() == true ){ continue; }

		//Get information about a person
		thuman->GetPosData(&tx, &ty, &tz, NULL);
		thuman->GetParamData(NULL, NULL, NULL, &tteamid);
		ty += VIEW_HEIGHT;

		if( GetRadarPos(tx, ty, tz, RadarPosX, RadarPosY, RadarSize, RadarWorldR, &x_2d, &y_2d, &y, true) == true ){
			//Transparency by printing
			if( (fabsf(y) < 40.0f) ){
				alpha = 1.0f;
			}
			else{
				alpha = 0.5f;
			}

			//Determines the color of the marker
			if( PlayerID == i ){ color = d3dg->GetColorCode(1.0f,1.0f,0.0f,alpha); }				//Players themselves
			else if( tteamid == myteamid ){ color = d3dg->GetColorCode(0.0f,0.5f,1.0f,alpha); }	//Ally.
			else{ color = d3dg->GetColorCode(1.0f,0.0f,0.5f,alpha); }								//Enemy.

			//Marker Drawing
			d3dg->Draw2DBox(x_2d-3, y_2d-3, x_2d+3, y_2d+3, color);
		}
	}
}

#ifdef ENABLE_DEBUGCONSOLE

//! @brief Add a new string to the debug console.
//! @param color Character Color
//! @param str Adding a string pointer
//! @attention New strings are always added from the bottom.
//! @attention If MAX_CONSOLELINES is exceeded, remove the first row, shift the remaining rows up, and append the new row at the bottom.
void maingame::AddInfoConsole(int color, const char *str)
{
	for(int i=0; i<MAX_CONSOLELINES; i++){
		if( InfoConsoleData[i].textdata[0] == NULL ){
			InfoConsoleData[i].color = color;
			strcpy(InfoConsoleData[i].textdata, str);
			return;
		}
	}

	for(int i=1; i<MAX_CONSOLELINES; i++){
		memcpy(&(InfoConsoleData[i-1]), &(InfoConsoleData[i]), sizeof(ConsoleData));
	}
	InfoConsoleData[MAX_CONSOLELINES-1].color = color;
	strcpy(InfoConsoleData[MAX_CONSOLELINES-1].textdata, str);
}

//! @brief Determine commands and get arguments
//! @param cmd Pointer of the command character to determine
//! @param num Pointer to receive given arguments
//! @return Retrieved: true; outside the test range: false.
bool maingame::GetCommandNum(const char *cmd, int *num)
{
	if( cmd == NULL ){ return false; }
	if( num == NULL ){ return false; }

	//Check the command name
	if( memcmp(NewCommand, cmd, strlen(cmd)) != 0 ){ return false; }
	if( NewCommand[strlen(cmd)] != ' ' ){ return false; }

	// the number of characters in "Command Name_X" is reached
	if( strlen(cmd)+2 > strlen(NewCommand) ){ return false; }

	//Is the number given?
	for(int i=(int)strlen(cmd)+1; NewCommand[i] != '\0'; i++){
		if( ((NewCommand[i] < '0')||('9' < NewCommand[i]))&&(NewCommand[i] != '+')&&(NewCommand[i] != '-') ){ return false; }
	}

	//Find a given number
	*num = atoi(&(NewCommand[ strlen(cmd)+1 ]));
	return true;
}

//! @brief Process debug-console input.
void maingame::InputConsole()
{
	char str[MAX_CONSOLELEN];
	NewCommand[0] = '\0';

	if( ConsoleTextBoxControl.ProcessTextBox(inputCtrl) == true ){
		//[ENTER]
		//Add to the console and register as unprocessed command
		strcpy(str, CONSOLE_PROMPT);
		strcat(str, ConsoleTextBoxControl.GetTextBoxStr());
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		strcpy(NewCommand, ConsoleTextBoxControl.GetTextBoxStr());

		ConsoleTextBoxControl.SetTextBoxStr("");
	}
}

//! @brief Main processing for the debug console.
void maingame::ProcessConsole()
{
	char str[MAX_CONSOLELEN];
	int id;

	//Command List
	if( strcmp(NewCommand, "help") == 0 ){
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "help          human        result          event            skip <NUM>");
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "config        mif          bd1    pd1      resinfo          info");
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "view          center       map    model    aiinfo <NUM>     tag");
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "radar         inmap        sky <NUM>       dark     fog     teleport <NUM>");
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "player <NUM>  revive       kill <NUM>      treat <NUM>      nodamage <NUM>");
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "newobj <NUM>  break <NUM>  delhuman <NUM>  delweapon <NUM>  delobj <NUM>");
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "nofight       caution      ailevel <NUM>   ff       bot     stop      estop");
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "speed         window       ss              clear            ver       exit");
	}

	//Human Statistics
	if( strcmp(NewCommand, "human") == 0 ){
		int alivemyfriend = 0;
		int myfriend = 0;
		int aliveenemy = 0;
		int enemy = 0;
		int myteamid;

		//Get player's team number
		ObjMgr.GetPlayerHumanObject()->GetParamData(NULL, NULL, NULL, &myteamid);

		for(int i=0; i<MAX_HUMAN; i++){
			int teamid;
			bool deadflag;
			human *thuman = ObjMgr.GetHumanObject(i);
			if( thuman->GetEnableFlag() == true ){
				//Get death status and team number
				deadflag = thuman->GetDeadFlag();
				thuman->GetParamData(NULL, NULL, NULL, &teamid);

				//Counting
				if( teamid == myteamid ){
					myfriend += 1;
					if( deadflag == false ){ alivemyfriend += 1; }
				}
				else{
					enemy += 1;
					if( deadflag == false ){ aliveenemy += 1; }
				}
			}
		}

		sprintf(str, "Friend:%d/%d  Enemy:%d/%d  Total:%d/%d",
			alivemyfriend, myfriend, aliveenemy, enemy, alivemyfriend + aliveenemy, myfriend + enemy);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
	}

	//Display provisional results.
	if( strcmp(NewCommand, "result") == 0 ){
		int intontarget;
		float rate;

		//Lifetime rate calculation
		intontarget = (int)floor(MainGameInfo.ontarget);
		if( MainGameInfo.fire == 0 ){
			rate = 0.0f;
		}
		else{
			rate = (float)intontarget / MainGameInfo.fire * 100;
		}

		sprintf(str, "Time %02d:%02d  /  Fired %d  /  On target %d(%0.2f)", framecnt/(int)GAMEFPS/60, framecnt/(int)GAMEFPS%60, MainGameInfo.fire, intontarget, MainGameInfo.ontarget);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		sprintf(str, "AR rate %.1f%%  /  Kill %d  /  HS %d", rate, MainGameInfo.kill, MainGameInfo.headshot);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
	}

	//Event task display
	if( strcmp(NewCommand, "event") == 0 ){
		for(int i=0; i<TOTAL_EVENTLINE; i++){
			signed short int p4 = Event[i].GetNextP4();
			pointdata data;
			if( PointData.SearchPointdata(&data, 0x08, 0, 0, 0, p4, 0) == 0 ){
				sprintf(str, "Event %d   No task.", i);
			}
			else{
				sprintf(str, "Event %d   [%d][%d][%d][%d]", i, data.p1, data.p2, data.p3, data.p4);
			}
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}
	}

	//Event task skip
	if( GetCommandNum("skip", &id) == true ){
		if( (0 <= id)&&(id < TOTAL_EVENTLINE) ){
			signed short int p4 = Event[id].GetNextP4();
			pointdata data;
			if( PointData.SearchPointdata(&data, 0x08, 0, 0, 0, p4, 0) == 0 ){
				sprintf(str, "Event %d   No task.", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
			else{
				EventSkip[id] = true;

				sprintf(str, "Skipped event [%d].", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
				sprintf(str, " ([%d][%d][%d][%d])", data.p1, data.p2, data.p3, data.p4);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}

	//config information
	if( strcmp(NewCommand, "config") == 0 ){
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "[Basic Config]");

		//Key code display
		char str2[8];
		char str3[MAX_CONSOLELEN];
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), " Keycode : ");
		for(int i=0; i*10<TOTAL_ControlKey; i++){
			strcpy(str, " ");
			for(int j=0; j<10; j++){
				if( i*10+j >= TOTAL_ControlKey ){ break; }
				sprintf(str2, " 0x%02X", GameConfig.GetKeycode(i*10+j));
				strcat(str, str2);
			}
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}

		strcpy(str, " ");

		//Mouse Sensitivity
		sprintf(str3, "MouseSensitivity : %2d           ", GameConfig.GetMouseSensitivity());
		strcat(str, str3);

		//Screen display mode
		if( GameConfig.GetFullscreenFlag() == false ){
			strcat(str, "FullscreenFlag : false (window)");
		}
		else{
			strcat(str, "FullscreenFlag : true (fullscreen)");
		}

		//Display.
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		strcpy(str, " ");

		//Sound setting
		if( GameConfig.GetSoundFlag() == false ){
			strcat(str, "SoundFlag : false (OFF)         ");
		}
		else{
			strcat(str, "SoundFlag : true (ON)           ");
		}

		//Bleeding Settings
		if( GameConfig.GetBloodFlag() == false ){
			strcat(str, "BloodFlag : false (OFF)");
		}
		else{
			strcat(str, "BloodFlag : true (ON)");
		}

		//Display.
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		strcpy(str, " ");

		//Screen brightness setting
		sprintf(str3, "Brightness : %2d                 ", GameConfig.GetBrightness());
		strcat(str, str3);

		//Mouse Inverting Settings
		if( GameConfig.GetInvertMouseFlag() == false ){
			strcat(str, "InvertMouseFlag : false (OFF)");
		}
		else{
			strcat(str, "InvertMouseFlag : true (ON)");
		}

		//Display.
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		strcpy(str, " ");

		//Frame skip setting
		if( GameConfig.GetFrameskipFlag() == false ){
			strcat(str, "FrameskipFlag : false (OFF)     ");
		}
		else{
			strcat(str, "FrameskipFlag : true (ON)       ");
		}

		//Setting to use another tertiary
		if( GameConfig.GetAnotherGunsightFlag() == false ){
			strcat(str, "AnotherGunsightFlag : false (OFF)");
		}
		else{
			strcat(str, "AnotherGunsightFlag : true (ON)");
		}

		//Display.
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		strcpy(str, " ");

		//Player Name Display
		char namestr[MAX_PLAYERNAME+1];
		GameConfig.GetPlayerName(namestr);
		strcat(str, "PlayerName : ");
		strcat(str, namestr);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "[Add Config]");

		strcpy(str, " ");

		//Language Settings
		sprintf(str3, "Language : %d     ", GameConfig.GetLanguage());
		strcat(str, str3);

		//Screen resolution
		sprintf(str3, "ScreenSize : %dx%d     ", GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight());
		strcat(str, str3);

		//Volume setting
		sprintf(str3, "MasterVolume : %d%%", (int)(GameConfig.GetVolume()*100));
		strcat(str, str3);

		//Display.
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
	}

	//MIF Information Display
	if( strcmp(NewCommand, "mif") == 0 ){
		bool AddonFlag;
		int MissionID = GameInfoData.selectmission_id;
		char str2[MAX_PATH];
		char str3[MAX_PATH];
		bool collisionflag, screenflag;

		if( GameInfoData.selectmission_id >= 0 ){ AddonFlag = false; }
		else{ AddonFlag = true; }

		//Header
		if( AddonFlag == true ){ sprintf(str, "[Addon Mission]   (MissionID:%d)", MissionID); }
		else{ sprintf(str, "[Standard Mission]   (MissionID:%d)", MissionID); }
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//MIF file path
		strcpy(str, "MIFpath : ");
		strcpy(str2, GameInfoData.mifpath);
		str2[(MAX_CONSOLELEN - strlen(str) - 1)] = '\0';
		strcat(str, str2);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Mission identification name
		strcpy(str, "Name : ");
		if( AddonFlag == true ){ strcpy(str2, MIFdata.GetMissionName()); }
		else{ GameParamInfo.GetOfficialMission(MissionID, str2, NULL, NULL, NULL, NULL, NULL); }
		str2[(MAX_CONSOLELEN - strlen(str) - 1)] = '\0';
		strcat(str, str2);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Mission official name
		strcpy(str, "FullName : ");
		if( AddonFlag == true ){ strcpy(str2, MIFdata.GetMissionFullname()); }
		else{ GameParamInfo.GetOfficialMission(MissionID, NULL, str2, NULL, NULL, NULL, NULL); }
		str2[(MAX_CONSOLELEN - strlen(str) - 1)] = '\0';
		strcat(str, str2);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Block Data File
		strcpy(str, "BD1file : ");
		if( AddonFlag == true ){
			MIFdata.GetDatafilePath(str2, str3);
			str2[(MAX_CONSOLELEN - strlen(str) - 1)] = '\0';
			strcat(str, str2);
		}
		else{
			GameParamInfo.GetOfficialMission(MissionID, NULL, NULL, str2, NULL, NULL, NULL);
			str2[(MAX_CONSOLELEN - strlen(str) - 8 - 1)] = '\0';
			strcat(str, str2);
			strcat(str, OFFICIALMISSION_BD1);
		}
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Point Data File
		strcpy(str, "PD1file : ");
		if( AddonFlag == true ){
			MIFdata.GetDatafilePath(str3, str2);
			str2[(MAX_CONSOLELEN - strlen(str) - 1)] = '\0';
			strcat(str, str2);
		}
		else{
			GameParamInfo.GetOfficialMission(MissionID, NULL, NULL, str2, str3, NULL, NULL);
			strcat(str2, str3);
			str2[(MAX_CONSOLELEN - strlen(str) - 4 - 1)] = '\0';
			strcat(str, str2);
			strcat(str, ".pd1");
		}
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Additional accessories information file
		strcpy(str, "AddOBJfile : ");
		strcpy(str2, MIFdata.GetAddSmallobjectFile());
		str2[(MAX_CONSOLELEN - strlen(str) - 1)] = '\0';
		strcat(str, str2);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Set values and Flags
		if( AddonFlag == true ){
			collisionflag = MIFdata.GetCollisionFlag();
			screenflag = MIFdata.GetScreenFlag();
		}
		else{
			GameParamInfo.GetOfficialMission(MissionID, NULL, NULL, NULL, NULL, &collisionflag, &screenflag);
		}
		sprintf(str, "Sky:%d    CollisionFlag:%d    DarkScreenFlag:%d", MIFdata.GetSkynumber(), (int)collisionflag, (int)screenflag);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
	}

	//Block Data Information
	if( strcmp(NewCommand, "bd1") == 0 ){
		bool AddonFlag;
		int MissionID = GameInfoData.selectmission_id;
		char str2[MAX_PATH];
		char str3[MAX_PATH];
		char fname1[_MAX_PATH];
		char fname2[_MAX_PATH];
		char flagstr1[4];
		char flagstr2[4];

		if( GameInfoData.selectmission_id >= 0 ){ AddonFlag = false; }
		else{ AddonFlag = true; }

		//File Name Display
		strcpy(str, "Filename : ");
		if( AddonFlag == true ){
			MIFdata.GetDatafilePath(str2, str3);
			str2[(MAX_CONSOLELEN - strlen(str) - 1)] = '\0';
			strcat(str, str2);
		}
		else{
			GameParamInfo.GetOfficialMission(MissionID, NULL, NULL, str2, NULL, NULL, NULL);
			str2[(MAX_CONSOLELEN - strlen(str) - 8 - 1)] = '\0';
			strcat(str, str2); 
			strcat(str, OFFICIALMISSION_BD1);
		}
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Total number of blocks
		sprintf(str, "TotalBlocks : %d", BlockData.GetTotaldatas());
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Print texture reading status display
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Texture : ");
		for(int i=0; i<(TOTAL_BLOCKTEXTURE/2); i++){
			//Get Texture ID
			BlockData.GetTexture(fname1, i);
			BlockData.GetTexture(fname2, i + (TOTAL_BLOCKTEXTURE/2));
			
			//Determines whether the texture is loaded
			if( d3dg->GetMapTextureID(i) == -1 ){ strcpy(flagstr1, "NG"); }
			else{ strcpy(flagstr1, "OK"); }
			if( d3dg->GetMapTextureID(i + (TOTAL_BLOCKTEXTURE/2)) == -1 ){ strcpy(flagstr2, "NG"); }
			else{ strcpy(flagstr2, "OK"); }

			//Display.
			sprintf(str, " %02d_%s %-31s %02d_%s %s", i, flagstr1, fname1, i + (TOTAL_BLOCKTEXTURE/2), flagstr2, fname2);
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}
	}

	//Point Data Information
	if( strcmp(NewCommand, "pd1") == 0 ){
		bool AddonFlag;
		int MissionID = GameInfoData.selectmission_id;
		char str2[MAX_PATH];
		char str3[MAX_PATH];
		pointdata pdata;
		int TotalPoints = PointData.GetTotaldatas();
		int HumanPoints = 0;
		int WeaponPoints = 0;
		int OjectPoints = 0;
		int HumaninfoPoints = 0;
		int PathPoints = 0;
		int EventPoints = 0;
		int info_paramid1 = 0;
		int info_paramid2 = 0;
		int info_textureid1 = 0;
		int info_textureid2 = 0;
		char fname1[_MAX_PATH];
		char fname2[_MAX_PATH];
		char flagstr1[4];
		char flagstr2[4];

		if( GameInfoData.selectmission_id >= 0 ){ AddonFlag = false; }
		else{ AddonFlag = true; }

		//File Name Display
		strcpy(str, "Filename : ");
		if( AddonFlag == true ){
			MIFdata.GetDatafilePath(str3, str2);
			str2[(MAX_CONSOLELEN - strlen(str) - 1)] = '\0';
			strcat(str, str2);
		}
		else{
			GameParamInfo.GetOfficialMission(MissionID, NULL, NULL, str2, str3, NULL, NULL);
			strcat(str2, str3);
			str2[(MAX_CONSOLELEN - strlen(str) - 4 - 1)] = '\0';
			strcat(str, str2);
			strcat(str, ".pd1");
		}
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Total number of points
		sprintf(str, "TotalPoints : %d", TotalPoints);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Display by counting various points
		for(int i=0; i<TotalPoints; i++){
			if( PointData.Getdata(&pdata, i) != 0 ){ continue; }

			if( (pdata.p1 == POINT_P1TYPE_HUMAN)||(pdata.p1 == POINT_P1TYPE_HUMAN2) ){ HumanPoints += 1; }
			if( (pdata.p1 == POINT_P1TYPE_WEAPON)||(pdata.p1 == POINT_P1TYPE_RAND_WEAPON) ){ WeaponPoints += 1; }
			if( pdata.p1 == POINT_P1TYPE_SMALLOBJ ){ OjectPoints += 1; }
			if( pdata.p1 == POINT_P1TYPE_HUMANINFO ){ HumaninfoPoints += 1; }
			if( (pdata.p1 == POINT_P1TYPE_AIPATH)||(pdata.p1 == POINT_P1TYPE_RAND_AIPATH) ){ PathPoints += 1; }
			if( (10 <= pdata.p1)&&(pdata.p1 <= 19) ){ EventPoints += 1; }
		}
		sprintf(str, "HumanPoints : %-3d       WeaponPoints : %-3d    SmallOjectPoints : %-3d", HumanPoints, WeaponPoints, OjectPoints);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		sprintf(str, "HumaninfoPoints : %-3d   AIpathPoints : %-3d    EventPoints : %-3d", HumaninfoPoints, PathPoints, EventPoints);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Determine the number of invalid points in reverse
		sprintf(str, "InvalidPoints : %d", TotalPoints - (HumanPoints + WeaponPoints + OjectPoints + HumaninfoPoints + PathPoints + EventPoints));
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Get the number of textures of people
		sprintf(str, "HumanTextures : %d/%d", Resource.GetHumanTextures(), MAX_LOADHUMANTEXTURE);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Read status of human information texture
		for(int i=0; i<(MAX_LOADHUMANTEXTURE/2); i++){
			//Get Texture ID
			Resource.GetHumanTextureInfo(i, &info_paramid1, &info_textureid1);
			Resource.GetHumanTextureInfo(i + (MAX_LOADHUMANTEXTURE/2), &info_paramid2, &info_textureid2);

			if( GameParamInfo.GetHumanTexturePath(info_paramid1, fname1) == 1 ){ strcpy(fname1, ""); }
			if( GameParamInfo.GetHumanTexturePath(info_paramid2, fname2) == 1 ){ strcpy(fname2, ""); }

			//Determines whether the texture is loaded
			if( info_textureid1 == -1 ){ strcpy(flagstr1, "NG"); }
			else{ strcpy(flagstr1, "OK"); }
			if( info_textureid2 == -1 ){ strcpy(flagstr2, "NG"); }
			else{ strcpy(flagstr2, "OK"); }

			//Display.
			sprintf(str, " %02d_%s %-31s %02d_%s %s", i, flagstr1, fname1, i + (MAX_LOADHUMANTEXTURE/2), flagstr2, fname2);
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}
	}

	//Resource information display
	if( strcmp(NewCommand, "resinfo") == 0 ){
		int human = 0;
		int weapon = 0;
		int smallobject = 0;
		int bullet = 0;
		int grenade = 0;
		int effect = 0;

		//Graphics
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "[Graphics]");
		sprintf(str, " model : %2d/%2d        texture : %2d/%2d", d3dg->GetTotalModels(), MAX_MODEL, d3dg->GetTotalTextures(), MAX_TEXTURE);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Sound
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "[Sound]");
		sprintf(str, " sound : %2d/%2d", SoundCtrl.GetTotalSounds(), MAX_LOADSOUND);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Data File
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "[Datafile]");
		sprintf(str, " blocks : %3d/%3d     points : %3d/%3d", BlockData.GetTotaldatas(), MAX_BLOCKS, PointData.GetTotaldatas(), MAX_POINTS);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);

		//Number of objects
		ObjMgr.GetTotalObjects(&human, &weapon, &smallobject, &bullet, &grenade, &effect);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "[Object]");
		sprintf(str, " human : %3d/%3d      weapon : %3d/%3d      smallobject : %2d/%2d", human, MAX_HUMAN, weapon, MAX_WEAPON, smallobject, MAX_SMALLOBJECT);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		sprintf(str, " bullet : %3d/%3d     grenade : %3d/%3d     effect : %3d/%3d", bullet, MAX_BULLET, grenade, MAX_GRENADE, effect, MAX_EFFECT);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		sprintf(str, " sound : %3d/%3d", GameSound->GetTotalSoundList(), MAX_SOUNDMGR_LIST);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
	}

	//Display debug text.
	if( strcmp(NewCommand, "info") == 0 ){
		if( ShowInfo_Debugmode == false ){
			ShowInfo_Debugmode = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Show Debug information.");
		}
		else{
			ShowInfo_Debugmode = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Hide Debug information.");
		}
	}

	//Free Camera Switching
	if( strcmp(NewCommand, "view") == 0 ){
		if( Camera_Debugmode == false ){
			Camera_Debugmode = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Change FreeView mode.");
		}
		else{
			Camera_Debugmode = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Change PlayerView mode.");
		}

		//Camera settings when debug mode starts.
		if( Camera_Debugmode == true ){
			camera_x = 100.0f;
			camera_y = 100.0f;
			camera_z = -100.0f;
			camera_rx = DegreeToRadian(135);
			camera_ry = DegreeToRadian(-40);
		}
	}

	//3D center line display
	if( strcmp(NewCommand, "center") == 0 ){
		if( CenterLine == false ){
			CenterLine = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Show World center position.");
		}
		else{
			CenterLine = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Hide World center position.");
		}
	}

	//Wire frame printing
	if( strcmp(NewCommand, "map") == 0 ){
		if( wireframe == false ){
			wireframe = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Draw map on the Wire frame.");
		}
		else{
			wireframe = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Draw map on the Normal.");
		}
	}

	//Switching model drawing flag
	if( strcmp(NewCommand, "model") == 0 ){
		if( nomodel == false ){
			nomodel = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Disable render model.");
		}
		else{
			nomodel = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Enable render model.");
		}
	}

	//AI Information Display
	if( GetCommandNum("aiinfo", &id) == true ){
		if( (0 <= id)&&(id < MAX_HUMAN) ){
			if( AIdebuginfoID == id ){
				//Disable if the same number is specified
				AIdebuginfoID = -1;
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Disable AI Debug information.");
			}
			else{
				//New People
				AIdebuginfoID = id;
				sprintf(str, "Enable AI Debug information. Human[%d].", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}
	else if( strcmp(NewCommand, "aiinfo") == 0 ){
		if( AIdebuginfoID != -1 ){
			//Invalidation if a person is specified
			AIdebuginfoID = -1;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Disable AI Debug information.");
		}
	}

	//Show tags for objects
	if( strcmp(NewCommand, "tag") == 0 ){
		if( tag == false ){
			tag = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Show object information.");
		}
		else{
			tag = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Hide object information.");
		}
	}

	//Draw
	if( strcmp(NewCommand, "radar") == 0 ){
		if( radar == false ){
			radar = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Show Radar.");
		}
		else{
			radar = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Hide Radar.");
		}
	}

	//Draw Blindfold
	if( strcmp(NewCommand, "inmap") == 0 ){
		if( Camera_Blind == false ){
			Camera_Blind = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Blindfold in map.");
		}
		else{
			Camera_Blind = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Not blindfold in map.");
		}
	}

	//Change background empty
	if( GetCommandNum("sky", &id) == true ){
		if( (0 <= id)&&(id <= 5) ){
			SkyNumber = id;

			//Resource Reconstruction
			Resource.CleanupSkyModelTexture();
			Resource.LoadSkyModelTexture(SkyNumber);

			sprintf(str, "Select SkyNumber %d.", id);
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}
	}

	//Dark screen
	if( strcmp(NewCommand, "dark") == 0 ){
		char path[MAX_PATH];
		char bdata[MAX_PATH];
		char pdata[MAX_PATH];

		//Switching flags
		if( DarkScreenFlag == false ){
			DarkScreenFlag = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Enable DarkScreen Flag.");
		}
		else{
			DarkScreenFlag = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Disable DarkScreen Flag.");
		}

		//.bd1 file path
		if( MIFdata.GetFiletype() == false ){
			GameParamInfo.GetOfficialMission(MainGameInfo.selectmission_id, NULL, NULL, path, NULL, NULL, NULL);
		}
		else{
			MIFdata.GetDatafilePath(bdata, pdata);
			GetFileDirectory(bdata, path);
		}

		//Block data initialization
		BlockData.CalculationBlockdata(DarkScreenFlag);
		d3dg->CleanupMapdata();
		d3dg->LoadMapdata(&BlockData, path);
	}

	//Fog Switching
	if( strcmp(NewCommand, "fog") == 0 ){
		//Switching flags
		if( FogFlag == false ){
			FogFlag = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Enable Fog Flag.");
		}
		else{
			FogFlag = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Disable Fog Flag.");
		}
	}

	//Player Move
	if( GetCommandNum("teleport", &id) == true ){
		if( (0 <= id)&&(id < MAX_HUMAN)&&(id != ObjMgr.GetPlayerID()) ){
			float x, y, z, rx;
			human *thuman = ObjMgr.GetHumanObject(id);

			//Get the target coordinates.
			thuman->GetPosData(&x, &y, &z, NULL);

			//Player Move
			ObjMgr.GetPlayerHumanObject()->GetPosData(NULL, NULL ,NULL, &rx);
			ObjMgr.GetPlayerHumanObject()->SetPosData(x + 5.0f, y + 5.0f, z + 5.0f, rx);

			sprintf(str, "Teleported player to Human[%d].", id);
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}
	}

	//Player Switching
	if( GetCommandNum("player", &id) == true ){
		if( (0 <= id)&&(id < MAX_HUMAN)&&(id != ObjMgr.GetPlayerID()) ){
			//Apply the Player Number
			ObjMgr.SetPlayerID(id);

			//Get player orientation
			ObjMgr.GetPlayerHumanObject()->GetRxRy(&mouse_rx, &mouse_ry);

			//Reset camera orientation when F1 mode
			if( Camera_F1mode == true ){
				camera_rx = DegreeToRadian(90);
			}

			sprintf(str, "Changed player to Human[%d].", id);
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}
	}

	//Reviving All Deaths
	if( strcmp(NewCommand, "revive") == 0 ){
		for(int i=0; i<MAX_HUMAN; i++){
			ObjMgr.HumanResuscitation(i);
		}
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "All human has resuscitation.");
	}

	//Recover
	if( GetCommandNum("treat", &id) == true ){
		if( (0 <= id)&&(id < MAX_HUMAN) ){
			int param, hp;
			HumanParameter data;
			human *thuman = ObjMgr.GetHumanObject(id);
			
			//Get initial HP
			thuman->GetParamData(&param, NULL, NULL, NULL);
			GameParamInfo.GetHuman(param, &data);
			hp = data.hp;

			//Recover
			if( thuman->SetHP(hp) == true ){
				sprintf(str, "Set the HP:%d to Human[%d].", data.hp, id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}

	//Unbeatable
	if( GetCommandNum("nodamage", &id) == true ){
		if( (0 <= id)&&(id < MAX_HUMAN) ){
			if( InvincibleID == id ){
				//Disable if the same number is specified
				InvincibleID = -1;
				ObjMgr.GetHumanObject(id)->SetInvincibleFlag(false);
				sprintf(str, "Not invincible Human[%d].", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
			else{
				//If someone is already specified, it will be disabled.
				if( InvincibleID != -1 ){
					ObjMgr.GetHumanObject(InvincibleID)->SetInvincibleFlag(false);
					sprintf(str, "Not invincible Human[%d].", InvincibleID);
					AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
				}

				//A new invincible person
				InvincibleID = id;
				ObjMgr.GetHumanObject(id)->SetInvincibleFlag(true);
				sprintf(str, "Invincible Human[%d].", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}
	else if( strcmp(NewCommand, "nodamage") == 0 ){
		if( InvincibleID != -1 ){
			//Invalidation if a person is specified
			ObjMgr.GetHumanObject(InvincibleID)->SetInvincibleFlag(false);
			sprintf(str, "Not invincible Human[%d].", InvincibleID);
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			InvincibleID = -1;
		}
	}

	//Kill.
	if( GetCommandNum("kill", &id) == true ){
		if( (0 <= id)&&(id < MAX_HUMAN) ){
			human *thuman = ObjMgr.GetHumanObject(id);
			if( thuman->GetEnableFlag() == true ){
				if( thuman->SetHP(0) == true ){
					sprintf(str, "Killed Human[%d].", id);
					AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
				}
			}
		}
	}

	//Newly placed accessories
	if( GetCommandNum("newobj", &id) == true ){
		if( (0 <= id)&&(id < TOTAL_PARAMETERINFO_SMALLOBJECT) ){
			int dataid = ObjMgr.AddSmallObjectIndex(camera_x + cosf(camera_rx)*20.0f, camera_y, camera_z + sinf(camera_rx)*20.0f, camera_rx*-1, id, true);
			if( dataid != -1 ){
				sprintf(str, "Add SmallObject[%d].", dataid);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}

	//Destroy the small object.
	if( GetCommandNum("break", &id) == true ){
		if( (0 <= id)&&(id < MAX_SMALLOBJECT) ){
			smallobject *tsmallobject = ObjMgr.GetSmallObject(id);
			if( tsmallobject->GetEnableFlag() == true ){
				tsmallobject->Destruction();
				sprintf(str, "Broke SmallObject[%d].", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}

	//Delete a person
	if( GetCommandNum("delhuman", &id) == true ){
		if( (0 <= id)&&(id < MAX_HUMAN) ){
			human *thuman = ObjMgr.GetHumanObject(id);
			if( thuman->GetEnableFlag() == true ){

				//Disable all weapons that a person has
				int dummy;
				class weapon *weaponlist[TOTAL_HAVEWEAPON];
				int lnbs[TOTAL_HAVEWEAPON];
				int nbs[TOTAL_HAVEWEAPON];
				thuman->GetWeapon(&dummy, weaponlist, NULL, NULL);
				for(int i=0; i<TOTAL_HAVEWEAPON; i++){
					if( weaponlist[i] != NULL ){
						weaponlist[i]->SetEnableFlag(false);
						weaponlist[i] = NULL;
					}
					lnbs[i] = 0;
					nbs[i] = 0;
				}
				thuman->SetWeapon(weaponlist, lnbs, nbs);

				thuman->SetEnableFlag(false);
				sprintf(str, "Delete human[%d].", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}

	//Remove weapons
	if( GetCommandNum("delweapon", &id) == true ){
		if( (0 <= id)&&(id < MAX_WEAPON) ){
			weapon *tweapon = ObjMgr.GetWeaponObject(id);
			if( tweapon->GetEnableFlag() == true ){
				
				//If a person has that weapon, remove the association (dispose)
				int dummy;
				class weapon *weaponlist[TOTAL_HAVEWEAPON];
				int lnbs[TOTAL_HAVEWEAPON];
				int nbs[TOTAL_HAVEWEAPON];
				for(int i=0; i<MAX_HUMAN; i++){
					human *thuman = ObjMgr.GetHumanObject(i);
					if( thuman->GetEnableFlag() == true ){
						thuman->GetWeapon(&dummy, weaponlist, lnbs, nbs);
						for(int j=0; j<TOTAL_HAVEWEAPON; j++){
							if( weaponlist[j] == tweapon ){
								weaponlist[j] = NULL;
							}
						}
						thuman->SetWeapon(weaponlist, lnbs, nbs);
					}
				}

				tweapon->SetEnableFlag(false);
				sprintf(str, "Delete weapon[%d].", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}

	//Remove small items
	if( GetCommandNum("delobj", &id) == true ){
		if( (0 <= id)&&(id < MAX_SMALLOBJECT) ){
			smallobject *tsmallobject = ObjMgr.GetSmallObject(id);
			if( tsmallobject->GetEnableFlag() == true ){
				tsmallobject->SetEnableFlag(false);
				sprintf(str, "Delete SmallObject[%d].", id);
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
			}
		}
	}

	//Enable friendly fire (FF).
	if( strcmp(NewCommand, "ff") == 0 ){
		if( ObjMgr.GetFriendlyFireFlag(0) == false ){	//Check No. 0 provisionally.
			for(int i=0; i<MAX_HUMAN; i++){
				ObjMgr.SetFriendlyFireFlag(i, true);
			}
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Enable Friendly Fire.");
		}
		else{
			for(int i=0; i<MAX_HUMAN; i++){
				ObjMgr.SetFriendlyFireFlag(i, false);
			}
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Disable Friendly Fire.");
		}
	}

	//AI Player Operation
	if( strcmp(NewCommand, "bot") == 0 ){
		if( PlayerAI == false ){
			PlayerAI = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Player is AI control. (bot)");
		}
		else{
			PlayerAI = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Player is User control.");
		}
	}

	//Non-combating AI
	if( strcmp(NewCommand, "nofight") == 0 ){
		if( AINoFight == false ){
			AINoFight = true;
			for(int i=0; i<MAX_HUMAN; i++){
				HumanAI[i].SetNoFightFlag(true);
			}
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "AI had no Fight.");
		}
		else{
			AINoFight = false;
			for(int i=0; i<MAX_HUMAN; i++){
				HumanAI[i].SetNoFightFlag(false);
			}
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "AI had Fight.");
		}
	}

	//Be alert of AI
	if( strcmp(NewCommand, "caution") == 0 ){
		for(int i=0; i<MAX_HUMAN; i++){
			HumanAI[i].SetCautionMode();
		}
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Set cautious AI.");
	}

	//AI Level Settings
	if( GetCommandNum("ailevel", &id) == true ){
		if( id == -1 ){
			//Revert AI Level to Default Value
			for(int i=0; i<MAX_HUMAN; i++){
				HumanAI[i].SetAIlevel(-1);
			}
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Set all human AI-level to default.");
		}
		if( (0 <= id)&&(id < TOTAL_PARAMETERINFO_AILEVEL) ){
			//Overwrite to AI level value
			for(int i=0; i<MAX_HUMAN; i++){
				HumanAI[i].SetAIlevel(id);
			}
			sprintf(str, "Set all human to AI-level [%d].", id);
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}
	}
	else if( strcmp(NewCommand, "ailevel") == 0 ){
		//Revert AI Level to Default Value
		for(int i=0; i<MAX_HUMAN; i++){
			HumanAI[i].SetAIlevel(-1);
		}
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Set all human AI-level to default.");
	}

	//Stop AI processing
	if( strcmp(NewCommand, "stop") == 0 ){
		if( AIstop == false ){
			AIstop = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Stopped AI control.");
		}
		else{
			AIstop = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Started AI control.");
		}
	}

	/*
	//Mission
	if( strcmp(NewCommand, "comp") == 0 ){
		end_framecnt += 1;
		MainGameInfo.missioncomplete = true;
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Mission complete.");
	}

	//Mission failure
	if( strcmp(NewCommand, "fail") == 0 ){
		end_framecnt += 1;
		MainGameInfo.missioncomplete = false;
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Mission fail.");
	}
	*/

	//Stop processing event points
	if( strcmp(NewCommand, "estop") == 0 ){
		if( EventStop == false ){
			EventStop = true;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Stopped Event chains.");
		}
		else{
			EventStop = false;
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Started Event chains.");
		}
	}

	//Version information acquisition
	if( strcmp(NewCommand, "ver") == 0 ){
		sprintf(str, "%s   Version:%s", GAMENAME, GAMEVERSION);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		sprintf(str, " Graphics : %s", GRAPHICS_CORE);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		sprintf(str, " Sound : %s", SOUND_CORE);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		sprintf(str, " Input : %s", INPUT_CORE);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
	}

	//execution speed
	if( strcmp(NewCommand, "speed") == 0 ){
		if( GameSpeed == 1 ){ GameSpeed = 2; }
		else if( GameSpeed == 2 ){ GameSpeed = 4; }
		else{ GameSpeed = 1; }

		sprintf(str, "Set GameSpeed x%d", GameSpeed);
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
	}

	//Window Full Screen Switching
	if( strcmp(NewCommand, "window") == 0 ){
		//Get current display mode
		bool flag = d3dg->GetFullScreenFlag();

		if( flag == false ){ flag = true; }
		else{ flag = false; }

		if( ChangeWindowMode(WindowCtrl, d3dg, inputCtrl, this, flag) != 0 ){
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "[Error] Change failed.");
		}
		else{
			if( flag == true ){
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Changed FullScreen mode.");
			}
			else{
				AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Changed Window mode.");
			}
		}
	}

	//Take a
	// Delay the screenshot by two frames so the console has time to disappear.
	if( ScreenShot == 3 ){
		char fname[256];

		//Determine the file name
		GetTimeName(fname);
		strcat(fname, ".bmp");

		//Shooting/Save
		if( d3dg->SaveScreenShot(fname) == true ){
			sprintf(str, "Saved Screenshot  (File:%s)", fname);
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), str);
		}
		else{
			AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "[Error] Save failed.");
		}

		ScreenShot = 0;
	}
	else{
		if( strcmp(NewCommand, "ss") == 0 ){
			ScreenShot = 1;
		}
	}

	//Clear
	if( strcmp(NewCommand, "clear") == 0 ){
		for(int i=0; i<MAX_CONSOLELINES; i++){
			InfoConsoleData[i].color = d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f);
			InfoConsoleData[i].textdata[0] = '\0';
		}
	}

	//Close the console
	if( strcmp(NewCommand, "exit") == 0 ){
		AddInfoConsole(d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), "Closed debug console.");
		Show_Console = false;
	}

#ifdef _DEBUG
	//Reset operation
	if( strcmp(NewCommand, "f12") == 0 ){
		GameState->PushF12Key();
		GameSpeed = 1;
	}
#endif

	if( strcmp(NewCommand, "mikan") == 0 ){
		AddInfoConsole(d3dg->GetColorCode(1.0f,0.5f,0.0f,1.0f), "Hello! I'm [-_-;](mikan).");
		AddInfoConsole(d3dg->GetColorCode(1.0f,0.5f,0.0f,1.0f), "Thank you for playing OpenXOPS!");
	}
}

//! @brief Render the debug console.
void maingame::RenderConsole()
{
	//Background.
	d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth(), (MAX_CONSOLELINES+1)*17 + 5 + 5, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.75f));

	//Characters in display
	for(int i=0; i<MAX_CONSOLELINES; i++){
		if( InfoConsoleData[i].textdata[0] != NULL ){
			d3dg->Draw2DTextureDebugFontText(5+1, i*17+5+1, InfoConsoleData[i].textdata, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
			d3dg->Draw2DTextureDebugFontText(5, i*17+5, InfoConsoleData[i].textdata, InfoConsoleData[i].color);
		}
	}

	//Incoming characters
	int cnt;
	char str[MAX_CONSOLELEN];
	for(cnt=0; cnt<MAX_CONSOLELINES; cnt++){
		if( InfoConsoleData[cnt].textdata[0] == NULL ){ break; }
	}
	strcpy(str, CONSOLE_PROMPT);
	strcat(str, ConsoleTextBoxControl.GetTextBoxStr());
	d3dg->Draw2DTextureDebugFontText(5+1, cnt*17+5+1, str, d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
	d3dg->Draw2DTextureDebugFontText(5, cnt*17+5, str, d3dg->GetColorCode(1.0f,1.0f,0.0f,1.0f));
}

#endif

void maingame::Destroy()
{
	//Block Data Release
	d3dg->CleanupMapdata();

	//Object Manager Release
	ObjMgr.Cleanup();

	//Release additional accessories
	for(int i=0; i<MAX_ADDSMALLOBJECT; i++){
		Resource.CleanupAddSmallObject(i);
	}

	//Free the background sky.
	Resource.CleanupSkyModelTexture();

#ifdef ENABLE_DEBUGCONSOLE
	//Release console data
	if( InfoConsoleData != NULL ){ delete [] InfoConsoleData; }
	ConsoleTextBoxControl.DestroyTextBox();
#endif

	GameState->NextState();
}

//! @brief Constructor
result::result()
{}

//! @brief Destructor
result::~result()
{}

//! @brief 2D drawing part of the result screen
void result::Render2D()
{
	char mname[64];
	char str[32];
	float effectA = GetEffectAlphaLoop(framecnt, 1.0f, 0.8f, true);
	int intontarget;
	float rate;

	//Lifetime rate calculation
	intontarget = (int)floor(GameInfoData.ontarget);
	if( GameInfoData.fire == 0 ){
		rate = 0.0f;
	}
	else{
		rate = (float)intontarget / GameInfoData.fire * 100;
	}

	//Note: The background image is automatically drawn.

	//Fixed character display
	d3dg->Draw2DTextureFontTextCenterScaling(0, 40, "RESULT", d3dg->GetColorCode(1.0f,0.0f,1.0f,effectA), 50, 42);

	//Get the mission name and display
	if( MIFdata.GetFiletype() == false ){
		GameParamInfo.GetOfficialMission(GameInfoData.selectmission_id, NULL, mname, NULL, NULL, NULL, NULL);
	}
	else{
		strcpy(mname, MIFdata.GetMissionFullname());
	}
	d3dg->Draw2DTextureFontTextCenterScaling(0, 100, mname, d3dg->GetColorCode(0.5f,0.5f,1.0f,1.0f), 18, 25);

	//Mission Clearance
	if( GameInfoData.missioncomplete == true ){
		d3dg->Draw2DTextureFontTextCenterScaling(0, 150, "mission successful", d3dg->GetColorCode(0.0f,1.0f,0.0f,1.0f), 24, 32);
	}
	else{
		d3dg->Draw2DTextureFontTextCenterScaling(0, 150, "mission failure", d3dg->GetColorCode(1.0f,0.0f,0.0f,1.0f), 24, 32);
	}

	//Result display
	sprintf(str, "Time  %dmin %dsec", GameInfoData.framecnt/(int)GAMEFPS/60, GameInfoData.framecnt/(int)GAMEFPS%60);
	d3dg->Draw2DTextureFontTextCenterScaling(0, 210, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 20, 32);
	sprintf(str, "Rounds fired  %d", GameInfoData.fire);
	d3dg->Draw2DTextureFontTextCenterScaling(0, 260, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 20, 32);
	sprintf(str, "Rounds on target  %d", intontarget);
	d3dg->Draw2DTextureFontTextCenterScaling(0, 310, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 20, 32);
	sprintf(str, "Accuracy rate  %.1f%%", rate);
	d3dg->Draw2DTextureFontTextCenterScaling(0, 360, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 20, 32);
	sprintf(str, "Kill  %d / HeadShot  %d", GameInfoData.kill, GameInfoData.headshot);
	d3dg->Draw2DTextureFontTextCenterScaling(0, 410, str, d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 20, 32);
}

//! @brief Initializing screen class (setting class)
void InitScreen(WindowControl *WindowCtrl, opening *Opening, mainmenu *MainMenu, briefing *Briefing, maingame *MainGame, result *Result)
{
	if( WindowCtrl == NULL ){ return; }
	if( Opening == NULL ){ return; }
	if( MainMenu == NULL ){ return; }
	if( Briefing == NULL ){ return; }
	if( MainGame == NULL ){ return; }
	if( Result == NULL ){ return; }

	Opening->SetClass(&GameState, WindowCtrl, &d3dg, &inputCtrl, &GameSound);
	MainMenu->SetClass(&GameState, WindowCtrl, &d3dg, &inputCtrl, &GameSound);
	Briefing->SetClass(&GameState, WindowCtrl, &d3dg, &inputCtrl);
	MainGame->SetClass(&GameState, WindowCtrl, &d3dg, &inputCtrl, &GameSound);
	Result->SetClass(&GameState, WindowCtrl, &d3dg, &inputCtrl);
}

//! @brief screenuting screen class
void ProcessScreen(WindowControl *WindowCtrl, opening *Opening, mainmenu *MainMenu, briefing *Briefing, maingame *MainGame, result *Result, unsigned int framecnt)
{
	if( WindowCtrl == NULL ){ return; }
	if( Opening == NULL ){ return; }
	if( MainMenu == NULL ){ return; }
	if( Briefing == NULL ){ return; }
	if( MainGame == NULL ){ return; }
	if( Result == NULL ){ return; }

	UNREFERENCED_PARAMETER(framecnt);
	int error;

	switch(GameState.GetState()){
		//Opening Initialization
		case STATE_CREATE_OPENING:
			error = Opening->Create();
            Opening->ClearRenderHistory();
			if( error != 0 ){
				if( (error&0x0001) != 0 ){
					WindowCtrl->ErrorInfo("block data open failed");
				}
				if( (error&0x0002) != 0 ){
					WindowCtrl->ErrorInfo("point data open failed");
				}
				//WindowCtrl->CloseWindow();
				GameState.PushBackSpaceKey();
			}
			break;

		//Opening
		case STATE_NOW_OPENING:
			Opening->CaptureRenderState();
            ObjMgr.CaptureRenderState();
            Opening->Input();
			Opening->Process();
			Opening->Sound();

			break;

		//Opening Disposal
		case STATE_DESTROY_OPENING:
			Opening->Destroy();
			break;

		//Menu Initialization
		case STATE_CREATE_MENU:
			error = MainMenu->Create();
            MainMenu->ClearRenderHistory();
			if( error != 0 ){
				if( (error&0x0001) != 0 ){
					WindowCtrl->ErrorInfo("block data open failed");
				}
				if( (error&0x0002) != 0 ){
					WindowCtrl->ErrorInfo("point data open failed");
				}
				WindowCtrl->CloseWindow();
			}
			break;

		//Run Menu
		case STATE_NOW_MENU:
			MainMenu->CaptureRenderState();
            ObjMgr.CaptureRenderState();
            MainMenu->Input();
			MainMenu->Process();
			MainMenu->Sound();

			break;

		//Menu disposal
		case STATE_DESTROY_MENU:
			MainMenu->Destroy();
			break;

		//Bleefing Initialization
		case STATE_CREATE_BRIEFING:
			error = Briefing->Create();
			if( error == 1 ){
				WindowCtrl->ErrorInfo("briefing data open failed");
				WindowCtrl->CloseWindow();
			}
			break;

		//Briefing execution
		case STATE_NOW_BRIEFING:
			Briefing->Input();
			Briefing->Process();

			break;

		//Briefing Disposal
		case STATE_DESTROY_BRIEFING:
			Briefing->Destroy();
			break;

		//Initialization of main game
		case STATE_CREATE_MAINGAME:
			error = MainGame->Create();
            MainGame->ClearRenderHistory();
			if( error != 0 ){
				if( (error&0x0001) != 0 ){
					WindowCtrl->ErrorInfo("block data open failed");
				}
				if( (error&0x0002) != 0 ){
					WindowCtrl->ErrorInfo("point data open failed");
				}
				WindowCtrl->CloseWindow();
			}
			break;

		//Main game execution
		case STATE_NOW_MAINGAME:
            if(MainGame->GetGameSpeed()==0){MainGame->CaptureRenderState();ObjMgr.CaptureRenderState();}
			for(int i=0; i<MainGame->GetGameSpeed(); i++){
				MainGame->CaptureRenderState();
                ObjMgr.CaptureRenderState();
                MainGame->Input();
				MainGame->Process();
				MainGame->Sound();
			}

			break;

		//Main Game Disposal
		case STATE_DESTROY_MAINGAME:
			MainGame->Destroy();
			break;

		//Initialization of results
		case STATE_CREATE_RESULT:
			error = Result->Create();
			break;

		//Run the results screen.
		case STATE_NOW_RESULT:
			Result->Input();
			Result->Process();

			break;

		//Dispose of the results screen.
		case STATE_DESTROY_RESULT:
			Result->Destroy();
			break;

		//End of game
		case STATE_EXIT:
			WindowCtrl->CloseWindow();
			break;

		default:
			break;
	}
}

// Presentation never advances simulation, input, sound, or clear-time counters.
void RenderScreen(WindowControl *WindowCtrl, opening *Opening, mainmenu *MainMenu, briefing *Briefing, maingame *MainGame, result *Result)
{
    switch(GameState.GetState()){
    case STATE_NOW_OPENING:

				if( Opening->RenderMain() == true ){
					int ReturnCode;
					ReturnCode = ResetGame(WindowCtrl);
					if( ReturnCode == 2 ){
						WindowCtrl->ErrorInfo("Reset‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
						WindowCtrl->CloseWindow();
					}
					else if( ReturnCode == 0 ){
						Opening->Recovery();

						//Restart the current screen
						//GameState.PushF12Key();
					}
				}
			
        break;
    case STATE_NOW_MENU:

				if( MainMenu->RenderMain() == true ){
					int ReturnCode;
					ReturnCode = ResetGame(WindowCtrl);
					if( ReturnCode == 2 ){
						WindowCtrl->ErrorInfo("Reset‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
						WindowCtrl->CloseWindow();
					}
					else if( ReturnCode == 0 ){
						MainMenu->Recovery();

						//Restart the current screen
						//GameState.PushF12Key();
					}
				}
			
        break;
    case STATE_NOW_BRIEFING:

				if( Briefing->RenderMain() == true ){
					int ReturnCode;
					ReturnCode = ResetGame(WindowCtrl);
					if( ReturnCode == 2 ){
						WindowCtrl->ErrorInfo("Reset‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
						WindowCtrl->CloseWindow();
					}
					else if( ReturnCode == 0 ){
						Briefing->Recovery();

						//Restart the current screen
						//GameState.PushF12Key();
					}
				}
			
        break;
    case STATE_NOW_MAINGAME:

				if( MainGame->RenderMain() == true ){
					int ReturnCode;
					ReturnCode = ResetGame(WindowCtrl);
					if( ReturnCode == 2 ){
						WindowCtrl->ErrorInfo("Reset‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
						WindowCtrl->CloseWindow();
					}
					else if( ReturnCode == 0 ){
						MainGame->Recovery();

						//Restart the current screen
						//GameState.PushF12Key();
					}
				}
			
        break;
    case STATE_NOW_RESULT:

				if( Result->RenderMain() == true ){
					int ReturnCode;
					ReturnCode = ResetGame(WindowCtrl);
					if( ReturnCode == 2 ){
						WindowCtrl->ErrorInfo("Reset‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
						WindowCtrl->CloseWindow();
					}
					else if( ReturnCode == 0 ){
						Result->Recovery();

						//Restart the current screen
						//GameState.PushF12Key();
					}
				}
			
        break;
    default: break;
    }
}

//! @brief Window Full Screen Switching
//! @return Success: 0; failure (continue game): 1; failure (exit game): 2.
int ChangeWindowMode(WindowControl *WindowCtrl, D3DGraphics *d3dg, InputControl *inputCtrl, scene *RecoveryScene, bool fullscreen)
{
	if( WindowCtrl == NULL ){ return 2; }
	if( d3dg == NULL ){ return 2; }
	if( inputCtrl == NULL ){ return 2; }
	if( RecoveryScene == NULL ){ return 2; }

	int returncode = 0;
	bool ErrorFlag = false;

	//Switch processing (first time)
	WindowCtrl->ChangeWindowMode(fullscreen);
	d3dg->SetFullScreenFlag(fullscreen);
	if( ResetGame(WindowCtrl) != 0 ){
		if( fullscreen == false ){
			//When the window fails, it ends as an error.
			WindowCtrl->ErrorInfo("Reset‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
			WindowCtrl->CloseWindow();
			ErrorFlag = true;
			returncode = 2;
		}
		else{
			//If the window  full screen fails, go back to the window mode and try to continue the game.
			// The GPU may not support full-screen mode at the specified resolution.
			fullscreen = false;

			WindowCtrl->ChangeWindowMode(fullscreen);
			d3dg->SetFullScreenFlag(fullscreen);
			if( ResetGame(WindowCtrl) != 0 ){
				//If it fails even if it is returned, it ends as an error.
				WindowCtrl->ErrorInfo("Reset‚ÉŽ¸”s‚µ‚Ü‚µ‚½");
				WindowCtrl->CloseWindow();
				ErrorFlag = true;
				returncode = 2;
			}
			else{
				returncode = 1;
			}
		}
	}

	//After processing, such as recovery, when switched successfully.
	if( ErrorFlag == false ){
		RecoveryScene->Recovery();

		//Get key input
		// Discard mouse movement caused by a display-resolution change.
		inputCtrl->GetInputState(true);
		inputCtrl->MoveMouseCenter();
	}

	return returncode;
}