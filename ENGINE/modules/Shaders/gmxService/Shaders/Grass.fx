#include "technique.h"
#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"
#include "Shadows\shd_Variables.h"


#ifdef _XBOX360
[maxtempreg(GMX_MAX_TEMP_REGS)]
[reduceTempRegUsage(GMX_RECOMMENDED_TEMP_REGS)]
#endif
float4 gmxGrassPixelShader(GMX_PS_IN pnt) : COLOR
{
        float4 Texel = tex2D(DiffuseMap, pnt.TexCoord12.xy);
        clip(Texel.a - gmxAlphaRef);

        float ShadowBright = (1.0 - tex2Dproj(ShadowMap, pnt.ScreenPosition_AndZIsBoneAlpha).a);
        float3 worldPos = float3 (pnt.vNormal_And_WorldPosX.w, pnt.vTangent_And_WorldPosY.w, pnt.vBinormal_And_WorldPosZ.w);
        
        ShadowBright *= 0.45;
        ShadowBright = clamp(ShadowBright, 0.0, 0.45);
        float3 ShadowColor = lerp (float3(1.0, 1.0, 1.0), vShadowColor.rgb, ShadowBright);
        
        

        Texel = saturate(Texel * GMX_ObjectsUserColor * 4);
        Texel.rgb = Texel.rgb * ShadowColor;
        
        return CalculateFog(worldPos, Texel);
}


Technique(gmx_Grass, GMX_VS_IN_STATIC)
{
        pass P0
        {
            AlphaRef = 128;
                AlphaBlendEnable = true;
                AlphaTestEnable = true;
                VertexShader = (vshader_static);
                PixelShader = compile ps_3_0 gmxGrassPixelShader();
        }
}

Technique(gmx_Grass_anim_1, GMX_VS_IN_ANIM)
{
        pass P0 
        {
            AlphaRef = 128;
                AlphaBlendEnable = true;
                AlphaTestEnable = true;
                VertexShader = (vshader_anim01);
                PixelShader = compile ps_3_0 gmxGrassPixelShader();
        }
}

Technique(gmx_Grass_anim_2, GMX_VS_IN_ANIM)
{
        pass P0
        {
            AlphaRef = 128;
                AlphaBlendEnable = true;
                AlphaTestEnable = true;
                VertexShader = (vshader_anim02);
                PixelShader = compile ps_3_0 gmxGrassPixelShader();
        }
}

Technique(gmx_Grass_anim_3, GMX_VS_IN_ANIM)
{
        pass P0
        {
            AlphaRef = 128;
                AlphaBlendEnable = true;
                AlphaTestEnable = true;
                VertexShader = (vshader_anim03);
                PixelShader = compile ps_3_0 gmxGrassPixelShader();
        }
}







