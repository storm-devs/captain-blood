#ifndef OCEAN_FX_HPP
#define OCEAN_FX_HPP

#include "stdVariables.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"

float  ocean2DistFade;			// 1.0f / distance_fade_height_to_zero
float4 ocean2CamPos;
float4 ocean2Shifts;			// posShift, texShift, 0, 0
float4 ocean2Wave1;				// scale1, amp1, scale1 * amp1, 0
float4 ocean2Wave2;				// scale2, amp2, scale2 * amp2, 0
float4 ocean2Bump;				// cos, sin, -sin, cos
float4 ocean2BumpScaleMove;		// scaleU, scaleV, moveU, moveV, 
float4 ocean2ReflTrans;			// refl_min, refl_max, trans_min, trans_max
float4 ocean2RefrParams;		// refr_power, refl_environment_power, refl_multiply, refl_color_multiply
float4 ocean2RefrParams2;		// refl_minus, 0, 0, 0
float4 ocean2Viewport;			// current viewport: 1/dx, 1/dy, 0, 0
//float4 ocean2RefrMapping;		// x, y, dx, dy
float4 ocean2SunRoadParams;		// xyz = (sun direction for sun road), w = sun road power
float4 ocean2SunLight;			// xyz = (sun direction for lighting), 0.0
float4 ocean2SunLightParams;	// x = dot_mul, y = add
float4 ocean2SunRoadColor;		// rgb, 1.0f
float4 ocean2FogParams;			// x - multiply, y - 1.0f / fog_distance, z - fogstart
float4 ocean2FogColor;
float4 ocean2FakeHeight;		// x - fake height
float4 Ocean2SkyRotate;			// переменная используется в Sky.cpp


#define OC2_SUNROAD_ENABLE 0
#define OC2_TRANSLUCENCE_ENABLE 1
#define OC2_FOAM_ENABLE 2
bool ocean2BoolParams[32];


float4 ocean2FoamParams1;	// x - foamStartY, y - foamHeightY, z - foamPowerY
float4 ocean2FoamParams2;	// x - foamUVScale, y - foamDisplacement, zw - start uv
float4 ocean2FoamColor;
float4 ocean2FoamRotate;	// xy, zw - 2x2 matrix

float4 ocean2FlatMapping;		// x, y, 1.0f / dx, 1.0f / dy
float4 ocean2SunBumpScale;
float4 ocean2BumpDistance;

texture Ocean2BumpTexture;
texture Ocean2ReflTexture;
texture Ocean2RefrTexture;
texture Ocean2FrenelTexture;
texture Ocean2FoamTexture;
texture Ocean2RTTexture;
texture Ocean2DMapTexture;
texture Ocean3DepthTexture;

sampler Ocean3DepthSampler =
sampler_state
{
    Texture = <Ocean3DepthTexture>;
    MipFilter = NONE;
    MinFilter = POINT;
    MagFilter = POINT;

	AddressU = Clamp;
	AddressV = Clamp;
};

sampler Ocean2FoamSampler = 
sampler_state
{
    Texture = <Ocean2FoamTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Wrap;
	AddressV = Wrap;
};

sampler Ocean2RTSampler = 
sampler_state
{
    Texture = <Ocean2RTTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler Ocean2DMapSampler = 
sampler_state
{
    Texture = <Ocean2DMapTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Border;
	AddressV = Border;
	BorderColor = 0x0;
};

sampler Ocean2FrenelSampler =
sampler_state
{
    Texture = <Ocean2FrenelTexture>;
    MipFilter = POINT;
    MinFilter = POINT;
    MagFilter = POINT;

	AddressU = Wrap;
};

sampler Ocean2BumpSampler =
sampler_state
{
    Texture = <Ocean2BumpTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Wrap;
	AddressV = Wrap;
	AddressW = Wrap;
};

sampler Ocean2ReflSampler =
sampler_state
{
    Texture = <Ocean2ReflTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};

sampler Ocean2RefrSampler =
sampler_state
{
    Texture = <Ocean2RefrTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};

texture Ocean2SphereMapTexture;

sampler Ocean2SphereMapSampler =
sampler_state
{
    Texture = <Ocean2SphereMapTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};

struct OceanVSIn
{
	float3 vPos : POSITION;
};

struct OceanVSOut
{
	float4 vPos : POSITION;
	float4 pos : TEXCOORD0;
};

struct OceanPSIn
{
	float4 pos : TEXCOORD0;
};

struct ZPIXEL
{
    float4 Color : COLOR0;
    float  Depth : DEPTH0;
};

float4		ocean2BumpPower;	// x - bump power, y - bump fade for water part transparency
float4		ocean2SkyColor;
float4		ocean2WaterColor;
float4		ocean2TranslucenceColor;
float4		ocean2TranslucenceAngle;
float4		ocean2TranslucenceParams;		// 

float3 fast_reflect(in float3 V, in float3 N)
{
	return 2.0f * N * dot(N, V) - V;
}

#endif