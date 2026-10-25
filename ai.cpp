//! @file ai.cpp
//! @brief Definition of AIcontrol class

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

#include "ai.h"

//! @brief Constructor
AIcontrol::AIcontrol(class ObjectManager *in_ObjMgr, int in_ctrlid, class BlockDataInterface *in_blocks, class PointDataInterface *in_Points, class ParameterInfo *in_Param, class Collision *in_CollD, class SoundManager *in_GameSound)
{
	ObjMgr = in_ObjMgr;
	ctrlid = in_ctrlid;
	if( in_ObjMgr != NULL ){ ctrlhuman = in_ObjMgr->GetHumanObject(in_ctrlid); }
	else{ ctrlhuman = NULL; }
	enemyhuman = NULL;
	blocks = in_blocks;
	Param = in_Param;
	CollD = in_CollD;
	GameSound = in_GameSound;

	AIlevel = 0;
	battlemode = AI_NORMAL;
	NoFight = false;
	posx = 0.0f;
	posy = 0.0f;
	posz = 0.0f;
	rx = 0.0f;
	ry = 0.0f;
	cautionback_posx = 0.0f;
	cautionback_posz = 0.0f;
	FaceCaution_flag = false;
	FaceCaution_rx = 0.0f;
	total_move = 0.0f;
	waitcnt = 0;
	movejumpcnt = 1*((int)GAMEFPS);
	gotocnt = 0;
	cautioncnt = 0;
	actioncnt = 0;
	longattack = false;
	EventWeaponShot = false;
	LevelParam = NULL;

	MoveNavi = new AIMoveNavi;
	ObjDriver = new AIObjectDriver;
	MoveNavi->SetClass(in_ObjMgr, in_ctrlid, in_Points);
	ObjDriver->SetClass(in_ObjMgr, in_ctrlid);
}

//! @brief Destructor
AIcontrol::~AIcontrol()
{
	delete MoveNavi;
	delete ObjDriver;
}

//! @brief Set target class
//! @attention If you do not set this function, the class itself will not function correctly.
void AIcontrol::SetClass(class ObjectManager *in_ObjMgr, int in_ctrlid, class BlockDataInterface *in_blocks, class PointDataInterface *in_Points, class ParameterInfo *in_Param, class Collision *in_CollD, class SoundManager *in_GameSound)
{
	ObjMgr = in_ObjMgr;
	ctrlid = in_ctrlid;
	ctrlhuman = in_ObjMgr->GetHumanObject(in_ctrlid);
	blocks = in_blocks;
	Param = in_Param;
	CollD = in_CollD;
	GameSound = in_GameSound;

	MoveNavi->SetClass(in_ObjMgr, in_ctrlid, in_Points);
	ObjDriver->SetClass(in_ObjMgr, in_ctrlid);
}

//! @brief Check if you are moving to the target location
//! @return Reached: true; not reached: false.
bool AIcontrol::CheckTargetPos(bool back)
{
	float target_posx, target_posz;
	int movemode;
	if( back == false ){
		MoveNavi->GetTargetPos(&target_posx, &target_posz, NULL, &movemode, NULL);
	}
	else{
		target_posx = cautionback_posx;
		target_posz = cautionback_posz;
		movemode = AI_NAVI_MOVE_RUN;
	}

	//Calculate the distance
	float x = posx - target_posx;
	float z = posz - target_posz;
	float r = x * x + z * z;

	if( movemode == AI_NAVI_MOVE_TRACKING ){	//If tracking a target.
		if( r < AI_ARRIVALDIST_TRACKING * AI_ARRIVALDIST_TRACKING ){
			return true;
		}
	}
	else{							//Otherwise.
		if( r < (AI_ARRIVALDIST_PATH * AI_ARRIVALDIST_PATH) ){
			return true;
		}
	}

	return false;
}

//! @brief Go to the target location
void AIcontrol::MoveTarget(bool back)
{
	float r, atan;
	int paramid;
	HumanParameter Paraminfo;
	bool zombie;

	float target_posx, target_posz;	
	int movemode;
	if( back == false ){
		MoveNavi->GetTargetPos(&target_posx, &target_posz, NULL, &movemode, NULL);
	}
	else{
		target_posx = cautionback_posx;
		target_posz = cautionback_posz;
		movemode = AI_NAVI_MOVE_RUN;
	}

	//Check whether this is a zombie.
	ctrlhuman->GetParamData(&paramid, NULL, NULL, NULL);
	Param->GetHuman(paramid, &Paraminfo);
	if( Paraminfo.type == 2 ){
		zombie = true;
	}
	else{
		zombie = false;
	}

	//Stop all movements once
	ObjDriver->ResetMode();

	//Determine the angle to the target point
	CheckTargetAngle(posx, 0.0f, posz, rx*-1 + (float)M_PI/2, 0.0f, target_posx, 0.0f, target_posz, 0.0f, &atan, NULL, &r);

	//Turn
	if( atan > DegreeToRadian(0.5f) ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT);
	}
	if( atan < DegreeToRadian(-0.5f) ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT);
	}

	//Move forward.
	if( zombie == true ){
		if( fabsf(atan) < DegreeToRadian(20) ){
			ObjDriver->SetModeFlag(AI_CTRL_MOVEWALK);
		}
	}
	else if( movemode == AI_NAVI_MOVE_RUN ){
		if( fabsf(atan) < DegreeToRadian(50) ){
			ObjDriver->SetModeFlag(AI_CTRL_MOVEFORWARD);
		}
	}
	else if( movemode == AI_NAVI_MOVE_RUN2 ){
		if( fabsf(atan) < DegreeToRadian(50) ){
			ObjDriver->SetModeFlag(AI_CTRL_MOVEFORWARD);
		}
	}
	else if( movemode == AI_NAVI_MOVE_WALK ){
		if( fabsf(atan) < DegreeToRadian(6) ){
			ObjDriver->SetModeFlag(AI_CTRL_MOVEWALK);
		}
	}
	else if( movemode == AI_NAVI_MOVE_TRACKING ){
		if( fabsf(atan) < DegreeToRadian(20) ){
			if( r < (AI_ARRIVALDIST_WALKTRACKING * AI_ARRIVALDIST_WALKTRACKING) ){
				ObjDriver->SetModeFlag(AI_CTRL_MOVEWALK);
			}
			else{
				ObjDriver->SetModeFlag(AI_CTRL_MOVEFORWARD);
			}
		}
	}

	//Jumping
	if( GetRand(16) == 0 ){
		MoveJump();
	}

	//If stuck, turn left or right randomly.
	if( GetRand(28) == 0 ){
		if( ctrlhuman->GetMovemode(true) != 0 ){
			if( ctrlhuman->GetTotalMove() - total_move < 0.1f ){
				if( GetRand(2) == 0 ){ ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT); }
				else{ ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT); }
			}
		}
	}
	total_move = ctrlhuman->GetTotalMove();
}

//! @brief Move to the target point (for priority driving)
void AIcontrol::MoveTarget2(bool back)
{
	float atan;

	float target_posx, target_posz;	
	if( back == false ){
		MoveNavi->GetTargetPos(&target_posx, &target_posz, NULL, NULL, NULL);
	}
	else{
		target_posx = cautionback_posx;
		target_posz = cautionback_posz;
	}

	//Determine the angle to the target point
	CheckTargetAngle(posx, 0.0f, posz, rx*-1 + (float)M_PI/2, 0.0f, target_posx, 0.0f, target_posz, 0.0f, &atan, NULL, NULL);

	//Front and rear movement processing
	if( fabsf(atan) < DegreeToRadian(56) ){
		ObjDriver->SetModeFlag(AI_CTRL_MOVEFORWARD);
	}
	if( fabsf(atan) > DegreeToRadian(123.5f) ){
		ObjDriver->SetModeFlag(AI_CTRL_MOVEBACKWARD);
	}

	//Processing left and right movement
	if( (DegreeToRadian(-146) < atan)&&(atan < DegreeToRadian(-33)) ){
		ObjDriver->SetModeFlag(AI_CTRL_MOVERIGHT);
	}
	if( (DegreeToRadian(33) < atan)&&(atan < DegreeToRadian(146)) ){
		ObjDriver->SetModeFlag(AI_CTRL_MOVELEFT);
	}

	//Jumping
	if( GetRand(16) == 0 ){
		MoveJump();
	}

	//If stuck, turn left or right randomly.
	if( GetRand(28) == 0 ){
		if( ctrlhuman->GetMovemode(true) != 0 ){
			if( ctrlhuman->GetTotalMove() - total_move < 0.1f ){
				if( GetRand(2) == 0 ){ ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT); }
				else{ ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT); }
			}
		}
	}
	total_move = ctrlhuman->GetTotalMove();
}

//! @brief Move back and forth and right randomly (for attacking)
void AIcontrol::MoveRandom()
{
	int forwardstart, backstart, sidestart;

	if( longattack == false ){
		forwardstart = 80;
		backstart = 90;
		sidestart = 70;
	}
	else{
		forwardstart = 120;
		backstart = 150;
		sidestart = 130;
	}

	//Start moving randomly
	if( GetRand(forwardstart) == 0 ){
		ObjDriver->SetModeFlag(AI_CTRL_MOVEFORWARD);
	}
	if( GetRand(backstart) == 0 ){
		ObjDriver->SetModeFlag(AI_CTRL_MOVEBACKWARD);
	}
	if( GetRand(sidestart) == 0 ){
		ObjDriver->SetModeFlag(AI_CTRL_MOVELEFT);
	}
	if( GetRand(sidestart) == 0 ){
		ObjDriver->SetModeFlag(AI_CTRL_MOVERIGHT);
	}

	//If you have a weapon,
	if( ctrlhuman->GetMainWeaponTypeNO() == ID_WEAPON_NONE ){
		// Start descending with 1/80 probability
		if( GetRand(80) == 0 ){
			ObjDriver->SetModeFlag(AI_CTRL_MOVEBACKWARD);
		}
	}

	// 1/3 probability or move flags are set
	if( (GetRand(3) == 0)||(ObjDriver->GetModeFlag(AI_CTRL_MOVEFORWARD | AI_CTRL_MOVEBACKWARD | AI_CTRL_MOVELEFT | AI_CTRL_MOVERIGHT)) ){
		float vx, vz;
		float Dist;

		if( GetRand(2) == 0 ){
			//Calculate the forward vector.
			vx = cosf(rx*-1 + (float)M_PI/2);
			vz = sinf(rx*-1 + (float)M_PI/2);
			if(
				(CollD->CheckALLBlockIntersectDummyRay(posx, posy + HUMAN_MAPCOLLISION_HEIGHT, posz, vx, 0, vz, NULL, NULL, &Dist, HUMAN_MAPCOLLISION_R) == true)||		//There is a block at the height of the waist (bug)
				(CollD->CheckALLBlockIntersectDummyRay(posx, posy - 1.0f, posz, vx, 0, vz, NULL, NULL, &Dist, HUMAN_MAPCOLLISION_R) == false)							//There is no block on the foot (fall)
			){
				//Clear the forward flag and set the backward flag.
				ObjDriver->DelModeFlag(AI_CTRL_MOVEFORWARD);
				ObjDriver->SetModeFlag(AI_CTRL_MOVEBACKWARD);
			}

			//Backward
			vx = cosf(rx*-1 + (float)M_PI/2 + (float)M_PI);
			vz = sinf(rx*-1 + (float)M_PI/2 + (float)M_PI);
			if(
				(CollD->CheckALLBlockIntersectDummyRay(posx, posy + HUMAN_MAPCOLLISION_HEIGHT, posz, vx, 0, vz, NULL, NULL, &Dist, HUMAN_MAPCOLLISION_R) == true)||		//There is a block at the height of the waist (bug)
				(CollD->CheckALLBlockIntersectDummyRay(posx, posy - 1.0f, posz, vx, 0, vz, NULL, NULL, &Dist, HUMAN_MAPCOLLISION_R) == false)							//There is no block on the foot (fall)
			){
				//Clear the backward flag and set the forward flag.
				ObjDriver->DelModeFlag(AI_CTRL_MOVEBACKWARD);
				ObjDriver->SetModeFlag(AI_CTRL_MOVEFORWARD);
			}
		}
		else{
			//
			vx = cosf(rx*-1);
			vz = sinf(rx*-1);
			if(
				(CollD->CheckALLBlockIntersectDummyRay(posx, posy + HUMAN_MAPCOLLISION_HEIGHT, posz, vx, 0, vz, NULL, NULL, &Dist, HUMAN_MAPCOLLISION_R) == true)||		//There is a block at the height of the waist (bug)
				(CollD->CheckALLBlockIntersectDummyRay(posx, posy - 1.0f, posz, vx, 0, vz, NULL, NULL, &Dist, HUMAN_MAPCOLLISION_R) == false)							//There is no block on the foot (fall)
			){
				//Delete the right move flag and set the left move flag
				ObjDriver->DelModeFlag(AI_CTRL_MOVERIGHT);
				ObjDriver->SetModeFlag(AI_CTRL_MOVELEFT);
			}

			vx = cosf(rx*-1 + (float)M_PI);
			vz = sinf(rx*-1 + (float)M_PI);
			if(
				(CollD->CheckALLBlockIntersectDummyRay(posx, posy + HUMAN_MAPCOLLISION_HEIGHT, posz, vx, 0, vz, NULL, NULL, &Dist, HUMAN_MAPCOLLISION_R) == true)||		//There is a block at the height of the waist (bug)
				(CollD->CheckALLBlockIntersectDummyRay(posx, posy - 1.0f, posz, vx, 0, vz, NULL, NULL, &Dist, HUMAN_MAPCOLLISION_R) == false)							//There is no block on the foot (fall)
			){
				//Delete the left move flag and set the right move flag
				ObjDriver->DelModeFlag(AI_CTRL_MOVELEFT);
				ObjDriver->SetModeFlag(AI_CTRL_MOVERIGHT);
			}
		}
	}

	//If there is an attack target
	if( enemyhuman != NULL ){
		float tx, ty, tz;

		enemyhuman->GetPosData(&tx, &ty, &tz, NULL);

		float x = posx - tx;
		float y = posy - ty;
		float z = posz - tz;
		float r = x * x + y * y + z * z;

		//If the enemy is too tight, re
		if( r < 20.0f * 20.0f ){
			ObjDriver->DelModeFlag(AI_CTRL_MOVEFORWARD);
			if( GetRand(70) == 0 ){
				ObjDriver->SetModeFlag(AI_CTRL_MOVEBACKWARD);
			}
		}
	}
}

//! @brief Look around from the current position.
void AIcontrol::TurnSeen()
{
	int turnstart, turnstop;

	float target_rx;
	int pointmode;
	MoveNavi->GetTargetPos(NULL, NULL, &target_rx, NULL, &pointmode);

	//If you are wary of shooting, head for the shot direction
	if( (battlemode == AI_CAUTION)&&(FaceCaution_flag == true) ){
		float tr;

		//Calculate the direction.
		tr = FaceCaution_rx - rx;
		tr = AngleNormalization(tr);

		//Turn
		if( tr > DegreeToRadian(2.5f) ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT);
		}
		if( tr < DegreeToRadian(-2.5f) ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT);
		}

		//Move to a normal warning if you finish facing a specific direction
		if( fabsf(tr) <= DegreeToRadian(2.5f) ){
			FaceCaution_flag = false;
		}

		return;
	}

	//Set the start and end probability of rotation
	if( battlemode == AI_ACTION ){
		return;
	}
	else if( battlemode == AI_CAUTION ){
		turnstart = 20;
		turnstop = 20;
	}
	else{
		if( pointmode == AI_NAVI_POINT_TRACKING ){ turnstart = 65; }
		else{ turnstart = 85; }
		turnstop = 18;
	}

	//Start spinning randomly
	if( GetRand(turnstart) == 0 ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT);
	}
	if( GetRand(turnstart) == 0 ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT);
	}

	if( (battlemode == AI_NORMAL)&&(pointmode == AI_NAVI_POINT_WAIT) ){
		//Beyond the Point randomly
		//Reproduction of "A little emphasis on the direction of the point"
		if( GetRand(80) == 0 ){
			float tr;
			tr = target_rx - rx;
			tr = AngleNormalization(tr);

			if( tr > 0.0f ){
				ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT);
			}
			if( tr < 0.0f ){
				ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT);
			}
		}
	}

	//Stop rotation randomly
	if( GetRand(turnstop) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_TURNRIGHT);
	}
	if( GetRand(turnstop) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_TURNLEFT);
	}
}

//! @brief Continue to look at a specific direction
bool AIcontrol::StopSeen()
{
	float tr;
	bool returnflag = false;

	float target_rx;
	MoveNavi->GetTargetPos(NULL, NULL, &target_rx, NULL, NULL);

	tr = target_rx - rx;
	tr = AngleNormalization(tr);

	ObjDriver->DelModeFlag(AI_CTRL_TURNRIGHT);
	ObjDriver->DelModeFlag(AI_CTRL_TURNLEFT);

	//Turn
	if( tr > DegreeToRadian(2.5f) ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT);
		returnflag = false;
	}
	if( tr < DegreeToRadian(-2.5f) ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT);
		returnflag = false;
	}

	//Determines whether you are going to a specific direction
	if( fabsf(tr) <= DegreeToRadian(2.5f) ){
		returnflag = true;
	}

	return returnflag;
}

//! @brief Jump if there is a obstacle in the direction of progress
bool AIcontrol::MoveJump()
{
	//Don't handle if you are standing
	// The movement flags in AIObjectDriver (ObjDriver) may also be used for this check.
	if( ctrlhuman->GetMovemode(false) == 0 ){ return false; }

	float dist_dummy;

	float check_rx = 0.0f;
	float new_posx, new_posy, new_posz;

	//Calculate the direction of travel.
	if( ObjDriver->GetModeFlag(AI_CTRL_MOVEWALK) == true ){
		check_rx = DegreeToRadian(0.0f);
	}
	if( ObjDriver->GetModeFlag(AI_CTRL_MOVEFORWARD) == true ){
		check_rx = DegreeToRadian(0.0f);
	}
	if( ObjDriver->GetModeFlag(AI_CTRL_MOVEBACKWARD) == true ){
		check_rx = DegreeToRadian(180.0f);
	}
	if( ObjDriver->GetModeFlag(AI_CTRL_MOVELEFT) == true ){
		check_rx = DegreeToRadian(90.0f);
	}
	if( ObjDriver->GetModeFlag(AI_CTRL_MOVERIGHT) == true ){
		check_rx = DegreeToRadian(-90.0f);
	}

	//Check collisions near the waist.
	new_posx = posx + cosf(rx*-1 + (float)M_PI/2 + check_rx) * (AI_CHECKJUMP_DIST + HUMAN_MAPCOLLISION_R);
	new_posy = posy + HUMAN_MAPCOLLISION_HEIGHT;
	new_posz = posz + sinf(rx*-1 + (float)M_PI/2 + check_rx) * (AI_CHECKJUMP_DIST + HUMAN_MAPCOLLISION_R);
	if( CollD->CheckALLBlockInside(new_posx, new_posy, new_posz) == true ){
		ObjMgr->MoveJump(ctrlid);
		return true;
	}

	//Check collisions for the entire body.
	new_posx = posx + cosf(rx*-1 + (float)M_PI/2 + check_rx) * AI_CHECKJUMP_DIST;
	new_posy = posy + AI_CHECKJUMP_HEIGHT;
	new_posz = posz + sinf(rx*-1 + (float)M_PI/2 + check_rx) * AI_CHECKJUMP_DIST;
	if( CollD->CheckALLBlockInside(new_posx, new_posy, new_posz) == true ){
		ObjMgr->MoveJump(ctrlid);
		return true;
	}
	else if( CollD->CheckALLBlockIntersectRay(new_posx, new_posy, new_posz, 0.0f, 1.0f, 0.0f, NULL, NULL, &dist_dummy, HUMAN_HEIGHT - AI_CHECKJUMP_HEIGHT) == true ){
		ObjMgr->MoveJump(ctrlid);
		return true;
	}

	return false;
}

//! @brief Attack
//! @todo Implement the zombie's grab attack.
void AIcontrol::Action()
{
	human* EnemyHuman = NULL;
	int paramid;
	HumanParameter Paraminfo;
	bool zombie;
	float posy2;
	float tx, ty, tz;

	EnemyHuman = enemyhuman;

	//Get coordinates
	EnemyHuman->GetPosData(&tx, &ty, &tz, NULL);
	posy2 = posy + VIEW_HEIGHT;
	ty += VIEW_HEIGHT;

	//Check whether this is a zombie.
	ctrlhuman->GetParamData(&paramid, NULL, NULL, NULL);
	Param->GetHuman(paramid, &Paraminfo);
	if( Paraminfo.type == 2 ){
		zombie = true;
	}
	else{
		zombie = false;
	}

	//Get the type of weapon you have
	int weaponid = ctrlhuman->GetMainWeaponTypeNO();

	float atanx, atany, r;

	//Corrects the target position
	if( zombie == true ){		//For zombies.
		float x = posx - tx;
		float z = posz - tz;
		float r = sqrtf(x * x + z * z);
		float scale;
		float mx, mz;

		if( r > 180.0f ){ r = 180.0f; }		//1800.0f max

		//Get the following parameters
		switch( Paraminfo.AItrackability ){
			case 0: scale = (0.05f * 2); break;
			case 1: scale = (0.10f * 2); break;
			case 2: scale = (0.13f * 2); break;
			case 3: scale = (0.17f * 2); break;
			default: scale = 0.0f;
		}

		//Anticipate the enemy's movement.
		EnemyHuman->GetMovePos(&mx, NULL, &mz);
		tx += mx * r * scale;
		tz += mz * r * scale;
	}
	else{		//For non-zombies.
		//If you have a grenamel
		if( weaponid == ID_WEAPON_GRENADE ){
			if( zombie == false ){
				float x = posx - tx;
				float z = posz - tz;
				float r = sqrtf(x * x + z * z);
				float scale;
				if( longattack == false ){ scale = 0.12f; }
				else{ scale = 0.4f; }

				//Change height depending on distance
				ty += (r - AI_CHECKSHORTATTACK_DIST) * scale;
			}
		}
		else{
			float mx, mz;
			float scale;
			EnemyHuman->GetMovePos(&mx, NULL, &mz);
			if( longattack == false ){ scale = 1.5f; }
			else{
				float x = posx - tx;
				float z = posz - tz;
				float r = sqrtf(x * x + z * z);

				scale = r * 0.12f;
			}

			//Anticipate the enemy's movement.
			tx += mx * scale;
			tz += mz * scale;
		}
	}

	//Determine the angle to the target point
	CheckTargetAngle(posx, posy2, posz, rx*-1 + (float)M_PI/2, ry, tx, ty, tz, 0.0f, &atanx, &atany, &r);

	//Time or decision to change orientation
	int randr = LevelParam->aiming;
	if( longattack == false ){ randr += 1; }
	else{ randr += 2; }

	if( randr != 0 ){
		//Turn
		if( atanx > 0.0f ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT);
			ObjDriver->DelModeFlag(AI_CTRL_TURNRIGHT);
		}
		if( atanx < 0.0f ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT);
			ObjDriver->DelModeFlag(AI_CTRL_TURNLEFT);
		}

		//Arm Angle
		if( zombie == true ){
			//ry = AI_ZOMBIEATTACK_ARMRY;

			//Turn
			if( ry < AI_ZOMBIEATTACK_ARMRY ){
				ObjDriver->SetModeFlag(AI_CTRL_TURNUP);
				ObjDriver->DelModeFlag(AI_CTRL_TURNDOWN);
			}
			if( ry > AI_ZOMBIEATTACK_ARMRY ){
				ObjDriver->SetModeFlag(AI_CTRL_TURNDOWN);
				ObjDriver->DelModeFlag(AI_CTRL_TURNUP);
			}
		}
		else{
			//If unarmed.
			if( weaponid == ID_WEAPON_NONE ){
				if( EnemyHuman->GetMainWeaponTypeNO() == ID_WEAPON_NONE ){	//If the enemy is also unarmed.
					//Keep aiming downward.
					ObjDriver->SetModeFlag(AI_CTRL_TURNDOWN);
					ObjDriver->DelModeFlag(AI_CTRL_TURNUP);
				}
				else{														//If the enemy has a weapon ~
					//Keep aiming upward.
					ObjDriver->SetModeFlag(AI_CTRL_TURNUP);
					ObjDriver->DelModeFlag(AI_CTRL_TURNDOWN);
				}
			}
			else{
				//Turn
				if( atany > 0.0f ){
					ObjDriver->SetModeFlag(AI_CTRL_TURNUP);
					ObjDriver->DelModeFlag(AI_CTRL_TURNDOWN);
				}
				if( atany < 0.0f ){
					ObjDriver->SetModeFlag(AI_CTRL_TURNDOWN);
					ObjDriver->DelModeFlag(AI_CTRL_TURNUP);
				}
			}
		}
	}

	//If unarmed and not a zombie.
	if( zombie == false ){
		if( weaponid == ID_WEAPON_NONE ){
			//Move backward with a fixed probability.
			if( GetRand(80) == 0 ){
				ObjDriver->SetModeFlag(AI_CTRL_MOVEBACKWARD);
			}
		}
	}

	
	if( zombie == true ){	//Zombie attack.
		float y = posy2 - ty;

		//If you are running, switch to walking once
		if( ObjDriver->GetModeFlag(AI_CTRL_MOVEFORWARD) ){
			ObjDriver->DelModeFlag(AI_CTRL_MOVEFORWARD);
			ObjDriver->SetModeFlag(AI_CTRL_MOVEWALK);
		}

		//Going forward to the enemy
		if( fabsf(atanx) <= DegreeToRadian(25) ){
			if( (fabsf(atanx) <= DegreeToRadian(15)) && (r < 24.0f*24.0f) && (actioncnt%50 > 20) ){
				//Revoke walking and run
				ObjDriver->SetModeFlag(AI_CTRL_MOVEFORWARD);
				ObjDriver->DelModeFlag(AI_CTRL_MOVEWALK);
			}
			else{
				ObjDriver->SetModeFlag(AI_CTRL_MOVEWALK);
			}
		}

		/*
		//Jumping
		if( GetRand(16) == 0 ){
			MoveJump();
		}
		*/

		if( (r < 9.0f*9.0f)&&( fabsf(y) < 10.0f) ){
			float x = posx - tx;
			float z = posz - tz;

			//Capture (to attract enemies)
			EnemyHuman->AddPosOrder(atan2f(z, x), 0.0f, 0.5f);

			//Move the enemy's perspective randomly
			float erx, ery;
			EnemyHuman->GetRxRy(&erx, &ery);
			switch(GetRand(3)){
				case 0: erx -= DegreeToRadian(2); break;
				case 1: erx += DegreeToRadian(2); break;
				default: break;
			}
			switch(GetRand(3)){
				case 0: ery -= DegreeToRadian(2); break;
				case 1: ery += DegreeToRadian(2); break;
				default: break;
			}
			EnemyHuman->SetRxRy(erx, ery);
		}

		if( actioncnt%50 == 0){
			if( ObjMgr->CheckZombieAttack(ctrlhuman, EnemyHuman) == true ){
				ObjMgr->HitZombieAttack(ctrlhuman, EnemyHuman);
			}
		}
	}
	else{					//Fire.
		int scopemode = 0;
		ScopeParameter sparam;
		float ShotAngle;

		//Get weapon scope settings
		if( weaponid != ID_WEAPON_NONE ){
			WeaponParameter wparam;
			Param->GetWeapon(weaponid, &wparam);
			scopemode = wparam.scopemode;
		}

		if( longattack == false ){
			//Calculate the tolerance for determining whether the enemy is in the sights.
			Param->GetScopeParam(scopemode, &sparam);
			ShotAngle = sparam.AI_ShotAngle;

			//Adjust for each AI level.
			ShotAngle += DegreeToRadian(0.5f) * LevelParam->limitserror;

			if( MoveNavi->GetRun2() == true ){
				ShotAngle *= 1.5f;
			}
		}
		else{
			//Calculate the tolerance for determining whether the enemy is in the sights.
			Param->GetScopeParam(scopemode, &sparam);
			ShotAngle = sparam.AI_ShotAngleLong;

			//Adjust for each AI level.
			ShotAngle += DegreeToRadian(0.2f) * LevelParam->limitserror;
		}

		//If the enemy is in the sights.
		float atanxy = fabsf(atanx) + fabsf(atany);
		if( atanxy < ShotAngle ){
			int rand = LevelParam->attack;
			if( longattack == true ){ rand += 1; }

			//Fire
			if( GetRand(rand) == 0 ){
				ObjMgr->ShotWeapon(ctrlid);
			}
		}
	}

	//Switching the distance and distance depending on the distance
	// Keep the current setting when the distance equals AI_CHECKSHORTATTACK_DIST exactly.
	if( r < AI_CHECKSHORTATTACK_DIST * AI_CHECKSHORTATTACK_DIST ){
		longattack = false;
	}
	if( (r > AI_CHECKSHORTATTACK_DIST * AI_CHECKSHORTATTACK_DIST)&&(MoveNavi->GetRun2() == false) ){
		longattack = true;
	}

	if( zombie == false ){
		//Move to random
		MoveRandom();
	}

	actioncnt += 1;
}

//! @brief Cancel Attacks
bool AIcontrol::ActionCancel()
{
	//Ends when non-combating flags are enabled
	if( NoFight == true ){
		return true;
	}

	//Ends when the enemy died
	if( enemyhuman->GetDeadFlag() == true ){
		return true;
	}

	//Get distance
	float tx, ty, tz;
	enemyhuman->GetPosData(&tx, &ty, &tz, NULL);
	float x = posx - tx;
	float y = posy - ty;
	float z = posz - tz;
	float r = x*x + y*y + z*z;

	//If the distance is too far, it is finished.
	if( r > 620.0f*620.0f ){
		return true;
	}

	if( longattack == false ){
		//Check whether the enemy is visible at the right time
		if( GetRand(40) == 0 ){
			//Stop if a block obstructs the view of the enemy.
			if( CheckLookEnemy(enemyhuman, AI_SEARCH_RX, AI_SEARCH_RY, 620.0f, NULL) == false ){
				return true;
			}
		}

		//Force termination.
		if( GetRand(550) == 0 ){
			return true;
		}
	}
	else{
		//Check whether the enemy is visible at the right time
		if( GetRand(30) == 0 ){
			//Stop if a block obstructs the view of the enemy.
			if( CheckLookEnemy(enemyhuman, AI_SEARCH_RX, AI_SEARCH_RY, 620.0f, NULL) == false ){
				return true;
			}
		}

		//Force termination.
		if( GetRand(450) == 0 ){
			return true;
		}
	}

	return false;
}

//! @brief Equip a weapon.
int AIcontrol::HaveWeapon()
{
	int selectweapon;
	class weapon *weapon[TOTAL_HAVEWEAPON];
	int lnbslist[TOTAL_HAVEWEAPON];
	int nbslist[TOTAL_HAVEWEAPON];
	int nbs = 0;

	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		weapon[i] = NULL;
		lnbslist[i] = 0;
		nbslist[i] = 0;
	}

	//Get weapon information
	ctrlhuman->GetWeapon(&selectweapon, weapon, lnbslist, nbslist);

	//Even if you don't have weapons, you can't switch if you have a number of bullets.
	if( (weapon[selectweapon] == NULL)&&(nbslist[selectweapon] > 0) ){
		return 0;
	}

	//Get total number of bullets if you have weapons
	if( weapon[selectweapon] != NULL ){
		weapon[selectweapon]->GetParamData(NULL, NULL, &nbs);
	}

	//If you don't have a weapon or the bullet is 0
	if( (weapon[selectweapon] == NULL)||(nbs == 0) ){
		//Select the next weapon.
		int notselectweapon = selectweapon + 1;
		if( notselectweapon == TOTAL_HAVEWEAPON ){ notselectweapon = 0; }

		//If you have the following weapons
		if( weapon[notselectweapon] != NULL ){
			//If the number of bullets of the following weapon is not 0
			weapon[notselectweapon]->GetParamData(NULL, NULL, &nbs);
			if( nbs > 0 ){
				//Switch weapons.
				ObjMgr->ChangeHaveWeapon(ctrlid);
				return 1;
			}
		}
	}

	return 0;
}

//! @brief Stop moving and directional change randomly
void AIcontrol::CancelMoveTurn()
{
	int forward, back, side, updown, rightleft;

	if( battlemode == AI_ACTION ){			//Attacking
		if( MoveNavi->GetRun2() == true ){	//Priority running
			ObjDriver->DelModeFlag(AI_CTRL_MOVEFORWARD);
			ObjDriver->DelModeFlag(AI_CTRL_MOVEBACKWARD);
			ObjDriver->DelModeFlag(AI_CTRL_MOVELEFT);
			ObjDriver->DelModeFlag(AI_CTRL_MOVERIGHT);
			ObjDriver->DelModeFlag(AI_CTRL_TURNUP);
			ObjDriver->DelModeFlag(AI_CTRL_TURNDOWN);
			ObjDriver->DelModeFlag(AI_CTRL_TURNLEFT);
			ObjDriver->DelModeFlag(AI_CTRL_TURNRIGHT);
			if( GetRand(3) == 0 ){
				ObjDriver->DelModeFlag(AI_CTRL_MOVEWALK);
			}
			return;
		}
		else{									//Other than priority running
			if( longattack == false ){
				forward = 6;
				back = 6;
				side = 7;
				updown = 5;
				rightleft = 6;
			}
			else{
				forward = 5;
				back = 4;
				side = 5;
				updown = 3;
				rightleft = 3;
			}
		}
	}
	else if( battlemode == AI_CAUTION ){	//On alert.
		forward = 10;
		back = 10;
		side = 10;
		updown = 14;
		rightleft = 20;
	}
	else{
		forward = 12;
		back = 12;
		side = 12;
		updown = 15;
		rightleft = 18;
	}

	//Stop moving randomly
	if( GetRand(forward) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_MOVEFORWARD);
	}
	if( GetRand(back) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_MOVEBACKWARD);
	}
	if( GetRand(side) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_MOVELEFT);
	}
	if( GetRand(side) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_MOVERIGHT);
	}
	if( GetRand(3) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_MOVEWALK);
	}

	//Stop rotation randomly
	if( GetRand(updown) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_TURNUP);
	}
	if( GetRand(updown) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_TURNDOWN);
	}
	if( GetRand(rightleft) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_TURNLEFT);
	}
	if( GetRand(rightleft) == 0 ){
		ObjDriver->DelModeFlag(AI_CTRL_TURNRIGHT);
	}
}

//! @brief Reload and throw weapons
//! @return Discard: 1; reload: 2; switch weapons: 3; toggle FULL/SEMI: 4; no action: 0.
int AIcontrol::ControlWeapon()
{
	int selectweapon;
	class weapon *weapon[TOTAL_HAVEWEAPON];
	int weaponid, lnbs, nbs;
	WeaponParameter paramdata;
	int burstcntS, burstcntN;

	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		weapon[i] = NULL;
	}

	//Get weapon information
	ctrlhuman->GetWeapon(&selectweapon, weapon, NULL, NULL);
	if( weapon[selectweapon] == NULL ){ return 0; }
	weapon[selectweapon]->GetParamData(&weaponid, &lnbs, &nbs);

	//Get the performance of weapons
	if( Param->GetWeapon(weaponid, &paramdata) == 1 ){ return 0; }

	//Disable the scope.
	ctrlhuman->SetDisableScope();

	//"Case" does nothing
	if( weaponid == ID_WEAPON_CASE ){ return 0; }

	//If holding a grenade during combat.
	if( battlemode == AI_ACTION ){
		if( weaponid == ID_WEAPON_GRENADE ){
			int nextselectweapon, nextnds;

			//Get the number of bullets of the next weapon
			nextselectweapon = selectweapon + 1;
			nextnds = 0;
			if( nextselectweapon == TOTAL_HAVEWEAPON ){
				nextselectweapon = 0;
			}
			if( weapon[nextselectweapon] != NULL ){
				weapon[nextselectweapon]->GetParamData(NULL, NULL, &nextnds);
			}

			if( longattack == false ){
				// Change with 1/100 probability
				if( (GetRand(100) == 0)&&(nextnds > 0) ){
					ObjMgr->ChangeHaveWeapon(ctrlid);
					return 3;
				}
			}
			else{
				// Change the probability of 1/66
				if( (GetRand(66) == 0)&&(nextnds > 0) ){
					ObjMgr->ChangeHaveWeapon(ctrlid);
					return 3;
				}
			}
		}
	}

	//If there is no ammunition left.
	if( lnbs == 0 ){
		int ways;

		//Determination of processing probability
		if( battlemode == AI_NORMAL ){ ways = 1; }
		else if( battlemode == AI_CAUTION ){ ways = 10; }
		else{ ways = 8; }

		// Processing with 1/ways probability
		if( GetRand(ways) == 0 ){
			int under;

			//Reload Probability
			if( battlemode == AI_NORMAL ){
				ways = 1;
				under = 0;
			}
			else if( battlemode == AI_CAUTION ){
				ways = 5;
				under = 3;
			}
			else{
				if( longattack == false ){
					// Probability is 3/4
					ways = 4;
					under = 2;
				}
				else{
					// Probability 2/3
					ways = 3;
					under = 1;
				}
			}

			//If there is no bullet, throw away
			if( nbs == 0 ){
				ObjMgr->DumpWeapon(ctrlid);
				return 1;
			}

			//Randomly reload or have weapons
			if( GetRand(ways) <= under ){
				ObjMgr->ReloadWeapon(ctrlid);
				return 2;
			}
			//else{
				ObjMgr->ChangeHaveWeapon(ctrlid);
				return 3;
			//}
		}
	}

	//If you are a weapon that can be switched in a resonance
	if( paramdata.ChangeWeaponID != -1 ){
		//Get current weapon resonance settings
		burstcntS = paramdata.burstcnt;

		//es new weapon resonance settings
		if( Param->GetWeapon(paramdata.ChangeWeaponID, &paramdata) == 1 ){ return 0; }
		burstcntN = paramdata.burstcnt;

		if( longattack == false ){
			//If you switch to SEMI or FULL in the near distance attack
			if( (burstcntS == 1)&&(burstcntN != 1) ){
				ObjMgr->ChangeWeaponID(ctrlid);	//Switch.
				return 4;
			}
		}
		else{
			//If you switch to FULL or SEMI in a distance attack
			if( (burstcntS != 1)&&(burstcntN == 1) ){
				ObjMgr->ChangeWeaponID(ctrlid);	//Switch.
				return 4;
			}
		}
	}

	return 0;
}

//! @brief Throwing grenades
//! @return In progress: 0; finished throwing: 1; not holding a grenade: 2.
//! @attention Handle the grenade-throwing path. For attacks after spotting an enemy, see Action().
int AIcontrol::ThrowGrenade()
{
	int selectweapon;
	class weapon *weapon[TOTAL_HAVEWEAPON];
	int weaponid, nbs, i;
	pointdata pdata;
	float posy2;
	float atan_rx, atan_ry;

	for(int i=0; i<TOTAL_HAVEWEAPON; i++){
		weapon[i] = NULL;
	}

	//Check whether you have a grenades
	ctrlhuman->GetWeapon(&selectweapon, weapon, NULL, NULL);
	for(i=0; i<TOTAL_HAVEWEAPON; i++){
		if( weapon[i] != NULL ){
			weapon[i]->GetParamData(&weaponid, NULL, &nbs);
			if( weaponid == ID_WEAPON_GRENADE ){
				break;
			}
		}
	}

	//Switch to grenades, switch to
	if( (i != TOTAL_HAVEWEAPON)&&(i != selectweapon) ){
		ObjMgr->ChangeHaveWeapon(ctrlid, i);
	}

	//Get the path and character height.
	MoveNavi->GetPathPointData(&pdata);
	posy2 = posy + VIEW_HEIGHT;

	//Stop all movements once
	ObjDriver->ResetMode();

	//Determine the angle to the target point
	CheckTargetAngle(posx, posy2, posz, rx*-1 + (float)M_PI/2, ry, pdata.x, pdata.y, pdata.z, 0.0f, &atan_rx, &atan_ry, NULL);

	//If you don't have a grenades
	if( i == TOTAL_HAVEWEAPON ){
		//* Even if you don't have a grenades, this frame will process it to the direction.
		// If this process does not have grenades and falls into the endless loop of grenades, continue to face the grenades throw path.
		// (Specification of XOPS)

		//Turn
		if( atan_rx > 0.0f ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT);
		}
		if( atan_rx < 0.0f ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT);
		}
		ArmAngle();

		return 2;
	}

	//Turn
	if( atan_rx > 0.0f ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNLEFT);
	}
	if( atan_rx < 0.0f ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNRIGHT);
	}
	if( atan_ry > 0.0f ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNUP);
	}
	if( atan_ry < 0.0f ){
		ObjDriver->SetModeFlag(AI_CTRL_TURNDOWN);
	}

	//Check whether it is determined by the tertiary
	if( (fabsf(atan_rx) < DegreeToRadian(1.5f))&&(fabsf(atan_ry) < DegreeToRadian(1.5f)) ){
		//Prevent automatic fire.
		if( (ctrlhuman->GetWeaponBlazingmode() == false)&&(EventWeaponShot == true) ){ return 0; }

		//Set Angle
		ctrlhuman->SetRxRy(rx, ry);

		//Throwing
		if( ObjMgr->ShotWeapon(ctrlid) != 0 ){
			EventWeaponShot = true;
			return 1;
		}
	}

	return 0;
}

//! @brief Set arm angle
void AIcontrol::ArmAngle()
{
	ObjDriver->DelModeFlag(AI_CTRL_TURNUP);
	ObjDriver->DelModeFlag(AI_CTRL_TURNDOWN);

	if( ctrlhuman->GetMainWeaponTypeNO() == ID_WEAPON_NONE ){	//Unarmed.
		//Keep aiming downward.
		ObjDriver->SetModeFlag(AI_CTRL_TURNDOWN);
	}
	else if( (battlemode == AI_CAUTION)&&(cautioncnt > 0) ){	//On alert.
		float addry2 = 0.0f - ry;

		//Turn
		if( addry2 > DegreeToRadian(1.0f) ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNUP);
		}
		if( addry2 < DegreeToRadian(-1.0f) ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNDOWN);
		}
	}
	else{									//Weapon holding at normal time
		//Turn
		if( ry < DegreeToRadian(-32) ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNUP);
		}
		if( ry > DegreeToRadian(-28) ){
			ObjDriver->SetModeFlag(AI_CTRL_TURNDOWN);
		}
	}
}

//! @brief Search for enemies
int AIcontrol::SearchEnemy()
{
	//If a non-combating flag is enabled, no enemy will be found
	if( NoFight == true ){
		return 0;
	}

	if( battlemode == AI_ACTION ){ return 0; }

	int weaponid = ctrlhuman->GetMainWeaponTypeNO();
	int weaponscope = 0;
	ScopeParameter sparam;
	int searchloops;
	float maxDist;
	float A_rx, A_ry, B_rx, B_ry;

	//Get scope mode if you have a weapon
	if( weaponid != ID_WEAPON_NONE ){
		WeaponParameter Wparam;

		Param->GetWeapon(weaponid, &Wparam);
		weaponscope = Wparam.scopemode;
	}

	//Set the number of enemy searches and their range (distance and angle).
	if( battlemode == AI_NORMAL ){
		searchloops = (LevelParam->search) * AI_TOTALHUMAN_SCALE;

		Param->GetScopeParam(weaponscope, &sparam);
		maxDist = sparam.AI_AddSearchEnemyMaxDist_Normal;
		maxDist += 12.0f*((LevelParam->search)-2) + 350.0f;
		A_rx = DegreeToRadian(110);
		A_ry = DegreeToRadian(60);
		B_rx = DegreeToRadian(60);
		B_ry = DegreeToRadian(40);
	}
	else {	//battlemode == AI_CAUTION
		searchloops = (LevelParam->search) * AI_TOTALHUMAN_SCALE + 4;

		Param->GetScopeParam(weaponscope, &sparam);
		maxDist = sparam.AI_AddSearchEnemyMaxDist_Caution;
		maxDist += 15.0f*((LevelParam->search)-2) + 420.0f;
		A_rx = DegreeToRadian(130);
		A_ry = DegreeToRadian(80);
		B_rx = DegreeToRadian(80);
		B_ry = DegreeToRadian(50);
	}

	//Search for enemies the specified number of times.
	for(int i=0; i<searchloops; i++){
		int targetid = GetRand(MAX_HUMAN);

		if( CheckLookEnemy(targetid, A_rx, A_ry, AI_CHECKSHORTATTACK_DIST, NULL) == true ){
			longattack = false;
			return 1;
		}

		if( CheckLookEnemy(targetid, B_rx, B_ry, maxDist, NULL) == true ){
			if( GetRand(4) == 0 ){
				if( MoveNavi->GetRun2() == true ){ longattack = false; }
				else{ longattack = true; }
				return 2;
			}
		}
	}
	return 0;
}

//! @brief Search for nearby enemies while attacking at long range.
int AIcontrol::SearchShortEnemy()
{
	float A_rx, A_ry;
	A_rx = DegreeToRadian(100);
	A_ry = DegreeToRadian(52);

	for(int i=0; i<3; i++){
		int targetid = GetRand(MAX_HUMAN);

		if( CheckLookEnemy(targetid, A_rx, A_ry, AI_CHECKSHORTATTACK_DIST, NULL) == true ){
			longattack = false;
			return 1;
		}
	}
	return 0;
}

//! @brief Determine whether the enemy is visible
bool AIcontrol::CheckLookEnemy(int id, float search_rx, float search_ry, float maxDist, float *out_minDist)
{
	if( ObjMgr == NULL ){ return false; }

	//Get the character object.
	class human* thuman;
	thuman = ObjMgr->GetHumanObject(id);
	if( thuman == NULL ){ return false; }

	//Overload the same name function
	return CheckLookEnemy(thuman, search_rx, search_ry, maxDist, out_minDist);
}

//! @brief Determine whether the enemy is visible
bool AIcontrol::CheckLookEnemy(class human* thuman, float search_rx, float search_ry, float maxDist, float *out_minDist)
{
	//return false;

	//Return if the data is invalid.
	if( ctrlhuman == NULL ){ return false; }
	if( thuman == NULL ){ return false; }
	if( thuman->GetEnableFlag() == false ){ return false; }
	if( thuman->GetDeadFlag() == true ){ return false; }

	//Get the coordinates of yourself and the enemy
	int ctrlteam, targetteam;
	float tx, ty, tz;
	ctrlhuman->GetParamData(NULL, NULL, NULL, &ctrlteam);
	thuman->GetParamData(NULL, NULL, NULL, &targetteam);
	thuman->GetPosData(&tx, &ty, &tz, NULL);

	//If the enemy is not the same person
	if( ctrlteam != targetteam ){
		float mrx, mry;
		float Dist2 , Dist;
		float Dist_dummy; 

		//Determine distance and get angles
		if( CheckTargetAngle(posx, posy, posz, rx*-1 + (float)M_PI/2, 0.0f, tx, ty, tz, maxDist, &mrx, &mry, &Dist2) == true ){
			//On the angle, if you are in the vision
			if( (fabsf(mrx) < search_rx/2)&&(fabsf(mry) < search_ry/2) ){
				float vx, vy, vz;

				Dist = sqrtf(Dist2);

				//Get the vector.
				vx = (tx - posx)/Dist;
				vy = (ty - posy)/Dist;
				vz = (tz - posz)/Dist;

				//If the block is not blocked (should be judged by ray, and if it is not hit)
				if( CollD->CheckALLBlockIntersectRay(posx, posy + VIEW_HEIGHT, posz, vx, vy, vz, NULL, NULL, &Dist_dummy, Dist) == false ){
					if( out_minDist != NULL ){ *out_minDist = Dist; }
					enemyhuman = thuman;
					return true;
				}
			}
		}
	}
	return false;
}

//! @brief Check whether there is a carcass
bool AIcontrol::CheckCorpse(int id)
{
	//If the class is set, it will not be processed.
	if( ObjMgr == NULL ){ return false; }
	if( ctrlhuman == NULL ){ return false; }

	if( (id < 0)||(MAX_HUMAN <= id) ){ return false; }

	//Get target classes
	class human* thuman;
	thuman = ObjMgr->GetHumanObject(id);
	if( thuman == NULL ){ return false; }
	if( thuman->GetEnableFlag() == false ){ return false; }

	//If you are dead
	if( thuman->GetDeadFlag() == true ){

		//Get Team Number
		int ctrlteam, targetteam;
		ctrlhuman->GetParamData(NULL, NULL, NULL, &ctrlteam);
		thuman->GetParamData(NULL, NULL, NULL, &targetteam);

		//If this is an ally.
		if( ctrlteam == targetteam ){
			float tposx, tposy, tposz;
			float atan;

			//Get distance from coordinates
			thuman->GetPosData(&tposx, &tposy, &tposz, NULL);

			//Calculate the distance and angle.
			if( CheckTargetAngle(posx, posy, posz, rx*-1 + (float)M_PI/2, 0.0f, tposx, tposy, tposz, 22.0f, &atan, NULL, NULL) == true ){
				if( fabsf(atan) < DegreeToRadian(40) ){
					return true;
				}
			}
		}
	}
	return false;
}

//! @brief Follow the movement path.
void AIcontrol::MovePath()
{
	int movemode, pointmode;
	MoveNavi->GetTargetPos(NULL, NULL, NULL, &movemode, &pointmode);

	if( movemode == AI_NAVI_MOVE_NULL ){			//Anomaly Path
		return;
	}

	if( CheckTargetPos(false) == false ){
		MoveTarget(false);
	}
	else{
		if( (pointmode == AI_NAVI_POINT_WAIT)||(pointmode == AI_NAVI_POINT_TRACKING) ){
			TurnSeen();
		}
		else if( pointmode == AI_NAVI_POINT_STOP_5SEC ){
			if( waitcnt < ((int)GAMEFPS)*5 ){
				if( StopSeen() == true ){
					waitcnt += 1;
				}
			}
			else{
				waitcnt = 0;
				MoveNavi->MovePathNextState();
				MoveNavi->MovePathNowState();
			}
		}
		else{	//pointmode == AI_NAVI_POINT_NULL
			waitcnt = 0;
			MoveNavi->MovePathNextState();
			MoveNavi->MovePathNowState();
		}
	}
}

//! @brief Attack main processing
//! @return unchanged: false change: true
bool AIcontrol::ActionMain()
{
	int newbattlemode = AI_ACTION;

	//Attack processing
	Action();

	//If a random pass is executed, processing execution * assumes "priority running"
	if( MoveNavi->GetMoveMode() == AI_RANDOM ){
		MoveNavi->MovePathNextState();
		MoveNavi->MovePathNowState();
	}

	if( MoveNavi->GetRun2() == true ){				//Priority running
		//Go to Target Point
		if( CheckTargetPos(false) == true ){
			//newbattlemode = AI_NORMAL;

			//Set this goal point to the back position at the time of warning status cancellation.
			MoveNavi->GetTargetPos(&cautionback_posx, &cautionback_posz, NULL, NULL, NULL);

			MoveNavi->MovePathNextState();
			MoveNavi->MovePathNowState();
		}
		else{
			MoveTarget2(false);
		}
	}
	else{									//Other than priority running
		//Find nearby enemies if you are in a distance attack
		if( longattack == true ){
			SearchShortEnemy();
		}
	}

	//Check whether the attack should end.
	if( ActionCancel() == true ){
		enemyhuman = NULL;

		if( MoveNavi->GetRun2() == true ){
			newbattlemode = AI_NORMAL;
		}
		else{
			newbattlemode = AI_CAUTION;
			cautioncnt = 160;
			FaceCaution_flag = false;
			FaceCaution_rx = 0.0f;
		}
	}

	//Determine settings
	if( battlemode != newbattlemode ){
		battlemode = newbattlemode;
		return true;
	}
	return false;
}

//! @brief Warning main processing
//! @return unchanged: false change: true
bool AIcontrol::CautionMain()
{
	int newbattlemode = AI_CAUTION;
	float caution_rx;

	//Get coordinates and team numbers
	int teamid;
	ctrlhuman->GetParamData(NULL, NULL, NULL, &teamid);

	//Get the hit and sound status.
	bool HitFlag = ctrlhuman->CheckHit(&caution_rx);
	soundlist soundlist[MAX_SOUNDMGR_LIST];
	int soundlists = GameSound->GetWorldSound(posx, posy + VIEW_HEIGHT, posz, teamid, soundlist);

	//If a non-combating flag is enabled, the sound will not be heard
	if( NoFight == true ){
		soundlists = 0;
	}

	//Main processing
	if( enemyhuman != NULL ){		//If you have already found an enemy
		newbattlemode = AI_ACTION;
		actioncnt = 0;
	}
	else if( SearchEnemy() != 0 ){	//If you find an enemy
		newbattlemode = AI_ACTION;
		actioncnt = 0;
	}
	else if( HitFlag == true ){		//Took a hit.
		cautioncnt = 160;					//Resume alerts
		FaceCaution_flag = true;
		FaceCaution_rx = caution_rx;
	}
	else if( soundlists > 0 ){		//Heard a sound.
		cautioncnt = 160;					//Resume alerts
	}
	else if( cautioncnt == 0 ){		//If you finish the warning
		if( CheckTargetPos(true) == false ){				//Is it away from the warning start point?
			MoveTarget(true);				//Close to the starting point of warning
		}
		else{
			newbattlemode = AI_NORMAL;
			FaceCaution_flag = false;
			FaceCaution_rx = 0.0f;

			//If you are waiting for a warning pass, go to next
			pointdata pdata;
			MoveNavi->GetPathPointData(&pdata);
			if( (pdata.p1 == POINT_P1TYPE_AIPATH)&&(pdata.p2 == 4) ){
				MoveNavi->MovePathNextState();
				MoveNavi->MovePathNowState();
			}
		}
	}
	else if( (cautioncnt < 100)&&(FaceCaution_flag == false) ){		//When 100 frames are cut, the warning ends randomly (count: 0)
		if( GetRand(50) == 0 ){ cautioncnt = 0; }
	}
	else{ cautioncnt -= 1; }

	//If you leave the target too far in the tail, you will end the warning randomly
	if( (MoveNavi->GetMoveMode() == AI_TRACKING)&&(GetRand(3) == 0) ){
		float x, z;
		float tx, tz;
		class human *targethuman = ObjMgr->GetHumanObject(MoveNavi->GetTargetHumanID());
		targethuman->GetPosData(&tx, NULL, &tz, NULL);
		x = posx - tx;
		z = posz - tz;
		if( (x*x + z*z) > 25.0f*25.0f ){
			cautioncnt = 0;
		}
	}

	//Rotation and arm angle
	TurnSeen();
	ArmAngle();

	//Determine settings
	if( battlemode != newbattlemode ){
		battlemode = newbattlemode;
		return true;
	}
	return false;
}

//! @brief Normal main processing
//! @return unchanged: false change: true
bool AIcontrol::NormalMain()
{
	int newbattlemode = AI_NORMAL;
	float caution_rx;
	int EventThrowGrenade = 0;

	MoveNavi->MovePathNowState();
	enemyhuman = NULL;

	//Get coordinates and team numbers
	int teamid;
	ctrlhuman->GetParamData(NULL, NULL, NULL, &teamid);

	//Get the hit and sound status.
	bool HitFlag = ctrlhuman->CheckHit(&caution_rx);
	soundlist soundlist[MAX_SOUNDMGR_LIST];
	int soundlists = GameSound->GetWorldSound(posx, posy + VIEW_HEIGHT, posz, teamid, soundlist);

	//If a non-combating flag is enabled, the sound will not be heard
	if( NoFight == true ){
		soundlists = 0;
	}

	//Random path
	if( MoveNavi->GetMoveMode() == AI_RANDOM ){
		MoveNavi->MovePathNextState();
		MoveNavi->MovePathNowState();
	}

	//Grenade-throwing path.
	if( MoveNavi->GetMoveMode() == AI_GRENADE ){
		EventThrowGrenade = ThrowGrenade();
		if( EventThrowGrenade != 0 ){
			MoveNavi->MovePathNextState();
			MoveNavi->MovePathNowState();
		}
	}
	if( EventThrowGrenade != 1 ){
		EventWeaponShot = false;
	}

	if( MoveNavi->GetMoveMode() == AI_RUN2 ){		//Priority running
		//Enter the attack when the enemy is found
		if( SearchEnemy() != 0 ){
			newbattlemode = AI_ACTION;
			//cautionback_posx = posx;
			//cautionback_posz = posz;
		}
		else{
			MovePath();		//Move execution
		}
	}
	else{							//Processing other than priority running
		//Process to enter a warning judgment
		if( HitFlag == true ){							//Took a hit.
			newbattlemode = AI_CAUTION;
			cautioncnt = 160;
			cautionback_posx = posx;
			cautionback_posz = posz;
			FaceCaution_flag = true;
			FaceCaution_rx = caution_rx;
		}
		else if(
			(SearchEnemy() != 0)||						//Spotted an enemy.
			(soundlists > 0)||							//Heard a sound.
			(CheckCorpse( GetRand(MAX_HUMAN) ) == true)	//Found a corpse.
		){
			newbattlemode = AI_CAUTION;
			cautioncnt = 160;
			cautionback_posx = posx;
			cautionback_posz = posz;
			FaceCaution_flag = false;
			FaceCaution_rx = 0.0f;
		}
		else{
			MovePath();		//Move execution
		}
	}

	//Set arm angle
	if( MoveNavi->GetMoveMode() != AI_GRENADE ){
		ArmAngle();
	}

	//Determine settings
	if( battlemode != newbattlemode ){
		battlemode = newbattlemode;
		return true;
	}
	return false;
}

//! @brief Initialization function
void AIcontrol::Init()
{
	//If the class is set, it will not be processed.
	if( ctrlhuman == NULL ){ return; }
	if( blocks == NULL ){ return; }
	if( CollD == NULL ){ return; }

	//If you are not using it, it will not be processed
	if( ctrlhuman->GetEnableFlag() == false ){ return; }

	//Initialize state
	NoFight = false;
	battlemode = AI_NORMAL;
	enemyhuman = NULL;
	FaceCaution_flag = false;
	FaceCaution_rx = 0.0f;
	total_move = 0.0f;
	waitcnt = 0;
	gotocnt = 0;
	cautioncnt = 0;
	actioncnt = 0;
	longattack = false;
	EventWeaponShot = false;

	MoveNavi->Init();
	ObjDriver->Init();

	//Get AI Levels and Settings
	int paramid;
	HumanParameter paramdata;
	ctrlhuman->GetParamData(&paramid, NULL, NULL, NULL);
	Param->GetHuman(paramid, &paramdata);
	AIlevel = paramdata.AIlevel;
	Param->GetAIlevel(AIlevel, &LevelParam);

	//Search for the following points
	//MoveNavi->MovePathNextState();
	MoveNavi->MovePathNowState();
}

//! @brief Set AI Level
//! @param level AI Level
void AIcontrol::SetAIlevel(int level)
{
	if( (level < -1)||(TOTAL_PARAMETERINFO_AILEVEL <= level) ){ return; }

	if( level == -1 ){
		//Get AI Level
		int paramid;
		HumanParameter paramdata;
		ctrlhuman->GetParamData(&paramid, NULL, NULL, NULL);
		Param->GetHuman(paramid, &paramdata);
		AIlevel = paramdata.AIlevel;
	}
	else{
		AIlevel = level;
	}

	//Getting the setting value
	Param->GetAIlevel(AIlevel, &LevelParam);
}

//! @brief Get AI Level
//! @return AI Level
int AIcontrol::GetAIlevel()
{
	return AIlevel;
}

//! @brief Wait to the specified location
//! @param px X Coordinate
//! @param pz Z Coordinates
//! @param rx Preferred facing direction.
//! @attention Force waiting at the specified coordinates regardless of the movement path. This remains in effect until Init() is called again.
void AIcontrol::SetHoldWait(float px, float pz, float rx)
{
	MoveNavi->SetHoldWait(px, pz, rx);
}

//! @brief Chase the specified person
//! @param id Data number of people
//! @attention Force tracking the specified character regardless of the movement path. This remains in effect until Init() is called again.
void AIcontrol::SetHoldTracking(int id)
{
	if( (0 <= id)&&(id < MAX_HUMAN) ){ MoveNavi->SetHoldTracking(id); }
}

//! @brief Forced to be alerted
//! @warning If you are running a priority run, this function will not do anything.
void AIcontrol::SetCautionMode()
{
	//Do not do anything if it is a priority run
	if( MoveNavi->GetRun2() == true ){ return; }

	if( battlemode == AI_NORMAL ){
		cautionback_posx = posx;
		cautionback_posz = posz;
	}
	battlemode = AI_CAUTION;
	cautioncnt = 160;
	FaceCaution_flag = false;
	FaceCaution_rx = 0.0f;
}

//! @brief Set non-combating flags
//! @param flag true: do not fight (non-combat) false: do combat (usually)
//! @attention If the flag is enabled, the enemy will not be affected.
void AIcontrol::SetNoFightFlag(bool flag)
{
	NoFight = flag;
}

//! @brief Battle mode acquisition
//! @param *mode Pointer to receive combat mode (number)
//! @param *modestr Pointer to receive a string of combat mode names
void AIcontrol::GetBattleMode(int *mode, char *modestr)
{
	if( mode != NULL ){
		*mode = battlemode;
	}
	if( modestr != NULL ){
		switch(battlemode){
			case 0: strcpy(modestr, "AI_DEAD"); break;
			case 1: strcpy(modestr, "AI_ACTION"); break;
			case 2: strcpy(modestr, "AI_CAUTION"); break;
			case 3: strcpy(modestr, "AI_NORMAL"); break;
			default: strcpy(modestr, "");
		}
	}
}

//! @brief Attack target data number
//! @return Character data ID (-1 if there is no target).
int AIcontrol::GetEnemyHumanID()
{
	if( enemyhuman == NULL ){
		return -1;
	}
	//else{
		return ObjMgr->GetHumanObjectID(enemyhuman);
	//}
}

//! @brief Get the target point (target) to move
//! @param posx Pointer to receive X coordinates at the target point
//! @param posz Pointer to receive the Z coordinates of the target point
//! @param movemode Pointer to receive the moving mode to the target point
void AIcontrol::GetMoveTargetPos(float *posx, float *posz, int *movemode)
{
	MoveNavi->GetTargetPos(posx, posz, NULL, movemode, NULL);

	//If it is not "priority driving" + during attacking and wary, you can check the position to return after warning.
	if( MoveNavi->GetRun2() == false ){
		if( (battlemode == AI_ACTION)||(battlemode == AI_CAUTION) ){
			*posx = cautionback_posx;
			*posz = cautionback_posz;
		}
	}
}

//! @brief Point data paths that are currently loaded
//! @param out_data Pointer to receive point data
void AIcontrol::GetPathPointData(pointdata *out_data)
{
	MoveNavi->GetPathPointData(out_data);
}

//! @brief Processing function
void AIcontrol::Process()
{
	//If the class is set, it will not be processed.
	if( ctrlhuman == NULL ){ return; }
	if( blocks == NULL ){ return; }
	if( CollD == NULL ){ return; }

	//Not processed if it is invalid class
	if( ctrlhuman->GetEnableFlag() == false ){ return; }

	//If dead.
	if( ctrlhuman->GetHP() <= 0 ){
		battlemode = AI_DEAD;
		return;
	}

	//My HP is not 0, but I have died... I have survived?
	if( battlemode == AI_DEAD ){
		battlemode = AI_NORMAL;
		MoveNavi->MovePathNowState();
	}

	//Get coordinates and angles
	ctrlhuman->GetPosData(&posx, &posy, &posz, NULL);
	ctrlhuman->GetRxRy(&rx, &ry);

	//Stop working randomly
	CancelMoveTurn();

	//If you are attacking or under warning
	if( (battlemode == AI_ACTION)||(battlemode == AI_CAUTION) ){
		//Equip a weapon.
		HaveWeapon();
	}

	//Run the main calculation.
	if( battlemode == AI_ACTION ){			//Attacking
		ActionMain();
	}
	else if( battlemode == AI_CAUTION ){	//On alert.
		CautionMain();
	}
	else{									//In the normal state.
		NormalMain();
	}

	//Moving and Directional Conversion
	ObjDriver->ControlObject();

	//Handling Weapons
	ControlWeapon();
}

//! @brief Constructor
AIMoveNavi::AIMoveNavi(class ObjectManager *in_ObjMgr, int in_ctrlid, class PointDataInterface *in_Points)
{
	ObjMgr = in_ObjMgr;
	ctrlid = in_ctrlid;
	Points = in_Points;

	movemode = AI_WAIT;
	hold = false;
	path_pointid = 0;
	target_humanid = -1;
	target_posx = 0.0f;
	target_posz = 0.0f;
	target_rx = 0.0f;
}

//! @brief Destructor
AIMoveNavi::~AIMoveNavi()
{}

//! @brief Set target class
//! @attention If you do not set this function, the class itself will not function correctly.
void AIMoveNavi::SetClass(class ObjectManager *in_ObjMgr, int in_ctrlid, class PointDataInterface *in_Points)
{
	ObjMgr = in_ObjMgr;
	ctrlid = in_ctrlid;
	Points = in_Points;
}

//! @brief Initialization function
void AIMoveNavi::Init()
{
	movemode = AI_NULL;
	hold = false;
	path_pointid = 0;
	target_humanid = -1;

	ObjMgr->GetHumanObject(ctrlid)->GetParamData(NULL, &path_pointid, NULL, NULL);
	MovePathNextState();
}

//! @brief Apply the target point to move
//! @return Finish: true Fail: false
//! @attention Call this function every frame to update the tracked character's position using its latest coordinates.
bool AIMoveNavi::MovePathNowState()
{
	if( hold == true ){
		if( movemode == AI_TRACKING ){
			class human *targethuman;
			targethuman = ObjMgr->GetHumanObject(target_humanid);
			targethuman->GetPosData(&target_posx, NULL, &target_posz, &target_rx);
		}
		return true;
	}


	//Get point information
	pointdata pdata;
	if( Points->Getdata(&pdata, path_pointid) != 0 ){
		movemode = AI_NULL;
		return false;
	}

	//Moving Pass
	if( pdata.p1 == POINT_P1TYPE_AIPATH ){
		//Move state setting
		switch(pdata.p2){
			case 0: movemode = AI_WALK; break;
			case 1: movemode = AI_RUN; break;
			case 2: movemode = AI_WAIT; break;
			case 3: movemode = AI_TRACKING; break;
			case 4: movemode = AI_WAIT; break;
			case 5: movemode = AI_STOP_5SEC; break;
			case 6: movemode = AI_GRENADE; break;
			case 7: movemode = AI_RUN2; break;
			default: break;
		}

		if( movemode == AI_TRACKING ){
			class human *targethuman;

			if( target_humanid == -1 ){
				signed short int nextpointp4 = pdata.p3;

				//Get information about points (persons)
				if( Points->SearchPointdata(&pdata, 0x08, 0, 0, 0, nextpointp4, 0) == 0 ){
					return false;
				}

				//Find a person and get a class
				targethuman = ObjMgr->SearchHuman(pdata.p4);
				if( targethuman == NULL ){ return false; }

				//Get a person's data number
				target_humanid = ObjMgr->GetHumanObjectID(targethuman);
			}

			targethuman = ObjMgr->GetHumanObject(target_humanid);
			targethuman->GetPosData(&target_posx, NULL, &target_posz, &target_rx);
		}
		else{
			//Apply the information.
			target_posx = pdata.x;
			target_posz = pdata.z;
			target_rx = pdata.r;
		}

		return true;
	}

	//Random Path
	if( pdata.p1 == POINT_P1TYPE_RAND_AIPATH ){
		movemode = AI_RANDOM;
		return false;
	}

	movemode = AI_NULL;
	return false;
}

//! @brief Search for the next target location
//! @return Finish: true Fail: false
bool AIMoveNavi::MovePathNextState()
{
	//Get point information
	pointdata pdata;
	if( Points->Getdata(&pdata, path_pointid) != 0 ){
		movemode = AI_NULL;
		return false;
	}

	signed short int nextpointp4 = pdata.p3;

	//Random Path Processing
	if( pdata.p1 == POINT_P1TYPE_RAND_AIPATH ){
		if( GetRand(2) == 0 ){
			nextpointp4 = pdata.p2;
		}
		else{
			nextpointp4 = pdata.p3;
		}
		movemode = AI_RANDOM;
	}

	//Search Point
	if( Points->SearchPointdata(&pdata, 0x08, 0, 0, 0, nextpointp4, 0) == 0 ){
		return false;
	}

	path_pointid = pdata.id;

	return true;
}

//! @brief Wait to the specified location
//! @param px X Coordinate
//! @param pz Z Coordinates
//! @param rx Preferred facing direction.
//! @attention Force waiting at the specified coordinates regardless of the movement path. This remains in effect until Init() is called again.
void AIMoveNavi::SetHoldWait(float px, float pz, float rx)
{
	movemode = AI_WAIT;
	hold = true;
	target_posx = px;
	target_posz = pz;
	target_rx = rx;
}

//! @brief Chase the specified person
//! @param id Data number of people
//! @attention Force tracking the specified character regardless of the movement path. This remains in effect until Init() is called again.
void AIMoveNavi::SetHoldTracking(int id)
{
	if( (id < 0)||(MAX_HUMAN <= id) ){ return; }

	movemode = AI_TRACKING;
	hold = true;
	target_humanid = id;
}

//! @brief Get moving mode
//! @return Moving mode
int AIMoveNavi::GetMoveMode()
{
	return movemode;
}

//! @brief Get priority running flags
//! @return Priority running: true Not a priority running: false
bool AIMoveNavi::GetRun2()
{
	if( movemode == AI_RUN2 ){
		return true;
	}
	//else{
		return false;
	//}
}

//! @brief Get the data number of the target (person)
//! @return Data number of people
int AIMoveNavi::GetTargetHumanID()
{
	return target_humanid;
}

//! @brief Point data paths that are currently loaded
//! @param out_data Pointer to receive point data
void AIMoveNavi::GetPathPointData(pointdata *out_data)
{
	Points->Getdata(out_data, path_pointid);
}

//! @brief Get the target point (target) to move
//! @param posx Pointer to receive X coordinates at the target point
//! @param posz Pointer to receive the Z coordinates of the target point
//! @param rx Pointer to receive the horizontal angle of the target point
//! @param out_movemode Pointer to receive the moving mode to the target point
//! @param out_pointmode Pointer to receive point mode at the target point
//! @warning This function does not update coordinates. Call MovePathNowState() first to update them.
void AIMoveNavi::GetTargetPos(float *posx, float *posz, float *rx, int *out_movemode, int *out_pointmode)
{
	if( posx != NULL ){ *posx = target_posx; }
	if( posz != NULL ){ *posz = target_posz; }
	if( rx != NULL ){ *rx = target_rx; }

	if( out_movemode != NULL ){
		if( (movemode == AI_WALK)||(movemode == AI_WAIT)||(movemode == AI_STOP_5SEC) ){
			*out_movemode = AI_NAVI_MOVE_WALK;
		}
		else if( movemode == AI_RUN ){
			*out_movemode = AI_NAVI_MOVE_RUN;
		}
		else if( movemode == AI_RUN2 ){
			*out_movemode = AI_NAVI_MOVE_RUN2;
		}
		else if( movemode == AI_TRACKING ){
			*out_movemode = AI_NAVI_MOVE_TRACKING;
		}
		else{
			*out_movemode = AI_NAVI_MOVE_NULL;
		}
	}

	if( out_pointmode != NULL ){
		if( movemode == AI_WAIT ){
			*out_pointmode = AI_NAVI_POINT_WAIT;
		}
		else if( movemode == AI_STOP_5SEC ){
			*out_pointmode = AI_NAVI_POINT_STOP_5SEC;
		}
		else if( movemode == AI_TRACKING ){
			*out_pointmode = AI_NAVI_POINT_TRACKING;
		}
		else if( movemode == AI_GRENADE ){	//Grenade-throwing path.
			*out_pointmode = AI_NAVI_POINT_GRENADE;
		}
		else{								//Moving path and abnormal path
			*out_pointmode = AI_NAVI_POINT_NULL;
		}
	}
}

//! @brief Constructor
AIObjectDriver::AIObjectDriver(class ObjectManager *in_ObjMgr, int in_ctrlid)
{
	ObjMgr = in_ObjMgr;
	ctrlid = in_ctrlid;

	moveturn_mode = 0;
	addrx = 0.0f;
	addry = 0.0f;
}

//! @brief Destructor
AIObjectDriver::~AIObjectDriver()
{}

//! @brief Set target class
//! @attention If you do not set this function, the class itself will not function correctly.
void AIObjectDriver::SetClass(class ObjectManager *in_ObjMgr, int in_ctrlid)
{
	ObjMgr = in_ObjMgr;
	ctrlid = in_ctrlid;
}

//! @brief Initialization function
void AIObjectDriver::Init()
{
	addrx = 0.0f;
	addry = 0.0f;

	ResetMode();
}

//! @brief Clear the moving rotation control flag (stop all movements at once)
void AIObjectDriver::ResetMode()
{
	moveturn_mode = 0;
}

//! @brief Move Rotation Control Flag Settings
//! @param flag Constant indicating the control mode (AIcontrolFlag enumeration).
void AIObjectDriver::SetModeFlag(int flag)
{
	SetFlag(moveturn_mode, flag);
}

//! @brief Move Rotation Control Flag Unlocked
//! @param flag Constant indicating the control mode (AIcontrolFlag enumeration).
void AIObjectDriver::DelModeFlag(int flag)
{
	DelFlag(moveturn_mode, flag);
}

//! @brief Move rotation control flag acquisition
//! @param flag Constant indicating the control mode (AIcontrolFlag enumeration).
bool AIObjectDriver::GetModeFlag(int flag)
{
	if( GetFlag(moveturn_mode, flag) == 0 ){
		return false;
	}
	//else{
		return true;
	//}
}

//! @brief Perform moving rotation control
//! @attention Call only once per frame.
void AIObjectDriver::ControlObject()
{
	float rx, ry;

	//Execute movement.
	if( GetFlag(moveturn_mode, AI_CTRL_MOVEFORWARD) ){
		ObjMgr->MoveForward(ctrlid);
	}
	if( GetFlag(moveturn_mode, AI_CTRL_MOVEBACKWARD) ){
		ObjMgr->MoveBack(ctrlid);
	}
	if( GetFlag(moveturn_mode, AI_CTRL_MOVELEFT) ){
		ObjMgr->MoveLeft(ctrlid);
	}
	if( GetFlag(moveturn_mode, AI_CTRL_MOVERIGHT) ){
		ObjMgr->MoveRight(ctrlid);
	}
	if( GetFlag(moveturn_mode, AI_CTRL_MOVEWALK) ){
		ObjMgr->MoveWalk(ctrlid);
	}

	//Change direction by adding rotational speed.
	if( GetFlag(moveturn_mode, AI_CTRL_TURNUP) ){
		addry += AI_ADDTURNRAD;
	}
	if( GetFlag(moveturn_mode, AI_CTRL_TURNDOWN) ){
		addry -= AI_ADDTURNRAD;
	}
	if( GetFlag(moveturn_mode, AI_CTRL_TURNLEFT) ){
		addrx -= AI_ADDTURNRAD;
	}
	if( GetFlag(moveturn_mode, AI_CTRL_TURNRIGHT) ){
		addrx += AI_ADDTURNRAD;
	}

	//Get angle
	ObjMgr->GetHumanObject(ctrlid)->GetRxRy(&rx, &ry);

	//Add to Angle
	rx += addrx;
	ry += addry;

	//Contains vertical rotation range
	if( ry > DegreeToRadian(70) ){ ry = DegreeToRadian(70); }
	if( ry < DegreeToRadian(-70) ){ ry = DegreeToRadian(-70); }

	//Set Angle
	ObjMgr->GetHumanObject(ctrlid)->SetRxRy(rx, ry);

	//Dampen rotational speed.
	addrx *= 0.8f;
	addry *= 0.8f;

	//Correction to 0.0f
	if( fabsf(addrx) < DegreeToRadian(0.2f) ){ addrx = 0.0f; }
	if( fabsf(addry) < DegreeToRadian(0.2f) ){ addry = 0.0f; }
}