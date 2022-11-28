#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"
#include "gmxService\gmx_VertexShader.h"
#include "gmxService\gmx_VertexShaderAnim.h"


float4 gmxShowOverDraw(GMX_PS_IN pnt) : COLOR
{
	return float4(0.0f, 1.0f, 0.0f, fOverDrawScale); 
}


Technique(gmx_ShowOverDraw, GMX_VS_IN_STATIC)
{
	pass P0
	{
                SrcBlend = srcalpha;
		DestBlend = one;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		VertexShader = (vshader_static);
		PixelShader = compile ps_2_0 gmxShowOverDraw();
	}
}

Technique(gmx_ShowOverDraw_anim_1, GMX_VS_IN_ANIM)
{
	pass P0 
	{
                SrcBlend = srcalpha;
		DestBlend = one;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim01);
		PixelShader = compile ps_2_0 gmxShowOverDraw();
	}
}

Technique(gmx_ShowOverDraw_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
                SrcBlend = srcalpha;
		DestBlend = one;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim02);
		PixelShader = compile ps_2_0 gmxShowOverDraw();
	}
}

Technique(gmx_ShowOverDraw_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
                SrcBlend = srcalpha;
		DestBlend = one;

                BlendOp = add;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim03);
		PixelShader = compile ps_2_0 gmxShowOverDraw();
	}
}





