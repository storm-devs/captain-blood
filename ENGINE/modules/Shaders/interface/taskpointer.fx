#include "technique.h"


texture TaskPointer_Texture;
sampler TaskPointer_Sampler =
sampler_state
{
    Texture = <TaskPointer_Texture>;
    MinFilter = Linear;
    MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct TaskPointer_Vertex
{
	float4 pos : POSITION;
};

struct TaskPointer_VS_OUTPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

TaskPointer_VS_OUTPUT TaskPointer_VS(TaskPointer_Vertex v)
{
	TaskPointer_VS_OUTPUT res;
	res.pos = float4(v.pos.x*2.0 - 1.0 + (1.0 - v.pos.z)*2.0f, 1.0 - v.pos.y*2.0, 0.0, 1.0);
	res.color = float4(1.0, 1.0, 1.0, v.pos.w);
	res.uv = float2(v.pos.x, v.pos.y);
	return res;
}

float4 TaskPointer_PS(float4 color : COLOR, float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(TaskPointer_Sampler, uv);
	res = res*color;
	return res;
}

float4 TaskPointerMask_PS(float4 color : COLOR, float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(TaskPointer_Sampler, uv);
	return res;
}


Technique(Interface_TaskpointerPicture, TaskPointer_Vertex)
{
	pass P0
	{
		CullMode = none;
		ZEnable = false;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 TaskPointer_VS();
		PixelShader = compile ps_2_0 TaskPointer_PS();
	}
}

Technique(Interface_TaskpointerPictureMask, TaskPointer_Vertex)
{
	pass P0
	{
		CullMode = none;
		ZEnable = true;
		ZWriteEnable = true;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		AlphaFunc = less;
		AlphaRef = 0x80;

		ColorWriteEnable = 0;

		VertexShader = compile vs_2_0 TaskPointer_VS();
		PixelShader = compile ps_2_0 TaskPointerMask_PS();
	}
}

