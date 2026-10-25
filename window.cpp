//! @file window.cpp
//! @brief Create and control the main window

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

#include "window.h"

//! @brief Constructor
WindowControl::WindowControl()
{
	hInstance = 0;
	nCmdShow = 0;
	hWnd = NULL;

	timeBeginPeriod(1);		//Paired with timeEndPeriod.
}

//! @brief Destructor
WindowControl::~WindowControl()
{
	timeEndPeriod(1);		//Paired with timeBeginPeriod.
}

//! @brief Configure application information
//! @param in_hInstance Instance handle.
//! @param in_nCmdShow Window display status
void WindowControl::SetParam(HINSTANCE in_hInstance, int in_nCmdShow)
{
	hInstance = in_hInstance;
	nCmdShow = in_nCmdShow;
}

//! @brief Main window creation
//! @param title Window Title
//! @param width Width of window
//! @param height window height
//! @param fullscreen false: window display true: full screen display
//! @return success: true failure: false
//! @warning You need to set the setting with the SetParam() function first.
bool WindowControl::InitWindow(const char* title, int width, int height, bool fullscreen)
{
	if( title == NULL ){ return false; }
	if( (width <= 0)||(height <= 0) ){ return false; }

	WNDCLASS wc;
	int x, y;
	RECT Rect;
	DWORD dwStyle;

	//Windows Class Registration
	wc.style		= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	= WindowProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= hInstance;
	wc.hIcon		= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINICON));
	wc.hCursor		= NULL;
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName	= "MainWindow";
	if( RegisterClass(&wc) == 0 ){
		return false;
	}

	if( fullscreen == false ){
		dwStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	}
	else{
		dwStyle = WS_POPUP;
	}

	//Window size
	Rect.left = 0;
	Rect.top = 0;
	Rect.right = width;
	Rect.bottom = height;
	AdjustWindowRect(&Rect, dwStyle, FALSE);
	width = Rect.right - Rect.left;
	height = Rect.bottom - Rect.top;

	//Display position
	x = 0;
	y = 0;

	/*
	//In the window mode, tap the window at the center of the screen
	if( fullscreen == false ){
		SystemParametersInfo(SPI_GETWORKAREA, 0, &Rect, 0);
		x = (Rect.right - width)/2;
		y = (Rect.bottom - height)/2;
	}
	*/

#if GRAPHIC_ENGINE == 8
    if(fullscreen){
        POINT origin = {0, 0};
        MONITORINFO monitor = {sizeof(MONITORINFO)};
        if(GetMonitorInfo(MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY), &monitor)){
            x = monitor.rcMonitor.left; y = monitor.rcMonitor.top;
            width = monitor.rcMonitor.right - x; height = monitor.rcMonitor.bottom - y;
        }
    }
#endif

	//Window creation
	hWnd = CreateWindow( "MainWindow", title, dwStyle, x, y, width, height, NULL, NULL, hInstance, NULL );
	if( hWnd == NULL ){
		return false;
	}

	//Display.
	ShowWindow( hWnd, nCmdShow );

	return true;
}

//! @brief Window Procedure in the Main Window
LRESULT WINAPI WindowControl::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg ){
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//! @brief Window Display Mode Switching
//! @param fullscreen false: window display true: full screen display
//! @return success: true failure: false
bool WindowControl::ChangeWindowMode(bool fullscreen)
{
	if( hWnd == NULL ){ return false; }

#if GRAPHIC_ENGINE == 8
    MONITORINFO monitor = {sizeof(MONITORINFO)};
    if(!GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &monitor)) return false;
    RECT rect = {0, 0, GameConfig.GetScreenWidth(), GameConfig.GetScreenHeight()};
    DWORD style = fullscreen ? WS_POPUP : (WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX);
    int x = monitor.rcMonitor.left, y = monitor.rcMonitor.top;
    if(fullscreen) rect = monitor.rcMonitor;
    else AdjustWindowRect(&rect, style, FALSE);
    SetWindowLong(hWnd, GWL_STYLE, style);
    SetWindowPos(hWnd, NULL, x, y, rect.right-rect.left, rect.bottom-rect.top,
                 SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
#else
	DWORD dwStyle;
	RECT Rect;
	int width, height;

	//Get window size
	GetClientRect(hWnd, &Rect);

	if( fullscreen == false ){
		ChangeDisplaySettings(NULL, 0);		//Revert Display Resolution

		dwStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	}
	else{
		dwStyle = WS_POPUP;
	}

	//Window size
	AdjustWindowRect(&Rect, dwStyle, FALSE);
	width = Rect.right - Rect.left;
	height = Rect.bottom - Rect.top;

	//Apply.
	SetWindowLong(hWnd, GWL_STYLE, dwStyle);
	SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

#endif

	return true;
}

//! @brief Get Window Handle
//! @return Window Handle
HWND WindowControl::GethWnd()
{
	return hWnd;
}

//! @brief Processing window messages (main loop judgment)
//! @return Idle: 0; game main loop executed: 1; window closed: -1.
//! @attention If "1" is returned from this function, please execute the main process of the program.
//! @attention If "-1" is returned from this function, release each resource and interface and exit the game.
int WindowControl::CheckMainLoop()
{
    MSG msg={};
    // Drain a bounded batch so high-rate mouse input cannot starve presentation.
    for(int i=0;i<64 && PeekMessage(&msg,NULL,0,0,PM_REMOVE);i++){
        if(msg.message==WM_QUIT)return -1;
        TranslateMessage(&msg);DispatchMessage(&msg);
    }
    if(GetActiveWindow()==hWnd)return 1;
    WaitMessage();
    return 2; // Always reset the clock on resume, even if this message activated us.
}

//! @brief Error message display
//! @param *str Message
void WindowControl::ErrorInfo(const char *str)
{
	if( str == NULL ){ return; }

	MessageBox(hWnd, str, "error", MB_OK);
}

//! @brief Request to close the window
void WindowControl::CloseWindow()
{
	PostMessage(hWnd, WM_CLOSE, 0L, 0L);
}

#ifdef ENABLE_DEBUGLOG
//! @brief Write the user's environment information to the debug log.
//! @note OS information and language settings at this time
void GetOperatingEnvironment()
{
	char str[256];
	char OSname[128];
	char OSver[128];
	char OSbit[128];
	int strsize;
	HKEY hKey = NULL;

	//Get OS information from the registry
	strcpy(OSname, "");
	strcpy(OSver, "");
	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_EXECUTE | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS ){
		strsize = sizeof(OSname);
		RegQueryValueEx(hKey, "ProductName", 0, NULL, (LPBYTE)OSname, (LPDWORD)&strsize);
		strsize = sizeof(OSver);
		RegQueryValueEx(hKey, "DisplayVersion", 0, NULL, (LPBYTE)OSver, (LPDWORD)&strsize);
		RegCloseKey(hKey);
	}

	//OS bit count acquisition
	SYSTEM_INFO SystemInfo = {0};
	GetNativeSystemInfo(&SystemInfo);
	switch(SystemInfo.wProcessorArchitecture){
		case PROCESSOR_ARCHITECTURE_AMD64: strcpy(OSbit, "64bit"); break;
		case PROCESSOR_ARCHITECTURE_IA64:  strcpy(OSbit, "64bit"); break;
		case PROCESSOR_ARCHITECTURE_INTEL: strcpy(OSbit, "32bit"); break;
		default: strcpy(OSbit, "unknown bit");
	}

	//Output OS information to log
	sprintf(str, "OS NameF%s %s (%s)", OSname, OSver, OSbit);
	OutputLog.WriteLog(LOG_CHECK, "Environment", str);


	//Output language information to log
	sprintf(str, "OS LanguageF0x%04hx / 0x%04hx", GetSystemDefaultLangID(), GetUserDefaultLangID());
	OutputLog.WriteLog(LOG_CHECK, "Environment", str);
}
#endif

//! @brief Frames Per Second
//! @param getcnt Cycle to get fps (frame unit)
//! @return fps
float GetFps(int getcnt)
{
    UNREFERENCED_PARAMETER(getcnt);
    static LARGE_INTEGER frequency={},last={};
    static unsigned int frames=0;
    static float fps=0;
    LARGE_INTEGER now;QueryPerformanceCounter(&now);
    if(!frequency.QuadPart){QueryPerformanceFrequency(&frequency);last=now;return fps;}
    ++frames;
    const double seconds=(double)(now.QuadPart-last.QuadPart)/frequency.QuadPart;
    if(seconds>=0.25){fps=(float)(frames/seconds);frames=0;last=now;}
    return fps;
}

//! @brief Frames Per Second: Frame Rate
//! @return Adjustment performed: true; no adjustment: false.
//! @attention If you do not initialize the WindowControl class, the timer is not accurate.
bool ControlFps()
{
	static unsigned int ptimec = 0;
	unsigned int nowtime;
	unsigned int waittime;

	nowtime = GetTimeMS();
	waittime = GAMEFRAMEMS - (nowtime - ptimec);
	if( (0 < waittime)&&(waittime <= GAMEFRAMEMS) ){
		//timeBeginPeriod(1);
		Sleep(waittime);
		//timeEndPeriod(1);
		
		ptimec = GetTimeMS();
		return true;
	}

	ptimec = nowtime;
	return false;
}

//! @brief Apply milliseconds
//! @return Milliseconds
//! @attention If you do not initialize the WindowControl class, the timer is not accurate.
unsigned int GetTimeMS()
{
	unsigned int time;

	//timeBeginPeriod(1);
	time = timeGetTime();
	//timeEndPeriod(1);

	return time;
}

//! @brief Get a string by date
//! @param str Pointer to receive a string
void GetTimeName(char *str)
{
	if( str == NULL ){ return; }

	time_t timer;
	struct tm *local;

	timer = time(NULL);
	local = localtime(&timer);

	sprintf(str, "%04d%02d%02d%02d%02d%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
}

//! @brief Initialize random numbers
void InitRand()
{
	srand(GetTimeMS());
}

//! @brief Returns random integer values
//! @param num Range
//! @return 0 to num-1
int GetRand(int num)
{
	if( num <= 0 ){ return 0; }

	return rand()%num;

	//return rand() / (RAND_MAX/num);

	//Algorithm of the original XOPS?
	//static int memory = GetTimeMS();
	//int x;
	//memory = memory * 214013 + 2745024;
	//x = memory >> 16;
	//x = x & 0x00007FFF;
	//return x%num;
}

//! @brief Rounding
//! @param r Value.
//! @return Value.
float Round(float r)
{
	return (r > 0.0f) ? floorf(r + 0.5f) : ceilf(r - 0.5f);
}

#ifdef ENABLE_PATH_DELIMITER_SLASH
//! @brief Convert path separators.
//! @param *str File path pointer
//! @return New File Path Pointer
//! @note Replace '\\' with '/'.
char* ChangePathDelimiter(char *str)
{
	if( str == NULL ){ return NULL; }

	static char newstr[MAX_PATH];
	strcpy(newstr, str);

	for(int i=0; i<(int)strlen(newstr); i++){
		if( newstr[i] == '\\' ){
			newstr[i] = '/';
		}
	}

	return newstr;
}
#endif
