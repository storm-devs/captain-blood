#include "technique.h"
#include "stdVariables.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_UberGenerator.h"

#include "shadows_pssm\pssm_vars.h"

/*float4 PSSM2_Proj_PShader_Recv(PSSM2_PS_INPUT In) : COLOR
{
	float4 DifTex = tex2D(DiffuseMap, In.TexCoord3.xy);
	DifTex.a *= GMX_ObjectsUserColor.a;

	float fDivW = 1.0 / In.TexCoord0.w;
	float2 vTexCoord = In.TexCoord0.xy * fDivW;

	float4 depth;
	depth.r = tex2D(PSSMShadowMapSampler0, vTexCoord).r;
	depth.g = tex2D(PSSMShadowMapSampler0, vTexCoord + vPSSM_TexOffsets[0].xy).r;
	depth.b = tex2D(PSSMShadowMapSampler0, vTexCoord + vPSSM_TexOffsets[1].xy).r;
	depth.a = tex2D(PSSMShadowMapSampler0, vTexCoord + vPSSM_TexOffsets[2].xy).r;
	float ztest =  saturate((In.TexCoord0.z * fDivW) - vPSSMZBias.x);
	half4 inLight = depth >= float4(ztest, ztest, ztest, ztest);
	half shadowTerm = dot(inLight, half4(0.25, 0.25, 0.25, 0.25));

	float fK = (1.0 - shadowTerm) * In.TexCoord2.w * (In.TexCoord1.w > 0.0);
	float3 vColor = lerp(float3(1.0, 1.0, 1.0), vPSSMShadowColor.rgb, fK);
	float4 vRes = float4(vColor, (1.01 - saturate(fK)) * (DifTex.a > gmxAlphaRef));

	return vRes;
}

Technique(PSSM_Proj_Recv, GMX_VS_IN_STATIC)
{
	pass P0
	{
		PSSM_BLEND_MODE_OFF
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv(true);
		PixelShader = compile ps_2_0 PSSM2_Proj_PShader_Recv();
	}
}

Technique(PSSM_Proj_Recv_anim_1, GMX_VS_IN_ANIM)
{
	pass P0
	{
		PSSM_BLEND_MODE_OFF
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(1, true);
		PixelShader = compile ps_2_0 PSSM2_Proj_PShader_Recv();
	}
}

Technique(PSSM_Proj_Recv_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		PSSM_BLEND_MODE_OFF
	
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(2, true);
		PixelShader = compile ps_2_0 PSSM2_Proj_PShader_Recv();
	}
}

Technique(PSSM_Proj_Recv_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		PSSM_BLEND_MODE_OFF
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(3, true);
		PixelShader = compile ps_2_0 PSSM2_Proj_PShader_Recv();
	}
}

Technique(PSSM_Proj_Recv_alpha, GMX_VS_IN_STATIC)
{
	pass P0
	{
		PSSM_BLEND_MODE_ON
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv(true);
		PixelShader = compile ps_2_0 PSSM2_Proj_PShader_Recv();
	}
}

Technique(PSSM_Proj_Recv_alpha_anim_1, GMX_VS_IN_ANIM)
{
	pass P0
	{
		PSSM_BLEND_MODE_ON
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(1, true);
		PixelShader = compile ps_2_0 PSSM2_Proj_PShader_Recv();
	}
}

Technique(PSSM_Proj_Recv_alpha_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		PSSM_BLEND_MODE_ON
	
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(2, true);
		PixelShader = compile ps_2_0 PSSM2_Proj_PShader_Recv();
	}
}

Technique(PSSM_Proj_Recv_alpha_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		PSSM_BLEND_MODE_ON
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(3, true);
		PixelShader = compile ps_2_0 PSSM2_Proj_PShader_Recv();
	}
}

*/