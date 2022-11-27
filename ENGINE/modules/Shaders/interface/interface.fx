#include "technique.h"
#include "stdVariables.h"
#include "DEBUG\dbgLine.fx"

Line_VS_OUTPUT vs_GizmoLineNoZ(LineVertex vrx)
{
	Line_VS_OUTPUT res;

	res.pos = mul(float4(vrx.pos, 1),mWorldViewProj);
	res.color = vrx.color;

	return res;
}

float4 ps_GizmoLineNoZ(Line_PS_INPUT pnt) : COLOR
{
	return pnt.color;
}

Technique(GizmoLineNoZ, LineVertex)
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;

		AlphaBlendEnable = false;

		CullMode = none;

		VertexShader = compile vs_2_0 vs_GizmoLineNoZ();
		PixelShader = compile ps_2_0 ps_GizmoLineNoZ();
	}
}

////////////////////////////////////////

Line_VS_OUTPUT vs_LocatorLine(LineVertex vrx)
{
	Line_VS_OUTPUT res;

	res.pos = mul(float4(vrx.pos, 1),mWorldViewProj);
	res.color = vrx.color;

	return res;
}

float4 ps_LocatorLine(Line_PS_INPUT pnt) : COLOR
{
	return pnt.color;
}

Technique(LocatorLine, LineVertex)
{
	pass P0
	{
		ZEnable = true;
		ZWriteEnable = false;

		AlphaBlendEnable = false;

		CullMode = none;

		VertexShader = compile vs_2_0 vs_LocatorLine();
		PixelShader = compile ps_2_0 ps_LocatorLine();
	}
}

float4 ps_LocatorLineNoZ(Line_PS_INPUT pnt) : COLOR
{
	return float4(pnt.color.rgb,0.5);
}

Technique(LocatorLineNoZ, LineVertex)
{
	pass P0
	{
		ZEnable = false;
		ZWriteEnable = false;

		AlphaBlendEnable = true;

		CullMode = none;

		VertexShader = compile vs_2_0 vs_LocatorLine();
		PixelShader = compile ps_2_0 ps_LocatorLineNoZ();
	}
}

////////////////////////////////////////

texture interfaceTexture;
sampler	interfaceSampler = sampler_state
{
	Texture = <interfaceTexture>;
	MipFilter = Linear;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct interfaceVS_IN
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct interfaceVS_OUT
{
	float4 Pos : POSITION0;
	float2 TexCoord : TEXCOORD0;
	float4 Color : TEXCOORD1;
};

interfaceVS_OUT vs_interfaceQuad(interfaceVS_IN IN)
{
	interfaceVS_OUT OUT;

	OUT.Pos = float4(IN.pos.xyz, 1.0f);
	OUT.TexCoord = IN.tex;
	
	float color = floor(IN.pos.w / 10);
	float alpha = fmod(IN.pos.w, 10);
	OUT.Color = float4( color, color, color, alpha );

	return OUT;
}

float4 ps_interfaceQuad(interfaceVS_OUT IN) : COLOR0
{
	return tex2D(interfaceSampler, IN.TexCoord)*IN.Color;
}

Technique(interfaceQuad, interfaceVS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
	//	ScissorTestEnable = true;
		AlphaRef = 1;
	 
		ZEnable = false;
		ZWriteEnable = false;
		
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		VertexShader = compile vs_2_0 vs_interfaceQuad();
		PixelShader = compile ps_2_0 ps_interfaceQuad();
	}
}

float interfaceBlackScale;

float4 ps_interfaceQuad_Black(interfaceVS_OUT IN) : COLOR0
{
	clip(1.0 - IN.TexCoord);

	float4 b = tex2D(interfaceSampler,IN.TexCoord);
	float4 c = b;

	c.rgb = dot(c.rgb,float3(0.3,0.59,0.11));
//	c.rgb = length(c.rgb)*0.57735;

//	return c*IN.Color;
	return lerp(b,c,interfaceBlackScale)*IN.Color;
}

Technique(interfaceQuad_Black, interfaceVS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
	//	ScissorTestEnable = true;
		AlphaRef = 1;
	 
		ZEnable = false;
		ZWriteEnable = false;
		
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		VertexShader = compile vs_2_0 vs_interfaceQuad();
		PixelShader = compile ps_2_0 ps_interfaceQuad_Black();
	}
}

