#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"
#include "gmxService\gmx_VertexShader.h"
#include "gmxService\gmx_VertexShaderAnim.h"



float4 gmxShowLightComplex(GMX_PS_IN pnt) : COLOR
{
	float4 Texel = tex2D(DiffuseMap, pnt.TexCoord12.xy);
	

	
	float df_nDotL = clamp(dot(pnt.vNormal_And_WorldPosX, vGlobalLightDirection.xyz), 0.4, 1.0);
	
	
	return float4 (GMX_ObjectsUserColor.rgb*df_nDotL, Texel.a); 
}


Technique(gmx_ShowLightComplex, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_static);
		PixelShader = compile ps_2_0 gmxShowLightComplex();
	}
}

Technique(gmx_ShowLightComplex_anim_1, GMX_VS_IN_ANIM)
{
	pass P0 
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim01);
		PixelShader = compile ps_2_0 gmxShowLightComplex();
	}
}

Technique(gmx_ShowLightComplex_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim02);
		PixelShader = compile ps_2_0 gmxShowLightComplex();
	}
}

Technique(gmx_ShowLightComplex_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim03);
		PixelShader = compile ps_2_0 gmxShowLightComplex();
	}
}





