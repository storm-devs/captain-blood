#include "technique.h"
#include "stdVariables.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_UberGenerator.h"

float3 vSkyFogColor;
float3 fSkyFogMultiply;		// x - SkyFogMultiply, y - 1.0f / SkyFogHeight
float4 cSkyColor;

struct SkyFogVSOut
{
	float4 vPos : POSITION;
    float2 uv : TEXCOORD0;
    float posy : TEXCOORD1;
};

struct SkyFogPSIn
{
	float2 uv : TEXCOORD0;
    float posy : TEXCOORD1;
};

SkyFogVSOut vs_SkyFog(GMX_VS_IN_STATIC In)
{
	SkyFogVSOut o;

	float3 vPos = In.vPos.xyz;
	
	o.vPos = mul(float4(vPos, 1.0), mWorldViewProj);
	o.uv = UnpackUV(In.vPackedUV1);
	o.posy = vPos.y;

	return o;
}

float4 ps_SkyFog(SkyFogPSIn p) : COLOR
{
	float4 tex = tex2D(DiffuseMap, p.uv);
	float k = saturate((1.0 - p.posy * fSkyFogMultiply.y) * fSkyFogMultiply.x);
	return float4(lerp(tex.rgb * cSkyColor, vSkyFogColor, k), cSkyColor.a);
}

Technique(SkyFog, GMX_VS_IN_STATIC)
{
	pass P0
	{
		ZWriteEnable = false;
	
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
	
		VertexShader = compile vs_2_0 vs_SkyFog();
		PixelShader = compile ps_2_0 ps_SkyFog();
	}
}

Technique(SkyFogSecond, GMX_VS_IN_STATIC)
{
	pass P0
	{
		ZWriteEnable = false;
	
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		AlphaTestEnable = false;
	
		VertexShader = compile vs_2_0 vs_SkyFog();
		PixelShader = compile ps_2_0 ps_SkyFog();
	}
}
