#ifndef _XBOX

#include "technique.h"
#include "stdVariables.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_UberGenerator.h"

#include "shadows_pssm\pssm_vars.h"

float4 PSSM_MT_PShader_Cast(PSSM_PS_INPUT In) : COLOR
{
	float4 DifTex = tex2D(DiffuseMap, In.TexCoord0.xy);
	clip(DifTex.a - gmxAlphaRef);
	
	float fDepth = In.TexCoord0.z; // / In.TexCoord0.w;
	
	return float4(fDepth, fDepth, fDepth, fDepth);
}

Technique(PSSM_MT_Cast, GMX_VS_IN_STATIC)
{
	pass P0
	{
		//CullMode = none;
		
		AlphaTestEnable = false;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 PSSM_VShader_Cast();
		PixelShader = compile ps_2_0 PSSM_MT_PShader_Cast();
	}
}

// ANIM 1
Technique(PSSM_MT_Cast_anim_1, GMX_VS_IN_ANIM)
{
	pass P0
	{
		//CullMode = none;
		
		AlphaTestEnable = false;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 PSSM_VShader_Cast_Anim(1);
		PixelShader = compile ps_2_0 PSSM_MT_PShader_Cast();
	}
}

// ANIM 2
Technique(PSSM_MT_Cast_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		//CullMode = none;
		
		AlphaTestEnable = false;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 PSSM_VShader_Cast_Anim(2);
		PixelShader = compile ps_2_0 PSSM_MT_PShader_Cast();
	}
}

// ANIM 3
Technique(PSSM_MT_Cast_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		//CullMode = none;
		
		AlphaTestEnable = false;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 PSSM_VShader_Cast_Anim(3);
		PixelShader = compile ps_2_0 PSSM_MT_PShader_Cast();
	}
}

#endif