//! @file collision.cpp
//! @brief Definition of Collision class

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

#include "collision.h"

//! @brief Constructor
Collision::Collision()
{
	BlockDataIF = NULL;
	cbdata = new Coll_Blockdata[MAX_BLOCKS];
	for(int i=0; i<MAX_BLOCKS; i++){
		cbdata[i].min_x = 0.0f;
		cbdata[i].min_y = 0.0f;
		cbdata[i].min_z = 0.0f;
		cbdata[i].max_x = 0.0f;
		cbdata[i].max_y = 0.0f;
		cbdata[i].max_z = 0.0f;
		cbdata[i].BoardBlock = false;
		cbdata[i].worldgroup = 0;
	}
}

//! @brief Destructor
Collision::~Collision()
{
	if( cbdata != NULL ){ delete [] cbdata; }
}

//! @brief Import block data
//! @param in_BlockDataIF Block Data
int Collision::InitCollision(BlockDataInterface* in_BlockDataIF)
{
	int bs;
	blockdata data;
	int vID[4];

	if( in_BlockDataIF == NULL ){ return 1; }
	BlockDataIF = in_BlockDataIF;

	bs = BlockDataIF->GetTotaldatas();

	for(int i=0; i<bs; i++){
		cbdata[i].BoardBlock = false;
	}

	for(int i=0; i<bs; i++){
		float cx = 0.0f;
		float cy = 0.0f;
		float cz = 0.0f;
		float x, y, z, rx, ry, r;

		BlockDataIF->Getdata(&data, i);

		//Seeking the center
		for(int j=0; j<8; j++){
			cx += data.x[j];
			cy += data.y[j];
			cz += data.z[j];
		}
		cx /= 8;
		cy /= 8;
		cz /= 8;

		//Recalculate the coordinates of each vertex.
		for(int j=0; j<8; j++){
			//Calculate the position relative to the center.
			x = data.x[j] - cx;
			y = data.y[j] - cy;
			z = data.z[j] - cz;
			rx = atan2f(z, x);
			ry = atan2f(y, sqrtf(x*x + z*z));
			r = sqrtf(x*x + y*y + z*z);

			//Enlarge.
			r += COLLISION_ADDSIZE;

			//Recalculate coordinates.
			cbdata[i].x[j] = cx + cosf(rx) * cosf(ry) * r;
			cbdata[i].y[j] = cy + sinf(ry) * r;
			cbdata[i].z[j] = cz + sinf(rx) * cosf(ry) * r;
		}
	}

	//Calculate the center of the plane used for collision tests.
	for(int i=0; i<bs; i++){
		for(int j=0; j<6; j++){
			blockdataface(j, &vID[0], NULL);

			cbdata[i].polygon_center_x[j] = 0.0f;
			cbdata[i].polygon_center_y[j] = 0.0f;
			cbdata[i].polygon_center_z[j] = 0.0f;

			//Calculate the center of the four vertices.
			for(int k=0; k<4; k++){
				cbdata[i].polygon_center_x[j] += cbdata[i].x[ vID[k] ];
				cbdata[i].polygon_center_y[j] += cbdata[i].y[ vID[k] ];
				cbdata[i].polygon_center_z[j] += cbdata[i].z[ vID[k] ];
			}
			cbdata[i].polygon_center_x[j] /= 4;
			cbdata[i].polygon_center_y[j] /= 4;
			cbdata[i].polygon_center_z[j] /= 4;
		}
	}

	//Detecting Board Blocks
	for(int i=0; i<bs; i++){
		cbdata[i].BoardBlock = false;

		for(int j=0; j<8; j++){
			for(int k=j+1; k<8; k++){
				//If the vertex is the same in one place, it is judged that it is a board shape.
				if( (cbdata[i].x[j] == cbdata[i].x[k])&&(cbdata[i].y[j] == cbdata[i].y[k])&&(cbdata[i].z[j] == cbdata[i].z[k]) ){
					cbdata[i].BoardBlock = true;
					j = 8; k = 8;	//break
				}
			}
		}

		//Calculate the center of the block.
		float mx = 0.0f;
		float my = 0.0f;
		float mz = 0.0f;
		for(int j=0; j<8; j++){
			mx += cbdata[i].x[j];
			my += cbdata[i].y[j];
			mz += cbdata[i].z[j];
		}
		mx /= 8;
		my /= 8;
		mz /= 8;

		//It is judged that it is a board shape if it is a surface even on one side to the center point of the block.
		for(int j=0; j<6; j++){
			if( CheckPolygonFront(i, j, mx, my, mz) == true ){
				cbdata[i].BoardBlock = true;
				break;
			}
		}
	}


	//Create the block AABB.
	for(int i=0; i<bs; i++){
		GetBlockPosMINMAX(i, &cbdata[i].min_x, &cbdata[i].min_y, &cbdata[i].min_z, &cbdata[i].max_x, &cbdata[i].max_y, &cbdata[i].max_z);
	}

	//Block spatial split group
	for(int i=0; i<bs; i++){
		cbdata[i].worldgroup = GetWorldGroup(cbdata[i].min_x, cbdata[i].min_z);
		if( GetWorldGroup(cbdata[i].max_x, cbdata[i].max_z) != cbdata[i].worldgroup ){
			cbdata[i].worldgroup = 0;
		}
	}

	return 0;
}

//! @brief Returns the coordinate maximum and minimum value of the block
//! @param id Block number to determine
//! @param *min_x Pointer to cut the minimum X coordinates
//! @param *min_y Pointer to cut the minimum Y coordinates
//! @param *min_z Pointer to apply minimum Z coordinates
//! @param *max_x Pointer cutting max X coordinates
//! @param *max_y Pointer cutting maximum Y coordinates
//! @param *max_z Pointer cutting maximum Z coordinates
void Collision::GetBlockPosMINMAX(int id, float *min_x, float *min_y, float *min_z, float *max_x, float *max_y, float *max_z)
{
	*min_x = cbdata[id].x[0];
	*min_y = cbdata[id].y[0];
	*min_z = cbdata[id].z[0];
	*max_x = cbdata[id].x[0];
	*max_y = cbdata[id].y[0];
	*max_z = cbdata[id].z[0];
	for(int i=1; i<8; i++){
		if( *min_x > cbdata[id].x[i] ){ *min_x = cbdata[id].x[i]; }
		if( *min_y > cbdata[id].y[i] ){ *min_y = cbdata[id].y[i]; }
		if( *min_z > cbdata[id].z[i] ){ *min_z = cbdata[id].z[i]; }

		if( *max_x < cbdata[id].x[i] ){ *max_x = cbdata[id].x[i]; }
		if( *max_y < cbdata[id].y[i] ){ *max_y = cbdata[id].y[i]; }
		if( *max_z < cbdata[id].z[i] ){ *max_z = cbdata[id].z[i]; }
	}

	//Slightly large for counter s against calculation errors.
	*min_x -= COLLISION_ADDSIZE;
	*min_y -= COLLISION_ADDSIZE;
	*min_z -= COLLISION_ADDSIZE;
	*max_x += COLLISION_ADDSIZE;
	*max_y += COLLISION_ADDSIZE;
	*max_z += COLLISION_ADDSIZE;
}

//! @brief Calculate the spatial-partition group.
//! @param x X Coordinate
//! @param z Z Coordinates
//! @return Group number
//! @attention The concept of "group number" refers to the document separately
int Collision::GetWorldGroup(float x, float z)
{
	if( (x > 0)&&(z > 0) ){ return 1; }
	if( (x < 0)&&(z > 0) ){ return 2; }
	if( (x < 0)&&(z < 0) ){ return 3; }
	if( (x > 0)&&(z < 0) ){ return 4; }
	return 0;
}

//! @brief Check collision between an AABB and a block.
//! @param id Block number to determine
//! @param min_x Min. X coordinates of objects
//! @param min_y Min. Y coordinates of the object
//! @param min_z Min. Z coordinates of objects
//! @param max_x Max. X coordinates of objects
//! @param max_y Maximum of objects Y coordinates
//! @param max_z Maximum of objects Z coordinates
//! @return Colliding: true; not colliding: false.
//! @attention If there is an error, "Not hit: false" will be returned.
bool Collision::CheckBlockAABB(int id, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
{
	if( BlockDataIF == NULL ){ return false; }
	if( (id < 0)||(BlockDataIF->GetTotaldatas() <= id) ){ return false; }

	//Board block is not calculated
	if( cbdata[id].BoardBlock == true ){
		return false;
	}

	return CollideBoxAABB(min_x, min_y, min_z, max_x, max_y, max_z, cbdata[id].min_x, cbdata[id].min_y, cbdata[id].min_z, cbdata[id].max_x, cbdata[id].max_y, cbdata[id].max_z);
}

//! @brief Check collision between an AABB and all blocks.
//! @param min_x Min. X coordinates of objects
//! @param min_y Min. Y coordinates of the object
//! @param min_z Min. Z coordinates of objects
//! @param max_x Max. X coordinates of objects
//! @param max_y Maximum of objects Y coordinates
//! @param max_z Maximum of objects Z coordinates
//! @return Colliding: true; not colliding: false.
//! @attention If there is an error, "Not hit: false" will be returned.
bool Collision::CheckALLBlockAABB(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
{
	if( BlockDataIF == NULL ){ return false; }

	int bs = BlockDataIF->GetTotaldatas();

	for(int i=0; i<bs; i++){
		if( CheckBlockAABB(i, min_x, min_y, min_z, max_x, max_y, max_z) == true ){ return true; }
	}

	return false;
}

//! @brief Check whether a specific coordinate is the front side of the block
//! @param id Block number to determine
//! @param face Face number to determine
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @return Front-facing: true; back-facing: false.
//! @warning You need to specify absolute coordinates, not vectors.
//! @attention Called automatically by the collision-detection functions.
bool Collision::CheckPolygonFront(int id, int face, float x, float y, float z)
{
	if( BlockDataIF == NULL ){ return false; }
	if( (id < 0)||(BlockDataIF->GetTotaldatas() <= id) ){ return false; }
	if( (face < 0)||(6 < face) ){ return false; }

	blockdata bdata;
	float vx, vy, vz;
	float d;

	BlockDataIF->Getdata(&bdata, id);

	//Focus on the center of the face, and seek the coordinate vector
	vx = cbdata[id].polygon_center_x[face] - x;
	vy = cbdata[id].polygon_center_y[face] - y;
	vz = cbdata[id].polygon_center_z[face] - z;

	//Dot product.
	d = bdata.material[face].vx*vx + bdata.material[face].vy*vy + bdata.material[face].vz*vz;

	if( d <= 0.0f ){
		return true;
	}
	return false;
}

//! @brief Check whether the surface of the block is on the surface for a specific direction
//! @param id Block number to determine
//! @param face Face number to determine
//! @param rx lateral angle
//! @return Front-facing: true; back-facing: false.
bool Collision::CheckPolygonFrontRx(int id, int face, float rx)
{
	if( BlockDataIF == NULL ){ return false; }
	if( (id < 0)||(BlockDataIF->GetTotaldatas() <= id) ){ return false; }
	if( (face < 0)||(6 < face) ){ return false; }

	blockdata bdata;
	float vx, vz;
	float d;

	BlockDataIF->Getdata(&bdata, id);

	//Calculate the vector.
	vx = cosf(rx);
	vz = sinf(rx);

	//Dot product.
	d = bdata.material[face].vx*vx + bdata.material[face].vz*vz;

	if( d <= 0.0f ){
		return true;
	}
	return false;
}

//! @brief Determination of the surface of the block and the ray (light)
//! @param blockid Block number to determine
//! @param face Face number to determine
//! @param RayPos_x Specify the position of the ray (first point) X coordinate
//! @param RayPos_y Specify the position of the ray (first point) Y coordinate
//! @param RayPos_z Specify the position of the ray (first point) Z coordinate
//! @param RayDir_x Specify the vector of the ray X component
//! @param RayDir_y Specify the vector of the ray Y component
//! @param RayDir_z Specify the vector of the ray Z component
//! @param out_Dist Pointer to receive the distance from the block.
//! @return Colliding: true; not colliding: false.
//! @warning Do not confuse RayPos (the start point) with RayDir (the direction vector).
//! @attention Called automatically by the collision-detection functions.
bool Collision::CheckIntersectTri(int blockid, int face, float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z, float *out_Dist)
{
	blockdata data;
	int vID[4];
	float d1, d2;
	float vx1, vy1, vz1;
	float dist;
	float x, y, z;
	float vx2, vy2, vz2;

	BlockDataIF->Getdata(&data, blockid);
	blockdataface(face, vID, NULL);

	//Dot product.
	d1 = data.material[face].vx*RayDir_x + data.material[face].vy*RayDir_y + data.material[face].vz*RayDir_z;

	if( d1 >= 0.0f ){
		return false;		//The face and the rays are d, or the rays are opposite to the face.
	}

	//Focus on the center of the face, and seek the coordinate vector
	vx1 = RayPos_x - cbdata[blockid].polygon_center_x[face];
	vy1 = RayPos_y - cbdata[blockid].polygon_center_y[face];
	vz1 = RayPos_z - cbdata[blockid].polygon_center_z[face];

	//Dot product.
	d2 = data.material[face].vx*vx1 + data.material[face].vy*vy1 + data.material[face].vz*vz1;		//The shortest distance to the surface

	//Distance and coordinates to crossing points
	dist = 1.0f / (d1*-1) * d2;
	x = RayPos_x + RayDir_x * dist;
	y = RayPos_y + RayDir_y * dist;
	z = RayPos_z + RayDir_z * dist;


	//ed if the entire block is not included in the A
	if( (x < cbdata[blockid].min_x)||(cbdata[blockid].max_x < x) ){ return false; }
	if( (y < cbdata[blockid].min_y)||(cbdata[blockid].max_y < y) ){ return false; }
	if( (z < cbdata[blockid].min_z)||(cbdata[blockid].max_z < z) ){ return false; }


	//Check whether there is a crossing point inside the block surface
	//  Calculate the position relative to each of the face's four edges and compare it with the face normal.

	//Cross product.
	vx2 = ((cbdata[blockid].y[ vID[1] ] - cbdata[blockid].y[ vID[0] ]) * (z - cbdata[blockid].z[ vID[0] ])) - ((y - cbdata[blockid].y[ vID[0] ]) * (cbdata[blockid].z[ vID[1] ] - cbdata[blockid].z[ vID[0] ]));
	vy2 = ((cbdata[blockid].z[ vID[1] ] - cbdata[blockid].z[ vID[0] ]) * (x - cbdata[blockid].x[ vID[0] ])) - ((z - cbdata[blockid].z[ vID[0] ]) * (cbdata[blockid].x[ vID[1] ] - cbdata[blockid].x[ vID[0] ]));
	vz2 = ((cbdata[blockid].x[ vID[1] ] - cbdata[blockid].x[ vID[0] ]) * (y - cbdata[blockid].y[ vID[0] ])) - ((x - cbdata[blockid].x[ vID[0] ]) * (cbdata[blockid].y[ vID[1] ] - cbdata[blockid].y[ vID[0] ]));

	//Dot product.
	d1 = data.material[face].vx*vx2 + data.material[face].vy*vy2 + data.material[face].vz*vz2;		//Calculate the relationship to the block face normal.

	if( d1 < 0.0f ){	//Exclude points outside the face.
		return false;
	}


	//Cross product.
	vx2 = ((cbdata[blockid].y[ vID[2] ] - cbdata[blockid].y[ vID[1] ]) * (z - cbdata[blockid].z[ vID[1] ])) - ((y - cbdata[blockid].y[ vID[1] ]) * (cbdata[blockid].z[ vID[2] ] - cbdata[blockid].z[ vID[1] ]));
	vy2 = ((cbdata[blockid].z[ vID[2] ] - cbdata[blockid].z[ vID[1] ]) * (x - cbdata[blockid].x[ vID[1] ])) - ((z - cbdata[blockid].z[ vID[1] ]) * (cbdata[blockid].x[ vID[2] ] - cbdata[blockid].x[ vID[1] ]));
	vz2 = ((cbdata[blockid].x[ vID[2] ] - cbdata[blockid].x[ vID[1] ]) * (y - cbdata[blockid].y[ vID[1] ])) - ((x - cbdata[blockid].x[ vID[1] ]) * (cbdata[blockid].y[ vID[2] ] - cbdata[blockid].y[ vID[1] ]));

	//Dot product.
	d1 = data.material[face].vx*vx2 + data.material[face].vy*vy2 + data.material[face].vz*vz2;		//Calculate the relationship to the block face normal.

	if( d1 < 0.0f ){	//Exclude points outside the face.
		return false;
	}


	//Cross product.
	vx2 = ((cbdata[blockid].y[ vID[3] ] - cbdata[blockid].y[ vID[2] ]) * (z - cbdata[blockid].z[ vID[2] ])) - ((y - cbdata[blockid].y[ vID[2] ]) * (cbdata[blockid].z[ vID[3] ] - cbdata[blockid].z[ vID[2] ]));
	vy2 = ((cbdata[blockid].z[ vID[3] ] - cbdata[blockid].z[ vID[2] ]) * (x - cbdata[blockid].x[ vID[2] ])) - ((z - cbdata[blockid].z[ vID[2] ]) * (cbdata[blockid].x[ vID[3] ] - cbdata[blockid].x[ vID[2] ]));
	vz2 = ((cbdata[blockid].x[ vID[3] ] - cbdata[blockid].x[ vID[2] ]) * (y - cbdata[blockid].y[ vID[2] ])) - ((x - cbdata[blockid].x[ vID[2] ]) * (cbdata[blockid].y[ vID[3] ] - cbdata[blockid].y[ vID[2] ]));

	//Dot product.
	d1 = data.material[face].vx*vx2 + data.material[face].vy*vy2 + data.material[face].vz*vz2;		//Calculate the relationship to the block face normal.

	if( d1 < 0.0f ){	//Exclude points outside the face.
		return false;
	}


	//Cross product.
	vx2 = ((cbdata[blockid].y[ vID[0] ] - cbdata[blockid].y[ vID[3] ]) * (z - cbdata[blockid].z[ vID[3] ])) - ((y - cbdata[blockid].y[ vID[3] ]) * (cbdata[blockid].z[ vID[0] ] - cbdata[blockid].z[ vID[3] ]));
	vy2 = ((cbdata[blockid].z[ vID[0] ] - cbdata[blockid].z[ vID[3] ]) * (x - cbdata[blockid].x[ vID[3] ])) - ((z - cbdata[blockid].z[ vID[3] ]) * (cbdata[blockid].x[ vID[0] ] - cbdata[blockid].x[ vID[3] ]));
	vz2 = ((cbdata[blockid].x[ vID[0] ] - cbdata[blockid].x[ vID[3] ]) * (y - cbdata[blockid].y[ vID[3] ])) - ((x - cbdata[blockid].x[ vID[3] ]) * (cbdata[blockid].y[ vID[0] ] - cbdata[blockid].y[ vID[3] ]));

	//Dot product.
	d1 = data.material[face].vx*vx2 + data.material[face].vy*vy2 + data.material[face].vz*vz2;		//Calculate the relationship to the block face normal.

	if( d1 < 0.0f ){	//Exclude points outside the face.
		return false;
	}


	*out_Dist = dist;

	return true;
}

//! @brief Check if the block is not buried
//! @param blockid Block number to determine
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param worldgroup Use a group of spaces to attempt to omit calculations (true: enabled / omit calculation false: disabled / full search)
//! @param *planeid Front-facing face index (NULL allowed).
//! @return buried: true unembedded: false
//! @warning If multiple faces match, *planeid returns only the first matching front-facing face (0 to 5).
bool Collision::CheckBlockInside(int blockid, float x, float y, float z, bool worldgroup, int *planeid)
{
	if( BlockDataIF == NULL ){ return false; }
	if( (blockid < 0)||(BlockDataIF->GetTotaldatas() <= blockid) ){ return false; }

	//Board block is not calculated
	if( cbdata[blockid].BoardBlock == true ){ return false; }


	//Broad-phase collision filtering.
	if( worldgroup == true ){

		//Space Split
		if( cbdata[blockid].worldgroup != 0 ){
			//Get the spatial group containing the observation point.
			int worldgroup = GetWorldGroup(x, z);

			if( worldgroup != 0 ){
				//If the group of space is different, it is not computed
				if( cbdata[blockid].worldgroup != worldgroup ){
					return false;
				}
			}
		}

		//Search by Range
		if( (x < cbdata[blockid].min_x)||(cbdata[blockid].max_x < x) ){ return false; }
		if( (y < cbdata[blockid].min_y)||(cbdata[blockid].max_y < y) ){ return false; }
		if( (z < cbdata[blockid].min_z)||(cbdata[blockid].max_z < z) ){ return false; }
	}


	//6 all the back side view from 6 side
	for(int i=0; i<6; i++){
		if( CheckPolygonFront(blockid, i, x, y, z) == true ){
			if( planeid != NULL ){ *planeid = i; }
			return false;	//Return if this is the front surface.
		}
	}

	return true;
}

//! @brief Check if all blocks are not buried
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @return buried: true unembedded: false
bool Collision::CheckALLBlockInside(float x, float y, float z)
{
	if( BlockDataIF == NULL ){ return false; }

	int bs = BlockDataIF->GetTotaldatas();

	for(int i=0; i<bs; i++){
		if( CheckBlockInside(i, x, y, z, true, NULL) == true ){ return true; }
	}

	return false;
}

//! @brief Blocks and Rays
//! @param blockid Block number to determine
//! @param RayPos_x Specify the position of the ray (first point) X coordinate
//! @param RayPos_y Specify the position of the ray (first point) Y coordinate
//! @param RayPos_z Specify the position of the ray (first point) Z coordinate
//! @param RayDir_x Specify the vector of the ray X component
//! @param RayDir_y Specify the vector of the ray Y component
//! @param RayDir_z Specify the vector of the ray Z component
//! @param face Pointer receiving the hit block face index (0 to 5; NULL allowed).
//! @param Dist Pointer to receive the distance from the block.
//! @param maxDist Maximum distance to perform judgment (invalid or endless in less than 0.0)
//! @return Colliding: true; not colliding: false.
//! @warning Do not confuse RayPos (the start point) with RayDir (the direction vector).
//! @warning If the maximum distance is not specified, the performance will be drastically reduced.
//! @attention Ignore back-facing block faces relative to the ray start, as well as flat blocks with zero thickness.
//! @attention In addition, if the ray hits multiple blocks, it returns the closest judgment from the first point of the ray.
bool Collision::CheckBlockIntersectRay(int blockid, float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z, int *face, float *Dist, float maxDist)
{
	if( BlockDataIF == NULL ){ return false; }
	if( (blockid < 0)||(BlockDataIF->GetTotaldatas() <= blockid) ){ return false; }
	if( (RayDir_x == 0.0f)&&(RayDir_y == 0.0f)&&(RayDir_z == 0.0f) ){ return false; }

	float pDist;
	float min_pDist = FLT_MAX;
	int min_blockface = -1;
	float rmin_x = 0.0f;
	float rmin_y = 0.0f;
	float rmin_z = 0.0f;
	float rmax_x = 0.0f;
	float rmax_y = 0.0f;
	float rmax_z = 0.0f;
	int worldgroupA = 0;
	int worldgroupB = 0;

	//Board block is not calculated
	if( cbdata[blockid].BoardBlock == true ){
		if( face != NULL ){ *face = 0; }
		*Dist = 0.0f;
		return false;
	}

	if( maxDist > 0.0f ){
		//Create the ray AABB.
		GetAABBRay(RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z, maxDist, &rmin_x, &rmin_y, &rmin_z, &rmax_x, &rmax_y, &rmax_z);
	}

	if( maxDist > 0.0f ){
		//Get the spatial groups containing the start and end points.
		worldgroupA = GetWorldGroup(RayPos_x, RayPos_z);
		worldgroupB = GetWorldGroup(RayPos_x + RayDir_x * maxDist, RayPos_z + RayDir_z * maxDist);
	}

	if( maxDist > 0.0f ){
		if( cbdata[blockid].worldgroup != 0 ){
			//If the first point and end point are not coming out of the space group
			if( (worldgroupA != 0)&&(worldgroupA == worldgroupB) ){

				//If the group of space is different, it is not computed
				if( cbdata[blockid].worldgroup == worldgroupA ){		//worldgroupA == worldgroupB
					//Determined by the border box
					if( CollideBoxAABB(cbdata[blockid].min_x, cbdata[blockid].min_y, cbdata[blockid].min_z, cbdata[blockid].max_x, cbdata[blockid].max_y, cbdata[blockid].max_z, rmin_x, rmin_y, rmin_z, rmax_x, rmax_y, rmax_z) == false ){
						//If not, this block is not checked.
						if( face != NULL ){ *face = 0; }
						*Dist = 0.0f;
						return false;
					}
				}
			}
		}
	}

	//Boundary box and lay
	if( CollideAABBRay(cbdata[blockid].min_x, cbdata[blockid].min_y, cbdata[blockid].min_z, cbdata[blockid].max_x, cbdata[blockid].max_y, cbdata[blockid].max_z, RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z, NULL, -1.0f) == false ){
		//If not, this block is not checked.
		if( face != NULL ){ *face = 0; }
		*Dist = 0.0f;
		return false;
	}

	//Determined by each polygon
	for(int i=0; i<6; i++){
		if( CheckPolygonFront(blockid, i, RayPos_x, RayPos_y, RayPos_z) == true ){
			if( CheckIntersectTri(blockid, i, RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z, &pDist) == true ){
				if( min_pDist > pDist ){
					min_pDist = pDist;
					min_blockface = i;
				}
			}
		}
	}

	//If the distance found exceeds the maximum distance, please check the judgment.
	if( maxDist >= 0.0f ){
		if( min_pDist > maxDist ){
			min_blockface = -1;
		}
	}

	//If not found, put the appropriate number in the pointer.
	if( min_blockface == -1 ){
		if( face != NULL ){ *face = 0; }
		*Dist = 0.0f;
		return false;
	}

	//Put the calculation result
	if( face != NULL ){ *face = min_blockface; }
	*Dist = min_pDist;
	return true;
}

//! @brief All blocks and rays
//! @param RayPos_x Specify the position of the ray (first point) X coordinate
//! @param RayPos_y Specify the position of the ray (first point) Y coordinate
//! @param RayPos_z Specify the position of the ray (first point) Z coordinate
//! @param RayDir_x Specify the vector of the ray X component
//! @param RayDir_y Specify the vector of the ray Y component
//! @param RayDir_z Specify the vector of the ray Z component
//! @param id Pointer receiving the hit block ID (NULL allowed).
//! @param face Pointer receiving the hit block face index (0 to 5; NULL allowed).
//! @param Dist Pointer to receive the distance from the block.
//! @param maxDist Maximum distance to perform judgment (invalid or endless in less than 0.0)
//! @return Colliding: true; not colliding: false.
//! @warning Do not confuse RayPos (the start point) with RayDir (the direction vector).
//! @warning If the maximum distance is not specified, the performance will be drastically reduced.
//! @warning Similar to CheckALLBlockIntersectDummyRay(), but more accurate and slower.
//! @attention Ignore back-facing block faces relative to the ray start, as well as flat blocks with zero thickness.
//! @attention In addition, if the ray hits multiple blocks, it returns the closest judgment </b> from the start point of the ray.
bool Collision::CheckALLBlockIntersectRay(float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z, int *id, int *face, float *Dist, float maxDist)
{
	if( BlockDataIF == NULL ){ return false; }
	if( (RayDir_x == 0.0f)&&(RayDir_y == 0.0f)&&(RayDir_z == 0.0f) ){ return false; }

	int bs = BlockDataIF->GetTotaldatas();
	float pDist;
	float min_pDist = FLT_MAX;
	int min_blockid = -1;
	int min_blockface = -1;
	float rmin_x = 0.0f;
	float rmin_y = 0.0f;
	float rmin_z = 0.0f;
	float rmax_x = 0.0f;
	float rmax_y = 0.0f;
	float rmax_z = 0.0f;
	int worldgroupA = 0;
	int worldgroupB = 0;

	if( maxDist > 0.0f ){
		//Create the ray AABB.
		GetAABBRay(RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z, maxDist, &rmin_x, &rmin_y, &rmin_z, &rmax_x, &rmax_y, &rmax_z);
	}

	if( maxDist > 0.0f ){
		//Get the spatial groups containing the start and end points.
		worldgroupA = GetWorldGroup(RayPos_x, RayPos_z);
		worldgroupB = GetWorldGroup(RayPos_x + RayDir_x * maxDist, RayPos_z + RayDir_z * maxDist);
	}
	
	for(int i=0; i<bs; i++){
		//Board block is not calculated
		if( cbdata[i].BoardBlock == true ){ continue; }

		if( maxDist > 0.0f ){
			if( cbdata[i].worldgroup != 0 ){
				//If the first point and end point are not coming out of the space group
				if( (worldgroupA != 0)&&(worldgroupA == worldgroupB) ){

					//If the group of space is different, it is not computed
					if( cbdata[i].worldgroup == worldgroupA ){		//worldgroupA == worldgroupB

						//Determined by the border box
						if( CollideBoxAABB(cbdata[i].min_x, cbdata[i].min_y, cbdata[i].min_z, cbdata[i].max_x, cbdata[i].max_y, cbdata[i].max_z, rmin_x, rmin_y, rmin_z, rmax_x, rmax_y, rmax_z) == false ){
							continue;	//If not, this block is not checked.
						}

					}
				}
			}
		}

		//Boundary box and lay
		if( CollideAABBRay(cbdata[i].min_x, cbdata[i].min_y, cbdata[i].min_z, cbdata[i].max_x, cbdata[i].max_y, cbdata[i].max_z, RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z, NULL, -1.0f) == false ){
			continue;		//If not, this block is not checked.
		}

		//Determined by each polygon
		for(int j=0; j<6; j++){
			if( CheckPolygonFront(i, j, RayPos_x, RayPos_y, RayPos_z) == true ){
				if( CheckIntersectTri(i, j, RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z, &pDist) == true ){
					if( min_pDist > pDist ){
						min_pDist = pDist;
						min_blockid = i;
						min_blockface = j;
					}
				}
			}
		}
	}

	//If the distance found exceeds the maximum distance, please check the judgment.
	if( maxDist >= 0.0f ){
		if( min_pDist > maxDist ){
			min_blockid = -1;
		}
	}

	//If not found, put the appropriate number in the pointer.
	if( min_blockid == -1 ){
		if( id != NULL ){ *id = 0; }
		if( face != NULL ){ *face = 0; }
		*Dist = 0.0f;
		return false;
	}

	//Put the calculation result
	if( id != NULL ){ *id = min_blockid; }
	if( face != NULL ){ *face = min_blockface; }
	*Dist = min_pDist;
	return true;
}

//! @brief Determining whether or not a block is collided with
//! @param RayPos_x Start point X coordinates
//! @param RayPos_y Start point Y coordinates
//! @param RayPos_z Start point Z coordinate
//! @param RayDir_x X component of the vector.
//! @param RayDir_y Y component of the vector.
//! @param RayDir_z Z component of the vector.
//! @param id Pointer receiving the hit block ID (NULL allowed).
//! @param face Pointer receiving the hit block face index (0 to 5; NULL allowed).
//! @param Dist Pointer of dummy variables (always return 0.0f)
//! @param maxDist Maximum distance to perform judgment (not specified below 0.0)
//! @return Colliding: true; not colliding: false.
//! @warning Don't make mistakes between the beginning and the vector.
//! @warning Similar to CheckALLBlockIntersectRay(), but faster and less accurate.
//! @attention Ignore back-facing block faces relative to the ray start, as well as flat blocks with zero thickness.
//! @attention In addition, if the ray hits multiple blocks, <b> returns the first found judgment </b>.
bool Collision::CheckALLBlockIntersectDummyRay(float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z, int *id, int *face, float *Dist, float maxDist)
{
	if( BlockDataIF == NULL ){ return false; }
	if( maxDist <= 0.0f ){ return false; }

	float RayPos2_x = RayPos_x + RayDir_x * maxDist/2;
	float RayPos2_y = RayPos_y + RayDir_y * maxDist/2;
	float RayPos2_z = RayPos_z + RayDir_z * maxDist/2;
	float RayPos3_x = RayPos_x + RayDir_x * maxDist;
	float RayPos3_y = RayPos_y + RayDir_y * maxDist;
	float RayPos3_z = RayPos_z + RayDir_z * maxDist;
	
	if( (id == NULL)&&(face == NULL) ){
		for(int i=0; i<MAX_BLOCKS; i++){
			//At the end
			if( CheckBlockInside(i, RayPos3_x, RayPos3_y, RayPos3_z, true, NULL) == true ){
				*Dist = 0.0f;
				return true;
			}

			//Mid-term
			if( CheckBlockInside(i, RayPos2_x, RayPos2_y, RayPos2_z, true, NULL) == true ){
				*Dist = 0.0f;
				return true;
			}
		}

		*Dist = 0.0f;
		return false;
	}

	for(int i=0; i<MAX_BLOCKS; i++){
		int surface;

		//Start point
		CheckBlockInside(i, RayPos_x, RayPos_y, RayPos_z, false, &surface);

		//At the end
		if( CheckBlockInside(i, RayPos3_x, RayPos3_y, RayPos3_z, true, &surface) == true ){
			if( id != NULL ){ *id = i; }
			if( face != NULL ){ *face = surface; }
			*Dist = 0.0f;
			return true;
		}

		//Mid-term
		if( CheckBlockInside(i, RayPos2_x, RayPos2_y, RayPos2_z, true, &surface) == true ){
			if( id != NULL ){ *id = i; }
			if( face != NULL ){ *face = surface; }
			*Dist = 0.0f;
			return true;
		}
	}

	if( id != NULL ){ *id = 0; }
	if( face != NULL ){ *face = 0; }
	*Dist = 0.0f;
	return false;
}

//! @brief Seeking vectors moving along blocks
void Collision::ScratchVector(int id, int face, float in_vx, float in_vy, float in_vz, float *out_vx, float *out_vy, float *out_vz)
{
	if( BlockDataIF == NULL ){ return; }
	if( (id < 0)||(BlockDataIF->GetTotaldatas() <= id) ){ return; }
	if( (face < 0)||(6 < face) ){ return; }

	blockdata bdata;
	BlockDataIF->Getdata(&bdata, id);

	//Dot product.
	float Dot = in_vx * bdata.material[face].vx + in_vy * bdata.material[face].vy + in_vz * bdata.material[face].vz;

	*out_vx = in_vx - Dot * bdata.material[face].vx;
	*out_vy = in_vy - Dot * bdata.material[face].vy;
	*out_vz = in_vz - Dot * bdata.material[face].vz;
}

//! @brief Seeking vectors reflected in blocks
void Collision::ReflectVector(int id, int face, float in_vx, float in_vy, float in_vz, float *out_vx, float *out_vy, float *out_vz)
{
	if( BlockDataIF == NULL ){ return; }
	if( (id < 0)||(BlockDataIF->GetTotaldatas() <= id) ){ return; }
	if( (face < 0)||(6 < face) ){ return; }

	blockdata bdata;
	BlockDataIF->Getdata(&bdata, id);

	//Dot product.
	float Dot = in_vx * bdata.material[face].vx + in_vy * bdata.material[face].vy + in_vz * bdata.material[face].vz;

	*out_vx = in_vx - 2.0f * Dot * bdata.material[face].vx;
	*out_vy = in_vy - 2.0f * Dot * bdata.material[face].vy;
	*out_vz = in_vz - 2.0f * Dot * bdata.material[face].vz;
}

//! @brief Find the angle at which the vector enters the block.
//! @return success: true failure: false
//! @attention Front-facing: 0.0 to PI/2 (0 to 90 degrees); back-facing: -0.0 to -PI/2 (0 to -90 degrees).
bool Collision::AngleVector(int id, int face, float in_vx, float in_vy, float in_vz, float *out_angle)
{
	if( BlockDataIF == NULL ){ return false; }
	if( (id < 0)||(BlockDataIF->GetTotaldatas() <= id) ){ return false; }
	if( (face < 0)||(6 < face) ){ return false; }
	if( out_angle == NULL ){ return false; }

	if( (in_vx == 0.0f)&&(in_vy == 0.0f)&&(in_vz == 0.0f) ){ return false; }

	blockdata bdata;
	BlockDataIF->Getdata(&bdata, id);

	VectorNormalization(&in_vx, &in_vy, &in_vz);

	//Dot product.
	float Dot = in_vx * bdata.material[face].vx + in_vy * bdata.material[face].vy + in_vz * bdata.material[face].vz;

	//Seeking Angle
	*out_angle = asinf(Dot) * -1;

	return true;
}

//! @brief Seeking the corners of vectors for blocks
//! @return success: true failure: false
//! @attention Angle between vectors: 0.0 to PI (0 to 180 degrees).
bool Collision::AngleVector2(int id, int face, float in_vx, float in_vy, float in_vz, float *out_angle)
{
	if( BlockDataIF == NULL ){ return false; }
	if( (id < 0)||(BlockDataIF->GetTotaldatas() <= id) ){ return false; }
	if( (face < 0)||(6 < face) ){ return false; }
	if( out_angle == NULL ){ return false; }

	if( (in_vx == 0.0f)&&(in_vy == 0.0f)&&(in_vz == 0.0f) ){ return false; }

	blockdata bdata;
	BlockDataIF->Getdata(&bdata, id);

	//Dot product.
	float Dot = in_vx * bdata.material[face].vx + in_vy * bdata.material[face].vy + in_vz * bdata.material[face].vz;

	//Exclude back-facing cases.
	if( Dot >= 0.0f ){ return false; }

	//Calculate the angle between the vectors.
	*out_angle = acosf(Dot);

	return true;
}

//! @brief Seeking Ray's A
//! @param RayPos_x Start point X coordinates
//! @param RayPos_y Start point Y coordinates
//! @param RayPos_z Start point Z coordinate
//! @param RayDir_x X component of the vector.
//! @param RayDir_y Y component of the vector.
//! @param RayDir_z Z component of the vector.
//! @param maxDist Maximum distance to judge
//! @param *min_x Pointer to cut the minimum X coordinates
//! @param *min_y Pointer to cut the minimum Y coordinates
//! @param *min_z Pointer to apply minimum Z coordinates
//! @param *max_x Pointer cutting max X coordinates
//! @param *max_y Pointer cutting maximum Y coordinates
//! @param *max_z Pointer cutting maximum Z coordinates
void GetAABBRay(float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z, float maxDist, float *min_x, float *min_y, float *min_z, float *max_x, float *max_y, float *max_z)
{
	float rmin_x, rmin_y, rmin_z, rmax_x, rmax_y, rmax_z;

	//Create the ray AABB.
	rmin_x = RayPos_x + RayDir_x * maxDist;
	rmin_y = RayPos_y + RayDir_y * maxDist;
	rmin_z = RayPos_z + RayDir_z * maxDist;
	rmax_x = rmin_x;
	rmax_y = rmin_y;
	rmax_z = rmin_z;
	if( rmin_x > RayPos_x ){ rmin_x = RayPos_x; }
	if( rmin_y > RayPos_y ){ rmin_y = RayPos_y; }
	if( rmin_z > RayPos_z ){ rmin_z = RayPos_z; }
	if( rmax_x < RayPos_x ){ rmax_x = RayPos_x; }
	if( rmax_y < RayPos_y ){ rmax_y = RayPos_y; }
	if( rmax_z < RayPos_z ){ rmax_z = RayPos_z; }

	//Slightly large for counter s against calculation errors.
	rmin_x -= COLLISION_ADDSIZE;
	rmin_y -= COLLISION_ADDSIZE;
	rmin_z -= COLLISION_ADDSIZE;
	rmax_x += COLLISION_ADDSIZE;
	rmax_y += COLLISION_ADDSIZE;
	rmax_z += COLLISION_ADDSIZE;

	*min_x = rmin_x;
	*min_y = rmin_y;
	*min_z = rmin_z;
	*max_x = rmax_x;
	*max_y = rmax_y;
	*max_z = rmax_z;
}

//! @brief AABB collision detection.
//! @param box1_min_x Object A Min. X Coordinate
//! @param box1_min_y Min. Y coordinates of object A
//! @param box1_min_z Min. Z Coordinate of Object A
//! @param box1_max_x Maximum X coordinates of object A
//! @param box1_max_y Maximum Y coordinates of object A
//! @param box1_max_z Maximum Z-coordinate of object A
//! @param box2_min_x Smallest X-coordinate of object B
//! @param box2_min_y Min. Y-coordinate of object B
//! @param box2_min_z Min. Z Coordinate of Object B
//! @param box2_max_x Object B max X coordinates
//! @param box2_max_y Maximum Y coordinate of object B
//! @param box2_max_z Maximum Z coordinate of object B
//! @return Colliding: true; not colliding: false.
//! @attention If there is an error, "Not hit: false" will be returned.
bool CollideBoxAABB(float box1_min_x, float box1_min_y, float box1_min_z, float box1_max_x, float box1_max_y, float box1_max_z, float box2_min_x, float box2_min_y, float box2_min_z, float box2_max_x, float box2_max_y, float box2_max_z)
{
	//Handle errors.
	if( box1_min_x > box1_max_x ){ return false; }
	if( box1_min_y > box1_max_y ){ return false; }
	if( box1_min_z > box1_max_z ){ return false; }
	if( box2_min_x > box2_max_x ){ return false; }
	if( box2_min_y > box2_max_y ){ return false; }
	if( box2_min_z > box2_max_z ){ return false; }

	if(
		(box1_min_x < box2_max_x)&&(box1_max_x > box2_min_x)&&
		(box1_min_y < box2_max_y)&&(box1_max_y > box2_min_y)&&
		(box1_min_z < box2_max_z)&&(box1_max_z > box2_min_z)
	){
		return true;
	}

	return false;
}

//! @brief Check collision between cylinders.
//! @param c1_x X coordinate of the base of cylinder 1.
//! @param c1_y Y coordinate of the base of cylinder 1.
//! @param c1_z Z coordinate of the base of cylinder 1.
//! @param c1_r Radius of cylinder 1.
//! @param c1_h Height of cylinder 1.
//! @param c2_x X coordinate of the base of cylinder 2.
//! @param c2_y Y coordinate of the base of cylinder 2.
//! @param c2_z Z coordinate of the base of cylinder 2.
//! @param c2_r Radius of cylinder 2.
//! @param c2_h Height of cylinder 2.
//! @param *angle Pointer receiving the contact angle relative to cylinder 1 (NULL allowed).
//! @param *length Pointer receiving the contact distance relative to cylinder 1 (NULL allowed).
//! @return Colliding: true; not colliding: false.
bool CollideCylinder(float c1_x, float c1_y, float c1_z, float c1_r, float c1_h, float c2_x, float c2_y, float c2_z, float c2_r, float c2_h, float *angle, float *length)
{
	//Determined with a lighter height ahead
	if( (c1_y < c2_y + c2_h)&&(c1_y + c1_h > c2_y) ){

		//Distance judgment
		float x = c1_x - c2_x;
		float z = c1_z - c2_z;
		float caser = x*x + z*z;
		float minr = (c1_r+c2_r) * (c1_r+c2_r);
		if( caser < minr ){
			if( angle != NULL ){ *angle = atan2f(z, x); }
			if( length != NULL ){ *length = sqrtf(minr) - sqrtf(caser); }
			return true;
		}
	}

	return false;
}

//! @brief Determination of sphere and point
//! @param s_x X coordinate of the sphere.
//! @param s_y Y coordinate of the sphere.
//! @param s_z Z coordinate of the sphere.
//! @param s_r Sphere radius.
//! @param p_x X Coordinate
//! @param p_y Y Coordinate
//! @param p_z Z Coordinates
//! @return Colliding: true; not colliding: false.
bool CollideSphereInside(float s_x, float s_y, float s_z, float s_r, float p_x, float p_y, float p_z)
{
	float x, y, z, d;

	//Calculate the distance
	x = s_x - p_x;
	y = s_y - p_y;
	z = s_z - p_z;
	d = x*x + y*y + z*z;

	//The distance is less than the radius.
	if( d < s_r*s_r ){
		return true;
	}

	return false;
}

//! @brief Determination of sphere and rays
//! @param s_x X coordinate of the sphere.
//! @param s_y Y coordinate of the sphere.
//! @param s_z Z coordinate of the sphere.
//! @param s_r Sphere radius.
//! @param RayPos_x Specify the position of the ray (first point) X coordinate
//! @param RayPos_y Specify the position of the ray (first point) Y coordinate
//! @param RayPos_z Specify the position of the ray (first point) Z coordinate
//! @param RayDir_x Specify the vector of the ray X component
//! @param RayDir_y Specify the vector of the ray Y component
//! @param RayDir_z Specify the vector of the ray Z component
//! @param Dist Pointer to receive the distance from the sphere
//! @param maxDist Maximum distance to perform judgment (invalid or endless in less than 0.0)
//! @return Colliding: true; not colliding: false.
//! @warning Do not confuse RayPos (the start point) with RayDir (the direction vector).
//! @warning If the maximum distance is not specified, the performance will be drastically reduced.
//! @todo If the first point of the ray is outside of the sphere and opposite to the sphere, can not be judged correctly?
bool CollideSphereRay(float s_x, float s_y, float s_z, float s_r, float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z, float *Dist, float maxDist)
{
	if( maxDist > 0.0f ){
		float pmin_x, pmin_y, pmin_z, pmax_x, pmax_y, pmax_z;

		//Create the ray AABB.
		GetAABBRay(RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z, maxDist, &pmin_x, &pmin_y, &pmin_z, &pmax_x, &pmax_y, &pmax_z);

		//Determined by the border box
		if( CollideBoxAABB(s_x - s_r, s_y - s_r, s_z - s_r, s_x + s_r, s_y + s_r, s_z + s_r, pmin_x, pmin_y, pmin_z, pmax_x, pmax_y, pmax_z) == false ){
			return false;
		}
	}

	float x, y, z, d;
	float MinDist, RayDist, RDist;

	//Distance between the point and ray start.
	x = s_x - RayPos_x;
	y = s_y - RayPos_y;
	z = s_z - RayPos_z;
	d = sqrtf(x*x + y*y + z*z);

	//The ray starts closer than the radius (inside the object).
	if( d < s_r ){
		*Dist = 0.0f;
		return true;
	}

	//Seeking the shortest distance between points (center of the sphere) and rays
	MinDist = DistancePosRay(s_x, s_y, s_z, RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z);

	if( MinDist <= s_r ){
		RayDist = sqrtf(d*d - MinDist*MinDist);		//(from the beginning point of the ray)
		RDist = sqrtf(s_r*s_r - MinDist*MinDist);	//Distance of closest approach to the point, based on the radius.

		*Dist = RayDist - RDist;	//Shortest ray-to-point distance minus the radius offset gives the shortest distance from the ray start to the radius.

		//If the maximum distance is farther than the maximum distance to be judged, it is not possible to win
		if( (maxDist > 0.0f)&&(maxDist < *Dist) ){
			return false;
		}
		return true;
	}

	return false;
}

//! @brief Check collision between an AABB and a ray.
//! @param box_min_x Min. X coordinates of objects
//! @param box_min_y Min. Y coordinates of the object
//! @param box_min_z Min. Z coordinates of objects
//! @param box_max_x Max. X coordinates of objects
//! @param box_max_y Maximum of objects Y coordinates
//! @param box_max_z Maximum of objects Z coordinates
//! @param RayPos_x Specify the position of the ray (first point) X coordinate
//! @param RayPos_y Specify the position of the ray (first point) Y coordinate
//! @param RayPos_z Specify the position of the ray (first point) Z coordinate
//! @param RayDir_x Specify the vector of the ray X component
//! @param RayDir_y Specify the vector of the ray Y component
//! @param RayDir_z Specify the vector of the ray Z component
//! @param Dist Pointer receiving the distance to the intersected AABB.
//! @param maxDist Maximum distance to perform judgment (invalid or endless in less than 0.0)
//! @return Colliding: true; not colliding: false.
//! @warning Do not confuse RayPos (the start point) with RayDir (the direction vector).
//! @warning If the maximum distance is not specified, the performance will be drastically reduced.
bool CollideAABBRay(float box_min_x, float box_min_y, float box_min_z, float box_max_x, float box_max_y, float box_max_z, float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z, float *Dist, float maxDist)
{
	if( box_min_x > box_max_x ){ return false; }
	if( box_min_y > box_max_y ){ return false; }
	if( box_min_z > box_max_z ){ return false; }

	//If the ray starts inside the AABB, it already intersects it.
	if( (box_min_x <= RayPos_x)&&(RayPos_x <= box_max_x)&&(box_min_y <= RayPos_y)&&(RayPos_y <= box_max_y)&&(box_min_z <= RayPos_z)&&(RayPos_z <= box_max_z) ){
		if( Dist != NULL ){ *Dist = 0.0f; }
		return true;
	}

	if( maxDist > 0.0f ){
		float pmin_x, pmin_y, pmin_z, pmax_x, pmax_y, pmax_z;

		//Create the ray AABB.
		GetAABBRay(RayPos_x, RayPos_y, RayPos_z, RayDir_x, RayDir_y, RayDir_z, maxDist, &pmin_x, &pmin_y, &pmin_z, &pmax_x, &pmax_y, &pmax_z);

		//Determined by the border box
		if( CollideBoxAABB(box_min_x, box_min_y, box_min_z, box_max_x, box_max_y, box_max_z, pmin_x, pmin_y, pmin_z, pmax_x, pmax_y, pmax_z) == false ){
			return false;
		}
	}

	//Normalize vectors
	VectorNormalization(&RayDir_x, &RayDir_y, &RayDir_z);

	//Each coordinate is stored in an array
	float box_min[] = {box_min_x, box_min_y, box_min_z};
	float box_max[] = {box_max_x, box_max_y, box_max_z};
	float RayPos[] = {RayPos_x, RayPos_y, RayPos_z};
	float RayDir[] = {RayDir_x, RayDir_y, RayDir_z};
	float Ray_min[3];
	float Ray_max[3];
	float Ray_tmin, Ray_tmax;

	//Process all three axes: X, Y, and Z.
	for(int axis=0; axis<3; axis++){
		if( ( (RayPos[axis] < box_min[axis])||(box_max[axis] < RayPos[axis]) )&&(RayDir[axis] == 0.0f) ){
			//If the direction of the vector is 0 even though it is not inside, already outside the A
			return false;
		}
		else{
			//Not inside, but if the vector has a direction ~
			float t1, t2;

			//Get the distance to the intersection where the vector passes through the AABB.
			t1 = (box_min[axis] - RayPos[axis]) / RayDir[axis];
			t2 = (box_max[axis] - RayPos[axis]) / RayDir[axis];

			//If the distance to the crossing point is the minimum and the maximum, replace it.
			if( t1 > t2 ){
				float temp = t1; t1 = t2; t2 = temp;
			}

			//Record as a axis information
			Ray_min[axis] = t1;
			Ray_max[axis] = t2;

			if( (box_min[axis] <= RayPos[axis])&&(RayPos[axis] <= box_max[axis]) ){
				//If inside, it is recorded as a axis information as it is
				Ray_min[axis] = 0.0f;
			}

			//A negative distance (opposite the vector direction) means there is no AABB intersection.
			if( Ray_min[axis] < 0.0f ){ return false; }
			if( Ray_max[axis] < 0.0f ){ return false; }
		}
	}

	//Cal the farthest 'minimum distance' and the closest 'maximum distance' on each axis
	Ray_tmin = Ray_min[0];
	Ray_tmax = Ray_max[0];
	if( Ray_tmin < Ray_min[1] ){ Ray_tmin = Ray_min[1]; }
	if( Ray_tmax > Ray_max[1] ){ Ray_tmax = Ray_max[1]; }
	if( Ray_tmin < Ray_min[2] ){ Ray_tmin = Ray_min[2]; }
	if( Ray_tmax > Ray_max[2] ){ Ray_tmax = Ray_max[2]; }

	//If the relationship between the minimum distance and maximum distance is correct~
	if( (Ray_tmax - Ray_tmin) > 0 ){
		//If the maximum distance is farther than the maximum distance to be judged, the judgment is disabled.
		if( maxDist > 0.0f ){
			if( Ray_tmin > maxDist ){ return false; }
		}

		//Store the distance and return.
		if( Dist != NULL ){ *Dist = Ray_tmin; }
		return true;
	}

	return false;
}

//! @brief Check collision between a cylinder and a point.
//! @param c_x X coordinate of the cylinder base.
//! @param c_y Y coordinate of the cylinder base.
//! @param c_z Z coordinate of the cylinder base.
//! @param c_r Cylinder radius.
//! @param c_h Cylinder height.
//! @param p_x X Coordinate
//! @param p_y Y Coordinate
//! @param p_z Z Coordinates
//! @return Colliding: true; not colliding: false.
bool CollideCylinderInside(float c_x, float c_y, float c_z, float c_r, float c_h, float p_x, float p_y, float p_z)
{
	float x, z, d;


	// Processing as a circle with the X-Z plane

	//Calculate the distance
	x = c_x - p_x;
	z = c_z - p_z;
	d = x*x + z*z;

	//There is no collision if the distance exceeds the radius.
	if( d > c_r*c_r ){
		return false;
	}


	//Use the same processing as the AABB for the Y axis only.

	if( (c_y <= p_y)&&(p_y <= (c_y+c_h)) ){
		//If Y axis is inside, it is hit
		return true;
	}

	return false;
}

//! @brief Check collision between a cylinder and a ray.
//! @param c_x X coordinate of the cylinder base.
//! @param c_y Y coordinate of the cylinder base.
//! @param c_z Z coordinate of the cylinder base.
//! @param c_r Cylinder radius.
//! @param c_h Cylinder height.
//! @param RayPos_x Specify the position of the ray (first point) X coordinate
//! @param RayPos_y Specify the position of the ray (first point) Y coordinate
//! @param RayPos_z Specify the position of the ray (first point) Z coordinate
//! @param RayDir_x Specify the vector of the ray X component
//! @param RayDir_y Specify the vector of the ray Y component
//! @param RayDir_z Specify the vector of the ray Z component
//! @param Dist Pointer to receive the distance from the cylinder.
//! @param maxDist Maximum distance to perform judgment (value exceeding 0.0f)
//! @return Colliding: true; not colliding: false.
//! @warning Do not confuse RayPos (the start point) with RayDir (the direction vector).
//! @attention It is a c that stands vertically to the Y-axis direction. The orientation is not changed.
bool CollideCylinderRay(float c_x, float c_y, float c_z, float c_r, float c_h, float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z, float *Dist, float maxDist)
{
	float x, z, d;
	float cMinDist, cRayDist, cRDist;
	float RayXZ_min, RayXZ_max;
	float Ray_min[2];
	float Ray_max[2];
	float Ray_tmin, Ray_tmax;


	// Processing as a circle with the X-Z plane

	//Distance between the point and ray start.
	x = c_x - RayPos_x;
	z = c_z - RayPos_z;
	d = sqrtf(x*x + z*z);

	//Determine the shortest distance between points (center of c s) and rays
	cMinDist = DistancePosRay(c_x, 0.0f, c_z, RayPos_x, 0.0f, RayPos_z, RayDir_x, 0.0f, RayDir_z);

	//There is no collision if the shortest distance exceeds the radius.
	if( cMinDist > c_r ){
		return false;
	}

	cRayDist = sqrtf(d*d - cMinDist*cMinDist);		//(from the beginning point of the ray)
	cRDist = sqrtf(c_r*c_r - cMinDist*cMinDist);	//Distance of closest approach to the point, based on the radius.

	if( d < c_r ){
		RayXZ_min = 0;		//Zero distance if the first point is inside the circle
	}
	else{
		RayXZ_min = cRayDist - cRDist;	//Shortest ray-to-point distance minus the radius offset gives the shortest distance from the ray start to the radius.
	}

	//Distance between point and end point
	x = c_x - RayPos_x + RayDir_x*maxDist;
	z = c_z - RayPos_z + RayDir_x*maxDist;
	d = sqrtf(x*x + z*z);

	if( d < c_r ){
		RayXZ_max = maxDist;		//Maximum distance if the end point is in the circle
	}
	else{
		RayXZ_max = cRayDist + cRDist;
	}

	//Before comparing to the Y-axis, the Y-axis can be adjusted to the Y-axis (convert the value of the bottom to the bevel)
	float Ray_ry = atan2f(RayDir_y, sqrtf(RayDir_x*RayDir_x + RayDir_z*RayDir_z));
	Ray_min[0] = RayXZ_min / cosf(Ray_ry);
	Ray_max[0] = RayXZ_max / cosf(Ray_ry);


	//Use the same processing as the AABB for the Y axis only.

	if( ( (RayPos_y < c_y)||((c_y+c_h) < RayPos_y) )&&(RayDir_y == 0.0f) ){
		//If the direction of the vector is 0 even though it is not inside, it is already outside the cylinder
		return false;
	}

	//Not inside, but if the vector has a direction ~
	float t1, t2;

	//Get the distance to the cross point where the vector penetrates the cylinder
	t1 = (c_y - RayPos_y) / RayDir_y;
	t2 = ((c_y+c_h) - RayPos_y) / RayDir_y;

	//If the distance to the crossing point is the minimum and the maximum, replace it.
	if( t1 > t2 ){
		float temp = t1; t1 = t2; t2 = temp;
	}

	//Record as a axis information
	Ray_min[1] = t1;
	Ray_max[1] = t2;

	if( (c_y <= RayPos_y)&&(RayPos_y <= (c_y+c_h)) ){
		//If inside, it is recorded as a axis information as it is
		Ray_min[1] = 0.0f;
	}

	//If the distance is negative (= vector reverse direction), it is not stopped
	if( Ray_min[1] < 0.0f ){ return false; }
	if( Ray_max[1] < 0.0f ){ return false; }


	//Conform both calculations

	//Cal the farthest 'minimum distance' and the closest 'maximum distance' on each axis
	Ray_tmin = Ray_min[0];
	Ray_tmax = Ray_max[0];
	if( Ray_tmin < Ray_min[1] ){ Ray_tmin = Ray_min[1]; }
	if( Ray_tmax > Ray_max[1] ){ Ray_tmax = Ray_max[1]; }

	//If the relationship between the minimum distance and maximum distance is correct~
	if( (Ray_tmax - Ray_tmin) > 0 ){
		//If the maximum distance is farther than the maximum distance to be judged, the judgment is disabled.
		if( maxDist > 0.0f ){
			if( Ray_tmin > maxDist ){ return false; }
		}

		//Store the distance and return.
		if( Dist != NULL ){ *Dist = Ray_tmin; }
		return true;
	}

	return false;
}

//! @brief Seeking the shortest distance between points and rays
//! @param Pos_x X coordinates of dots
//! @param Pos_y Point Y coordinates
//! @param Pos_z Z-coordinate of dot
//! @param RayPos_x Specify the position of the ray (first point) X coordinate
//! @param RayPos_y Specify the position of the ray (first point) Y coordinate
//! @param RayPos_z Specify the position of the ray (first point) Z coordinate
//! @param RayDir_x Specify the vector of the ray X component
//! @param RayDir_y Specify the vector of the ray Y component
//! @param RayDir_z Specify the vector of the ray Z component
//! @return Shortest distance
//! @warning Do not confuse RayPos (the start point) with RayDir (the direction vector).
//! @attention The direction of the ray is considered, but the length of the ray is not considered.
float DistancePosRay(float Pos_x, float Pos_y, float Pos_z, float RayPos_x, float RayPos_y, float RayPos_z, float RayDir_x, float RayDir_y, float RayDir_z)
{
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;
	float Dot;

	x1 = Pos_x - RayPos_x;
	y1 = Pos_y - RayPos_y;
	z1 = Pos_z - RayPos_z;
	x2 = RayDir_x;
	y2 = RayDir_y;
	z2 = RayDir_z;

	//Dot product.
	Dot = x1 * x2 + y1 * y2 + z1 * z2;

	//If the ray vector is inverted
	if( Dot < 0.0f ){
		return sqrtf(x1*x1 + y1*y1 + z1*z1);
	}

	//Cross product.
	x3 = y1 * z2 - z1 * y2;
	y3 = z1 * x2 - x1 * z2;
	z3 = x1 * y2 - y1 * x2;

	return sqrtf(x3*x3 + y3*y3 + z3*z3) / sqrtf(RayDir_x*RayDir_x + RayDir_y*RayDir_y + RayDir_z*RayDir_z);
}

//! @brief Determination of Lines and Lines (2D)
//! @param A1x The beginning point of line A X coordinates
//! @param A1y The beginning point of line A Y coordinate
//! @param A2x End point of line A X coordinate
//! @param A2y End point of line A Y coordinate
//! @param B1x The beginning point of line B X coordinates
//! @param B1y First point of line B Y coordinates
//! @param B2x End point of line B X coordinate
//! @param B2y End point of line B Y coordinate
//! @param out_x Pointer receiving the intersection X coordinate (NULL allowed).
//! @param out_y Pointer receiving the intersection Y coordinate (NULL allowed).
//! @return Intersects: true; does not intersect: false.
bool Collide2DLine(int A1x, int A1y, int A2x, int A2y, int B1x, int B1y, int B2x, int B2y, int *out_x, int *out_y)
{
	//Seeking Line s
	int Avx = A2x - A1x;
	int Avy = A2y - A1y;
	int Bvx = B2x - B1x;
	int Bvy = B2y - B1y;

	float v1_v2 = (float)(Avx * Bvy - Avy * Bvx);	//Cross product.
	if( v1_v2 == 0.0f ){
		return false;	//Parallel.
	}

	float vx = (float)(B1x - A1x);
	float vy = (float)(B1y - A1y);
	float v_v1 = vx * Avy - vy * Avx;	//Cross product.
	float v_v2 = vx * Bvy - vy * Bvx;	//Cross product.
	float t1 = v_v2 / v1_v2;
	float t2 = v_v1 / v1_v2;

	if( (t1 <= 0)||(1 <= t1)||(t2 <= 0)||(1 <= t2) ){
		return false;	//Not crossed
	}

	if( out_x != NULL ){ *out_x = (int)(A1x + Avx * t1); }
	if( out_y != NULL ){ *out_y = (int)(A1y + Avy * t1); }

	return true;
}

//! @brief Call for Lines in Square Shape (2D)
//! @param line_x1 Origin of the line X coordinate
//! @param line_y1 The beginning point of the line Y coordinate
//! @param line_x2 End point of line X coordinate
//! @param line_y2 End point of line Y coordinate
//! @param box_x1 Upper left of square X coordinate
//! @param box_y1 Upper left of square Y coordinates
//! @param box_x2 Square right bottom X coordinates
//! @param box_y2 Right-bottom of square Y coordinate
//! @param out_line_x1 Pointer to receive the X coordinates of the first line that fits in the square
//! @param out_line_y1 Pointer to receive the first point of line Y coordinates that fit in square
//! @param out_line_x2 Pointer to receive the end point X coordinates of the line that fits in the square
//! @param out_line_y2 Pointer to receive the end point Y coordinate of the line that fits in the square
//! @return Visible (render): true; invisible (do not render): false.
//! @warning Arguments must satisfy box_x1 < box_x2 and box_x1 < box_x2. (The condition is repeated in the original comment.)
//! @note Used to draw the map on the simple radar.
bool Get2DLineInBox(int line_x1, int line_y1, int line_x2, int line_y2, int box_x1, int box_y1, int box_x2, int box_y2, int *out_line_x1, int *out_line_y1, int *out_line_x2, int *out_line_y2)
{
	//Invalid rectangle specification.
	if( (box_x1 >= box_x2)||(box_y1 >= box_y2) ){ return false; }

	//If you are in the upper and lower right space,
	if( (line_x1 < box_x1)&&(line_x2 < box_x1) ){ return false; }
	if( (line_y1 < box_y1)&&(line_y2 < box_y1) ){ return false; }
	if( (box_x2 < line_x1)&&(box_x2 < line_x2) ){ return false; }
	if( (box_y2 < line_y1)&&(box_y2 < line_y2) ){ return false; }

	//Already fit in square
	if( (box_x1 <= line_x1)&&(line_x1 <= box_x2)&&(box_y1 <= line_y1)&&(line_y1 <= box_y2) ){
		if( (box_x1 <= line_x2)&&(line_x2 <= box_x2)&&(box_y1 <= line_y2)&&(line_y2 <= box_y2) ){
			*out_line_x1 = line_x1;
			*out_line_y1 = line_y1;
			*out_line_x2 = line_x2;
			*out_line_y2 = line_y2;
			return true;
		}
	}

	int x, y;

	//Top edge.
	if( Collide2DLine(box_x1, box_y1, box_x2, box_y1, line_x1, line_y1, line_x2, line_y2, &x, &y) == true ){
		//If the first point is inside a square shape, change the end point (= the end point is inside).
		if( (box_x1 <= line_x1)&&(line_x1 <= box_x2)&&(box_y1 <= line_y1)&&(line_y1 <= box_y2) ){
			line_x2 = x;
			line_y2 = y;
		}
		else{
			line_x1 = x;
			line_y1 = y;
		}
	}
	//Right side
	if( Collide2DLine(box_x2, box_y1, box_x2, box_y2, line_x1, line_y1, line_x2, line_y2, &x, &y) == true ){
		//If the first point is inside a square shape, change the end point (= the end point is inside).
		if( (box_x1 <= line_x1)&&(line_x1 <= box_x2)&&(box_y1 <= line_y1)&&(line_y1 <= box_y2) ){
			line_x2 = x;
			line_y2 = y;
		}
		else{
			line_x1 = x;
			line_y1 = y;
		}
	}
	//Lower side
	if( Collide2DLine(box_x2, box_y2, box_x1, box_y2, line_x1, line_y1, line_x2, line_y2, &x, &y) == true ){
		//If the first point is inside a square shape, change the end point (= the end point is inside).
		if( (box_x1 <= line_x1)&&(line_x1 <= box_x2)&&(box_y1 <= line_y1)&&(line_y1 <= box_y2) ){
			line_x2 = x;
			line_y2 = y;
		}
		else{
			line_x1 = x;
			line_y1 = y;
		}
	}
	//Left Side
	if( Collide2DLine(box_x1, box_y2, box_x1, box_y1, line_x1, line_y1, line_x2, line_y2, &x, &y) == true ){
		//If the first point is inside a square shape, change the end point (= the end point is inside).
		if( (box_x1 <= line_x1)&&(line_x1 <= box_x2)&&(box_y1 <= line_y1)&&(line_y1 <= box_y2) ){
			line_x2 = x;
			line_y2 = y;
		}
		else{
			line_x1 = x;
			line_y1 = y;
		}
	}

	//Check whether it fits in square again
	if( (box_x1 <= line_x1)&&(line_x1 <= box_x2)&&(box_y1 <= line_y1)&&(line_y1 <= box_y2) ){
		if( (box_x1 <= line_x2)&&(line_x2 <= box_x2)&&(box_y1 <= line_y2)&&(line_y2 <= box_y2) ){
			*out_line_x1 = line_x1;
			*out_line_y1 = line_y1;
			*out_line_x2 = line_x2;
			*out_line_y2 = line_y2;
			return true;
		}
	}

	return false;
}

//! @brief Distance judgment and angle calculation from observation point to target point
//! @param pos_x X coordinates of observation points
//! @param pos_y Y coordinates of observation points
//! @param pos_z Z coordinates of observation points
//! @param rx Horizontal angle of the observation point.
//! @param ry Vertical Angle of Observation Point
//! @param target_x X coordinates of target points
//! @param target_y Target point Y coordinates
//! @param target_z Target point Z coordinates
//! @param checkdist Determination distance (disabled by 0.0f or less)
//! @param out_rx Pointer receiving the horizontal angle to the target point (PI to -PI; NULL allowed).
//! @param out_ry Pointer receiving the vertical angle to the target point (NULL allowed).
//! @param out_dist2 Pointer receiving the <b>squared</b> distance to the target point (NULL allowed).
//! @return success: true failure: false
//! @warning out_dist2 is the <b>squared</b> distance. Use sqrtf() if the actual distance is needed.
//! @attention If a distance is given to the checkdist argument, the distance is also determined from the observation point to the target point. If it is far from the specified distance, it returns false without specifying the angle.
//! @attention On the other hand, if the argument checkdist gives 0.0f or less, the distance will not be judged. The function always returns true.
bool CheckTargetAngle(float pos_x, float pos_y, float pos_z, float rx, float ry, float target_x, float target_y, float target_z, float checkdist, float *out_rx, float *out_ry, float *out_dist2)
{
	float x, y, z;
	float dist2 = 0.0f;
	float angleX, angleY;

	x = target_x - pos_x;
	y = target_y - pos_y;
	z = target_z - pos_z;

	if( (checkdist > 0.0f)||(out_dist2 != NULL) ){
		dist2 = (x*x + y*y + z*z);
	}

	if( checkdist > 0.0f ){
		if( dist2 > checkdist * checkdist ){
			return false;
		}
	}

	if( out_rx != NULL ){
		angleX = atan2f(z, x) - rx;
		angleX = AngleNormalization(angleX);

		*out_rx = angleX;
	}
	if( out_ry != NULL ){
		angleY = atan2f(y, sqrtf(x*x + z*z))  - ry;

		*out_ry = angleY;
	}
	if( out_dist2 != NULL ){ *out_dist2 = dist2; }

	return true;
}

//! @brief Normalize the vector.
//! @param vx vx pointer
//! @param vy vy pointer
//! @param vz vz pointer
//! @return Length of the vector before normalization.
float VectorNormalization(float *vx, float *vy, float *vz)
{
	float r = sqrtf((*vx) * (*vx) + (*vy) * (*vy) + (*vz) * (*vz));
	if( r > 0.0f ){
		*vx /= r;
		*vy /= r;
		*vz /= r;
	}
	return r;
}