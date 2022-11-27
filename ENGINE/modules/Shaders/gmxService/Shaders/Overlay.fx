#include "technique.h"
#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"



struct gmx_OverlayVSOUT
{
        float4 pos:POSITION;
        float2 uv1:TEXCOORD0;  
        float2 uv2:TEXCOORD1;  
};


gmx_OverlayVSOUT vs_Overlay(GMX_VS_IN_STATIC v)
{
        gmx_OverlayVSOUT o;
        o.pos = mul( float4(v.vPos.xyz, 1.0), mWorldViewProj);
        o.uv1 = UnpackUV(v.vPackedUV1);
        o.uv2 = UnpackUV(v.vPackedUV2);
        return o;
}

gmx_OverlayVSOUT vs_OverlayAnim(GMX_VS_IN_ANIM v, uniform int nNumBones)
{
       gmx_OverlayVSOUT o;
        
       GMX_VS_OUT tmp = GMX_VertexShaderAnim(v, nNumBones);
       o.pos = tmp.Position;

       o.uv1 = UnpackUV(v.s0.vPackedUV1);
       o.uv2 = UnpackUV(v.s0.vPackedUV1);
       return o;
}


float4 ps_Overlay(float2 uv: TEXCOORD0, float2 uv2: TEXCOORD1) : COLOR
{
        float4 Texel = tex2D(DiffuseMap, uv);
        
        Texel.rgb = Texel.rgb * GMX_ObjectsUserColor.rgb * GMX_ObjectsUserColor.aaa;
        return float4(Texel.rgb, 1.0); 
}




Technique(gmx_Overlay, GMX_VS_IN_STATIC)
{
        pass P0
        {
                ZWriteEnable = false;
                CullMode = none;
                AlphaBlendEnable = true;
                SrcBlend = destcolor;
                DestBlend = srccolor;
                VertexShader = compile vs_2_0 vs_Overlay();
                PixelShader = compile ps_2_0 ps_Overlay();
        }
}


Technique(gmx_Overlay_anim_1, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
                CullMode = none;
                AlphaBlendEnable = true;
                SrcBlend = destcolor;
                DestBlend = srccolor;
                VertexShader = compile vs_2_0 vs_OverlayAnim(1);
                PixelShader = compile ps_2_0 ps_Overlay();
        }
}

Technique(gmx_Overlay_anim_2, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
                CullMode = none;
                AlphaBlendEnable = true;
                SrcBlend = destcolor;
                DestBlend = srccolor;
                VertexShader = compile vs_2_0 vs_OverlayAnim(2);
                PixelShader = compile ps_2_0 ps_Overlay();
        }
}

Technique(gmx_Overlay_anim_3, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
                CullMode = none;
                AlphaBlendEnable = true;
                SrcBlend = destcolor;
                DestBlend = srccolor;
                VertexShader = compile vs_2_0 vs_OverlayAnim(3);
                PixelShader = compile ps_2_0 ps_Overlay();
        }
}





