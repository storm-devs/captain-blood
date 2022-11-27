#include "technique.h"
#include "stdVariables.h"
#include "DEBUG\dbgLine.fx"


Line_VS_OUTPUT vs_EditorLineNoZ( LineVertex vrx)
{
 Line_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
 res.color = vrx.color;
 return res;
}


float4 ps_EditorLineNoZFirstPass(Line_PS_INPUT pnt) : COLOR
{
 return float4(pnt.color.rgb, 0.18);
}

float4 ps_EditorLineNoZ(Line_PS_INPUT pnt) : COLOR
{
 return pnt.color;
}






Technique(EditorLineNoZ, LineVertex)
{
/* multipass no longer supported!
 pass P0
  {
    ZEnable = FALSE;
    CullMode = none;
    AlphaBlendEnable = true;
    VertexShader = compile vs_2_0 vs_EditorLineNoZ();
    PixelShader = compile ps_2_0 ps_EditorLineNoZFirstPass();
  }
*/  
 pass P0
  {
    ZEnable = TRUE;
    CullMode = none;
    AlphaBlendEnable = false;
    VertexShader = compile vs_2_0 vs_EditorLineNoZ();
    PixelShader = compile ps_2_0 ps_EditorLineNoZ();
  }
}


