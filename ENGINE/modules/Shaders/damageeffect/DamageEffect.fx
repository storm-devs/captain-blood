#include "technique.h"
#include "stdVariables.h"

texture DamageTexture;

sampler DamageSampler =
sampler_state
{
	Texture = <DamageTexture>;
	MinFilter = Linear;
	MagFilter = Linear;
};

float Alpha;

struct DamageEffect_VS_IN
{
	float2 pos : POSITION0;
	float4 col : TEXCOORD1;
};

struct DamageEffect_VS_OUT
{
	float4 pos : POSITION0;
	float4 col : TEXCOORD1;
	float2 tuv : TEXCOORD2;
};

struct DamageEffect_PS_IN
{
	float4 col : TEXCOORD1;
	float2 tuv : TEXCOORD2;
};

DamageEffect_VS_OUT DamageEffect_VS(DamageEffect_VS_IN v)
{
	DamageEffect_VS_OUT o;

	float2 pos = v.pos;

	pos.x = pos.x*2.0 - 1.0;
	pos.y = 1.0 - pos.y*2.0;

	o.pos = float4(pos,0.0,1.0);
	o.col = v.col;
	o.tuv = v.pos.xy;

	return o;
}

float4 DamageEffect_PS(DamageEffect_PS_IN v) : COLOR0
{
	float4 col; float alpha = Alpha;

	if( alpha < 0.0 )
	{
		col = 1.0; alpha = -alpha;
	}
	else
		col = tex2D(DamageSampler,v.tuv);

	col	  *= v.col;
	col.a *= alpha;

	return col;
}

Technique(DamageEffect,DamageEffect_VS_IN)
{
	pass P0
	{
		CullMode = none;

		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		ZEnable = false;
		ZWriteenable = false;

		VertexShader = compile vs_2_0 DamageEffect_VS();
		PixelShader = compile ps_2_0 DamageEffect_PS();
	}
}
