#include "technique.h"
#include "stdVariables.h"


texture SunTexture;

sampler STexture =
sampler_state
{
    Texture = <SunTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};



struct SunParticles_VS_IN
{
        float3 vOffset : POSITION;
        float4 vColor : COLOR0;
        float2 UV1 : TEXCOORD1;
        float Angle : TEXCOORD3;
        float3 vPosition : TEXCOORD5;
        float fDisolve : TEXCOORD6;
};

struct SunParticles_VS_OUT
{
        float4 Position: POSITION0;
        float4 Color : TEXCOORD0;
        float2 UV1 : TEXCOORD1;
        float fDisolve : TEXCOORD2;
};


struct SunParticles_PS_IN
{
        float4 Color : TEXCOORD0;
        float2 UV1 : TEXCOORD1;
        float fDisolve : TEXCOORD2;
};



SunParticles_VS_OUT vs_SunParticles(SunParticles_VS_IN v)
{
        SunParticles_VS_OUT o;

        float vSin;
        float vCos;
        sincos(v.Angle, vSin, vCos);

        float2 ViewOffset;
        ViewOffset.x = (v.vOffset.x * vCos) + (v.vOffset.y * vSin);
        ViewOffset.y = ((v.vOffset.y * vCos) - (v.vOffset.x * vSin));

        float4 ViewSpace = mul(float4(v.vPosition, 1.0), mView);
        ViewSpace.xy = ViewSpace.xy + ViewOffset;
        o.Position = mul(ViewSpace, mProjection);

        //   fBlend; // К - блендинга между кадрами
        //   AddPowerK; // K - силы выжигания

        o.UV1 = v.UV1;
        o.Color = v.vColor;
        o.fDisolve = v.fDisolve;

        return o;
}



float4 ps_SunParticles(SunParticles_PS_IN v) : COLOR0
{
 half4 pixel = tex2D(STexture, v.UV1) * (float4(v.Color.xyz, v.fDisolve));
 
 pixel = pixel * 2.0;

 return pixel; 

}

float4 ps_SunParticlesBlend(SunParticles_PS_IN v) : COLOR0
{
 half4 pixel = tex2D(STexture, v.UV1) * (float4(v.Color.xyz, v.fDisolve));

 return pixel; 

}






Technique(BBCollection_Add2X, SunParticles_VS_IN)
{
        pass P0
        {
                CullMode = none;
                ZWriteenable = false;
                ZEnable = false;

                SrcBlend = one;
                DestBlend = one;
                AlphaTestEnable = false;
                AlphaBlendEnable = true;

                VertexShader = compile vs_2_0 vs_SunParticles();
                PixelShader = compile ps_2_0 ps_SunParticles();
        }
}

Technique(BBCollection_Blend, SunParticles_VS_IN)
{
        pass P0
        {
                CullMode = none;
                ZWriteenable = false;
                ZEnable = true;

                SrcBlend = srcalpha;
                DestBlend = invsrcalpha;
                AlphaTestEnable = false;
                AlphaBlendEnable = true;

                VertexShader = compile vs_2_0 vs_SunParticles();
                PixelShader = compile ps_2_0 ps_SunParticlesBlend();
        }
}

