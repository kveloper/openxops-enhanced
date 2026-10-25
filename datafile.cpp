//! @file datafile.cpp
//! @brief Defining Data Management Class

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

#include "datafile.h"

//! @brief Constructor
DataInterface::DataInterface()
{
	datas = 0;
}

//! @brief Destructor
DataInterface::~DataInterface()
{}

//! @brief Read data files
//! @param fname File Name
//! @return Success:0 failure:1
int DataInterface::LoadFiledata(const char *fname)
{
	//Handle unused arguments.
	UNREFERENCED_PARAMETER(fname);

	if( fname == NULL ){ return 1; }

	return 0;
}

//! @brief Acquisition of total data
//! @return Total number of data entries.
int DataInterface::GetTotaldatas()
{
	return datas;
}

//! @brief Get Data
//! @param out_data Pointer to receive
//! @param id Data Number
//! @return Success:0 Failure:non 0
int DataInterface::Getdata(void *out_data, int id)
{
	//Handle unused arguments.
	UNREFERENCED_PARAMETER(out_data);
	UNREFERENCED_PARAMETER(id);

	return 0;
}

//! @brief Constructor
BlockDataInterface::BlockDataInterface()
{
	//blockdata structure initialization
	data = new blockdata[MAX_BLOCKS];

	//Texture setting initialization
	for(int i=0; i<TOTAL_BLOCKTEXTURE; i++){
		strcpy(texture[i], "");
	}
}

//! @brief Destructor
BlockDataInterface::~BlockDataInterface()
{
	//blockdata structure release
	if( data != NULL ){ delete [] data; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	//OutputLog.WriteLog(LOG_CLEANUP, "BD1", "");
#endif
}

//! @brief Read block data files
//! @param fname File Name
//! @return Success:0 failure:1
int BlockDataInterface::LoadFiledata(const char *fname)
{
	if( fname == NULL ){ return 1; }

	FILE *fp;
	unsigned char bdata_header[2];
	float bdata_main[80];
	char bdata_mainb[30];

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "BD1", fname);
#endif

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	fname = ChangePathDelimiter(fname);
#endif

	//Load files
	fp = fopen(fname, "rb");
	if( fp == NULL ){ return 1; }

	//Get Texture
	for(int i=0; i<TOTAL_BLOCKTEXTURE; i++){
		fread(texture[i], 1, 31, fp );
	}

	//Get the number of data entries.
	fread( bdata_header, 1, 2, fp );
	datas = (int)Get2ByteLittleEndian(&(bdata_header[0]));
	if( datas > MAX_BLOCKS ){ datas = MAX_BLOCKS; }

	//Get block data minutes
	for(int i=0; i<datas; i++){
		//Get vertex information and surface information for block
		fread( bdata_main, 4, 24+48, fp );
		fread( bdata_mainb, 1, 28, fp );

		//Set the data number
		data[i].id = i;

		//Set the vertex information of the block
		for(int j=0; j<8; j++){
			data[i].x[j] = bdata_main[j];
			data[i].y[j] = bdata_main[j+8];
			data[i].z[j] = bdata_main[j+16];
		}

		//Set the block surface information
		for(int j=0; j<6; j++){
			data[i].material[j].textureID = (int)bdata_mainb[j*4];
			data[i].material[j].u[0] = bdata_main[j*4+24];
			data[i].material[j].v[0] = bdata_main[j*4+48];
			data[i].material[j].u[1] = bdata_main[j*4+25];
			data[i].material[j].v[1] = bdata_main[j*4+49];
			data[i].material[j].u[2] = bdata_main[j*4+26];
			data[i].material[j].v[2] = bdata_main[j*4+50];
			data[i].material[j].u[3] = bdata_main[j*4+27];
			data[i].material[j].v[3] = bdata_main[j*4+51];
		}
	}

	//Free File Handle
	fclose( fp );

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif

	return 0;
}

//! @brief Calculate block-data normals and shading.
//! @param darkflag Darken Block
//! @attention Run once after loading with LoadBlockdata().
void BlockDataInterface::CalculationBlockdata(bool darkflag)
{
	int vID[4];
	int uvID[4];
	float g;
	float xs, ys, zs;
	float g1;
	float xs1, ys1, zs1;
	float g2;
	float xs2, ys2, zs2;
	float lx, ly, lz;
	float rx, ry, rz, a;

	//Set the angle of light source
	lx = cosf(LIGHT_RX);
	ly = sinf(LIGHT_RY);
	lz = sinf(LIGHT_RX);

	//Processing the surface information of each block
	for(int i=0; i<datas; i++){
		for(int j=0; j<6; j++){

			//es a block vertex data association
			blockdataface(j, vID, uvID);

			//Initialization of center point
			data[i].material[j].center_x = 0.0f;
			data[i].material[j].center_y = 0.0f;
			data[i].material[j].center_z = 0.0f;

			//Calculate the center of the four vertices.
			for(int k=0; k<4; k++){
				data[i].material[j].center_x += data[i].x[ vID[k] ];
				data[i].material[j].center_y += data[i].y[ vID[k] ];
				data[i].material[j].center_z += data[i].z[ vID[k] ];
			}
			data[i].material[j].center_x /= 4;
			data[i].material[j].center_y /= 4;
			data[i].material[j].center_z /= 4;

			//Look for the line of the face (vector) and its length
			xs1 = ((data[i].y[ vID[3] ] - data[i].y[ vID[2] ]) * (data[i].z[ vID[0] ] - data[i].z[ vID[2] ])) - ((data[i].y[ vID[0] ] - data[i].y[ vID[2] ]) * (data[i].z[ vID[3] ] - data[i].z[ vID[2] ]));
			ys1 = ((data[i].z[ vID[3] ] - data[i].z[ vID[2] ]) * (data[i].x[ vID[0] ] - data[i].x[ vID[2] ])) - ((data[i].z[ vID[0] ] - data[i].z[ vID[2] ]) * (data[i].x[ vID[3] ] - data[i].x[ vID[2] ]));
			zs1 = ((data[i].x[ vID[3] ] - data[i].x[ vID[2] ]) * (data[i].y[ vID[0] ] - data[i].y[ vID[2] ])) - ((data[i].x[ vID[0] ] - data[i].x[ vID[2] ]) * (data[i].y[ vID[3] ] - data[i].y[ vID[2] ]));
			g1 = sqrtf(xs1 * xs1 + ys1 * ys1 + zs1 * zs1);

			//Also calculate for the other triangle.
			xs2 = ((data[i].y[ vID[1] ] - data[i].y[ vID[0] ]) * (data[i].z[ vID[2] ] - data[i].z[ vID[0] ])) - ((data[i].y[ vID[2] ] - data[i].y[ vID[0] ]) * (data[i].z[ vID[1] ] - data[i].z[ vID[0] ]));
			ys2 = ((data[i].z[ vID[1] ] - data[i].z[ vID[0] ]) * (data[i].x[ vID[2] ] - data[i].x[ vID[0] ])) - ((data[i].z[ vID[2] ] - data[i].z[ vID[0] ]) * (data[i].x[ vID[1] ] - data[i].x[ vID[0] ]));
			zs2 = ((data[i].x[ vID[1] ] - data[i].x[ vID[0] ]) * (data[i].y[ vID[2] ] - data[i].y[ vID[0] ])) - ((data[i].x[ vID[2] ] - data[i].x[ vID[0] ]) * (data[i].y[ vID[1] ] - data[i].y[ vID[0] ]));
			g2 = sqrtf(xs2 * xs2 + ys2 * ys2 + zs2 * zs2);

			//Adopt correct calculation result
			if( g1 > g2 ){
				xs = xs1;
				ys = ys1;
				zs = zs1;
				g = g1;
			}
			else{
				xs = xs2;
				ys = ys2;
				zs = zs2;
				g = g2;
			}

			//Normalize the normal vector.
			data[i].material[j].vx = xs / g;
			data[i].material[j].vy = ys / g;
			data[i].material[j].vz = zs / g;

			//Seeking the brightness of the face
			rx = data[i].material[j].vx + lx;
			ry = data[i].material[j].vy + ly;
			rz = data[i].material[j].vz + lz;
			//a = sqrtf(fabsf(rx*rx + ry*ry + rz*rz)) / 2.0f;
			//data[i].material[j].shadow = a/2 + 0.5f;
			a = sqrtf(rx*rx + ry*ry + rz*rz) / 3.0f;
			data[i].material[j].shadow = a/2;
			if( darkflag == false ){
				data[i].material[j].shadow += 0.5f;
			}
			else{
				data[i].material[j].shadow += 0.3f;
			}
		}
	}
}

//! @brief es the file name of the texture set to block data
//! @param fname String type pointer to receive the file name
//! @param id Texture number
//! @return Success:0 Failure:non 0
int BlockDataInterface::GetTexture(char *fname, int id)
{
	if( data == NULL ){ return 1; }
	if( fname == NULL ){ return 2; }
	if( (id < 0)||(TOTAL_BLOCKTEXTURE <= id) ){ return 2; }

	//Copy the texture name to the pointer
	strcpy(fname, texture[id]);

	return 0;
}

//! @brief Get block data
//! @param out_data Receive blockdata type pointer
//! @param id Data Number
//! @return Success:0 Failure:non 0
int BlockDataInterface::Getdata(blockdata *out_data, int id)
{
	if( data == NULL ){ return 1; }
	if( out_data == NULL ){ return 2; }
	if( (id < 0)||(datas <= id) ){ return 2; }

	//Get block data
	memcpy(out_data, &(data[id]), sizeof(blockdata));

	return 0;
}

//! @brief es a block vertex data association
//
//	        3 o----o2
//	     /|    /|
//	    / |   / |
//	0 o----o1 |
//	    |7 o-| --o6
//	  |  /  |  /
//	  | /   | /
//	4 o----o5
//
//	C-----D
//	|     |
//	|     |
//	B-----A
//
//	Top[0](ABCD) = (1032)
//	lower[1] = (6745)
//	Previous[2] = (5401)
//	Right[3] = (6512)
//	Back[4] = (7623)
//	Left[5] = (4730)
bool blockdataface(int faceID, int* vID, int* uvID)
{
	if( faceID == 0 ){
		if( vID != NULL ){
			vID[0] = 1;		vID[1] = 0;		vID[2] = 3;		vID[3] = 2;
		}
		if( uvID != NULL ){
			uvID[0] = 2;	uvID[1] = 3;	uvID[2] = 0;	uvID[3] = 1;
		}
		return true;
	}
	if( faceID == 1 ){
		if( vID != NULL ){
			vID[0] = 6;		vID[1] = 7;		vID[2] = 4;		vID[3] = 5;
		}
		if( uvID != NULL ){
			uvID[0] = 2;	uvID[1] = 3;	uvID[2] = 0;	uvID[3] = 1;
		}
		return true;
	}
	if( faceID == 2 ){
		if( vID != NULL ){
			vID[0] = 5;		vID[1] = 4;		vID[2] = 0;		vID[3] = 1;
		}
		if( uvID != NULL ){
			uvID[0] = 2;	uvID[1] = 3;	uvID[2] = 0;	uvID[3] = 1;
		}
		return true;
	}
	if( faceID == 3 ){
		if( vID != NULL ){
			vID[0] = 6;		vID[1] = 5;		vID[2] = 1;		vID[3] = 2;
		}
		if( uvID != NULL ){
			uvID[0] = 2;	uvID[1] = 3;	uvID[2] = 0;	uvID[3] = 1;
		}
		return true;
	}
	if( faceID == 4 ){
		if( vID != NULL ){
			vID[0] = 7;		vID[1] = 6;		vID[2] = 2;		vID[3] = 3;
		}
		if( uvID != NULL ){
			uvID[0] = 2;	uvID[1] = 3;	uvID[2] = 0;	uvID[3] = 1;
		}
		return true;
	}
	if( faceID == 5 ){
		if( vID != NULL ){
			vID[0] = 4;		vID[1] = 7;		vID[2] = 3;		vID[3] = 0;
		}
		if( uvID != NULL ){
			uvID[0] = 2;	uvID[1] = 3;	uvID[2] = 0;	uvID[3] = 1;
		}
		return true;
	}
	return false;
}

//! @brief Constructor
PointDataInterface::PointDataInterface()
{
	//pointdata structure initialization
	data = new pointdata[MAX_POINTS];

	//Initializing Event Messages
	for(int i=0; i<MAX_POINTMESSAGES; i++){
		text[i] = new char[MAX_POINTMESSAGEBYTE];
		text[i][0] = '\0';
	}
}

//! @brief Destructor
PointDataInterface::~PointDataInterface()
{
	//pointdata structure release
	if( data != NULL ){ delete [] data; }

	//Event message release
	for(int i=0; i<MAX_POINTMESSAGES; i++){
		if( text[i] != NULL ){ delete [] text[i]; }
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	//OutputLog.WriteLog(LOG_CLEANUP, "PD1", "");
#endif
}

//! @brief Read Point Data Files
//! @param fname File Name
//! @return Success:0 failure:1
int PointDataInterface::LoadFiledata(const char *fname)
{
	if( fname == NULL ){ return 1; }

	FILE *fp;
	unsigned char pdata_header[2];
	float pdata_mainf[4];
	signed char pdata_mainc[4];
	char fname2[MAX_PATH];

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "PD1", fname);
#endif

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	fname = ChangePathDelimiter(fname);
#endif

	//Load files
	fp = fopen( fname, "rb" );
	if( fp == NULL ){ return 1; }

	//Get the number of data entries.
	fread( pdata_header, 1, 2, fp );
	datas = (int)Get2ByteLittleEndian(&(pdata_header[0]));
	if( datas > MAX_POINTS ){ datas = MAX_POINTS; }

	//Processing point data minutes
	for(int i=0; i<datas; i++){
		//Data number setting
		data[i].id = i;

		//Coordinate data acquisition
		fread( pdata_mainf, 4, 4, fp );
		data[i].x = pdata_mainf[0];
		data[i].y = pdata_mainf[1];
		data[i].z = pdata_mainf[2];
		data[i].r = pdata_mainf[3];

		//Get Parameters
		fread( pdata_mainc, 1, 4, fp );
		data[i].p1 = (signed short int)pdata_mainc[0];
		data[i].p2 = (signed short int)pdata_mainc[1];
		data[i].p3 = (signed short int)pdata_mainc[2];
		data[i].p4 = (signed short int)pdata_mainc[3];
	}

	//Close File Pointer
	fclose( fp );

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif

	//Initializing Event Messages
	for(int i=0; i<MAX_POINTMESSAGES; i++){
		text[i][0] = '\0';
	}

	//Generate a file with the same name and the .msg extension.
	strcpy(fname2, fname);
	//PathRemoveExtension(fname2);
	for(int i=(int)strlen(fname2)-1; i>0; i--){
		if( fname2[i] == '.' ){
			fname2[i] = '\0';
			break;
		}
	}
	strcat(fname2, ".msg");

	LoadMSGFiledata(fname2);

	return 0;
}

//! @brief Read message data files
//! @param fname File Name
//! @return Success:0 failure:1
int PointDataInterface::LoadMSGFiledata(char *fname)
{
	if( fname == NULL ){ return 1; }

	FILE *fp;

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	fname = ChangePathDelimiter(fname);
#endif

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "MSG", fname);
#endif

	//Read files
	fp = fopen( fname, "r" );
	if( fp != NULL ){
		//Get message data
		for(int i=0; i<MAX_POINTMESSAGES; i++){
			if( fgets(text[i], MAX_POINTMESSAGEBYTE, fp) == NULL ){ break; }

			//with '\n' if you have '\r'
			for(int j=0; j<MAX_POINTMESSAGEBYTE; j++){
				if( text[i][j] == '\r' ){ text[i][j] = '\n'; }
			}
		}

		//Free File Pointer
		fclose( fp );

#ifdef ENABLE_DEBUGLOG
		//Write to the log
		OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif
		return 0;
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_ERROR, "", "");
#endif
	return 1;
}

//! @brief Get Point Data
//! @param out_data Receive pointdata type pointer
//! @param id Data Number
//! @return Success:0 Failure:non 0
int PointDataInterface::Getdata(pointdata *out_data, int id)
{
	if( data == NULL ){ return 1; }
	if( out_data == NULL ){ return 2; }
	if( (id < 0)||(datas <= id) ){ return 2; }

	//Copy data to pointer
	memcpy(out_data, &(data[id]), sizeof(pointdata));

	return 0;
}

//! @brief Rewrite point parameters
//! @param id Data Number
//! @param p1 Parameter 1
//! @param p2 Parameter 2
//! @param p3 3 Parameters
//! @param p4 4 Parameters
//! @return Success:0 Failure:non 0
//! @warning Use outside AI movement paths (p1:3 or p1:8) is unsupported and not recommended.
int PointDataInterface::SetParam(int id, signed short int p1, signed short int p2, signed short int p3, signed short int p4)
{
	if( data == NULL ){ return 1; }
	if( (id < 0)||(datas <= id) ){ return 2; }

	//Override Parameters
	data[id].p1 = p1;
	data[id].p2 = p2;
	data[id].p3 = p3;
	data[id].p4 = p4;

	return 0;
}

//! @brief Get a message (1 line)
//! @param str Pointer to receive a string
//! @param id Data Number
//! @return Success:0 failure:1
int PointDataInterface::GetMessageText(char *str, int id)
{
	if( str == NULL ){ return 1; }
	if( (id < 0)||(MAX_POINTMESSAGES <= id) ){ return 1; }

	//Copy messages to pointers
	strcpy(str, text[id]);
	return 0;
}

//! @brief Search Point Data
//! @param id Pointer to receive the first data number (NULL OK)
//! @param pmask Parameter Mask
//! @param p1 Parameter 1
//! @param p2 Parameter 2
//! @param p3 3 Parameters
//! @param p4 4 Parameters
//! @param offset Data to start searching
//! @return Number of points
//! @note The mask of the parameter ifies the search target of p1 to p4.
//! @note The p1:1 bit, p2 bit, p3 bit, and p4:4 bit are used for the first to fourth bit.
//! @note Set searched bits to 1 and excluded bits to 0. Bits 7 through 5 are ignored.
//! @code
//! //16decimal number -
//! 0x02 // Search only p2
//! 0x05 // Search p1 and p3
//! 0xF1 // Search only p1 (ignore bits 7 through 5).
//! @endcode
int PointDataInterface::SearchPointdata(int* id, unsigned char pmask, signed short int p1, signed short int p2, signed short int p3, signed short int p4, int offset)
{
	int cnt = 0;

	if( offset < 0 ){ offset = 0; }
	if( offset >= datas ){ offset = datas; }

	//Data action processing from offset value
	for(int i=offset; i<datas; i++){

		if( ((pmask&0x01) == 0)||(p1 == data[i].p1) ){		//not specified in the mask or p1 is matched
			if( ((pmask&0x02) == 0)||(p2 == data[i].p2) ){		//not specified in the mask or p2 is matched
				if( ((pmask&0x04) == 0)||(p3 == data[i].p3) ){		//not specified in the mask or p3 matches
					if( ((pmask&0x08) == 0)||(p4 == data[i].p4) ){		//not specified in the mask or p4 matches

						if( (cnt == 0)&&(id != NULL) ){			//First, id is not NULL.
							*id = i;
						}

						//Number of searches+1
						cnt += 1;

					}
				}
			}
		}

	}

	//Returns the search number
	return cnt;
}

//! @brief Search Point Data
//! @param out_data Pointdata type pointer to receive the first data (NULL OK)
//! @param pmask Parameter Mask
//! @param p1 Parameter 1
//! @param p2 Parameter 2
//! @param p3 3 Parameters
//! @param p4 4 Parameters
//! @param offset Data to start searching
//! @return Number of points
//! @attention For more information about this function, see the overload function.
int PointDataInterface::SearchPointdata(pointdata *out_data, unsigned char pmask, signed short int p1, signed short int p2, signed short int p3, signed short int p4, int offset)
{
	int id, total;

	//Search for points in the same condition
	total = SearchPointdata(&id, pmask, p1, p2, p3, p4, offset);
	
	//Get data if applicable
	if( total > 0 ){
		Getdata(out_data, id);
	}

	//Returns the search number
	return total;
}

//! @brief Constructor
MIFInterface::MIFInterface()
{
	datas = 0;

	mif = false;
	strcpy(mission_name, "");
	strcpy(mission_fullname, "");
	strcpy(blockfile_path, "");
	strcpy(pointfile_path, "");
	skynumber = 0;
	strcpy(picturefileA_path, "!");
	strcpy(picturefileB_path, "!");
	strcpy(addsmallobject_path, "");
	strcpy(briefingtext, "");
	collision = false;
	screen = false;
	AddSmallObjectData = new addsmallobject[MAX_ADDSMALLOBJECT];
}

//! @brief Destructor
MIFInterface::~MIFInterface()
{
	if( AddSmallObjectData != NULL ){ delete [] AddSmallObjectData; }
}

//! @brief Read data files
//! @param fname File Name
//! @param addfile Load additional files
//! @return Success:0 failure:1
int MIFInterface::LoadFiledata(const char *fname, bool addfile)
{
	if( fname == NULL ){ return 1; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "MIF", fname);
#endif

	mif = true;

	//If the extension is .txt
	if( CheckFileExtension(fname, ".txt") == true ){
		//Enable MIF flags
		mif = false;
	}

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	fname = ChangePathDelimiter(fname);
#endif

	if( mif == false ){
		if( LoadDefaultTextFiledata(fname) != 0 ){
			//briefing data open failed
			return 1;
		}
	}
	else{
		if( LoadMissionInfoFiledata(fname) != 0 ){
			//briefing data open failed
			return 1;
		}
	}

#ifdef ENABLE_AUTOLOADMIF
	//If the drive letter is specified (= full path)
	if( CheckFullPath(fname) == true ){
		char path[MAX_PATH];

		//Get mif directory
		GetFileDirectory(fname, path);

		//Rewrite each file path to full path
		ChangeExePathToFullPath(path, blockfile_path);
		ChangeExePathToFullPath(path, pointfile_path);
		ChangeExePathToFullPath(path, picturefileA_path);
		ChangeExePathToFullPath(path, picturefileB_path);
		ChangeExePathToFullPath(path, addsmallobject_path);
	}
#endif

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif


	//Adding additional accessories to initial value
	for(int i=0; i<MAX_ADDSMALLOBJECT; i++){
		strcpy(AddSmallObjectData[i].modelpath, "");
		strcpy(AddSmallObjectData[i].texturepath, "");
		AddSmallObjectData[i].decide = 0;
		AddSmallObjectData[i].hp = 0;
		strcpy(AddSmallObjectData[i].soundpath, "");
		AddSmallObjectData[i].jump = 0;
	}

	//If you specify an additional accessory information file for something
	if( (addfile == true)&&(strcmp(addsmallobject_path, "") != 0)&&(strcmp(addsmallobject_path, "!") != 0) ){
		LoadAddSmallObjectFiledata(addsmallobject_path);
	}
#ifdef ENABLE_DEBUGLOG
	else{
		//Write to the log
		OutputLog.WriteLog(LOG_CHECK, "(Add SmallObject)", "File not found");
	}
#endif

#ifdef ENABLE_AUTOLOADMIF
	//If the drive letter is specified (= full path)
	if( CheckFullPath(fname) == true ){
		char path[MAX_PATH];

		//Get mif directory
		GetFileDirectory(fname, path);

		//Rewrite each file path to full path
		for(int i=0; i<MAX_ADDSMALLOBJECT; i++){
			ChangeExePathToFullPath(path, AddSmallObjectData[i].modelpath);
			ChangeExePathToFullPath(path, AddSmallObjectData[i].texturepath);
			ChangeExePathToFullPath(path, AddSmallObjectData[i].soundpath);
		}
	}
#endif

	return 0;
}

//! @brief Read Mission Files (.txt)
//! @param fname File Name
//! @return Success:0 failure:1
int MIFInterface::LoadDefaultTextFiledata(const char *fname)
{
	if( fname == NULL ){ return 1; }

	FILE *fp;
	char str[64];

	//Open a file
	fp = fopen( fname, "r" );
	if( fp != NULL ){
		//Get Image A
		fgets(str, 64, fp);
		DeleteLinefeed(str);
		if( strcmp(str, "!") == 0 ){
			strcpy(picturefileA_path, "!");
		}
		else{
			//\rate data\\dataefing\\  to  .bmp
			strcpy(picturefileA_path, "data\\briefing\\");
			strcat(picturefileA_path, str);
			strcat(picturefileA_path, ".bmp");
		}

		//Get Image B
		fgets(str, 64, fp);
		DeleteLinefeed(str);
		if( strcmp(str, "!") == 0 ){
			strcpy(picturefileB_path, "!");
		}
		else{
			//\rate data\\dataefing\\  to  .bmp
			strcpy(picturefileB_path, "data\\briefing\\");
			strcat(picturefileB_path, str);
			strcat(picturefileB_path, ".bmp");
		}

		//Background sky ID.
		fgets(str, 16, fp);
		DeleteLinefeed(str);
		skynumber = atoi(str);

		//Getting a briefing text
		strcpy(briefingtext, "");
		for(int i=0; i<17; i++ ){
			if( fgets(str, 50, fp) == NULL ){ break; }
			strcat(briefingtext, str);
			datas += 1;
		}

		//Free File Handle
		fclose( fp );

		//Initialization of values that cannot be obtained
		strcpy(mission_name, "");
		strcpy(mission_fullname, "");
		strcpy(blockfile_path, "");
		strcpy(pointfile_path, "");
		strcpy(addsmallobject_path, "");
		collision = false;
		screen = false;

		return 0;
	}

	return 1;
}

//! @brief Read Mission Files (.mif)
//! @param fname File Name
//! @return Success:0 failure:1
int MIFInterface::LoadMissionInfoFiledata(const char *fname)
{
	if( fname == NULL ){ return 1; }

	FILE *fp;
	char str[64];

	//Open a file
	fp = fopen( fname, "r" );
	if( fp != NULL ){
		//Mission identification name
		fgets(mission_name, 24, fp);
		DeleteLinefeed(mission_name);

		//Mission official name
		fgets(mission_fullname, 64, fp);
		DeleteLinefeed(mission_fullname);

		//Block Data File
		fgets(blockfile_path, _MAX_PATH, fp);
		DeleteLinefeed(blockfile_path);

		//Point Data File
		fgets(pointfile_path, _MAX_PATH, fp);
		DeleteLinefeed(pointfile_path);

		//Background sky ID.
		fgets(str, 16, fp);
		DeleteLinefeed(str);
		skynumber = atoi(str);

		//Get Percentage and Screen Settings
		fgets(str, 16, fp);
		DeleteLinefeed(str);
		if( strcmp(str, "1") == 0 ){
			collision = true;
			screen = false;
		}
		else if( strcmp(str, "2") == 0 ){
			collision = false;
			screen = true;
		}
		else if( strcmp(str, "3") == 0 ){
			collision = true;
			screen = true;
		}
		else{	// == "0"
			collision = false;
			screen = false;
		}

		//Get additional accessories information file
		fgets(addsmallobject_path, _MAX_PATH, fp);
		DeleteLinefeed(addsmallobject_path);

		//Get Image A
		fgets(picturefileA_path, _MAX_PATH, fp);
		DeleteLinefeed(picturefileA_path);

		//Get Image B
		fgets(picturefileB_path, _MAX_PATH, fp);
		DeleteLinefeed(picturefileB_path);

		//Getting a briefing text
		strcpy(briefingtext, "");
		for(int i=0; i<17; i++ ){
			if( fgets(str, 50, fp) == NULL ){ break; }
			strcat(briefingtext, str);
			datas += 1;
		}

		//Free File Handle
		fclose( fp );

		return 0;
	}

	return 1;
}

//! @brief Read additional accessories information files
//! @param fname File Name
//! @return Success:0 failure:1
int MIFInterface::LoadAddSmallObjectFiledata(const char *fname)
{
	if( fname == NULL ){ return 1; }

	FILE *fp;
	char str1[_MAX_PATH];
	char str2[_MAX_PATH];
	char str3[32];
	char str4[32];
	char str5[_MAX_PATH];
	char str6[32];

	char str[64];

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	fname = ChangePathDelimiter(fname);
#endif

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "(Add SmallObject)", fname);
#endif

	//Open a file
	fp = fopen( fname, "r" );
	if( fp != NULL ){
		for(int i=0; i<MAX_ADDSMALLOBJECT; i++){
			//6 lines
			//Failed to get all six lines
			if( fgets(str1, _MAX_PATH, fp) == NULL ){ break; }
			if( fgets(str2, _MAX_PATH, fp) == NULL ){ break; }
			if( fgets(str3, 16, fp) == NULL ){ break; }
			if( fgets(str4, 16, fp) == NULL ){ break; }
			if( fgets(str5, _MAX_PATH, fp) == NULL ){ break; }
			if( fgets(str6, 16, fp) == NULL ){ break; }


			//Model Data Path
			DeleteLinefeed(str1);
			strcpy(AddSmallObjectData[i].modelpath, str1);

			//Texture Path
			DeleteLinefeed(str2);
			strcpy(AddSmallObjectData[i].texturepath, str2);

			//Collision volume size.
			DeleteLinefeed(str3);
			AddSmallObjectData[i].decide = atoi(str3);
#ifdef ENABLE_ADDOBJ_PARAM8BIT
			AddSmallObjectData[i].decide = AddSmallObjectData[i].decide & 0x0000007F;
#endif

			//Durability.
			DeleteLinefeed(str4);
			AddSmallObjectData[i].hp = atoi(str4);
#ifdef ENABLE_ADDOBJ_PARAM8BIT
			AddSmallObjectData[i].hp = AddSmallObjectData[i].hp & 0x0000007F;
#endif

			//Sound Data Path
			DeleteLinefeed(str5);
			strcpy(AddSmallObjectData[i].soundpath, str5);

			//Knockback strength.
			DeleteLinefeed(str6);
			AddSmallObjectData[i].jump = atoi(str6);
#ifdef ENABLE_ADDOBJ_PARAM8BIT
			AddSmallObjectData[i].jump = AddSmallObjectData[i].jump & 0x000000FF;
#endif

			//Read the dummy line in front of the following additional accessory information
			if( i+1 < MAX_ADDSMALLOBJECT ){
				if( fgets(str, 32, fp) == NULL ){ break; }
			}
		}

		//Free File Handle
		fclose( fp );

#ifdef ENABLE_DEBUGLOG
		//Write to the log
		OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif
		return 0;
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_ERROR, "", "");
#endif
	return 1;
}

//! @brief Change the relative path to the addon folder to the .exe criteria -> .mif criteria
//! @param dir Full path to .mif file
//! @param fname File paths to be processed
//! @return Convert: true Unconverted: false
//! @attention Rewrite the pointer of fname directly.
bool MIFInterface::ChangeExePathToFullPath(char *dir, char *fname)
{
	if( dir == NULL ){ return false; }
	if( fname == NULL ){ return false; }

	char str[MAX_PATH];
	int index = 0;

	//No processing if invalid file path
	if( strcmp(fname, "") == 0 ){ return false; }
	if( strcmp(fname, "!") == 0 ){ return false; }

	//Determines whether the addon folder is referenced
	sprintf(str, "%s", ADDONDIR);
	if( memcmp(fname, str, strlen(str)) == 0 ){ index = (int)strlen(str); }
	sprintf(str, "\\%s", ADDONDIR);
	if( memcmp(fname, str, strlen(str)) == 0 ){ index = (int)strlen(str); }
	sprintf(str, "/%s", ADDONDIR);
	if( memcmp(fname, str, strlen(str)) == 0 ){ index = (int)strlen(str); }
	sprintf(str, ".\\%s", ADDONDIR);
	if( memcmp(fname, str, strlen(str)) == 0 ){ index = (int)strlen(str); }
	sprintf(str, "./%s", ADDONDIR);
	if( memcmp(fname, str, strlen(str)) == 0 ){ index = (int)strlen(str); }

	//If you refer to the addon folder
	if( index != 0 ){
		//Abort if the combined string would exceed MAX_PATH.
		if( strlen(dir)+strlen(fname)-index+1 >= MAX_PATH -1 ){ return false; }

		//Remove addon characters
		strcpy(str, fname);
		for(int i=0; i<(int)strlen(str)-index+1; i++){ str[i] = str[i+index]; }

		//Path up to .mif + file path to be processed
		strcpy(fname, dir);
		strcat(fname, str);
		return true;
	}
	return false;
}

//! @brief Get the format of the imported data file
//! @return Standard format: false MIF format: true
//! @attention Call this after successfully loading the file.
bool MIFInterface::GetFiletype()
{
	return mif;
}

//! @brief d a mission identification name
//! @return Pointer of identifying name (maximum 24)
char* MIFInterface::GetMissionName()
{
	return mission_name;
}

//! @brief d the official mission name
//! @return Official name pointer (max. 64)
char* MIFInterface::GetMissionFullname()
{
	return mission_fullname;
}

//! @brief Get block data and point data paths
//! @param *blockfile Pointer to receive block data
//! @param *pointfile Pointer to receive point data
void MIFInterface::GetDatafilePath(char *blockfile, char *pointfile)
{
	if( blockfile == NULL ){ return; }
	if( pointfile == NULL ){ return; }

	strcpy(blockfile, blockfile_path);
	strcpy(pointfile, pointfile_path);
}

//! @brief Get the background sky
//! @return Sky ID (0 to 5).
//! @attention Number 0 means "no background".
int MIFInterface::GetSkynumber()
{
	return skynumber;
}

//! @brief Get the path of a briefing image file
//! @param *picturefileA Pointer to receive the path of image file A
//! @param *picturefileB Pointer to receive the path of image file B
//! @attention "!" will be returned if you don't set it.
//! @attention If only one image is used, the image file B returns "!".
void MIFInterface::GetPicturefilePath(char *picturefileA, char *picturefileB)
{
	if( picturefileA == NULL ){ return; }
	if( picturefileB == NULL ){ return; }

	strcpy(picturefileA, picturefileA_path);
	strcpy(picturefileB, picturefileB_path);
}

//! @brief Bleefing text (text) acquired.
//! @return Pointer of writing (Max.:816)
//! @attention Up to 17 lines will be returned at once, including the newline code.
//! @attention Use GetTotaldatas() to get the number of rows.
char* MIFInterface::GetBriefingText()
{
	return briefingtext;
}

//! @brief Get the additional collision-detection flag.
//! @return enabled: true disabled: false
bool MIFInterface::GetCollisionFlag()
{
	return collision;
}

//! @brief Get a flag that shows the screen dark
//! @return enabled: true disabled: false
bool MIFInterface::GetScreenFlag()
{
	return screen;
}

//! @brief Get additional accessory information file path
//! @return Pointer for additional accessory information file path (max._MAX_PATH)
//! @attention This function is normally not necessary. Information in the file such as the model data path can be retrieved by other functions.
char* MIFInterface::GetAddSmallobjectFile()
{
	return addsmallobject_path;
}

//! @brief Get the model data path for additional accessories
//! @param id Data number (usually 0 fixed)
//! @return Pointer of model data path (max._MAX_PATH)
char* MIFInterface::GetAddSmallobjectModelPath(int id)
{
	if( (id < 0)||(MAX_ADDSMALLOBJECT <= id) ){ return NULL; }
	return AddSmallObjectData[id].modelpath;
}

//! @brief Get Texture Path for Additional Accessories
//! @param id Data number (usually 0 fixed)
//! @return Texture path pointer (max._MAX_PATH)
char* MIFInterface::GetAddSmallobjectTexturePath(int id)
{
	if( (id < 0)||(MAX_ADDSMALLOBJECT <= id) ){ return NULL; }
	return AddSmallObjectData[id].texturepath;
}

//! @brief s the size of the judgement for additional accessories
//! @param id Data number (usually 0 fixed)
//! @return Collision volume size.
int MIFInterface::GetAddSmallobjectDecide(int id)
{
	if( (id < 0)||(MAX_ADDSMALLOBJECT <= id) ){ return 0; }
	return AddSmallObjectData[id].decide;
}

//! @brief es endurance for additional accessories
//! @param id Data number (usually 0 fixed)
//! @return Durability.
int MIFInterface::GetAddSmallobjectHP(int id)
{
	if( (id < 0)||(MAX_ADDSMALLOBJECT <= id) ){ return 0; }
	return AddSmallObjectData[id].hp;
}

//! @brief Get the sound data path for additional accessories
//! @param id Data number (usually 0 fixed)
//! @return Sound data path pointer (max._MAX_PATH)
char* MIFInterface::GetAddSmallobjectSoundPath(int id)
{
	if( (id < 0)||(MAX_ADDSMALLOBJECT <= id) ){ return NULL; }
	return AddSmallObjectData[id].soundpath;
}

//! @brief es additional accessories.
//! @param id Data number (usually 0 fixed)
//! @return Knockback strength.
int MIFInterface::GetAddSmallobjectJump(int id)
{
	if( (id < 0)||(MAX_ADDSMALLOBJECT <= id) ){ return 0; }
	return AddSmallObjectData[id].jump;
}

//! @brief Constructor
AddonList::AddonList()
{
	datas = 0;

	for(int i=0; i<MAX_ADDONLIST; i++){
		strcpy(filename[i], "");
		strcpy(mission_name[i], "");
	}
}

//! @brief Destructor
AddonList::~AddonList()
{}

//! @brief Get .mif file
void AddonList::GetMIFlist(const char *dir)
{
	if( dir == NULL ){ return; }

	char SearchDIR[_MAX_PATH];
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;

	//mrate .mif search conditions
	strcpy(SearchDIR, dir);
	strcat(SearchDIR, "\\*.mif");

	//Search.
	hFind = FindFirstFile(SearchDIR, &FindFileData);
	if( hFind != INVALID_HANDLE_VALUE ){
		strcpy(filename[0], FindFileData.cFileName);
		datas += 1;

		while( FindNextFile(hFind, &FindFileData) == TRUE ){
			strcpy(filename[datas], FindFileData.cFileName);
			datas += 1;
		}
	}
	FindClose(hFind);
}

//! @brief Get the mission name
void AddonList::GetMissionName(const char *dir)
{
	if( dir == NULL ){ return; }

	for(int i=0; i<datas; i++){
		char str[_MAX_PATH];
		MIFInterface mifdata;

		//Generate the filename.
		strcpy(str, dir);
		strcat(str, "\\");
		strcat(str, filename[i]);

		//Load using MIFInterface.
		mifdata.LoadFiledata(str, false);

		//Get the mission name
		strcpy(mission_name[i], mifdata.GetMissionName());
	}
}

//! @brief Sort mission names.
void AddonList::Sort()
{
	if( datas <= 1 ){ return; }

	char mission_name_c[MAX_ADDONLIST][24];
	char temp[_MAX_PATH];
	int cmp;

	//Convert the mission name to lowercase
	for(int i=0; i<datas; i++){
		for(int j=0; j<(int)strlen(mission_name[i]); j++){
			mission_name_c[i][j] = (char)tolower(mission_name[i][j]);
		}
		mission_name_c[i][strlen(mission_name[i])] = '\0';
	}

	//Low speed bubble sorting (^ ^;
	for(int i=0; i<datas-1; i++){
		for(int j=i+1; j<datas; j++){
			cmp = strcmp(mission_name_c[i], mission_name_c[j]);
			if(cmp > 0){
				strcpy(temp, mission_name_c[i]);
				strcpy(mission_name_c[i], mission_name_c[j]);
				strcpy(mission_name_c[j], temp);

				strcpy(temp, mission_name[i]);
				strcpy(mission_name[i], mission_name[j]);
				strcpy(mission_name[j], temp);

				strcpy(temp, filename[i]);
				strcpy(filename[i], filename[j]);
				strcpy(filename[j], temp);
			}
		}
	}

}

//! @brief Get ADDON List
//! @param dir directory with ADDON(.mif) (standard: "addon\\")
//! @return Total number of addon
int AddonList::LoadFiledata(const char *dir)
{
	if( dir == NULL ){ return 0; }

	datas = 0;

	//Get .mif file
	GetMIFlist(dir);

	//Get the mission name
	GetMissionName(dir);

	//Sort mission names.
	Sort();

#ifdef ENABLE_DEBUGLOG
	char str[128];
	sprintf(str, "Research addon directory (Directory name : [%s]  Total : %d )", dir, datas);

	//Write to the log
	OutputLog.WriteLog(LOG_CHECK, "Directory", str);
#endif

	return datas;
}

//! @brief Get the mission name
//! @param id Data Number
//! @return Mission Name
char* AddonList::GetMissionName(int id)
{
	if( (id < 0)||(MAX_ADDONLIST <= id ) ){ return NULL; }
	return mission_name[id];
}

//! @brief Get File Name
//! @param id Data Number
//! @return File Name
char* AddonList::GetFileName(int id)
{
	if( (id < 0)||(MAX_ADDONLIST <= id ) ){ return NULL; }
	return filename[id];
}

//! @brief Constructor
INIFileInterface::INIFileInterface()
{
	inifp = NULL;
}

//! @brief Destructor
INIFileInterface::~INIFileInterface()
{
	ReleaseINIFile();
}

//! @brief Read INI Files
//! @param fname File Name
//! @return Success:false Fail:true
//! @attention If an INI file is already loaded, this function fails until ReleaseINIFile() is called.
bool INIFileInterface::LoadINIFile(const char *fname)
{
	if( fname == NULL ){ return true; }
	if( inifp != NULL ){ return true; }

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	fname = ChangePathDelimiter(fname);
#endif

	inifp = fopen(fname, "rb");
	if( inifp == NULL ){ return true; }

	return false;
}

//! @brief Get a string
//! @param sectionname Section name (NULL allowed).
//! @param keyname Key Name
//! @param defaultvalue default value
//! @param valuestr Value.
//! @param strbuflen Maximum number of characters (disabled in -1)
//! @return Error Codes
//! @attention Returns the default value if the file is not opened or the key is not found.
//! @attention If the acquired string exceeds the maximum number of characters, it is cut to the maximum number of characters.
//! @note Error code 1: Argument failure, 2: File is not opened, 3: Not found
int INIFileInterface::GetINIFileString(const char *sectionname, const char *keyname, const char *defaultvalue, char *valuestr, int strbuflen)
{
	if( valuestr == NULL ){ return 1; }
	if( defaultvalue == NULL ){ strcpy(valuestr, ""); return 1; }
	if( keyname == NULL ){ strcpy(valuestr, defaultvalue); return 1; }

	int state = 0;
	char readline[512];
	char buf[512];

	//Failed if the file is not loaded
	if( inifp == NULL ){
		strcpy(valuestr, defaultvalue);
		return 2;
	}

	fseek(inifp, 0, SEEK_SET);
	strcpy(readline, "");

	//If the section name is not specified, the section name judgment is disabled.
	if( sectionname == NULL ){
		state = 1;
	}
	else if( sectionname[0] == '\0' ){
		state = 1;
	}

	//Data size variable check
	if( strbuflen <= -2 ){ strbuflen = 0; }
	if( strbuflen == -1 ){ strbuflen = 512; }
	if( strbuflen > 512 ){ strbuflen = 512; }

	while( fgets(readline, 512, inifp) != NULL ){
		//Leave a comment
		if( readline[0] == ';' ){ continue; }

		//Section Name Determination
		if( state == 0 ){//if( sectionname != NULL ){
			char str[64];
			sprintf(str, "[%s]", sectionname);
			strcpy(buf, readline);
			buf[ strlen(str) ] = '\0';
			if( strcmp(buf, str) == 0 ){
				state = 1;
				continue;
			}
		}


		//Key Name Determination
		if( state == 1 ){
			//Invalid if the first character is "["
			if( readline[0] == '[' ){ break; }

			char readline2[512];
			int strcnt = 0;
			int quotationmode = 0;
			char str[64];

			//Generate text without spaces, comments, or empty quotes ("").
			strcpy(readline2, "");
			for(int i=0; i<(int)strlen(readline); i++){
				if( (readline[i] == '\r')||(readline[i] == '\n') ){ break; }

				if( quotationmode == 0 ){
					if( (readline[i] == ' ')||(readline[i] == '\t') ){ continue; }
					if( readline[i] == ';' ){ break; }

					if( readline[i] == '\'' ){
						quotationmode = 1;
						continue;
					}
					if( readline[i] == '"' ){
						quotationmode = 2;
						continue;
					}
				}
				if( (quotationmode == 1)&&(readline[i] == '\'') ){
					quotationmode = 0;
					continue;
				}
				if( (quotationmode == 2)&&(readline[i] == '"') ){
					quotationmode = 0;
					continue;
				}

				readline2[strcnt] = readline[i];
				readline2[strcnt+1] = '\0';
				strcnt += 1;
			}

			//Determine the character behind "="
			sprintf(str, "%s=", keyname);
			strcpy(buf, readline2);
			buf[ strlen(str) ] = '\0';
			if( strcmp(buf, str) == 0 ){
				state = 1;

				//Data size adjustment
				strcpy(buf, &(readline2[strlen(str)]));
				buf[strbuflen-1] = '\0';

				//Returns a value
				strcpy(valuestr, buf);
				return 0;
			}
		}
	}

	//Returns an error if the value is not found
	strcpy(valuestr, defaultvalue);
	return 3;
}

//! @brief Get int value
//! @param sectionname Section name (NULL allowed).
//! @param keyname Key Name
//! @param defaultvalue default value
//! @param errorcode Pointer receiving the error code (NULL allowed).
//! @return Value.
//! @attention Returns the default value if the file is not opened or the key is not found.
//! @note Error code 1: Argument failure, 2: File is not opened, 3: Not found
int INIFileInterface::GetINIFileInt(const char *sectionname, const char *keyname, int defaultvalue, int *errorcode)
{
	if( keyname == NULL ){ return defaultvalue; }

	char defaultint[64];
	char buf[64];
	int error;

	sprintf(defaultint, "%d", defaultvalue);
	error = GetINIFileString(sectionname, keyname, defaultint, buf, 64);
	if( errorcode != NULL ){ *errorcode = error; }
	return atoi(buf);
}

//! @brief Get float value
//! @param sectionname Section name (NULL allowed).
//! @param keyname Key Name
//! @param defaultvalue default value
//! @param errorcode Pointer receiving the error code (NULL allowed).
//! @return Value.
//! @attention Returns the default value if the file is not opened or the key is not found.
//! @note Error code 1: Argument failure, 2: File is not opened, 3: Not found
float INIFileInterface::GetINIFileFloat(const char *sectionname, const char *keyname, float defaultvalue, int *errorcode)
{
	if( keyname == NULL ){ return defaultvalue; }

	char defaultfloat[64];
	char buf[64];
	int error;

	sprintf(defaultfloat, "%f", defaultvalue);
	error = GetINIFileString(sectionname, keyname, defaultfloat, buf, 64);
	if( errorcode != NULL ){ *errorcode = error; }
	for(int i=(int)strlen(buf)-1; i>0; i--){
		if( buf[i] == 'f' ){ buf[i] = '\0'; }
		if( buf[i] == 'F' ){ buf[i] = '\0'; }
	}
	return (float)atof(buf);
}

//! @brief Get bool value
//! @param sectionname Section name (NULL allowed).
//! @param keyname Key Name
//! @param defaultvalue default value
//! @param errorcode Pointer receiving the error code (NULL allowed).
//! @return Value.
//! @attention Returns the default value if the file is not opened or the key is not found.
//! @note Error code 1: Argument failure, 2: File is not opened, 3: Not found
bool INIFileInterface::GetINIFileBool(const char *sectionname, const char *keyname, bool defaultvalue, int *errorcode)
{
	if( keyname == NULL ){ return defaultvalue; }

	char defaultbool[64];
	char buf[64];
	int error;

	if( defaultvalue == false ){
		strcpy(defaultbool, "false");
	}
	else{
		strcpy(defaultbool, "true");
	}
	error = GetINIFileString(sectionname, keyname, defaultbool, buf, 64);
	if( errorcode != NULL ){ *errorcode = error; }
	for(int j=0; j<=(int)strlen(buf); j++){ if(('A'<=buf[j])&&(buf[j]<='Z')){ buf[j] += 32; } }
	if( strcmp(buf, "false") == 0 ){
		return false;
	}
	else if( strcmp(buf, "0") == 0 ){
		return false;
	}
	//else{
		return true;
	//}
}

//! @brief Free INI files
void INIFileInterface::ReleaseINIFile()
{
	if( inifp == NULL ){ return; }

	fclose(inifp);
	inifp = NULL;
}


//! @brief Remove line endings for fgets().
//! @param str String.
//! @return Replacement made: 0; no replacement: 1.
int DeleteLinefeed(char *str)
{
	if( str == NULL ){ return 1; }

	char *pstr;
	bool flag = false;

	pstr = strchr(str, '\n');
	if( pstr ){
		*pstr = '\0';
		flag = true;
	}

	pstr = strchr(str, '\r');
	if( pstr ){
		*pstr = '\0';
		flag = true;
	}

	if( flag == true ){
		return 0;
	}
	return 1;
}

//! @brief Determines whether the file path is full path
//! @param path File path
//! @return Full path (absolute path): true non-full path (relative path): false
//! @warning Only Windows environment is assumed
bool CheckFullPath(const char *path)
{
	if( path == NULL ){ return false; }
	if( strlen(path) < 2 ){ return false; }

	if( path[1] == ':' ){ return true; }
	return false;
}

//! @brief Remove file name from file path
//! @param path File path (pointer)
//! @param dir Pointer to receive directory name
void GetFileDirectory(const char *path, char *dir)
{
	if( path == NULL ){ return; }
	if( dir == NULL ){ return; }

	strcpy(dir, path);

	//'\\' or '/' from the end, '\0'
	for(int i=(int)strlen(dir)-1; i>0; i--){
		if( dir[i] == '\\' ){ dir[i+1] = '\0'; break; }
		if( dir[i] == '/' ){ dir[i+1] = '\0'; break; }
	}
}

//! @brief Determine file extension
//! @param filepath File Name
//! @param checkstr Filename extension.
//! @return Applicable: true Not applicable: false
//! @attention Please specify the extension with "".".It does not distinguish case or case.
bool CheckFileExtension(const char *filepath, const char *checkstr)
{
	if( filepath == NULL ){ return false; }
	if( checkstr == NULL ){ return false; }
	if( strlen(filepath) >= MAX_PATH ){ return false; }
	if( strlen(checkstr) >= 16 ){ return false; }
	if( strlen(filepath) < strlen(checkstr) ){ return false; }

	char filepath2[MAX_PATH];
	char checkstr2[16];

	//Convert file name to lowercase
	for(int i=0; i<(int)strlen(filepath); i++){
		filepath2[i] = (char)tolower((int)filepath[i]);
	}
	filepath2[ strlen(filepath) ] = '\0';

	//Convert the file extension to lowercase
	for(int i=0; i<(int)strlen(checkstr); i++){
		checkstr2[i] = (char)tolower((int)checkstr[i]);
	}
	checkstr2[ strlen(checkstr) ] = '\0';

	//Determining the extension
	for(int i=(int)strlen(filepath2)-1; i>0; i--){
		if( filepath2[i] == '.' ){
			if( strcmp(&(filepath2[i]), checkstr2) == 0 ){ return true; }
			return false;
		}
	}
	return false;
}

//! @brief Determines the existence of directories
//! @param dir Directory name
//! @return true non-existent: false
bool CheckDirectory(const char *dir)
{
	struct stat statBuf;
	if( stat(dir, &statBuf) == 0 ){ return true; } 
	return false;
}
