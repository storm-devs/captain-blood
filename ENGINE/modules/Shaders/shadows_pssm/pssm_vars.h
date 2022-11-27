#ifndef PSSM_VARS
#define PSSM_VARS

#include "gmxService\gmx_VertexShaderAnim.h"

float3 vPSSMShadowColor;
float3 vPSSMShading;					// x - full dist, y - fading size, z - fading start
float3 vPSSMLightDir;
float4 vPSSMshdK;
float4 vPSSMZBias;
float4 vPSSMCamPos;
float4 pssm_TexParams;		// (dx, 1.0f / dx, 0.0f, 0.0f)
float fPSSMShadowMapSize;
float fPSSMShadowPow;
float4 v4PSSMSplits;
float4x4 mPSSM_ShadowMap;
texture	tPSSM_ShadowMap;
texture tPSSM_linearZTexture;
float4 v4PSSM_LightViewOnlyZ;	// only z component from mPSSM_LightView
float4 vPSSM_TexOffsets[4];

sampler PSSMShadowMapSampler =
sampler_state
{
	Texture = <tPSSM_ShadowMap>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;

	AddressU = Border;
	AddressV = Border;

	BorderColor = 0xFFFFFFFF;
};

struct PSSM_VS_OUTPUT_NPS
{
    float4 pos : POSITION;
};

struct PSSM_VS_OUTPUT
{
    float4 pos : POSITION;
    float4 TexCoord0 : TEXCOORD0;
};

struct PSSM_PS_INPUT
{
    float4 TexCoord0 : TEXCOORD0;
};

PSSM_VS_OUTPUT_NPS PSSM_VShader_CastNPS(GMX_VS_IN_STATIC In)
{
	PSSM_VS_OUTPUT_NPS res;
	res.pos = mul( float4(In.vPos, 1.0f), mWorldViewProj);
	return res;
}

PSSM_VS_OUTPUT PSSM_VShader_Cast(GMX_VS_IN_STATIC In)
{
	PSSM_VS_OUTPUT res;

	float4 vPos = mul( float4(In.vPos, 1.0f), mWorldViewProj);
	res.pos = vPos;
	res.TexCoord0.xy = UnpackUV(In.vPackedUV1);
	res.TexCoord0.zw = vPos.zw;
	
	return res;
}

PSSM_VS_OUTPUT PSSM_VShader_Cast_Anim(GMX_VS_IN_ANIM In, uniform int nNumBones)
{
	PSSM_VS_OUTPUT res;
	GMX_VS_OUT gmx_out = GMX_VertexShaderAnim(In, nNumBones);

	res.pos = gmx_out.Position;
	res.TexCoord0.xy = gmx_out.TexCoord12.xy;
	res.TexCoord0.zw = res.pos.zw;

	return res;
}

PSSM_VS_OUTPUT_NPS PSSM_VShader_CastNPS_Anim(GMX_VS_IN_ANIM In, uniform int nNumBones)
{
	PSSM_VS_OUTPUT_NPS res;

	GMX_VS_OUT gmx_out = GMX_VertexShaderAnim(In, nNumBones);
	res.pos = gmx_out.Position;

	return res;
}

struct PSSM2_VS_OUTPUT
{
    float4 pos : POSITION;
    
    float4 TexCoord0 : TEXCOORD0;
    float4 TexCoord1 : TEXCOORD1;
    float4 TexCoord2 : TEXCOORD2;
    float4 TexCoord3 : TEXCOORD3;
};

struct PSSM2_PS_INPUT
{
    float4 TexCoord0 : TEXCOORD0;
    float4 TexCoord1 : TEXCOORD1;
    float4 TexCoord2 : TEXCOORD2;
    float4 TexCoord3 : TEXCOORD3;
};

PSSM2_VS_OUTPUT PSSM2_VShader_Recv(GMX_VS_IN_STATIC In, uniform bool isProj)
{
	PSSM2_VS_OUTPUT res;

	res.pos = mul(float4(In.vPos, 1.0f), mWorldViewProj);

	float4 vPos = mul( float4(In.vPos, 1.0f), mWorld);
	float3 vNormal = UnpackVec(In.vPackedNormal);

	float fDist;
	if (isProj)
	{
		fDist = dot(vPos, v4PSSM_LightViewOnlyZ);
	}
	else
	{
		fDist = res.pos.z;
	}

	float fDistance = 1.0 - saturate((fDist - vPSSMShading.z) * vPSSMShading.y);
	
	float fLdotN = abs(dot(vNormal, vPSSMLightDir));

	res.TexCoord0 = mul(vPos, mPSSM_ShadowMap);
	res.TexCoord1.xyz = 0.0f;//mul(vPos, mPSSM_ShadowMap1);
	res.TexCoord1.w = fDist;
	res.TexCoord2.xyz = 0.0f;//mul(vPos, mPSSM_ShadowMap2);
	res.TexCoord2.w = GMX_ObjectsUserColor.a * fPSSMShadowPow * fDistance * lerp(vPSSMshdK.x, vPSSMshdK.y, fLdotN);
	res.TexCoord3.xy = UnpackUV(In.vPackedUV1);
	res.TexCoord3.z = res.pos.z;
	res.TexCoord3.w = 1.0 + 8.0f * saturate(lerp(1.0, 0.0, abs(fLdotN) * 4.0f));

	return res;
}

PSSM2_VS_OUTPUT PSSM2_VShader_Recv_Anim(GMX_VS_IN_ANIM In, uniform int nNumBones, uniform bool isProj)
{
	PSSM2_VS_OUTPUT res;

	GMX_VS_OUT gmx_out = GMX_VertexShaderAnim(In, nNumBones);

	float3 vNormal;
	float4 vWorldPos;

	res.pos = gmx_out.Position;

	vNormal = gmx_out.vNormal_And_WorldPosX.xyz;
	vWorldPos.x = gmx_out.vNormal_And_WorldPosX.w;
	vWorldPos.y = gmx_out.vTangent_And_WorldPosY.w;
	vWorldPos.z = gmx_out.vBinormal_And_WorldPosZ.w;
	vWorldPos.w = 1.0f;

	float fDist;
	if (isProj)
	{
		fDist = dot(vWorldPos, v4PSSM_LightViewOnlyZ);
	}
	else
	{
		fDist = res.pos.z;
	}

	float fDistance = 1.0 - saturate((fDist - vPSSMShading.z) * vPSSMShading.y);
	float fLdotN = abs(dot(vNormal, vPSSMLightDir));

	res.TexCoord0 = mul(vWorldPos, mPSSM_ShadowMap);
	res.TexCoord1.xyz = 0.0f;//mul(vWorldPos, mPSSM_ShadowMap1);
	res.TexCoord1.w = fDist;
	res.TexCoord2.xyz = 0.0f;//mul(vWorldPos, mPSSM_ShadowMap2);
	res.TexCoord2.w = GMX_ObjectsUserColor.a * fPSSMShadowPow * fDistance * lerp(vPSSMshdK.x, vPSSMshdK.y, fLdotN);
	res.TexCoord3.xy = gmx_out.TexCoord12.xy;//UnpackUV(In.s0.vPackedUV1);
	res.TexCoord3.z = res.pos.z;
	res.TexCoord3.w = 1.0 + 8.0f * saturate(lerp(1.0, 0.0, abs(fLdotN) * 4.0f));

	return res;
}

float PSSM_GetShadowTermSimple(sampler texSampler, float2 texCoord, float ztest)
{
	float dd = tex2D(texSampler, texCoord).r;
	return dd >= ztest;
}

float PSSM_GetShadowTermNormal(sampler texSampler, float2 texCoord, float ztest)
{
	const float smxy = pssm_TexParams.x;
	const float smdiv = pssm_TexParams.y;

	float4 depth;
#ifdef _XBOX
	asm {
		tfetch2D depth.x___, texCoord, texSampler, OffsetX = 0.0, OffsetY = 0.0
		tfetch2D depth._x__, texCoord, texSampler, OffsetX = 1.0, OffsetY = 0.0
		tfetch2D depth.__x_, texCoord, texSampler, OffsetX = 0.0, OffsetY = 1.0
		tfetch2D depth.___x, texCoord, texSampler, OffsetX = 1.0, OffsetY = 1.0
	};
#else
	depth.r = tex2D(texSampler, texCoord).r;
	depth.g = tex2D(texSampler, texCoord + float2(smdiv, 0.0f)).r;
	depth.b = tex2D(texSampler, texCoord + float2(0.0f, smdiv)).r;
	depth.a = tex2D(texSampler, texCoord + float2(smdiv, smdiv)).r;
#endif

	half4 inLight = depth >= float4(ztest, ztest, ztest, ztest);
	float2 texLA = frac(texCoord.xy * float2(smxy, smxy));
	float2 grad = lerp(inLight.rb, inLight.ga, texLA.x);
	return lerp(grad.x, grad.y, texLA.y);
}

float PSSM_GetShadowTerm(sampler texSampler, float2 texCoord, float ztest)
{
	float4 depth, depth1;
	float depth2;
	half4 inLight, inLight1;
	half inLight2;
	
	const float smxy = pssm_TexParams.x;
	const float smdiv = pssm_TexParams.y;
	
#ifdef _XBOX
	asm {
		tfetch2D depth.x___, texCoord, texSampler, OffsetX = 0.0, OffsetY = 0.0
		tfetch2D depth._x__, texCoord, texSampler, OffsetX = 1.0, OffsetY = 0.0
		tfetch2D depth.__x_, texCoord, texSampler, OffsetX = 0.0, OffsetY = 1.0
		tfetch2D depth.___x, texCoord, texSampler, OffsetX = 1.0, OffsetY = 1.0

		tfetch2D depth1.x___, texCoord, texSampler, OffsetX = -1.0, OffsetY = -1.0
		tfetch2D depth1._x__, texCoord, texSampler, OffsetX = 0.0, OffsetY = -1.0
		tfetch2D depth1.__x_, texCoord, texSampler, OffsetX = 1.0, OffsetY = -1.0
		tfetch2D depth1.___x, texCoord, texSampler, OffsetX = -1.0, OffsetY = 0.0

		tfetch2D depth2.x___, texCoord, texSampler, OffsetX = -1.0, OffsetY = 1.0
	};
#else
	depth.r = tex2D(texSampler, texCoord).r;
	depth.g = tex2D(texSampler, texCoord + float2(smdiv, 0.0f)).r;
	depth.b = tex2D(texSampler, texCoord + float2(0.0f, smdiv)).r;
	depth.a = tex2D(texSampler, texCoord + float2(smdiv, smdiv)).r;

	depth1.r = tex2D(texSampler, texCoord + float2(-smdiv, -smdiv)).r;
	depth1.g = tex2D(texSampler, texCoord + float2(0.0, -smdiv)).r;
	depth1.b = tex2D(texSampler, texCoord + float2(smdiv, -smdiv)).r;
	depth1.a = tex2D(texSampler, texCoord + float2(-smdiv, 0.0f)).r;

	depth2 = tex2D(texSampler, texCoord + float2(-smdiv, smdiv)).r;
#endif

	inLight = depth >= float4(ztest, ztest, ztest, ztest);
	inLight1 = depth1 >= float4(ztest, ztest, ztest, ztest);
	inLight2 = depth2.r >= ztest;

	float2 texLA = frac(texCoord.xy * float2(smxy, smxy));
	
	float4 l1;
	l1.r = lerp(inLight.r, inLight.g, texLA.x);
	//l1.g = lerp(inLight1.r, inLight1.g, texLA.x);
	//l1.b = lerp(inLight1.g, inLight1.b, texLA.x);
	l1.gb = lerp(inLight1.rg, inLight1.gb, texLA.x);
	l1.a = lerp(inLight1.a, inLight.r, texLA.x);

	float4 l2;
	l2.r = lerp(inLight.b, inLight.a, texLA.x);
	l2.g = l1.a;
	l2.b = l1.r;
	l2.a = lerp(inLight2, inLight.b, texLA.x);
	
	return dot(lerp(l1, l2, texLA.y), float4(0.25, 0.25, 0.25, 0.25));
}

#ifndef _XBOX
float4 PSSM2_SolidFill_PShader_Recv(PSSM2_PS_INPUT In) : COLOR
{
	float3 fSplits;
	fSplits.x = (In.TexCoord3.z <= v4PSSMSplits.y);
	fSplits.y = (In.TexCoord3.z > v4PSSMSplits.y) * (In.TexCoord3.z <= v4PSSMSplits.z);
	fSplits.z = (In.TexCoord3.z > v4PSSMSplits.z);

	return float4(fSplits, 1.0);
}

Technique(PSSM_Recv_Solid, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaRef = 1;
		AlphaTestEnable = true;
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv(false);
		PixelShader = compile ps_2_0 PSSM2_SolidFill_PShader_Recv();
	}
}

Technique(PSSM_Recv_Solid_anim_1, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaRef = 1;
		AlphaTestEnable = true;
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(1, false);
		PixelShader = compile ps_2_0 PSSM2_SolidFill_PShader_Recv();
	}
}

Technique(PSSM_Recv_Solid_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaRef = 1;
		AlphaTestEnable = true;
	
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(2, false);
		PixelShader = compile ps_2_0 PSSM2_SolidFill_PShader_Recv();
	}
}

Technique(PSSM_Recv_Solid_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaBlendEnable = false;
		AlphaRef = 1;
		AlphaTestEnable = true;
		
		VertexShader = compile vs_2_0 PSSM2_VShader_Recv_Anim(3, false);
		PixelShader = compile ps_2_0 PSSM2_SolidFill_PShader_Recv();
	}
}
#endif

#define PSSM_BLEND_MODE_ON		\
	AlphaRef = 1;				\
	AlphaTestEnable = true;		\
	AlphaBlendEnable = true;	\
	BlendOp = min;				\
	SrcBlend = one;				\
	DestBlend = one;			\


#define PSSM_BLEND_MODE_OFF		\
	AlphaRef = 1;				\
	AlphaTestEnable = true;		\
	AlphaBlendEnable = false;	\


#endif