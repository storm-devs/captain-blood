#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"


float4 gmxShowOcclus(GMX_PS_IN pnt) : COLOR
{
	float4 Texel = tex2D(SpecularMap, pnt.TexCoord12.zw);
	Texel.rgb = Texel.bbb;
	Texel.a = 1.0;
	return Texel; 
}


Technique(gmx_ShowOcclusion, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_static);
		PixelShader = compile ps_2_0 gmxShowOcclus();
	}
}

Technique(gmx_ShowOcclusion_anim_1, GMX_VS_IN_ANIM)
{
	pass P0 
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim01);
		PixelShader = compile ps_2_0 gmxShowOcclus();
	}
}

Technique(gmx_ShowOcclusion_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim02);
		PixelShader = compile ps_2_0 gmxShowOcclus();
	}
}

Technique(gmx_ShowOcclusion_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim03);
		PixelShader = compile ps_2_0 gmxShowOcclus();
	}
}







