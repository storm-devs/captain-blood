#include "technique.h"
#include "debug/dbgRects.fx"

Technique(BonusGlow, RectVertex)
{
 pass P0
  {
	ZEnable = true;
	ZWriteEnable = false;
	CullMode = none;
	AlphaBlendEnable = true;
	AlphaTestEnable = false;
	srcblend = srcalpha;
	destblend = one;		

    VertexShader = compile vs_2_0 vs_dbgRects();
    PixelShader = compile ps_2_0 ps_dbgRects();
  }
}

