#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"


#ifdef _XBOX360
[maxtempreg(GMX_MAX_TEMP_REGS)]
[reduceTempRegUsage(GMX_RECOMMENDED_TEMP_REGS)]
#endif
float4 gmxShowTexture(GMX_PS_IN pnt, uniform bool bDontLight, uniform int userLightMode) : COLOR
{
        float2 uv1 = pnt.TexCoord12.xy;

        float4 Texel = tex2D(DiffuseMap, uv1);
        clip(Texel.a - gmxAlphaRef);
        
        //return Texel;
        
        float2 uv2 = pnt.TexCoord12.zw;
        float3 worldPos = float3 (pnt.vNormal_And_WorldPosX.w, pnt.vTangent_And_WorldPosY.w, pnt.vBinormal_And_WorldPosZ.w);
        

        if (bDontLight)
        {
                if (userLightMode == USERLIGHTMODE_ADD)
                {
                        Texel.rgb = saturate(Texel.rgb + GMX_ObjectsUserColor.rgb);
                        Texel.a = saturate(Texel.a * GMX_ObjectsUserColor.a);
                        
                        Texel.a = saturate(Texel.a * pnt.ScreenPosition_AndZIsBoneAlpha.z);
                } 

                if (userLightMode == USERLIGHTMODE_MULTIPLY)
                {
                        Texel = saturate(Texel * GMX_ObjectsUserColor);
                        
                        Texel.a = saturate(Texel.a * pnt.ScreenPosition_AndZIsBoneAlpha.z);
                }

                return CalculateFog(worldPos, Texel);
        }


        //В мировом пространстве нормаль...
        float3 Normal = pnt.vNormal_And_WorldPosX;

        float4 ShadowMapTex = tex2Dproj(ShadowMap, pnt.ScreenPosition_AndZIsBoneAlpha);

        if (userLightMode == USERLIGHTMODE_ADD)
        {
                Texel.rgb = saturate(Texel.rgb + GMX_ObjectsUserColor.rgb);
                Texel.a = saturate(Texel.a * GMX_ObjectsUserColor.a);
                Texel.a = saturate(Texel.a * pnt.ScreenPosition_AndZIsBoneAlpha.z);
        } 

        if (userLightMode == USERLIGHTMODE_MULTIPLY)
        {
                Texel = saturate(Texel * GMX_ObjectsUserColor);
                Texel.a = saturate(Texel.a * pnt.ScreenPosition_AndZIsBoneAlpha.z);
        } 


        float3 DiffuseColor = float3 (0.0, 0.0, 0.0);
        float3 SpecularColor = float3 (0.0, 0.0, 0.0);
        float3 viewVec = vCamPosRelativeWorld - worldPos;
        float3 ReflectVector = normalize( -reflect( viewVec, Normal));


        //------- calculate lighting -----------------
#ifdef DISABLE_SPECUALR_LIGHT_FROM_POINT_LIGHTS
        CalculatePointLights(worldPos, 1.0, float3(1, 1, 1), Normal, worldPos, ShadowMapTex.rgb, DiffuseColor);
#else
        CalculatePointLights(worldPos, 1.0, float3(1, 1, 1), Normal, worldPos, ShadowMapTex.rgb, DiffuseColor, SpecularColor);
#endif  




        //------- calculate directional lighting -----------------
        float df_nDotL = saturate(dot(Normal, vGlobalLightDirection.xyz));
        DiffuseColor += (df_nDotL * vGlobalLightColor.xyz * ShadowMapTex.rgb);

        float db_nDotL = saturate(dot(Normal, -vGlobalLightDirection.xyz));
        DiffuseColor += ((db_nDotL * vGlobalLightBackColor.xyz) * ShadowMapTex.rgb);


        //------- final lighting -----------------
        
        
        float3 abmientLight = vAmbientLight.xyz * ShadowMapTex.rgb;
        
        float3 LightValue = (DiffuseColor * Texel.rgb);
        LightValue = saturate(LightValue * abmientLight);
        float3 TexelValue = (Texel.rgb * abmientLight);

        float3 finalColor = TexelValue + LightValue;
        float3 finalColorGammaCorrected = pow(finalColor, float3(vAmbientLight.a, vAmbientLight.a, vAmbientLight.a));


        return CalculateFog(worldPos, float4 (finalColorGammaCorrected, Texel.a));
}


STATIC_UBERSHADER(gmx_SingleTexture,gmxShowTexture,USERLIGHTMODE_ADD)

ANIMATED_UBERSHADER_01(gmx_SingleTexture,gmxShowTexture,USERLIGHTMODE_ADD)

ANIMATED_UBERSHADER_02(gmx_SingleTexture,gmxShowTexture,USERLIGHTMODE_ADD)

ANIMATED_UBERSHADER_03(gmx_SingleTexture,gmxShowTexture,USERLIGHTMODE_ADD)



