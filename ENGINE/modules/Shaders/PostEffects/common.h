#include "stdVariables.h"

struct PEFS_VSIN
{
	float4 pos	: POSITION;
	float2 uv	: TEXCOORD0;
};

struct PEFS_VSOUT
{
	float4 pos	: POSITION;
	float2 uv	: TEXCOORD0;
};

struct PEFS_PSIN
{
	float2 uv	: TEXCOORD0;
};

PEFS_VSOUT vs_PEFS(PEFS_VSIN v)
{
	PEFS_VSOUT o;

	o.pos = float4(v.pos.x, v.pos.y, 0.5f, 1.0f);
	o.uv = v.uv;
	
	return o;
}

// текстура с z-buffer'ом для x360
texture peDepthTexture;
sampler peDepthSampler =
sampler_state
{
    Texture = <peDepthTexture>;
    MipFilter = NONE;
    MinFilter = POINT;
    MagFilter = POINT;

	AddressU = Clamp;
	AddressV = Clamp;
};

// текстура с экраном до постэффектов
texture peOriginalScreenTexture;
sampler peOriginalScreenSampler =
sampler_state
{
    Texture = <peOriginalScreenTexture>;
    MipFilter = NONE;
    MinFilter = POINT;
    MagFilter = POINT;

	AddressU = Clamp;
	AddressV = Clamp;
};

sampler peOriginalScreenSamplerLinear =
sampler_state
{
    Texture = <peOriginalScreenTexture>;
    MipFilter = NONE;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};

texture peSourceTexture;
sampler peSourceSampler =
sampler_state
{
    Texture = <peSourceTexture>;

    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};
	
float4 peScreenSize;		// x,y = (width, height), z,w = 1.0f / (widht, height)
float4 peScreenSize2;		// x = sqrt(width^2 + height^2), 0, 0, 0
float4 peSharpness;			// center k, sides k, 0, 0
float4 peGlowParams;		// power, threshold, foreFactor, backFactor
float4 peGlowParams2;		// 
float4 peRadialBlurParams;	// focus_power, power, scr_x, scr_y
float4 peGlowObjParams;		// per object - power, threshold, 0, 0
float4 peDOFParams;			// current Z (projected), current blur factor, 0.0f, 0.0f
float4 peDOFParams2;		// focusDistance, 1.0 / focusDistance, 1.0 / focusDistance, blurFactor

texture preGammaTexture;
sampler preGammaSampler =
sampler_state
{
    Texture = <preGammaTexture>;

    MipFilter = POINT;
    MinFilter = POINT;
    MagFilter = POINT;

	AddressU = Clamp;
	AddressV = Clamp;
};

float4 ps_GammaCorrection(PEFS_PSIN pnt) : COLOR
{
	float4 tex = tex2D(preGammaSampler, pnt.uv);
	return GammaCorrection(tex); // float4(pow(tex.rgb, GammaValue), tex.a);
}

Technique(GammaCorrectionTech, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = true;
		SrcBlend = one;
		DestBlend = zero;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_PEFS();
		PixelShader = compile ps_2_0 ps_GammaCorrection();
	}
}

float4 ps_PEFinal(PEFS_PSIN pnt) : COLOR
{
	float4 src = tex2D(peOriginalScreenSampler, pnt.uv);
	return float4(src.rgb, 0.0f);
}

Technique(PE_Final, PEFS_VSIN)
{
	pass P0
	{
		CullMode = none;
		ZEnable = false;
		AlphaTestEnable = false;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 vs_PEFS();
		PixelShader = compile ps_2_0 ps_PEFinal();
	}
}

#ifndef _XBOX
	float4 peToolsParams;		// nearU, widthU, nearV, heightV

	float4 ps_PEToolsWindow(PEFS_PSIN pnt) : COLOR
	{
		float4 test;
		// Сделано в лоб, все равно работает тока в редакторе
		test.x = (pnt.uv.x - peToolsParams.x < 0.0f) ? 0.0f : 1.0f;
		test.y = (pnt.uv.y - peToolsParams.z < 0.0f) ? 0.0f : 1.0f;
		test.z = (peToolsParams.y - pnt.uv.x < 0.0f) ? 0.0f : 1.0f;
		test.w = (peToolsParams.w - pnt.uv.y < 0.0f) ? 0.0f : 1.0f;

		clip(3.9f - dot(test, float4(1.0f, 1.0f, 1.0f, 1.0f)));

		float4 src = tex2D(peOriginalScreenSampler, pnt.uv);
		return float4(src.rgb, 0.0f);
	}

	Technique(PE_ToolsWindow, PEFS_VSIN)
	{
		pass P0
		{
			CullMode = none;
			ZEnable = false;
			AlphaTestEnable = false;
			AlphaBlendEnable = false;

			VertexShader = compile vs_2_0 vs_PEFS();
			PixelShader = compile ps_2_0 ps_PEToolsWindow();
		}
	}
#endif