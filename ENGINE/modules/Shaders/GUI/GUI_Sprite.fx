#include "technique.h"
#include "GUI\Gui_common.h"

GUISPR_VS_OUTPUT GUISPRVertexShader( GUISpriteVertex vrx)
{
 GUISPR_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), GUI_CliperMatrix);
 res.color = vrx.color;
 res.uv = vrx.uv;
 return res;
}


float4 GUISPRPixelShader(GUISPR_PS_INPUT pnt) : COLOR
{
 float4 tex = tex2D(GUISampler, pnt.uv);
 return (tex * pnt.color);
}









Technique(GUISprite, GUISpriteVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = true;
   AlphaBlendEnable = true;
   ZEnable = FALSE;

   VertexShader = compile vs_2_0 GUISPRVertexShader();
   PixelShader = compile ps_2_0 GUISPRPixelShader();
  }
}


