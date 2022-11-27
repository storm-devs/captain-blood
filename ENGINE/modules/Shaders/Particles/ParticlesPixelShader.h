#ifndef PARTICLES_PIXEL_SHADER
#define PARTICLES_PIXEL_SHADER


#include "stdVariables.h"
#include "Particles\ParticlesCommon.h"
#include "Particles\ParticlesStruct.h"

#ifdef _XBOX360
 float fParticleSoftness = 1.0f;
 float4x4 mProjInverse;
#else
 float4 fitToViewport;
#endif




float4 ps_Particles(Particles_PS_IN v, uniform bool bOverDraw, uniform bool bSoftParticles) : COLOR0
{
	
	//Считаем цвет диффузный в точке
	half4 Frame1 = tex2D(ParticlesTexture, v.UV1); 
	half4 Frame2 = tex2D(ParticlesTexture, v.UV2); 
	half4 TexelColor = lerp(Frame2, Frame1, v.inParams.x);
	TexelColor = TexelColor * v.DiffuseColor;


#ifdef _XBOX360

	if (bSoftParticles == true)
	{
		float3 p = v.PositionShader.xyz / v.PositionShader.w;


		p.xy = 0.5f * p.xy + 0.5f;
		p.y = 1.0f - p.y;


		//Читаем из Depth значение
		//float pixelDepth = tex2D(DepthSampler, p.xy).r;
		float pixelDepth;
		asm {
			tfetch2D pixelDepth.x___, p.xy, DepthSampler, OffsetX = 0.5, OffsetY = 0.5
		};


		//Получаем линейное значение глубины в Depth
		float4 vDepth = mul(float4(p.xy, pixelDepth, 1.0), mProjInverse);
		float fLinearDepth = vDepth.z / vDepth.w;

		//Получаем линейное значение глубины у частицы
		float4 vParticleDepth = mul(float4(p.xy, p.z, 1.0), mProjInverse);
		float fLinearParticleDepth = vParticleDepth.z / vParticleDepth.w;

		//Разницу в глубине считаем
		FLOAT fZDifference = (fLinearDepth - fLinearParticleDepth) * fParticleSoftness;

		TexelColor.a *= clamp( fZDifference, 0.0f, 1.0f );
	}

#endif


	



	//Считаем нормаль в точке  
	half4 Bump1 = tex2D(ParticlesNormalMap, v.UV1); 
	half3 TexelNormal = 2.0 * Bump1.xyz - 1.0;


	//Считаем освещение в точке  
	half Light = clamp(dot(TexelNormal.xyz, v.LightDir.xyz), 0.1, 1.0);
	half3 LightColor = half3 (Light, Light, Light) * LightDirProjectionColor.rgb;

	//Считаем финальный цвет пиксела...  

	half3 AmbientInfluence = lerp (LightAmbientParticles, half3(1.0, 1.0, 1.0), v.inParams.w);

	LightColor = (LightColor * TexelColor.rgb) * AmbientInfluence;
	half3 FinalColor = (TexelColor.rgb * AmbientInfluence) + (LightColor * TexelColor.a);
	TexelColor.rgb = lerp (FinalColor, (TexelColor.rgb*AmbientInfluence), v.inParams.z);

	TexelColor.rgb = TexelColor.rgb * TexelColor.aaa;
	TexelColor.a = (TexelColor.a * v.inParams.y); 


	if (bOverDraw == true)
	{
		TexelColor = float4(0.0f, 1.0f, 0.0f, fOverDrawScale);
	}

	return TexelColor;
}






float4 ps_ParticlesDistorsion(Particles_PS_IN v, uniform bool bOverDraw) : COLOR0
{
	float3 p = v.PositionShader.xyz / v.PositionShader.w;

	p.xy = (0.5f * p.xy + 0.5f);
	p.y = 1.0 - p.y;

#ifndef _XBOX
	p.xy = (p.xy * fitToViewport.zw) + fitToViewport.xy;
#endif

	half4 Bump1 = tex2D(DistortMap, v.UV1);
	half Mask = tex2D(DistortMap, v.UV2).a;

	Bump1.xy = (Mask * ((Bump1.xy - 0.5) * 2.0)) * v.inParams.y;
	Bump1.zw = 0.0;

	p.xy += (Bump1.xy * float2(0.02, 0.02));


	float4 ScreenTex = tex2D(ScreenSurf, p.xy);

	if (bOverDraw == true)
	{
		ScreenTex = float4(0.0f, 1.0f, 0.0f, fOverDrawScale);
	}

	return ScreenTex;
}




#endif