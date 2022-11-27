#include "technique.h"

#include "stdVariables.h"

struct ShowDetector_Vertex
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct ShowDetector_VS_OUTPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

ShowDetector_VS_OUTPUT ShowDetector_VS(ShowDetector_Vertex v)
{
	ShowDetector_VS_OUTPUT res;
	res.pos = mul(float4(v.pos, 1), mWorldViewProj);
	res.color = v.color;
	return res;
}

float4 ShowDetector_PS(float4 color : COLOR) : COLOR
{
	return color;
}

Technique(ShowDetector, ShowDetector_Vertex)
{
	pass P0
	{
		zenable = true;
		zwriteenable = false;

		CullMode = ccw;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 ShowDetector_VS();
		PixelShader = compile ps_2_0 ShowDetector_PS();
	}
}


