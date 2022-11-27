#include "technique.h"
#include "PostEffects\common.h"

float4 ps_PEGlow(PEFS_PSIN pnt) : COLOR
{
	float4 src = tex2D(peOriginalScreenSampler, pnt.uv);

	float4 glow = tex2D(peSourceSampler, pnt.uv);
	//glow.rgb = lerp(glow.rgb, glow.aaa, peGlowParams2.x);
	float k = glow.a;
	k = k * 2.0f - 1.0f;
	k = k * k * k;
	k = k * 0.5f + 0.5f;

	return float4(src.rgb + peGlowParams.x * lerp(glow.rgb, k, peGlowParams2.x), src.a);
}

Technique(PE_Glow, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_PEGlow();
	}
}

// ===============
// Уменьшение картинки в 4 раза 
//

float4 ps_resize4x4Glow(PEFS_PSIN pnt) : COLOR
{
	float4 t0, t1, t2, t3;
	float du = peScreenSize.z;
	float dv = peScreenSize.w;

	t0 = tex2D(peOriginalScreenSamplerLinear, pnt.uv + float2(-du, -dv));
	t1 = tex2D(peOriginalScreenSamplerLinear, pnt.uv + float2(du, -dv));
	t2 = tex2D(peOriginalScreenSamplerLinear, pnt.uv + float2(-du, dv));
	t3 = tex2D(peOriginalScreenSamplerLinear, pnt.uv + float2(du, dv));

	float4 res = (t0 + t1 + t2 + t3) * 0.25f;

	res.rgb = res.rgb * peGlowParams.z + res.rgb * peGlowParams.w * res.a * 4.0f;//((res.a > 0.0f) ? 1.0f : 0.0f);

	float max_component = max(max(res.r, res.g), res.b);

	res.rgb = (max_component < peGlowParams.y) ? 0.0f : res.rgb;

	res.a = length(res.rgb) * 0.57735;

	return res;
}

float4 ps_resize4x4GlowHQ(PEFS_PSIN pnt) : COLOR
{
	float4 t0, t1, t2, t3;

	float4 tt[16];

#ifndef _XBOX
	float du = peScreenSize.z;
	float dv = peScreenSize.w;

	tt[0] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -1.5f, dv * -1.5f));
	tt[1] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -0.5f, dv * -1.5f));
	tt[2] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  0.5f, dv * -1.5f));
	tt[3] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  1.5f, dv * -1.5f));

	tt[4] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -1.5f, dv * -0.5f));
	tt[5] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -0.5f, dv * -0.5f));
	tt[6] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  0.5f, dv * -0.5f));
	tt[7] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  1.5f, dv * -0.5f));

	tt[8] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -1.5f, dv *  0.5f));
	tt[9] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -0.5f, dv *  0.5f));
	tt[10] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  0.5f, dv *  0.5f));
	tt[11] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  1.5f, dv *  0.5f));


	tt[12] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -1.5f, dv *  1.5f));
	tt[13] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -0.5f, dv *  1.5f));
	tt[14] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  0.5f, dv *  1.5f));
	tt[15] = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  1.5f, dv *  1.5f));
#else
	float2 uv = pnt.uv;

	asm 
	{
		tfetch2D tt[0], uv, peOriginalScreenSampler, OffsetX = -1.5, OffsetY = -1.5
		tfetch2D tt[1], uv, peOriginalScreenSampler, OffsetX = -0.5, OffsetY = -1.5
		tfetch2D tt[2], uv, peOriginalScreenSampler, OffsetX =  0.5, OffsetY = -1.5
		tfetch2D tt[3], uv, peOriginalScreenSampler, OffsetX =  1.5, OffsetY = -1.5

		tfetch2D tt[4], uv, peOriginalScreenSampler, OffsetX = -1.5, OffsetY = -0.5
		tfetch2D tt[5], uv, peOriginalScreenSampler, OffsetX = -0.5, OffsetY = -0.5
		tfetch2D tt[6], uv, peOriginalScreenSampler, OffsetX =  0.5, OffsetY = -0.5
		tfetch2D tt[7], uv, peOriginalScreenSampler, OffsetX =  1.5, OffsetY = -0.5

		tfetch2D tt[8], uv, peOriginalScreenSampler, OffsetX = -1.5, OffsetY = 0.5
		tfetch2D tt[9], uv, peOriginalScreenSampler, OffsetX = -0.5, OffsetY = 0.5
		tfetch2D tt[10], uv, peOriginalScreenSampler, OffsetX =  0.5, OffsetY = 0.5
		tfetch2D tt[11], uv, peOriginalScreenSampler, OffsetX =  1.5, OffsetY = 0.5

		tfetch2D tt[12], uv, peOriginalScreenSampler, OffsetX = -1.5, OffsetY = 1.5
		tfetch2D tt[13], uv, peOriginalScreenSampler, OffsetX = -0.5, OffsetY = 1.5
		tfetch2D tt[14], uv, peOriginalScreenSampler, OffsetX =  0.5, OffsetY = 1.5
		tfetch2D tt[15], uv, peOriginalScreenSampler, OffsetX =  1.5, OffsetY = 1.5
	};
#endif

	float4 res = 0.0f;

	[unroll]
	for (int i=0; i<16; i++)
	{
		float3 tmp = tt[i] * (peGlowParams.z + peGlowParams.w * tt[i].a * 4.0f);

		float max_component = max(max(tmp.r, tmp.g), tmp.b);
		tmp.rgb = (max_component < peGlowParams.y) ? 0.0f : tmp.rgb;
		
		res.rgb += tmp;
	}

	res.rgb = res * (1.0f / 16.0f);
	res.a = length(res.rgb) * 0.57735;

	return res;
}

Technique(PE_Resize4x4Glow, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_resize4x4Glow();
	}
}

Technique(PE_Resize4x4GlowHQ, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_resize4x4GlowHQ();
	}
}