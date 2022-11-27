#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"


float4 gmx_LightMap_ps(GMX_PS_IN pnt, uniform bool bDontLight, uniform int userLightMode) : COLOR
{
		float4 Texel = tex2D(SpecularMap, pnt.TexCoord12.xy);
		float4 TexelLightMap = tex2D(DiffuseMap, pnt.TexCoord12.zw);

		float4 result = float4(float3(Texel.rgb * TexelLightMap.rgb), Texel.a);

		result.rgb = saturate(result.rgb + GMX_ObjectsUserColor.rgb);
		result.a = saturate(result.a * GMX_ObjectsUserColor.a);

		return result;
}


STATIC_UBERSHADER(gmx_LightMap, gmx_LightMap_ps,USERLIGHTMODE_NONE)




