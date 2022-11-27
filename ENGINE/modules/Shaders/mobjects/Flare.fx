#include "technique.h"

#include "stdVariables.h"


float  flare_u_texel_size;
float  flare_v_texel_size;
float4 flare_color;

texture FlareTexture;
sampler FlareSampler =
sampler_state
{
    Texture = <FlareTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = Clamp;
	AddressV = Clamp;
};

texture FlareMaskTex;
sampler FlareMaskSampler =
sampler_state
{
    Texture = <FlareMaskTex>;
    MinFilter = Point;
    MagFilter = Point;
    AddressU = Clamp;
	AddressV = Clamp;
};

texture FlareSecMaskTex;
sampler FlareSecMaskSampler =
sampler_state
{
    Texture = <FlareSecMaskTex>;
    MinFilter = Point;
    MagFilter = Point;
    AddressU = Clamp;
	AddressV = Clamp;
};


struct FlareVertex
{
	float3 pos : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
	float2 uv2: TEXCOORD1;
	float3 uv3: TEXCOORD2;
};

struct Flare_VS_OUTPUT
{
	float4 pos: POSITION;
	float4 color : COLOR0;
	float4 uv : TEXCOORD0;
};

struct Flare_PS_INPUT
{
	float4 color : COLOR0;
	float4 uv : TEXCOORD0;
};


Flare_VS_OUTPUT vs_FlareRectsMask( FlareVertex vrx)
{
	Flare_VS_OUTPUT res; 
	float4 pos = float4(vrx.uv3.x, vrx.uv3.y, vrx.uv3.z, 1); 
 
	res.pos = mul(pos, mWorldViewProj);
	res.color = vrx.color;
	res.uv.xy = vrx.uv;
	res.uv.zw = vrx.uv2;
	return res;
}

float4 ps_FlareRects(Flare_PS_INPUT pnt) : COLOR
{ 
	return flare_color;
}

Flare_VS_OUTPUT vs_FlareRects9( FlareVertex vrx)
{
	Flare_VS_OUTPUT res;
	res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
	res.color = vrx.color;
	res.uv.xy = vrx.uv;
	res.uv.zw = vrx.uv2;
	return res;
}

float4 ps_FlareRects9(Flare_PS_INPUT pnt) : COLOR
{
	float4 tex = tex2D(FlareSampler, pnt.uv.xy);

	float pow = tex2D(FlareMaskSampler, pnt.uv.zw).r;

	return tex * pnt.color * pow;
}


Technique(FlareNoZ, FlareVertex)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		srcblend = srcalpha;
		destblend = one;		
		
		VertexShader = compile vs_2_0 vs_FlareRectsMask();
		PixelShader = compile ps_2_0 ps_FlareRects();
	}
}

Technique(FlareMask, FlareVertex)
{
	pass P0
	{
		ZEnable = true;
        ZWriteEnable = false;
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		srcblend = srcalpha;
		destblend = one;		
		
		VertexShader = compile vs_2_0 vs_FlareRectsMask();
		PixelShader = compile ps_2_0 ps_FlareRects();
	}
}

Technique(Flare9, FlareVertex)
{
	pass P0
	{
		ZEnable = false;
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		srcblend = srcalpha;
		destblend = one;
		
		VertexShader = compile vs_2_0 vs_FlareRects9();
		PixelShader = compile ps_2_0 ps_FlareRects9();
	}
}









struct Flare2DVertex
{
	float3 pos   : POSITION; 
	float3 wpos  : TEXCOORD0;		// world pos 
	float3 uv    : TEXCOORD1;
	float2 uv2   : TEXCOORD2;
};

struct Flare2DVS_OUTPUT
{
	float4 pos   : POSITION; 
	float4 uv    : TEXCOORD0;		// uv.w - posttransform Z value
	float2 uv2   : TEXCOORD1;
};

struct Flare2DPS_INPUT
{ 
	float4 uv    : TEXCOORD0;		// uv.w - posttransform Z value
	float2 uv2   : TEXCOORD1;
};


Flare2DVS_OUTPUT vs_Flare2D( Flare2DVertex vrx)
{
	Flare2DVS_OUTPUT res;
	res.pos = float4(vrx.pos, 1); 
	res.uv.xyz = vrx.uv;

	float4 wpos = mul( float4(vrx.wpos, 1.0f), mProjection);
	res.uv.w = wpos.z / wpos.w;

	res.uv2 = vrx.uv2;
	return res;
}

float4 ps_Flare2D(Flare2DPS_INPUT pnt) : COLOR
{
	float4 tex = tex2D(FlareMaskSampler, pnt.uv);
	return tex; 
}

float4 ps_Flare2D9Mask(Flare2DPS_INPUT pnt) : COLOR
{
	float pow = tex2D(FlareMaskSampler, float2(pnt.uv.x + flare_u_texel_size, pnt.uv.y - flare_v_texel_size)).r +
				 tex2D(FlareMaskSampler, float2(pnt.uv.x + flare_u_texel_size, pnt.uv.y)).r + 
				 tex2D(FlareMaskSampler, float2(pnt.uv.x + flare_u_texel_size, pnt.uv.y + flare_v_texel_size)).r + 
			 
				 tex2D(FlareMaskSampler, float2(pnt.uv.x, pnt.uv.y - flare_v_texel_size)).r +
				 tex2D(FlareMaskSampler, float2(pnt.uv.x, pnt.uv.y)).r +
				 tex2D(FlareMaskSampler, float2(pnt.uv.x, pnt.uv.y + flare_v_texel_size)).r + 
			 
				 tex2D(FlareMaskSampler, float2(pnt.uv.x - flare_u_texel_size, pnt.uv.y - flare_v_texel_size)).r + 
				 tex2D(FlareMaskSampler, float2(pnt.uv.x - flare_u_texel_size, pnt.uv.y)).r +
				 tex2D(FlareMaskSampler, float2(pnt.uv.x - flare_u_texel_size, pnt.uv.y + flare_v_texel_size)).r;
			 
	pow *= 0.11f; 
     
	return saturate(tex2D(FlareSecMaskSampler, pnt.uv2).r + (pow - (1.0f - (pow>0))) * pnt.uv.z); 
}

Technique(Flare2D, Flare2DVertex)
{
	pass P0
	{
		AlphaBlendEnable = false;
		VertexShader = compile vs_2_0 vs_Flare2D();
		PixelShader = compile ps_2_0 ps_Flare2D();
	}
}

Technique(Flare2D9Mask, Flare2DVertex)
{
	pass P0
	{
		AlphaBlendEnable = false;    
		VertexShader = compile vs_2_0 vs_Flare2D();
		PixelShader = compile ps_2_0 ps_Flare2D9Mask();
	}
}

#ifdef _XBOX
	float4 ps_XBFastFlare2D9Mask(Flare2DPS_INPUT pnt) : COLOR
	{
		static const float2 uv = float2(flare_u_texel_size, flare_v_texel_size);
		const float ztest = pnt.uv.w;

		float pos;
		
		pos  = ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x + uv.x, pnt.uv.y - uv.y)).r;
  		pos += ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x + uv.x, pnt.uv.y)).r;
		pos += ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x + uv.x, pnt.uv.y + uv.y)).r;
				 
		pos += ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x, pnt.uv.y - uv.y)).r;
		pos += ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x, pnt.uv.y)).r;
		pos += ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x, pnt.uv.y + uv.y)).r;
				 
		pos += ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x - uv.x, pnt.uv.y - uv.y)).r;
		pos += ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x - uv.x, pnt.uv.y)).r;
		pos += ztest <= tex2D(FlareMaskSampler, float2(pnt.uv.x - uv.x, pnt.uv.y + uv.y)).r;
				 
		pos *= 0.11f; 
	     
		return saturate(tex2D(FlareSecMaskSampler, pnt.uv2).r + (pos - (1.0f - (pos>0))) * pnt.uv.z); 
	}


	Technique(XBFast_Flare2D9Mask, Flare2DVertex)
	{
		pass P0
		{
			AlphaBlendEnable = false;    
			VertexShader = compile vs_2_0 vs_Flare2D();
			PixelShader = compile ps_2_0 ps_XBFastFlare2D9Mask();
		}
	}
#endif