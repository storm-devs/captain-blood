#include "technique.h"

#include "font.fx"

//----------------------------------------------------------------------------------------------------

texture TaskViewer_Texture;

struct TaskViewer_Vertex
{
	float4 pos : POSITION;
};

struct TaskViewer_VS_OUTPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};


//----------------------------------------------------------------------------------------------------


TaskViewer_VS_OUTPUT TaskViewerMask_VS(TaskViewer_Vertex v)
{
	TaskViewer_VS_OUTPUT res;
	res.pos = float4(v.pos.x*2.0 - 1.0, 1.0 - v.pos.y*2.0, 0.0, 1.0);
	res.color = float4(1.0, 1.0, 1.0, v.pos.w);
	res.uv = float2(v.pos.x*0.25 - 0.25 + v.pos.z*0.5, v.pos.y);
	return res;
}

sampler TaskViewerC_Sampler =
sampler_state
{
    Texture = <TaskViewer_Texture>;
    MinFilter = Linear;
    MagFilter = Linear;
	AddressU = Wrap;
	AddressV = Clamp;
};

float4 TaskViewerMask_PS(float4 color : COLOR, float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(TaskViewerC_Sampler, uv);
	res = float4(1, 1, 1, 1) - res;
	return res;
}

Technique(Interface_TaskviewerPictureMask, TaskViewer_Vertex)
{
	pass P0
	{
		CullMode = none;
		ZEnable = true;
		ZWriteEnable = true;

		AlphaBlendEnable = false;
		AlphaTestEnable = true;

		ColorWriteEnable = 0;

		VertexShader = compile vs_2_0 TaskViewerMask_VS();
		PixelShader = compile ps_2_0 TaskViewerMask_PS();
	}
}

//----------------------------------------------------------------------------------------------------

TaskViewer_VS_OUTPUT TaskViewer_VS(TaskViewer_Vertex v)
{
	TaskViewer_VS_OUTPUT res;
	res.pos = float4(v.pos.x*2.0 - 1.0, 1.0 - v.pos.y*2.0, 0.5, 1.0);
	res.color = float4(1.0, 1.0, 1.0, v.pos.w);
	res.uv = float2(v.pos.x, v.pos.y);
	return res;
}

/*
sampler TaskViewerCC_Sampler =
sampler_state
{
    Texture = <TaskViewer_Texture>;
    MinFilter = Linear;
    MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};
*/


float4 TaskViewer_PS(float4 color : COLOR, float2 uv : TEXCOORD) : COLOR
{
	float4 res = tex2D(TaskViewerC_Sampler, clamp(uv, float2(0.0, 0.0), float2(1.0, 1.0)));
	res = res*color;
	return res;
}

Technique(Interface_TaskviewerPicture, TaskViewer_Vertex)
{
	pass P0
	{
		CullMode = none;
		ZEnable = false;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 TaskViewer_VS();
		PixelShader = compile ps_2_0 TaskViewer_PS();
	}
}

//----------------------------------------------------------------------------------------------------


Font_VS_OUTPUT TaskviewerFontVertexShader( FontVertex vrx)
{
	Font_VS_OUTPUT res;
	res.pos = float4(vrx.pos.x, vrx.pos.y, 0.5, 1);
	res.uv = vrx.uv;
	return res;
}



Technique(Interface_TaskviewerFont, FontVertex)
{
	pass P0
	{
		CullMode = none;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;
		ZEnable = false;
		AlphaRef = 1;
		

		VertexShader = compile vs_2_0 TaskviewerFontVertexShader();
		PixelShader = compile ps_2_0 FontPixelShader();
	}
}

