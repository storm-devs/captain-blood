#ifndef MinigunTrace
#define MinigunTrace

#include "technique.h"
#include "gmxService\gmx_Variables.h"
#include "stdVariables.h"

texture tMinigunTexture;


sampler MinigunTexture =
sampler_state
{
	Texture = <tMinigunTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	
	AddressU = Clamp;
	AddressV = Wrap;
};

struct Minigun_Vertex
{
	float3 pos : POSITION;
	float4 color : COLOR;
	float2 TexCoord : TexCoord0;
};

struct MINIGUN_VS_OUTPUT
{
    float4 pos : POSITION;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct MINIGUN_PS_INPUT
{    
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

MINIGUN_VS_OUTPUT Minigun_VShader(Minigun_Vertex In)
{
 MINIGUN_VS_OUTPUT res;    
    
 res.pos = mul( float4(In.pos, 1.0), mWorldViewProj); 
 
 res.TexCoord.xy = In.TexCoord.xy;
 
 res.Color = In.color;
    
 return res;
}


float4 Minigun_PShader(MINIGUN_PS_INPUT In) : COLOR
{ 
	float4 DifTex = tex2D(MinigunTexture, In.TexCoord.xy);         
      
	return DifTex * In.Color;
}

Technique(Minigun_Trail, Minigun_Vertex)
{
 pass P0
  {	    
	CullMode = none;
  	
	zenable = true;
	zwriteenable = false;

	CullMode = none;

	AlphaTestEnable = false;
	AlphaBlendEnable = true;
	srcblend = one;
	destblend = one;

	VertexShader = compile vs_2_0 Minigun_VShader();
	PixelShader = compile ps_2_0 Minigun_PShader();
  }
}

Technique(Minigun_Bullet, Minigun_Vertex)
{
 pass P0
  {	    
	CullMode = none;
  	
	zenable = true;
	zwriteenable = false;

	CullMode = none;

	AlphaTestEnable = false;
	AlphaBlendEnable = true;
	srcblend = srcalpha;
	destblend = invsrcalpha;

	VertexShader = compile vs_2_0 Minigun_VShader();
	PixelShader = compile ps_2_0 Minigun_PShader();
  }
}


#endif
