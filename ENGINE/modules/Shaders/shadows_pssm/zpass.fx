#include "technique.h"
#include "stdVariables.h"

struct ZPASS_VS_IN
{
    float3 pos : POSITION;
};

struct ZPASS_VS_OUT
{
    float4 pos : POSITION;
};

ZPASS_VS_OUT PSSM_VShader_ZPass(ZPASS_VS_IN In)
{
	ZPASS_VS_OUT res;
	res.pos = mul(float4(In.pos, 1.0f), mWorldViewProj);
	return res;
}

float4 PSSM_PShader_ZPass() : COLOR
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

Technique(PSSM_ZPass, ZPASS_VS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		ColorWriteEnable = 0;
		
		VertexShader = compile vs_2_0 PSSM_VShader_ZPass();
		PixelShader = null;
	}
}

Technique(PSSM_ZPass_WireFrame, ZPASS_VS_IN)
{
	pass P0
	{
		FillMode = wireframe;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 PSSM_VShader_ZPass();
		PixelShader = compile ps_2_0 PSSM_PShader_ZPass();
	}
}
