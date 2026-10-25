//! @file gamemain.h
//! @brief Game main processing header

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

#ifndef GAMEMAIN_H
#define GAMEMAIN_H

#define MAINMENU_X (GameConfig.GetScreenWidth() - 360)	//!< Display of menu X coordinates (top left)
#define MAINMENU_Y (GameConfig.GetScreenHeight() - 340)	//!< Display of menu Y coordinates ()
#define TOTAL_MENUITEMS 8								//!< Number of missions displayed on menu 1 screen
#define MAINMENU_H (TOTAL_MENUITEMS+2)*30 + 25			//!< Menu display size/height
#define MAINMENU_OPTION_X 10							//!< Display of menu X coordinates of optional item link (Left top)
#define MAINMENU_OPTION_Y (MAINMENU_Y+MAINMENU_H - 16)	//!< Display menu Y coordinates of optional item link ()

#define INPUT_ARROWKEYS_ANGLE DegreeToRadian(4)		//!< Rotation angle with directional key
#define INPUT_F1NUMKEYS_ANGLE DegreeToRadian(2)		//!< The rotation angle of the tenky from the three-person viewpoint

#define HUDA_WEAPON_POSX (GameConfig.GetScreenWidth() - 255)	//!< X coordinates to draw weapon information
#define HUDA_WEAPON_POSY (GameConfig.GetScreenHeight() - 98)	//!< Y coordinates to draw weapon information
#define HUDA_WEAPON_SIZEW 8										//!< Area and horizontal size to draw weapon information (number of 32 pixels)
#define HUDA_WEAPON_SIZEH 3										//!< Area and vertical size for drawing weapon information (number of 32 pixels)

#define VIEW_HEIGHT 19.0f							//!< Height of viewpoint
#define VIEW_DIST 0.1f								//!< Distance from center to viewpoint
#define VIEW_F1MODE_ANGLE DegreeToRadian(-22.5f)	//!< Three-person viewpoint angle (initial)
#define VIEW_F1MODE_DIST 14.0f						//!< Three-person perspective distance (max)
#define VIEW_FREECAMERA_SCALE 1.5f					//!< Travel speed with free camera (2x high speed mode)
#define VIEWANGLE_NORMAL DegreeToRadian(65)			//!< Standard Viewing Angle

#define TOTAL_EVENTLINE 3				//!< Number of Events
#define TOTAL_EVENTENTRYPOINT_0 -100	//!< Line 0 start identification number
#define TOTAL_EVENTENTRYPOINT_1 -110	//!< Line 1 Start Identification Number
#define TOTAL_EVENTENTRYPOINT_2 -120	//!< Line 2 Start Identification Number

#define TOTAL_EVENTENT_SHOWMESSEC 5.0f		//!< Number of seconds to display an event message

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 3		//!< Select include file.
#endif
#include "main.h"

#ifdef ENABLE_MENUOPTIONS
 #define OPTIONS_P1_W 340															//!< Option screen No.1 Width
 #define OPTIONS_P1_H (30*6)														//!< Height of options screen No. 1.
 #define OPTIONS_P1_X (GameConfig.GetScreenWidth() - 360)							//!< Option screen No.1 X coordinate
 #define OPTIONS_P1_Y (MAINMENU_Y+MAINMENU_H - OPTIONS_P1_H - 24)					//!< Option screen No.1 Y coordinate
 #define OPTIONS_P1_DATAS 3															//!< Option screen No.1 Number of link texts
 #define OPTIONS_P2_W 600															//!< Option screen No.2 Width
 #define OPTIONS_P2_H 360															//!< Height of options screen No. 2.
 #define OPTIONS_P2_X ((GameConfig.GetScreenWidth() - OPTIONS_P2_W)/2)				//!< Option screen No.2 X coordinate
 #define OPTIONS_P2_Y (105 + (GameConfig.GetScreenHeight()-105 - OPTIONS_P2_H)/2)	//!< Option screen No.2 Y coordinates
 #define OPTIONS_P2_DATAS 32														//!< Option screen No.2 Number of link texts
 #define OPTIONS_P3_W 600															//!< Option screen No.3 Width
 #define OPTIONS_P3_H 320															//!< Height of options screen No. 3.
 #define OPTIONS_P3_X ((GameConfig.GetScreenWidth() - OPTIONS_P3_W)/2)				//!< Option screen No.3 X coordinate
 #define OPTIONS_P3_Y (105 + (GameConfig.GetScreenHeight()-105 - OPTIONS_P3_H)/2)	//!< Option screen No.3 Y coordinates
 #define OPTIONS_P3_DATAS 1															//!< Option screen No.3 Number of link texts
#if (GRAPHIC_ENGINE == 0)||(GRAPHIC_ENGINE == 1)
 #define OPTIONS_LICENSELINE 26											//!< Number of lines of the license statement
#elif GRAPHIC_ENGINE == 8
 #define OPTIONS_LICENSELINE 6											//!< Number of lines of the license statement
#endif	//GRAPHIC_ENGINE
#endif	//ENABLE_MENUOPTIONS

#ifdef ENABLE_DEBUGCONSOLE
 #define MAX_CONSOLELEN 78		//!< Number of characters per debug-console row.
 #define MAX_CONSOLELINES 12	//!< Number of debug-console rows.
 #define CONSOLE_PROMPT ">"		//!< Debug-console prompt.
#endif

int InitGame(WindowControl *WindowCtrl, int mode, char *MIFpath);
void CleanupGame();

//! A structure that receives the state of the game
typedef struct
{
	int selectmission_id;	//!< Selected Missions
	char mifpath[MAX_PATH];	//!< MIF file path
	bool missioncomplete;	//!< Mission Completed
	unsigned int framecnt;	//!< Number of frames
	int fire;		//!< Number of shots fired.
	float ontarget;	//!< Number of hits.
	int kill;		//!< Number of defeated enemies
	int headshot;	//!< The number of lives in the enemy head
} GameInfo;

#ifdef ENABLE_MENUOPTIONS
//! Menu link text structure
typedef struct
{
	bool enable;	//!< Enabled flags as link text
	int pos_x;		//!< X Coordinate
	int pos_y;		//!< Y Coordinate
	int text_w;		//!< Width of characters
	int text_h;		//!< Character height
	int text_defaultcolor;		//!< Standard character color
	int text_cursorcolor;		//!< Character color at the time of printing
	char text[32];				//!< Textbook
} MenuLinkTextData;
#endif	//ENABLE_MENUOPTIONS

#ifdef ENABLE_DEBUGCONSOLE
//! Structure that manages the console
typedef struct
{
	int color;							//!< Color:
	char textdata[MAX_CONSOLELEN+1];	//!< String.
} ConsoleData;
#endif

//! @brief Opening Screen Management Class
//! @details Manage the opening screen.
class opening : public D3Dscene
{
	float add_camera_x;			//!< Change in camera X coordinate.
	float add_camera_y;			//!< Change in camera Y coordinate.
	float add_camera_z;			//!< Camera Coordinate Z Weight Loss
	float add_camera_rx;		//!< Change in camera rotation RX.
	float add_camera_ry;		//!< Change in camera rotation RY.
	//int opening_banner; // Texture ID displayed during the opening.
	void Render3D();
	void Render2D();

public:
	opening();
	~opening();
	int Create();
	int Recovery();
	void Input();
	void Process();
	void Destroy();
};

//! @brief Menu screen management class
//! @details Manage the menu screen.
class mainmenu : public D3Dscene
{
	char demopath[MAX_PATH];		//!< Demonstration File Path
	int modescreen;					//!< Display screen
	int mainmenu_mouseX;			//!< Menu screen mouse X coordinate
	int mainmenu_mouseY;			//!< Menu screen mouse Y coordinates
	int mainmenu_mouseY_old;		//!< Menu screen mouse Y coordinates (previous frame)
	class ScrollbarObject scrollbar_official;	//!< Standard Mission Scroll Bar
	class ScrollbarObject scrollbar_addon;		//!< addon scrollbar
    int cursorTexture;
	int gametitle;					//!< Game title image
#ifdef ENABLE_MENUOPTIONS
	MenuLinkTextData Options_p1LinkTextData[OPTIONS_P1_DATAS];		//!< Option screen No.1 Link text data
	MenuLinkTextData Options_p2LinkTextData[OPTIONS_P2_DATAS];		//!< Option screen No.2 Link text data
	MenuLinkTextData Options_p3LinkTextData[OPTIONS_P3_DATAS];		//!< Option screen No.3 Link text data
	class ScrollbarObject scrollbar_license;		//!< License text scrollbar
	int Options_p2LinkTextID;						//!< Option screen No.2 ID of pressed link text
	bool Options_FullscreenFlag_old;				//!< Option screen No.2 Full screen flag
	int Options_KeyConfigID;						//!< Option screen No.2 Key setting ID
	int Options_KeyConfigCnt;						//!< Option screen No.2 Key setting count (Number of frames)
	const char *licenseinfo[OPTIONS_LICENSELINE];	//!< License Statement
#endif	//ENABLE_MENUOPTIONS

	void Render3D();
	void Render2D();
#ifdef ENABLE_MENUOPTIONS
	void CreateOptions();
	void InputOptions();
	void ProcessOptions();
	void RenderOptions();
#endif	//ENABLE_MENUOPTIONS

public:
	mainmenu();
	~mainmenu();
	int Create();
	int Recovery();
	void Input();
	void Process();
	void Destroy();
};

//! @brief Briefing screen management class
//! @details Manage the briefing screen.
class briefing : public D2Dscene
{
	bool TwoTexture;	//!< Use two briefing images.
	int TextureA;		//!< Briefing image A
	int TextureB;		//!< Bleefing Image B
	void Render2D();

public:
	briefing();
	~briefing();
	int Create();
	int Recovery();
	void Destroy();
};

//! @brief Main game screen management class
//! @details Manage the main game screen.
class maingame : public D3Dscene
{
	//class EventControl Event[TOTAL_EVENTLINE]; // Event control class.
	int SkyNumber;			//!< Background empty number
	bool DarkScreenFlag;	//!< Dark screen
	bool FogFlag;			//!< Fog Enabled
	float mouse_rx;				//!< Horizontal angle controlled by the mouse.
	float mouse_ry;				//!< Vertical axis angle by mouse
	float view_rx;				//!< Difference between mouse and camera angles (horizontal axis).
	float view_ry;				//!< Difference between mouse angle and camera angle (vertical axis)
	float add_camera_rx;		//!< Change in camera rotation RX.
	float add_camera_ry;		//!< Change in camera rotation RY.
	bool ShowInfo_Debugmode;	//!< Debug mode for displaying coordinates and other information.
	bool Camera_Debugmode;		//!< Camera debug mode.
	bool tag;					//!< Show tags for objects
	bool radar;					//!< Simple touch panel display
	bool wireframe;				//!< Display Printing Wire Frame
	bool nomodel;				//!< Model drawing flag
	bool CenterLine;			//!< Display the center line in the 3D space
	bool Camera_Blind;			//!< Blindfolded Drawing
	bool Camera_F1mode;			//!< Camera F1 Mode
	int Camera_F2mode;			//!< Camera F2 Mode
	bool Camera_HOMEmode;		//!< Camera HOME mode
	bool Cmd_F5;				//!< Backing F5 mode
	int InvincibleID;			//!< Determination of an invincible person
	bool PlayerAI;				//!< Delegate Player Operations to AI
	bool AIstop;				//!< Stop AI processing
	bool AINoFight;				//!< Prevent the AI from fighting.
	int AIdebuginfoID;			//!< Display AI debug information.
	int start_framecnt;			//!< Count at the start of the main game
    unsigned int elapsed_ticks; // Frozen HUD time; simulation ticks only.
    bool elapsed_stopped;
	int end_framecnt;			//!< Main Game End Count
	bool EventStop;				//!< Stop event processing
	bool EventSkip[TOTAL_EVENTLINE];	//!< Skip event processing
	int GameSpeed;				//!< execution speed (1 or more integers)
	int message_id;				//!< Event Message Number
	int message_cnt;			//!< Event Message Count
	bool redflash_flag;			//!< Red Flash Drawing Flag
	int time;		//!< timer
	int time_input;					//!< Processing time of input acquisition
	int time_process_object;		//!< Processing time of the basic object
	int time_process_ai;			//!< AI Processing Time
	int time_process_event;			//!< Event Processing Time
	int time_sound;					//!< Sound Processing Time
	int time_render;				//!< Drawing Processing Time
	GameInfo MainGameInfo;			//!< Management Class for Result
	bool CheckInputControl(int CheckKey, int mode);
	void InputPlayer(human *myHuman, int mouse_x, int mouse_y, float MouseSensitivity);
	void InputViewCamera(int mouse_x, int mouse_y, float MouseSensitivity);
	void Render3D();
	void Render2D();
	bool GetRadarPos(float in_x, float in_y, float in_z, int RadarPosX, int RadarPosY, int RadarSize, float RadarWorldR, int *out_x, int *out_y, float *local_y, bool check);
	void RenderRadar();

#ifdef ENABLE_DEBUGCONSOLE
	bool Show_Console;						//!< Display the debug console.
	ConsoleData *InfoConsoleData;			//!< Displayed debug-console data.
	TextBoxObject ConsoleTextBoxControl;	//!< Debug-console input data.
	char NewCommand[MAX_CONSOLELEN];		//!< Newly entered unprocessed commands
	int ScreenShot;		//!< Take a screenshot.
	void AddInfoConsole(int color, const char *str);
	bool GetCommandNum(const char *cmd, int *num);
	void InputConsole();
	void ProcessConsole();
	void RenderConsole();
#endif

public:
	maingame();
	~maingame();
	void SetShowInfoFlag(bool flag);
	int GetGameSpeed();
	int Create();
	int Recovery();
	void Input();
	void Process();
	void Sound();
	void Destroy();
};

//! @brief Class for managing the results screen.
//! @details Manage the result screen.
class result : public D2Dscene
{
	void Render2D();

public:
	result();
	~result();
};

void InitScreen(WindowControl *WindowCtrl, opening *Opening, mainmenu *MainMenu, briefing *Briefing, maingame *MainGame, result *Result);
void RenderScreen(WindowControl *WindowCtrl, opening *Opening, mainmenu *MainMenu, briefing *Briefing, maingame *MainGame, result *Result);
void ProcessScreen(WindowControl *WindowCtrl, opening *Opening, mainmenu *MainMenu, briefing *Briefing, maingame *MainGame, result *Result, unsigned int framecnt);
int ChangeWindowMode(WindowControl *WindowCtrl, D3DGraphics *d3dg, InputControl *inputCtrl, scene *RecoveryScene, bool fullscreen);

#endif