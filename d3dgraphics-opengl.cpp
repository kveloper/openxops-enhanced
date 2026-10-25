//! @file d3dgraphics-opengl.cpp
//! @brief D3DGraphics class definitions (OpenGL implementation).

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

// ***** OpenGL core only *****
//
// libjpeg
//    Copyright (C) 1991-2014, Thomas G. Lane, Guido Vollbeding.
//    this software is based in part on the work of the Independent JPEG Group
//
// zlib
//    Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler
//
// libpng
//    Copyright (c) 1998-2014 Glenn Randers-Pehrson
//
// ****************************

#include "d3dgraphics.h"
#include "presentation.h"
#include <vector>
#include <string>
#include <map>

#if GRAPHIC_ENGINE == 8

// Register only for this process; no system-wide font installation is needed.
struct GamePrivateFont {
    WCHAR path[MAX_PATH]; bool loaded;
    GamePrivateFont() : loaded(false) {
        GetFullPathNameW(L"data\\SB_Aggro_L.ttf", MAX_PATH, path, NULL);
        loaded = AddFontResourceExW(path, FR_PRIVATE, NULL) > 0;
        if(!loaded) OutputDebugStringA("Cannot load data/SB_Aggro_L.ttf; using the system fallback font.\n");
    }
    ~GamePrivateFont() { if(loaded) RemoveFontResourceExW(path, FR_PRIVATE, NULL); }
};
static HFONT CreateGameFont() {
    static GamePrivateFont file;
    int pixels = (int)(18.0f * GameConfig.GetScreenHeight()/480.0f);
    if(pixels < 1) pixels = 1;
    return CreateFontW(-pixels,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,
        file.loaded ? L"SB Aggro Light" : L"Dotum");
}

//! @brief Constructor
D3DGraphics::D3DGraphics()
{
	hWnd = NULL;
	hGLRC = NULL;
	width = 0;
	height = 0;
	fullscreenflag = false;
	LanguageMode = 0;
	SystemFont = NULL;
	SystemSmallFont = NULL;
	now_SystemFontUStr = new WCHAR [1];
	now_SystemFontUStr[0] = NULL;
	SystemFontListIdx = 0;
	SystemFontListIdxSize = 0;
	SystemFont_posz = 1.0f;
	now_textureid = -1;

	camera_x = 0.0f;
	camera_y = 0.0f;
	camera_z = 0.0f;
	camera_rx = 0.0f;
	camera_ry = 0.0f;
	viewangle = 0.0f;

	BlockDataIF = NULL;
	bs = 0;
	for(int i=0; i<TOTAL_BLOCKTEXTURE; i++){
		mapTextureID[i] = -1;
	}

	TextureFontFname[0] = '\0';
	TextureFont = -1;

	for( int i=0; i<TOTAL_HAVEWEAPON; i++ ){
		HUD_myweapon_x[i] = 0.0f;
		HUD_myweapon_y[i] = 0.0f;
		HUD_myweapon_z[i] = 0.0f;
	}

#ifdef ENABLE_DEBUGCONSOLE
	ZeroMemory(DebugFontData, sizeof(unsigned char)*96*16);
	TextureDebugFont = -1;
#endif
}

//! @brief Destructor
D3DGraphics::~D3DGraphics()
{
	DestroyD3D();
}

//! @brief Full-screen flag setting
//! @param fullscreen Full-screen flag
//! @attention This flag can also be set by InitD3D() during initialization.
void D3DGraphics::SetFullScreenFlag(bool fullscreen)
{
	fullscreenflag = fullscreen;
}

//! @brief Full-screen flag acquisition
//! @return Full-screen flag
bool D3DGraphics::GetFullScreenFlag()
{
	return fullscreenflag;
}

//! @brief Language Selection Settings
//! @param mode Language Selection
//! @attention This setting can also be specified through InitD3D() during initialization.
void D3DGraphics::SetlanguageMode(int mode)
{
	LanguageMode = mode;
}

//! @brief Set up language selection
//! @return Language Selection
int D3DGraphics::GetlanguageMode()
{
	return LanguageMode;
}

//! @brief Initialization @n
//! (OpenGL 1.1)
//! @param WindowCtrl WindowControl class pointer
//! @param TextureFontFilename Texture font file name to use
//! @param fullscreen false: window display true: full screen display
//! @param language English/Japanese:0English/Korean:1)
//! @return Success:0 failure:1
int D3DGraphics::InitD3D(WindowControl *WindowCtrl, const char *TextureFontFilename, bool fullscreen, int language)
{
	if( WindowCtrl == NULL ){ return 1; }
	if( TextureFontFilename == NULL ){ return 1; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_INIT, "Graphics", "OpenGL");
#endif

	hWnd = WindowCtrl->GethWnd();

	// Projection and UI retain the configured logical resolution.
	width = GameConfig.GetScreenWidth();
	height = GameConfig.GetScreenHeight();

	fullscreenflag = fullscreen;
	LanguageMode = language;

	// Borderless fullscreen keeps the desktop display mode unchanged.



	HDC hDC;
	int pfdID;
	BOOL bResult;

	//Pixel Format
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof (PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0,
		0, 0, 0,
		0, 0,
		0, 0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,
		0,
		0
	};

	//Get Device Context
	hDC = GetDC(hWnd);

	//Get pixel format
	pfdID = ChoosePixelFormat(hDC, &pfd);	
	if( pfdID == 0 ){ return 1; }

	//Pixel Format
	bResult = SetPixelFormat(hDC, pfdID, &pfd);
	if( bResult == FALSE ){ return 1; }

	//Context
	hGLRC = wglCreateContext(hDC);
	if( hGLRC == NULL ){ return 1; }
    wglMakeCurrent(hDC,hGLRC);
    // Request uncapped presentation; a driver-level override may still enforce VSync.
    typedef BOOL (WINAPI *SwapIntervalProc)(int);
    PROC swapProc=wglGetProcAddress("wglSwapIntervalEXT");
    if(swapProc && swapProc!=(PROC)1 && swapProc!=(PROC)2 && swapProc!=(PROC)3 && swapProc!=(PROC)-1)
        ((SwapIntervalProc)swapProc)(0);


	//Device Context Release
	ReleaseDC(hWnd, hDC);

	//System Font Preparation
	if( LanguageMode == 1 ){
		// ---English/Korean---

		//Font Name: MS Goth Size:18
		SystemFont = CreateFont((int)(((float)GameConfig.GetScreenHeight() / 480) * 18), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Dotum");
		//Font Name: MS Goth Size:12
		SystemSmallFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Dotum");
	}
	else{
		// ---English/Japanese---

		//Font Name: MS Goth Size:18
		//SystemFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "MS Gothic");
		SystemFont = CreateFont((int)(((float)GameConfig.GetScreenHeight() / 480) * 18), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "ÇlÇr ÉSÉVÉbÉN");
		//Font Name: MS Goth Size:12
		SystemSmallFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "ÇlÇr ÉSÉVÉbÉN");
	}

    DeleteObject(SystemFont);
    SystemFont = CreateGameFont();

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif

	//Set the file name of the image for texture font
	strcpy(TextureFontFname, TextureFontFilename);

	//Get images for texture fonts
	TextureFont = LoadTexture(TextureFontFname, true, false);


	//HUD Coordinate to draw weapons currently possessed
	HUD_myweapon_x[0] = GameConfig.GetScreenWidth() - 140.0f;
	HUD_myweapon_y[0] = GameConfig.GetScreenHeight() - 40.0f;
	HUD_myweapon_z[0] = 0.86f;

	//HUD: coordinates for rendering the spare weapon.
	HUD_myweapon_x[1] = GameConfig.GetScreenWidth() - 72.0f;
	HUD_myweapon_y[1] = GameConfig.GetScreenHeight() - 25.0f;
	HUD_myweapon_z[1] = 0.93f;


#ifdef ENABLE_DEBUGCONSOLE
	InitDebugFontData();
	if( LoadDebugFontTexture() == false ){
		return 1;
	}
#endif


#ifdef ENABLE_OPENGL_LIBJPEGPNG
	//Initialize libjpeg.
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
#endif

	return 0;
}

//! @brief Reset @n
//! (e.g. return from window minimization)
//! @param WindowCtrl WindowControl class pointer
//! @return Success: 0; waiting: 1; failure: 2.
int D3DGraphics::ResetD3D(WindowControl *WindowCtrl)
{
	if( WindowCtrl == NULL ){ return 2; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_INIT, "Graphics", "OpenGL (Reset)");
#endif

	//Resource Release
	CleanupD3Dresource();

	if( hGLRC != NULL ){
		wglDeleteContext(hGLRC);
		hGLRC = NULL;
	}


	//  End of resource cleanup.
	// Initialization processing from here

	hWnd = WindowCtrl->GethWnd();

	// Borderless fullscreen keeps the desktop display mode unchanged.



	HDC hDC;
	int pfdID;
	BOOL bResult;

	//Pixel Format
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof (PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0,
		0, 0, 0,
		0, 0,
		0, 0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,
		0,
		0
	};

	//Get Device Context
	hDC = GetDC(hWnd);

	//Get pixel format
	pfdID = ChoosePixelFormat(hDC, &pfd);	
	if( pfdID == 0 ){ return 1; }

	//Pixel Format
	bResult = SetPixelFormat(hDC, pfdID, &pfd);
	if( bResult == FALSE ){ return 1; }

	//Context
	hGLRC = wglCreateContext(hDC);
	if( hGLRC == NULL ){ return 1; }
    wglMakeCurrent(hDC,hGLRC);
    // Request uncapped presentation; a driver-level override may still enforce VSync.
    typedef BOOL (WINAPI *SwapIntervalProc)(int);
    PROC swapProc=wglGetProcAddress("wglSwapIntervalEXT");
    if(swapProc && swapProc!=(PROC)1 && swapProc!=(PROC)2 && swapProc!=(PROC)3 && swapProc!=(PROC)-1)
        ((SwapIntervalProc)swapProc)(0);


	//Device Context Release
	ReleaseDC(hWnd, hDC);

	//System Font Preparation
	if( LanguageMode == 1 ){
		// ---English/Korean---

		//Font Name: MS Goth Size:18
		SystemFont = CreateFont((int)(((float)GameConfig.GetScreenHeight() / 480) * 18), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Dotum");
		//Font Name: MS Goth Size:12
		SystemSmallFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Dotum");
	}
	else{
		// ---English/Japanese---

		//Font Name: MS Goth Size:18
		//SystemFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "MS Gothic");
		SystemFont = CreateFont((int)(((float)GameConfig.GetScreenHeight() / 480) * 18), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "ÇlÇr ÉSÉVÉbÉN");
		//Font Name: MS Goth Size:12
		SystemSmallFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "ÇlÇr ÉSÉVÉbÉN");
	}

    DeleteObject(SystemFont);
    SystemFont = CreateGameFont();

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif

	//Get images for texture fonts
	TextureFont = LoadTexture(TextureFontFname, true, false);

#ifdef ENABLE_DEBUGCONSOLE
	if( LoadDebugFontTexture() == false ){
		return 1;
	}
#endif

	return 0;
}

//! @brief Release resources.
//! @attention This function is automatically called, but it can be called explicitly.
void D3DGraphics::DestroyD3D()
{
	if( hGLRC == NULL ){ return; }

	CleanupD3Dresource();

	if( hGLRC != NULL ){
		wglDeleteContext(hGLRC);
		hGLRC = NULL;
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Graphics", "OpenGL");
#endif

#ifdef ENABLE_OPENGL_LIBJPEGPNG
	//libjpeg release
	jpeg_destroy_decompress(&cinfo);
#endif
}

//! @brief Release device resources
void D3DGraphics::CleanupD3Dresource()
{
	if( TextureFont != -1 ){
		CleanupTexture(TextureFont);
		TextureFont = -1;
	}

#ifdef ENABLE_DEBUGCONSOLE
	if( TextureDebugFont != -1 ){
		CleanupTexture(TextureDebugFont);
		TextureDebugFont = -1;
	}
#endif

	for(int i=0; i<MAX_MODEL; i++){
		CleanupModel(i);
	}
	for(int i=0; i<MAX_TEXTURE; i++){
		CleanupTexture(i);
	}

	if( SystemFont != NULL ){
		DeleteObject(SystemFont);
		SystemFont = NULL;
	}
	if( SystemSmallFont != NULL ){
		DeleteObject(SystemSmallFont);
		SystemSmallFont = NULL;
	}
	if( now_SystemFontUStr != NULL ){
		delete [] now_SystemFontUStr;
		now_SystemFontUStr = NULL;
	}
	if( SystemFontListIdx != 0 ){
		glDeleteLists(SystemFontListIdx, SystemFontListIdxSize);
		SystemFontListIdx = 0;
	}
}

//! @brief Read model files (.x)
//! @param filename File Name
//! @return Success: Model data number (0 or more) Failure: -1
int D3DGraphics::LoadModel(const char* filename)
{
	if( filename == NULL ){ return -1; }

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "Model", filename);
#endif

	int id = -1;
	FILE *fp;
	char buf[256];
	char str[256];

	int vertexs = 0;
	MODELVDATA *vertex = NULL;
	int polygons = 0;
	int *index = NULL;
	int in_vertexs, in_polygons;
	MODELVDATA *old_vertex;
	int *old_index;

	char stroks[] = " ;,";		//Delimiter string.

	//Finding an empty data number
	for(int i=0; i<MAX_MODEL; i++){
		if( pmodel[i].useflag == false ){
			id = i;
			break;
		}
	}
	if( id == -1 ){ return -1; }

	//Load files
	fp = fopen(filename, "r");
	if( fp == NULL ){ return -1; }

	//Get Magic Code
	fgets(buf, 256, fp);
	buf[ strlen("xof 0302txt") ] = '\0';
	if( strcmp(buf, "xof 0302txt") != 0 ){
		fclose( fp );
		return -1;		//not X file
	}

	while( fgets(buf, 256, fp) != NULL ){
		strcpy(str, buf);
		str[ strlen("Mesh") ] = '\0';
		if( strcmp(str, "Mesh") == 0 ){
			
			fgets(buf, 256, fp);
			in_vertexs = atoi(buf);

			if( vertexs == 0 ){
				//If the first mesh data is used, you only need to create the area.
				vertex = new MODELVDATA [in_vertexs];
			}
			else{
				//After the second time, re-copies the area and deletes the old area.
				old_vertex = vertex;
				vertex = new MODELVDATA [vertexs+in_vertexs];
				memcpy(vertex, old_vertex, sizeof(MODELVDATA)*vertexs);
				delete [] old_vertex;
			}

			for(int i=0; i<in_vertexs; i++){
				fgets(buf, 256, fp);
				vertex[i+vertexs].x = (float)atof( strtok(buf, stroks) ) * -1;
				vertex[i+vertexs].y = (float)atof( strtok(NULL, stroks) );
				vertex[i+vertexs].z = (float)atof( strtok(NULL, stroks) );
			}

			fgets(buf, 256, fp);

			fgets(buf, 256, fp);
			in_polygons = atoi(buf);

			if( polygons == 0 ){
				//If the first index data is used, you only need to create an area.
				index = new int [in_polygons*5];
			}
			else{
				//After the second time, re-copies the area and deletes the old area.
				old_index = index;
				index = new int [(polygons+in_polygons)*5];
				memcpy(index, old_index, sizeof(int)*polygons*5);
				delete [] old_index;
			}

			for(int i=0; i<in_polygons; i++){
				fgets(buf, 256, fp);
				index[(i+polygons)*5] = atoi( strtok(buf, stroks) );
				for(int j=0; j<index[(i+polygons)*5]; j++){
					index[(i+polygons)*5 + j + 1] = atoi( strtok(NULL, stroks) ) + vertexs;
				}
			}

			while( fgets(buf, 256, fp) != NULL ){
				strcpy(str, buf);
				str[ strlen(" MeshTextureCoords") ] = '\0';
				if( strcmp(str, " MeshTextureCoords") == 0 ){

					fgets(buf, 256, fp);
					if( atoi(buf) != in_vertexs ){ break; }

					for(int i=0; i<in_vertexs; i++){
						fgets(buf, 256, fp);
						vertex[i+vertexs].u = (float)atof( strtok(buf, stroks) );
						vertex[i+vertexs].v = (float)atof( strtok(NULL, stroks) );
					}

					break;
				}
			}

			vertexs += in_vertexs;
			polygons += in_polygons;
		}
	}

	//Free File Handle
	fclose( fp );

	float *VertexAry = new float [polygons*6*3];
	float *ColorAry = new float [polygons*6*4];
	float *ColorGrayAry = new float [polygons*6*4];
	float *TexCoordAry = new float [polygons*6*2];
	int vid;
	int cnt = 0;

	for(int i=0; i<polygons; i++){
		if( index[i*5] == 3 ){
			//Triangle
			vid = index[i*5+1];
			VertexAry[0 + cnt*3] = vertex[vid].x;
			VertexAry[1 + cnt*3] = vertex[vid].y;
			VertexAry[2 + cnt*3] = vertex[vid].z;
			TexCoordAry[0 + cnt*2] = vertex[vid].u;
			TexCoordAry[1 + cnt*2] = vertex[vid].v;

			VertexAry[3 + cnt*3] = vertex[vid].x;
			VertexAry[4 + cnt*3] = vertex[vid].y;
			VertexAry[5 + cnt*3] = vertex[vid].z;
			TexCoordAry[2 + cnt*2] = vertex[vid].u;
			TexCoordAry[3 + cnt*2] = vertex[vid].v;

			vid = index[i*5+3];
			VertexAry[6 + cnt*3] = vertex[vid].x;
			VertexAry[7 + cnt*3] = vertex[vid].y;
			VertexAry[8 + cnt*3] = vertex[vid].z;
			TexCoordAry[4 + cnt*2] = vertex[vid].u;
			TexCoordAry[5 + cnt*2] = vertex[vid].v;

			vid = index[i*5+2];
			VertexAry[9 + cnt*3] = vertex[vid].x;
			VertexAry[10 + cnt*3] = vertex[vid].y;
			VertexAry[11 + cnt*3] = vertex[vid].z;
			TexCoordAry[6 + cnt*2] = vertex[vid].u;
			TexCoordAry[7 + cnt*2] = vertex[vid].v;

			VertexAry[12 + cnt*3] = vertex[vid].x;
			VertexAry[13 + cnt*3] = vertex[vid].y;
			VertexAry[14 + cnt*3] = vertex[vid].z;
			TexCoordAry[8 + cnt*2] = vertex[vid].u;
			TexCoordAry[9 + cnt*2] = vertex[vid].v;

			VertexAry[15 + cnt*3] = vertex[vid].x;
			VertexAry[16 + cnt*3] = vertex[vid].y;
			VertexAry[17 + cnt*3] = vertex[vid].z;
			TexCoordAry[10 + cnt*2] = vertex[vid].u;
			TexCoordAry[11 + cnt*2] = vertex[vid].v;

			cnt += 6;
		}
		else{
			//Square type
			vid = index[i*5+1];
			VertexAry[0 + cnt*3] = vertex[vid].x;
			VertexAry[1 + cnt*3] = vertex[vid].y;
			VertexAry[2 + cnt*3] = vertex[vid].z;
			TexCoordAry[0 + cnt*2] = vertex[vid].u;
			TexCoordAry[1 + cnt*2] = vertex[vid].v;

			VertexAry[3 + cnt*3] = vertex[vid].x;
			VertexAry[4 + cnt*3] = vertex[vid].y;
			VertexAry[5 + cnt*3] = vertex[vid].z;
			TexCoordAry[2 + cnt*2] = vertex[vid].u;
			TexCoordAry[3 + cnt*2] = vertex[vid].v;

			vid = index[i*5+4];
			VertexAry[6 + cnt*3] = vertex[vid].x;
			VertexAry[7 + cnt*3] = vertex[vid].y;
			VertexAry[8 + cnt*3] = vertex[vid].z;
			TexCoordAry[4 + cnt*2] = vertex[vid].u;
			TexCoordAry[5 + cnt*2] = vertex[vid].v;

			vid = index[i*5+2];
			VertexAry[9 + cnt*3] = vertex[vid].x;
			VertexAry[10 + cnt*3] = vertex[vid].y;
			VertexAry[11 + cnt*3] = vertex[vid].z;
			TexCoordAry[6 + cnt*2] = vertex[vid].u;
			TexCoordAry[7 + cnt*2] = vertex[vid].v;

			vid = index[i*5+3];
			VertexAry[12 + cnt*3] = vertex[vid].x;
			VertexAry[13 + cnt*3] = vertex[vid].y;
			VertexAry[14 + cnt*3] = vertex[vid].z;
			TexCoordAry[8 + cnt*2] = vertex[vid].u;
			TexCoordAry[9 + cnt*2] = vertex[vid].v;

			VertexAry[15 + cnt*3] = vertex[vid].x;
			VertexAry[16 + cnt*3] = vertex[vid].y;
			VertexAry[17 + cnt*3] = vertex[vid].z;
			TexCoordAry[10 + cnt*2] = vertex[vid].u;
			TexCoordAry[11 + cnt*2] = vertex[vid].v;

			cnt += 6;
		}
	}

	if( polygons > 0 ){
		//Color information array available
		ColorAry[0] = 1.0f;
		ColorAry[1] = 1.0f;
		ColorAry[2] = 1.0f;
		ColorAry[3] = 1.0f;
		for(int i=1; i<cnt; i++){
			memcpy(&(ColorAry[i*4]), ColorAry, sizeof(float)*4);
		}
		ColorGrayAry[0] = 0.8f;
		ColorGrayAry[1] = 0.8f;
		ColorGrayAry[2] = 0.8f;
		ColorGrayAry[3] = 1.0f;
		for(int i=1; i<cnt; i++){
			memcpy(&(ColorGrayAry[i*4]), ColorGrayAry, sizeof(float)*4);
		}
	}

	delete [] vertex;
	delete [] index;

	pmodel[id].useflag = true;
	pmodel[id].polygons = polygons;
	pmodel[id].VertexAry = VertexAry;
	pmodel[id].ColorAry = ColorAry;
	pmodel[id].ColorGrayAry = ColorGrayAry;
	pmodel[id].TexCoordAry = TexCoordAry;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", id);
#endif
	return id;
}

//! @brief Create intermediate data for model files (moofing)
//! @param idA Model A Data Number
//! @param idB Model B Data Number
//! @return Success: new model data ID (0 or greater); failure: -1.
//! @attention Model A and Model B must have the same vertices, polygons and indexes.
//! @attention If each model data is incorrect or the vertex is different, it will fail.
int D3DGraphics::MorphingModel(int idA, int idB)
{
#ifdef ENABLE_DEBUGLOG
	char str[128];
	sprintf(str, "Create morphing model  ID : %d and %d", idA, idB);

	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "Model", str);
#endif

	//Find out if the data is correct
	if( (idA < 0)||(MAX_MODEL <= idA) ){ return -1; }
	if( pmodel[idA].useflag == false ){ return -1; }
	if( (idB < 0)||(MAX_MODEL <= idB) ){ return -1; }
	if( pmodel[idB].useflag == false ){ return -1; }

	int idN = -1;
	int numpA, numpB;

	//Finding an empty data number
	for(int i=0; i<MAX_MODEL; i++){
		if( pmodel[i].useflag == false ){
			idN = i;
			break;
		}
	}
	if( idN == -1 ){ return -1; }

	//Get the number of polygon
	numpA = pmodel[idA].polygons;
	numpB = pmodel[idB].polygons;

	//Check if the number of polygons is the same
	if( numpA != numpB ){ return -1; }

	float *VertexAry = new float [numpA*6*3];
	float *ColorAry = new float [numpA*6*4];
	float *ColorGrayAry = new float [numpA*6*4];
	float *TexCoordAry = new float [numpA*6*2];

	//Read and calculate each vertex.
	for(int i=0; i<numpA*6; i++){
		VertexAry[i*3 + 0] = (pmodel[idA].VertexAry[i*3 + 0] + pmodel[idB].VertexAry[i*3 + 0])/2;
		VertexAry[i*3 + 1] = (pmodel[idA].VertexAry[i*3 + 1] + pmodel[idB].VertexAry[i*3 + 1])/2;
		VertexAry[i*3 + 2] = (pmodel[idA].VertexAry[i*3 + 2] + pmodel[idB].VertexAry[i*3 + 2])/2;
	}

	//Copy material information
	for(int i=0; i<numpA*6; i++){
		ColorAry[i*4 + 0] = pmodel[idA].ColorAry[i*4 + 0];
		ColorAry[i*4 + 1] = pmodel[idA].ColorAry[i*4 + 1];
		ColorAry[i*4 + 2] = pmodel[idA].ColorAry[i*4 + 2];
		ColorAry[i*4 + 3] = pmodel[idA].ColorAry[i*4 + 3];
		ColorGrayAry[i*4 + 0] = pmodel[idA].ColorGrayAry[i*4 + 0];
		ColorGrayAry[i*4 + 1] = pmodel[idA].ColorGrayAry[i*4 + 1];
		ColorGrayAry[i*4 + 2] = pmodel[idA].ColorGrayAry[i*4 + 2];
		ColorGrayAry[i*4 + 3] = pmodel[idA].ColorGrayAry[i*4 + 3];
		TexCoordAry[i*2 + 0] = pmodel[idA].TexCoordAry[i*2 + 0];
		TexCoordAry[i*2 + 1] = pmodel[idA].TexCoordAry[i*2 + 1];
	}

	pmodel[idN].useflag = true;
	pmodel[idN].polygons = numpA;
	pmodel[idN].VertexAry = VertexAry;
	pmodel[idN].ColorAry = ColorAry;
	pmodel[idN].ColorGrayAry = ColorGrayAry;
	pmodel[idN].TexCoordAry = TexCoordAry;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", idN);
#endif
	return idN;
}

//! @brief Release model files
//! @param id Model Data Number
void D3DGraphics::CleanupModel(int id)
{
	if( (id < 0)||(MAX_MODEL <= id) ){ return; }
	if( pmodel[id].useflag == false ){ return; }

	delete pmodel[id].VertexAry;
	delete pmodel[id].ColorAry;
	delete pmodel[id].ColorGrayAry;
	delete pmodel[id].TexCoordAry;
	pmodel[id].useflag = false;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Model", id);
#endif
}

//! @brief Get the number of loaded models
//! @return Number of models.
int D3DGraphics::GetTotalModels()
{
	int cnt = 0;

	//Counting elements in use
	for(int i=0; i<MAX_MODEL; i++){
		if( pmodel[i].useflag == true ){ cnt += 1; }
	}

	return cnt;
}

//! @brief Texture format is determined by the extension
//! @param filename File Name
//! @param nowformat Current detected format.
//! @return Newly detected format.
int D3DGraphics::CheckTextureFileExtension(const char* filename, int nowformat)
{
	char filename2[MAX_PATH];

	//Convert file name to lower case (for extension deter )
	for(int i=0; i<(int)strlen(filename); i++){
		filename2[i] = (char)tolower(filename[i]);
	}
	filename2[ strlen(filename) ] = '\0';

	//Determine file format by extension
	for(int i=(int)strlen(filename2)-1; i>0; i--){
		if( filename2[i] == '.' ){
			if( strcmp(&(filename2[i]), ".bmp") == 0 ){
				return 1;
			}
			if( strcmp(&(filename2[i]), ".dds") == 0 ){
				return 2;
			}
			if( strcmp(&(filename2[i]), ".jpeg") == 0 ){
				return 3;
			}
			if( strcmp(&(filename2[i]), ".jpg") == 0 ){
				return 3;
			}
			if( strcmp(&(filename2[i]), ".jpe") == 0 ){
				return 3;
			}
			if( strcmp(&(filename2[i]), ".png") == 0 ){
				return 4;
			}
		}
	}

	return nowformat;
}

//! @brief Texture format is determined by file header
//! @param filename File Name
//! @param nowformat Current detected format.
//! @return Newly detected format.
int D3DGraphics::CheckTextureFileTypeFlag(const char* filename, int nowformat)
{
	FILE *fp;
	unsigned char header[4];

	//Load files
	fp = fopen(filename, "rb");
	if( fp == NULL ){ return false; }

	//Read Header
	fread(header, 1, 4, fp);

	//Free File Handle
	fclose( fp );

	if( (header[0x00] == 'B')&&(header[0x01] == 'M') ){															//.bmp
		return 1;
	}
	if( (header[0x00] == 'D')&&(header[0x01] == 'D')&&(header[0x02] == 'S') ){									//.dds
		return 2;
	}
	if( (header[0x00] == 0xFF)&&(header[0x01] == 0xD8) ){														//.jpg
		return 3;
	}
	if( (header[0x00] == 0x89)&&(header[0x01] == 'P')&&(header[0x02] == 'N')&&(header[0x03] == 'G') ){			//.png (the actual signature is eight bytes).
		return 4;
	}

	return nowformat;
}

//! @brief Read Texture
//! @param filename File Name
//! @param texturefont Texture Font Flag
//! @param BlackTransparent Make black pixels transparent.
//! @return Success: texture data ID (0 or greater); failure: -1.
int D3DGraphics::LoadTexture(const char* filename, bool texturefont, bool BlackTransparent)
{
	//Handle unused arguments.
	UNREFERENCED_PARAMETER(texturefont);

	if( filename == NULL ){ return -1; }

	int id = -1;
	int format = 0;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "Texture", filename);
#endif

	//Finding an empty data number
	for(int i=0; i<MAX_TEXTURE; i++){
		if( ptextures[i].useflag == false ){
			id = i;
			break;
		}
	}
	if( id == -1 ){ return -1; }

	//First, determine the file format with the extension
	format = CheckTextureFileExtension(filename, format);

	//Also check the file header.
	format = CheckTextureFileTypeFlag(filename, format);

	//Unsupported formats
	if( format == 0 ){ return -1; }

	if( format == 1 ){	// .bmp
		if( LoadBMPTexture(filename, BlackTransparent, &(ptextures[id])) == false ){
			return -1;
		}
	}
	if( format == 2 ){	// .dds
		if( LoadDDSTexture(filename, BlackTransparent, &(ptextures[id])) == false ){
			return -1;
		}
	}
	if( format == 3 ){	// .jpeg
		if( LoadJPEGTexture(filename, BlackTransparent, &(ptextures[id])) == false ){
			return -1;
		}
	}
	if( format == 4 ){	// .png
		if( LoadPNGTexture(filename, BlackTransparent, &(ptextures[id])) == false ){
			return -1;
		}
	}


	//Texture enabled
	glEnable(GL_TEXTURE_2D);

	HDC hDC;
	hDC = GetDC(hWnd);
	wglMakeCurrent(hDC, hGLRC);
	glGenTextures(1, &(textureobjname[id]));
	ReleaseDC(hWnd, hDC);

	glBindTexture(GL_TEXTURE_2D, textureobjname[id]);

	//OpenGL
	int width = ptextures[id].width;
	int height = ptextures[id].height;
	unsigned char *data = ptextures[id].data;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	//MIP Map Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Multiplication
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//Texture Invalid
	glDisable(GL_TEXTURE_2D);

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	//OutputLog.WriteLog(LOG_COMPLETE, "", id);

	char str[32];
	sprintf(str, "IDÅF%d (size:%dx%d)", id, width, height);
	OutputLog.WriteLog(LOG_COMPLETE, "", str);
#endif

	return id;


	/*
	unsigned char *data = new unsigned char [16*4];

	data[0*4 + 0] = 255;	data[0*4 + 1] = 255;	data[0*4 + 2] = 255;	data[0*4 + 3] = 255;
	data[1*4 + 0] = 0;		data[1*4 + 1] = 0;		data[1*4 + 2] = 0;		data[1*4 + 3] = 255;
	data[2*4 + 0] = 255;	data[2*4 + 1] = 255;	data[2*4 + 2] = 255;	data[2*4 + 3] = 255;
	data[3*4 + 0] = 0;		data[3*4 + 1] = 0;		data[3*4 + 2] = 0;		data[3*4 + 3] = 255;
	data[4*4 + 0] = 255;	data[4*4 + 1] = 0;		data[4*4 + 2] = 0;		data[4*4 + 3] = 255;
	data[5*4 + 0] = 0;		data[5*4 + 1] = 255;	data[5*4 + 2] = 0;		data[5*4 + 3] = 255;
	data[6*4 + 0] = 0;		data[6*4 + 1] = 0;		data[6*4 + 2] = 255;	data[6*4 + 3] = 255;
	data[7*4 + 0] = 0;		data[7*4 + 1] = 0;		data[7*4 + 2] = 0;		data[7*4 + 3] = 255;
	data[8*4 + 0] = 128;	data[8*4 + 1] = 0;		data[8*4 + 2] = 0;		data[8*4 + 3] = 255;
	data[9*4 + 0] = 0;		data[9*4 + 1] = 128;	data[9*4 + 2] = 0;		data[9*4 + 3] = 255;
	data[10*4 + 0] = 0;		data[10*4 + 1] = 0;		data[10*4 + 2] = 128;	data[10*4 + 3] = 255;
	data[11*4 + 0] = 0;		data[11*4 + 1] = 0;		data[11*4 + 2] = 0;		data[11*4 + 3] = 255;
	data[12*4 + 0] = 255;	data[12*4 + 1] = 255;	data[12*4 + 2] = 0;		data[12*4 + 3] = 255;
	data[13*4 + 0] = 255;	data[13*4 + 1] = 0;		data[13*4 + 2] = 255;	data[13*4 + 3] = 255;
	data[14*4 + 0] = 0;		data[14*4 + 1] = 255;	data[14*4 + 2] = 255;	data[14*4 + 3] = 255;
	data[15*4 + 0] = 255;	data[15*4 + 1] = 255;	data[15*4 + 2] = 255;	data[15*4 + 3] = 255;

	ptextures[id].data = data;
	ptextures[id].width = 4;
	ptextures[id].height = 4;

	ptextures[id].useflag = true;

	return id;
	*/
}

//! @brief Load BMP files
//! @param filename File Name
//! @param BlackTransparent Make black pixels transparent.
//! @param ptexture TEXTUREDATA struct pointer to receive
//! @return success: true failure: false
//! @attention Normally called from LoadTexture().
bool D3DGraphics::LoadBMPTexture(const char* filename, bool BlackTransparent, TEXTUREDATA *ptexture)
{
	FILE *fp;
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int index;

	//Load files
	fp = fopen(filename, "rb");
	if( fp == NULL ){ return false; }

	//Read Header
	fread(header, 1, 54, fp);

	if( (header[0x00] != 'B')||(header[0x01] != 'M') ){
		fclose(fp);
		return false;		//not .bmp
	}

	// Load integer from byte array
	dataPos = *(int*)&(header[0x0E]) + 14;
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	index = *(int*)&(header[0x1C]);

	//Move to the top of real data
	fseek(fp, dataPos, SEEK_SET);

	unsigned char *data = new unsigned char [width*height*4];

	//16-color palette mode for each pixel 4-bit
	if( index == 4 ){
		unsigned char pixel;
		unsigned char pixelH;
		unsigned char pixelL;
		unsigned char *pallet = new unsigned char [16*4];
		fread(pallet, 1, 16*4, fp);

		for(int h=height-1; h>=0; h--){
			for(int w=0; w<((int)width/2); w++){
				fread(&pixel, 1, 1, fp);
				pixelH = (pixel >> 4)&0x0F;
				pixelL = pixel&0x0F;

				data[(h*width+w*2)*4 + 0] = pallet[pixelH*4 + 2];
				data[(h*width+w*2)*4 + 1] = pallet[pixelH*4 + 1];
				data[(h*width+w*2)*4 + 2] = pallet[pixelH*4 + 0];
				data[(h*width+w*2)*4 + 3] = 255;

				data[(h*width+w*2+1)*4 + 0] = pallet[pixelL*4 + 2];
				data[(h*width+w*2+1)*4 + 1] = pallet[pixelL*4 + 1];
				data[(h*width+w*2+1)*4 + 2] = pallet[pixelL*4 + 0];
				data[(h*width+w*2+1)*4 + 3] = 255;

				if( BlackTransparent == true ){
					//Transmit if black
					if( (data[(h*width+w*2)*4 + 0] == 0)&&(data[(h*width+w*2)*4 + 1] == 0)&&(data[(h*width+w*2)*4 + 2] == 0) ){
						data[(h*width+w*2)*4 + 3] = 0;
					}
					if( (data[(h*width+w*2+1)*4 + 0] == 0)&&(data[(h*width+w*2+1)*4 + 1] == 0)&&(data[(h*width+w*2+1)*4 + 2] == 0) ){
						data[(h*width+w*2+1)*4 + 3] = 0;
					}
				}
			}

			if( (((int)width/2)%4) != 0 ){
				//Padding to be multiple of 4
				unsigned char dummy[3];
				fread(&dummy, 1, 4 - (((int)width/2)%4), fp);
			}
		}

		delete []pallet;
	}

	//256-color palette mode for each pixel 8-bit
	if( index == 8 ){
		unsigned char pixel;
		unsigned char *pallet = new unsigned char [256*4];
		fread(pallet, 1, 256*4, fp);

		for(int h=height-1; h>=0; h--){
			for(int w=0; w<(int)width; w++){
				fread(&pixel, 1, 1, fp);

				data[(h*width+w)*4 + 0] = pallet[pixel*4 + 2];
				data[(h*width+w)*4 + 1] = pallet[pixel*4 + 1];
				data[(h*width+w)*4 + 2] = pallet[pixel*4 + 0];
				data[(h*width+w)*4 + 3] = 255;

				if( BlackTransparent == true ){
					//Transmit if black
					if( (data[(h*width+w)*4 + 0] == 0)&&(data[(h*width+w)*4 + 1] == 0)&&(data[(h*width+w)*4 + 2] == 0) ){
						data[(h*width+w)*4 + 3] = 0;
					}
				}
			}

			if( (width%4) != 0 ){
				//Padding to be multiple of 4
				unsigned char dummy[3];
				fread(&dummy, 1, 4 - (width%4), fp);
			}
		}

		delete []pallet;
	}

	//24 bits of each pixel
	if( index == 24 ){
		unsigned char pixel[3];
		for(int h=height-1; h>=0; h--){
			for(int w=0; w<(int)width; w++){
				fread(&pixel, 1, 3, fp);

				data[(h*width+w)*4 + 0] = pixel[2];
				data[(h*width+w)*4 + 1] = pixel[1];
				data[(h*width+w)*4 + 2] = pixel[0];
				data[(h*width+w)*4 + 3] = 255;

				if( BlackTransparent == true ){
					//Transmit if black
					if( (data[(h*width+w)*4 + 0] == 0)&&(data[(h*width+w)*4 + 1] == 0)&&(data[(h*width+w)*4 + 2] == 0) ){
						data[(h*width+w)*4 + 3] = 0;
					}
				}
			}

			if( (width%4) != 0 ){
				//Padding to be multiple of 4
				unsigned char dummy[3];
				fread(&dummy, 1, 4 - (width%4), fp);
			}
		}
	}

	//Full color with 32-bit each pixel
	if( index == 32 ){
		unsigned char pixel[4];
		for(int h=height-1; h>=0; h--){
			for(int w=0; w<(int)width; w++){
				fread(&pixel, 1, 4, fp);

				data[(h*width+w)*4 + 0] = pixel[2];
				data[(h*width+w)*4 + 1] = pixel[1];
				data[(h*width+w)*4 + 2] = pixel[0];
				data[(h*width+w)*4 + 3] = 255;

				if( BlackTransparent == true ){
					//Transmit if black
					if( (data[(h*width+w)*4 + 0] == 0)&&(data[(h*width+w)*4 + 1] == 0)&&(data[(h*width+w)*4 + 2] == 0) ){
						data[(h*width+w)*4 + 3] = 0;
					}
				}
			}

			//Note: If 1 pixel = 4 bytes, padding is unnecessary.
		}
	}

	//Free File Handle
	fclose( fp );

	//Substitute the structure
	ptexture->data = data;
	ptexture->width = width;
	ptexture->height = height;

	ptexture->useflag = true;

	return true;
}

//! @brief Load a DDS file.
//! @param filename File Name
//! @param BlackTransparent Make black pixels transparent.
//! @param ptexture TEXTUREDATA struct pointer to receive
//! @return success: true failure: false
//! @attention Normally called from LoadTexture().
bool D3DGraphics::LoadDDSTexture(const char* filename, bool BlackTransparent, TEXTUREDATA *ptexture)
{
	FILE *fp;
	unsigned char header[124+4];
	unsigned int width, height;
	unsigned int index;
	unsigned int flag;
	unsigned int Caps;

	//Load files
	fp = fopen(filename, "rb");
	if( fp == NULL ){ return false; }

	//Read Header
	fread(header, 1, 124+4, fp);

	if( (header[0x00] != 'D')||(header[0x01] != 'D')||(header[0x02] != 'S')||(header[0x03] != ' ') ){
		fclose(fp);
		return false;		//not .dds
	}

	// Load integer from byte array
	width = *(int*)&(header[0x10]);
	height = *(int*)&(header[0x0C]);
	index = *(int*)&(header[0x58]);
	flag = *(int*)&(header[0x08]);
	Caps = *(int*)&(header[0x70]);

	if( (index != 16)&&(index != 32) ){
		fclose(fp);
		return false;		//Unsupported formats
	}

	if( (flag & 0x00020000)&&(Caps & 0x00400000) ){		//DDSD_MIPMAPCOUNT and DDSCAPS_MIPMAP.
		//Mipp map information reading (Dummy)
		fread(header, 1, 128, fp);
	}

	unsigned char *data = new unsigned char [width*height*4];

	for(int h=0; h<(int)height; h++){
		for(int w=0; w<(int)width; w++){
			unsigned char pixel[4];
			fread(&pixel, 1, index/8, fp);

			if( index == 16 ){		//16 bits per pixel
				data[(h*width+w)*4 + 0] = (pixel[1]<<4)&0xF0;
				data[(h*width+w)*4 + 1] = pixel[0]&0xF0;
				data[(h*width+w)*4 + 2] = (pixel[0]<<4)&0xF0;
				data[(h*width+w)*4 + 3] = pixel[1]&0xF0;
			}
			if( index == 32 ){		//32-bit each pixel
				data[(h*width+w)*4 + 0] = pixel[2];
				data[(h*width+w)*4 + 1] = pixel[1];
				data[(h*width+w)*4 + 2] = pixel[0];
				data[(h*width+w)*4 + 3] = pixel[3];
			}

			if( BlackTransparent == true ){
				//Transmit if black
				if( (data[(h*width+w)*4 + 0] == 0)&&(data[(h*width+w)*4 + 1] == 0)&&(data[(h*width+w)*4 + 2] == 0) ){
					data[(h*width+w)*4 + 3] = 0;
				}
			}
		}
	}

	//Free File Handle
	fclose( fp );

	//Substitute the structure
	ptexture->data = data;
	ptexture->width = width;
	ptexture->height = height;

	ptexture->useflag = true;

	return true;
}

//! @brief Load a JPEG file.
//! @param filename File Name
//! @param BlackTransparent Make black pixels transparent.
//! @param ptexture TEXTUREDATA struct pointer to receive
//! @return success: true failure: false
//! @attention Normally called from LoadTexture().
bool D3DGraphics::LoadJPEGTexture(const char* filename, bool BlackTransparent, TEXTUREDATA *ptexture)
{
#ifdef ENABLE_OPENGL_LIBJPEGPNG
	FILE *fp;
	JSAMPARRAY img;
	unsigned int width, height;

	//Load files
	fp = fopen(filename, "rb");
	if( fp == NULL ){ return false; }
	jpeg_stdio_src(&cinfo, fp);

	//Parameter setting
	jpeg_read_header(&cinfo, true);

	//Start decompression.
	jpeg_start_decompress(&cinfo);

	//Allocate memory
	img = (JSAMPARRAY)new JSAMPROW [cinfo.output_height];
	for(int i=0; i<(int)cinfo.output_height; i++){
		img[i] = (JSAMPROW)new JSAMPLE [cinfo.output_width * cinfo.out_color_components];
	}

	//Decompress.
	while( cinfo.output_scanline < cinfo.output_height ) {
		jpeg_read_scanlines(&cinfo, img + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
	}

	//Finish decompression.
	jpeg_finish_decompress(&cinfo);


	//Free File Handle
	fclose( fp );


	// Load integer from byte array
	width = (int)cinfo.output_width;
	height = (int)cinfo.output_height;

	unsigned char *data = new unsigned char [width*height*4];

	for(int h=0; h<(int)height; h++){
		//1 line acquisition
		JSAMPROW p = img[h];

		for(int w=0; w<(int)width; w++){
			data[(h*width+w)*4 + 0] = p[w*3 + 0];
			data[(h*width+w)*4 + 1] = p[w*3 + 1];
			data[(h*width+w)*4 + 2] = p[w*3 + 2];
			data[(h*width+w)*4 + 3] = 255;

			if( BlackTransparent == true ){
				//Transmit if black
				if( (data[(h*width+w)*4 + 0] == 0)&&(data[(h*width+w)*4 + 1] == 0)&&(data[(h*width+w)*4 + 2] == 0) ){
					data[(h*width+w)*4 + 3] = 0;
				}
			}

		}
	}

	//Free memory
	for(int i=0; i<(int)cinfo.output_height; i++){
		delete [] img[i];
	}
	delete [] img;

	//Substitute the structure
	ptexture->data = data;
	ptexture->width = width;
	ptexture->height = height;

	ptexture->useflag = true;

	return true;
#else
	return false;
#endif
}

//! @brief Load PNG files
//! @param filename File Name
//! @param BlackTransparent Make black pixels transparent.
//! @param ptexture TEXTUREDATA struct pointer to receive
//! @return success: true failure: false
//! @attention Normally called from LoadTexture().
bool D3DGraphics::LoadPNGTexture(const char* filename, bool BlackTransparent, TEXTUREDATA *ptexture)
{
#ifdef ENABLE_OPENGL_LIBJPEGPNG
	FILE *fp;
	png_byte sig[4];
	png_structp pPng;
    png_infop pInfo;
	unsigned int width, height;
	bool pallet;

	//Load files
	fp = fopen(filename, "rb");
	if( fp == NULL ){ return false; }

	//PNG file or judgment
	fread(sig, 4, 1, fp);
	if( png_sig_cmp(sig, 0, 4) != 0 ){
		fclose(fp);
		return false;	//not .png
	}

	//Initializing structures
	pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    pInfo = png_create_info_struct(pPng);

	//Get information
	png_init_io(pPng, fp);
	png_set_sig_bytes(pPng, 4);
	png_read_info(pPng, pInfo);

	//Get texture size
	width = png_get_image_width(pPng, pInfo);
	height =  png_get_image_height(pPng, pInfo);

	//Interless method or judgment
	if( png_get_interlace_type(pPng, pInfo) != PNG_INTERLACE_NONE ){
		png_destroy_read_struct(&pPng, &pInfo, (png_infopp)NULL);
		fclose(fp);
		return false;	//Not compatible with interless.
	}

	//Bit depth detection
	if( png_get_bit_depth(pPng, pInfo) != 8 ){
		png_destroy_read_struct(&pPng, &pInfo, (png_infopp)NULL);
		fclose(fp);
		return false;	//It does not correspond to other than 8-bit depth.
	}

	//Color type judgment
	if( (png_get_color_type(pPng, pInfo) == PNG_COLOR_TYPE_GRAY)||(png_get_color_type(pPng, pInfo) == PNG_COLOR_TYPE_GRAY_ALPHA) ){
		png_destroy_read_struct(&pPng, &pInfo, (png_infopp)NULL);
		fclose(fp);
		return false;	//ed because it seems to be not compatible with gray scale. (Not verified)
	}

	//Color type acquisition
	if( png_get_color_type(pPng, pInfo) == PNG_COLOR_TYPE_PALETTE ){
		pallet = true;
	}
	else{
		pallet = false;
	}

	unsigned char *data = new unsigned char [width*height*4];

	//Initialize alpha channel
	png_set_add_alpha(pPng, 0xff, PNG_FILLER_AFTER);

	if( pallet == false ){
		// Convert a tRNS chunk to an alpha channel if present.
		if( png_get_valid(pPng, pInfo, PNG_INFO_tRNS) ){
			png_set_tRNS_to_alpha(pPng);
		}

		//Ensuring work area for one line
		png_bytep buf = new png_byte[width*4];

		for(int h=0; h<(int)height; h++){
			//1 line acquisition
			png_read_row(pPng, buf, NULL);

			for(int w=0; w<(int)width; w++){
				data[(h*width+w)*4 + 0] = buf[w*4 + 0];
				data[(h*width+w)*4 + 1] = buf[w*4 + 1];
				data[(h*width+w)*4 + 2] = buf[w*4 + 2];
				data[(h*width+w)*4 + 3] = buf[w*4 + 3];

				if( BlackTransparent == true ){
					//Transmit if black
					if( (data[(h*width+w)*4 + 0] == 0)&&(data[(h*width+w)*4 + 1] == 0)&&(data[(h*width+w)*4 + 2] == 0) ){
						data[(h*width+w)*4 + 3] = 0;
					}
				}
			}
		}

		//Release work area for one line
		delete [] buf;
	}
	else{
		png_colorp palette;
		int num;

		//Get the palette.
		png_get_PLTE(pPng, pInfo, &palette, &num);

		//Ensuring work area for one line
		png_bytep buf = new png_byte[width];

		for(int h=0; h<(int)height; h++){
			//1 line acquisition
			png_read_row(pPng, buf, NULL);

			for(int w=0; w<(int)width; w++){
				data[(h*width+w)*4 + 0] = palette[ buf[w] ].red;
				data[(h*width+w)*4 + 1] = palette[ buf[w] ].green;
				data[(h*width+w)*4 + 2] = palette[ buf[w] ].blue;
				data[(h*width+w)*4 + 3] = 255;

				if( BlackTransparent == true ){
					//Transmit if black
					if( (data[(h*width+w)*4 + 0] == 0)&&(data[(h*width+w)*4 + 1] == 0)&&(data[(h*width+w)*4 + 2] == 0) ){
						data[(h*width+w)*4 + 3] = 0;
					}
				}
			}
		}

		//Release work area for one line
		delete [] buf;
	}

	//Release resources.
	png_read_end(pPng, NULL);
	png_destroy_read_struct(&pPng, &pInfo, (png_infopp)NULL);

	//Free File Handle
	fclose( fp );

	//Substitute the structure
	ptexture->data = data;
	ptexture->width = width;
	ptexture->height = height;

	ptexture->useflag = true;

	return true;
#else
	return false;
#endif
}

#ifdef ENABLE_DEBUGCONSOLE
//! @brief Load the debug font.
//! @return success: true failure: false
//! @attention Call InitDebugFontData() before calling this function.
bool D3DGraphics::LoadDebugFontTexture()
{
	int charwidth = 8;
	int charheight = 16;
	int width = charwidth * 16;
	int height = charheight * 8;

	int datacnt = 0;
	int id = -1;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "Texture", "DebugFontTexture");
#endif

	//Failed if already loaded
	if( TextureDebugFont != -1 ){ return false; }

	//Finding an empty data number
	for(int i=0; i<MAX_TEXTURE; i++){
		if( ptextures[i].useflag == false ){
			id = i;
			break;
		}
	}
	if( id == -1 ){ return false; }

	unsigned char *data = new unsigned char [width*height*4];

	//2 lines with control code 32 characters are empty.
	for(int cnt_y=0; cnt_y<2; cnt_y++){
		for(int line_y=0; line_y<charheight; line_y++){
			for(int cnt_x=0; cnt_x<16; cnt_x++){
				for(int line_x=0; line_x<charwidth; line_x++){
					data[datacnt + 0] = 0;
					data[datacnt + 1] = 0;
					data[datacnt + 2] = 0;
					data[datacnt + 3] = 0;

					datacnt += 4;
				}
			}
		}
	}

	//Create data for 6 lines
	for(int cnt_y=0; cnt_y<6; cnt_y++){
		for(int line_y=0; line_y<charheight; line_y++){
			for(int cnt_x=0; cnt_x<16; cnt_x++){
				for(int line_x=(charwidth-1); line_x>=0; line_x--){
					unsigned char mask;

					//Make a mask for bit judgment
					switch(line_x){
						case 0: mask = 0x01; break;
						case 1: mask = 0x02; break;
						case 2: mask = 0x04; break;
						case 3: mask = 0x08; break;
						case 4: mask = 0x10; break;
						case 5: mask = 0x20; break;
						case 6: mask = 0x40; break;
						case 7: mask = 0x80; break;
						default: mask = 0x00;		//Effectively an error.
					}

					//If the corresponding bit is 1, white, black if 0.
					if( (DebugFontData[cnt_y*16 + cnt_x][line_y] & mask) != 0 ){
						data[datacnt + 0] = 255;
						data[datacnt + 1] = 255;
						data[datacnt + 2] = 255;
						data[datacnt + 3] = 255;
					}
					else{
						data[datacnt + 0] = 0;
						data[datacnt + 1] = 0;
						data[datacnt + 2] = 0;
						data[datacnt + 3] = 0;
					}

					datacnt += 4;
				}
			}
		}
	}

	//Texture enabled
	glEnable(GL_TEXTURE_2D);

	HDC hDC;
	hDC = GetDC(hWnd);
	wglMakeCurrent(hDC, hGLRC);
	glGenTextures(1, &(textureobjname[id]));
	ReleaseDC(hWnd, hDC);

	glBindTexture(GL_TEXTURE_2D, textureobjname[id]);

	//OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	//MIP Map Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Multiplication
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//Texture Invalid
	glDisable(GL_TEXTURE_2D);


	//Substitute the structure
	ptextures[id].data = data;
	ptextures[id].width = width;
	ptextures[id].height = height;

	ptextures[id].useflag = true;


#ifdef ENABLE_DEBUGLOG
	//Write to the log
	//OutputLog.WriteLog(LOG_COMPLETE, "", id);

	char str[32];
	sprintf(str, "IDÅF%d (size:%dx%d)", id, width, height);
	OutputLog.WriteLog(LOG_COMPLETE, "", str);
#endif

	//Texture ID setting
	TextureDebugFont = id;
	return true;
}
#endif

//! @brief Get texture size
//! @param id Texture Data Number
//! @param width Pointer to receive width
//! @param height Pointer receiving the height.
//! @return Success:0 failure:1
//! @attention Get the surface size loaded into the GPU, which may differ from the original texture size.
int D3DGraphics::GetTextureSize(int id, int *width, int *height)
{
	//If the data number is specified, it will not be processed.
	if( id == -1 ){ return 1; }
	if( ptextures[id].useflag == false ){ return 1; }
	if( width == NULL ){ return 1; }
	if( height == NULL ){ return 1; }

	*width = ptextures[id].width;
	*height = ptextures[id].height;

	return 0;
}

//! @brief Texture
//! @param TextureID Texture Data Number
void D3DGraphics::SetTexture(int TextureID)
{
	if( now_textureid == TextureID ){
		return;
	}

	//OpenGL
	glBindTexture(GL_TEXTURE_2D, textureobjname[TextureID]);

	now_textureid = TextureID;
}

//! @brief Free Texture
//! @param id Texture Data Number
void D3DGraphics::CleanupTexture(int id)
{
	if( (id < 0)||(MAX_TEXTURE <= id) ){ return; }
	if( ptextures[id].useflag == false ){ return; }

	delete ptextures[id].data;
	glDeleteTextures(1, &(textureobjname[id]));
	ptextures[id].useflag = false;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Texture", id);
#endif
}

//! @brief Get the number of read textures
//! @return Number of textures
int D3DGraphics::GetTotalTextures()
{
	int cnt = 0;

	//Counting elements in use
	for(int i=0; i<MAX_TEXTURE; i++){
		if( ptextures[i].useflag == true ){ cnt += 1; }
	}

	return cnt;
}

//! @brief Start drawing process
//! @return Success:0 failure:1
//! @attention You must first call the drawing process.
int D3DGraphics::StartRender()
{
	HDC hDC;

	hDC = BeginPaint(hWnd, &Paint_ps);

	//Context
	wglMakeCurrent(hDC, hGLRC);

    // Clear the entire client area, including the letterbox bars.
    glDisable(GL_SCISSOR_TEST);
    RECT client;
    GetClientRect(hWnd, &client);
    GameViewport viewport = FitGameViewport(client.right, client.bottom, width, height);
    glViewport(viewport.x, client.bottom - viewport.y - viewport.height, viewport.width, viewport.height);
	//Initialize
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//Mixing (transmitter enabled)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Alpha Test
	glAlphaFunc(GL_GREATER, 0.0f);
	glEnable(GL_ALPHA_TEST);

	//2D system font Z coordinate initialization
	SystemFont_posz = 1.0f;

	return 0;
}

//! @brief Ends all drawing processing
//! @return Success:false Fail:true
//! @attention It is necessary to call at the end of the drawing process.
bool D3DGraphics::EndRender()
{
	glFlush();
    SwapBuffers(Paint_ps.hdc);
	wglMakeCurrent(NULL, NULL);

	EndPaint(hWnd, &Paint_ps);

	return false;
}

//! @brief Reset Z buffer
void D3DGraphics::ResetZbuffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

//! @brief Revert the world space to its origin (0,0,0), etc.
void D3DGraphics::ResetWorldTransform()
{
	float camera_y_flag;

	if( fabsf(camera_ry) <= (float)M_PI/2 ){
		camera_y_flag = 1.0f;
	}
	else{
		camera_y_flag = -1.0f;
	}

	//When looking straight up or down, invert the camera up direction to match DirectX behavior.
	if( fabsf(camera_ry) == (float)M_PI/2 ){
		camera_y_flag *= -1;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(viewangle*(180.0f/(float)M_PI), (float)width/height, CLIPPINGPLANE_NEAR, CLIPPINGPLANE_FAR);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera_x*-1, camera_y, camera_z, camera_x*-1 + cosf(camera_rx*-1 + (float)M_PI)*cosf(camera_ry), camera_y + sinf(camera_ry), camera_z + sinf(camera_rx*-1 + (float)M_PI)*cosf(camera_ry), 0.0f, camera_y_flag, 0.0f);
}

//! @brief Set the coordinates, angles, and expansion rate of the world space
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param rx Horizontal angle.
//! @param ry Vertical angle.
//! @param size Expansion rate
void D3DGraphics::SetWorldTransform(float x, float y, float z, float rx, float ry, float size)
{
	SetWorldTransform(x, y, z, rx, ry, 0.0f, size);
}

//! @brief Set the coordinates, angles, and expansion rate of the world space
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param rx Horizontal angle.
//! @param ry1 Vertical angle.
//! @param ry2 Vertical angle.
//! @param size Expansion rate
void D3DGraphics::SetWorldTransform(float x, float y, float z, float rx, float ry1, float ry2, float size)
{
	ResetWorldTransform();

	glMatrixMode(GL_MODELVIEW);

	glTranslatef(x*-1, y, z);
	glRotatef(rx*-1*(180.0f/(float)M_PI), 0.0f, 1.0f, 0.0f);
	glRotatef(ry1*(180.0f/(float)M_PI), 1.0f, 0.0f, 0.0f);
	glRotatef(ry2*-1*(180.0f/(float)M_PI), 0.0f, 0.0f, 1.0f);
	glScalef(size, size, size);
}

//! @brief Set the coordinates, angles, and magnification rate of the world space (for effects)
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param rx Horizontal angle.
//! @param ry Vertical angle.
//! @param rt Rotation Angle
//! @param size Expansion rate
void D3DGraphics::SetWorldTransformEffect(float x, float y, float z, float rx, float ry, float rt, float size)
{
	ResetWorldTransform();

	glMatrixMode(GL_MODELVIEW);

	glTranslatef(x*-1, y, z);
	glRotatef(rx*-1*(180.0f/(float)M_PI), 0.0f, 1.0f, 0.0f);
	glRotatef(ry*-1*(180.0f/(float)M_PI), 0.0f, 0.0f, 1.0f);
	glRotatef(rt*(180.0f/(float)M_PI), 1.0f, 0.0f, 0.0f);
	glScalef(size, size, size);
}

//! @brief Set the world space to where people have weapons
//! @param x X Coordinate
//! @param y Y Coordinate
//! @param z Z Coordinates
//! @param mx X coordinates of the model based on the original
//! @param my Y coordinates of the model based on the original
//! @param mz Z coordinate of the model based on the origin of the hand
//! @param rx Horizontal angle.
//! @param ry Vertical angle.
//! @param size Expansion rate
void D3DGraphics::SetWorldTransformHumanWeapon(float x, float y, float z, float mx, float my, float mz, float rx, float ry, float size)
{
	ResetWorldTransform();

	glMatrixMode(GL_MODELVIEW);

	glTranslatef(x*-1, y, z);
	glRotatef(rx*-1*(180.0f/(float)M_PI), 0.0f, 1.0f, 0.0f);
	glRotatef(ry*(180.0f/(float)M_PI), 1.0f, 0.0f, 0.0f);
	glTranslatef(mx*-1, my, mz);
	glScalef(size, size, size);
}

//! @brief Set the world space to draw weapons
//! @param rotation Rotate weapons
//! @param camera_rx Camera horizontal axis angle
//! @param camera_ry Camera vertical axis angle
//! @param rx Weapon vertical axis angle
//! @param size Drawing size
//! @note rotation: true for the currently equipped weapon; false for the spare weapon (rx is ignored).
//! @todo Micro-adjustment of position and size
void D3DGraphics::SetWorldTransformPlayerWeapon(bool rotation, float camera_rx, float camera_ry, float rx, float size)
{
	float screenX, screenY, screenZ;
	double modelview[16];
	double projection[16];
	int viewport[4];
	double objX, objY, objZ;

	if( rotation == true ){
		screenX = HUD_myweapon_x[0];
		screenY = GameConfig.GetScreenHeight() - HUD_myweapon_y[0];
		screenZ = HUD_myweapon_z[0];
	}
	else{
		screenX = HUD_myweapon_x[1];
		screenY = GameConfig.GetScreenHeight() - HUD_myweapon_y[1];
		screenZ = HUD_myweapon_z[1];
	}

	ResetWorldTransform();

	glMatrixMode(GL_MODELVIEW);

	//Get the model-view matrix, perspective projection matrix, and viewport.
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

    // HUD positions are logical pixels; unprojection expects physical viewport pixels.
    screenX = viewport[0] + screenX * viewport[2] / width;
    screenY = viewport[1] + screenY * viewport[3] / height;
	//Determine object coordinates from screen coordinates
	gluUnProject(screenX, screenY, screenZ, modelview, projection, viewport, &objX, &objY, &objZ);

	//size = size * 0.3f;
	size = size * (0.0004f*GameConfig.GetScreenHeight()*GameConfig.GetScreenHeight() - 0.92f*GameConfig.GetScreenHeight() + 650.0f) / 1000.f;

	//Calculate the matrix.
	glTranslated(objX, objY, objZ);
	glRotatef(camera_rx*(180.0f/(float)M_PI), 0.0f, 1.0f, 0.0f);
	glRotatef(camera_ry*-1*(180.0f/(float)M_PI), 0.0f, 0.0f, 1.0f);
	if( rotation == true ){
		glRotatef(rx*-1*(180.0f/(float)M_PI), 0.0f, 1.0f, 0.0f);
	}
	else{
		glRotatef(180, 0.0f, 1.0f, 0.0f);
	}
	glScalef(size, size, size);
}

//! @brief Get coordinates in world space.
//! @param *x Pointer to receive x-axis
//! @param *y Pointer to receive y axis
//! @param *z Pointer to receive z-axis
void D3DGraphics::GetWorldTransformPos(float *x, float *y, float *z)
{
	if( (x == NULL)||(y == NULL)||(z == NULL) ){ return; }

	GLfloat model[16];
	glMatrixMode(GL_MODELVIEW);
	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	*x = model[12];
	*y = model[13];
	*z = model[14];
}

//! @brief Set the fog
//! @param enablefog Fog Activation
//! @param skynumber Sky ID.
void D3DGraphics::SetFog(bool enablefog, int skynumber)
{
	if( enablefog == true ){
		float fogColor[4];

		//Color is determined by the empty number.
		switch(skynumber){
			case 1:		fogColor[0] = 0.25f;	fogColor[1] = 0.25f+0.0625;	fogColor[2] = 0.25f;	fogColor[3] = 1.0f;		break;
			case 2:		fogColor[0] = 0.0625;	fogColor[1] = 0.0625;		fogColor[2] = 0.0625;	fogColor[3] = 1.0f;		break;
			case 3:		fogColor[0] = 0.0f;		fogColor[1] = 0.0625;		fogColor[2] = 0.125;	fogColor[3] = 1.0f;		break;
			case 4:		fogColor[0] = 0.125;	fogColor[1] = 0.0625;		fogColor[2] = 0.0625;	fogColor[3] = 1.0f;		break;
			case 5:		fogColor[0] = 0.25f;	fogColor[1] = 0.125;		fogColor[2] = 0.125;	fogColor[3] = 1.0f;		break;
			default:	fogColor[0] = 0.0f;		fogColor[1] = 0.0f;			fogColor[2] = 0.0f;		fogColor[3] = 1.0f;		break;
		}

		float fog_st = 100;
		float fog_end = 800;
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, fogColor);
		glHint(GL_FOG_HINT, GL_NICEST);
		glFogf(GL_FOG_START, fog_st);
		glFogf(GL_FOG_END, fog_end);

		glEnable(GL_FOG);
	}
	else{
		glDisable(GL_FOG);
	}
}

//! @brief Set the camera (point of view)
//! @param in_camera_x Camera X-coordinate
//! @param in_camera_y Camera Y coordinates
//! @param in_camera_z Camera Z coordinates
//! @param in_camera_rx Camera horizontal axis angle
//! @param in_camera_ry Vertical camera angle (between -PI and PI).
//! @param in_viewangle Viewing Angle
//! @todo If you turn right/right, the model drawing of your own weapon will be more likely.
void D3DGraphics::SetCamera(float in_camera_x, float in_camera_y, float in_camera_z, float in_camera_rx, float in_camera_ry, float in_viewangle)
{
    RECT client;
    GetClientRect(hWnd, &client);
    GameViewport viewport = FitGameViewport(client.right, client.bottom, width, height);
    glViewport(viewport.x, client.bottom - viewport.y - viewport.height, viewport.width, viewport.height);
	
	camera_x = in_camera_x;
	camera_y = in_camera_y;
	camera_z = in_camera_z;
	camera_rx = in_camera_rx;
	camera_ry = in_camera_ry;
	viewangle = in_viewangle;

	ResetWorldTransform();
}

//! @brief Include Smart Data
//! @param in_BlockDataIF Block Data
//! @param directory The directory where block data exists
void D3DGraphics::LoadMapdata(BlockDataInterface* in_BlockDataIF, const char *directory)
{
	//If the block data is not specified, it will not be processed.
	if( in_BlockDataIF == NULL ){ return; }
	if( directory == NULL ){ return; }

	char fname[MAX_PATH];
	char fnamefull[MAX_PATH];

	//Set Class
	BlockDataIF = in_BlockDataIF;

	//Get block count
	bs = BlockDataIF->GetTotaldatas();

	//Texture reading
	for(int i=0; i<TOTAL_BLOCKTEXTURE; i++){
		//Get Texture Name
		BlockDataIF->GetTexture(fname, i);

		if( strcmp(fname, "") == 0 ){	//If not specified, it will not be processed
			mapTextureID[i] = -1;
		}
		else{
			//rate and read "Directory + File Name"
			strcpy(fnamefull, directory);
			strcat(fnamefull, fname);
			mapTextureID[i] = LoadTexture(fnamefull, false, false);
		}
	}
}

//! @brief Print data
//! @param wireframe Wire Frame Display
void D3DGraphics::RenderMapdata(bool wireframe)
{
	//If the block data is not loaded, it will not be processed.
	if( BlockDataIF == NULL ){ return; }

	blockdata data;
	int textureID;
	int vID[4];
	int uvID[4];
	float *VertexAry = new float [bs*6 * 6*3];
	float *ColorAry = new float [bs*6 * 6*4];
	float *TexCoordAry = new float [bs*6 * 6*2];

	if( wireframe == true ){
		int linecolor = GetColorCode(0.0f,1.0f,0.0f,1.0f);

		//Wire Frame Display
		for(int i=0; i<bs; i++){
			BlockDataIF->Getdata(&data, i);
			Renderline(data.x[0], data.y[0], data.z[0], data.x[1], data.y[1], data.z[1], linecolor);
			Renderline(data.x[1], data.y[1], data.z[1], data.x[2], data.y[2], data.z[2], linecolor);
			Renderline(data.x[2], data.y[2], data.z[2], data.x[3], data.y[3], data.z[3], linecolor);
			Renderline(data.x[3], data.y[3], data.z[3], data.x[0], data.y[0], data.z[0], linecolor);
			Renderline(data.x[4], data.y[4], data.z[4], data.x[5], data.y[5], data.z[5], linecolor);
			Renderline(data.x[5], data.y[5], data.z[5], data.x[6], data.y[6], data.z[6], linecolor);
			Renderline(data.x[6], data.y[6], data.z[6], data.x[7], data.y[7], data.z[7], linecolor);
			Renderline(data.x[7], data.y[7], data.z[7], data.x[4], data.y[4], data.z[4], linecolor);
			Renderline(data.x[0], data.y[0], data.z[0], data.x[4], data.y[4], data.z[4], linecolor);
			Renderline(data.x[1], data.y[1], data.z[1], data.x[5], data.y[5], data.z[5], linecolor);
			Renderline(data.x[2], data.y[2], data.z[2], data.x[6], data.y[6], data.z[6], linecolor);
			Renderline(data.x[3], data.y[3], data.z[3], data.x[7], data.y[7], data.z[7], linecolor);
		}
		return;
	}

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	for(textureID=0; textureID<TOTAL_BLOCKTEXTURE; textureID++){
		int cnt = 0;

		//Set if the texture is not loaded correctly
		if( mapTextureID[textureID] == -1 ){
			//Texture Invalid
			glDisable(GL_TEXTURE_2D);
		}
		else if( ptextures[ mapTextureID[textureID] ].useflag == false ){
			//Texture Invalid
			glDisable(GL_TEXTURE_2D);
		}
		else{
			//Texture enabled
			glEnable(GL_TEXTURE_2D);

			//Set Texture
			SetTexture(mapTextureID[textureID]);
		}

		for(int i=0; i<bs; i++){
			//Data Acquisition
			BlockDataIF->Getdata(&data, i);

			for(int j=0; j<6; j++){
				//Get Texture Data Number
				int ID = data.material[j].textureID;

				if( textureID == ID ){
					//es the association of the vertex data on the surface
					blockdataface(j, &vID[0], &uvID[0]);

					//Prepare vertex array
					VertexAry[0 + cnt*18] = data.x[ vID[1] ]*-1;		VertexAry[1 + cnt*18] = data.y[ vID[1] ];	VertexAry[2 + cnt*18] = data.z[ vID[1] ];
					VertexAry[3 + cnt*18] = data.x[ vID[1] ]*-1;		VertexAry[4 + cnt*18] = data.y[ vID[1] ];	VertexAry[5 + cnt*18] = data.z[ vID[1] ];
					VertexAry[6 + cnt*18] = data.x[ vID[0] ]*-1;		VertexAry[7 + cnt*18] = data.y[ vID[0] ];	VertexAry[8 + cnt*18] = data.z[ vID[0] ];
					VertexAry[9 + cnt*18] = data.x[ vID[2] ]*-1;		VertexAry[10 + cnt*18] = data.y[ vID[2] ];	VertexAry[11 + cnt*18] = data.z[ vID[2] ];
					VertexAry[12 + cnt*18] = data.x[ vID[3] ]*-1;		VertexAry[13 + cnt*18] = data.y[ vID[3] ];	VertexAry[14 + cnt*18] = data.z[ vID[3] ];
					VertexAry[15 + cnt*18] = data.x[ vID[3] ]*-1;		VertexAry[16 + cnt*18] = data.y[ vID[3] ];	VertexAry[17 + cnt*18] = data.z[ vID[3] ];

					//Color information array available
					ColorAry[0 + cnt*24] = data.material[j].shadow;
					ColorAry[1 + cnt*24] = data.material[j].shadow;
					ColorAry[2 + cnt*24] = data.material[j].shadow;
					ColorAry[3 + cnt*24] = 1.0f;
					for(int k=1; k<6; k++){
						memcpy(&(ColorAry[k*4 + cnt*24]), &(ColorAry[cnt*24]), sizeof(float)*4);
					}

					//Prepare the UV-coordinate array.
					TexCoordAry[0 + cnt*12] = data.material[j].u[ uvID[1] ];	TexCoordAry[1 + cnt*12] = data.material[j].v[ uvID[1] ];
					TexCoordAry[2 + cnt*12] = data.material[j].u[ uvID[1] ];	TexCoordAry[3 + cnt*12] = data.material[j].v[ uvID[1] ];
					TexCoordAry[4 + cnt*12] = data.material[j].u[ uvID[0] ];	TexCoordAry[5 + cnt*12] = data.material[j].v[ uvID[0] ];
					TexCoordAry[6 + cnt*12] = data.material[j].u[ uvID[2] ];	TexCoordAry[7 + cnt*12] = data.material[j].v[ uvID[2] ];
					TexCoordAry[8 + cnt*12] = data.material[j].u[ uvID[3] ];	TexCoordAry[9 + cnt*12] = data.material[j].v[ uvID[3] ];
					TexCoordAry[10 + cnt*12] = data.material[j].u[ uvID[3] ];	TexCoordAry[11 + cnt*12] = data.material[j].v[ uvID[3] ];

					cnt += 1;
				}
			}
		}

		//Render
		glVertexPointer(3, GL_FLOAT, 0, VertexAry);
		glColorPointer(4, GL_FLOAT, 0, ColorAry);
		glTexCoordPointer(2, GL_FLOAT, 0, TexCoordAry);
		glDrawArrays(GL_TRIANGLE_STRIP, 1, 6*cnt-2);
	}

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	delete [] VertexAry;
	delete [] ColorAry;
	delete [] TexCoordAry;
}

//! @brief Get a map texture.
//! @param id Texture number
//! @return Texture Data Number (Failure:-1)
int D3DGraphics::GetMapTextureID(int id)
{
	if( (id < 0)||(TOTAL_BLOCKTEXTURE <= id ) ){ return -1; }
	return mapTextureID[id];
}

//! @brief Free the map data.
void D3DGraphics::CleanupMapdata()
{
	//Free Texture
	for(int i=0; i<TOTAL_BLOCKTEXTURE; i++){
		CleanupTexture(mapTextureID[i]);
	}

	bs = 0;

	BlockDataIF = NULL;
}

//! @brief Draw a model file
//! @param id_model Model Data Number
//! @param id_texture Texture Data Number
//! @param darkflag Dark Model
//! @param HUDmode Set the model for HUD rendering.
//! @param nomodel Disable model drawing
void D3DGraphics::RenderModel(int id_model, int id_texture, bool darkflag, bool HUDmode, bool nomodel)
{
	//Failed if an invalid argument is set
	if( id_model == -1 ){ return; }
	if( pmodel[id_model].useflag == false ){ return; }

	if( nomodel == true ){
		Renderline(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, GetColorCode(1.0f,0.5f,0.0f,1.0f));
		Renderline(0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, GetColorCode(1.0f,0.5f,0.0f,1.0f));
		Renderline(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, GetColorCode(1.0f,0.5f,0.0f,1.0f));
		return;
	}

	//Set if the texture is not loaded correctly
	if( id_texture == -1 ){
		//Texture Invalid
		glDisable(GL_TEXTURE_2D);
	}
	else if( ptextures[id_texture].useflag == false ){
		//Texture Invalid
		glDisable(GL_TEXTURE_2D);
	}
	else{
		//Texture enabled
		glEnable(GL_TEXTURE_2D);

		//Set Texture
		SetTexture(id_texture);
	}

	float *ColorAry = NULL;
	if( darkflag == false ){
		ColorAry = pmodel[id_model].ColorAry;
	}
	else{
		ColorAry = pmodel[id_model].ColorGrayAry;
	}

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Render
	glVertexPointer(3, GL_FLOAT, 0, pmodel[id_model].VertexAry);
	glColorPointer(4, GL_FLOAT, 0, ColorAry);
	glTexCoordPointer(2, GL_FLOAT, 0, pmodel[id_model].TexCoordAry);
	glDrawArrays(GL_TRIANGLE_STRIP, 1, pmodel[id_model].polygons*6-2);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

//! @brief Drawing Board
//! @param id_texture Texture Data Number
//! @param alpha Transparency (0.0 to 1.0 0.0: fully transparent)
//! @param nomodel Disable Board Drawing
void D3DGraphics::RenderBoard(int id_texture, float alpha, bool nomodel)
{
	//If the texture is not set, it will not be processed.
	if( id_texture == -1 ){ return; }
	if( ptextures[id_texture].useflag == false ){ return; }

	if( nomodel == true ){
		Renderline(0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, GetColorCode(0.75f,0.75f,0.75f,1.0f));
		Renderline(0.0f, -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, GetColorCode(0.75f,0.75f,0.75f,1.0f));
		Renderline(0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, GetColorCode(0.75f,0.75f,0.75f,1.0f));
		Renderline(0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, GetColorCode(0.75f,0.75f,0.75f,1.0f));
		return;
	}

	float VertexAry[4*3];
	float ColorAry[4*4];
	float TexCoordAry[4*2];

	//Texture enabled
	glEnable(GL_TEXTURE_2D);

	//Set Texture
	SetTexture(id_texture);

	//Prepare vertex array
	VertexAry[0] = 0.0f;	VertexAry[1] = 0.5f;	VertexAry[2] = 0.5f;
	VertexAry[3] = 0.0f;	VertexAry[4] = -0.5f;	VertexAry[5] = 0.5f;
	VertexAry[6] = 0.0f;	VertexAry[7] = 0.5f;	VertexAry[8] = -0.5f;
	VertexAry[9] = 0.0f;	VertexAry[10] = -0.5f;	VertexAry[11] = -0.5f;

	//Color information array available
	ColorAry[0] = 1.0f;
	ColorAry[1] = 1.0f;
	ColorAry[2] = 1.0f;
	ColorAry[3] = alpha;
	for(int i=1; i<4; i++){
		memcpy(&(ColorAry[i*4]), ColorAry, sizeof(float)*4);
	}

	//Prepare the UV-coordinate array.
	TexCoordAry[0] = 1.0f;	TexCoordAry[1] = 0.0f;
	TexCoordAry[2] = 1.0f;	TexCoordAry[3] = 1.0f;
	TexCoordAry[4] = 0.0f;	TexCoordAry[5] = 0.0f;
	TexCoordAry[6] = 0.0f;	TexCoordAry[7] = 1.0f;

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Render
	glVertexPointer(3, GL_FLOAT, 0, VertexAry);
	glColorPointer(4, GL_FLOAT, 0, ColorAry);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoordAry);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

//! @brief Settings for Effect Drawing
void D3DGraphics::StartEffectRender()
{
	glDisable(GL_DEPTH_TEST);
}

//! @brief Unset for effect drawing
void D3DGraphics::EndEffectRender()
{
	glEnable(GL_DEPTH_TEST);
}

//! @brief Set screen brightness
//! @param Width Width
//! @param Height Height.
//! @param Brightness Screen brightness (unchanged with 0, brightness degree at 1 or more)
void D3DGraphics::ScreenBrightness(int Width, int Height, int Brightness)
{
	//Does not process if the brightness is unchanged (lightweight)
	if( Brightness == 0 ){ return; }

	//Set transparency and draw
	float alpha = 0.02f * Brightness;
	Draw2DBox(0, 0, Width, Height, GetColorCode(1.0f,1.0f,1.0f,alpha));
}

//! @brief Display center lines for debugging.
void D3DGraphics::RenderCenterline()
{
	ResetWorldTransform();
	Renderline(100.0f, 0.0f, 0.0f, -100.0f, 0.0f, 0.0f, GetColorCode(1.0f,0.0f,0.0f,1.0f));
	Renderline(0.0f, 100.0f, 0.0f, 0.0f, -100.0f, 0.0f, GetColorCode(0.0f,1.0f,0.0f,1.0f));
	Renderline(0.0f, 0.0f, 100.0f, 0.0f, 0.0f, -100.0f, GetColorCode(0.0f,0.0f,1.0f,1.0f));
}

//! @brief Display lines for debugging.
void D3DGraphics::Renderline(float x1, float y1, float z1, float x2, float y2, float z2, int color)
{
	float VertexAry[2*3];
	unsigned char ColorAry[2*4];

	//Texture Invalid
	glDisable(GL_TEXTURE_2D);

	//Prepare vertex array
	VertexAry[0] = (float)x1*-1;	VertexAry[1] = (float)y1;	VertexAry[2] = (float)z1;
	VertexAry[3] = (float)x2*-1;	VertexAry[4] = (float)y2;	VertexAry[5] = (float)z2;

	//Color information array available
	Set4ByteBigEndian(&(ColorAry[0]), color);
	memcpy(&(ColorAry[4]), ColorAry, sizeof(unsigned char)*4);

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	//Display.
	glVertexPointer(3, GL_FLOAT, 0, VertexAry);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, ColorAry);
	glDrawArrays(GL_LINE_STRIP, 0, 2);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

//! @brief Get the longest number of characters
//! @param str String (Newline code: OK)
//! @return Number of characters
//! @attention Multibyte characters are counted as 2 per character.
int D3DGraphics::StrMaxLineLen(const char *str)
{
	int maxlen = 0;
	int cnt = 0;

	for(int i=0; i<(int)strlen(str); i++){
		if( str[i] == '\n' ){
			if( maxlen < cnt ){
				maxlen = cnt;
			}
			cnt = 0;
		}
		else{
			cnt += 1;
		}
	}

	if( maxlen < cnt ){
		maxlen = cnt;
	}

	return maxlen;
}

//! @brief Display character (using system font)
//! @param x x Coordinate
//! @param y y coordinates
//! @param str String (Newline code: OK)
//! @param color Color:
//! @warning This function can be called at most 100 times per frame (OpenGL implementation only).
//! @warning The <b> display is very low. If you call it many times on the </b> screen, it will affect the performance.
//! @warningPlease take measures such as "Display at once using newline code" and "Utilizing texture fonts for non-Japanese characters".
//! @attention The type and size of the font are fixed. It is difficult to see if the character is double overlapping and does not give up a standing feeling.
//! @todo There is a case where the first character is missing.
// GDI rasterization produces an alpha texture so glyphs scale with the game viewport.
void D3DGraphics::DrawGameFontText(int x,int y,int layoutWidth,const char* str,int color,bool centered)
{
    if(!str || !*str || !SystemFont) return;
    const UINT codepage = LanguageMode == 1 ? 949 : 932;
    int count=MultiByteToWideChar(codepage,0,str,-1,NULL,0);
    if(count<=1) return;
    std::wstring text(count,L'\0');
    MultiByteToWideChar(codepage,0,str,-1,&text[0],count); text.resize(count-1);
    LOGFONTW fontInfo={}; GetObjectW(SystemFont,sizeof(fontInfo),&fontInfo);
    const int margin=centered ? height/30 : 0;
    const int available=centered ? layoutWidth-margin*2 : 0;
    if(centered && available<=0) return;
    const std::wstring key=std::to_wstring(fontInfo.lfHeight)+L":"+std::to_wstring(available)+L":"+text;
    struct Raster {int width,height,tw,th;std::vector<unsigned char> pixels;};
    static std::map<std::wstring,Raster> cache;
    auto found=cache.find(key);
    if(found==cache.end()){
        HDC dc=CreateCompatibleDC(NULL); if(!dc) return;
        HGDIOBJ oldFont=SelectObject(dc,SystemFont);
        UINT flags=DT_NOPREFIX|DT_EXPANDTABS|(centered ? DT_CENTER|DT_WORDBREAK : DT_LEFT);
        RECT measured={0,0,available,0};
        DrawTextW(dc,text.c_str(),(int)text.size(),&measured,flags|DT_CALCRECT);
        Raster raster={}; raster.width=centered ? available : measured.right;
        raster.height=measured.bottom;
        if(raster.width<=0 || raster.height<=0){SelectObject(dc,oldFont);DeleteDC(dc);return;}
        raster.tw=1;raster.th=1;
        while(raster.tw<raster.width) raster.tw*=2;
        while(raster.th<raster.height) raster.th*=2;
        BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth=raster.width;info.bmiHeader.biHeight=-raster.height;
        info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
        void* bits=NULL;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
        if(!bitmap){SelectObject(dc,oldFont);DeleteDC(dc);return;}
        HGDIOBJ oldBitmap=SelectObject(dc,bitmap);
        ZeroMemory(bits,raster.width*raster.height*4);
        SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(255,255,255));
        RECT bounds={0,0,raster.width,raster.height};
        DrawTextW(dc,text.c_str(),(int)text.size(),&bounds,flags);GdiFlush();
        raster.pixels.resize(raster.tw*raster.th*4,0);
        const unsigned char* source=(const unsigned char*)bits;
        for(int row=0;row<raster.height;row++) for(int col=0;col<raster.width;col++){
            int from=(row*raster.width+col)*4,to=(row*raster.tw+col)*4;
            raster.pixels[to]=raster.pixels[to+1]=raster.pixels[to+2]=255;
            raster.pixels[to+3]=source[from];
        }
        SelectObject(dc,oldBitmap);SelectObject(dc,oldFont);DeleteObject(bitmap);DeleteDC(dc);
        if(cache.size()>=32) cache.clear();
        found=cache.emplace(key,std::move(raster)).first;
    }
    const Raster& raster=found->second;
    glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT|GL_COLOR_BUFFER_BIT);
    Start2DRender();
    glEnable(GL_TEXTURE_2D);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_FOG);
    GLuint texture;glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,raster.tw,raster.th,0,GL_RGBA,GL_UNSIGNED_BYTE,raster.pixels.data());
    glColor4ub((color>>24)&255,(color>>16)&255,(color>>8)&255,color&255);
    const float left=(float)(x+margin),top=(float)y;
    const float right=left+raster.width,bottom=top+raster.height;
    const float u=(float)raster.width/raster.tw,v=(float)raster.height/raster.th;
    glBegin(GL_QUADS);
    glTexCoord2f(0,0);glVertex3f(left,top,0);
    glTexCoord2f(u,0);glVertex3f(right,top,0);
    glTexCoord2f(u,v);glVertex3f(right,bottom,0);
    glTexCoord2f(0,v);glVertex3f(left,bottom,0);
    glEnd();glDeleteTextures(1,&texture);
    End2DRender();glPopAttrib();
}


void D3DGraphics::Draw2DMSFontText(int x, int y, const char *str, int color)
{
    DrawGameFontText(x,y,0,str,color,false);
}

//! @brief Display character (using system font) [with scaling function]
//! @param x x Coordinate
//! @param y y coordinates
//! @param str String (Newline code: OK)
//! @param color Color:
//! @attention Scaling from 640x480 resolution to current resolution. Other than that, it is equivalent to the Draw2DMSFontText() function.
void D3DGraphics::Draw2DMSFontTextScaling(int x, int y, const char *str, int color)
{
	if( str == NULL ){ return; }

	float scaling_x = (float)GameConfig.GetScreenHeight() / 480;//(float)GameConfig.GetScreenWidth() / 640;
	float scaling_y = (float)GameConfig.GetScreenHeight() / 480;

	Draw2DMSFontText((int)(scaling_x * x), (int)(scaling_y * y), str, color);
}

//! @brief Centralized display of characters (using system fonts)
//! @param x x Coordinate
//! @param y y coordinates
//! @param w Width.
//! @param h Vertical size
//! @param str String (Newline code: OK)
//! @param color Color:
void D3DGraphics::Draw2DMSFontTextCenter(int x, int y, int w, int h, const char *str, int color)
{
    UNREFERENCED_PARAMETER(h);
    DrawGameFontText(x,y,w,str,color,true);
}

//! @brief Display the character in the center (using system font) [with scaling function]
//! @param x x Coordinate
//! @param y y coordinates
//! @param w Width.
//! @param h Vertical size
//! @param str String (Newline code: OK)
//! @param color Color:
//! @attention scaling from 640x480 resolution to current resolution. Other than that, it is equivalent to the Draw2DMSFontTextCenter() function.
void D3DGraphics::Draw2DMSFontTextCenterScaling(int x, int y, int w, int h, const char *str, int color)
{
	if( str == NULL ){ return; }

	float scaling_x = (float)GameConfig.GetScreenHeight() / 480;//(float)GameConfig.GetScreenWidth() / 640;
	float scaling_y = (float)GameConfig.GetScreenHeight() / 480;

	Draw2DMSFontTextCenter((int)(scaling_x * x), (int)(scaling_y * y), w, h, str, color);
}

//! @brief Display characters (using system fonts, small)
//! @param x x Coordinate
//! @param y y coordinates
//! @param str String (Newline code: OK)
//! @param color Color:
//! @note The basic specification is equivalent to Draw2DMSFontText(), but the character is smaller.
//! @todo Displaying the same string at the normal font size may cause a bug. Needs verification.
void D3DGraphics::Draw2DMSSmallFontText(int x, int y, const char *str, int color)
{
	if( str == NULL ){ return; }

	int len = (int)strlen(str);
	WCHAR *ustr;

	y += 12;

	Start2DRender();
	glEnable(GL_DEPTH_TEST);

	//Texture Invalid
	glDisable(GL_TEXTURE_2D);

	//Convert to Unicode string
	ustr = new WCHAR [len+1];
	MultiByteToWideChar(CP_ACP,	0, str, -1, ustr, len + 1);

	//If you are a new string, you can create a resource
	if( now_SystemFontUStr == NULL || lstrcmpW(ustr, now_SystemFontUStr) != 0 ){
		GLuint listIdx;
		HDC hDC;

		//Remove old data
		glDeleteLists(SystemFontListIdx, SystemFontListIdxSize);
		delete [] now_SystemFontUStr;

		//Device Context Settings
		hDC = GetDC(hWnd);
		wglMakeCurrent(hDC, hGLRC);
		SelectObject(hDC, SystemSmallFont);

		//Create a display list
		listIdx = glGenLists(len);
		wglUseFontBitmapsW(hDC, ustr[0], 1, listIdx);	//Dummy
		for(int i=0; i<lstrlenW(ustr); i++){
			wglUseFontBitmapsW(hDC, ustr[i], 1, listIdx+i);
		}

		//Device Context Disposal
		ReleaseDC(hWnd, hDC);

		//Configure settings
		now_SystemFontUStr = new WCHAR [len+1];
		lstrcpyW(now_SystemFontUStr, ustr);
		SystemFontListIdx = listIdx;
		SystemFontListIdxSize = len;
	}

	//Set coordinates and colors
	glBitmap(0, 0, 0, 0, 10, 0, NULL);
	glRasterPos3f((float)x, (float)y, SystemFont_posz);
	glColor4ub((color>>24)&0xFF, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);

	for(int i=0; i<lstrlenW(ustr); i++){
		if( ustr[i] == '\n' ){
			//Insert a line break.
			y += 12;
			glRasterPos3f((float)x, (float)y, SystemFont_posz);
		}
		else{
			//Display List Drawing
			glCallList(SystemFontListIdx + i);
		}
	}

	SystemFont_posz -= 0.01f;

	//Disposal of Unicode strings
	delete [] ustr;

	//glDisable(GL_DEPTH_TEST);
	End2DRender();
}

//! @brief Setting for 2D drawing
void D3DGraphics::Start2DRender()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

//! @brief Texture font size calculation
//! @param str String (return code: <b> non</b>)
//! @param base_w Default single character width
//! @param base_h Default single character height
//! @param limitlen Default characters
//! @param w Pointer to get the width of the character
//! @param h Pointer to get the height of the character
void D3DGraphics::GetFontTextSize(const char *str, int base_w, int base_h, int limitlen, int *w, int *h)
{
	*w = base_w;
	*h = base_h;
	if( str == NULL ){ return; }
	if( limitlen <= 0 ){ return; }

	if( (int)strlen(str) <= limitlen ){ return; }

	float percent = (float)limitlen / strlen(str);

	*w = (int)((float)base_w * percent);
	*h = (int)((float)base_h * percent);
	return;
}

//! @brief Draw characters (using texture fonts)
//! @param x x Coordinate
//! @param y y coordinates
//! @param str String (return code: <b> non</b>)
//! @param color Color:
//! @param fontwidth Width of the character
//! @param fontheight Height of the character
//! @attention It is difficult to see if you double overlap the character and do not give a standing feeling.
void D3DGraphics::Draw2DTextureFontText(int x, int y, const char *str, int color, int fontwidth, int fontheight)
{
	//If the texture font has failed to be retrieved, it will not be processed
	if( TextureFont == -1 ){ return; }
	if( str == NULL ){ return; }

	int strlens = (int)strlen(str);

	float *VertexAry = new float [strlens*6*2];
	unsigned char *ColorAry = new unsigned char [strlens*6*4];
	float *TexCoordAry = new float [strlens*6*2];

	//Apply 2D drawing settings
	Start2DRender();

	int w;
	float font_u, font_v;
	float t_u, t_v;

	//1-character UV-coordinate curve
	font_u = 1.0f / 16;
	font_v = 1.0f / 16;

	//Texture enabled
	glEnable(GL_TEXTURE_2D);

	//Set Texture
	SetTexture(TextureFont);

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Color information array available
	Set4ByteBigEndian(&(ColorAry[0]), color);
	for(int i=1; i<strlens*6; i++){
		memcpy(&(ColorAry[i*4]), ColorAry, sizeof(unsigned char)*4);
	}

	// Given character minutes loop
	for(int i=0; i<strlens; i++){
		//Calculate UV coordinates.
		w = str[i];
		if( w < 0 ){ w += 256; }
		t_u = (w % 16) * font_u;
		t_v = (w / 16) * font_v;

		VertexAry[0 + i*12] = (float)x + i*fontwidth;			VertexAry[1 + i*12] = (float)y;
		VertexAry[2 + i*12] = (float)x + i*fontwidth;			VertexAry[3 + i*12] = (float)y;
		VertexAry[4 + i*12] = (float)x + fontwidth + i*fontwidth;	VertexAry[5 + i*12] = (float)y;
		VertexAry[6 + i*12] = (float)x + i*fontwidth;			VertexAry[7 + i*12] = (float)y + fontheight;
		VertexAry[8 + i*12] = (float)x + fontwidth + i*fontwidth;	VertexAry[9 + i*12] = (float)y + fontheight;
		VertexAry[10 + i*12] = (float)x + fontwidth + i*fontwidth;	VertexAry[11 + i*12] = (float)y + fontheight;
		TexCoordAry[0 + i*12] = t_u;		TexCoordAry[1 + i*12] = t_v;
		TexCoordAry[2 + i*12] = t_u;		TexCoordAry[3 + i*12] = t_v;
		TexCoordAry[4 + i*12] = t_u + font_u;	TexCoordAry[5 + i*12] = t_v;
		TexCoordAry[6 + i*12] = t_u;		TexCoordAry[7 + i*12] = t_v + font_v;
		TexCoordAry[8 + i*12] = t_u + font_u;	TexCoordAry[9 + i*12] = t_v + font_v;
		TexCoordAry[10 + i*12] = t_u + font_u;	TexCoordAry[11 + i*12] = t_v + font_v;
	}

	//Render
	glVertexPointer(2, GL_FLOAT, 0, VertexAry);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, ColorAry);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoordAry);
	glDrawArrays(GL_TRIANGLE_STRIP, 1, strlens*6-2);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	//Unset for 2D drawing
	End2DRender();

	delete [] VertexAry;
	delete [] ColorAry;
	delete [] TexCoordAry;
}

//! @brief Display character (with texture font) [Scaling function]
//! @param x x Coordinate
//! @param y y coordinates
//! @param str String (return code: <b> non</b>)
//! @param color Color:
//! @param fontwidth Width of the character
//! @param fontheight Height of the character
//! @attention Scale from a 640x480 reference resolution to the current resolution. Otherwise equivalent to Draw2DTextureFontText().
void D3DGraphics::Draw2DTextureFontTextScaling(int x, int y, const char *str, int color, int fontwidth, int fontheight)
{
	if( str == NULL ){ return; }

	float scaling_x = (float)GameConfig.GetScreenHeight() / 480;//(float)GameConfig.GetScreenWidth() / 640;
	float scaling_y = (float)GameConfig.GetScreenHeight() / 480;
	
	Draw2DTextureFontText((int)(scaling_x * x), (int)(scaling_y * y), str, color, (int)(scaling_x * fontwidth), (int)(scaling_x * fontheight));
}

//! @brief Displays the character in the center (using texture fonts)
//! @param x x Coordinate addition
//! @param y y coordinates
//! @param str String (return code: <b> non</b>)
//! @param color Color:
//! @param fontwidth Width of the character
//! @param fontheight Height of the character
//! @attention It is difficult to see if you double overlap the character and do not give a standing feeling.
void D3DGraphics::Draw2DTextureFontTextCenter(int x, int y, const char *str, int color, int fontwidth, int fontheight)
{
	if( str == NULL ){ return; }

	Draw2DTextureFontText((GameConfig.GetScreenWidth() - strlen(str)*fontwidth)/2 + x, y, str, color, fontwidth, fontheight);
}

//! @brief Display character in the center (with texture font) [with scaling function]
//! @param x x Coordinate addition
//! @param y y coordinates
//! @param str String (return code: <b> non</b>)
//! @param color Color:
//! @param fontwidth Width of the character
//! @param fontheight Height of the character
//! @attention Scaling from 640x480 resolution to current resolution. Other than that, it is equivalent to the Draw2DTexture ntTextCenter() function.
void D3DGraphics::Draw2DTextureFontTextCenterScaling(int x, int y, const char *str, int color, int fontwidth, int fontheight)
{
	if( str == NULL ){ return; }

	float scaling_x = (float)GameConfig.GetScreenHeight() / 480;//(float)GameConfig.GetScreenWidth() / 640;
	float scaling_y = (float)GameConfig.GetScreenHeight() / 480;
	
	Draw2DTextureFontTextCenter((int)(scaling_x * x), (int)(scaling_y * y), str, color, (int)(scaling_x * fontwidth), (int)(scaling_x * fontheight));
}

#ifdef ENABLE_DEBUGCONSOLE
//! @brief Display text using the debug font.
//! @param x x Coordinate
//! @param y y coordinates
//! @param str String (return code: <b> non</b>)
//! @param color Color:
//! @attention The width and height of the single character are 8x16 fixed.
//! @attention It is difficult to see if you double overlap the character and do not give a standing feeling.
void D3DGraphics::Draw2DTextureDebugFontText(int x, int y, const char *str, int color)
{
	if( str == NULL ){ return; }

	int fontwidth = 8;
	int fontheight = 16;

	//If the texture font has failed to be retrieved, it will not be processed
	if( TextureDebugFont == -1 ){ return; }

	int strlens = (int)strlen(str);

	float *VertexAry = new float [strlens*6*2];
	unsigned char *ColorAry = new unsigned char [strlens*6*4];
	float *TexCoordAry = new float [strlens*6*2];

	//Apply 2D drawing settings
	Start2DRender();

	int w;
	float font_u, font_v;
	float t_u, t_v;

	//1-character UV-coordinate curve
	font_u = 1.0f / 16;
	font_v = 1.0f / 8;

	//Texture enabled
	glEnable(GL_TEXTURE_2D);

	//Set Texture
	SetTexture(TextureDebugFont);

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Color information array available
	Set4ByteBigEndian(&(ColorAry[0]), color);
	for(int i=1; i<strlens*6; i++){
		memcpy(&(ColorAry[i*4]), ColorAry, sizeof(unsigned char)*4);
	}

	// Given character minutes loop
	for(int i=0; i<strlens; i++){
		//Calculate UV coordinates.
		w = str[i];
		if( w < 0 ){ w = ' '; }
		t_u = (w % 16) * font_u;
		t_v = (w / 16) * font_v;

		VertexAry[0 + i*12] = (float)x + i*fontwidth;			VertexAry[1 + i*12] = (float)y;
		VertexAry[2 + i*12] = (float)x + i*fontwidth;			VertexAry[3 + i*12] = (float)y;
		VertexAry[4 + i*12] = (float)x + fontwidth + i*fontwidth;	VertexAry[5 + i*12] = (float)y;
		VertexAry[6 + i*12] = (float)x + i*fontwidth;			VertexAry[7 + i*12] = (float)y + fontheight;
		VertexAry[8 + i*12] = (float)x + fontwidth + i*fontwidth;	VertexAry[9 + i*12] = (float)y + fontheight;
		VertexAry[10 + i*12] = (float)x + fontwidth + i*fontwidth;	VertexAry[11 + i*12] = (float)y + fontheight;
		TexCoordAry[0 + i*12] = t_u;		TexCoordAry[1 + i*12] = t_v;
		TexCoordAry[2 + i*12] = t_u;		TexCoordAry[3 + i*12] = t_v;
		TexCoordAry[4 + i*12] = t_u + font_u;	TexCoordAry[5 + i*12] = t_v;
		TexCoordAry[6 + i*12] = t_u;		TexCoordAry[7 + i*12] = t_v + font_v;
		TexCoordAry[8 + i*12] = t_u + font_u;	TexCoordAry[9 + i*12] = t_v + font_v;
		TexCoordAry[10 + i*12] = t_u + font_u;	TexCoordAry[11 + i*12] = t_v + font_v;
	}

	//Render
	glVertexPointer(2, GL_FLOAT, 0, VertexAry);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, ColorAry);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoordAry);
	glDrawArrays(GL_TRIANGLE_STRIP, 1, strlens*6-2);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	//Unset for 2D drawing
	End2DRender();

	delete [] VertexAry;
	delete [] ColorAry;
	delete [] TexCoordAry;
}
#endif

//! @brief Draw Lines
//! @param x1 x coordinates of the first point
//! @param y1 y coordinates of the first point
//! @param x2 x coordinates of the endpoint
//! @param y2 Endpoint y coordinates
//! @param color Color:
void D3DGraphics::Draw2DLine(int x1, int y1, int x2, int y2, int color)
{
	float VertexAry[2*2];
	unsigned char ColorAry[2*4];

	//Apply 2D drawing settings
	Start2DRender();

	//Texture Invalid
	glDisable(GL_TEXTURE_2D);

	//Prepare vertex array
	VertexAry[0] = (float)x1;	VertexAry[1] = (float)y1;
	VertexAry[2] = (float)x2;	VertexAry[3] = (float)y2;

	//Color information array available
	Set4ByteBigEndian(&(ColorAry[0]), color);
	memcpy(&(ColorAry[4]), ColorAry, sizeof(unsigned char)*4);

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	//Render
	glVertexPointer(2, GL_FLOAT, 0, VertexAry);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, ColorAry);
	glDrawArrays(GL_LINE_STRIP, 0, 2);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	//Unset for 2D drawing
	End2DRender();
}

//! @brief Draw a circle (16 squares)
//! @param x Center x coordinates
//! @param y Center y coordinates
//! @param r Radius.
//! @param color Color:
void D3DGraphics::Draw2DCycle(int x, int y, int r, int color)
{
	float VertexAry[(16+1)*2];
	unsigned char ColorAry[(16+1)*4];

	//Apply 2D drawing settings
	Start2DRender();

	//Texture Invalid
	glDisable(GL_TEXTURE_2D);

	//Set vertex coordinates
	for(int i=0; i<16+1; i++){
		float x2, y2;
		x2 = (float)x + cosf((float)M_PI*2/16 * i) * r;
		y2 = (float)y + sinf((float)M_PI*2/16 * i) * r;
		VertexAry[i*2] = x2;	VertexAry[i*2+1] = y2;
	}

	//Color information array available
	Set4ByteBigEndian(&(ColorAry[0]), color);
	for(int i=1; i<16+1; i++){
		memcpy(&(ColorAry[i*4]), ColorAry, sizeof(unsigned char)*4);
	}

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	//Render
	glVertexPointer(2, GL_FLOAT, 0, VertexAry);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, ColorAry);
	glDrawArrays(GL_LINE_STRIP, 0, 16+1);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	//Unset for 2D drawing
	End2DRender();
}

//! @brief Drawing rectangle
//! @param x1 Top left x coordinate
//! @param y1 Upper left y coordinates
//! @param x2 Right-bottom x coordinate
//! @param y2 Right-bottom y-coordinate
//! @param color Color:
void D3DGraphics::Draw2DBox(int x1, int y1, int x2, int y2, int color)
{
	float VertexAry[4*2];
	unsigned char ColorAry[4*4];

	//Apply 2D drawing settings
	Start2DRender();

	//Texture Invalid
	glDisable(GL_TEXTURE_2D);

	//Prepare vertex array
	VertexAry[0] = (float)x1;	VertexAry[1] = (float)y1;
	VertexAry[2] = (float)x2;	VertexAry[3] = (float)y1;
	VertexAry[4] = (float)x1;	VertexAry[5] = (float)y2;
	VertexAry[6] = (float)x2;	VertexAry[7] = (float)y2;

	//Color information array available
	Set4ByteBigEndian(&(ColorAry[0]), color);
	for(int i=1; i<4; i++){
		memcpy(&(ColorAry[i*4]), ColorAry, sizeof(unsigned char)*4);
	}

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	//Render
	glVertexPointer(2, GL_FLOAT, 0, VertexAry);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, ColorAry);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	//Unset for 2D drawing
	End2DRender();
}

//! @brief Drawing rectangle [with scaling function]
//! @param x1 Top left x coordinate
//! @param y1 Upper left y coordinates
//! @param x2 Right-bottom x coordinate
//! @param y2 Right-bottom y-coordinate
//! @param color Color:
//! @attention scaling from 640x480 resolution to current resolution. Other than that, it is equivalent to Draw2DBox() function.
void D3DGraphics::Draw2DBoxScaling(int x1, int y1, int x2, int y2, int color)
{
	float scaling_x = (float)GameConfig.GetScreenHeight() / 480;//(float)GameConfig.GetScreenWidth() / 640;
	float scaling_y = (float)GameConfig.GetScreenHeight() / 480;

	Draw2DBox((int)(scaling_x * x1), (int)(scaling_y * y1), (int)(scaling_x * x2), (int)(scaling_y * y2), color);
}

//! @brief Drawing Images
//! @param x x Coordinate
//! @param y y coordinates
//! @param id Texture Data Number
//! @param width Width
//! @param height Height.
//! @param alpha Transparency (0.0 to 1.0)
void D3DGraphics::Draw2DTexture(int x, int y, int id, int width, int height, float alpha)
{
	//Do not process if the invalid texture number is specified
	if( id == -1 ){ return; }

	float VertexAry[4*2];
	float ColorAry[4*4];
	float TexCoordAry[4*2];

	//Apply 2D drawing settings
	Start2DRender();

	//Texture enabled
	glEnable(GL_TEXTURE_2D);

	//Set Texture
	SetTexture(id);

	//Prepare vertex array
	VertexAry[0] = (float)x;		VertexAry[1] = (float)y;
	VertexAry[2] = (float)x+width;	VertexAry[3] = (float)y;
	VertexAry[4] = (float)x;		VertexAry[5] = (float)y+height;
	VertexAry[6] = (float)x+width;	VertexAry[7] = (float)y+height;

	//Color information array available
	ColorAry[0] = 1.0f;
	ColorAry[1] = 1.0f;
	ColorAry[2] = 1.0f;
	ColorAry[3] = alpha;
	for(int i=1; i<4; i++){
		memcpy(&(ColorAry[i*4]), ColorAry, sizeof(float)*4);
	}

	//Prepare the UV-coordinate array.
	TexCoordAry[0] = 0.0f;	TexCoordAry[1] = 0.0f;
	TexCoordAry[2] = 1.0f;	TexCoordAry[3] = 0.0f;
	TexCoordAry[4] = 0.0f;	TexCoordAry[5] = 1.0f;
	TexCoordAry[6] = 1.0f;	TexCoordAry[7] = 1.0f;

	//Enable the array.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Render
	glVertexPointer(2, GL_FLOAT, 0, VertexAry);
	glColorPointer(4, GL_FLOAT, 0, ColorAry);
	glTexCoordPointer(2, GL_FLOAT, 0, TexCoordAry);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//Array Disabling
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	//Unset for 2D drawing
	End2DRender();
}

//! @brief Drawing Images with Scaling Function
//! @param x x Coordinate
//! @param y y coordinates
//! @param id Texture Data Number
//! @param width Width
//! @param height Height.
//! @param alpha Transparency (0.0 to 1.0)
//! @attention scaling from 640x480 resolution to current resolution. Other than that, it is equivalent to the Draw2DTexture() function.
void D3DGraphics::Draw2DTextureScaling(int x, int y, int id, int width, int height, float alpha)
{
	float scaling_x = (float)GameConfig.GetScreenHeight() / 480;//(float)GameConfig.GetScreenWidth() / 640;
	float scaling_y = (float)GameConfig.GetScreenHeight() / 480;

	Draw2DTexture((int)(scaling_x * x), (int)(scaling_y * y), id, (int)(scaling_x * width), (int)(scaling_y * height), alpha);
}

//! @brief Unset for 2D drawing
void D3DGraphics::End2DRender()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

//! @brief Save screen s
//! @param filename File Name
//! @return success: true failure: false
bool D3DGraphics::SaveScreenShot(const char* filename)
{
	if( filename == NULL ){ return false; }

	HDC hDC;
	FILE *fp;
	unsigned char header[54];
	unsigned char pixel[3];

    RECT client; GetClientRect(hWnd, &client);
    GameViewport viewport = FitGameViewport(client.right, client.bottom, width, height);
    const int captureWidth = viewport.width, captureHeight = viewport.height;
    if(captureWidth <= 0 || captureHeight <= 0) return false;
    const int padding = (4 - (captureWidth*3)%4)%4;
    const int imageBytes = (captureWidth*3+padding)*captureHeight;
    unsigned char *dataBuffer = new unsigned char[captureWidth*captureHeight*3];

	//Device Context Settings
	hDC = GetDC(hWnd);
	wglMakeCurrent(hDC, hGLRC);

	//Store in a buffer
    GLint packAlignment; glGetIntegerv(GL_PACK_ALIGNMENT, &packAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(viewport.x, client.bottom-viewport.y-viewport.height, captureWidth, captureHeight, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);
    glPixelStorei(GL_PACK_ALIGNMENT, packAlignment);

	//Save files
	fp = fopen(filename, "wb");
	if( fp == NULL ){
		delete [] dataBuffer;
        ReleaseDC(hWnd, hDC);
		return false;		//Cannot save files
	}

	//rate headers
	for(int i=0; i<54; i++){ header[i] = 0x00; }
	header[0x00] = 'B';
	header[0x01] = 'M';
	Set4ByteLittleEndian(&(header[0x02]), 54 + imageBytes);
	header[0x0A] = 54;
	header[0x0E] = 54 - 14;
	Set4ByteLittleEndian(&(header[0x12]), captureWidth);
	Set4ByteLittleEndian(&(header[0x16]), captureHeight);
	header[0x1A] = 1;
	header[0x1C] = 24;
	header[0x1E] = 0;
	Set4ByteLittleEndian(&(header[0x22]), imageBytes);

	//Write a header
	fwrite(header, 1, 54, fp);

	for(int h=0; h<captureHeight; h++){
		for(int w=0; w<captureWidth; w++){
			pixel[2] = dataBuffer[(h*captureWidth+w)*3 + 0];
			pixel[1] = dataBuffer[(h*captureWidth+w)*3 + 1];
			pixel[0] = dataBuffer[(h*captureWidth+w)*3 + 2];

			fwrite(&pixel, 1, 3, fp);
		}

		if( padding != 0 ){
			//Padding to be multiple of 4
			unsigned char dummy[3];
			dummy[0] = 0x00;
			dummy[1] = 0x00;
			dummy[2] = 0x00;
			fwrite(&dummy, 1, padding, fp);
		}
	}

	//Free File Handle
	fclose(fp);

	delete [] dataBuffer;

	ReleaseDC(hWnd, hDC);

	return true;
}

//! @brief Get Color Code
//! @param red Red(0.0f to 1.0f)
//! @param green Green(0.0f to 1.0f)
//! @param blue Blue(0.0f to 1.0f)
//! @param alpha Transparency (0.0f to 1.0f)
//! @return Color Code
int D3DGraphics::GetColorCode(float red, float green, float blue, float alpha)
{
	unsigned char red2, green2, blue2, alpha2;
	red2 = (unsigned char)(red*255);
	green2 = (unsigned char)(green*255);
	blue2 = (unsigned char)(blue*255);
	alpha2 = (unsigned char)(alpha*255);

	return (red2 << 24) | (green2 << 16) | (blue2 << 8) | alpha2;
}

#endif	//GRAPHIC_ENGINE