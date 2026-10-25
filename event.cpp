//! @file event.cpp
//! @brief EventControl class definitions.

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

#include "event.h"

//! @brief Constructor
EventControl::EventControl(PointDataInterface *in_Point, ObjectManager *in_ObjMgr)
{
	Point = in_Point;
	ObjMgr = in_ObjMgr;
	nextp4 = 0;
	waitcnt = 0;
}

//! @brief Destructor
EventControl::~EventControl()
{}

//! @brief Set target class
//! @param in_Point Point Data Management Class
//! @param in_ObjMgr Object Management Class
//! @attention If you do not set this function, the class itself will not function correctly.
void EventControl::SetClass(PointDataInterface *in_Point, ObjectManager *in_ObjMgr)
{
	Point = in_Point;
	ObjMgr = in_ObjMgr;
}

//! @brief Reset.
//! @param EntryP4 Identification number to start event processing (-100, -110, -120, etc.)
void EventControl::Reset(signed short int EntryP4)
{
	nextp4 = EntryP4;
	waitcnt = 0;
}

//! @brief Next Identification Number
//! @return P4: Identification Number
signed short int EventControl::GetNextP4()
{
	return nextp4;
}

//! @brief Determination of death of the target person
//! @param targetP4 Target identification number
//! @param reverse Conditional reversal (survival deter )
//! @return true: Go to next event false: Don't go next
//! @attention If the target person does not exist, it will be false.
bool EventControl::CheckDead(signed short int targetP4, bool reverse)
{
	human* thuman;

	thuman = ObjMgr->SearchHuman(targetP4);
	if( thuman == NULL ){ return false; }

	if( reverse == false ){
		if( thuman->GetDeadFlag() == false ){ return false; }
	}
	else{
		if( thuman->GetDeadFlag() == true ){ return false; }
	}
	return true;
}

//! @brief Determining the arrival of the target person
//! @param pos_x X Coordinate to judge
//! @param pos_y Determine Y coordinates
//! @param pos_z Z coordinates to determine
//! @param dist_r Distance to determine (radius)
//! @param targetP4 Target identification number
//! @param reverse Conditional Inverted (Undetermined)
//! @return true: Go to next event false: Don't go next
//! @attention If the target person does not exist, it will be false.
bool EventControl::CheckArrival(float pos_x, float pos_y, float pos_z, float dist_r, signed short int targetP4, bool reverse)
{
	human* thuman;
	float hx, hy, hz;
	float x, y, z;

	thuman = ObjMgr->SearchHuman(targetP4);
	if( thuman == NULL ){ return false; }
	thuman->GetPosData(&hx, &hy, &hz, NULL);
	x = hx - pos_x;
	y = hy - pos_y;
	z = hz - pos_z;

	if( reverse == false ){
		if( sqrtf(x*x + y*y+ + z*z) > dist_r ){ return false; }
	}
	else{
		if( sqrtf(x*x + y*y+ + z*z) < dist_r ){ return false; }
	}
	return true;
}

//! @brief Check whether the target character owns the specified weapon.
//! @param WeaponID Type number of weapons
//! @param targetP4 Target identification number
//! @param reverse Invert the condition to test for non-ownership.
//! @return true: Go to next event false: Don't go next
//! @attention If the target person does not exist, it will be false.
bool EventControl::CheckHaveWeapon(int WeaponID, signed short int targetP4, bool reverse)
{
	human* thuman;
	int selectweapon;
	weapon *out_weapon[TOTAL_HAVEWEAPON];
	int now_weaponid;
	bool flag = false;			//false: un, true: ownership

	thuman = ObjMgr->SearchHuman(targetP4);
	if( thuman == NULL ){ return false; }

	//Get all of your weapon objects
	thuman->GetWeapon(&selectweapon, out_weapon, NULL, NULL);

	//Search All
	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		if( out_weapon[i] != NULL ){

			//the weapon type number and whether it is the target weapon.
			out_weapon[i]->GetParamData(&now_weaponid, NULL, NULL);
			if( now_weaponid == WeaponID ){
				flag = true;
			}

		}
	}

	if( reverse == false ){
		if( flag == false ){ return false; }
	}
	else{
		if( flag == true ){ return false; }
	}
	return true;
}

//! @brief Move mode for target travel path
//! @param targetP4 Target identification number
//! @param newmode Moving mode
//! @return success: true failure: false
bool EventControl::SetMovePathMode(signed short int targetP4, int newmode)
{
	pointdata pathdata;
	int pid;

	if( Point->SearchPointdata(&pid, 0x08, 0, 0, 0, targetP4, 0) > 0 ){

		//If the target is AI path, force the parameter to be rewritten
		Point->Getdata(&pathdata, pid);
		if( (pathdata.p1 == POINT_P1TYPE_AIPATH)||(pathdata.p1 == POINT_P1TYPE_RAND_AIPATH) ){
			pathdata.p2 = (signed short int)newmode;
			Point->SetParam(pid, pathdata.p1, pathdata.p2, pathdata.p3, pathdata.p4);
			return true;
		}

	}
	return false;
}

//! @brief Change the team number of the target person
//! @param targetP4 Target identification number
//! @param TeamID Team number
//! @return success: true failure: false
bool EventControl::SetTeamID(signed short int targetP4, int TeamID)
{
	human* thuman;

	thuman = ObjMgr->SearchHuman(targetP4);
	if( thuman == NULL ){ return false; }

	thuman->SetTeamID(TeamID);
	return true;
}

//! @brief Check whether the target small object has been destroyed.
//! @param targetP4 Target identification number
//! @param reverse Conditional inverted (undestructive deter )
//! @return true: Go to next event false: Don't go next
//! @attention If there is no target accessory, it will be processed as destroyed.
bool EventControl::CheckBreakSmallObject(signed short int targetP4, bool reverse)
{
	smallobject* tsmallobject;
	bool flag = true;			//false: Undestructed, true: Undestructed or not

	tsmallobject = ObjMgr->SearchSmallobject(targetP4);
	if( tsmallobject != NULL ){
		if( tsmallobject->GetEnableFlag() == true ){ flag = false; }
	}

	if( reverse == false ){
		if( flag == false ){ return false; }
	}
	else{
		if( flag == true ){ return false; }
	}
	return true;
}

//! @brief Time wait (in seconds)
//! @param sec sec.
//! @return true: Go to next event false: Don't go next
bool EventControl::WaitSec(int sec)
{
	if( ((int)GAMEFPS * sec) > waitcnt ){
		waitcnt += 1;
		return false;
	}
	waitcnt = 0;
	return true;
}

//! @brief Message Display
//! @param SetID Message Number
//! @param MessageID MessageID pointer
//! @param SetMessageID SetMessageID pointer
//! @attention Even if the message number is specified outside the range, the new message will be processed as specified.
void EventControl::SetMessage(int SetID, int *MessageID, bool *SetMessageID)
{
	if( (0 <= SetID)&&(SetID < MAX_POINTMESSAGES) ){
		*MessageID = SetID;
	}
	*SetMessageID = true;
}

//! @brief Execute processing.
//! @param SkipFlag Skip flag for event tasks
//! @param Param Pointer of structure that receives event parameters
//! @return Number of steps executed.
//! @attention If there is no change in the passed parameters (structural), the passed parameters will not be operated.
//! @attention SetMessageID of the structure is true when a message event is executed. <b>It is not changed from the previous time.</b>
int EventControl::ProcessEventPoint(bool SkipFlag, EventParameter *Param)
{
	if( Param == NULL ){ return 0; }

	pointdata data;
	int cnt = 0;

	for(int i=0; i<TOTAL_EVENTFRAMESTEP; i++){
		cnt = i;

		//Find the following points
		if( Point->SearchPointdata(&data, 0x08, 0, 0, 0, nextp4, 0) == 0 ){ return cnt; }

		//If the number is invalid, it will not be processed.
		if( (data.p1 < 10)||(19 < data.p1) ){
#ifdef ENABLE_CHECKOPENXOPSEVENT
			if( data.p1 != 29 ){ return cnt; }
#else
			return cnt;
#endif
		}

		//If the skip flag is set on the first iteration, proceed to the next event.
		if( (cnt == 0)&&(SkipFlag == true) ){
			nextp4 = data.p3;
			continue;
		}

		switch(data.p1){
			case 10:	//Achieving mission
				Param->endcnt += 1;
				Param->complete = true;
				return cnt;

			case 11:	//Failure
				Param->endcnt += 1;
				Param->complete = false;
				return cnt;

			case 12:	//Death Waiting
				if( CheckDead(data.p2, false) == false ){ return cnt; }
				nextp4 = data.p3;
				break;

			case 13:	//Wait for arrival
				if( CheckArrival(data.x, data.y, data.z, DISTANCE_CHECKPOINT, data.p2, false) == false ){ return cnt; }
				nextp4 = data.p3;
				break;

			case 14:	//Change to walk
				SetMovePathMode(data.p2, 0);
				nextp4 = data.p3;
				break;

			case 15:	//Waiting for Destruction
				if( CheckBreakSmallObject(data.p2, false) == false ){ return cnt; }
				nextp4 = data.p3;
				break;

			case 16:	//Waiting for Case
				if( CheckArrival(data.x, data.y, data.z, DISTANCE_CHECKPOINT, data.p2, false) == false ){ return cnt; }
				if( CheckHaveWeapon(ID_WEAPON_CASE, data.p2, false) == false ){ return cnt; }
				nextp4 = data.p3;
				break;

			case 17:	//Waiting Time
				if( WaitSec(PointParamToInt(data.p2)) == false ){ return cnt; }
				nextp4 = data.p3;
				break;

			case 18:	//Message
				SetMessage(PointParamToInt(data.p2), &(Param->MessageID), &(Param->SetMessageID));
				nextp4 = data.p3;
				break;

			case 19:	//Team Changes
				SetTeamID(data.p2, 0);
				nextp4 = data.p3;
				break;

#ifdef ENABLE_CHECKOPENXOPSEVENT
			case 29:	//OpenXOPS
				if( data.p2 != 0 ){ return cnt; }
				nextp4 = data.p3;
				break;
#endif

			//If you want to add a new event point, you can find it here.
			// * Competition of type number strictly prohibited
			// * Rewrite condition judgment (range check) just before this switch statement.
		}
	}
	return cnt;
}