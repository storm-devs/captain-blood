#include "technique.h"
#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"



struct decalVSOUT
{
        float4 pos:POSITION;
        float2 uv:TEXCOORD0;  
};


decalVSOUT vs_Decal(GMX_VS_IN_STATIC v)
{
        decalVSOUT o;
        o.pos = mul( float4(v.vPos.xyz, 1.0), mWorldViewProj);
        o.uv = UnpackUV(v.vPackedUV1);
        return o;
}

decalVSOUT vs_Decal_anim(GMX_VS_IN_ANIM v, uniform int nNumBones)
{
	decalVSOUT o;
	GMX_VS_OUT tmp = GMX_VertexShaderAnim(v, nNumBones);
	o.pos = tmp.Position;
	o.uv = UnpackUV(v.s0.vPackedUV1);
	return o;
}




float4 ps_Decal(float2 uv: TEXCOORD0) : COLOR
{
        float4 Tex1 = tex2D(DiffuseMap, uv);
        return Tex1; 
}

float4 ps_DecalNull(float2 uv: TEXCOORD0) : COLOR
{
        return float4(0.0, 0.0, 0.0, 0.0); 
}






Technique(gmx_Decal, GMX_VS_IN_STATIC)
{
        pass P0
        {
                ZWriteEnable = false;
				AlphaBlendEnable = true;
				AlphaTestEnable = false;
				SrcBlend = srcalpha;
				DestBlend = invsrcalpha;
                VertexShader = compile vs_2_0 vs_Decal();
                PixelShader = compile ps_2_0 ps_Decal();
        }
}


//------------------------------------------------------------------------------

Technique(gmx_Decal_anim_1, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
				AlphaBlendEnable = true;
				AlphaTestEnable = false;
				SrcBlend = srcalpha;
				DestBlend = invsrcalpha;
                VertexShader = compile vs_2_0 vs_Decal_anim(1);
                PixelShader = compile ps_2_0 ps_Decal();
        }
}

//------------------------------------------------------------------------------

Technique(gmx_Decal_anim_2, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
				AlphaBlendEnable = true;
				AlphaTestEnable = false;
				SrcBlend = srcalpha;
				DestBlend = invsrcalpha;
                VertexShader = compile vs_2_0 vs_Decal_anim(2);
                PixelShader = compile ps_2_0 ps_Decal();
        }
}

//------------------------------------------------------------------------------

Technique(gmx_Decal_anim_3, GMX_VS_IN_ANIM)
{
        pass P0
        {
                ZWriteEnable = false;
				AlphaBlendEnable = true;
				AlphaTestEnable = false;
				SrcBlend = srcalpha;
				DestBlend = invsrcalpha;
                VertexShader = compile vs_2_0 vs_Decal_anim(3);
                PixelShader = compile ps_2_0 ps_Decal();
        }
}

