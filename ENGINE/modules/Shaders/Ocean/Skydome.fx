#include "technique.h"
struct SkyDomeVertex
{
	float3 pos : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
};

struct SkyDomeVS_OUT
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct SkyDomePS_IN
{
	float2 uv : TEXCOORD0;
};

SkyDomeVS_OUT vs_SkyDome(SkyDomeVertex v)
{
	SkyDomeVS_OUT res;
	res.pos = float4(v.pos, 1);
	res.uv = v.uv;
	return res;
}

#define PI 3.14159265358f

float3 SkyDome_GetFromUV(float2 uv)
{
	float u = uv.x;
	float v = uv.y;

	float angx, angy;
	if (u <= 0.5f)
	{
		angy = acos(1.0f - 8.0f * u * u);
	}
	else
	{
		u = 1.0f - u;
		angy = 3.14159265358f * 2.0f - acos(1.0f - 8.0f * u * u);
	}

	angx = -acos(1.0f - v * v);

	float sinAx = sin(angx);
	float cosAx = cos(angx);
	float sinAy = sin(-angy - PI / 2.0f);
	float cosAy = cos(-angy - PI / 2.0f);
	
	//Создаём матрицу с порядком вращений rz*rx*ry
	float3 vec;
	vec.x = cosAx*sinAy;
	vec.y = -sinAx;	
	vec.z = cosAx*cosAy;

	return vec;
}

// =================================================================
//
// =================================================================

#define SKYDOME_NX	0
#define SKYDOME_PX	1
#define SKYDOME_PY	2
#define SKYDOME_NZ	3
#define SKYDOME_PZ	4
bool SkyDomeBools[32];

texture SkyDomeTexture;

sampler SkyDomeSampler =
sampler_state
{
    Texture = <SkyDomeTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};

float4 ps_SkyDome2(SkyDomePS_IN vv) : COLOR
{
	float3 vec = SkyDome_GetFromUV(vv.uv);

	float3 avec;
	avec = abs(vec);

	float4 result = -4.0f;

	if (avec.x > avec.y && avec.x > avec.z)
	{
		if (vec.x < 0)
		{
			if (SkyDomeBools[SKYDOME_NX])
			{
				float s = 0.5f - vec.z / (2.0f * vec.x);
				float t = 0.5f + vec.y / (2.0f * vec.x);
				result = tex2D(SkyDomeSampler, float2(s, t));		// NX
			}
		}
		else
		{
			if (SkyDomeBools[SKYDOME_PX])
			{
				float s = 0.5f - vec.z / (2.0f * vec.x);
				float t = 0.5f - vec.y / (2.0f * vec.x);
				result = tex2D(SkyDomeSampler, float2(s, t));		// PX
			}
		}
	}
	else if (avec.y > avec.x && avec.y > avec.z)
	{
		if (vec.y >= 0)
		{
			if (SkyDomeBools[SKYDOME_PY])
			{
				float s = 0.5f + vec.x / (2.0f * vec.y);
				float t = 0.5f + vec.z / (2.0f * vec.y);
				result = tex2D(SkyDomeSampler, float2(s, t));		// PY
			}
		}
	}
	else
	{
		if (vec.z < 0)
		{
			if (SkyDomeBools[SKYDOME_NZ])
			{
				float s = 0.5f + vec.x / (2.0f * vec.z);
				float t = 0.5f + vec.y / (2.0f * vec.z);
				result = tex2D(SkyDomeSampler, float2(s, t));		// NZ
			}
		}
		else
		{
			if (SkyDomeBools[SKYDOME_PZ])
			{
				float s = 0.5f + vec.x / (2.0f * vec.z);
				float t = 0.5f - vec.y / (2.0f * vec.z);
				result = tex2D(SkyDomeSampler, float2(s, t));		// PZ
			}
		}
	}

	clip(result.x + 2.0f);
	return result;
}

Technique(Create_SkyDome2, SkyDomeVertex)
{
	pass P0
	{
	    CullMode = none;
		ZEnable = false;
		AlphaTestEnable = false;
		AlphaBlendEnable = false;
   
		VertexShader = compile vs_3_0 vs_SkyDome();
		PixelShader = compile ps_3_0 ps_SkyDome2();
	}
}

