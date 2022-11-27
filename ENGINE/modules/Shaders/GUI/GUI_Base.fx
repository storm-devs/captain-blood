#include "technique.h"
#include "GUI\Gui_common.h"


GUI_VS_OUTPUT GUIVertexShader( GUIVertex vrx)
{
 GUI_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), GUI_CliperMatrix);
 res.color = vrx.color;
 return res;
}


float4 GUIPixelShader(GUI_PS_INPUT pnt) : COLOR
{
 return pnt.color;
}









Technique(GUIBase, GUIVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = true;
   AlphaBlendEnable = true;
   ZEnable = FALSE;

   VertexShader = compile vs_2_0 GUIVertexShader();
   PixelShader = compile ps_2_0 GUIPixelShader();
  }
}


