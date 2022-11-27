#ifndef GMXPACK
#define GMXPACK

//Распаковать вектор 4-х компонентный...
float3 UnpackVec (in float4 PackVec4)
{
 float3 UnVec3 = (PackVec4.xyz * 2.0) - 1.0;
 return UnVec3;
}

//Распаковать UV координаты
float2 UnpackUV (in float2 PackedTex)
{
 float2 TexCoord = (PackedTex * (12.0 / 32767.5));
 return TexCoord; 
}


//Распаковка 3-х весов
float3 UnpackWeights (in float2 packedWeight)
{
   float3 Weight;
   Weight.xy = (packedWeight * (0.5f / 32767.5f)) + 0.5f;
	 Weight.z = 1.0 - Weight.x - Weight.y;
   return Weight;
}

#endif