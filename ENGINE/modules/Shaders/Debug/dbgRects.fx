#include "stdVariables.h"


texture RectTexture;
sampler RectSampler =
sampler_state
{
    Texture = <RectTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
};





struct RectVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
 float2 uv : TEXCOORD0;
};



struct Rect_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
};

struct Rect_PS_INPUT
{
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
};



Rect_VS_OUTPUT vs_dbgRects( RectVertex vrx)
{
 Rect_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
 res.color = vrx.color;
 res.uv = vrx.uv;
 return res;
}


float4 ps_dbgRects(Rect_PS_INPUT pnt) : COLOR
{
 float4 tex = tex2D(RectSampler, pnt.uv);

 return tex * pnt.color;
}









Technique(dbgRects, RectVertex)
{
 pass P0
  {
    AlphaBlendEnable = true;
    VertexShader = compile vs_2_0 vs_dbgRects();
    PixelShader = compile ps_2_0 ps_dbgRects();
  }
}


