#include "technique.h"
#include "stdVariables.h"

float4 TargPntColor;
texture TargPntTexture;
sampler TargPntSampler =
sampler_state
{
    Texture = <TargPntTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct TargPntVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
 float2 uv : TEXCOORD0;
 float2 uv2 : TEXCOORD1;
};



struct TargPnt_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
 float2 uv2 : TEXCOORD1;
};

struct TargPnt_PS_INPUT
{
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
 float2 uv2 : TEXCOORD1;
};



TargPnt_VS_OUTPUT vs_TargPnts( TargPntVertex vrx)
{
 TargPnt_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
 res.color = vrx.color;
 res.uv = vrx.uv;
 res.uv2 = vrx.uv2;
 return res;
}


float4 ps_TargPnts(TargPnt_PS_INPUT pnt) : COLOR
{
 float4 tex = tex2D(TargPntSampler, pnt.uv);
 // clamp uv:
 float2 uv2 = float2( max(pnt.uv2.x,0.5), pnt.uv2.y );
 float4 tex2 = tex2D(TargPntSampler, uv2);
 
 float kblend = 1.0 - tex.w;
 
 float4 val = tex * TargPntColor + tex2 * pnt.color * float4(kblend,kblend,kblend,1.0);

 return val;
}





Technique(TargetPoints, TargPntVertex)
{
 pass P0
  {
    AlphaRef = 1;
	AlphaTestEnable = true;
	AlphaBlendEnable = true;
	SrcBlend = srcalpha;
	DestBlend = invsrcalpha;
	CullMode = none;
	ZEnable = true;
	ZWriteEnable = true;
    VertexShader = compile vs_2_0 vs_TargPnts();
    PixelShader = compile ps_2_0 ps_TargPnts();
  }
}
