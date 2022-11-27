#include "technique.h"

float histogram_min;
float histogram_max;

texture histogram_source;
sampler histogram_source_sampler =
sampler_state
{
    Texture = <histogram_source>;
    MinFilter = Point;
    MagFilter = Point;
};

struct histogram_FillScreen_Vertex
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct histogram_FillScreen_VS_OUTPUT
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
};

histogram_FillScreen_VS_OUTPUT histogram_FillScreen_VS(histogram_FillScreen_Vertex v)
{
	histogram_FillScreen_VS_OUTPUT res;
	res.pos = v.pos;
	res.uv = v.uv;
	return res;
}

float4 histogram_FillScreen_Tex_PS(float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(histogram_source_sampler, uv);
	float brightness = res.r * 0.299f + res.g * 0.587f + res.b * 0.114f;
	
	res.a = 1.0f;
	
	if (brightness < histogram_min)
	{
	 res.a = 0.0f;
	}

	if (brightness > histogram_max)
	{
	 res.a = 0.0f;
	}
	
	return res;
}


float4 histogram_FillScreen_Tex_PS_Dark(float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(histogram_source_sampler, uv);
	float brightness = res.r * 0.299f + res.g * 0.587f + res.b * 0.114f;
	
	
	res.a = 0.0f;
	
	if (brightness < 0.06)
	{
	 
	 res.a = 1.0f;
	}

	
	return float4 (1.0, 0.0, 0.0, res.a);
}

float4 histogram_FillScreen_Tex_PS_Light(float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(histogram_source_sampler, uv);
	float brightness = res.r * 0.299f + res.g * 0.587f + res.b * 0.114f;
	

	res.a = 0.0f;
	
	if (brightness > 0.945)
	{
	 
	 res.a = 1.0f;
	}


	
	return float4 (1.0, 0.0, 0.0, res.a);
}




Technique(BuildHistogram, histogram_FillScreen_Vertex)
{
	pass P0
	{
		CullMode = none;
		ZWriteEnable = false;

		AlphaBlendEnable = false;
		AlphaTestEnable = true;
		AlphaRef = 1;

		VertexShader = compile vs_2_0 histogram_FillScreen_VS();
		PixelShader = compile ps_2_0 histogram_FillScreen_Tex_PS();
	}
}

//ShowDarkAreas
//ShowLightAreas



Technique(ShowDarkAreas, histogram_FillScreen_Vertex)
{
	pass P0
	{
		CullMode = none;
		ZWriteEnable = false;

		AlphaBlendEnable = false;
		AlphaTestEnable = true;
		AlphaRef = 1;

		VertexShader = compile vs_2_0 histogram_FillScreen_VS();
		PixelShader = compile ps_2_0 histogram_FillScreen_Tex_PS_Dark();
	}
}

Technique(ShowLightAreas, histogram_FillScreen_Vertex)
{
	pass P0
	{
		CullMode = none;
		ZWriteEnable = false;

		AlphaBlendEnable = false;
		AlphaTestEnable = true;
		AlphaRef = 1;

		VertexShader = compile vs_2_0 histogram_FillScreen_VS();
		PixelShader = compile ps_2_0 histogram_FillScreen_Tex_PS_Light();
	}
}
