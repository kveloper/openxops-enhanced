//! @file event.h
//! @brief Declaration of EventControl class

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

#ifndef EVENT_H
#define EVENT_H

#define TOTAL_EVENTFRAMESTEP 6			//!< Number of steps per line in one frame
#define DISTANCE_CHECKPOINT 25.0f		//!< Determination distance of waiting for arrival and waiting for case

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 3		//!< Select include file.
#endif
#include "main.h"

//! Structure for Event Parameters
typedef struct
{
	int endcnt;				//!< End Count
	bool complete;			//!< Mission success flag
	int MessageID;			//!< Event Message Number
	bool SetMessageID;		//!< Event message flag
} EventParameter;

//! @brief Class that handles events
//! @details Process mission event points.
//! @details Each instance handles one event sequence. Create separate instances to process multiple sequences in parallel; for example, use three instances for three concurrent sequences.
class EventControl
{
	class PointDataInterface *Point;		//!< PointDataInterface class pointer
	class ObjectManager *ObjMgr;			//!< Pointer to the ObjectManager class.
	signed short int nextp4;				//!< Next P4: Identification Number
	int waitcnt;							//!< Time wait count

	bool CheckDead(signed short int targetP4, bool reverse);
	bool CheckArrival(float pos_x, float pos_y, float pos_z, float dist_r, signed short int targetP4, bool reverse);
	bool CheckHaveWeapon(int WeaponID, signed short int targetP4, bool reverse);
	bool SetMovePathMode(signed short int targetP4, int newmode);
	bool SetTeamID(signed short int targetP4, int TeamID);
	bool CheckBreakSmallObject(signed short int targetP4, bool reverse);
	bool WaitSec(int sec);
	void SetMessage(int SetID, int *MessageID, bool *SetMessageID);

public:
	EventControl(PointDataInterface *in_Point = NULL, ObjectManager *in_ObjMgr = NULL);
	~EventControl();
	void SetClass(PointDataInterface *in_Point, ObjectManager *in_ObjMgr);
	void Reset(signed short int EntryP4);
	signed short int GetNextP4();
	int ProcessEventPoint(bool SkipFlag, EventParameter *Param);
};

#endif