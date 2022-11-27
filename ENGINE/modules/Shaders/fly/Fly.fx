#include "technique.h"
#include "stdVariables.h"

texture FlyTexture;

sampler FlySampler =
sampler_state
{
	Texture = <FlyTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

struct Fly_VS_IN
{
	float3 pos : POSITION0;
	float4 col : COLOR0;
	float2 tuv : TEXCOORD1;
	float3 glp : TEXCOORD2;
};

struct Fly_VS_OUT
{
	float4 pos : POSITION0;
	float2 tuv : TEXCOORD1;
	float4 col : TEXCOORD2;
};

struct Fly_PS_IN
{
	float2 tuv : TEXCOORD1;
	float4 col : TEXCOORD2;
};

Fly_VS_OUT vs_Fly(Fly_VS_IN v)
{
	Fly_VS_OUT o;

	float4x4 m = mInverseView; m[3] = 0.0;

	o.pos = mul(mul(float4(v.pos,1.0),m) + float4(v.glp,1.0),mWorldViewProj);
	o.tuv = v.tuv;
	o.col = v.col;

	return o;
}

float4 ps_Fly(Fly_PS_IN v) : COLOR0
{
	half4 pixel = tex2D(FlySampler,v.tuv)*v.col;

	return pixel;
}

Technique(Fly,Fly_VS_IN)
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

		VertexShader = compile vs_2_0 vs_Fly();
		PixelShader = compile ps_2_0 ps_Fly();
	}
}

#include "gmxservice\gmx_Variables.h"

struct Fly_VS_IN_
{
	float2 pos : POSITION0;
	float3 uva : TEXCOORD1;
	float3 glp : TEXCOORD2;
};

struct Fly_VS_OUT_
{
	float4 pos : POSITION0;
	float3 uva : TEXCOORD1;
};

struct Fly_PS_IN_
{
	float3 uva : TEXCOORD1;
};

Fly_VS_OUT_ vs_Fly_(Fly_VS_IN_ v)
{
	Fly_VS_OUT_ o;

	float4x4 m = mInverseView; m[3] = 0.0;

	o.pos = mul(mul(float4(v.pos,0.0,1.0),m) + float4(v.glp,1.0),mWorldViewProj);
	o.uva = v.uva;

	return o;
}

float4 ps_Fly_(Fly_PS_IN_ v) : COLOR0
{
	float4 pixel = tex2D(FlySampler,v.uva.xy);

//	pixel.rgb = 0.0;
	pixel.a *= v.uva.z;

	pixel.rgb = pixel.rgb*0.5*(vAmbientLight.xyz + 1.0) + 0.0;

	return pixel;
}

Technique(Fly_,Fly_VS_IN_)
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

		VertexShader = compile vs_2_0 vs_Fly_();
		PixelShader = compile ps_2_0 ps_Fly_();
	}
}
