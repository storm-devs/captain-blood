#include "technique.h"
#include "PostEffects\common.h"

#include "stdVariables.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_UberGenerator.h"

struct PEGO_VS_OUTPUT
{
    float4 pos : POSITION;
    float4 TexCoord0 : TEXCOORD0;
	float4 scr_uv : TEXCOORD1;
};

struct PEGO_PS_INPUT
{
    float4 TexCoord0 : TEXCOORD0;
	float4 scr_uv : TEXCOORD1;
};

PEGO_VS_OUTPUT vs_PEGO(GMX_VS_IN_STATIC In)
{
	PEGO_VS_OUTPUT res;

	float4 vPos = mul( float4(In.vPos, 1.0f), mWorldViewProj);
	//vPos.z *= 0.999999f;
	res.pos = vPos;
	res.TexCoord0.xy = UnpackUV(In.vPackedUV1);
	res.TexCoord0.zw = vPos.zw;
	res.scr_uv = vPos;
	
	return res;
}

PEGO_VS_OUTPUT vs_PEGO_Anim(GMX_VS_IN_ANIM In, uniform int nNumBones)
{
	PEGO_VS_OUTPUT res;
	GMX_VS_OUT gmx_out = GMX_VertexShaderAnim(In, nNumBones);

	float4 pos = gmx_out.Position;
	//pos.z *= 0.999999f;
	res.pos = pos;
	res.TexCoord0.xy = gmx_out.TexCoord12.xy;
	res.TexCoord0.zw = res.pos.zw;
	res.scr_uv = pos;//gmx_out.Position;

	return res;
}

float4 ps_PEGO(PEGO_PS_INPUT pnt) : COLOR
{
	float4 DifTex = tex2D(DiffuseMap, pnt.TexCoord0.xy);
	clip(DifTex.a - gmxAlphaRef);

#ifdef _XBOX
	// на X-BOX'e используем отсечение по Z прямо в шейдере, чтобы не восстанавливать Z-buffer
	/*float3 ruv;
	ruv.xyz = pnt.scr_uv.xyz / pnt.scr_uv.w * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f);
	float4 tex = tex2D(peDepthSampler, ruv.xy);
	clip(tex.r - ruv.z - 0.0000001f);*/
#endif
	
	return float4(0.0f, 0.0f, 0.0f, peGlowObjParams.x * 0.25f);
}

Technique(PEGO, GMX_VS_IN_STATIC)
{
	pass P0
	{
		AlphaTestEnable = false;
		AlphaBlendEnable = false;
		ZWriteEnable = false;
		ColorWriteEnable = alpha;
#ifdef _XBOX
		//ZEnable = false;
#endif

		VertexShader = compile vs_2_0 vs_PEGO();
		PixelShader = compile ps_2_0 ps_PEGO();
	}
}

Technique(PEGO_anim_1, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaTestEnable = false;
		AlphaBlendEnable = false;
		ZWriteEnable = false;
		ColorWriteEnable = alpha;
#ifdef _XBOX
		//ZEnable = false;
#endif

		VertexShader = compile vs_2_0 vs_PEGO_Anim(1);
		PixelShader = compile ps_2_0 ps_PEGO();
	}
}

Technique(PEGO_anim_2, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaTestEnable = false;
		AlphaBlendEnable = false;
		ZWriteEnable = false;
		ColorWriteEnable = alpha;
#ifdef _XBOX
		//ZEnable = false;
#endif

		VertexShader = compile vs_2_0 vs_PEGO_Anim(2);
		PixelShader = compile ps_2_0 ps_PEGO();
	}
}

Technique(PEGO_anim_3, GMX_VS_IN_ANIM)
{
	pass P0
	{
		AlphaTestEnable = false;
		AlphaBlendEnable = false;
		ZWriteEnable = false;
		ColorWriteEnable = alpha;
#ifdef _XBOX
		//ZEnable = false;
#endif

		VertexShader = compile vs_2_0 vs_PEGO_Anim(3);
		PixelShader = compile ps_2_0 ps_PEGO();
	}
}
