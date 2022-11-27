#include "technique.h"
#include "PostEffects\common.h"

PEFS_VSOUT vs_PESharpness(PEFS_VSIN v)
{
	PEFS_VSOUT o;

	o.pos = float4(v.pos.x, v.pos.y, 1.0f, 1.0f);
	o.uv = v.uv;
	
	return o;
}

float4 CalcSharpness(in float4 src, float2 uv)
{
	float2 duv = peScreenSize.zw * 0.5f;

	float4 up		= tex2D(peOriginalScreenSamplerLinear, uv + float2( duv.x,  duv.y));
	float4 down		= tex2D(peOriginalScreenSamplerLinear, uv + float2(-duv.x,  duv.y));
	float4 left		= tex2D(peOriginalScreenSamplerLinear, uv + float2( duv.x, -duv.y));
	float4 right	= tex2D(peOriginalScreenSamplerLinear, uv + float2(-duv.x, -duv.y));

	float4 blur = (src + up + down + left + right) * 0.2f;
	float4 res = src - blur;
	res = sqrt(abs(res)) * sign(res);
	return src + res * 3.0f * peSharpness.x * 0.1f;
}

float4 ps_PESharpness(PEFS_PSIN pnt) : COLOR
{
	float4 src = tex2D(peOriginalScreenSampler, pnt.uv);
	float4 src2 = CalcSharpness(src, pnt.uv);

#ifdef _XBOX
	float depth = tex2D(peDepthSampler, pnt.uv);
	return (depth >= 0.9999f) ? float4(src.rgb, 0.0f) : float4(src2.rgb, 0.0f);
#else
    return float4(src2.rgb, 0.0f);
#endif
}

Technique(PE_Sharpness, PEFS_VSIN)
{
	pass P0
	{
#ifdef _XBOX
		ZEnable = false;
#else
		ZWriteEnable = false;
		ZFunc = greater;
#endif
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PESharpness();
		PixelShader = compile ps_3_0 ps_PESharpness();
	}
}

#ifdef _XBOX

struct PEZPRITE_PIXEL
{
    float4 Color : COLOR0;
    float  Depth : DEPTH0;
};

PEZPRITE_PIXEL ps_PESharpnessWithZ(PEFS_PSIN pnt)
{
	PEZPRITE_PIXEL o;

	float4 src = tex2D(peOriginalScreenSampler, pnt.uv);
	float4 src2 = CalcSharpness(src, pnt.uv);

	float depth = tex2D(peDepthSampler, pnt.uv);

	o.Color = (depth >= 0.9999f) ? float4(src.rgb, 0.0f) : float4(src2.rgb, 0.0f);
	o.Depth = depth;
	return o;
}

Technique(PE_SharpnessWithZ, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = true;
		ZFUNC = ALWAYS;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PESharpness();
		PixelShader = compile ps_3_0 ps_PESharpnessWithZ();
	}
}
#endif
