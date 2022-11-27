#include "technique.h"
#include "gmxservice\gmx_Variables.h"
#include "stdVariables.h"

texture TrailTexture;
texture TrailTexture_BM;

sampler TrailSampler =
sampler_state
{
	Texture = <TrailTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler TrailSampler_BM =
sampler_state
{
	Texture = <TrailTexture_BM>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
/*
struct Trail_VS_IN
{
//	float3 pos : POSITION0;
	float2 tuv : TEXCOORD1;
//	float4 col : TEXCOORD2;
	float4 col : COLOR0;
	float3 glp : TEXCOORD3;	// global position
	float  rot : TEXCOORD4; // rotate angle
};
*/
struct Trail_VS_IN
{
	float3 glp : POSITION0;
	float4 col : COLOR0;
	float4 tar : TEXCOORD0;
};

struct Trail_VS_OUT
{
	float4 pos : POSITION0;
	float2 tuv : TEXCOORD1;
	float4 col : TEXCOORD2;
	float4 nor : TEXCOORD3; // normal
	float4 dir : TEXCOORD4; // light direction
};

struct Trail_PS_IN
{
	float2 tuv : TEXCOORD1;
	float4 col : TEXCOORD2;
	float4 nor : TEXCOORD3;
	float4 dir : TEXCOORD4;
};

Trail_VS_OUT vs_Trail(Trail_VS_IN v)
{
	Trail_VS_OUT o;

	float _sin;
	float _cos;

	sincos(v.tar.z,_sin,_cos);

/*	float3x3 RZ;

	RZ[0] = float3( _cos,-_sin, 0);
	RZ[1] = float3( _sin, _cos, 0);
	RZ[2] = float3(	   0,	 0,	1);

//	float4 pos = float4(mul(v.pos,RZ),1.0);
	float4 pos = float4(mul(float3(v.tar.w,0.0,0.0),RZ),1.0);*/

	float4 pos = float4(float3(_cos,_sin,0.0)*v.tar.w,1.0);

	float4x4 m = mInverseView;

	m[3] = 0.0;

	o.pos = mul(mul(pos,m) + float4(v.glp,1.0),mWorldViewProj);

//	o.tuv = v.tuv;
	o.tuv = v.tar.xy;

	o.col = v.col;

	o.nor = normalize(mul(v.glp - vCamPos,mView));
	o.dir = -normalize(mul(vGlobalLightDirection.xyz,mView));

	return o;
}

float4 ps_Trail(Trail_PS_IN v) : COLOR0
{
	half4 pixel = tex2D(TrailSampler,v.tuv)*v.col;

	float2 bm = 2*(tex2D(TrailSampler_BM,v.tuv) - 0.5);

	float3 n = v.nor.xyz;
	//float4 n = normalize(v.nor);

	n.x -= bm.x*1.5;
	n.y -= bm.y*1.5;

//	float k = 0.5*dot(normalize(n),-normalize(v.dir.xyz)) + 0.5;

//	k = 0.5 + k*0.5;
//	k = 0.6 + k*0.4;

	float k = dot(normalize(n), v.dir.xyz);

	k = (k > 0.0) ? 1.0f : min(1.0,1.3f + k*0.8);

	pixel.rgb *= k;

	return pixel;
}

Technique(Trail,Trail_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;
		ZEnable = true;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		VertexShader = compile vs_2_0 vs_Trail();
		PixelShader = compile ps_2_0 ps_Trail();
	}
}
