#include "technique.h"
#include "stdVariables.h"
//#include "gmxService/gmx_UberShader.h"
//#include "arcadesea/Instancing_UberGenerator.h"

texture		ballShadowTex;
sampler		shadowSampler = sampler_state
{
	Texture = <ballShadowTex>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

	AddressU = Clamp;
	AddressV = Clamp;
};


struct BallShadowVS_IN
{
	float3	vPos : POSITION;
	float3	vTexCoordAlpha : TEXCOORD0;	
};
	
struct BallShadowVS_OUT
{
	float4 Pos: POSITION0;
	float3 TexAlpha : TEXCOORD0;
};


BallShadowVS_OUT BallShadowVS(BallShadowVS_IN IN) 
{
	BallShadowVS_OUT OUT;
	OUT.Pos = mul(float4(IN.vPos, 1.0f), mWorldViewProj);
	OUT.TexAlpha = IN.vTexCoordAlpha;
	
	return OUT;
} 

float4 BallShadowPS(BallShadowVS_OUT IN) : COLOR
{
	float4 OUT = tex2D(shadowSampler, IN.TexAlpha.xy);
	OUT.a *= IN.TexAlpha.z;
	return OUT;
}


Technique(BallShadow, BallShadowVS_IN)
{
	pass P0
	{
		AlphaTestEnable = true;
		AlphaRef = 0x00000002;
		
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		
		CullMode = none;
		ZEnable = true;
		ZWriteEnable = false;

		VertexShader = compile vs_2_0 BallShadowVS();
		PixelShader = compile ps_2_0 BallShadowPS();
	}
}



//STATIC_INSTANCING_UBERSHADER(gmx_InstancedStaticGeom,InstancedPixelShader,USERLIGHTMODE_ADD)