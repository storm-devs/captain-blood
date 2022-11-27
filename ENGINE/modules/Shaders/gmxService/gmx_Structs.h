#ifndef GMXSTRUCTS
#define GMXSTRUCTS


struct GMX_VS_IN_STATIC
{
 float3 vPos : POSITION;
 float4 vPackedNormal : COLOR0;
 float4 vPackedTangent : COLOR1;
 float4 vPackedBinormal : COLOR2;
 float2 vPackedUV1 : BLENDWEIGHT0;
 float2 vPackedUV2 : BLENDWEIGHT2;
};


struct GMX_VS_IN_ANIM_STREAM0
{
 float3 vPos : POSITION;
 float4 vPackedNormal : COLOR0;
 float4 vPackedTangent : COLOR1;
 float4 vPackedBinormal : COLOR2;
 float2 vPackedUV1 : BLENDWEIGHT0;
 float2 vPackedWeight12 : BLENDWEIGHT1;
};

struct GMX_VS_IN_ANIM_STREAM1
{
 float4 vBonesID:COLOR3;
};



#ifdef _XBOX

struct GMX_VS_IN_ANIM_BONESSTREAM
{
	float4 bone_const0:TEXCOORD0;
	float4 bone_const1:TEXCOORD1;
	float4 bone_const2:TEXCOORD2;
};

#endif


struct GMX_VS_IN_ANIM
{
 GMX_VS_IN_ANIM_STREAM0 s0;
 GMX_VS_IN_ANIM_STREAM1 s1;

#ifdef _XBOX
 GMX_VS_IN_ANIM_BONESSTREAM s2;
#endif

};






struct GMX_VS_OUT
{
        //screen position
        float4 Position:POSITION;

        //two texture coordinates
        float4 TexCoord12:TEXCOORD0;

        //screen position x,y,w  (в z лежит цвет кости)
        float4 ScreenPosition_AndZIsBoneAlpha:TEXCOORD1; 

        //basis vectors
        float4 vNormal_And_WorldPosX:TEXCOORD2;
        float4 vTangent_And_WorldPosY:TEXCOORD3;
        float4 vBinormal_And_WorldPosZ:TEXCOORD4;
};


struct GMX_PS_IN
{
        //two texture coordinates
        float4 TexCoord12:TEXCOORD0;

        //screen position x,y,w  (в z лежит цвет кости)
        float4 ScreenPosition_AndZIsBoneAlpha:TEXCOORD1; 

        //basis vectors
        float4 vNormal_And_WorldPosX:TEXCOORD2;
        float4 vTangent_And_WorldPosY:TEXCOORD3;
        float4 vBinormal_And_WorldPosZ:TEXCOORD4;
};






#endif