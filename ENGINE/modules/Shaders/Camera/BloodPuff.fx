#include "technique.h"


texture BloodTexture;
sampler BloodSampler =
sampler_state
{
    Texture = <BloodTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = Clamp;
    AddressV = Clamp;
};



struct BloodVertex
{
 float3 pos   : POSITION;
 float4 color : COLOR;
 float2 uv    : TEXCOORD0;
 float2 data  : TEXCOORD1;
};



struct Blood_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
};

struct Blood_PS_INPUT
{
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
};



Blood_VS_OUTPUT vs_bloodpuff( BloodVertex vrx)
{
 Blood_VS_OUTPUT res;
 
 float cs = cos(vrx.pos.z);
 float sn = sin(vrx.pos.z);
				
 res.pos = float4(cs * vrx.pos.x - sn * vrx.pos.y+vrx.data.x,
				  sn * vrx.pos.x + cs * vrx.pos.y+vrx.data.y,
				  0.0f,1);				
				 
 res.color = vrx.color;
 res.uv = vrx.uv;
 return res;
}

float4 ps_bloodpuff(Blood_PS_INPUT pnt) : COLOR
{
 float4 tex = tex2D(BloodSampler, pnt.uv);
 return tex * pnt.color;
}



Technique(bloodpuff, BloodVertex)
{
 pass P0
  {
    CullMode = none;
    AlphaBlendEnable = true;

    VertexShader = compile vs_2_0 vs_bloodpuff();
    PixelShader = compile ps_2_0 ps_bloodpuff();
  }
}