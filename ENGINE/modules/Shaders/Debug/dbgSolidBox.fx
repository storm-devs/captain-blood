#include "stdVariables.h"



float3 vLightDir;
float3 vScale;
float3 vCenter;
float4 vColor;


struct SolidBoxVertex
{
 float3 pos : POSITION;
 float3 nrm : NORMAL;
};



struct SBox_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
};

struct SBox_PS_INPUT
{
 float4 color : COLOR0;
};



SBox_VS_OUTPUT vs_dbgSolidBox( SolidBoxVertex vrx)
{
 SBox_VS_OUTPUT res;


 float3 v_pos = vrx.pos * vScale;
 v_pos += vCenter;
 res.pos = mul(float4(v_pos, 1), mWorldViewProj);


 float k = saturate(dot(vrx.nrm, vLightDir));
 k += 0.2;


 res.color = float4(k, k, k, 1.0) * vColor;
 return res;
}


float4 ps_dbgSolidBox(SBox_PS_INPUT pnt) : COLOR
{
 return pnt.color;
}









Technique(dbgSolidBox, SolidBoxVertex)
{
 pass P0
  {
    AlphaBlendEnable = true;
    VertexShader = compile vs_2_0 vs_dbgSolidBox();
    PixelShader = compile ps_2_0 ps_dbgSolidBox();
  }
}


