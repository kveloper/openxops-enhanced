//! @file statemachine.h
//! @brief StateMachine Class Declaration

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

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 3		//!< Select include file.
#endif
#include "main.h"

//! Constants that represent the state
enum State
{
	STATE_NULL = 0,			//!< [error] Invalid state
	STATE_CREATE_OPENING,	//!< Opening screen Initialization and preparation
	STATE_NOW_OPENING,		//!< Opening screen running
	STATE_DESTROY_OPENING,	//!< Opening Screen End/Disposal
	STATE_CREATE_MENU,		//!< Initialization and preparation of menu screen
	STATE_NOW_MENU,			//!< Menu screen running
	STATE_DESTROY_MENU,		//!< Menu screen End/disposal
	STATE_CREATE_BRIEFING,	//!< Briefing screen Initialization and preparation
	STATE_NOW_BRIEFING,		//!< Briefing screen running
	STATE_DESTROY_BRIEFING,	//!< Briefing screen End/disposal
	STATE_CREATE_MAINGAME,	//!< Main screen Initialization and preparation
	STATE_NOW_MAINGAME,		//!< Main screen running
	STATE_DESTROY_MAINGAME,	//!< Main screen End/Disposal
	STATE_CREATE_RESULT,	//!< Result display screen Initialization and preparation
	STATE_NOW_RESULT,		//!< Result display screen running
	STATE_DESTROY_RESULT,	//!< Result display screen End/disposal
	STATE_EXIT				//!< End of game
};

//! @brief State transition class
//! @details Manage the entire game screen transition.
class StateMachine
{
	int StateMode;	//!< State Transition Mode
	int NowState;	//!< Current state
	bool back;		//!< ESC key is pressed
	bool f12;		//!< F12 key pressed

public:
	StateMachine();
	~StateMachine();
	void SetStateMode(int mode);
	void NextState();
	void PushMouseButton();
	void PushBackSpaceKey();
	void PushF12Key();
	int GetState();
};

#endif