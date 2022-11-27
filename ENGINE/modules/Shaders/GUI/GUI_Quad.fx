#include "technique.h"
#ifndef _XBOX

texture guiQuadTexture;
sampler	guiQuadSampler = sampler_state
{
	Texture = <guiQuadTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

//	AddressU = Wrap;
//	AddressV = Wrap;
	AddressU = Clamp;
	AddressV = Clamp;
};
struct VS_IN
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct VS_OUT
{
	float4 Pos : POSITION0;
	float2 TexCoord : TEXCOORD0;
	float4 Color : TEXCOORD1;
};

VS_OUT vsGUI_Quad(VS_IN IN)
{
	VS_OUT OUT;

	OUT.Pos = float4(IN.pos.xyz, 1.0f);
	OUT.TexCoord = IN.tex;
	
	float color = floor(IN.pos.w / 10);
	float alpha = fmod(IN.pos.w, 10);
	OUT.Color = float4( color, color, color, alpha );

	return OUT;
}


float4 psGUI_Quad(VS_OUT IN) : COLOR0
{
	return tex2D(guiQuadSampler, IN.TexCoord)*IN.Color;
}

Technique(GUI_Quad, VS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		ScissorTestEnable = true;
		AlphaRef = 1;
	 
		ZEnable = false;
		ZWriteEnable = false;
		
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		VertexShader = compile vs_2_0 vsGUI_Quad();
		PixelShader = compile ps_2_0 psGUI_Quad();
	}
}

float QuadBlackScale;

float4 psGUI_Quad_Black(VS_OUT IN) : COLOR0
{
	float4 b = tex2D(guiQuadSampler,IN.TexCoord);
	float4 c = b;

	c.rgb = dot(c.rgb,float3(0.3,0.59,0.11));
//	c.rgb = length(c.rgb)*0.57735;

//	return c*IN.Color;
	return lerp(b,c,QuadBlackScale)*IN.Color;
}

Technique(GUI_Quad_Black, VS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		ScissorTestEnable = true;
		AlphaRef = 1;
	 
		ZEnable = false;
		ZWriteEnable = false;
		
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		VertexShader = compile vs_2_0 vsGUI_Quad();
		PixelShader = compile ps_2_0 psGUI_Quad_Black();
	}
}

#endif