#ifndef STDVARS
#define STDVARS

#include "technique.h"

float fOverDrawScale = 0.03125f;

float4x4 mWorldViewProj;
float4x4 mWorld;
float4x4 mView;
float4x4 mProjection;
float4x4 mInverseView;


//Время в приложении (в 10 раз медленнее), используется для слайдинга и т.д.
float time;

//pos составляющая матрицы World
float3 vWorldPos;

//Направление камеры в мировом пространстве 
float3 vCamDirRelativeWorld;


//Позиция камеры в мировом пространстве 
float3 vCamPosRelativeWorld;

//Позиция камеры в объектном пространстве 
float3 vCamPos;


//Текстура заменяющая pow для степени 32
texture texSpecular;
sampler tSpecular =
sampler_state
{
        Texture = <texSpecular>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Clamp;
        AddressV = Clamp;
};



//Текстура из которой получаеться коэфицент сдвига для parralax maping
texture texParralaxK;
sampler tParralaxK =
sampler_state
{
        Texture = <texParralaxK>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Clamp;
        AddressV = Clamp;
};



//Нормализующий кубмап
texture texNormalsCube;
sampler tNormalsCubemap =
sampler_state
{
        Texture = <texNormalsCube>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;
};



float fFogHeightDensity;
float fFogHeightMax;
float fFogHeightMin;

float fFogHeightDistanceInv;

float fFogDensity;
float fFogDistanceMin;
float fFogDistanceMax;

float fFogDistanceMinSquared;
float fFogDistanceLen;

float4 cFogColor;


float CalculateDistanceFogK (float3 vWorldPos)
{
	float3 dist = vWorldPos - vCamPosRelativeWorld;
	float fDistToPointSquared = dot (dist, dist);

	float fMinDistSquared = fFogDistanceMin * fFogDistanceMin;
	float fMaxDistSquared = fFogDistanceMax * fFogDistanceMax;
	float fFogLen = 1.0f / (fFogDistanceMax - fMinDistSquared);

	float distFogK = ((fDistToPointSquared - fFogDistanceMinSquared) * fFogDistanceLen);

	return saturate(distFogK) * fFogDensity;
}


float CalculateHeightFogK (float3 vWorldPos)
{
	float heightFogK = 1.0 - ((vWorldPos.y - fFogHeightMin) * fFogHeightDistanceInv);

	return saturate(heightFogK) * fFogHeightDensity;
}


float CalculateFogFactor (float3 vWorldPos)
{
	float fDistK = CalculateDistanceFogK(vWorldPos);
	float fHeightK = CalculateHeightFogK(vWorldPos);

	float fFogK = saturate(fDistK + fHeightK);

	return fFogK;
}

float4 ApplyFogColor(float4 texelColor, float fFogK)
{
	return lerp (texelColor, cFogColor, float4(fFogK, fFogK, fFogK, 0.0));
}

float4 CalculateFog (float3 vWorldPos, float4 texelColor)
{
	float fFogK = CalculateFogFactor(vWorldPos);

	return ApplyFogColor(texelColor, fFogK);
	//return lerp (texelColor, cFogColor, float4(fFogK, fFogK, fFogK, 0.0));
}

float GammaValue;

float4 GammaCorrection(float4 src)
{
	return float4(pow(src.rgb, GammaValue), src.a);
}

#endif