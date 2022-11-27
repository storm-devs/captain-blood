#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"


#ifdef _XBOX360
[maxtempreg(GMX_MAX_TEMP_REGS)]
[reduceTempRegUsage(GMX_RECOMMENDED_TEMP_REGS)]
#endif
float4 gmxShowTextureFoliage(GMX_PS_IN pnt, uniform bool bDontLight, uniform int userLightMode) : COLOR
{
  float2 uv1 = pnt.TexCoord12.xy;

  
  float2 uv2 = pnt.TexCoord12.zw;
        
  float4 Bump1 = tex2D(NormalMap, (uv1 * 0.06) + float2 (time, -time));
        
  Bump1.xy = ((Bump1.xy - 0.5) * 2.0);
  Bump1.zw = 0.0;
  uv1 += (Bump1.xy * float2(0.03, 0.03));

  float4 Texel = tex2D(DiffuseMap, uv1);
  clip(Texel.a - gmxAlphaRef);

  
  
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



        float4 Specular = tex2D(SpecularMap, uv2);
        float Shininess = Specular.r;
        float Gloss = Specular.g;
        float AmbientOcclusion = Specular.b;
        float4 ShadowMapTex = tex2Dproj(ShadowMap, pnt.ScreenPosition_AndZIsBoneAlpha);
        Gloss *= ShadowMapTex.a;
        Texel.rgb = Texel.rgb * AmbientOcclusion;


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
        CalculatePointLights(worldPos, Shininess, ReflectVector, Normal, worldPos, ShadowMapTex.rgb, DiffuseColor);
#else
        CalculatePointLights(worldPos, Shininess, ReflectVector, Normal, worldPos, ShadowMapTex.rgb, DiffuseColor, SpecularColor);
#endif




        //------- calculate directional lighting -----------------
        float df_nDotL = saturate(dot(Normal, vGlobalLightDirection.xyz));
        DiffuseColor += (df_nDotL * vGlobalLightColor.xyz * ShadowMapTex.rgb);

        float db_nDotL = saturate(dot(Normal, -vGlobalLightDirection.xyz));
        DiffuseColor += ((db_nDotL * vGlobalLightBackColor.xyz) * ShadowMapTex.rgb);

        float d_rDotL = saturate(dot( ReflectVector, vGlobalLightDirectionSpec.xyz));
        d_rDotL = pow(d_rDotL, Shininess * 32.0f);
        SpecularColor += (vGlobalLightColorSpecular.xyz * d_rDotL);
        
        
        
        SpecularColor = float3 (0.0, 0.0, 0.0);
        Gloss = 0.0;

        //------- final lighting -----------------
        
        float3 abmientLight = vAmbientLight.xyz * ShadowMapTex.rgb;
        
        float3 LightValue = (DiffuseColor * Texel.rgb) + (SpecularColor * Gloss);
        LightValue = saturate(LightValue * abmientLight);
        float3 TexelValue = (Texel.rgb * abmientLight);

        float3 finalColor = TexelValue + LightValue;
        float3 finalColorGammaCorrected = pow(finalColor, float3(vAmbientLight.a, vAmbientLight.a, vAmbientLight.a));


 
        return CalculateFog(worldPos, float4 (finalColorGammaCorrected, Texel.a));

}


STATIC_UBERSHADER(gmx_Foliage,gmxShowTextureFoliage,USERLIGHTMODE_ADD)

ANIMATED_UBERSHADER_01(gmx_Foliage,gmxShowTextureFoliage,USERLIGHTMODE_ADD)

ANIMATED_UBERSHADER_02(gmx_Foliage,gmxShowTextureFoliage,USERLIGHTMODE_ADD)

ANIMATED_UBERSHADER_03(gmx_Foliage,gmxShowTextureFoliage,USERLIGHTMODE_ADD)



