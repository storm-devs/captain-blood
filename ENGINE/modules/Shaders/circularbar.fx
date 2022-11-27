#include "technique.h"

texture CircularTexture;

sampler CircularSampler =
sampler_state
{
	Texture = <CircularTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct Circular_VS_IN
{
	float3 pos : POSITION0;
	float2 tuv : TEXCOORD1;
	float  alp : TEXCOORD2;
};

struct Circular_VS_OUT
{
	float4 pos : POSITION0;
	float2 tuv : TEXCOORD1;
	float  alp : TEXCOORD2;
};

struct Circular_PS_IN
{
	float2 tuv : TEXCOORD1;
	float  alp : TEXCOORD2;
};

Circular_VS_OUT vs_Circular(Circular_VS_IN v)
{
	Circular_VS_OUT o;

	o.pos = float4(v.pos,1.0);
	o.tuv = v.tuv;
	o.alp = v.alp;

	return o;
}

float4 ps_Circular(Circular_PS_IN v) : COLOR0
{
	half4 pixel = tex2D(CircularSampler,v.tuv);

	pixel.a *= v.alp;

	return pixel;
}

Technique(Circular,Circular_VS_IN)
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

		VertexShader = compile vs_2_0 vs_Circular();
		PixelShader = compile ps_2_0 ps_Circular();
	}
}

float4 ps_Circular_shadow(Circular_PS_IN v) : COLOR0
{
	half4 pixel = tex2D(CircularSampler,v.tuv);

	pixel.a *= v.alp;
//	pixel.a *= 0.3;

	pixel.rgb = 0.0;

	return pixel;
}

Technique(Circular_shadow,Circular_VS_IN)
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

		VertexShader = compile vs_2_0 vs_Circular();
		PixelShader = compile ps_2_0 ps_Circular_shadow();
	}
}




//---------------------------------------------------------------------

#include "stdVariables.h"


float4 ls_ps_Circular(Circular_PS_IN v) : COLOR0
{
	half4 pixel = tex2D(CircularSampler,v.tuv);
	pixel.a *= v.alp;
	return GammaCorrection(pixel);
}


float4 ls_ps_Circular_shadow(Circular_PS_IN v) : COLOR0
{
	half4 pixel = tex2D(CircularSampler,v.tuv);
	pixel.a *= v.alp;
	pixel.rgb = 0.0;
	return GammaCorrection(pixel);
}



Technique(ls_Circular,Circular_VS_IN)
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

		VertexShader = compile vs_2_0 vs_Circular();
		PixelShader = compile ps_2_0 ls_ps_Circular();
	}
}


Technique(ls_Circular_shadow,Circular_VS_IN)
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

		VertexShader = compile vs_2_0 vs_Circular();
		PixelShader = compile ps_2_0 ls_ps_Circular_shadow();
	}
}
