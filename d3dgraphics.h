//! @file d3dgraphics.h
//! @brief D3DGraphics class declaration.

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

#ifndef D3DGRAPHICS_H
#define D3DGRAPHICS_H

#define MAX_MODEL 72		//!< Maximum number of models
#define MAX_TEXTURE 72		//!< Maximum Texture

#define CLIPPINGPLANE_NEAR 1.0f		//!< Near clip surface (minimum distance to draw)
#define CLIPPINGPLANE_FAR 800.0f	//!< Distance clip surface (approximately the maximum distance to be drawn)

#ifndef H_LAYERLEVEL
 #define H_LAYERLEVEL 1		//!< Select include file.
#endif
#include "main.h"

//! @brief Selecting Graphic Scores to Use
//! @details DirectX 9.0c (fixed pipeline): 0; DirectX 9.0c (shaders): 1; OpenGL 1.1: 8.
//! @attention When using DirectX 9.0c shaders, also edit "resource.rc".
#ifndef GRAPHIC_ENGINE
#define GRAPHIC_ENGINE 0
#endif

#if (GRAPHIC_ENGINE == 0)||(GRAPHIC_ENGINE == 1)

#define BLOCKDATA_GPUMEMORY	//!< @brief Select block-data storage. @details Defined: GPU memory; undefined (commented out): main memory.

#pragma warning(disable:4819)		//VC++ warning prevention
#include <d3dx9.h>
#pragma warning(default:4819)

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#if GRAPHIC_ENGINE == 0
 #define GRAPHICS_CORE "DirectX 9.0"				//!< Version display information
#else
 #define GRAPHICS_CORE "DirectX 9.0 (Shader)"		//!< Version display information
 #define HLSLFILE 201								//!< HLSL files in resources
#endif

#elif GRAPHIC_ENGINE == 8

//#define ENABLE_OPENGL_LIBJPEGPNG // Use libjpeg and libpng with OpenGL.

#include <ctype.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#ifdef ENABLE_OPENGL_LIBJPEGPNG
 #include <jpeglib.h>
 #include <zlib.h>
 #include <png.h>

 //The Release/Win32 project links matching static JPEG, PNG, and zlib libraries from lib/modern.




 //#pragma comment(lib, "legacy_stdio_definitions.lib")



#endif

#define GRAPHICS_CORE "OpenGL 1.1"		//!< Version display information

#endif	//GRAPHIC_ENGINE

#if (GRAPHIC_ENGINE == 0)||(GRAPHIC_ENGINE == 1)

//! Structure for 3D polygon drawing
typedef struct
{
	D3DXVECTOR3 position;	//!< The position
	DWORD       color;		//!< The color 
	FLOAT       tu;			//!< The texture coordinates
	FLOAT       tv;			//!< The texture coordinates
} VERTEXTXTA;

//! Structure for 2D polygon drawing
typedef struct
{
	FLOAT    x;		//!< position
	FLOAT    y;		//!< position
	FLOAT    z;		//!< position
	FLOAT    rhw;	//!< vector
	D3DCOLOR color;	//!< color
	FLOAT    tu;	//!< texture coordinates
	FLOAT    tv;	//!< texture coordinates
} TLVERTX;

#elif GRAPHIC_ENGINE == 8

//! Model data vertex storage structure
typedef struct
{
	float x;	//!< position
	float y;	//!< position
	float z;	//!< position
	float u;	//!< texture coordinates
	float v;	//!< texture coordinates
} MODELVDATA;

//! Model data structure
typedef struct
{
	bool useflag;			//!< Enabled flags
	int polygons;			//!< Polygon count.
	float *VertexAry;		//!< Array storing vertices.
	float *ColorAry;		//!< Array storing colors.
	float *ColorGrayAry;	//!< Color collection array (dark)
	float *TexCoordAry;		//!< Texture coordinate storage array
} MODELDATA;

//! Texture structure
typedef struct
{
	bool useflag;			//!< Enabled flags
	int width;				//!< Width
	int height;				//!< Height.
	unsigned char *data;	//!< Real Data
} TEXTUREDATA;

#endif	//GRAPHIC_ENGINE


//! @brief Class for screen drawing
//! @details Draw screens and process directly related to them.
//! @details However, there is a function to perform 3D drawing, 2D drawing, model file and texture management.
class D3DGraphics
{

#if (GRAPHIC_ENGINE == 0)||(GRAPHIC_ENGINE == 1)

	LPDIRECT3D9 pD3D;					//!< DIRECT3D9 Pointer
	LPDIRECT3DDEVICE9 pd3dDevice;		//!< DIRECT3DDEVICE9 Pointer
	float aspect;						//!< Screen aspect ratio
	bool fullscreenflag;				//!< Full Screen Display
	int LanguageMode;					//!< Language Selection
	LPD3DXMESH pmesh[MAX_MODEL];				//!< D3DXMESH pointer for X file
	DWORD nummaterials[MAX_MODEL];				//!< Number of materials (for X files)
	LPDIRECT3DTEXTURE9 ptextures[MAX_TEXTURE];	//!< Store textures

#if GRAPHIC_ENGINE == 1
	LPDIRECT3DVERTEXSHADER9	hVertexShader;			//!< vertice shader
	LPD3DXCONSTANTTABLE hVSConstantTable;			//!< Vertex shader constant table
	D3DXHANDLE hVSWorldVertexPos;					//!< Transformation matrix of vertex shader
	D3DXHANDLE hVSWorldViewPos;						//!< Transformation matrix of vertex shader
	D3DXHANDLE hVSWorldViewProj;					//!< Transformation matrix of vertex shader
	D3DXHANDLE hVSDiffuse;							//!< Diffuse color for the vertex shader.
	LPDIRECT3DPIXELSHADER9 pPixelShader;			//!< Pixel Shader
	LPD3DXCONSTANTTABLE pPSConstantTable;			//!< Pixel Shader Constant Table
	D3DXHANDLE hPSViewPos;							//!< Pixel shader camera coordinates
	D3DXHANDLE hPSEnable2DRendering;				//!< 2D drawing flag for pixel shader
	D3DXHANDLE hPSEnableTexture;					//!< Pixel shader texture flag
	D3DXHANDLE hPSEnableFog;						//!< Pixel shader fog valid flag
	D3DXHANDLE hPSFogColor;							//!< Pixel shader fog color
	D3DXHANDLE hPSFogStart;							//!< Pixel shader fog start depth
	D3DXHANDLE hPSFogEnd;							//!< Pixel shader fog depth
	D3DXMATRIX VSWorldPosMatrix;					//!< Conversion matrix given to the vertex shader
	D3DXMATRIX VSViewPosMatrix;						//!< Conversion matrix given to the vertex shader
	D3DXMATRIX VSViewProjMatrix;					//!< Conversion matrix given to the vertex shader
#endif

	class BlockDataInterface* BlockDataIF;		//!< Pointer to a class that stores the imported block data
	int bs;										//!< Number of blocks
	int mapTextureID[TOTAL_BLOCKTEXTURE];		//!< Texture number
#ifdef BLOCKDATA_GPUMEMORY
	LPDIRECT3DVERTEXBUFFER9 g_pVB;				//!< Vertex data managed by the GPU.
#else
	VERTEXTXTA g_pVertices[MAX_BLOCKS][6][4];	//!< Vertex data managed by the CPU.
#endif

	float HUD_myweapon_x[TOTAL_HAVEWEAPON];		//!< X coordinate for rendering the HUD weapon.
	float HUD_myweapon_y[TOTAL_HAVEWEAPON];		//!< Y coordinate for rendering the HUD weapon.
	float HUD_myweapon_z[TOTAL_HAVEWEAPON];		//!< Z coordinate for rendering the HUD weapon.

	bool StartRenderFlag;			//!< Flag indicating that StartRender() is running.
	bool EnableFogFlag;				//!< Flags that represent when using fog

	LPD3DXSPRITE ptextsprite;			//!< 2D texture sprite.
	LPD3DXFONT pxmsfont;				//!< System Fonts
	LPD3DXFONT pxmssmallfont;			//!< System Font (Small)
	char TextureFontFname[_MAX_PATH];	//!< Texture font file name
	int TextureFont;					//!< Texture Font Texture ID

	int InitSubset();
	void CleanupD3Dresource();
	void Start2DMSFontTextRender();
	void End2DMSFontTextRender();
	void Start2DRender();
	void End2DRender();

#elif GRAPHIC_ENGINE == 8

	HWND hWnd;				//!< Window Handle
	HGLRC hGLRC;			//!< OpenGL Context
	int width;				//!< Width
	int height;				//!< Height.
	bool fullscreenflag;	//!< Full Screen Display
	int LanguageMode;		//!< Language Selection
	PAINTSTRUCT Paint_ps;					//!< Used by BeginPaint() and EndPaint().
	MODELDATA pmodel[MAX_MODEL];			//!< Store model data
	TEXTUREDATA ptextures[MAX_TEXTURE];		//!< Store textures
	GLuint textureobjname[MAX_TEXTURE];		//!< Texture object
	HFONT SystemFont;				//!< Logical font for system fonts
	HFONT SystemSmallFont;			//!< Logical fonts for system fonts (small)
	WCHAR *now_SystemFontUStr;		//!< String by system font currently displayed (Unicode)
	GLuint SystemFontListIdx;		//!< System Font Display List
	int SystemFontListIdxSize;		//!< System Font Display List Size
	float SystemFont_posz;			//!< System Font Z coordinates
	int now_textureid;		//!< Current Texture Number

	float camera_x;			//!< Camera Coordinate
	float camera_y;			//!< Camera Coordinate
	float camera_z;			//!< Camera Coordinate
	float camera_rx;		//!< Camera Rotation Angle
	float camera_ry;		//!< Camera Rotation Angle
	float viewangle;		//!< Camera Viewing Angle

	class BlockDataInterface* BlockDataIF;		//!< Pointer to a class that stores the imported block data
	int bs;										//!< Number of blocks
	int mapTextureID[TOTAL_BLOCKTEXTURE];		//!< Texture number

	char TextureFontFname[_MAX_PATH];	//!< Texture font file name
	int TextureFont;				//!< Texture Font Texture ID

	float HUD_myweapon_x[TOTAL_HAVEWEAPON];		//!< X coordinate for rendering the HUD_A weapon.
	float HUD_myweapon_y[TOTAL_HAVEWEAPON];		//!< Y coordinate for rendering the HUD_A weapon.
	float HUD_myweapon_z[TOTAL_HAVEWEAPON];		//!< Z coordinate for rendering the HUD_A weapon.

#ifdef ENABLE_OPENGL_LIBJPEGPNG
	jpeg_decompress_struct cinfo;	//!< libjpeg
	jpeg_error_mgr jerr;			//!< libjpeg
#endif

	void CleanupD3Dresource();
	int CheckTextureFileExtension(const char* filename, int nowformat);
	int CheckTextureFileTypeFlag(const char* filename, int nowformat);
	bool LoadBMPTexture(const char* filename, bool BlackTransparent, TEXTUREDATA *ptexture);
	bool LoadDDSTexture(const char* filename, bool BlackTransparent, TEXTUREDATA *ptexture);
	bool LoadJPEGTexture(const char* filename, bool BlackTransparent, TEXTUREDATA *ptexture);
	bool LoadPNGTexture(const char* filename, bool BlackTransparent, TEXTUREDATA *ptexture);
	void SetTexture(int TextureID);
	int StrMaxLineLen(const char *str);
	void Start2DRender();
	void End2DRender();

#endif	//GRAPHIC_ENGINE

#ifdef ENABLE_DEBUGCONSOLE
	unsigned char DebugFontData[96][16];	//!< Array storing debug font data.
	int TextureDebugFont;					//!< Texture ID of the debug font.
	void InitDebugFontData();
	bool LoadDebugFontTexture();
#endif

public:
	D3DGraphics();
	~D3DGraphics();
	void SetFullScreenFlag(bool fullscreen);
	bool GetFullScreenFlag();
	void SetlanguageMode(int mode);
	int GetlanguageMode();
	int InitD3D(WindowControl *WindowCtrl, const char *TextureFontFilename, bool fullscreen, int language);
	int ResetD3D(WindowControl *WindowCtrl);
	void DestroyD3D();
	int LoadModel(const char* filename);
	int MorphingModel(int idA, int idB);
	int GetTotalModels();
	void CleanupModel(int id);
	int LoadTexture(const char* filename, bool texturefont, bool BlackTransparent);
	int GetTextureSize(int id, int *width, int *height);
	int GetTotalTextures();
	void CleanupTexture(int id);
	int StartRender();
	bool EndRender();
	void ResetZbuffer();
	void ResetWorldTransform();
	void SetWorldTransform(float x, float y, float z, float rx, float ry, float size);
	void SetWorldTransform(float x, float y, float z, float rx, float ry1, float ry2, float size);
	void SetWorldTransformEffect(float x, float y, float z, float rx, float ry, float rt, float size);
	void SetWorldTransformHumanWeapon(float x, float y, float z, float mx, float my, float mz, float rx, float ry, float size);
	void SetWorldTransformPlayerWeapon(bool rotation, float camera_rx, float camera_ry, float rx, float size);
	void GetWorldTransformPos(float *x, float *y, float *z);
	void SetFog(bool enablefog, int skynumber);
	void SetCamera(float camera_x, float camera_y, float camera_z, float camera_rx, float camera_ry, float viewangle);
	void LoadMapdata(class BlockDataInterface* in_BlockDataIF, const char *directory);
	void RenderMapdata(bool wireframe);
	int GetMapTextureID(int id);
	void CleanupMapdata();
	void RenderModel(int id_model, int id_texture, bool darkflag, bool HUDmode, bool nomodel);
	void RenderBoard(int id_texture, float alpha, bool nomodel);
	void StartEffectRender();
	void EndEffectRender();
	void ScreenBrightness(int Width, int Height, int Brightness);
	void RenderCenterline();
	void Renderline(float x1, float y1, float z1, float x2, float y2, float z2, int color);
#if GRAPHIC_ENGINE == 8
    void DrawGameFontText(int x, int y, int layoutWidth, const char* str, int color, bool centered);
#endif
	void Draw2DMSFontText(int x, int y, const char *str, int color);
	void Draw2DMSFontTextScaling(int x, int y, const char *str, int color);
	void Draw2DMSFontTextCenter(int x, int y, int w, int h, const char *str, int color);
	void Draw2DMSFontTextCenterScaling(int x, int y, int w, int h, const char *str, int color);
	void Draw2DMSSmallFontText(int x, int y, const char *str, int color);
	void GetFontTextSize(const char *str, int base_w, int base_h, int limitlen, int *w, int *h);
	void Draw2DTextureFontText(int x, int y, const char *str, int color, int fontwidth, int fontheight);
	void Draw2DTextureFontTextScaling(int x, int y, const char *str, int color, int fontwidth, int fontheight);
	void Draw2DTextureFontTextCenter(int x, int y, const char *str, int color, int fontwidth, int fontheight);
	void Draw2DTextureFontTextCenterScaling(int x, int y, const char *str, int color, int fontwidth, int fontheight);
#ifdef ENABLE_DEBUGCONSOLE
	void Draw2DTextureDebugFontText(int x, int y, const char *str, int color);
#endif
	void Draw2DLine(int x1, int y1, int x2, int y2, int color);
	void Draw2DCycle(int x, int y, int r, int color);
	void Draw2DBox(int x1, int y1, int x2, int y2, int color);
	void Draw2DBoxScaling(int x1, int y1, int x2, int y2, int color);
	void Draw2DTexture(int x, int y, int id, int width, int height, float alpha);
	void Draw2DTextureScaling(int x, int y, int id, int width, int height, float alpha);
	bool SaveScreenShot(const char *fname);
	int GetColorCode(float red, float green, float blue, float alpha);
};

#endif