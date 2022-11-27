#ifndef _XBOX

#include "technique.h"
#include "stdVariables.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_UberGenerator.h"

#include "shadows_pssm\pssm_vars.h"

/*half4 PSSM2_MT_PShader_Recv(PSSM2_PS_INPUT In) : COLOR
{
	half4 DifTex = tex2D(DiffuseMap, In.TexCoord3.xy);
	clip(DifTex.a - gmxAlphaRef);
	
	float t = In.TexCoord3.w;
	float ztest;

	half shadow;
	if (In.TexCoord3.z <= v4PSSMSplits.y)
	{
		ztest = In.TexCoord0.z - (t * vPSSMZBias.x);
		shadow = PSSM_GetShadowTerm(PSSMShadowMapSampler0, In.TexCoord0, ztest);
	}
	else if (In.TexCoord3.z > v4PSSMSplits.z)
	{
		ztest = In.TexCoord2.z - (t * vPSSMZBias.z);
		shadow = PSSM_GetShadowTerm(PSSMShadowMapSampler2, In.TexCoord2, ztest);
	}
	else
	{
		ztest = In.TexCoord1.z - (t * vPSSMZBias.y);
		shadow = PSSM_GetShadowTerm(PSSMShadowMapSampler1, In.TexCoord1, ztest);
	}
	
	half fK = saturate((1.0 - shadow) * In.TexCoord2.w);
	
	half3 vColor = lerp(half3(1.0, 1.0, 1.0), vPSSMShadowColor.rgb, fK);
	half4 vRes = half4(vColor, 1.0f);//(1.01 - fK) * (DifTex.a > gmxAlphaRef));

	return vRes;
}

Technique(PSSM_MT_Recv, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv(false);
		PixelShader = compile ps_2_0 PSSM2_MT_PShader_Recv();
	}
}

Technique(PSSM_MT_Recv_anim_1, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(1, false);
		PixelShader = compile ps_2_0 PSSM2_MT_PShader_Recv();
	}
}

Technique(PSSM_MT_Recv_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
	
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(2, false);
		PixelShader = compile ps_2_0 PSSM2_MT_PShader_Recv();
	}
}

Technique(PSSM_MT_Recv_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(3, false);
		PixelShader = compile ps_2_0 PSSM2_MT_PShader_Recv();
	}
}*/

#endif