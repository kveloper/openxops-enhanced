//! @file d3dgraphics-directx.cpp
//! @brief D3DGraphics class definitions (DirectX implementation).

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

#include "d3dgraphics.h"

#if (GRAPHIC_ENGINE == 0)||(GRAPHIC_ENGINE == 1)

//! @brief Constructor
D3DGraphics::D3DGraphics()
{
	pD3D = NULL;
	pd3dDevice = NULL;
	aspect = 1.0f;
	fullscreenflag = false;
	LanguageMode = 0;
	for(int i=0; i<MAX_MODEL; i++){
		pmesh[i] = NULL;
		nummaterials[i] = 0;
	}
	for(int i=0; i<MAX_TEXTURE; i++){
		ptextures[i] = NULL;
	}

#if GRAPHIC_ENGINE == 1
	hVertexShader = NULL;
	hVSConstantTable = NULL;
	hVSWorldVertexPos = NULL;
	hVSWorldViewPos = NULL;
	hVSWorldViewProj = NULL;
	hVSDiffuse = NULL;
	pPixelShader = NULL;
	pPSConstantTable = NULL;
	hPSViewPos = NULL;
	hPSEnable2DRendering = NULL;
	hPSEnableTexture = NULL;
	hPSEnableFog = NULL;
	hPSFogColor = NULL;
	hPSFogStart = NULL;
	hPSFogEnd = NULL;
#endif

	BlockDataIF = NULL;
	bs = 0;
	for(int i=0; i<TOTAL_BLOCKTEXTURE; i++){
		mapTextureID[i] = -1;
	}
#ifdef BLOCKDATA_GPUMEMORY
	g_pVB = NULL;
#else
	ZeroMemory(g_pVertices, sizeof(VERTEXTXTA)*MAX_BLOCKS*6*4);
#endif

	for( int i=0; i<TOTAL_HAVEWEAPON; i++ ){
		HUD_myweapon_x[i] = 0.0f;
		HUD_myweapon_y[i] = 0.0f;
		HUD_myweapon_z[i] = 0.0f;
	}

	StartRenderFlag = false;
	EnableFogFlag = false;

	ptextsprite = NULL;
	pxmsfont = NULL;
	pxmssmallfont = NULL;
	TextureFontFname[0] = '\0';
	TextureFont = -1;

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
//! (DirectX 9)
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
	OutputLog.WriteLog(LOG_INIT, "Graphics", "DirectX");
#endif

	D3DPRESENT_PARAMETERS d3dpp;
	RECT rec;

	GetClientRect(WindowCtrl->GethWnd(), &rec);

	fullscreenflag = fullscreen;
	LanguageMode = language;

	//Create D3D9
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if( pD3D == NULL ){
		return 1;
	}

	//3 a D3D device
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	if( fullscreenflag == false ){
		d3dpp.Windowed = TRUE;
		d3dpp.BackBufferWidth = rec.right;
		d3dpp.BackBufferHeight = rec.bottom;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.FullScreen_RefreshRateInHz = 0;
	}
	else{
		D3DDISPLAYMODE dispmode;
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dispmode);

		d3dpp.Windowed = FALSE;
		d3dpp.BackBufferWidth = rec.right;
		d3dpp.BackBufferHeight = rec.bottom;
		d3dpp.BackBufferFormat = dispmode.Format;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.FullScreen_RefreshRateInHz = dispmode.RefreshRate;
	}

	if( FAILED( pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, WindowCtrl->GethWnd(), D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pd3dDevice) ) ){
		if( FAILED( pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, WindowCtrl->GethWnd(), D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pd3dDevice) ) ){
			return 1;
		}
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif

	//Set the file name of the image for texture font
	strcpy(TextureFontFname, TextureFontFilename);

	//Detailed setting of drawing relationship
	if( InitSubset() != 0){
		return 1;
	}
	

#if GRAPHIC_ENGINE == 1
	HRESULT hr;
	LPD3DXBUFFER pCode;
	LPD3DXBUFFER pErrorMes;

	//Initialization of vertex shader
	//hr = D3DXCompileShaderFromFile("DirectX_Shader01.fx", NULL, NULL, "VertexShader_Main", "vs_2_0", 0, &pCode, &pErrorMes, &hVSConstantTable);
	hr = D3DXCompileShaderFromResource(NULL, MAKEINTRESOURCE(HLSLFILE), NULL, NULL, "VertexShader_Main", "vs_2_0", 0, &pCode, &pErrorMes, &hVSConstantTable);
	if( FAILED(hr) ){
#ifdef _DEBUG
		//Error display during shader compilation
		OutputDebugStringA( (const char*)pErrorMes->GetBufferPointer() );
		//WindowCtrl->ErrorInfo( (const char*)pErrorMes->GetBufferPointer() );
#endif
		return 1;
	}
	hr = pd3dDevice->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &hVertexShader);
	pCode->Release();
	if( FAILED(hr) ){ return 1; }

	//Vertex shader related variables
	hVSWorldVertexPos = hVSConstantTable->GetConstantByName(NULL, "WorldVertexPos");
	if( hVSWorldVertexPos == NULL ){ return 1; }
	hVSWorldViewPos = hVSConstantTable->GetConstantByName(NULL, "WorldViewPos");
	if( hVSWorldViewPos == NULL ){ return 1; }
	hVSWorldViewProj = hVSConstantTable->GetConstantByName(NULL, "WorldViewProj");
	if( hVSWorldViewProj == NULL ){ return 1; }
	hVSDiffuse = hVSConstantTable->GetConstantByName(NULL, "Diffuse");
	if( hVSDiffuse == NULL ){ return 1; }

	//Pixel shader initialization
	//hr = D3DXCompileShaderFromFile("DirectX_Shader01.fx", NULL, NULL, "PixelShader_Main", "ps_2_0", 0, &pCode, &pErrorMes, &pPSConstantTable);
	hr = D3DXCompileShaderFromResource(NULL, MAKEINTRESOURCE(HLSLFILE), NULL, NULL, "PixelShader_Main", "ps_2_0", 0, &pCode, &pErrorMes, &pPSConstantTable);
	if( FAILED(hr) ){
#ifdef _DEBUG
		//Error display during shader compilation
		OutputDebugStringA( (const char*)pErrorMes->GetBufferPointer() );
		//WindowCtrl->ErrorInfo( (const char*)pErrorMes->GetBufferPointer() );
#endif
		return 1;
	}
	hr = pd3dDevice->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), &pPixelShader);
	pCode->Release();
	if( FAILED(hr) ){ return 1; }

	//Related Parameters of Pixel Shader
	hPSViewPos = pPSConstantTable->GetConstantByName(NULL, "ViewPos");
	//if( hPSViewPos == NULL ){ return 1; }
	hPSEnable2DRendering = pPSConstantTable->GetConstantByName(NULL, "Enable2DRendering");
	//if( hPSEnable2DRendering == NULL ){ return 1; }
	hPSEnableTexture = pPSConstantTable->GetConstantByName(NULL, "EnableTexture");
	if( hPSEnableTexture == NULL ){ return 1; }
	hPSEnableFog = pPSConstantTable->GetConstantByName(NULL, "EnableFog");
	if( hPSEnableFog == NULL ){ return 1; }
	hPSFogColor = pPSConstantTable->GetConstantByName(NULL, "FogColor");
	if( hPSFogColor == NULL ){ return 1; }
	hPSFogStart = pPSConstantTable->GetConstantByName(NULL, "FogStart");
	if( hPSFogStart == NULL ){ return 1; }
	hPSFogEnd = pPSConstantTable->GetConstantByName(NULL, "FogEnd");
	if( hPSFogEnd == NULL ){ return 1; }

	D3DXMatrixIdentity(&VSWorldPosMatrix);
	D3DXMatrixIdentity(&VSViewPosMatrix);
	D3DXMatrixIdentity(&VSViewProjMatrix);
#endif


	//Set the Aspect Ratio
	aspect = (float)rec.right / (float)rec.bottom;

	//Turn off mouse cursor
	//ShowCursor(FALSE);


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

	return 0;
}

//! @brief Reset @n
//! (e.g. return from window minimization)
//! @param WindowCtrl WindowControl class pointer
//! @return Success: 0; waiting: 1; failure: 2.
int D3DGraphics::ResetD3D(WindowControl *WindowCtrl)
{
	if( WindowCtrl == NULL ){ return 2; }

	//If you lose focus, wait
	if( pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICELOST ){
		return 1;
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CHECK, "Graphics", "DirectX lost");
#endif

	//Resource Release
	CleanupD3Dresource();

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_INIT, "Graphics", "DirectX (Reset)");
#endif

	D3DPRESENT_PARAMETERS d3dpp;
	RECT rec;

	GetClientRect(WindowCtrl->GethWnd(), &rec);

	//3 a D3D device
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	if( fullscreenflag == false ){
		d3dpp.Windowed = TRUE;
		d3dpp.BackBufferWidth = rec.right;
		d3dpp.BackBufferHeight = rec.bottom;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.FullScreen_RefreshRateInHz = 0;
	}
	else{
		D3DDISPLAYMODE dispmode;
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dispmode);

		d3dpp.Windowed = FALSE;
		d3dpp.BackBufferWidth = rec.right;
		d3dpp.BackBufferHeight = rec.bottom;
		d3dpp.BackBufferFormat = dispmode.Format;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.FullScreen_RefreshRateInHz = dispmode.RefreshRate;
	}

	if( FAILED( pd3dDevice->Reset(&d3dpp) ) ){
		return 2;
	}

	//Detailed setting of drawing relationship
	if( InitSubset() != 0){
		return 2;
	}

#ifdef ENABLE_DEBUGCONSOLE
	if( LoadDebugFontTexture() == false ){
		return 2;
	}
#endif

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", "");
#endif

	return 0;
}

//! @brief Release resources.
//! @attention This function is automatically called, but it can be called explicitly.
void D3DGraphics::DestroyD3D()
{
	if( (pd3dDevice == NULL)&&(pD3D == NULL) ){ return; }

	//Resource Release
	CleanupD3Dresource();

#if GRAPHIC_ENGINE == 1
	if( pPSConstantTable != NULL ){
		pPSConstantTable->Release();
		pPSConstantTable = NULL;
	}
	if( pPixelShader != NULL ){
		pPixelShader->Release();
		pPixelShader = NULL;
	}
	if( hVSConstantTable != NULL ){
		hVSConstantTable->Release();
		hVSConstantTable = NULL;
	}
	if( hVertexShader != NULL ){
		hVertexShader->Release();
		hVertexShader = NULL;
	}
#endif

	if( pd3dDevice != NULL ){
		pd3dDevice->Release();
		pd3dDevice = NULL;
	}
	if( pD3D != NULL ){
		pD3D->Release();
		pD3D = NULL;
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Graphics", "DirectX");
#endif
}

//! @brief Detail setting of drawing relationship
//! @attention Run only once at initialization.
int D3DGraphics::InitSubset()
{
	//Light
	//pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(0,255,255,255) );
	pd3dDevice->LightEnable(0, FALSE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// Using Texture Filters
	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	//Specify the detail level (LOD) bias of the MIP map.
	float LODBias = -0.2f;
	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&LODBias)) );

	//Alfa Blending
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	//Transmittance processing
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//Checking whether it is compatible with alpha testing
	D3DCAPS9 Caps;
	pd3dDevice->GetDeviceCaps(&Caps);
	if( Caps.AlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL ){
		//Alpha Test Settings
		// Perfectly transparent pixels do not draw
		pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000001);
		pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
		pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	}

	//Deep buffer comparison function
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

	//Back of Polygon
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);


	//Initializing text sprites
	if( FAILED( D3DXCreateSprite( pd3dDevice, &ptextsprite ) ) ){
		return 1;
	}

	if( LanguageMode == 1 ){
		// ---English/Korean---

		//Font Name:Dotum Size:18
		if( FAILED( D3DXCreateFont( pd3dDevice, (int)(((float)GameConfig.GetScreenHeight() / 480) * -18), 0, FW_NORMAL, 1, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS,
										DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Dotum", &pxmsfont) ) 
		){
			return 1;
		}
		//Font Name:Dotum Size:12
		if( FAILED( D3DXCreateFont( pd3dDevice, -12, 0, FW_NORMAL, 1, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS,
										DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Dotum", &pxmssmallfont) ) 
		){
			return 1;
		}
	}
	else{
		// ---English/Japanese---

		//Font Name: MS Goth Size:18
		//if( FAILED( D3DXCreateFont( pd3dDevice, -18, 0, FW_NORMAL, 1, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		if( FAILED( D3DXCreateFont( pd3dDevice, (int)(((float)GameConfig.GetScreenHeight() / 480) * -18), 0, FW_NORMAL, 1, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
										DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "‚l‚r ƒSƒVƒbƒN", &pxmsfont) ) 
		){
			return 1;
		}
		//Font Name: MS Goth Size:12
		if( FAILED( D3DXCreateFont( pd3dDevice, -12, 0, FW_NORMAL, 1, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
										DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "‚l‚r ƒSƒVƒbƒN", &pxmssmallfont) ) 
		){
			return 1;
		}
	}

	//Get images for texture fonts
	TextureFont = LoadTexture(TextureFontFname, true, false);

	//Initialize fog again
	EnableFogFlag = false;

	return 0;
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

	if( pxmsfont != NULL ){
		pxmsfont->Release();
		pxmsfont = NULL;
	}
	if( pxmssmallfont != NULL ){
		pxmssmallfont->Release();
		pxmssmallfont = NULL;
	}
	if( ptextsprite != NULL ){
		ptextsprite->Release();
		ptextsprite = NULL;
	}

	CleanupMapdata();

	for(int i=0; i<MAX_MODEL; i++){
		CleanupModel(i);
	}
	for(int i=0; i<MAX_TEXTURE; i++){
		CleanupTexture(i);
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

	//Find vacant elements
	for(int i=0; i<MAX_MODEL; i++){
		if( pmesh[i] == NULL ){
			id = i;
			break;
		}
	}
	if( id == -1 ){ return -1; }

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	filename = ChangePathDelimiter(filename);
#endif

	//Load the .x file into the buffer
	if( FAILED( D3DXLoadMeshFromX( filename, D3DXMESH_SYSTEMMEM, pd3dDevice, NULL, 
				NULL, NULL, &nummaterials[id], &pmesh[id] ) ) ) {
		return -1;
	}

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
	if( pmesh[idA] == NULL ){ return -1; }
	if( (idB < 0)||(MAX_MODEL <= idB) ){ return -1; }
	if( pmesh[idB] == NULL ){ return -1; }

	int idN = -1;
	int numvA, numvB;
	LPDIRECT3DVERTEXBUFFER9 pvbA, pvbB, pvbN;
	D3DXVECTOR3 *pVerticesA, *pVerticesB, *pVerticesN;
	int FVFsize;

	//Find vacant elements
	for(int i=0; i<MAX_MODEL; i++){
		if( pmesh[i] == NULL ){
			idN = i;
			break;
		}
	}
	if( idN == -1 ){ return -1; }

	//Get the vertex count.
	numvA = pmesh[idA]->GetNumVertices();
	numvB = pmesh[idB]->GetNumVertices();

	//Check if the number of vertex is the same
	if( numvA != numvB ){ return -1; }

	//Copy vertex data (dummy for virtually securing area)
	if( pmesh[idA]->CloneMeshFVF(pmesh[idA]->GetOptions(), pmesh[idA]->GetFVF(), pd3dDevice, &pmesh[idN]) != D3D_OK ){
		return -1;
	}

	//Copy material information
	nummaterials[idN] = nummaterials[idA];

	//Get buffer
	pmesh[idA]->GetVertexBuffer(&pvbA);
	pmesh[idB]->GetVertexBuffer(&pvbB);
	pmesh[idN]->GetVertexBuffer(&pvbN);

	//Get the number of bytes per vertice
	FVFsize = D3DXGetFVFVertexSize(pmesh[idN]->GetFVF());

	//Read and calculate each vertex.
	for(int i=0; i<numvA; i++){
		pvbA->Lock(i*FVFsize, sizeof(D3DXVECTOR3), (void**)&pVerticesA, D3DLOCK_READONLY);
		pvbB->Lock(i*FVFsize, sizeof(D3DXVECTOR3), (void**)&pVerticesB, D3DLOCK_READONLY);
		pvbN->Lock(i*FVFsize, sizeof(D3DXVECTOR3), (void**)&pVerticesN, 0);

		//Average the values.
		pVerticesN->x = (pVerticesA->x + pVerticesB->x)/2;
		pVerticesN->y = (pVerticesA->y + pVerticesB->y)/2;
		pVerticesN->z = (pVerticesA->z + pVerticesB->z)/2;

		pvbA->Unlock();
		pvbB->Unlock();
		pvbN->Unlock();
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_COMPLETE, "", idN);
#endif
	return idN;
}

//! @brief Get the number of loaded models
//! @return Number of models.
int D3DGraphics::GetTotalModels()
{
	int cnt = 0;

	//Counting elements in use
	for(int i=0; i<MAX_MODEL; i++){
		if( pmesh[i] != NULL ){ cnt += 1; }
	}

	return cnt;
}

//! @brief Release model files
//! @param id Model Data Number
void D3DGraphics::CleanupModel(int id)
{
	if( (id < 0)||(MAX_MODEL <= id) ){ return; }
	if( pmesh[id] != NULL ){
		pmesh[id]->Release();
		pmesh[id] = NULL;

#ifdef ENABLE_DEBUGLOG
		//Write to the log
		OutputLog.WriteLog(LOG_CLEANUP, "Model", id);
#endif
	}
}

//! @brief Read Texture
//! @param filename File Name
//! @param texturefont Texture Font Flag
//! @param BlackTransparent Make black pixels transparent.
//! @return Success: texture data ID (0 or greater); failure: -1.
int D3DGraphics::LoadTexture(const char* filename, bool texturefont, bool BlackTransparent)
{
	if( filename == NULL ){ return -1; }

	int id = -1;
	D3DXIMAGE_INFO info;
	int MipLevels;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "Texture", filename);
#endif

	//Finding an empty data number
	for(int i=0; i<MAX_TEXTURE; i++){
		if( ptextures[i] == NULL ){
			id = i;
			break;
		}
	}
	if( id == -1 ){ return -1; }

#ifdef ENABLE_PATH_DELIMITER_SLASH
	//Convert path separators.
	filename = ChangePathDelimiter(filename);
#endif

	//Get File Information
	if( D3DXGetImageInfoFromFile(filename, &info) != D3D_OK ){ return -1; }

	//Set the MIP Map Level
	if( texturefont == true ){
		MipLevels = 1;
	}
	else{
		MipLevels = 4;//D3DX_DEFAULT;
	}

	//Read Texture
	if( BlackTransparent == false ){
		if( FAILED( D3DXCreateTextureFromFileEx(pd3dDevice, filename, info.Width, info.Height, MipLevels, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0x00000000, NULL, NULL, &ptextures[id]) ) ) {
			return -1;
		}
	}
	else{
		if( FAILED( D3DXCreateTextureFromFileEx(pd3dDevice, filename, info.Width, info.Height, MipLevels, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, D3DCOLOR_ARGB(255, 0, 0, 0), NULL, NULL, &ptextures[id]) ) ) {
			return -1;
		}
	}

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	//OutputLog.WriteLog(LOG_COMPLETE, "", id);

	char str[32];
	int width, height;
	GetTextureSize(id, &width, &height);
	sprintf(str, "IDF%d (size:%dx%d)", id, width, height);
	OutputLog.WriteLog(LOG_COMPLETE, "", str);
#endif
	return id;
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
	int headersize = 54;
	int bufsize = headersize  + width*height*3;
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
		if( ptextures[i] == NULL ){
			id = i;
			break;
		}
	}
	if( id == -1 ){ return false; }

	//.bmp deployment area creation
	unsigned char *bmpdata = new unsigned char [bufsize];

	//.bmp header creation
	for(int i=0; i<headersize; i++){ bmpdata[i] = 0x00; }
	bmpdata[0x00] = 'B';
	bmpdata[0x01] = 'M';
	Set4ByteLittleEndian(&(bmpdata[0x02]), bufsize);
	bmpdata[0x0A] = (unsigned char)headersize;
	bmpdata[0x0E] = (unsigned char)(headersize - 14);
	Set4ByteLittleEndian(&(bmpdata[0x12]), width);
	Set4ByteLittleEndian(&(bmpdata[0x16]), height);
	bmpdata[0x1A] = 1;
	bmpdata[0x1C] = 24;
	bmpdata[0x1E] = 0;
	Set4ByteLittleEndian(&(bmpdata[0x22]), bufsize - headersize);

	datacnt = headersize;

	//Create data for 6 lines
	for(int cnt_y=5; cnt_y>=0; cnt_y--){
		for(int line_y=(charheight-1); line_y>=0; line_y--){
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
						bmpdata[datacnt + 0] = 255;
						bmpdata[datacnt + 1] = 255;
						bmpdata[datacnt + 2] = 255;
					}
					else{
						bmpdata[datacnt + 0] = 0;
						bmpdata[datacnt + 1] = 0;
						bmpdata[datacnt + 2] = 0;
					}

					datacnt += 3;
				}
			}
		}
	}

	//2 lines on the control code 32 characters are empty.
	for(int cnt_y=0; cnt_y<2; cnt_y++){
		for(int line_y=(charheight-1); line_y>=0; line_y--){
			for(int cnt_x=0; cnt_x<16; cnt_x++){
				for(int line_x=0; line_x<charwidth; line_x++){
					bmpdata[datacnt + 0] = 0;
					bmpdata[datacnt + 1] = 0;
					bmpdata[datacnt + 2] = 0;

					datacnt += 3;
				}
			}
		}
	}

	//Load as .bmp texture
	if( FAILED( D3DXCreateTextureFromFileInMemoryEx(pd3dDevice, bmpdata, bufsize, width, height, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, D3DCOLOR_ARGB(255, 0, 0, 0), NULL, NULL, &ptextures[id]) ) ) {
		TextureDebugFont = -1;
		return false;
	}

	/*
	//Save as .bmp
	FILE *fp;
	fp = fopen("debugfont-test.bmp", "wb");
	fwrite(bmpdata, sizeof(unsigned char), bufsize, fp);
	fclose(fp);
	*/

	//.bmp deployment area release
	delete [] bmpdata;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	//OutputLog.WriteLog(LOG_COMPLETE, "", id);

	char str[32];
	sprintf(str, "IDF%d (size:%dx%d)", id, width, height);
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
	if( ptextures[id] == NULL ){ return 1; }
	if( width == NULL ){ return 1; }
	if( height == NULL ){ return 1; }

	IDirect3DSurface9 *surface;
	D3DSURFACE_DESC desc;

	//Get the surface.
	ptextures[id]->GetSurfaceLevel(0, &surface);

	//Get width and height
	surface->GetDesc(&desc);
	*width = desc.Width;
	*height = desc.Height;

	//Release the face
	surface->Release();

	return 0;
}

//! @brief Get the number of read textures
//! @return Number of textures
int D3DGraphics::GetTotalTextures()
{
	int cnt = 0;

	//Counting elements in use
	for(int i=0; i<MAX_TEXTURE; i++){
		if( ptextures[i] != NULL ){ cnt += 1; }
	}

	return cnt;
}

//! @brief Free Texture
//! @param id Texture Data Number
void D3DGraphics::CleanupTexture(int id)
{
	if( (id < 0)||(MAX_TEXTURE <= id) ){ return; }
	if( ptextures[id] != NULL ){
		ptextures[id]->Release();
		ptextures[id] = NULL;

#ifdef ENABLE_DEBUGLOG
		//Write to the log
		OutputLog.WriteLog(LOG_CLEANUP, "Texture", id);
#endif
	}
}

//! @brief Start drawing process
//! @return Success:0 failure:1
//! @attention You must first call the drawing process.
int D3DGraphics::StartRender()
{
	if( StartRenderFlag == true ){ return 1; }

	//Initialize the area
	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);

	if( SUCCEEDED( pd3dDevice->BeginScene() ) ){
		//Z buffer initialization
		pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

#if GRAPHIC_ENGINE == 1
		//vertex shader and pixel shader setting
		pd3dDevice->SetVertexShader(hVertexShader);
		pd3dDevice->SetPixelShader(pPixelShader);
#endif

		//World transformation matrix to zero coordinates
		ResetWorldTransform();

		//Set the rendering-in-progress flag.
		StartRenderFlag = true;
		return 0;
	}

	return 1;
}

//! @brief Ends all drawing processing
//! @return Success:false Fail:true
//! @attention It is necessary to call at the end of the drawing process.
bool D3DGraphics::EndRender()
{
	//Ends when drawing
	if( StartRenderFlag == true ){
		pd3dDevice->EndScene();
	}

	HRESULT hr = pd3dDevice->Present(NULL, NULL, NULL, NULL);

	//flag to false
	StartRenderFlag = false;

	if( hr == D3DERR_DEVICELOST ){
		return true;
	}
	return false;
}

//! @brief Reset Z buffer
void D3DGraphics::ResetZbuffer()
{
	//Disable Z buffer once, and re-enable after initialization
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
}

//! @brief Revert the world space to its origin (0,0,0), etc.
void D3DGraphics::ResetWorldTransform()
{
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
#else
	hVSConstantTable->SetMatrix(pd3dDevice, hVSWorldVertexPos, &matWorld);
	VSWorldPosMatrix = matWorld;
#endif
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
	D3DXMATRIX matWorld;
	D3DXMATRIX matWorld1, matWorld2, matWorld3, matWorld4, matWorld5;

	//Create a matrix
	D3DXMatrixTranslation(&matWorld1, x, y, z);
	D3DXMatrixRotationY(&matWorld2, rx);
	D3DXMatrixRotationX(&matWorld3, ry1);
	D3DXMatrixRotationZ(&matWorld4, ry2);
	D3DXMatrixScaling(&matWorld5, size, size, size);

	//Calculate.
	matWorld = matWorld5 * matWorld4 * matWorld3 * matWorld2 * matWorld1;

	//Application
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
#else
	hVSConstantTable->SetMatrix(pd3dDevice, hVSWorldVertexPos, &matWorld);
	VSWorldPosMatrix = matWorld;
#endif
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
	D3DXMATRIX matWorld;
	D3DXMATRIX matWorld1, matWorld2, matWorld3, matWorld4, matWorld5;

	//Create a matrix
	D3DXMatrixTranslation(&matWorld1, x, y, z);
	D3DXMatrixRotationY(&matWorld2, rx);
	D3DXMatrixRotationZ(&matWorld3, ry);
	D3DXMatrixRotationX(&matWorld4, rt);
	D3DXMatrixScaling(&matWorld5, size, size, size);

	//Calculate.
	matWorld = matWorld5 * matWorld4 * matWorld3 * matWorld2 * matWorld1;

	//Application
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
#else
	hVSConstantTable->SetMatrix(pd3dDevice, hVSWorldVertexPos, &matWorld);
	VSWorldPosMatrix = matWorld;
#endif
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
	D3DXMATRIX matWorld;
	D3DXMATRIX matWorld1, matWorld2, matWorld3, matWorld4, matWorld5;

	//Create a matrix
	D3DXMatrixTranslation(&matWorld1, x, y, z);
	D3DXMatrixRotationY(&matWorld2, rx);
	D3DXMatrixRotationX(&matWorld3, ry);
	D3DXMatrixTranslation(&matWorld4, mx, my, mz);
	D3DXMatrixScaling(&matWorld5, size, size, size);

	//Calculate.
	matWorld = matWorld5 * matWorld4 * matWorld3 * matWorld2 * matWorld1;

	//Application
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
#else
	hVSConstantTable->SetMatrix(pd3dDevice, hVSWorldVertexPos, &matWorld);
	VSWorldPosMatrix = matWorld;
#endif
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
	D3DXMATRIX matWorld;
	D3DXMATRIX matWorldV;
	D3DXMATRIXA16 matProj;
	D3DVIEWPORT9 pViewport;
	D3DXVECTOR3 p1;
	D3DXMATRIX matWorld1, matWorld2, matWorld3, matWorld4, matWorld5, matWorld6;

	if( rotation == true ){
		p1 = D3DXVECTOR3(HUD_myweapon_x[0], HUD_myweapon_y[0], HUD_myweapon_z[0]);
	}
	else{
		p1 = D3DXVECTOR3(HUD_myweapon_x[1], HUD_myweapon_y[1], HUD_myweapon_z[1]);
	}

	pd3dDevice->GetViewport(&pViewport);

#if GRAPHIC_ENGINE == 0
	//Camera Coordinate
	pd3dDevice->GetTransform(D3DTS_VIEW, &matWorldV);

	//Camera setting (shadow conversion matrix) viewangle
	pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);
#else
	//Camera Coordinate
	matWorldV = VSViewPosMatrix;

	//Camera setting (shadow conversion matrix) viewangle
	matProj = VSViewProjMatrix;
#endif

	D3DXMatrixIdentity(&matWorld);

	//Project the vector from screen space into object space.
	D3DXVec3Unproject(&p1, &p1, &pViewport, &matProj, &matWorldV, &matWorld);

	//size = size * 0.3f;
	size = size * (0.0004f*GameConfig.GetScreenHeight()*GameConfig.GetScreenHeight() - 0.92f*GameConfig.GetScreenHeight() + 650.0f) / 1000.f;

	//Calculate the matrix.
	D3DXMatrixTranslation(&matWorld1, p1.x, p1.y, p1.z);
	D3DXMatrixRotationY(&matWorld2, camera_rx *-1);
	D3DXMatrixRotationZ(&matWorld3, camera_ry);
	if( rotation == true ){
		D3DXMatrixRotationY(&matWorld5, rx);
	}
	else{
		D3DXMatrixRotationY(&matWorld5, D3DX_PI);
	}
	D3DXMatrixScaling(&matWorld6, size, size, size);

	matWorld = matWorld6 * matWorld5 * matWorld3 * matWorld2 * matWorld1;

	//Application
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
#else
	hVSConstantTable->SetMatrix(pd3dDevice, hVSWorldVertexPos, &matWorld);
	VSWorldPosMatrix = matWorld;
#endif
}

//! @brief Get coordinates in world space.
//! @param *x Pointer to receive x-axis
//! @param *y Pointer to receive y axis
//! @param *z Pointer to receive z-axis
void D3DGraphics::GetWorldTransformPos(float *x, float *y, float *z)
{
	if( (x == NULL)||(y == NULL)||(z == NULL) ){ return; }

	D3DXMATRIX matWorld;
#if GRAPHIC_ENGINE == 0
	pd3dDevice->GetTransform(D3DTS_WORLD, &matWorld);
#else
	matWorld = VSWorldPosMatrix;
#endif
	*x = matWorld._41;
	*y = matWorld._42;
	*z = matWorld._43;
}

//! @brief Set the fog
//! @param enablefog Fog Activation
//! @param skynumber Sky ID.
void D3DGraphics::SetFog(bool enablefog, int skynumber)
{
	if( enablefog == true ){
		if( EnableFogFlag == false ){
			//Fog
			float fog_st = 100;
			float fog_end = 800;
#if GRAPHIC_ENGINE == 0
			pd3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_RGBA(0, 0, 0, 0));
			pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
			pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
			pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fog_st));
			pd3dDevice->SetRenderState(D3DRS_FOGEND,   *(DWORD*)(&fog_end));
#else
			float FogColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			pPSConstantTable->SetBool(pd3dDevice, hPSEnableFog, true);
			pPSConstantTable->SetFloatArray(pd3dDevice, hPSFogColor, FogColor, 4);
			pPSConstantTable->SetFloat(pd3dDevice, hPSFogStart, fog_st);
			pPSConstantTable->SetFloat(pd3dDevice, hPSFogEnd, fog_end);
#endif
		}

		D3DCOLOR skycolor;

		//Color is determined by the empty number.
		switch(skynumber){
			case 1: skycolor = D3DCOLOR_RGBA(64, 64+16, 64, 0); break;
			case 2: skycolor = D3DCOLOR_RGBA(16, 16, 16, 0); break;
			case 3: skycolor = D3DCOLOR_RGBA(0, 16, 32, 0); break;
			case 4: skycolor = D3DCOLOR_RGBA(32, 16, 16, 0); break;
			case 5: skycolor = D3DCOLOR_RGBA(64, 32, 32, 0); break;
			default: skycolor = D3DCOLOR_RGBA(0, 0, 0, 0); break;
		}

		//Set the fog
#if GRAPHIC_ENGINE == 0
		pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, skycolor);
#else
		float FogColor[4] = {(float)((skycolor>>16)&0xFF)/255.0f, (float)((skycolor>>8)&0xFF)/255.0f, (float)(skycolor&0xFF)/255.0f, 1.0f};
		pPSConstantTable->SetFloatArray(pd3dDevice, hPSFogColor, FogColor, 4);
#endif

		EnableFogFlag = true;
	}
	else{
#if GRAPHIC_ENGINE == 0
		pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
#else
		pPSConstantTable->SetBool(pd3dDevice, hPSEnableFog, false);
#endif
		EnableFogFlag = false;
	}
}

//! @brief Set the camera (point of view)
//! @param camera_x Camera X-coordinate
//! @param camera_y Camera Y coordinates
//! @param camera_z Camera Z coordinates
//! @param camera_rx Camera horizontal axis angle
//! @param camera_ry Camera vertical axis angle
//! @param viewangle Viewing Angle
//! @todo If you turn right/right, the model drawing of your own weapon will be more likely.
void D3DGraphics::SetCamera(float camera_x, float camera_y, float camera_z, float camera_rx, float camera_ry, float viewangle)
{
	float vUpVecF_x, vUpVecF_y, vUpVecF_z;
	D3DXMATRIX matWorld;
	D3DXMATRIXA16 matView;

	//Normalize camera_ry to the range -PI to PI.
	camera_ry = AngleNormalization(camera_ry);

	//Determines the orientation of the camera
	if( fabsf(camera_ry) == D3DX_PI/2 ){
		//If the camera is facing up or downward, the top of the camera is lateralized.
		vUpVecF_x = cosf(camera_rx); vUpVecF_y = 0.0f; vUpVecF_z = sinf(camera_rx);
	}
	else{
		if( fabsf(camera_ry) < D3DX_PI/2 ){
			vUpVecF_x = 0.0f; vUpVecF_y = 1.0f; vUpVecF_z = 0.0f;
		}
		else{
			vUpVecF_x = 0.0f; vUpVecF_y = -1.0f; vUpVecF_z = 0.0f;
		}
	}

	D3DXMatrixIdentity(&matWorld);
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
#else
	hVSConstantTable->SetMatrix(pd3dDevice, hVSWorldVertexPos, &matWorld);
	VSWorldPosMatrix = matWorld;
#endif

	//Camera Coordinate
	D3DXVECTOR3 vEyePt( camera_x, camera_y, camera_z );
	D3DXVECTOR3 vLookatPt( cosf(camera_rx)*cosf(camera_ry) + camera_x, sinf(camera_ry) + camera_y, sinf(camera_rx)*cosf(camera_ry) + camera_z );
	D3DXVECTOR3 vUpVec( vUpVecF_x, vUpVecF_y, vUpVecF_z );
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
#else
	hVSConstantTable->SetMatrix(pd3dDevice, hVSWorldViewPos, &matView);
	VSViewPosMatrix = matView;

	if( hPSViewPos != NULL ){
		float Camera[4] = {camera_x, camera_y, camera_z, 1.0f};
		pPSConstantTable->SetFloatArray(pd3dDevice, hPSViewPos, Camera, 4);
	}
#endif

	//Camera setting (shadow conversion matrix) viewangle
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, viewangle, aspect, CLIPPINGPLANE_NEAR, CLIPPINGPLANE_FAR);
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
#else
	hVSConstantTable->SetMatrix(pd3dDevice, hVSWorldViewProj, &matProj);
	VSViewProjMatrix = matProj;
#endif
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
	//int bs;
	blockdata data;
	int vID[4];
	int uvID[4];

	//Set Class
	BlockDataIF = in_BlockDataIF;

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

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_LOAD, "Mapdata", "(Vertex data)");
#endif

#ifdef BLOCKDATA_GPUMEMORY
	VERTEXTXTA* pVertices;

	//Get block count
	bs = BlockDataIF->GetTotaldatas();

	//Create a block minutes buffer
	pd3dDevice->CreateVertexBuffer(bs*6*4*sizeof(VERTEXTXTA), 0, D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1, D3DPOOL_DEFAULT, &g_pVB, NULL);

	for(int i=0; i<bs; i++){
		//Get Data
		BlockDataIF->Getdata(&data, i);

		for(int j=0; j<6; j++){
			//es the association of the vertex data on the surface
			blockdataface(j, &vID[0], &uvID[0]);

			//Lock the GPU buffer for one face.
			g_pVB->Lock((i*6+j)*4*sizeof(VERTEXTXTA), 4*sizeof(VERTEXTXTA), (void**)&pVertices, 0);

			//Set vertex coordinates, UV coordinates and colors
			pVertices[0].position = D3DXVECTOR3( data.x[ vID[1] ], data.y[ vID[1] ], data.z[ vID[1] ] );
			pVertices[0].tu       = data.material[j].u[ uvID[1] ];
			pVertices[0].tv       = data.material[j].v[ uvID[1] ];
			pVertices[1].position = D3DXVECTOR3( data.x[ vID[2] ], data.y[ vID[2] ], data.z[ vID[2] ] );
			pVertices[1].tu       = data.material[j].u[ uvID[2] ];
			pVertices[1].tv       = data.material[j].v[ uvID[2] ];
			pVertices[2].position = D3DXVECTOR3( data.x[ vID[0] ], data.y[ vID[0] ], data.z[ vID[0] ] );
			pVertices[2].tu       = data.material[j].u[ uvID[0] ];
			pVertices[2].tv       = data.material[j].v[ uvID[0] ];
			pVertices[3].position = D3DXVECTOR3( data.x[ vID[3] ], data.y[ vID[3] ], data.z[ vID[3] ] );
			pVertices[3].tu       = data.material[j].u[ uvID[3] ];
			pVertices[3].tv       = data.material[j].v[ uvID[3] ];
			for(int k=0; k<4; k++){
				pVertices[k].color = D3DCOLOR_COLORVALUE(data.material[j].shadow, data.material[j].shadow, data.material[j].shadow, 1.0f);
			}

			//Unlock the GPU buffer.
			g_pVB->Unlock();
		}
	}
#else
	//Get block count
	bs = BlockDataIF->GetTotaldatas();

	for(int i=0; i<bs; i++){
		//Get Data
		BlockDataIF->Getdata(&data, i);

		for(int j=0; j<6; j++){
			//es the association of the vertex data on the surface
			blockdataface(j, vID, uvID);

			//Set vertex coordinates, UV coordinates and colors
			g_pVertices[i][j][0].position = D3DXVECTOR3( data.x[ vID[1] ], data.y[ vID[1] ], data.z[ vID[1] ] );
			g_pVertices[i][j][0].tu       = data.material[j].u[ uvID[1] ];
			g_pVertices[i][j][0].tv       = data.material[j].v[ uvID[1] ];
			g_pVertices[i][j][1].position = D3DXVECTOR3( data.x[ vID[2] ], data.y[ vID[2] ], data.z[ vID[2] ] );
			g_pVertices[i][j][1].tu       = data.material[j].u[ uvID[2] ];
			g_pVertices[i][j][1].tv       = data.material[j].v[ uvID[2] ];
			g_pVertices[i][j][2].position = D3DXVECTOR3( data.x[ vID[0] ], data.y[ vID[0] ], data.z[ vID[0] ] );
			g_pVertices[i][j][2].tu       = data.material[j].u[ uvID[0] ];
			g_pVertices[i][j][2].tv       = data.material[j].v[ uvID[0] ];
			g_pVertices[i][j][3].position = D3DXVECTOR3( data.x[ vID[3] ], data.y[ vID[3] ], data.z[ vID[3] ] );
			g_pVertices[i][j][3].tu       = data.material[j].u[ uvID[3] ];
			g_pVertices[i][j][3].tv       = data.material[j].v[ uvID[3] ];
			for(int k=0; k<4; k++){
				g_pVertices[i][j][k].color = D3DCOLOR_COLORVALUE(data.material[j].shadow, data.material[j].shadow, data.material[j].shadow, 1.0f);
			}
		}
	}
#endif

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	//OutputLog.WriteLog(LOG_COMPLETE, "", id);

	char str[32];
	sprintf(str, "Blocks:%d, Vertices:%d", bs, bs*6*4);
	OutputLog.WriteLog(LOG_COMPLETE, "", str);
#endif
}

//! @brief Print data
//! @param wireframe Wire Frame Display
void D3DGraphics::RenderMapdata(bool wireframe)
{
	//If the block data is not loaded, it will not be processed.
	if( BlockDataIF == NULL ){ return; }

	blockdata data;
	int textureID;

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

	//Set deep buffer comparison function
	//pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);


#ifdef BLOCKDATA_GPUMEMORY
	//Data Settings
	pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(VERTEXTXTA));

	for(textureID=0; textureID<TOTAL_BLOCKTEXTURE; textureID++){
		//Set if the texture is not loaded correctly
#if GRAPHIC_ENGINE == 0
		if( mapTextureID[textureID] == -1 ){
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
			pd3dDevice->SetTexture(0, NULL);
		}
		else if( ptextures[ mapTextureID[textureID] ] == NULL ){
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
			pd3dDevice->SetTexture(0, NULL);
		}
		else{
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
			pd3dDevice->SetTexture(0, ptextures[mapTextureID[textureID]] );
		}
#else
		if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, false); }
		if( mapTextureID[textureID] == -1 ){
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
			pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
			pd3dDevice->SetTexture(0, NULL);
		}
		else if( ptextures[ mapTextureID[textureID] ] == NULL ){
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
			pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
			pd3dDevice->SetTexture(0, NULL);
		}
		else{
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
			pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, true);
			pd3dDevice->SetTexture(0, ptextures[mapTextureID[textureID]] );
		}
#endif

		for(int i=0; i<bs; i++){
			//Data Acquisition
			BlockDataIF->Getdata(&data, i);

			for(int j=0; j<6; j++){
				//Get Texture Data Number
				int ID = data.material[j].textureID;

				if( textureID == ID ){
					//Draw a face
					pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, (i*6+j)*4, 2);
				}
			}
		}
	}
#else
	//Configure Data
	pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);

	for(textureID=0; textureID<TOTAL_BLOCKTEXTURE; textureID++){
		//Set if the texture is not loaded correctly
#if GRAPHIC_ENGINE == 0
		if( mapTextureID[textureID] == -1 ){
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
			pd3dDevice->SetTexture(0, NULL);
		}
		else if( ptextures[ mapTextureID[textureID] ] == NULL ){
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
			pd3dDevice->SetTexture(0, NULL);
		}
		else{
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
			pd3dDevice->SetTexture(0, ptextures[mapTextureID[textureID]] );
		}
#else
		if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, false); }
		if( mapTextureID[textureID] == -1 ){
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
			pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
			pd3dDevice->SetTexture(0, NULL);
		}
		else if( ptextures[ mapTextureID[textureID] ] == NULL ){
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
			pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
			pd3dDevice->SetTexture(0, NULL);
		}
		else{
			pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
			pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, true);
			pd3dDevice->SetTexture(0, ptextures[mapTextureID[textureID]] );
		}
#endif

		for(int i=0; i<bs; i++){
			//Data Acquisition
			BlockDataIF->Getdata(&data, i);

			for(int j=0; j<6; j++){
				//Get Texture Data Number
				int ID = data.material[j].textureID;

				if( textureID == ID ){
					//Draw a face
					pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_pVertices[i][j], sizeof(VERTEXTXTA));
				}
			}
		}
	}
#endif

	//Undo deep buffer comparison function
	//pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
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
#ifdef BLOCKDATA_GPUMEMORY
	if( g_pVB == NULL ){ return; }
#endif
	if( (bs == 0)&&(BlockDataIF == NULL) ){ return; }

	//Free Texture
	for(int i=0; i<TOTAL_BLOCKTEXTURE; i++){
		CleanupTexture(mapTextureID[i]);
	}

#ifdef BLOCKDATA_GPUMEMORY
	//Vertex data release
	if( g_pVB != NULL ){
		g_pVB->Release();
		g_pVB = NULL;
	}
#endif
	bs = 0;

	BlockDataIF = NULL;

#ifdef ENABLE_DEBUGLOG
	//Write to the log
	OutputLog.WriteLog(LOG_CLEANUP, "Mapdata", "iVertex dataj");
#endif
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
	//if( id_texture == -1 ){ return; }

	//Failed if the specified model is not initialized
	if( pmesh[id_model] == NULL ){ return; }

	if( nomodel == true ){
		Renderline(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, GetColorCode(1.0f,0.5f,0.0f,1.0f));
		Renderline(0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, GetColorCode(1.0f,0.5f,0.0f,1.0f));
		Renderline(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, GetColorCode(1.0f,0.5f,0.0f,1.0f));
		return;
	}

	float Brightness;

	if( darkflag == false ){
		Brightness = 1.0f;
	}
	else{
		Brightness = 0.8f;
	}

	//ate writing
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	//Tint setting
#if GRAPHIC_ENGINE == 0
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(mtrl));
	mtrl.Emissive = D3DXCOLOR(Brightness, Brightness, Brightness, 1.0f);
	pd3dDevice->SetMaterial(&mtrl);
#else
	float DiffuseColor[4] = {Brightness, Brightness, Brightness, 1.0f};
	hVSConstantTable->SetFloatArray(pd3dDevice, hVSDiffuse, DiffuseColor, 4);
#endif

	//Texture Settings
#if GRAPHIC_ENGINE == 0
	if( id_texture == -1 ){
		pd3dDevice->SetTexture(0, NULL);
	}
	else if( ptextures[id_texture] == NULL ){
		pd3dDevice->SetTexture(0, NULL);
	}
	else{
		pd3dDevice->SetTexture( 0, ptextures[id_texture] );
	}
#else
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, false); }
	if( id_texture == -1 ){
		pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
		pd3dDevice->SetTexture(0, NULL);
	}
	else if( ptextures[id_texture] == NULL ){
		pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
		pd3dDevice->SetTexture(0, NULL);
	}
	else{
		pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, true);
		pd3dDevice->SetTexture( 0, ptextures[id_texture] );
	}
#endif

	//Render
	for(int i=0; i<(signed)nummaterials[id_model]; i=i+1){
		pmesh[id_model]->DrawSubset(i);
	}

#if GRAPHIC_ENGINE == 1
	//Tint Resetting
	float DiffuseColor2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	hVSConstantTable->SetFloatArray(pd3dDevice, hVSDiffuse, DiffuseColor2, 4);
#endif

	//Disable Writing
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}

//! @brief Drawing Board
//! @param id_texture Texture Data Number
//! @param alpha Transparency (0.0 to 1.0 0.0: fully transparent)
//! @param nomodel Disable Board Drawing
void D3DGraphics::RenderBoard(int id_texture, float alpha, bool nomodel)
{
	//If the texture is not set, it will not be processed.
	if( id_texture == -1 ){ return; }

	if( nomodel == true ){
		Renderline(0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, GetColorCode(0.75f,0.75f,0.75f,1.0f));
		Renderline(0.0f, -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, GetColorCode(0.75f,0.75f,0.75f,1.0f));
		Renderline(0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, GetColorCode(0.75f,0.75f,0.75f,1.0f));
		Renderline(0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, GetColorCode(0.75f,0.75f,0.75f,1.0f));
		return;
	}

	VERTEXTXTA BoardVertices[4];

	//Set vertex coordinates, UV coordinates, color/transparency
	BoardVertices[0].position = D3DXVECTOR3(0.0f, 0.5f, -0.5f);
	BoardVertices[0].tu       = 1.0f;
	BoardVertices[0].tv       = 0.0f;
	BoardVertices[1].position = D3DXVECTOR3(0.0f, -0.5f, -0.5f);
	BoardVertices[1].tu       = 1.0f;
	BoardVertices[1].tv       = 1.0f;
	BoardVertices[2].position = D3DXVECTOR3(0.0f, 0.5f, 0.5f);
	BoardVertices[2].tu       = 0.0f;
	BoardVertices[2].tv       = 0.0f;
	BoardVertices[3].position = D3DXVECTOR3(0.0f, -0.5f, 0.5f);
	BoardVertices[3].tu       = 0.0f;
	BoardVertices[3].tv       = 1.0f;
	for(int i=0; i<4; i++){
		BoardVertices[i].color = D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, alpha);
	}

	//Set Alpha Blend
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//Set and draw textures and data formats
#if GRAPHIC_ENGINE == 1
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, false); }
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, true);
#endif
	pd3dDevice->SetTexture(0, ptextures[id_texture]);
	pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, BoardVertices, sizeof(VERTEXTXTA));

	//Undo Alpha Blend
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
}

//! @brief Settings for Effect Drawing
void D3DGraphics::StartEffectRender()
{
	//Deep buffer comparison function Disable
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
}

//! @brief Unset for effect drawing
void D3DGraphics::EndEffectRender()
{
	//Deep buffer comparison function Enabled
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
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
	Draw2DBox(0, 0, Width, Height, D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,alpha));
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
	VERTEXTXTA mv[2];

	mv[0].position = D3DXVECTOR3(x1, y1, z1);
	mv[1].position = D3DXVECTOR3(x2, y2, z2);
	for(int i=0; i<2; i++){
		mv[i].color = color;
		mv[i].tu = 0.0f;
		mv[i].tv = 0.0f;
	}

#if GRAPHIC_ENGINE == 1
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, false); }
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
#endif
	pd3dDevice->SetTexture(0, NULL);
	pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, mv, sizeof(VERTEXTXTA));
}

//! @brief Start displaying text with 2D system font
//! @attention Initialize DirectX ID3DXSprite.
void D3DGraphics::Start2DMSFontTextRender()
{
	ptextsprite->Begin(D3DXSPRITE_ALPHABLEND);
}

//! @brief Display character (using system font)
//! @param x x Coordinate
//! @param y y coordinates
//! @param str String (Newline code: OK)
//! @param color Color:
//! @warning The <b> display is very low. If you call it many times on the </b> screen, it will affect the performance.
//! @warning Please take measures such as "Display at once using newline code" and "Utilizing texture fonts for non-Japanese characters".
//! @attention Use DirectX ID3DXSprite to display text with the system font.
//! @attention The type and size of the font are fixed. It is difficult to see if the character is double overlapping and does not give up a standing feeling.
void D3DGraphics::Draw2DMSFontText(int x, int y, const char *str, int color)
{
	//if( ptextsprite == NULL ){ return; }
	if( str == NULL ){ return; }

	//Initializing text sprites
	Start2DMSFontTextRender();

	//Set the standard coordinates
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	ptextsprite->SetTransform(&matWorld);

	//Show Letters
	RECT rc = {x, y, 0, 0};
	pxmsfont->DrawText(ptextsprite, str, -1, &rc, DT_NOCLIP, color);

	//Text Sprite Release
	End2DMSFontTextRender();
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
//! @warning The <b> display is very low. If you call it many times on the </b> screen, it will affect the performance.
//! @warning Please take measures such as "Display at once using newline code" and "Utilizing texture fonts for non-Japanese characters".
//! @attention Use DirectX ID3DXSprite to display text with the system font.
//! @attention The type and size of the font are fixed. It is difficult to see if the character is double overlapping and does not give up a standing feeling.
void D3DGraphics::Draw2DMSFontTextCenter(int x, int y, int w, int h, const char *str, int color)
{
	//if( ptextsprite == NULL ){ return; }
	if( str == NULL ){ return; }

	//Initializing text sprites
	Start2DMSFontTextRender();

	//Set the standard coordinates
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	ptextsprite->SetTransform(&matWorld);

	//Show Letters
	RECT rc = {x, y, x+w, y+h};
	pxmsfont->DrawText(ptextsprite, str, -1, &rc, DT_CENTER, color);

	//Text Sprite Release
	End2DMSFontTextRender();
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
void D3DGraphics::Draw2DMSSmallFontText(int x, int y, const char *str, int color)
{
	//if( ptextsprite == NULL ){ return; }
	if( str == NULL ){ return; }

	//Initializing text sprites
	Start2DMSFontTextRender();

	//Set the standard coordinates
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	ptextsprite->SetTransform(&matWorld);

	//Show Letters
	RECT rc = {x, y, 0, 0};
	pxmssmallfont->DrawText(ptextsprite, str, -1, &rc, DT_NOCLIP, color);

	//Text Sprite Release
	End2DMSFontTextRender();
}

//! @brief 2D Ends text display with system font
//! @attention Release DirectX ID3DXSprite.
void D3DGraphics::End2DMSFontTextRender()
{
	ptextsprite->End();
}

//! @brief Setting for 2D drawing
void D3DGraphics::Start2DRender()
{
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//Set deep buffer comparison function
	pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

	//Temporarily disable fog
#if GRAPHIC_ENGINE == 0
	pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
#else
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableFog, false);
#endif
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

//! @brief Display character (using texture font)
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

	//Apply 2D drawing settings
	Start2DRender();

	int w;
	float font_u, font_v;
	float t_u, t_v;
	TLVERTX pBoxVertices[4];

	//1-character UV-coordinate curve
	font_u = 1.0f / 16;
	font_v = 1.0f / 16;

	//Revert the World Coordinate to the Origin
	ResetWorldTransform();

	//Set Texture to Font Texture
#if GRAPHIC_ENGINE == 1
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, true); }
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, true);
#endif
	pd3dDevice->SetTexture( 0, ptextures[TextureFont] );

	//Set the data format
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	// Given character minutes loop
	for(int i=0; i<(int)strlen(str); i++){
		//Calculate UV coordinates.
		w = str[i];
		if( w < 0 ){ w += 256; }
		t_u = (w % 16) * font_u;
		t_v = (w / 16) * font_v;

		//Set vertex coordinates, UV coordinates and colors
		pBoxVertices[0].x = (float)x + i*fontwidth;
		pBoxVertices[0].y = (float)y;
		pBoxVertices[0].tu = t_u;
		pBoxVertices[0].tv = t_v;
		pBoxVertices[1].x = (float)x + fontwidth + i*fontwidth;
		pBoxVertices[1].y = (float)y;
		pBoxVertices[1].tu = t_u + font_u;
		pBoxVertices[1].tv = t_v;
		pBoxVertices[2].x = (float)x + i*fontwidth;
		pBoxVertices[2].y = (float)y + fontheight;
		pBoxVertices[2].tu = t_u;
		pBoxVertices[2].tv = t_v + font_v;
		pBoxVertices[3].x = (float)x + fontwidth + i*fontwidth;
		pBoxVertices[3].y = (float)y + fontheight;
		pBoxVertices[3].tu = t_u + font_u;
		pBoxVertices[3].tv = t_v + font_v;
		for(int j=0; j<4; j++){
			pBoxVertices[j].z = 0.0f;
			pBoxVertices[j].rhw = 1.0f;
			pBoxVertices[j].color = color;
		}

		//Display.
		pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pBoxVertices, sizeof(TLVERTX));
	}

	//Unset for 2D drawing
	End2DRender();
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

	//Apply 2D drawing settings
	Start2DRender();

	// Disable Texture Filter
	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	int w;
	float font_u, font_v;
	float t_u, t_v;
	TLVERTX pBoxVertices[4];

	//1-character UV-coordinate curve
	font_u = 1.0f / 16;
	font_v = 1.0f / 8;

	//Revert the World Coordinate to the Origin
	ResetWorldTransform();

	//Set Texture to Font Texture
#if GRAPHIC_ENGINE == 1
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, true); }
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, true);
#endif
	pd3dDevice->SetTexture( 0, ptextures[TextureDebugFont] );

	//Set the data format
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	// Given character minutes loop
	for(int i=0; i<(int)strlen(str); i++){
		//Calculate UV coordinates.
		w = str[i];
		if( w < 0 ){ w = ' '; }
		t_u = (w % 16) * font_u;
		t_v = (w / 16) * font_v;

		//Set vertex coordinates, UV coordinates and colors
		pBoxVertices[0].x = (float)x + i*fontwidth;
		pBoxVertices[0].y = (float)y;
		pBoxVertices[0].tu = t_u;
		pBoxVertices[0].tv = t_v;
		pBoxVertices[1].x = (float)x + fontwidth + i*fontwidth;
		pBoxVertices[1].y = (float)y;
		pBoxVertices[1].tu = t_u + font_u;
		pBoxVertices[1].tv = t_v;
		pBoxVertices[2].x = (float)x + i*fontwidth;
		pBoxVertices[2].y = (float)y + fontheight;
		pBoxVertices[2].tu = t_u;
		pBoxVertices[2].tv = t_v + font_v;
		pBoxVertices[3].x = (float)x + fontwidth + i*fontwidth;
		pBoxVertices[3].y = (float)y + fontheight;
		pBoxVertices[3].tu = t_u + font_u;
		pBoxVertices[3].tv = t_v + font_v;
		for(int j=0; j<4; j++){
			pBoxVertices[j].z = 0.0f;
			pBoxVertices[j].rhw = 1.0f;
			pBoxVertices[j].color = color;
		}

		//Display.
		pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pBoxVertices, sizeof(TLVERTX));
	}

	//Enable Texture Filters
	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	//Unset for 2D drawing
	End2DRender();
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
	TLVERTX pLineVertices[2];

	//Apply 2D drawing settings
	Start2DRender();

	//Revert the World Coordinate to the Origin
	ResetWorldTransform();

	//Set vertex coordinates and colors
	pLineVertices[0].x = (float)x1;
	pLineVertices[0].y = (float)y1;
	pLineVertices[1].x = (float)x2;
	pLineVertices[1].y = (float)y2;
	for(int i=0; i<2; i++){
		pLineVertices[i].z = 0.0f;
		pLineVertices[i].rhw = 1.0f;
		pLineVertices[i].color = color;
		pLineVertices[i].tu = 0.0f;
		pLineVertices[i].tv = 0.0f;
	}

#if GRAPHIC_ENGINE == 1
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, true); }
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
#endif
	pd3dDevice->SetTexture(0, NULL);

	//Set and draw data format.
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, pLineVertices, sizeof(TLVERTX));

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
	TLVERTX pLineVertices[16+1];

	//Apply 2D drawing settings
	Start2DRender();

	//Revert the World Coordinate to the Origin
	ResetWorldTransform();

	//Set vertex coordinates and colors
	for(int i=0; i<16+1; i++){
		pLineVertices[i].x = (float)x + cosf(DegreeToRadian((360.0f/16.0f)) * i) * r;
		pLineVertices[i].y = (float)y + sinf(DegreeToRadian((360.0f/16.0f)) * i) * r;

		pLineVertices[i].z = 0.0f;
		pLineVertices[i].rhw = 1.0f;
		pLineVertices[i].color = color;
		pLineVertices[i].tu = 0.0f;
		pLineVertices[i].tv = 0.0f;
	}

#if GRAPHIC_ENGINE == 1
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, true); }
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
#endif
	pd3dDevice->SetTexture(0, NULL);

	//Set and draw data format.
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 16, pLineVertices, sizeof(TLVERTX));

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
	TLVERTX pBoxVertices[4];

	//Apply 2D drawing settings
	Start2DRender();

	//Revert the World Coordinate to the Origin
	ResetWorldTransform();

	//Set vertex coordinates and colors
	pBoxVertices[0].x = (float)x1;
	pBoxVertices[0].y = (float)y1;
	pBoxVertices[1].x = (float)x2;
	pBoxVertices[1].y = (float)y1;
	pBoxVertices[2].x = (float)x1;
	pBoxVertices[2].y = (float)y2;
	pBoxVertices[3].x = (float)x2;
	pBoxVertices[3].y = (float)y2;
	for(int i=0; i<4; i++){
		pBoxVertices[i].z = 0.0f;
		pBoxVertices[i].rhw = 1.0f;
		pBoxVertices[i].color = color;
		pBoxVertices[i].tu = 0.0f;
		pBoxVertices[i].tv = 0.0f;
	}

#if GRAPHIC_ENGINE == 1
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, true); }
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, false);
#endif
	pd3dDevice->SetTexture(0, NULL);

	//Set and draw data format.
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pBoxVertices, sizeof(TLVERTX));

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

	TLVERTX pBoxVertices[4];

	//Apply 2D drawing settings
	Start2DRender();

	//Revert the World Coordinate to the Origin
	ResetWorldTransform();

	//Set vertex coordinates, UV coordinates and colors
	pBoxVertices[0].x = (float)x;
	pBoxVertices[0].y = (float)y;
	pBoxVertices[0].tu = 0.0f;
	pBoxVertices[0].tv = 0.0f;
	pBoxVertices[1].x = (float)x + width;
	pBoxVertices[1].y = (float)y;
	pBoxVertices[1].tu = 1.0f;
	pBoxVertices[1].tv = 0.0f;
	pBoxVertices[2].x = (float)x;
	pBoxVertices[2].y = (float)y + height;
	pBoxVertices[2].tu = 0.0f;
	pBoxVertices[2].tv = 1.0f;
	pBoxVertices[3].x = (float)x + width;
	pBoxVertices[3].y = (float)y + height;
	pBoxVertices[3].tu = 1.0f;
	pBoxVertices[3].tv = 1.0f;
	for(int i=0; i<4; i++){
		pBoxVertices[i].z = 0.0f;
		pBoxVertices[i].rhw = 1.0f;
		pBoxVertices[i].color = D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,alpha);
	}

	//Set and draw textures and data formats
#if GRAPHIC_ENGINE == 1
	if( hPSEnable2DRendering != NULL ){ pPSConstantTable->SetBool(pd3dDevice, hPSEnable2DRendering, true); }
	pPSConstantTable->SetBool(pd3dDevice, hPSEnableTexture, true);
#endif
	pd3dDevice->SetTexture( 0, ptextures[id] );
	pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pBoxVertices, sizeof(TLVERTX));

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
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//Undo deep buffer comparison function
	pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	//Revert fog
	if( EnableFogFlag == true ){
#if GRAPHIC_ENGINE == 0
		pd3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
#else
		pPSConstantTable->SetBool(pd3dDevice, hPSEnableFog, true);
#endif
	}
}

//! @brief Save screen s
//! @param filename File Name
//! @return success: true failure: false
bool D3DGraphics::SaveScreenShot(const char* filename)
{
	if( filename == NULL ){ return false; }

	LPDIRECT3DSURFACE9 pSurface = NULL;
	HRESULT hr;

	//Create a surface and capture the screen.
	pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface);

	//Output the surface to the image
	hr = D3DXSaveSurfaceToFile(filename, D3DXIFF_BMP, pSurface, NULL, NULL);

	//Release resources.
	pSurface->Release();

	if( hr == D3D_OK ){
		return true;
	}
	return false;
}

//! @brief Get Color Code
//! @param red Red(0.0f to 1.0f)
//! @param green Green(0.0f to 1.0f)
//! @param blue Blue(0.0f to 1.0f)
//! @param alpha Transparency (0.0f to 1.0f)
//! @return Color Code
int D3DGraphics::GetColorCode(float red, float green, float blue, float alpha)
{
	return D3DCOLOR_COLORVALUE(red, green, blue, alpha);
}

#endif	//GRAPHIC_ENGINE