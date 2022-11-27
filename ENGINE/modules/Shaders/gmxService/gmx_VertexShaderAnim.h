#ifndef GMX_VS_ANIM_IMPL
#define GMX_VS_ANIM_IMPL


#include "stdVariables.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"



/*
#ifdef _XBOX
sampler GMX_skinBonesStream : register(vf2);
#endif
*/

/*
#ifdef _XBOX
[maxtempreg(20)]
[reduceTempRegUsage(14)]
#endif
*/
GMX_VS_OUT GMX_VertexShaderAnim(GMX_VS_IN_ANIM v, uniform int nNumBones)
{
 GMX_VS_OUT o;

 int BoneIndexes[4] = (int[4])D3DCOLORtoUBYTE4( v.s1.vBonesID );
 float Weights[3] = (float[3])UnpackWeights(v.s0.vPackedWeight12);


 float3 vNormal = UnpackVec(v.s0.vPackedNormal);
 float3 vTangent = UnpackVec(v.s0.vPackedTangent);
 float3 vBinormal = UnpackVec(v.s0.vPackedBinormal);


 float3 vPos = 0.0;
 float3 Normal = 0.0;
 float3 Tangent = 0.0;
 float3 Binormal = 0.0;


 float LastWeight = 0.0f;


#ifdef _XBOX

 float4 bone0_const0;
 float4 bone0_const1;
 float4 bone0_const2;

 float4 bone1_const0;
 float4 bone1_const1;
 float4 bone1_const2;

 float4 bone2_const0;
 float4 bone2_const1;
 float4 bone2_const2;

 int bone0_idx = BoneIndexes[0];
 int bone1_idx = BoneIndexes[1];
 int bone2_idx = BoneIndexes[2];


 if (nNumBones >= 1)
 {
	 asm
	 {
/*
		vfetch_full bone0_const0, bone0_idx, GMX_skinBonesStream, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=0, Signed=true, NumFormat=integer, Stride=12, PrefetchCount=8
		vfetch_mini bone0_const1, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=4,  Signed=true, NumFormat=integer
		vfetch_full bone0_const2, bone0_idx, GMX_skinBonesStream, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=8, Signed=true, NumFormat=integer, Stride=12
*/
 
		 vfetch bone0_const0, bone0_idx, texcoord0, UseTextureCache=true;
		 vfetch bone0_const1, bone0_idx, texcoord1, UseTextureCache=true;
		 vfetch bone0_const2, bone0_idx, texcoord2, UseTextureCache=true;
	 };
 }

 if (nNumBones >= 2)
 {
	 asm
	 {
/*
		 vfetch_full bone1_const0, bone1_idx, GMX_skinBonesStream, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=0, Signed=true, NumFormat=integer, Stride=12, PrefetchCount=8
		 vfetch_mini bone1_const1, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=4,  Signed=true, NumFormat=integer
		 vfetch_full bone1_const2, bone1_idx, GMX_skinBonesStream, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=8, Signed=true, NumFormat=integer, Stride=12
*/


		 vfetch bone1_const0, bone1_idx, texcoord0, UseTextureCache=true;
		 vfetch bone1_const1, bone1_idx, texcoord1, UseTextureCache=true;
		 vfetch bone1_const2, bone1_idx, texcoord2, UseTextureCache=true;

	 };
 }

 if (nNumBones >= 3)
 {
	 asm
	 {
/*
		 vfetch_full bone2_const0, bone2_idx, GMX_skinBonesStream, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=0, Signed=true, NumFormat=integer, Stride=12, PrefetchCount=8
		 vfetch_mini bone2_const1, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=4,  Signed=true, NumFormat=integer
		 vfetch_full bone2_const2, bone2_idx, GMX_skinBonesStream, DataFormat=FMT_32_32_32_32_FLOAT, UseTextureCache=true, Offset=8, Signed=true, NumFormat=integer, Stride=12
*/

		 vfetch bone2_const0, bone2_idx, texcoord0, UseTextureCache=true;
		 vfetch bone2_const1, bone2_idx, texcoord1, UseTextureCache=true;
		 vfetch bone2_const2, bone2_idx, texcoord2, UseTextureCache=true;

	 };
 }


#else

 float4 bone0_const0 = GMX_skinBones[(BoneIndexes[0]*3) + 0];
 float4 bone0_const1 = GMX_skinBones[(BoneIndexes[0]*3) + 1];
 float4 bone0_const2 = GMX_skinBones[(BoneIndexes[0]*3) + 2];


 float4 bone1_const0 = GMX_skinBones[(BoneIndexes[1]*3) + 0];
 float4 bone1_const1 = GMX_skinBones[(BoneIndexes[1]*3) + 1];
 float4 bone1_const2 = GMX_skinBones[(BoneIndexes[1]*3) + 2];

 float4 bone2_const0 = GMX_skinBones[(BoneIndexes[2]*3) + 0];
 float4 bone2_const1 = GMX_skinBones[(BoneIndexes[2]*3) + 1];
 float4 bone2_const2 = GMX_skinBones[(BoneIndexes[2]*3) + 2];

#endif


 if (nNumBones == 1)
 {
	 //Одна кость
	 vPos = mulByMatrix4x3(v.s0.vPos, bone0_const0, bone0_const1, bone0_const2);
	 Normal = mulByMatrix3x3(vNormal, bone0_const0, bone0_const1, bone0_const2);
	 Tangent = mulByMatrix3x3(vTangent, bone0_const0, bone0_const1, bone0_const2);
	 Binormal = mulByMatrix3x3(vBinormal, bone0_const0, bone0_const1, bone0_const2);

	 o.ScreenPosition_AndZIsBoneAlpha.z = bone0_const2.x;
 } else
 {
	 if (nNumBones == 2)
	 {
		 //Две кости
		 LastWeight = LastWeight + Weights[0];

		 vPos += mulByMatrix4x3(v.s0.vPos, bone0_const0, bone0_const1, bone0_const2) * Weights[0];
		 Normal += mulByMatrix3x3(vNormal, bone0_const0, bone0_const1, bone0_const2) * Weights[0];
		 Tangent += mulByMatrix3x3(vTangent, bone0_const0, bone0_const1, bone0_const2) * Weights[0];
		 Binormal += mulByMatrix3x3(vBinormal, bone0_const0, bone0_const1, bone0_const2) * Weights[0];

		 LastWeight = 1.0f - LastWeight; 

		 vPos += mulByMatrix4x3(v.s0.vPos, bone1_const0, bone1_const1, bone1_const2) * LastWeight;
		 Normal += mulByMatrix3x3(vNormal, bone1_const0, bone1_const1, bone1_const2) * LastWeight;
		 Tangent += mulByMatrix3x3(vTangent, bone1_const0, bone1_const1, bone1_const2) * LastWeight;
		 Binormal += mulByMatrix3x3(vBinormal, bone1_const0, bone1_const1, bone1_const2) * LastWeight;

		 o.ScreenPosition_AndZIsBoneAlpha.z = bone0_const2.x;
	 } else
	 {
		 if (nNumBones == 3)
		 {
			 //Три кости
			 LastWeight = LastWeight + Weights[0];

			 vPos += mulByMatrix4x3(v.s0.vPos, bone0_const0, bone0_const1, bone0_const2) * Weights[0];
			 Normal += mulByMatrix3x3(vNormal, bone0_const0, bone0_const1, bone0_const2) * Weights[0];
			 Tangent += mulByMatrix3x3(vTangent, bone0_const0, bone0_const1, bone0_const2) * Weights[0];
			 Binormal += mulByMatrix3x3(vBinormal, bone0_const0, bone0_const1, bone0_const2) * Weights[0];

			 LastWeight = LastWeight + Weights[1];
			 vPos += mulByMatrix4x3(v.s0.vPos, bone1_const0, bone1_const1, bone1_const2) * Weights[1];
			 Normal += mulByMatrix3x3(vNormal, bone1_const0, bone1_const1, bone1_const2) * Weights[1];
			 Tangent += mulByMatrix3x3(vTangent, bone1_const0, bone1_const1, bone1_const2) * Weights[1];
			 Binormal += mulByMatrix3x3(vBinormal, bone1_const0, bone1_const1, bone1_const2) * Weights[1];

			 LastWeight = 1.0f - LastWeight; 

			 vPos += mulByMatrix4x3(v.s0.vPos, bone2_const0, bone2_const1, bone2_const2) * LastWeight;
			 Normal += mulByMatrix3x3(vNormal, bone2_const0, bone2_const1, bone2_const2) * LastWeight;
			 Tangent += mulByMatrix3x3(vTangent, bone2_const0, bone2_const1, bone2_const2) * LastWeight;
			 Binormal += mulByMatrix3x3(vBinormal, bone2_const0, bone2_const1, bone2_const2) * LastWeight;

			 o.ScreenPosition_AndZIsBoneAlpha.z = bone0_const2.x;
		 }
	 }
 }


 o.Position = mul( float4(vPos, 1.0), mWorldViewProj);
 o.TexCoord12.xy = UnpackUV(v.s0.vPackedUV1);
 o.TexCoord12.zw = o.TexCoord12.xy;

 float3 worldPos = mul( float4(vPos, 1.0), mWorld);

 float3x3 tangentToWorld;
 tangentToWorld[0] = mul (Normal, mWorld); 
 tangentToWorld[1] = mul (Tangent, mWorld); 
 tangentToWorld[2] = mul (Binormal, mWorld); 

 o.vNormal_And_WorldPosX = float4 (tangentToWorld[0], worldPos.x);
 o.vTangent_And_WorldPosY = float4 (tangentToWorld[1], worldPos.y);
 o.vBinormal_And_WorldPosZ = float4 (tangentToWorld[2], worldPos.z);

 o.ScreenPosition_AndZIsBoneAlpha.xyw = mul( float4(vPos, 1.0), GMX_ShadowMatrix).xyw;
 

 return o;

}



#endif