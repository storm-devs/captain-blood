#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"


float4 gmxShowShin(GMX_PS_IN pnt) : COLOR
{
	float4 Texel = tex2D(SpecularMap, pnt.TexCoord12.zw);
	Texel.rgb = Texel.rrr;
	Texel.a = 1.0;
	return Texel; 
}


Technique(gmx_ShowShinines, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_static);
		PixelShader = compile ps_2_0 gmxShowShin();
	}
}

Technique(gmx_ShowShinines_anim_1, GMX_VS_IN_ANIM)
{
	pass P0 
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim01);
		PixelShader = compile ps_2_0 gmxShowShin();
	}
}

Technique(gmx_ShowShinines_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim02);
		PixelShader = compile ps_2_0 gmxShowShin();
	}
}

Technique(gmx_ShowShinines_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim03);
		PixelShader = compile ps_2_0 gmxShowShin();
	}
}











