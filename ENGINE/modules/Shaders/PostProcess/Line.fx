#include "technique.h"

struct _LineVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
};

struct _LINE_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
};

struct _LINE_PS_INPUT
{
 float4 color : COLOR0;
};




_LINE_VS_OUTPUT _LineVertexShader( _LineVertex vrx)
{
 _LINE_VS_OUTPUT res;
 res.pos = float4(vrx.pos, 1);
 res.color = vrx.color;
 return res;
}


float4 _LinePixelShader(_LINE_PS_INPUT pnt) : COLOR
{
 return pnt.color;
}









Technique(RenderLine, _LineVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = true;
   AlphaBlendEnable = true;
   ZEnable = FALSE;

   VertexShader = compile vs_2_0 _LineVertexShader();
   PixelShader = compile ps_2_0 _LinePixelShader();
  }
}


