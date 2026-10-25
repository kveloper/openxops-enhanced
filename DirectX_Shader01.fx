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

//Global Variables
float4x4 WorldVertexPos;	//Model coordinates
float4x4 WorldViewPos;		//Camera Coordinate
float4x4 WorldViewProj;		//Transformation matrix
float4   Diffuse;		//Diffuse color.
float4   ViewPos;		//Camera Coordinate (*Un))
bool     Enable2DRendering;	//2D drawing flag (*un)ed)
bool     EnableTexture;		//Texture flag
bool     EnableFog;		//Fog-enabled flag.
float4   FogColor;		//Fog Color
float    FogStart;		//Start depth of fog
float    FogEnd;		//End depth of fog

// Function of vertex shader
void VertexShader_Main(
	in  float4 inPos      : POSITION,	//[Input] Coordinate (model space)
	in  float4 inDiff     : COLOR0,		//[Input] Color
	in  float4 inTexture  : TEXCOORD0,	//[Input] Texture coordinates

	out float4 outPos      : POSITION,	//[Output] Coordinate (shadow space)
	out float4 outDiff     : COLOR0,	//[Output] Color
	out float4 outTexture  : TEXCOORD0,	//[Output] Texture coordinates
	out float4 outPosWorld : TEXCOORD1,	//[Output] Coordinates (World Coordinates)
	out float4 outPosView  : TEXCOORD2)	//[Output] Coordinate (shadow space)
{
	float4 VertexPos = mul(inPos, WorldVertexPos);

	outPos = mul(mul(VertexPos, WorldViewPos), WorldViewProj);	//Hanging the vertex and conversion matrix
	outDiff = max(inDiff, Diffuse);					//Calculate the color.
	outTexture = inTexture;						//Texture coordinates are
	outPosWorld = VertexPos;					//Output world coordinates as it is
	outPosView = outPos;						//Output projection space directly.
}


texture Tex;				//Texture used in pixel shader
sampler TexSampler = sampler_state	//Texture Sampling
{
	Texture   = (Tex);
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

//Pixel Shader
void PixelShader_Main(
	in  float4 inDiff     : COLOR0,		//[Input] Color
	in  float2 inTexture  : TEXCOORD0,	//[Input] Texture coordinates
	in  float4 inWorldPos : TEXCOORD1,	//[Input] World coordinates
	in  float4 inViewPos  : TEXCOORD2,	//[Input] Shading space

	out float4 outDiff    : COLOR0)		//[Output] Color
{
	if( EnableTexture == true ){
		outDiff = inDiff * tex2D(TexSampler, inTexture);
	}
	else{
		outDiff = inDiff;
	}

	//Fog Calculator
	if( EnableFog == true ){
		float fog;
		fog = max(0.0f, (inViewPos.z - FogStart)/(FogEnd - FogStart));
		outDiff.rgb = lerp(outDiff.rgb, FogColor.rgb, fog);			//D3DFOG_LINEAR
		//outDiff.rgb = lerp(outDiff.rgb, FogColor.rgb, pow(fog, 2.0f));	//D3DFOG_EXP2
	}
}
