#ifndef PARTICLES_COMMON
#define PARTICLES_COMMON

//Базовая текстура
texture tParticlesTexture;
sampler ParticlesTexture =
sampler_state
{
	Texture = <tParticlesTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};


//нормал мап, для партиклов
texture tParticlesNormalMap;
sampler ParticlesNormalMap =
sampler_state
{
	Texture = <tParticlesNormalMap>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};


//проекция направленного источника в плоскость партиклов
float3 LightDirProjection;

//Ambient освещение сцены
float3 LightAmbientParticles;

//цвет источника света
float4 LightDirProjectionColor;




#ifdef _XBOX360

//Карта глубины для soft particles
texture DepthMap;
sampler DepthSampler =
sampler_state
{
	Texture = <DepthMap>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;

	AddressU = Clamp;
	AddressV = Clamp;
};

#endif




//Карта искажений
texture tDistortMap;
sampler DistortMap =
sampler_state
{
	Texture = <tDistortMap>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};


//Текущая картинка
texture tScreenSurf;
sampler ScreenSurf =
sampler_state
{
	Texture = <tScreenSurf>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};


//Сила искажений
float3 DistorsionSize;


#endif