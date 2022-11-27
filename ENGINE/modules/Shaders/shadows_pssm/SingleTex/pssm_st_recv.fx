//#ifdef _XBOX

#include "technique.h"
#include "stdVariables.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_UberGenerator.h"

#include "shadows_pssm\pssm_vars.h"

struct test_vsout
{
	float4 pos : POSITION;
	float4 tc0 : TEXCOORD0;
};

struct test_psin
{
	float4 tc0 : TEXCOORD0;
};

test_vsout VS_Recv(GMX_VS_IN_STATIC In, uniform bool isProj)
{
	test_vsout res;

	float4 wpos = mul(float4(In.vPos, 1.0f), mWorldViewProj);
	res.pos = wpos;

	float3 vNormal = UnpackVec(In.vPackedNormal);
	float fLdotN = abs(dot(vNormal, vPSSMLightDir));

	float fDist;
	if (isProj)
	{
		float4 worldpos = mul(float4(In.vPos, 1.0f), mWorld);
		fDist = dot(worldpos, v4PSSM_LightViewOnlyZ);
	}
	else
	{
		fDist = res.pos.z;
	}

	float fDistance = 1.0 - saturate((fDist - vPSSMShading.z) * vPSSMShading.y);

	res.tc0.x = wpos.z * 0.01f;
	res.tc0.y = GMX_ObjectsUserColor.a * fPSSMShadowPow * fDistance * lerp(vPSSMshdK.x, vPSSMshdK.y, fLdotN);
	res.tc0.zw = UnpackUV(In.vPackedUV1);

	return res;
}

test_vsout VS_Recv_Anim(GMX_VS_IN_ANIM In, uniform int nNumBones, uniform bool isProj)
{
	test_vsout res;

	GMX_VS_OUT gmx_out = GMX_VertexShaderAnim(In, nNumBones);

	float3 vNormal = gmx_out.vNormal_And_WorldPosX.xyz;
	res.pos = gmx_out.Position;

	float fDist;
	if (isProj)
	{
		float4 vWorldPos;
		vWorldPos.x = gmx_out.vNormal_And_WorldPosX.w;
		vWorldPos.y = gmx_out.vTangent_And_WorldPosY.w;
		vWorldPos.z = gmx_out.vBinormal_And_WorldPosZ.w;
		vWorldPos.w = 1.0f;

		fDist = dot(vWorldPos, v4PSSM_LightViewOnlyZ);
	}
	else
	{
		fDist = res.pos.z;
	}

	float fDistance = 1.0 - saturate((fDist - vPSSMShading.z) * vPSSMShading.y);
	float fLdotN = abs(dot(vNormal, vPSSMLightDir));

	res.tc0.x = res.pos.z * 0.01f;
	res.tc0.y = GMX_ObjectsUserColor.a * fPSSMShadowPow * fDistance * lerp(vPSSMshdK.x, vPSSMshdK.y, fLdotN);
	res.tc0.zw = gmx_out.TexCoord12.xy;

	return res;
}

float4 PSSM2_ST_PShader_Recv(test_psin In) : COLOR
{
	half4 DifTex = tex2D(DiffuseMap, In.tc0.zw);
	clip(DifTex.a - gmxAlphaRef);

	return float4(In.tc0.x, In.tc0.y, 0.0f, 1.0f);
}

struct DefVsIn
{
	float4 pos : POSITION;
	float3 dir : TEXCOORD0;
	float2 uv : TEXCOORD1;
};

struct DefVsOut
{
	float4 pos : POSITION;
	float3 dir : TEXCOORD0;
	float2 uv : TEXCOORD1;
};

struct DefPsIn
{
	float3 dir : TEXCOORD0;
	float2 uv : TEXCOORD1;
};

sampler linearZSampler =
sampler_state
{
	Texture = <tPSSM_linearZTexture>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

DefVsOut DefVS(DefVsIn In)
{
	DefVsOut res;

	res.pos = In.pos;
	res.dir = In.dir;
	res.uv = In.uv;

	return res;
}

float4 DefPS(DefPsIn In) : COLOR
{
	float2 zzz = tex2D(linearZSampler, In.uv);

	zzz.x *= 100.0f;

	float4 realpos;
	realpos.xyz = vPSSMCamPos.xyz + In.dir.xyz * zzz.x;
	realpos.w = 1.0f;

	float ztest;
	float shadow = 0.0f;

	float4 pos = mul(realpos, mWorldViewProj);
	
	clip(v4PSSMSplits.x - pos.z);

	//if (pos.z <= v4PSSMSplits.y)
	//{
		float4 _ss = mul(realpos, mPSSM_ShadowMap);
		shadow = PSSM_GetShadowTerm(PSSMShadowMapSampler, _ss, _ss.z - vPSSMZBias.x);
	//}
	/*else if (pos.z > v4PSSMSplits.z)
	{
		float4 _ss = mul(realpos, mPSSM_ShadowMap);
		shadow = PSSM_GetShadowTerm(PSSMShadowMapSampler, _ss, _ss.z - vPSSMZBias.z);
	}
	else
	{
		float4 _ss = mul(realpos, mPSSM_ShadowMap);
		shadow = PSSM_GetShadowTerm(PSSMShadowMapSampler, _ss, _ss.z - vPSSMZBias.y);
	}*/

	float fK = saturate((1.0 - shadow) * zzz.y);
	
	float3 vColor = lerp(float3(1.0, 1.0, 1.0), vPSSMShadowColor.rgb, fK);

	return float4(vColor, 1.0f);
}

Technique(DefShade, DefVsIn)
{
	pass P0
	{
		CullMode = none;
		//ZEnable = false;
		ZWriteEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 DefVS();
		PixelShader = compile ps_2_0 DefPS();
	}
}

Technique(PSSM_ST_Recv, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 VS_Recv(false);
		PixelShader = compile ps_2_0 PSSM2_ST_PShader_Recv();
	}
}

Technique(PSSM_ST_Recv_anim_1, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 VS_Recv_Anim(1, false);
		PixelShader = compile ps_2_0 PSSM2_ST_PShader_Recv();
	}
}

Technique(PSSM_ST_Recv_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
	
		VertexShader = compile vs_2_0 VS_Recv_Anim(2, false);
		PixelShader = compile ps_2_0 PSSM2_ST_PShader_Recv();
	}
}

Technique(PSSM_ST_Recv_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 VS_Recv_Anim(3, false);
		PixelShader = compile ps_2_0 PSSM2_ST_PShader_Recv();
	}
}

//#endif