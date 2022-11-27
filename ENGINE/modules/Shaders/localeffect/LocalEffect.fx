#include "technique.h"
#include "gmxservice\gmx_Variables.h"
#include "stdVariables.h"
#include "particles\ParticlesCommon.h"

texture NativeTexture;

sampler NativeSampler =
sampler_state
{
	Texture = <NativeTexture>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler NativeSampler_ =
sampler_state
{
	Texture = <NativeTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

texture NormalTexture;

sampler NormalSampler =
sampler_state
{
	Texture = <NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

texture DiffuseTexture;

sampler DiffuseSampler =
sampler_state
{
	Texture = <DiffuseTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

struct LocalEffect_VS_IN
{
	float3 pos : POSITION0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float  alp : TEXCOORD3;
	float  ble : TEXCOORD4;
};

struct LocalEffect_VS_OUT
{
	float4 pos : POSITION0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float4 ppp : TEXCOORD3;
	float4 dir : TEXCOORD4;
	float  alp : TEXCOORD5;
	float  ble : TEXCOORD6;
};

struct LocalEffect_PS_IN
{
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float4 ppp : TEXCOORD3;
	float4 dir : TEXCOORD4;
	float  alp : TEXCOORD5;
	float  ble : TEXCOORD6;
};

LocalEffect_VS_OUT vs_LocalEffect(LocalEffect_VS_IN v)
{
	LocalEffect_VS_OUT o;

	o.pos = float4(v.pos,1.0);

	o.uv1 = v.uv1;
	o.uv2 = v.uv2;

	o.ppp = o.pos;
	o.dir = mul(-vGlobalLightDirection.xyz,mView);

	o.alp = v.alp;
	o.ble = v.ble;

	return o;
}

float4 ps_LocalEffect(LocalEffect_PS_IN v) : COLOR0
{
	float2 n = 2*(lerp(tex2D(NormalSampler,v.uv1), tex2D(NormalSampler,v.uv2), v.ble) - 0.5)*0.02;
	float a = 1.0 - v.alp;	
	n *= 1.0 - a*a;
	
	float2 q = v.ppp*0.5 + 0.5;
	q.y = 1.0 - q.y;
	q.x += -n.x;
	q.y -= -n.y;
	
	float4 p = tex2D(NativeSampler,q);
	
	float4 z = lerp(tex2D(DiffuseSampler,v.uv1), tex2D(DiffuseSampler,v.uv2), v.ble);

	float4 u = float4(1.0,1.0,1.0,0.44)*z;

	p = p*lerp(1.0,u,u.w);
	p += u*0.4f*u.w;

	float3 N = normalize(float3(n*20,1.0));
	float3 L = float3(0.408248, 0.408248,-0.8165);
	float3 R = refract(L,N,1.47);
	float3 H = L;

	float diff = dot(N,-L);
	float spec = pow(max(0.0f,dot(R, H)),64)*0.8;

	//// add spec from glob light ////

	L = normalize(v.dir);
	R = reflect(L,N);
	H = float3(0.0,0.0,-1.0);

	spec += pow(max(0.001f,dot(R,-H)),64)*0.15;	
	p = p*diff + spec;
	p.w = z.w*v.alp*(1.0 + spec*5);	
	return p;
}

Technique(LocalEffect,LocalEffect_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;		

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		AlphaBlendEnable = true;

		AlphaTestEnable = true;
		AlphaRef = 4;

		VertexShader = compile vs_2_0 vs_LocalEffect();
		PixelShader = compile ps_2_0 ps_LocalEffect();
	}
}

struct SimpleQuad_VS_IN
{
	float2 pos : POSITION0;
	float2 tuv : TEXCOORD1;
};

struct SimpleQuad_VS_OUT
{
	float4 pos : POSITION0;
	float2 tuv : TEXCOORD1;
};

struct SimpleQuad_PS_IN
{
	float2 tuv : TEXCOORD1;
};

SimpleQuad_VS_OUT vs_SimpleQuad(SimpleQuad_VS_IN v)
{
	SimpleQuad_VS_OUT o;

	o.pos = float4(v.pos,0.0,1.0);
	o.tuv = v.tuv;

	return o;
}

float4 ps_SimpleQuad(SimpleQuad_PS_IN v) : COLOR0
{
	return tex2D(NativeSampler,v.tuv);
}

Technique(SimpleQuad,SimpleQuad_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;
	//	ZEnable = true;
		ZEnable = false;

	//	SrcBlend = srcalpha;
	//	DestBlend = invsrcalpha;
		AlphaTestEnable = false;
	//	AlphaBlendEnable = true;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 vs_SimpleQuad();
		PixelShader = compile ps_2_0 ps_SimpleQuad();
	}
}

float4 ps_SimpleQuad_(SimpleQuad_PS_IN v) : COLOR0
{
	return tex2D(NativeSampler_,v.tuv);
}

Technique(SimpleQuad_,SimpleQuad_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;
	//	ZEnable = true;
		ZEnable = false;

	//	SrcBlend = srcalpha;
	//	DestBlend = invsrcalpha;
		AlphaTestEnable = false;
	//	AlphaBlendEnable = true;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 vs_SimpleQuad();
		PixelShader = compile ps_2_0 ps_SimpleQuad_();
	}
}

struct LocalLines_VS_IN
{
	float3 pos : POSITION0;
	float2 tuv : TEXCOORD1;
};

struct LocalLines_VS_OUT
{
	float4 pos : POSITION0;
	float2 tuv : TEXCOORD1;
	float  dst : TEXCOORD2;
};

struct LocalLines_PS_IN
{
	float2 tuv : TEXCOORD1;
	float  dst : TEXCOORD2;
};

LocalLines_VS_OUT vs_LocalLines(LocalLines_VS_IN v)
{
	LocalLines_VS_OUT o;

	o.pos = mul(float4(v.pos,1.0),mWorldViewProj);
	o.tuv = v.tuv;
	o.dst = length(v.pos - vCamPos);

	return o;
}

float4 ps_LocalLines(LocalLines_PS_IN v) : COLOR0
{
	float4 p = 1.0;

//	p.a = 1.0 - 2.0*abs(0.5 - v.tuv.x);
//	p.a = 1.0 - v.dst*0.4;
//	p.a = 0.8 - v.dst*0.3;

	p.a = 1.0;

	p.a *= 1.0 - v.tuv.y*0.7;
	p.a *= 1.0 - 2.0*abs(0.5 - v.tuv.x);
//	p.a *= 0.7 - v.dst*0.05;
	p.a *= 0.7 - v.dst*0.01;

	return p; 
}

Technique(LocalLines,LocalLines_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;
		ZEnable = true;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		VertexShader = compile vs_2_0 vs_LocalLines();
		PixelShader = compile ps_2_0 ps_LocalLines();
	}
}
