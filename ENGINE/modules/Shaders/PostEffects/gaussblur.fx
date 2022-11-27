#include "technique.h"
#include "PostEffects\common.h"

float4 peBlurWeights[25];

float4 ps_gaussblur25H(PEFS_PSIN pnt) : COLOR
{
	float4 t0 = 0.0f;

	float du = peScreenSize.z * 4.0f;

	[unroll]
	for (int i=-12; i<=12; i++)
	{
		t0 += peBlurWeights[i + 12] * tex2D(peSourceSampler, pnt.uv + float2(du * float(i), 0.0f));
	}

    return t0;
}

float4 ps_gaussblur25V(PEFS_PSIN pnt) : COLOR
{
	float4 t0 = 0.0f;

	float dv = peScreenSize.w * 4.0f;

	[unroll]
	for (int i=-12; i<=12; i++)
	{
		t0 += peBlurWeights[i + 12] * tex2D(peSourceSampler, pnt.uv + float2(0.0f, dv * float(i)));
	}

    return t0;
}

Technique(PE_GaussBlur25H, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_gaussblur25H();
	}
}

Technique(PE_GaussBlur25V, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_gaussblur25V();
	}
}

//	============
//
//


float4 ps_gaussblur7H(PEFS_PSIN pnt) : COLOR
{
	float du = peScreenSize.z * 4.0f;

	float4 src = 0.0f;
	float4 t0 = tex2D(peSourceSampler, pnt.uv + float2(du * -2.0f, 0.0f));
	float4 t1 = tex2D(peSourceSampler, pnt.uv + float2(du * -1.0f, 0.0f));
	float4 t2 = tex2D(peSourceSampler, pnt.uv);
	float4 t3 = tex2D(peSourceSampler, pnt.uv + float2(du *  1.0f, 0.0f));
	float4 t4 = tex2D(peSourceSampler, pnt.uv + float2(du *  2.0f, 0.0f));

	src += 0.10f * t0 * float4(t0.aaa, 1.0f);
	src += 0.25f * t1 * float4(t1.aaa, 1.0f);
	src += 0.50f * t2 * float4(t2.aaa, 1.0f);
	src += 0.25f * t3 * float4(t3.aaa, 1.0f);
	src += 0.10f * t4 * float4(t4.aaa, 1.0f);

	return float4(src.rgb / max(0.0000001f, src.a), src.a);
}

float4 ps_gaussblur7V(PEFS_PSIN pnt) : COLOR
{
	float dv = peScreenSize.w * 4.0f;

	float4 src = 0.0f;
	float4 t0 = tex2D(peSourceSampler, pnt.uv + float2(0.0f, dv * -2.0f));
	float4 t1 = tex2D(peSourceSampler, pnt.uv + float2(0.0f, dv * -1.0f));
	float4 t2 = tex2D(peSourceSampler, pnt.uv);
	float4 t3 = tex2D(peSourceSampler, pnt.uv + float2(0.0f, dv *  1.0f));
	float4 t4 = tex2D(peSourceSampler, pnt.uv + float2(0.0f, dv *  2.0f));

	src += 0.12f * t0 * float4(t0.aaa, 1.0f);
	src += 0.25f * t1 * float4(t1.aaa, 1.0f);
	src += 0.40f * t2 * float4(t2.aaa, 1.0f);
	src += 0.25f * t3 * float4(t3.aaa, 1.0f);
	src += 0.12f * t4 * float4(t4.aaa, 1.0f);

	return src / max(0.0000001f, src.a);
}

Technique(PE_GaussBlur7H, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_gaussblur7H();
	}
}

Technique(PE_GaussBlur7V, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_gaussblur7V();
	}
}
