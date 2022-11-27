#include "technique.h"
#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"


float4 gmxWeaponSFXPixelShader(GMX_PS_IN pnt) : COLOR
{
    float Alpha = (pnt.ScreenPosition_AndZIsBoneAlpha.z * GMX_ObjectsUserColor.a);
    
        float4 Texel = tex2D(DiffuseMap, pnt.TexCoord12.xy) * Alpha;
        
        Texel.rgb = Texel.rgb + GMX_ObjectsUserColor.rgb;

        return Texel; 
}


Technique(gmx_WeaponSFX, GMX_VS_IN_STATIC)
{
        pass P0
        {
           CullMode = None;
           ZWriteEnable = false;

           SrcBlend = srcalpha;
           DestBlend = one;
           AlphaTestEnable = false;
           AlphaBlendEnable = true;

                VertexShader = (vshader_static);
                PixelShader = compile ps_2_0 gmxWeaponSFXPixelShader();
        }
}

Technique(gmx_WeaponSFX_anim_1, GMX_VS_IN_ANIM)
{
        pass P0 
        {
           CullMode = None;
           ZWriteEnable = false;

           SrcBlend = srcalpha;
           DestBlend = one;
           AlphaTestEnable = false;
           AlphaBlendEnable = true;

//                AlphaBlendEnable = true;
//                AlphaTestEnable = true;
                VertexShader = (vshader_anim01);
                PixelShader = compile ps_2_0 gmxWeaponSFXPixelShader();
        }
}

Technique(gmx_WeaponSFX_anim_2, GMX_VS_IN_ANIM)
{
        pass P0
        {
           CullMode = None;
           ZWriteEnable = false;

           SrcBlend = srcalpha;
           DestBlend = one;
           AlphaTestEnable = false;
           AlphaBlendEnable = true;

//                AlphaBlendEnable = true;
//                AlphaTestEnable = true;
                VertexShader = (vshader_anim02);
                PixelShader = compile ps_2_0 gmxWeaponSFXPixelShader();
        }
}

Technique(gmx_WeaponSFX_anim_3, GMX_VS_IN_ANIM)
{
        pass P0
        {
           CullMode = None;
           ZWriteEnable = false;

           SrcBlend = srcalpha;
           DestBlend = one;
           AlphaTestEnable = false;
           AlphaBlendEnable = true;

//                AlphaBlendEnable = true;
//                AlphaTestEnable = true;
                VertexShader = (vshader_anim03);
                PixelShader = compile ps_2_0 gmxWeaponSFXPixelShader();
        }
}







