#include "technique.h"
#include "stdVariables.h"
#include "gmxservice\gmx_Variables.h"
#include "shadows\shd_Variables.h"

texture BloodStainDiff;

sampler BloodStainDiffSampler =
sampler_state
{
	Texture = <BloodStainDiff>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    
	AddressU = Clamp;
	AddressV = Clamp;
};

texture BloodStainNorm;

sampler BloodStainNormSampler =
sampler_state
{
	Texture = <BloodStainNorm>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    
	AddressU = Clamp;
	AddressV = Clamp;
};

texture BloodStainShad;

sampler BloodStainShadSampler =
sampler_state
{
	Texture = <BloodStainShad>;
	MinFilter = Linear;
	MagFilter = Linear;

	AddressU = Clamp;
	AddressV = Clamp;
};

float  BloodStainTime;	// mission global time
float3 BloodStainK;		// float3(1.0f/atlas_column_count,1.0f/atlas_row_count,0)

struct BloodStain_VS_IN
{
	float3 pos : POSITION;
	float4 col : COLOR0;
	float2 tuv : TEXCOORD0;
	float3 tgx : TEXCOORD1;
	float3 tgy : TEXCOORD2;
	float3 tgz : TEXCOORD3;
	float4 t   : TEXCOORD4;
	float3 off : TEXCOORD5;
	float2 spe : TEXCOORD6;
};

struct BloodStain_VS_OUT
{
	float4 pos : POSITION;
	float4 col : TEXCOORD0;
	float2 tuv : TEXCOORD1;
	float3 dir : TEXCOORD2;
	float3 cam : TEXCOORD3;
	float4 scr : TEXCOORD4;
	float4 off : TEXCOORD5;
	float3 ppp : TEXCOORD6;
};

struct BloodStain_PS_IN
{
	float4 col : TEXCOORD0;
	float2 tuv : TEXCOORD1;
	float3 dir : TEXCOORD2;
	float3 cam : TEXCOORD3;
	float4 scr : TEXCOORD4;
	float4 off : TEXCOORD5;
	float3 ppp : TEXCOORD6;
};

BloodStain_VS_OUT vs_BloodStain(BloodStain_VS_IN v)
{
	BloodStain_VS_OUT o;

	o.pos = mul(float4(v.pos,1.0),mWorldViewProj);
	o.scr = mul(float4(v.pos,1.0),GMX_ShadowMatrix);

	o.tuv = v.tuv;

	float4 c = v.col;

//	v.t.x - born global time
//	v.t.y - fade in time
//	v.t.z - fade out time
//	v.t.w - live total time

	float del = v.off.x; // born delay

	float t = BloodStainTime - v.t.x;

	t = max(0.0,t - del);

	c.w *= min(1.0,        (t)/v.t.y);
	c.w *= min(1.0,(v.t.w - t)/v.t.z);

	o.col = c;

	float3x3 m;

	m[0] = v.tgx;
	m[1] = v.tgy;
	m[2] = v.tgz;

	o.dir = mul(-vGlobalLightDirection.xyz,m);
	o.cam = mul( vCamPos - v.pos,m);

	o.off.xy = v.off.yz;
	o.off.zw = v.spe;

	o.ppp = v.pos;

	return o;
}

float4 ps_BloodStain(BloodStain_PS_IN v) : COLOR
{
	float2 tuv = clamp(v.tuv,0.0,BloodStainK.xy) + BloodStainK.xy*v.off.xy;

	float4 text = tex2D(BloodStainDiffSampler,tuv);

	clip(text.w - 0.1);

//	float2 n = 2*(tex2D(BloodStainNormSampler,tuv).xy - 0.5)*v.col.w;
	float2 n = 2*(tex2D(BloodStainNormSampler,tuv).xy - 0.5);

	n.x *= -1.0;
	n.y *= -1.0;

//	float3 N = normalize(float3(n*2.0,1.0));
	float3 N = normalize(float3(n*1.8,1.0));

//	float3 L = normalize(v.dir);
	float3 L = normalize(v.dir + float3(0.0,0.0,1.0));

//	float diff = max(0.0,dot(N,L))*0.5;
	float diff = max(0.0,dot(N,L));

//	L = normalize(float3(v.tuv.xy,0.0) - float3(0.5,0.5,-1.0));
//	L = normalize(float3(v.tuv.xy,0.0) - float3(0.5,0.5,-0.5));

//	L = normalize(v.dir + float3(0.0,0.0,3.0));
//	L = normalize(float3(0.0,0.0,0.5));

	L = normalize(float3(tuv,0.0) - float3(BloodStainK.xy*(v.off.xy + 0.5),-0.5));

	float3 R = reflect(L,N);

	float3 H = normalize(v.cam);
/*
//	float spec = pow(max(0.0f,dot(R,H)),64)*0.5;
	float spec = pow(max(0.0f,dot(R,H)),32)*0.5;*/

//	v.off.z - spec power
//	v.off.w - spec multi

	float spec = pow(max(0.0f,dot(R,H)),clamp(v.off.z,1.0,32.0))*0.5;

	spec *= v.off.w;

	float4 tShadowColor = tex2Dproj(BloodStainShadSampler,v.scr);

	float Bright = dot(tShadowColor.rgb,1.0)/3.0;
	float ShadowBright = (1.0 - Bright);

	spec *= Bright*Bright*Bright;

//	if( Bright < 0.99 )
//		spec = 0.0;

//	text = text*diff*v.col*0.5*(1.0 + float4(vAmbientLight,1.0)) + spec*(1.0f - ShadowBright);
//	text = text*v.col*0.5*(float4(vAmbientLight.xyz,1.0) + diff) + spec;//*(1.0f - ShadowBright);
	text.rgb = text.rgb*v.col.rgb*0.5*(vAmbientLight.xyz + diff) + spec;

//	text *= tex2Dproj(ShadowMap,v.scr);

	////

//	float ShadowBright = (1.0 - tex2Dproj(ShadowMap,v.scr).a);

//	ShadowBright *= 0.45;
//	ShadowBright = clamp(ShadowBright,0.0,0.45);

//	float3 ShadowColor = lerp(float3(1.0,1.0,1.0),/*v*/tShadowColor.rgb,ShadowBright);
//	text.rgb *=  ShadowColor;
	text.rgb *= tShadowColor.rgb;

//	text.rgb *= 1.0 - ShadowBright;

	////

//	text.w = a*v.col.w*(1.0 + spec*5.0);
	text.w *= v.col.w*(1.0 + spec*5.0);

	text = CalculateFog(v.ppp,text);

//	text.b += 1.0 - text.w;
//	text.w += (1.0 - text.w)*0.5*v.col.w;

	return text;
}

Technique(BloodStain, BloodStain_VS_IN)
{
	pass P0
	{
		ZWriteEnable = false;

		CullMode = CCW;

		AlphaBlendEnable = true;

	//	ZEnable = true;
	//	ZFunc = Equal;

		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_BloodStain();
		PixelShader = compile ps_2_0 ps_BloodStain();
	}
}
