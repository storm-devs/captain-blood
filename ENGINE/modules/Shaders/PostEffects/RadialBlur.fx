#include "technique.h"
#include "PostEffects\common.h"

float4 ps_PERadialBlur(PEFS_PSIN pnt) : COLOR
{
	float4 src = tex2D(peOriginalScreenSampler, pnt.uv);

	float2 dir = pnt.uv - peRadialBlurParams.zw;//float2(0.5f, 0.5f);
	float dir_len = length(dir);
	//dir /= dir_len;
	float k = pow(min(1.0f, dir_len * 1.414213f), peRadialBlurParams.x);

	dir *= k * peRadialBlurParams.y / peScreenSize2.x;//1468.0f;

	const static float sum = 1.0f / (10.0f + 4.0f * (5 + 4 + 3 + 2 + 1));

	src	 = 10.0 * sum * src;
	float dist = 1.0f;
	float ff = 5.0f;
	[unroll]
	for (int i=0; i<5; i++)
	{
		float4 t0 = tex2D(peOriginalScreenSamplerLinear, pnt.uv - dir);
		float4 t1 = tex2D(peOriginalScreenSamplerLinear, pnt.uv - dir * 2.0f);
		float4 t2 = tex2D(peOriginalScreenSamplerLinear, pnt.uv - dir * 3.0f);
		float4 t3 = tex2D(peOriginalScreenSamplerLinear, pnt.uv - dir * 4.0f);
		
		src += ff * sum * (t0 + t1 + t2 + t3);

		pnt.uv -= dir * 4.0f;

		ff -= 1.0f;
	}
	return src;
}

Technique(PE_RadialBlur, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_PERadialBlur();
	}
}

