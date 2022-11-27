#include "technique.h"

#include "stdVariables.h"

//----------------------------------------------------------------------------------------------------

struct AnxEditorDrawFill_Vertex
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct AnxEditorDrawFill_VS_OUTPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

AnxEditorDrawFill_VS_OUTPUT AnxEditorDrawFill_VS(AnxEditorDrawFill_Vertex v)
{
	AnxEditorDrawFill_VS_OUTPUT res;
	res.pos = mul(float4(v.pos, 1), mWorldViewProj);
	res.color = v.color;
	return res;
}

float4 AnxEditorDrawFill_PS(float4 color : COLOR) : COLOR
{
	return color;
}


Technique(AnxScreenDrawFill, AnxEditorDrawFill_Vertex)
{
	pass P0
	{
		zenable = false;

		CullMode = none;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 AnxEditorDrawFill_VS();
		PixelShader = compile ps_2_0 AnxEditorDrawFill_PS();
	}
}


//----------------------------------------------------------------------------------------------------

texture AnxEditorDrawTexture;
sampler AnxEditorDrawTexSampler =
sampler_state
{
    Texture = <AnxEditorDrawTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
};

struct AnxEditorDrawTex_Vertex
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};


struct AnxEditorDrawTex_VS_OUTPUT
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
};

AnxEditorDrawTex_VS_OUTPUT AnxEditorDrawTex_VS(AnxEditorDrawTex_Vertex v)
{
	AnxEditorDrawTex_VS_OUTPUT res;
	res.pos = mul(float4(v.pos, 1), mWorldViewProj);
	res.uv = v.uv;
	return res;
}

float4 AnxEditorDrawTex_PS(float2 uv : TEXCOORD) : COLOR
{
	float4 tex = tex2D(AnxEditorDrawTexSampler, uv);
	return tex;
}

Technique(AnxScreenDrawTexture, AnxEditorDrawTex_Vertex)
{
	pass P0
	{
		zenable = false;

		CullMode = none;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 AnxEditorDrawTex_VS();
		PixelShader = compile ps_2_0 AnxEditorDrawTex_PS();
	}
}

