//! @file input.h
//! @brief Declaration of InputControl class

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

#ifndef INPUT_H
#define INPUT_H

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 1		//!< Select include file.
#endif
#include "main.h"

#include <windows.h>

#define INPUT_INTERFACE 0	//!< @brief Select the input interface. @details WindowsInput: 0; RawInput: 1; DirectInput: 2.

#if INPUT_INTERFACE == 0
 #define INPUT_CORE "WindowsInput"					//!< Version display information
#elif INPUT_INTERFACE == 1
 #include <commctrl.h>
 #pragma comment (lib, "comctl32.lib")

 #define INPUT_CORE "RawInput"						//!< Version display information
#elif INPUT_INTERFACE == 2
 #define DIRECTINPUT_VERSION 0x0800	//!< DIRECTINPUT_VERSION undefined. Defaulting to version 0x0800
 #include <dinput.h>

 #pragma comment(lib, "dinput8.lib")
 #pragma comment(lib, "dxguid.lib")

 #define INPUT_CORE "DirectInput"					//!< Version display information
#endif

//! @brief Class for managing input devices.
//! @details Manage input devices such as mouse and keyboard.
class InputControl
{
	HWND hWnd;				//!< Window Handle
#if INPUT_INTERFACE == 2
	LPDIRECTINPUT8 pDI;					//!< DIRECTINPUT8 pointer
	LPDIRECTINPUTDEVICE8 pDIDevice;		//!< For DIRECTINPUTDEVICE8 pointer keyboard
	LPDIRECTINPUTDEVICE8 pMouse;		//!< DIRECTINPUTDEVICE8 for pointer mouse
#else
	bool InitFlag;			//!< Initialization flag
#endif
	bool MouseLimitFlag;	//!< Flag to limit mouse movement
	char keys[256];			//!< Array to store key information
	char keys_lt[256];		//!< (Last) Array to store key information
	int mx;					//!< Mouse X Coordinate
	int my;					//!< Mouse Y coordinates
#if INPUT_INTERFACE == 0
	POINT point_lt;			//!< (previous) mouse coordinates
#endif
	bool mbl;				//!< Left mouse button
	bool mbr;				//!< Right mouse button
	bool mbl_lt;			//!< Left mouse button
	bool mbr_lt;			//!< Right-button of mouse
#if INPUT_INTERFACE == 1
	char keys_proc[256];	//!< Array to store key information (procedure)
	int mx_proc;			//!< Mouse X Coordinate (Procedure)
	int my_proc;			//!< Y coordinates of the mouse (procedure)
	bool mbl_proc;			//!< Mouse Left Button (Procedure)
	bool mbr_proc;			//!< Mouse Right Button (Procedure)

	static LRESULT CALLBACK SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	void GetRowInput(WPARAM wParam, LPARAM lParam);
#endif

public:
	InputControl();
	~InputControl();
	int InitInput(WindowControl *WindowCtrl);
	void DestroyInput();
	void SetMouseLimitFlag(bool flag);
	void GetInputState(bool mousemode);
	void MoveMouseCenter();
	bool CheckKeyNow(int id);
	bool CheckKeyDown(int id);
	bool CheckKeyUp(int id);
	void GetMouseMovement(int *x, int *y);
	bool CheckMouseButtonNowL();
	bool CheckMouseButtonDownL();
	bool CheckMouseButtonUpL();
	bool CheckMouseButtonNowR();
	bool CheckMouseButtonDownR();
	bool CheckMouseButtonUpR();
	int CheckInputAll();
};

int OriginalkeycodeToDinputdef(int code);
bool GetDoubleKeyCode(int id, int *CodeL, int *CodeR);
int GetEscKeycode();
int GetHomeKeycode();
int GetFunctionKeycode(int key);

#endif