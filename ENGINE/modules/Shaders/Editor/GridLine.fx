#include "technique.h"
#include "stdVariables.h"
#include "DEBUG\dbgLine.fx"


Line_VS_OUTPUT vs_EditorGridLine( LineVertex vrx)
{
 Line_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
 res.color = vrx.color;
 return res;
}


float4 ps_EditorGridLine(Line_PS_INPUT pnt) : COLOR
{
 return pnt.color;
}







Technique(EditorGridLine, LineVertex)
{
 pass P0
  {
    CullMode = none;
    AlphaBlendEnable = true;
    VertexShader = compile vs_2_0 vs_EditorGridLine();
    PixelShader = compile ps_2_0 ps_EditorGridLine();
  }
}


