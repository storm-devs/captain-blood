#include "technique.h"
#include "GUI\Gui_common.h"


GUI_VS_OUTPUT GUISVertexShader( GUIVertex vrx)
{
 GUI_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), GUI_CliperMatrix);
 res.color = vrx.color;
 return res;
}


float4 GUISPixelShader(GUI_PS_INPUT pnt) : COLOR
{
 return float4(pnt.color.rgb, 0.5);
}









Technique(GUISelected, GUIVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = false;
   AlphaBlendEnable = true;
   ZEnable = FALSE;

   VertexShader = compile vs_2_0 GUISVertexShader();
   PixelShader = compile ps_2_0 GUISPixelShader();
  }
}


