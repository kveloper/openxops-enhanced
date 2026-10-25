//! @file datafile.h
//! @brief Data Management Class Declaration

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

#ifndef DATAFILE_H
#define DATAFILE_H

#define MAX_BLOCKS 160			//!< Maximum number of blocks
#define TOTAL_BLOCKTEXTURE 10	//!< Total textures

//#define MAX_POINTS 200
#define MAX_POINTS 256			//!< Maximum Points

//! @brief Number of additional accessories
//! @attention When increasing these settings, also review MAX_MODEL, MAX_TEXTURE, and MAX_LOADSOUND.
//! @note At the bottom of the additional accessory configuration file, the following additional accessory setting values are described. However, please insert the dummy line ("//"recommended) between the setting values of each additional accessory.
#define MAX_ADDSMALLOBJECT 1

#define MAX_POINTMESSAGES 16			//!< Maximum number of messages in the .msg file
#define MAX_POINTMESSAGEBYTE (71+2)		//!< .msg file maximum bytes per line

#define LIGHT_RX DegreeToRadian(190)	//!< Horizontal light angle (for block shading).
#define LIGHT_RY DegreeToRadian(120)	//!< Vertical light angle (for block shading).

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 1		//!< Select include file.
#endif
#include "main.h"

#include <ctype.h>
#include <sys/stat.h>

//! Structure for block data (use with blockdata structure)
typedef struct
{
	int textureID;				//!< textureID
	float u[4];					//!< textureUV-U
	float v[4];					//!< textureUV-V
	float center_x;				//!< center
	float center_y;				//!< center
	float center_z;				//!< center
	float vx;					//!< normal vector 
	float vy;					//!< normal vector 
	float vz;					//!< normal vector 
	float shadow;				//!< shadow 
} b_material;
//! Structure for block data
typedef struct
{
	int id;						//!< DataID
	float x[8];					//!< Position
	float y[8];					//!< Position
	float z[8];					//!< Position
	b_material material[6];		//!< Material data
} blockdata;

//! Structure for Point Data
typedef struct
{
	int id;						//!< DataID
	float x;					//!< Position
	float y;					//!< Position
	float z;					//!< Position
	float r;					//!< Rotation
	signed short int p1;		//!< Param
	signed short int p2;		//!< Param
	signed short int p3;		//!< Param
	signed short int p4;		//!< Param
} pointdata;

//P1 type number of point data
#define POINT_P1TYPE_HUMAN			1	//!< Param1 type
#define POINT_P1TYPE_WEAPON			2	//!< Param1 type
#define POINT_P1TYPE_AIPATH			3	//!< Param1 type
#define POINT_P1TYPE_HUMANINFO		4	//!< Param1 type
#define POINT_P1TYPE_SMALLOBJ		5	//!< Param1 type
#define POINT_P1TYPE_HUMAN2			6	//!< Param1 type
#define POINT_P1TYPE_RAND_WEAPON	7	//!< Param1 type
#define POINT_P1TYPE_RAND_AIPATH	8	//!< Param1 type

//! Structure for additional accessories
typedef struct
{
	char modelpath[_MAX_PATH];		//!< Model Data Path
	char texturepath[_MAX_PATH];	//!< Texture Path
	int decide;						//!< Collision volume size.
	int hp;							//!< Durability.
	char soundpath[_MAX_PATH];		//!< Sound Data Path
	int jump;						//!< Knockback strength.
} addsmallobject;

//! @brief Data management class (basal class)
//! @details Base class for classes that load game data from files.
class DataInterface
{
protected:
	int datas;		//!< Number of Data

public:
	DataInterface();
	~DataInterface();
	virtual int LoadFiledata(const char *fname);
	virtual int GetTotaldatas();
	virtual int Getdata(void *out_data, int id);
};

//! @brief Class to manage block data
//! @details Manage block-data files used as map data.
//! @details In addition to loading files, calculate map shading (the brightness of each face).
class BlockDataInterface : public DataInterface
{
	blockdata *data;								//!< Structure representing block data
	char texture[TOTAL_BLOCKTEXTURE][_MAX_PATH];	//!< Texture Name

public:
	BlockDataInterface();
	~BlockDataInterface();
	int LoadFiledata(const char *fname);
	void CalculationBlockdata(bool darkflag);
	int GetTexture(char *fname, int id);
	int Getdata(blockdata *out_data, int id);
};

bool blockdataface(int faceID, int* vID, int* uvID);

//! @brief Class to manage point data
//! @details Manage point data files used as mission data.
//! @details In addition to reading files, you can also search for points.
class PointDataInterface : public DataInterface
{
	pointdata *data;							//!< Structure representing block data
	char *text[MAX_POINTMESSAGES];				//!< Event Message

	int LoadMSGFiledata(char *fname);

public:
	PointDataInterface();
	~PointDataInterface();
	int LoadFiledata(const char *fname);
	int Getdata(pointdata *out_data, int id);
	int SetParam(int id, signed short int p1, signed short int p2, signed short int p3, signed short int p4);
	int GetMessageText(char *str, int id);
	int SearchPointdata(int* id, unsigned char pmask, signed short int p1, signed short int p2, signed short int p3, signed short int p4, int offset = 0);
	int SearchPointdata(pointdata *out_data, unsigned char pmask, signed short int p1, signed short int p2, signed short int p3, signed short int p4, int offset = 0);
};

//! @brief F to manage MIF
//! @details Manage MIF (MISSION INFORMATION FILE).
//! @details Standard mission briefing file (.txt) is also supported.
class MIFInterface : public DataInterface
{
	bool mif;								//!< File Format .mif
	char mission_name[24];					//!< Mission identification name
	char mission_fullname[64];				//!< Mission official name
	char blockfile_path[_MAX_PATH];			//!< Block Data File
	char pointfile_path[_MAX_PATH];			//!< Point Data File
	int skynumber;							//!< Background empty ID (None:0)
	char picturefileA_path[_MAX_PATH];		//!< Briefing image A
	char picturefileB_path[_MAX_PATH];		//!< Briefing image B (additional image).
	char addsmallobject_path[_MAX_PATH];	//!< Path to Additional Accessories Information File
	char briefingtext[816];					//!< Briefing Texts and Texts
	bool collision;					//!< Perform additional collision checks.
	bool screen;					//!< Darken the screen
	addsmallobject *AddSmallObjectData;		//!< Additional accessories

	int LoadDefaultTextFiledata(const char *fname);
	int LoadMissionInfoFiledata(const char *fname);
	int LoadAddSmallObjectFiledata(const char *fname);
	bool ChangeExePathToFullPath(char *dir, char *fname);

public:
	MIFInterface();
	~MIFInterface();
	int LoadFiledata(const char *fname, bool addfile);
	bool GetFiletype();
	char* GetMissionName();
	char* GetMissionFullname();
	void GetDatafilePath(char *blockfile, char *pointfile);
	int GetSkynumber();
	void GetPicturefilePath(char *picturefileA, char *picturefileB);
	char* GetBriefingText();
	bool GetCollisionFlag();
	bool GetScreenFlag();
	char* GetAddSmallobjectFile();
	char* GetAddSmallobjectModelPath(int id);
	char* GetAddSmallobjectTexturePath(int id);
	int GetAddSmallobjectDecide(int id);
	int GetAddSmallobjectHP(int id);
	char* GetAddSmallobjectSoundPath(int id);
	int GetAddSmallobjectJump(int id);
};

//! @brief ADDON List Management Class
//! @details Manage the .mif in a specific directory as the ADDON list.
class AddonList : public DataInterface
{
	char filename[MAX_ADDONLIST][_MAX_PATH];	//!< .mif filename
	char mission_name[MAX_ADDONLIST][24];		//!< Mission identification name

	void GetMIFlist(const char *dir);
	void GetMissionName(const char *dir);
	void Sort();

public:
	AddonList();
	~AddonList();
	int LoadFiledata(const char *dir);
	char *GetMissionName(int id);
	char *GetFileName(int id);
};

//! @brief Class for managing INI files
//! @details Manage INI files.
class INIFileInterface
{
	FILE *inifp;	//!< File pointer

public:
	INIFileInterface();
	~INIFileInterface();
	bool LoadINIFile(const char *fname);
	int GetINIFileString(const char *sectionname, const char *keyname, const char *defaultvalue, char *valuestr, int strbuflen);
	int GetINIFileInt(const char *sectionname, const char *keyname, int defaultvalue, int *errorcode);
	float GetINIFileFloat(const char *sectionname, const char *keyname, float defaultvalue, int *errorcode);
	bool GetINIFileBool(const char *sectionname, const char *keyname, bool defaultvalue, int *errorcode);
	void ReleaseINIFile();
};

int DeleteLinefeed(char *str);
bool CheckFullPath(const char *path);
void GetFileDirectory(const char *path, char *dir);
bool CheckFileExtension(const char *filepath, const char *checkstr);
bool CheckDirectory(const char *dir);

#endif