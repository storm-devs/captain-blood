#include "technique.h"
#include "ocean\ocean.h"

OceanVSOut vs_Ocean(OceanVSIn v)
{
	OceanVSOut o;

	float l = dot(v.vPos, v.vPos);
	float k = ocean2FakeHeight.x * saturate((l - (500.0f * 500.0f)) * (1.0f / (2500.0f * 2500.0f)));

	float3 vPos = v.vPos + float3(ocean2CamPos.x, k, ocean2CamPos.z);
	
	float4 pos = mul(float4(vPos, 1.0), mWorldViewProj);
	o.vPos = pos;
	o.pos = pos * float4(0.5f, -0.5f, 1.0f, 1.0f);
	
	return o;
}

OceanVSOut vs_OceanFlatNoZ(OceanVSIn v)
{
	OceanVSOut o;

	float l = dot(v.vPos, v.vPos);
	float k = ocean2FakeHeight.x * saturate((l - (500.0f * 500.0f)) * (1.0f / (2500.0f * 2500.0f)));

	float3 vPos = v.vPos + float3(ocean2CamPos.x, k, ocean2CamPos.z);

	float4 pos = mul(float4(vPos, 1.0), mWorldViewProj);
	o.vPos = pos;
	o.pos = pos * float4(0.5f, -0.5f, 1.0f, 1.0f);
	
	return o;
}

float4 ocean3uv05;		// 0.5 + 0.5f/texWidth, 0.5 + 0.5f/texHeight

float4 ps_Ocean(OceanPSIn p) : COLOR
{
	// FIX-ME переделать на tex2DProj и посчитать матрицу
	//float4 pos = mul(p.pos, mWorldViewProj);
	float4 oceanTex = tex2D(Ocean2RTSampler, p.pos.xy / p.pos.w + ocean3uv05.xy);//float2(0.5f, 0.5f));
	//clip(oceanTex.a - 1.0f);
	return float4(oceanTex.rgb, 1.0);
}

float4 ps_BadWater(OceanPSIn p) : COLOR
{
	return float4(1.0f, 0.0f, 0.0f , 1.0);
}

float4 ps_OceanflatNoZ(OceanPSIn p) : COLOR
{
	// FIX-ME переделать на tex2DProj и посчитать матрицу
	//float4 pos = mul(p.pos, mWorldViewProj);
	float2 uv = p.pos.xy / p.pos.w + ocean3uv05.xy;//float2(0.5f, 0.5f);
	float4 oceanTex0 = tex2D(Ocean2RTSampler, uv);
	//float4 oceanTex1 = tex2D(Ocean2RTSampler, uv - float2(2.5f / 1152.0f, 2.5f / 640.0f));
	//float4 oceanTex2 = tex2D(Ocean2RTSampler, uv + float2(2.5f / 1152.0f, 2.5f / 640.0f));
	//float4 oceanTex3 = tex2D(Ocean2RTSampler, uv + float2(5.5f / 1152.0f, 5.5f / 640.0f));
	//float4 res = max(max(oceanTex0, oceanTex1), max(oceanTex2, oceanTex3));
	//return float4(res.rgb, 1.0);
	return float4(oceanTex0.rgb, 1.0f);
}

Technique(Ocean, OceanVSIn)
{
	pass P0
	{
		CullMode = none;
		ZEnable = true;
		ZWriteEnable = true;
	
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 vs_Ocean();
		PixelShader = compile ps_2_0 ps_Ocean();
	}
}

Technique(Ocean2_flatNoZ, OceanVSIn)
{
	pass P0
	{
		CullMode = none;
		ZWriteEnable = false;
	
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 vs_OceanFlatNoZ();
		PixelShader = compile ps_2_0 ps_OceanflatNoZ();
	}
}

Technique(BadWater, OceanVSIn)
{
	pass P0
	{
		CullMode = none;
	
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_2_0 vs_Ocean();
		PixelShader = compile ps_2_0 ps_BadWater();
	}
}

struct Ocean_FullScreenQuad_VSIN
{
	float4 pos	: POSITION;
	float2 uv	: TEXCOORD0;
};

struct Ocean_FullScreenQuad_VSOUT
{
	float4 pos	: POSITION;
	float2 uv	: TEXCOORD0;
};

struct Ocean_FullScreenQuad_PSIN
{
	float2 uv	: TEXCOORD0;
};

float4  ocean2RestoreColorParams;			// x, y, dx, dy
texture ocean2ScreenCopyTexture;

sampler ocean2ScreenCopySampler =
sampler_state
{
    Texture = <ocean2ScreenCopyTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

Ocean_FullScreenQuad_VSOUT vs_Ocean2ScreenCopy(Ocean_FullScreenQuad_VSIN v)
{
	Ocean_FullScreenQuad_VSOUT o;

	o.pos = float4(v.pos.x, v.pos.y, 0.5f, 1.0f);
	o.uv = ocean2RestoreColorParams.xy + v.uv.xy * ocean2RestoreColorParams.zw;
	
	return o;
}

float4 ps_Ocean2ScreenCopy(Ocean_FullScreenQuad_PSIN pnt) : COLOR
{
	float4 color = tex2D(ocean2ScreenCopySampler, pnt.uv);
	return color;
}

Technique(Ocean2ScreenCopy, Ocean_FullScreenQuad_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_Ocean2ScreenCopy();
		PixelShader = compile ps_2_0 ps_Ocean2ScreenCopy();
	}
}

float4 ps_Ocean3ClearAlpha(Ocean_FullScreenQuad_PSIN pnt) : COLOR
{
	return float4(1.0f, 1.0f, 1.0f, 0.0f);
}

Technique(Ocean3ClearAlpha, Ocean_FullScreenQuad_VSIN)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = true;
		SrcBlend = zero;
		DestBlend = SrcColor;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_Ocean2ScreenCopy();
		PixelShader = compile ps_2_0 ps_Ocean3ClearAlpha();
	}
}

float4 ps_Ocean3RefractAlpha(OceanPSIn p) : COLOR
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

Technique(Ocean3RefractAlpha, OceanVSIn)
{
	pass P0
	{
		ZWriteEnable = false;
		CullMode = none;
		AlphaBlendEnable = true;
		SrcBlend = one;
		DestBlend = InvSrcColor;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_Ocean();
		PixelShader = compile ps_2_0 ps_Ocean3RefractAlpha();
	}
}

OceanVSOut vs_Ocean3ZPrePass(OceanVSIn v)
{
	OceanVSOut o;

	float3 vPos = v.vPos + float3(ocean2CamPos.x, 0.0f, ocean2CamPos.z);
	
	float4 pos = mul(float4(vPos, 1.0), mWorldViewProj);
	o.vPos = pos;
	o.pos = pos * float4(0.5f, -0.5f, 1.0f, 1.0f);
	
	return o;
}

#define ZPP_ALPHA	( 61.0f * (1.0f / 255.0f) )

float4 ps_Ocean3ZPrePassSetAlpha(OceanPSIn p) : COLOR
{
	return float4(0.0f, 0.0f, 0.0f, ZPP_ALPHA);
}

Technique(Ocean3ZPrePassSetAlpha, OceanVSIn)
{
	pass P0
	{
		ZWriteEnable = false;
		CullMode = none;
		AlphaBlendEnable = true;
		SrcBlend = one;
		DestBlend = InvSrcColor;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_Ocean();
		PixelShader = compile ps_2_0 ps_Ocean3ZPrePassSetAlpha();
	}
}

ZPIXEL ps_Ocean3ZPrePass(OceanPSIn p)
{
	float2 uv = p.pos.xy / p.pos.w + ocean3uv05.xy;

#ifdef _XBOX
	float curZ = p.pos.z / p.pos.w;
	float curDepth = tex2D(Ocean3DepthSampler, uv).r;
	clip(curDepth - curZ);
#else
	float alpha = tex2D(Ocean3DepthSampler, uv).a;
	clip(0.001961f - abs(alpha - ZPP_ALPHA));		// 0.001961f = 0.5 / 255.0
#endif

	ZPIXEL o;
  	o.Color = 0;
	o.Depth = 1.0f;
  	return o;
}

Technique(Ocean3ZPrePass, OceanVSIn)
{
	pass P0
	{
		ZEnable = true;
		ZWriteEnable = true;
		ZFunc = always;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		ColorWriteEnable = 0;

		VertexShader = compile vs_2_0 vs_Ocean3ZPrePass();
		PixelShader = compile ps_2_0 ps_Ocean3ZPrePass();
	}
}

// чистим RT + DEPTH_RT своими силами, чтобы на Xbox'e не убился HIZ из-за Clear()
ZPIXEL ps_Ocean3ClearRT_Depth(Ocean_FullScreenQuad_PSIN p)
{
	ZPIXEL o;
  	o.Color = 0;
	o.Depth = 1.0f;
  	return o;
}

Technique(Ocean3ClearRT_Depth, Ocean_FullScreenQuad_VSIN)
{
	pass P0
	{
		ZEnable = true;
		ZWriteEnable = true;
		ZFunc = always;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_Ocean2ScreenCopy();
		PixelShader = compile ps_2_0 ps_Ocean3ClearRT_Depth();
	}
}