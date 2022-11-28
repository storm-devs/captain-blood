#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"
#include "gmxService\gmx_VertexShader.h"
#include "gmxService\gmx_VertexShaderAnim.h"


float4 gmxShowBatches(GMX_PS_IN pnt) : COLOR
{
	return float4(GMX_ObjectsUserColor.x,GMX_ObjectsUserColor.y,GMX_ObjectsUserColor.z, 1.0); 
}


Technique(gmx_ShowBatches, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_static);
		PixelShader = compile ps_2_0 gmxShowBatches();
	}
}

Technique(gmx_ShowBatches_anim_1, GMX_VS_IN_ANIM)
{
	pass P0 
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim01);
		PixelShader = compile ps_2_0 gmxShowBatches();
	}
}

Technique(gmx_ShowBatches_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim02);
		PixelShader = compile ps_2_0 gmxShowBatches();
	}
}

Technique(gmx_ShowBatches_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim03);
		PixelShader = compile ps_2_0 gmxShowBatches();
	}
}





