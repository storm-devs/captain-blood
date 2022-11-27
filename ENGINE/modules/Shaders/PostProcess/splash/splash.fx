#include "technique.h"
#include "gmxservice\gmx_Variables.h"
#include "stdVariables.h"
#include "particles\ParticlesCommon.h"

texture Splash_NativeTexture;

sampler Splash_NativeSampler =
sampler_state
{
	Texture = <Splash_NativeTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

texture Splash_NormalTexture;

sampler Splash_NormalSampler =
sampler_state
{
	Texture = <Splash_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

texture Splash_DiffuseTexture;

sampler Splash_DiffuseSampler =
sampler_state
{
	Texture = <Splash_DiffuseTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct SplashFilter_VS_IN
{
	float3 pos : POSITION0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float  alp : TEXCOORD3;
	float  ble : TEXCOORD4;
	float4 col : TEXCOORD5;
};

struct SplashFilter_VS_OUT
{
	float4 pos : POSITION0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float4 ppp : TEXCOORD3;
	float4 dir : TEXCOORD4;
	float  alp : TEXCOORD5;
	float  ble : TEXCOORD6;
	float4 col : TEXCOORD7;
};

struct SplashFilter_PS_IN
{
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float4 ppp : TEXCOORD3;
	float4 dir : TEXCOORD4;
	float  alp : TEXCOORD5;
	float  ble : TEXCOORD6;
	float4 col : TEXCOORD7;
};

SplashFilter_VS_OUT vs_SplashFilter(SplashFilter_VS_IN v)
{
	SplashFilter_VS_OUT o;

	o.pos = float4(v.pos,1.0);

	o.uv1 = v.uv1;
	o.uv2 = v.uv2;

	o.ppp = o.pos;
	o.dir = mul(-vGlobalLightDirection.xyz,mView);

	o.alp = v.alp;
	o.ble = v.ble;

	o.col = v.col;

	return o;
}

float4 ps_SplashFilter(SplashFilter_PS_IN v) : COLOR0
{
	float2 n = 2*(lerp(
		tex2D(Splash_NormalSampler,v.uv1),
		tex2D(Splash_NormalSampler,v.uv2),v.ble) - 0.5)*0.02;

//	n *= v.alp;

	float a = 1.0 - v.alp;

	n *= 1.0 - a*a;

	float2 q = v.ppp*0.5 + 0.5;

	q.y = 1.0 - q.y;

	q.x += -n.x;
	q.y -= -n.y;

	float4 p = tex2D(Splash_NativeSampler,q);

	float4 z = lerp(
		tex2D(Splash_DiffuseSampler,v.uv1),
		tex2D(Splash_DiffuseSampler,v.uv2),v.ble);

//	float4 u = v.col;
	float4 u = v.col*z;

//	p = p*(1.0 - z.w*u.w) + u*z.w*u.w;
//	p = p*(1.0 -	 u.w) + u*	  u.w;
//	p = p*u;
	p = p*lerp(1.0,u,u.w);

//	p += u*0.5f;
//	p += u*0.4f;
	p += u*0.4f*u.w;

	float3 N = normalize(float3(n*20,1.0));

//	float3 L = normalize(v.dir);
//	float3 L = float3(0.0,0.0,-1.0);

//	float3 L = normalize(float3(1.0,-1.0,-5.0));
	float3 L = normalize(float3(1.0, 1.0,-2.0));

//	float3 R = reflect(L,N);

//	float3 R = refract(L,N,1.40);
//	float3 R = refract(L,N,1.45);
	float3 R = refract(L,N,1.47);

//	float3 H = float3(0.0,0.0,-1.0);
	float3 H = L;

//	float diff = abs(dot(N,-L));
	float diff = dot(N,-L);

//	float spec = pow(max(0.0f,dot(R,-H)),64)*0.5;

//	float spec = pow(max(0.0f,dot(R, H)),64)*0.5;
//	float spec = pow(max(0.0f,dot(R, H)),64)*0.7;
	float spec = pow(max(0.0f,dot(R, H)),64)*0.8;

	//// add spec from glob light ////

	L = normalize(v.dir);

	R = reflect(L,N);

//	H = L;

	H = float3(0.0,0.0,-1.0);

	spec += pow(max(0.0f,dot(R,-H)),64)*0.15;

	//////////////////////////////////

//	if( abs(n.x) < 0.01 &&
//		abs(n.y) < 0.01 ) spec = 0.0;

	p = p*diff + spec;

	p.w = z.w*v.alp*(1.0 + spec*5);
//	p.w = z.w*v.alp*(1.0 + spec*5)*v.col.w;

	return p;
}

Technique(SplashFilter,SplashFilter_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;
	//	ZEnable = true;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		AlphaBlendEnable = true;

	//	AlphaTestEnable = false;
		AlphaTestEnable = true;
		AlphaRef = 4;

		VertexShader = compile vs_2_0 vs_SplashFilter();
		PixelShader = compile ps_2_0 ps_SplashFilter();
	}
}
