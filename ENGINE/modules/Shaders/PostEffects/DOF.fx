#include "technique.h"
#include "PostEffects\common.h"
#include "stdVariables.h"

PEFS_VSOUT vs_DOFZ(PEFS_VSIN v)
{
	PEFS_VSOUT o;

	o.pos = float4(v.pos.x, v.pos.y, peDOFParams.x, 1.0f);
	o.uv = v.uv;
	
	return o;
}

float4 ps_DOFZ(PEFS_PSIN pnt) : COLOR
{
	float alpha = peDOFParams.y;
	return float4(0.0f, 0.0f, 0.0f, alpha);
}

Technique(PE_DOFZNear, PEFS_VSIN)
{
	pass P0
	{
		CullMode = none;
		AlphaTestEnable = false;
		AlphaBlendEnable = false;
		ZWriteEnable = false;
		ColorWriteEnable = alpha;
		ZFunc = greater;

		VertexShader = compile vs_2_0 vs_DOFZ();
		PixelShader = compile ps_2_0 ps_DOFZ();
	}
}

Technique(PE_DOFZFar, PEFS_VSIN)
{
	pass P0
	{
		CullMode = none;
		AlphaTestEnable = false;
		AlphaBlendEnable = false;
		ZWriteEnable = false;
		ColorWriteEnable = alpha;
		ZFunc = less;

		VertexShader = compile vs_2_0 vs_DOFZ();
		PixelShader = compile ps_2_0 ps_DOFZ();
	}
}

/*float3 DoFPoissonFilter(in sampler srcTex, in float3 src, in float2 uv, in float2 invTexelSize, in float radius)
{
	float2 taps[16] = {
		float2( -0.94201624, -0.39906216 ),		float2( 0.94558609, -0.76890725 ),
		float2( -0.094184101, -0.92938870 ),	float2( 0.34495938, 0.29387760 ),
		float2( -0.91588581, 0.45771432 ),		float2( -0.81544232, -0.87912464 ),
		float2( -0.38277543, 0.27676845 ),		float2( 0.97484398, 0.75648379 ),
		float2( 0.44323325, -0.97511554 ),		float2( 0.53742981, -0.47373420 ),
		float2( -0.26496911, -0.41893023 ),		float2( 0.79197514, 0.19090188 ),
		float2( -0.24188840, 0.99706507 ),		float2( -0.81409955, 0.91437590 ),
		float2( 0.19984126, 0.78641367 ),		float2( 0.14383161, -0.14100790 ) };

	float3 result = src;

	[unroll]
	for (int i=0; i<16; i++)
	{
		float4 tmp = tex2D(srcTex, uv + invTexelSize * taps[i] * radius);
		result += tmp;
	}
	
	return result * (1.0f / 17.0f); // Return average
}*/

float4 ps_PEDOF(PEFS_PSIN pnt) : COLOR
{
	float4 src = tex2D(peOriginalScreenSamplerLinear, pnt.uv);
	float4 tapLo = tex2D(peSourceSampler, pnt.uv);

	return lerp(src, tapLo, src.a);
}

Technique(PE_DOF, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_PEDOF();
	}
}

// ==============
// Восстанавливаем Z значение и рисуем в альфу коэффициент для DOF'a
// 
#ifdef _XBOX

float4 ps_PEDOFCalculateAlpha(PEFS_PSIN pnt) : COLOR
{
	float depth = tex2D(peDepthSampler, pnt.uv).r;

	const static float m32 = mProjection[3][2];
	const static float m22 = mProjection[2][2];

	float dist = m32 * (1.0f + m22 / (depth - m22));

	/*float k1 = (dist - peDOFParams2.x) * peDOFParams2.y;
	float k2 = (peDOFParams2.x - dist) * peDOFParams2.z;
	float kf = (dist >= peDOFParams2.x) ? k1 : k2;
	kf = min(1.0f, kf);*/

	static const float4 peDOFLimiter = peDOFParams2.x * float4(1.1765f, 0.2857f, 7.0f, 0.6967f);
	static const float4 peDOF_FarCoeff = float4(0.0f, 0.61f, 0.757f, 1.0f);
	static const float4 peDOF_NearCoeff = float4(0.0f, 0.26f, -0.05f, 1.0f);
	static const float peDOF_DividerFar = peDOFParams2.y / (peDOFLimiter.z - peDOFLimiter.x);
	static const float peDOF_DividerNear = peDOFParams2.z / (peDOFLimiter.w - peDOFLimiter.y);
	float2 distances;
	distances = max(float2(dist, dist), peDOFLimiter.xy);
	distances = min(distances, peDOFLimiter.zw);
	float t = (dist >= peDOFParams2.x) ? (distances.x - peDOFLimiter.x) * peDOF_DividerFar :
										 (peDOFLimiter.w - distances.y) * peDOF_DividerNear;
	t = min(1.0f, t);
	float4 p = (dist >= peDOFParams2.x) ? peDOF_FarCoeff : peDOF_NearCoeff;
	float i = 1.0f - t;
	float v = 3.0f*i* (i*t * p.y + t*t * p.z) + t*t*t * p.w;

	return float4(0.0f, 0.0f, 0.0f, v * peDOFParams2.w);
	//return float4(0.0f, 0.0f, 0.0f, kf * kf * peDOFParams2.w);
}

Technique(PE_DOFCalculateAlpha, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		ColorWriteEnable = alpha;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_PEDOFCalculateAlpha();
	}
}

#endif

//	======================
//  Ресайз в 4 раза 
//

float4 peDOFUV[24];
float4 ps_dofScale(PEFS_PSIN pnt) : COLOR
{
	/*float4 t0 = 0.0f;

	float4 src = tex2D(peOriginalScreenSamplerLinear, pnt.uv);

	[unroll]
	for (int i=0; i<24; i++)
	{
		float4 tmp = tex2D(peOriginalScreenSamplerLinear, pnt.uv + peDOFUV[i].xy);
		t0 += lerp(src, tmp, tmp.a);
	}

	return t0 * (1.0f / 25.0f);*/

	float du = peScreenSize.z;
	float dv = peScreenSize.w;

	float4 tmp;
	float4 res = 0.0f;

#ifdef _XBOX
	float4 t0, t1, t2, t3;

	float2 uv = pnt.uv;

	asm 
	{
		tfetch2D t0, uv, peOriginalScreenSampler, OffsetX = -1.5, OffsetY = -1.5
		tfetch2D t1, uv, peOriginalScreenSampler, OffsetX = -0.5, OffsetY = -1.5
		tfetch2D t2, uv, peOriginalScreenSampler, OffsetX =  0.5, OffsetY = -1.5
		tfetch2D t3, uv, peOriginalScreenSampler, OffsetX =  1.5, OffsetY = -1.5
	};
	res += t0 * float4(t0.aaa, 1.0f);
	res += t1 * float4(t1.aaa, 1.0f);
	res += t2 * float4(t2.aaa, 1.0f);
	res += t3 * float4(t3.aaa, 1.0f);

	asm 
	{
		tfetch2D t0, uv, peOriginalScreenSampler, OffsetX = -1.5, OffsetY = -0.5
		tfetch2D t1, uv, peOriginalScreenSampler, OffsetX = -0.5, OffsetY = -0.5
		tfetch2D t2, uv, peOriginalScreenSampler, OffsetX =  0.5, OffsetY = -0.5
		tfetch2D t3, uv, peOriginalScreenSampler, OffsetX =  1.5, OffsetY = -0.5
	};
	res += t0 * float4(t0.aaa, 1.0f);
	res += t1 * float4(t1.aaa, 1.0f);
	res += t2 * float4(t2.aaa, 1.0f);
	res += t3 * float4(t3.aaa, 1.0f);

	asm 
	{
		tfetch2D t0, uv, peOriginalScreenSampler, OffsetX = -1.5, OffsetY =  0.5
		tfetch2D t1, uv, peOriginalScreenSampler, OffsetX = -0.5, OffsetY =  0.5
		tfetch2D t2, uv, peOriginalScreenSampler, OffsetX =  0.5, OffsetY =  0.5
		tfetch2D t3, uv, peOriginalScreenSampler, OffsetX =  1.5, OffsetY =  0.5
	};
	res += t0 * float4(t0.aaa, 1.0f);
	res += t1 * float4(t1.aaa, 1.0f);
	res += t2 * float4(t2.aaa, 1.0f);
	res += t3 * float4(t3.aaa, 1.0f);

	asm 
	{
		tfetch2D t0, uv, peOriginalScreenSampler, OffsetX = -1.5, OffsetY =  1.5
		tfetch2D t1, uv, peOriginalScreenSampler, OffsetX = -0.5, OffsetY =  1.5
		tfetch2D t2, uv, peOriginalScreenSampler, OffsetX =  0.5, OffsetY =  1.5
		tfetch2D t3, uv, peOriginalScreenSampler, OffsetX =  1.5, OffsetY =  1.5
	};
	res += t0 * float4(t0.aaa, 1.0f);
	res += t1 * float4(t1.aaa, 1.0f);
	res += t2 * float4(t2.aaa, 1.0f);
	res += t3 * float4(t3.aaa, 1.0f);

#else
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -1.5f, dv * -1.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -0.5f, dv * -1.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  0.5f, dv * -1.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  1.5f, dv * -1.5f));	res += tmp * float4(tmp.aaa, 1.0f);

	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -1.5f, dv * -0.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -0.5f, dv * -0.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  0.5f, dv * -0.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  1.5f, dv * -0.5f));	res += tmp * float4(tmp.aaa, 1.0f);

	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -1.5f, dv *  0.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -0.5f, dv *  0.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  0.5f, dv *  0.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  1.5f, dv *  0.5f));	res += tmp * float4(tmp.aaa, 1.0f);

	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -1.5f, dv *  1.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du * -0.5f, dv *  1.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  0.5f, dv *  1.5f));	res += tmp * float4(tmp.aaa, 1.0f);
	tmp = tex2D(peOriginalScreenSampler, pnt.uv + float2(du *  1.5f, dv *  1.5f));	res += tmp * float4(tmp.aaa, 1.0f);
#endif

	return float4(res.rgb / max(res.a, 0.0000001f), res.a * (1.0f / 16.0f));
}

Technique(PE_DOFScale, PEFS_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_PEFS();
		PixelShader = compile ps_3_0 ps_dofScale();
	}
}
