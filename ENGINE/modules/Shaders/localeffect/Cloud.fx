#include "technique.h"
#include "gmxservice\gmx_Variables.h"
#include "stdVariables.h"

texture CloudTexture;

sampler CloudSampler =
sampler_state
{
	Texture = <CloudTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

struct Cloud_VS_IN
{
	float3 pos : POSITION0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float  ble : TEXCOORD3;
	float4 col : TEXCOORD4;
	float  rot : TEXCOORD5;
	float3 glp : TEXCOORD6;
};

struct Cloud_VS_OUT
{
	float4 pos : POSITION0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float  ble : TEXCOORD3;
	float4 col : TEXCOORD4;
};

struct Cloud_PS_IN
{
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float  ble : TEXCOORD3;
	float4 col : TEXCOORD4;
};

Cloud_VS_OUT vs_Cloud(Cloud_VS_IN v)
{
	Cloud_VS_OUT o;

	float _sin;
	float _cos;

	sincos(v.rot,_sin,_cos);

	float3x3 RZ;

	RZ[0] = float3( _cos,-_sin, 0);
	RZ[1] = float3( _sin, _cos, 0);
	RZ[2] = float3(	   0,	 0,	1);

	float4 pos = float4(mul(v.pos,RZ),1.0);

	float4x4 m = mInverseView;

	m[3] = 0.0;

	o.pos = mul(mul(pos,m) + float4(v.glp,1.0),mWorldViewProj);

	o.uv1 = v.uv1;
	o.uv2 = v.uv2;

	o.ble = v.ble;
	o.col = v.col;

	return o;
}

float4 ps_Cloud(Cloud_PS_IN v) : COLOR0
{
	float4 pixel = lerp(
		tex2D(CloudSampler,v.uv1),
		tex2D(CloudSampler,v.uv2),v.ble);

	pixel *= v.col;

	return pixel;
}

Technique(Cloud,Cloud_VS_IN)
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

		VertexShader = compile vs_2_0 vs_Cloud();
		PixelShader = compile ps_2_0 ps_Cloud();
	}
}
