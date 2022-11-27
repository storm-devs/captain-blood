#include "technique.h"
#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"



struct volumeLightVSOUT
{
        float4 pos:POSITION;
        float2 uv1:TEXCOORD0;  
        float2 uv2:TEXCOORD1;  
};


volumeLightVSOUT vs_VolumeLight(GMX_VS_IN_STATIC v)
{
        volumeLightVSOUT o;
        o.pos = mul( float4(v.vPos.xyz, 1.0), mWorldViewProj);
        o.uv1 = UnpackUV(v.vPackedUV1);
        o.uv2 = UnpackUV(v.vPackedUV2);
        return o;
}

volumeLightVSOUT vs_VolumeLightAnim(GMX_VS_IN_ANIM v, uniform int nNumBones)
{
        volumeLightVSOUT o;
        
	    GMX_VS_OUT tmp = GMX_VertexShaderAnim(v, nNumBones);
	    o.pos = tmp.Position;

        o.uv1 = UnpackUV(v.s0.vPackedUV1);
        o.uv2 = UnpackUV(v.s0.vPackedUV1);
        return o;
        
}




float4 ps_VolumeLight(float2 uv: TEXCOORD0, float2 uv2: TEXCOORD1) : COLOR
{
		//rays
        float4 Tex1 = tex2D(SpecularMap, uv);
        Tex1.rgb = saturate(Tex1.rgb * (GMX_ObjectsUserColor.rgb * 4.0));
        
        //dust (with scroll)
        float4 Tex2 = tex2D(DiffuseMap, uv2 + float2 (0.0, -time*0.5));
        Tex2.rgb = saturate((Tex2.rgb * 2.0f) * GMX_ObjectsUserColor.a);
        
        float3 res = Tex1.rgb * Tex2.rgb;

        return float4(res, 1.0); 
}




Technique(gmx_VolumetricRays, GMX_VS_IN_STATIC)
{
        pass P0
        {
                ZWriteEnable = false;
                CullMode = none;
                AlphaBlendEnable = true;
                SrcBlend = one;
                DestBlend = one;
                VertexShader = compile vs_2_0 vs_VolumeLight();
                PixelShader = compile ps_2_0 ps_VolumeLight();
        }
}


Technique(gmx_VolumetricRays_anim_1, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
                CullMode = none;
                AlphaBlendEnable = true;
                SrcBlend = one;
                DestBlend = one;
                VertexShader = compile vs_2_0 vs_VolumeLightAnim(1);
                PixelShader = compile ps_2_0 ps_VolumeLight();
        }
}

Technique(gmx_VolumetricRays_anim_2, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
                CullMode = none;
                AlphaBlendEnable = true;
                SrcBlend = one;
                DestBlend = one;
                VertexShader = compile vs_2_0 vs_VolumeLightAnim(2);
                PixelShader = compile ps_2_0 ps_VolumeLight();
        }
}

Technique(gmx_VolumetricRays_anim_3, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
                CullMode = none;
                AlphaBlendEnable = true;
                SrcBlend = one;
                DestBlend = one;
                VertexShader = compile vs_2_0 vs_VolumeLightAnim(3);
                PixelShader = compile ps_2_0 ps_VolumeLight();
        }
}





