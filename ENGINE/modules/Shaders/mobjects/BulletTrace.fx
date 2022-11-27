#include "technique.h"
#ifndef BulletTrace
#define BulletTrace

#include "gmxService\gmx_Variables.h"
#include "stdVariables.h"

float fPower;

texture tBulletTexture;


sampler BulletTexture =
sampler_state
{
	Texture = <tBulletTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	
	AddressU = Clamp;
	AddressV = Clamp;
};

struct Bullet_Vertex
{
	float3 pos : POSITION;
	float4 color : COLOR;
	float2 TexCoord : TexCoord0;
};

struct BLT_VS_OUTPUT
{
    float4 pos : POSITION;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct BLT_PS_INPUT
{    
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

BLT_VS_OUTPUT BLT_VShader(Bullet_Vertex In)
{
	BLT_VS_OUTPUT res;    
    
	res.pos = mul( float4(In.pos, 1.0), mWorldViewProj); 
 	res.TexCoord.xy = In.TexCoord.xy;
 	res.Color.rgb = In.color.rgb * fPower; 
	res.Color.a = In.color.a;
    
	return res;
}


float4 BLT_PShader(BLT_PS_INPUT In) : COLOR
{ 
	float4 DifTex = tex2D(BulletTexture, In.TexCoord.xy);         
      
	return DifTex * In.Color;
}

Technique(Bullet_Trace, Bullet_Vertex)
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

		VertexShader = compile vs_2_0 BLT_VShader();
		PixelShader = compile ps_2_0 BLT_PShader();  
	}
}

Technique(SwordTrailAdd, Bullet_Vertex)
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
		destblend = one;	

		VertexShader = compile vs_2_0 BLT_VShader();
		PixelShader = compile ps_2_0 BLT_PShader();  
	}
}

Technique(SwordTrailMultiply, Bullet_Vertex)
{
	pass P0
	{	    
		CullMode = none;
  	
		zenable = true;
		zwriteenable = false;

		CullMode = none;

		AlphaTestEnable = false;
		AlphaBlendEnable = true;
		srcblend = destcolor;
		destblend = zero;	

		VertexShader = compile vs_2_0 BLT_VShader();
		PixelShader = compile ps_2_0 BLT_PShader();  
	}
}

#endif
