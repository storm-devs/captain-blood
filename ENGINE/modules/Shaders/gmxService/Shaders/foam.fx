#include "technique.h"
#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"
#include "gmxService\gmx_VertexShader.h"
#include "gmxService\gmx_VertexShaderAnim.h"


sampler FoamMap =
sampler_state
{
        Texture = <GMX_TextureStage1>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Clamp;
        AddressV = Wrap;
};



float4 gmxFoam_ps(GMX_PS_IN pnt) : COLOR
{
   float2 uv = pnt.TexCoord12.xy;
   
   
   //0..1
   float timeFactor = (time * 10.0) + (GMX_ObjectsUserColor.a * 50);
   float nrmWave = 0.5 + cos(timeFactor) * 0.5; 

   //1.0...1.3
   uv.x = uv.x * (1.0 + (nrmWave * 0.3));
   
   
   
   float4 Texel = tex2D(FoamMap, uv);
   Texel.rgb = Texel.rgb * GMX_ObjectsUserColor.rgb;
   
   return Texel;

   //return float4(uv.xxx, 1.0); 
}


Technique(gmx_Foam, GMX_VS_IN_STATIC)
{
        pass P0
        {
                AlphaBlendEnable = true;
                AlphaTestEnable = false;
                SrcBlend = one;
                DestBlend = one;
                
                ZWriteenable = false;
                ZEnable = true;
                
                VertexShader = (vshader_static);
                PixelShader = compile ps_2_0 gmxFoam_ps();
        }
}

Technique(gmx_Foam_anim_1, GMX_VS_IN_ANIM)
{
        pass P0
        {
                AlphaBlendEnable = true;
                AlphaTestEnable = false;
                SrcBlend = one;
                DestBlend = one;
                
                ZWriteenable = false;
                ZEnable = true;
                
                VertexShader = (vshader_anim01);
                PixelShader = compile ps_2_0 gmxFoam_ps();
        }
}

Technique(gmx_Foam_anim_2, GMX_VS_IN_ANIM)
{
        pass P0
        {
                AlphaBlendEnable = true;
                AlphaTestEnable = false;
                SrcBlend = one;
                DestBlend = one;
                
                ZWriteenable = false;
                ZEnable = true;
                
                VertexShader = (vshader_anim02);
                PixelShader = compile ps_2_0 gmxFoam_ps();
        }
}

Technique(gmx_Foam_anim_3, GMX_VS_IN_ANIM)
{
        pass P0
        {
                AlphaBlendEnable = true;
                AlphaTestEnable = false;
                SrcBlend = one;
                DestBlend = one;
                
                ZWriteenable = false;
                ZEnable = true;
                
                VertexShader = (vshader_anim03);
                PixelShader = compile ps_2_0 gmxFoam_ps();
        }
}


