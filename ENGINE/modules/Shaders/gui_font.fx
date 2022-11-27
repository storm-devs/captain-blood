#include "technique.h"
#include "Font.fx"


Technique(GUIFont, FontVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = TRUE;
   AlphaBlendEnable = true;
   ZEnable = FALSE;

   VertexShader = compile vs_2_0 FontVertexShader();
   PixelShader = compile ps_2_0 FontPixelShaderAlpha();
  }
}


