#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"


float4 gmxShowTangents(GMX_PS_IN pnt) : COLOR
{
        return float4(0.5 + (pnt.vTangent_And_WorldPosY.xyz * 0.5), 1.0); 
}


Technique(gmx_ShowTangents, GMX_VS_IN_STATIC)
{
        pass P0
        {
                AlphaBlendEnable = false;
                AlphaTestEnable = false;
                VertexShader = (vshader_static);
                PixelShader = compile ps_2_0 gmxShowTangents();
        }
}

Technique(gmx_ShowTangents_anim_1, GMX_VS_IN_ANIM)
{
        pass P0 
        {
                AlphaBlendEnable = false;
                AlphaTestEnable = false;
                VertexShader = (vshader_anim01);
                PixelShader = compile ps_2_0 gmxShowTangents();
        }
}

Technique(gmx_ShowTangents_anim_2, GMX_VS_IN_ANIM)
{
        pass P0
        {
                AlphaBlendEnable = false;
                AlphaTestEnable = false;
                VertexShader = (vshader_anim02);
                PixelShader = compile ps_2_0 gmxShowTangents();
        }
}

Technique(gmx_ShowTangents_anim_3, GMX_VS_IN_ANIM)
{
        pass P0
        {
                AlphaBlendEnable = false;
                AlphaTestEnable = false;
                VertexShader = (vshader_anim03);
                PixelShader = compile ps_2_0 gmxShowTangents();
        }
}




