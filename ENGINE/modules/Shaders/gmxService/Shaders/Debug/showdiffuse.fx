#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"
#include "gmxService\gmx_VertexShader.h"
#include "gmxService\gmx_VertexShaderAnim.h"


float4 gmxShowDiffuse(GMX_PS_IN pnt) : COLOR
{
/*
    float fFog = CalculateFogFactor(pnt.vWorldPos);
    return float4 (fFog, fFog, fFog, 1.0);
*/    
    

	float4 Texel = tex2D(DiffuseMap, pnt.TexCoord12.xy);
	return Texel; 
}


Technique(gmx_ShowDiffuse, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_static);
		PixelShader = compile ps_2_0 gmxShowDiffuse();
	}
}

Technique(gmx_ShowDiffuse_anim_1, GMX_VS_IN_ANIM)
{
	pass P0 
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim01);
		PixelShader = compile ps_2_0 gmxShowDiffuse();
	}
}

Technique(gmx_ShowDiffuse_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim02);
		PixelShader = compile ps_2_0 gmxShowDiffuse();
	}
}

Technique(gmx_ShowDiffuse_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		VertexShader = (vshader_anim03);
		PixelShader = compile ps_2_0 gmxShowDiffuse();
	}
}





