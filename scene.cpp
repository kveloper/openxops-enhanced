//! @file scene.cpp
//! @brief Defining a class that manages each screen

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

#include "scene.h"

//! @brief Constructor
scene::scene()
{
	GameState = NULL;
	WindowCtrl = NULL; 
	d3dg = NULL;
	inputCtrl = NULL;
	framecnt = 0;
}

//! @brief Destructor
scene::~scene()
{}

//! @brief Set Class
void scene::SetClass(StateMachine *in_GameState, WindowControl *in_WindowCtrl, D3DGraphics *in_d3dg, InputControl *in_inputCtrl)
{
	GameState = in_GameState;
	WindowCtrl = in_WindowCtrl; 
	d3dg = in_d3dg;
	inputCtrl = in_inputCtrl;
}

//! @brief Scene creation
int scene::Create()
{
	inputCtrl->MoveMouseCenter();
	framecnt = 0;

	GameState->NextState();
	return 0;
}

//! @brief Scene drawing recovery
int scene::Recovery()
{
	return 0;
}

//! @brief Scene input processing
void scene::Input()
{
	inputCtrl->GetInputState(false);

	if( inputCtrl->CheckKeyDown(GetEscKeycode()) ){
		GameState->PushBackSpaceKey();
	}
	else if( inputCtrl->CheckMouseButtonUpL() ){
		GameState->PushMouseButton();
	}
}

//! @brief Scene calculation processing
void scene::Process()
{
	framecnt += 1;
}

//! @brief Scene drawing processing
//! @return Success:false Fail:true
//! @attention If it fails, call the ResetD3D() function in the D3DGraphics class.
bool scene::RenderMain()
{
	if( d3dg->StartRender() ){ return true; }

	d3dg->Draw2DBox(0, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), d3dg->GetColorCode(0.0f,0.0f,0.0f,1.0f));
	d3dg->Draw2DTextureFontText(10, 10, "hello world !", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 20, 32);

	d3dg->ScreenBrightness(GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), GameConfig.GetBrightness());

	return d3dg->EndRender();
}

//! @brief Scene release processing
void scene::Destroy()
{
	GameState->NextState();
}

//! @brief Set alpha and transparency based on frame count (1 time)
//! @param tframecnt Count value to be targeted
//! @param MaxAlpha Maximum alpha value
//! @param timingsec Interval in seconds.
//! @param offsetsec Offsets/seconds
//! @param reversal Inverted flag (false: field in true: field out)
//! @return Alpha Value
float scene::GetEffectAlpha(unsigned int tframecnt, float MaxAlpha, float timingsec, float offsetsec, bool reversal)
{
	float alpha;
	unsigned int frametiming;
	unsigned int frameoffset;

	frametiming = (unsigned int)(timingsec*GAMEFPS);
	frameoffset = (unsigned int)(offsetsec*GAMEFPS);
	alpha = MaxAlpha/frametiming * (tframecnt - frameoffset);

	if( alpha < 0.0f ){ alpha = 0.0f; }
	if( alpha > 1.0f ){ alpha = 1.0f; }

	if( reversal == true ){
		return 1.0f - alpha;
	}
	return alpha;
}

//! @brief Set alpha and transparency based on frame count (loop)
//! @param tframecnt Count value to be targeted
//! @param MaxAlpha Maximum alpha value
//! @param timingsec Interval in seconds.
//! @param reversal Inverted flag (false: field in true: field out)
//! @return Alpha Value
float scene::GetEffectAlphaLoop(unsigned int tframecnt, float MaxAlpha, float timingsec, bool reversal)
{
	float alpha;
	unsigned int frametiming;

	frametiming = (unsigned int)(timingsec*GAMEFPS);
	alpha = MaxAlpha/frametiming * (tframecnt%frametiming);

	if( reversal == true ){
		return 1.0f - alpha;
	}
	return alpha;
}

//! @brief Constructor
D2Dscene::D2Dscene()
{
	gametitle = -1;
}

//! @brief Destructor
D2Dscene::~D2Dscene()
{}

//! @brief 2D Scene Initialization
int D2Dscene::Create()
{
	gametitle = d3dg->LoadTexture("data\\title.dds", false, false);
	GameState->NextState();
	if( gametitle == -1 ){ return 1; }
	return 0;
}

//! @brief Scene drawing recovery
int D2Dscene::Recovery()
{
	//Initialize
	gametitle = d3dg->LoadTexture("data\\title.dds", false, false);
	if( gametitle == -1 ){ return 1; }
	return 0;
}

//! @brief 2D scene drawing processing(2D)
void D2Dscene::Render2D()
{
	d3dg->Draw2DTextureFontText(10, 10, "hello world !", d3dg->GetColorCode(1.0f,1.0f,1.0f,1.0f), 20, 32);
}

//! @brief 2D scene drawing processing (main)
//! @return Success:false Fail:true
//! @attention If it fails, call the ResetD3D() function in the D3DGraphics class.
bool D2Dscene::RenderMain()
{
	if( d3dg->StartRender() ){ return true; }

	d3dg->Draw2DTexture(0, 0, gametitle, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), 0.4f);
	d3dg->Draw2DBox(11, 11, GameConfig.GetScreenWidth() - 10, GameConfig.GetScreenHeight() - 10, d3dg->GetColorCode(0.0f,0.0f,0.0f,0.75f));

	Render2D();

	d3dg->ScreenBrightness(GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), GameConfig.GetBrightness());

	return d3dg->EndRender();
}

//! @brief 2D Scene Release
void D2Dscene::Destroy()
{
	d3dg->CleanupTexture(gametitle);
	GameState->NextState();
}


//! @brief Constructor
D3Dscene::D3Dscene()
{
	GameSound = NULL;
	camera_x = 0.0f;
	camera_y = 0.0f;
	camera_z = 0.0f;
	camera_rx = 0.0f;
	camera_ry = 0.0f;
}

//! @brief Destructor
D3Dscene::~D3Dscene()
{}

//! @brief Set up 3D scene class
void D3Dscene::SetClass(StateMachine *in_GameState, WindowControl *in_WindowCtrl, D3DGraphics *in_d3dg, InputControl *in_inputCtrl, SoundManager *in_GameSound)
{
	GameState = in_GameState;
	WindowCtrl = in_WindowCtrl; 
	d3dg = in_d3dg;
	inputCtrl = in_inputCtrl;
	GameSound = in_GameSound;
}

//! @brief 3D scene primary calculation processing
void D3Dscene::Process()
{}

//! @brief 3D scene sound playback processing
void D3Dscene::Sound()
{
	//Play Sound
	GameSound->PlayWorldSound(camera_x, camera_y, camera_z, camera_rx, 1);
}

//! @brief 3D scene drawing processing (3D)
void D3Dscene::Render3D()
{}

//! @brief 3D scene drawing processing(2D)
void D3Dscene::Render2D()
{}

//! @brief 3D scene drawing processing (main)
//! @return Success:false Fail:true
//! @attention If it fails, call the ResetD3D() function in the D3DGraphics class.
void D3Dscene::CaptureRenderState(){
    float pose[]={camera_x,camera_y,camera_z,camera_rx,camera_ry};cameraHistory.Capture(pose);
}

bool D3Dscene::RenderMain()
{
	if( d3dg->StartRender() ){ return true; }

    float saved[]={camera_x,camera_y,camera_z,camera_rx,camera_ry};
    float pose[]={camera_x,camera_y,camera_z,camera_rx,camera_ry};
    cameraHistory.Blend(pose,RenderInterpolationAlpha);
    camera_x=pose[0];camera_y=pose[1];camera_z=pose[2];camera_rx=pose[3];camera_ry=pose[4];
	Render3D();
	Render2D();

	d3dg->ScreenBrightness(GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight(), GameConfig.GetBrightness());

    camera_x=saved[0];camera_y=saved[1];camera_z=saved[2];camera_rx=saved[3];camera_ry=saved[4];
	return d3dg->EndRender();
}