#include "technique.h"
#include "gmxservice\gmx_Variables.h"
#include "stdVariables.h"

// x = частота по x; y = частота по y; z = время; w = амплитуда колебаний
float4	sailFreqTimeAmp;
// rgb = цвет паруса; a = альфа
float4	sailColorAlpha;
// rgb = цвет паруса на просвет; a = множитель цвета
float4	sailGlimpseColorScale;

texture		ambientCubemap;			// кубмап амбиента
sampler		ambientCubeMapSampler = sampler_state
{
	Texture = <ambientCubemap>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
	AddressW = Clamp;
};

texture		glimpseTexture;			// текстура просвечиваемости
sampler		glimpseSampler = sampler_state
{
	Texture = <glimpseTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

	AddressU = Wrap;
	AddressV = Wrap;
};

texture		sailTexture;			// диффуз-текстура паруса
sampler		sailSampler = sampler_state
{
	Texture = <sailTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

	AddressU = Wrap;
	AddressV = Wrap;
};


struct FakeSailVS_IN
{
	float3	vPos		: POSITION;
	float3	vNormal		: NORMAL;
	float2	vTexCoord	: TEXCOORD0;
	float	vBlend		: TEXCOORD1;
};

struct SailVS_IN
{
	float3	vPos : POSITION;
	float3	vNormal : NORMAL;
	float2	vTexCoord : TEXCOORD0;
};

struct SailVS_OUT
{
	float4	Pos : POSITION;
	float2	Tex : TEXCOORD0;
	float3	Normal : TEXCOORD1;
	float4	sailColorAlpha : TEXCOORD2;
	float4	sailGlimpseColorScale : TEXCOORD3;
};


SailVS_OUT SailVS(SailVS_IN IN)
{
	SailVS_OUT OUT;

	OUT.Normal = normalize( (mul(float4(IN.vPos,1),mWorld) - mInverseView[3]).xyz );
	OUT.Tex = IN.vTexCoord;
	OUT.sailColorAlpha = sailColorAlpha;
	OUT.sailGlimpseColorScale = sailGlimpseColorScale;

	OUT.Pos = mul(float4(IN.vPos, 1), mWorldViewProj);

	return OUT;
}

SailVS_OUT FakeSailVS(FakeSailVS_IN IN)
{
	SailVS_OUT OUT;

	OUT.Normal = normalize( (mul(float4(IN.vPos,1),mWorld) - mInverseView[3]).xyz );
	OUT.Tex = IN.vTexCoord;
	OUT.sailColorAlpha = sailColorAlpha;
	OUT.sailGlimpseColorScale = sailGlimpseColorScale;

	float4 FreqPhase = float4(sailFreqTimeAmp.xy, IN.vPos.x*IN.vPos.x+IN.vPos.z*IN.vPos.z, IN.vPos.y*IN.vPos.y);
	float4 wavePacked;
	sincos(FreqPhase.xy*sailFreqTimeAmp.z + FreqPhase.zw, wavePacked.xy, wavePacked.zw);
	float4 Pos = float4(IN.vPos + sailFreqTimeAmp.w*IN.vBlend*(wavePacked.x + wavePacked.y) * IN.vNormal, 1);

	OUT.Pos = mul(Pos, mWorldViewProj);

	return OUT;
}

// общий кусок освещения паруса - для всех версий шейдеров
float4 ShadeSail(SailVS_OUT IN)
{
	float4 Color;

	float4 texel = tex2D( sailSampler, IN.Tex.xy );
	float3 ambient = vAmbientLight.xyz;

	float Cos = dot(IN.Normal, vGlobalLightDirection.xyz);
	if ( Cos >= 0)
		Color = float4(vGlobalLightColor.xyz*IN.sailColorAlpha.rgb, 1.0f)*Cos;
	else
		Color = -float4(IN.sailGlimpseColorScale.rgb, 1.0f)*float4(vGlobalLightBackColor.xyz, 1.0f)*tex2D(glimpseSampler, IN.Tex.xy)*Cos;
	float3 diffuse = Color.rgb;

	return float4(saturate(texel.xyz * diffuse * ambient) + (texel.xyz * ambient), IN.sailColorAlpha.w);
}

float4 SailPS(SailVS_OUT IN) : COLOR
{
	return ShadeSail(IN);
}

Technique(FakeSailCCW, FakeSailVS_IN)
{
	pass P0
	{
		AlphaTestEnable = false;
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		CullMode = none;
		ZEnable = true;
		ZWriteEnable = true;

		VertexShader = compile vs_2_0 FakeSailVS();
		PixelShader = compile ps_2_0 SailPS();
	}
}

Technique(SailCCW, SailVS_IN)
{
	pass P0
	{
		AlphaTestEnable = false;
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		CullMode = none;
		ZEnable = true;
		ZWriteEnable = true;

		VertexShader = compile vs_2_0 SailVS();
		PixelShader = compile ps_2_0 SailPS();
	}
}
