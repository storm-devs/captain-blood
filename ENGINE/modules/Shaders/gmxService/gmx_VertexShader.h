#ifndef GMX_VS_IMPL
#define GMX_VS_IMPL

#include "stdVariables.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"





GMX_VS_OUT GMX_VertexShader(GMX_VS_IN_STATIC v)
{
	GMX_VS_OUT o;

	float3 Normal = UnpackVec(v.vPackedNormal);
	float3 Tangent = UnpackVec(v.vPackedTangent);
	float3 Binormal = UnpackVec(v.vPackedBinormal);

	o.Position = mul( float4(v.vPos, 1.0), mWorldViewProj);
	o.TexCoord12.xy = UnpackUV(v.vPackedUV1);
	o.TexCoord12.zw = UnpackUV(v.vPackedUV2);

	o.ScreenPosition_AndZIsBoneAlpha = mul( float4(v.vPos, 1.0), GMX_ShadowMatrix);;
	float3 worldPos = mul( float4(v.vPos, 1.0), mWorld);

	o.ScreenPosition_AndZIsBoneAlpha.z = 1.0f;

	float3x3 tangentToWorld;
	tangentToWorld[0] = mul (Normal, mWorld); 
	tangentToWorld[1] = mul (Tangent, mWorld); 
	tangentToWorld[2] = mul (Binormal, mWorld); 

	o.vNormal_And_WorldPosX = float4 (tangentToWorld[0], worldPos.x);
	o.vTangent_And_WorldPosY = float4 (tangentToWorld[1], worldPos.y);
	o.vBinormal_And_WorldPosZ = float4 (tangentToWorld[2], worldPos.z);

	return o;
}




#endif