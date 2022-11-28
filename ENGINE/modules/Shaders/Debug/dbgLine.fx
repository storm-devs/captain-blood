#include "stdVariables.h"



struct LineVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
};



struct Line_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
};

struct Line_PS_INPUT
{
 float4 color : COLOR0;
};



Line_VS_OUTPUT vs_Line( LineVertex vrx)
{
 Line_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
 res.color = vrx.color;
 return res;
}


float4 ps_Line(Line_PS_INPUT pnt) : COLOR
{
 return pnt.color;
}









Technique(dbgLine, LineVertex)
{
 pass P0
  {
    CullMode = none;
    AlphaBlendEnable = true;
    VertexShader = compile vs_2_0 vs_Line();
    PixelShader = compile ps_2_0 ps_Line();
  }
}


