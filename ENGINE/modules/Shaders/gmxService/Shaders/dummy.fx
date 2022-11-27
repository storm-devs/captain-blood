#include "technique.h"
#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"


struct dummyVSOUT
{
        float4 pos:POSITION;
        float4 color:COLOR0;  
};


dummyVSOUT vs_dummy(GMX_VS_IN_STATIC v)
{
        dummyVSOUT o;

        o.pos = mul( float4(v.vPos.xyz, 1.0), mWorldViewProj);
        
        float3 Normal = UnpackVec(v.vPackedNormal);
        
        float k = saturate(dot (Normal, vGlobalLightDirection.xyz));
        k += 0.2f;
        
        o.color = float4(k, k, k, 1);

        return o;
}




float4 ps_dummy(float4 color: COLOR0) : COLOR
{
        float4 pixel;
        pixel.rgb = saturate(color.rgb + GMX_ObjectsUserColor.rgb);
        pixel.a = saturate(color.a * GMX_ObjectsUserColor.a);

        return pixel;
}


Technique(gmx_Dummy, GMX_VS_IN_STATIC)
{
        pass P0
        {
                AlphaBlendEnable = true;
                AlphaTestEnable = false;
                VertexShader = compile vs_2_0 vs_dummy();
                PixelShader = compile ps_2_0 ps_dummy();
        }
}



