#ifndef GMXUBERSHADER
#define GMXUBERSHADER


#include "gmxService\gmx_Structs.h"
#include "gmxService\gmx_VertexShader.h"
#include "gmxService\gmx_VertexShaderAnim.h"




#define USERLIGHTMODE_ADD      0
#define USERLIGHTMODE_MULTIPLY 1
#define USERLIGHTMODE_NONE     3


void CalcPointLight (int index,
                                         float Shininess,
                                         float3 ReflectVector,
                                         float3 Normal,
                                         float3 TexelWorldPos,
                                         float3 ShadowValue,
                                         in out float3 DiffuseColor,
                                         in out float3 SpecularColor
                                         )
{
        float3 d = vPointLightPos30[index] - TexelWorldPos;
        float3 dir = normalize(d);
        float Attenuation = saturate(1.0 - dot (d, d) * fPointLightParams[index].x); 
        float nDotL = saturate(dot(Normal, dir));


        float3 lightColor = vPointLightColor30[index].rgb * saturate(ShadowValue + fPointLightParams[index].w);


        DiffuseColor += nDotL * Attenuation * lightColor;

#ifndef DISABLE_SPECUALR_LIGHT_FROM_POINT_LIGHTS
        float rDotL = saturate(dot( ReflectVector, dir));
        rDotL = pow(rDotL, Shininess * 32.0f);

        float PntSpecularPower = (rDotL * Attenuation);
        SpecularColor += (PntSpecularPower * vPointLightColorSpecular30[index].rgb);
#endif
}


void CalculatePointLights (float3 vWorldPos,
                                                   float Shininess,
                                                   float3 ReflectVector,
                                                   float3 Normal,
                                                   float3 TexelWorldPos,
                                                   float3 ShadowValue,
#ifdef DISABLE_SPECUALR_LIGHT_FROM_POINT_LIGHTS
                                                   in out float3 DiffuseColor
#else
                                                   in out float3 DiffuseColor,
                                                   in out float3 SpecularColor
#endif
                                                   )
{

#ifdef DISABLE_SPECUALR_LIGHT_FROM_POINT_LIGHTS
        float3 SpecularColor = float3(0, 0, 0);
#endif

        //------- calculate lighting -----------------
        if (bLightEnabled30[0])
        {
                CalcPointLight(0, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                if (bLightEnabled30[1])
                {
                        CalcPointLight(1, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                        if (bLightEnabled30[2])
                        {
                                CalcPointLight(2, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                if (bLightEnabled30[3])
                                {
                                        CalcPointLight(3, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                        if (bLightEnabled30[4])
                                        {
                                                CalcPointLight(4, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                                if (bLightEnabled30[5])
                                                {
                                                        CalcPointLight(5, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                                        if (bLightEnabled30[6])
                                                        {
                                                                CalcPointLight(6, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                                                if (bLightEnabled30[7])
                                                                {
                                                                        CalcPointLight(7, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }


        if (bLightEnabled30[8])
        {
                CalcPointLight(8, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                if (bLightEnabled30[9])
                {
                        CalcPointLight(9, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                        if (bLightEnabled30[10])
                        {
                                CalcPointLight(10, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                if (bLightEnabled30[11])
                                {
                                        CalcPointLight(11, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                        if (bLightEnabled30[12])
                                        {
                                                CalcPointLight(12, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                                if (bLightEnabled30[13])
                                                {
                                                        CalcPointLight(13, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                                        if (bLightEnabled30[14])
                                                        {
                                                                CalcPointLight(14, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);

                                                                if (bLightEnabled30[15])
                                                                {
                                                                        CalcPointLight(15, Shininess, ReflectVector, Normal, vWorldPos, ShadowValue, DiffuseColor, SpecularColor);
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }

}



#ifdef _XBOX360
[maxtempreg(GMX_MAX_TEMP_REGS)]
[reduceTempRegUsage(GMX_RECOMMENDED_TEMP_REGS)]
#endif
float4 gmxUberShader(GMX_PS_IN pnt, uniform bool bDontLight, uniform int userLightMode) : COLOR
{
        float2 uv1 = pnt.TexCoord12.xy;
		float4 Texel = tex2D(DiffuseMap, uv1);

		clip(Texel.a - gmxAlphaRef);

		//return Texel;

        float2 uv2 = pnt.TexCoord12.zw;


        float4 TexelNormal = 2.0 * tex2D(NormalMap, uv1) - 1.0;
        

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
        float3 Normal = pnt.vNormal_And_WorldPosX.xyz * TexelNormal.z + pnt.vTangent_And_WorldPosY.xyz * TexelNormal.x + pnt.vBinormal_And_WorldPosZ.xyz * -TexelNormal.y;



        float4 Specular = tex2D(SpecularMap, uv2);
        float Shininess = Specular.r;
        Shininess = Shininess + 0.1;
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




        SpecularColor = (SpecularColor * Gloss);


		//фейковый спекуляр "из глаз"
		d_rDotL = saturate(dot( ReflectVector, float3 (-vCamDirRelativeWorld.x, vCamDirRelativeWorld.y, -vCamDirRelativeWorld.z)));
		d_rDotL = pow(d_rDotL, vAmbientLightSpecular.a);
		SpecularColor += ((vAmbientLightSpecular.rgb * d_rDotL) * ShadowMapTex.a);


        //------- final lighting -----------------

        float3 abmientLight = vAmbientLight.xyz * ShadowMapTex.rgb;

        float3 LightValue = (DiffuseColor * Texel.rgb) + SpecularColor;

        LightValue = saturate(LightValue * abmientLight);
        float3 TexelValue = (Texel.rgb * abmientLight);

        float3 finalColor = TexelValue + LightValue;
        float3 finalColorGammaCorrected = pow(finalColor, float3(vAmbientLight.a, vAmbientLight.a, vAmbientLight.a));


        return CalculateFog(worldPos, float4 (finalColorGammaCorrected, Texel.a));
}



#endif