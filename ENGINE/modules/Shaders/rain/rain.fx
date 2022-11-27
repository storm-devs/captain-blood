#include "technique.h"
#include "stdVariables.h"


float3 RainCameraPos;
float4 RainColor;
texture RainTexture;

sampler RainSampler =
sampler_state
{
    Texture = <RainTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};



struct RainWeather_VS_IN
{
	float3 vPos : POSITION;
	float2 UV1 : BLENDWEIGHT0;		// become into short2
};

struct RainWeather_VS_OUT
{
	float4 Position: POSITION0;
	float2 UV1 : TEXCOORD1;
};


struct RainWeather_PS_IN
{
	float2 UV1 : TEXCOORD1;
};



RainWeather_VS_OUT vs_RainWeather(RainWeather_VS_IN v)
{
	RainWeather_VS_OUT o;
	o.Position = mul(float4(v.vPos, 1.0), mWorldViewProj);
	o.UV1 = v.UV1 * float2(1.0f / 32767.0f, 1.0f / 32767.0f);
	return o;
}


float4 ps_RainWeather(RainWeather_PS_IN v) : COLOR0
{
 half4 pixel = tex2D(RainSampler, v.UV1) * RainColor;
 return pixel; 
}






Technique(RainWeather, RainWeather_VS_IN)
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

		VertexShader = compile vs_2_0 vs_RainWeather();
		PixelShader = compile ps_2_0 ps_RainWeather();
	}
}

