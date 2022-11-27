#include "technique.h"
#include "stdVariables.h"

float4 postProcess_FillScreen_Color;

texture postProcess_FillScreen_Texture;
sampler postProcess_FillScreen_Sampler =
sampler_state
{
    Texture = <postProcess_FillScreen_Texture>;
    MinFilter = Linear;
    MagFilter = Linear;
};

struct postProcess_FillScreen_Vertex
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct postProcess_FillScreen_VS_OUTPUT
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct postProcess_FillScreen_VS_OUTPUTALPHA
{
    float4 pos : POSITION;
    float3 uv_and_alpha : TEXCOORD;
    
};



postProcess_FillScreen_VS_OUTPUTALPHA postProcess_FillScreen_VS_Alpha(postProcess_FillScreen_Vertex v)
{
	postProcess_FillScreen_VS_OUTPUTALPHA res;
	res.pos = float4 (v.pos.xyz, 1.0);
	res.uv_and_alpha.xy = v.uv;
	
	//альфа
	res.uv_and_alpha.z = v.pos.w;
	return res;
}

float4 postProcess_FillScreen_Tex_PS_Alpha(float3 uv_and_alpha : TEXCOORD) : COLOR
{
	float4 res = tex2D(postProcess_FillScreen_Sampler, uv_and_alpha.xy);
	res = res*postProcess_FillScreen_Color;
	res.a = res.a * uv_and_alpha.z;
	return res;
}



postProcess_FillScreen_VS_OUTPUT postProcess_FillScreen_VS(postProcess_FillScreen_Vertex v)
{
	postProcess_FillScreen_VS_OUTPUT res;
	res.pos = v.pos;
	res.uv = v.uv;
	return res;
}

float4 postProcess_FillScreen_Tex_PS(float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(postProcess_FillScreen_Sampler, uv);
	res = res*postProcess_FillScreen_Color;
	return res;
}

float4 postProcess_FillScreen_Color_PS(float2 uv : TEXCOORD) : COLOR
{
	float4 res = postProcess_FillScreen_Color;
	return res;
}

Technique(PostProcessFillScreen_Texture, postProcess_FillScreen_Vertex)
{
	pass P0
	{
		CullMode = none;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 postProcess_FillScreen_VS();
		PixelShader = compile ps_2_0 postProcess_FillScreen_Tex_PS();
	}
}

Technique(PostProcessFillScreen_TextureVAlpha, postProcess_FillScreen_Vertex)
{
	pass P0
	{
		CullMode = none;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 postProcess_FillScreen_VS_Alpha();
		PixelShader = compile ps_2_0 postProcess_FillScreen_Tex_PS_Alpha();
	}
}


Technique(PostProcessFillScreen_Color, postProcess_FillScreen_Vertex)
{
	pass P0
	{
		CullMode = none;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		
		ZEnable = false;

		VertexShader = compile vs_2_0 postProcess_FillScreen_VS();
		PixelShader = compile ps_2_0 postProcess_FillScreen_Color_PS();
	}
}










//-------------------------------------------------------------------------------------------------





float4 ls_postProcess_FillScreen_Tex_PS(float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(postProcess_FillScreen_Sampler, uv);
	res = res*postProcess_FillScreen_Color;
	return GammaCorrection(res);
}

float4 ls_postProcess_FillScreen_Tex_PS_Alpha(float3 uv_and_alpha : TEXCOORD) : COLOR
{
	float4 res = tex2D(postProcess_FillScreen_Sampler, uv_and_alpha.xy);
	res = res*postProcess_FillScreen_Color;
	res.a = res.a * uv_and_alpha.z;
	return GammaCorrection(res);
}



Technique(ls_PostProcessFillScreen_Texture, postProcess_FillScreen_Vertex)
{
        pass P0
        {
                CullMode = none;

                SrcBlend = srcalpha;
                DestBlend = invsrcalpha;

                AlphaBlendEnable = true;
                AlphaTestEnable = false;

                VertexShader = compile vs_2_0 postProcess_FillScreen_VS();
                PixelShader = compile ps_2_0 ls_postProcess_FillScreen_Tex_PS();
        }
}

Technique(ls_PostProcessFillScreen_TextureVAlpha, postProcess_FillScreen_Vertex)
{
	pass P0
	{
		CullMode = none;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 postProcess_FillScreen_VS_Alpha();
		PixelShader = compile ps_2_0 ls_postProcess_FillScreen_Tex_PS_Alpha();
	}
}
