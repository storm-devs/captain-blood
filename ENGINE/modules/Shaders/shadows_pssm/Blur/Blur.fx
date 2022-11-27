#include "technique.h"
float shdsprite_TexSize;

texture shdsprite_Texture;
sampler shdsprite_Sampler =
sampler_state
{
    Texture = <shdsprite_Texture>;
    
    MinFilter = point;
    MagFilter = point;
};

struct ShdSpriteVertex
{
	float3 pos : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
};

struct shdsprite_VS_OUTPUT
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct shdsprite_PS_INPUT
{
	float2 uv : TEXCOORD0;
};

shdsprite_VS_OUTPUT vs_shdsprite(ShdSpriteVertex vrx)
{
	shdsprite_VS_OUTPUT res;
	res.pos = float4(vrx.pos, 1);
	res.uv = vrx.uv;
	return res;
}

float4 ps_shdsprite(shdsprite_PS_INPUT pnt) : COLOR
{
	const static float d = shdsprite_TexSize;
	//float4 tex = tex2D(shdsprite_Sampler, pnt.uv) * 0.64;

	float4 tex = tex2D(shdsprite_Sampler, pnt.uv);
	float4 tex1 = tex2D(shdsprite_Sampler, pnt.uv + float2(d, 0));
	float4 tex2 = tex2D(shdsprite_Sampler, pnt.uv + float2(0, d));

	tex = min(min(tex, tex1), tex2);

	//float _dot = dot(tex.rgb, float3(1.0f, 1.0f, 1.0f));
	//float _dot1 = dot(tex1.rgb, float3(1.0f, 1.0f, 1.0f));
	//float _dot2 = dot(tex2.rgb, float3(1.0f, 1.0f, 1.0f));

	//tex = min(tex, min(tex1, tex2));

	//if ((_dot >= 1.99f) && (_dot1 * _dot2 < 3.0f))
	//{
		//tex = (tex1 + tex2) * 0.5f;
	//}

	/*#define KERNEL_SIZE 9
	
	float kernel[KERNEL_SIZE];
	float2 offset[KERNEL_SIZE];

	offset[0] = float2(-d, -d);
	offset[1] = float2(0.0, -d);
	offset[2] = float2(d, -d);
   
	offset[3] = float2(-d, 0.0);
	offset[4] = float2(0.0, 0.0);
	offset[5] = float2(d, 0.0);
   
	offset[6] = float2(-d, d);
	offset[7] = float2(0.0, d);
	offset[8] = float2(d, d);
   
	kernel[0] = 1.0/16.0; 	kernel[1] = 2.0/16.0;	kernel[2] = 1.0/16.0;
	kernel[3] = 2.0/16.0;	kernel[4] = 4.0/16.0;	kernel[5] = 2.0/16.0;
	kernel[6] = 1.0/16.0;  	kernel[7] = 2.0/16.0;	kernel[8] = 1.0/16.0;

	float4 tex = float4(0.0, 0.0, 0.0, 0.0);

	for (int i=0; i<KERNEL_SIZE; i++ )
	{
		float4 tmp = tex2D(shdsprite_Sampler, pnt.uv + offset[i]);
		tex += tmp * kernel[i];
	}*/

	//tex += 0.09 * tex2D(shdsprite_Sampler, pnt.uv + float2(0, d));
	//tex += 0.09 * tex2D(shdsprite_Sampler, pnt.uv + float2(0, -d));
	//tex += 0.09 * tex2D(shdsprite_Sampler, pnt.uv + float2(d, 0));
	//tex += 0.09 * tex2D(shdsprite_Sampler, pnt.uv + float2(-d, 0));

	//tex += 0.025 * tex2D(shdsprite_Sampler, pnt.uv + float2(d, d));
	//tex += 0.025 * tex2D(shdsprite_Sampler, pnt.uv + float2(d, -d));
	//tex += 0.025 * tex2D(shdsprite_Sampler, pnt.uv + float2(d, d));
	//tex += 0.025 * tex2D(shdsprite_Sampler, pnt.uv + float2(-d, d));
	
	//return float4(0.0f, 1.0f, 0.0f, 0.5f);
	return tex;
}

Technique(shdsrv_Blur, ShdSpriteVertex)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_shdsprite();
		PixelShader = compile ps_2_0 ps_shdsprite();
	}
}
