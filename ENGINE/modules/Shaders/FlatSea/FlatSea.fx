//GMXFlatSea

#include "technique.h"
#include "stdVariables.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"


float fs_bumpTextureAnimFrame;

float4 fFlatSeaBumpScale;
float4 fs_vBumpOffset;




float4 fs_vBumpScale;
float4 fs_vWaterColor;
float4 fs_vUnderWaterFogColor;
float4 fs_vColor;
float4 fs_vUnderWaterColor;




texture fs_bumpTexture;
texture fs_reflTexture;
texture fs_refrTexture;


sampler samplerBump =
sampler_state
{
    Texture = <fs_bumpTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};


sampler samplerReflection =
sampler_state
{
        Texture = <fs_reflTexture>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Clamp;
        AddressV = Clamp;
};


sampler samplerRefraction =
sampler_state
{
        Texture = <fs_refrTexture>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Clamp;
        AddressV = Clamp;
};




struct FlatSeaOut
{
  float4 Position:POSITION;
  
  float3 texCoord0:TEXCOORD0;
  float4 texCoord1:TEXCOORD1;
  
  float3 VertexPos:TEXCOORD2;
};

struct FlatSeaIn
{
  float3 texCoord0:TEXCOORD0;
  float4 texCoord1:TEXCOORD1;
  float3 VertexPos:TEXCOORD2;
};



FlatSeaOut vs_FlatSea(GMX_VS_IN_STATIC v)
{
        FlatSeaOut o;
        o.Position = mul( float4(v.vPos, 1.0), mWorldViewProj);
        
        o.texCoord0.xy = UnpackUV(v.vPackedUV1) * fFlatSeaBumpScale.xy;
        o.texCoord0.xy += fs_vBumpOffset.xy;
        o.texCoord0.z = fs_bumpTextureAnimFrame;
        
        o.texCoord1 = mul( float4(v.vPos, 1.0), mWorldViewProj);
        
        o.VertexPos = v.vPos;
        return o;
}

float4 ps_FlatSea(FlatSeaIn pnt) : COLOR
{
        float3 Bump = tex3D(samplerBump, pnt.texCoord0.xyz).rbg;
        Bump = (Bump * 2.0) - 1.0;



        float LightK = clamp(dot(Bump, vGlobalLightDirection.xyz), 0.7, 1);
        float3 Diffuse = LightK * fs_vWaterColor.rgb;

        //Вектор глаз <-> тексель
        float3 ViewVectorObjectSpace = vCamPos - pnt.VertexPos.xyz;
        float Angle = dot( normalize(ViewVectorObjectSpace), float3(0, 1, 0));
        Angle = clamp((Angle * Angle) , 0.01, 1.0);


        
        float2 screenUV = pnt.texCoord1.xy / float2(pnt.texCoord1.w, pnt.texCoord1.w);
        screenUV = (screenUV * 0.5) + 0.5;
        screenUV.y = (1.0 - screenUV.y);
        
        float2 ReflectionUV = screenUV + (Bump.xz * fs_vBumpScale.xy);
        float2 RefractionUV = screenUV + (Bump.xz * fs_vBumpScale.z * fs_vBumpScale.xy);
        
        
        float3 txReflection = tex2D(samplerReflection, ReflectionUV).rgb;
        float3 txRefraction = tex2D(samplerRefraction, RefractionUV).rgb;
        
        txReflection = txReflection * fs_vColor.rgb;
        txReflection = lerp (txReflection, fs_vWaterColor.rgb, fs_vWaterColor.a);
  
        txRefraction = txRefraction * fs_vUnderWaterColor.rgb;
        txRefraction = lerp (txRefraction, fs_vUnderWaterFogColor.rgb, fs_vUnderWaterFogColor.a);
  
  
        float3 ResultColor = lerp (txReflection, txRefraction, Angle);
  
        float3 LightValue = (ResultColor*Diffuse) * vAmbientLight.xyz;

        float3 ResultRGB =  (ResultColor * vAmbientLight.xyz) + LightValue;
        

        return float4(ResultRGB, 1.0);
}


Technique(GMXFlatSea, GMX_VS_IN_STATIC)
{
        pass P0
        {
                AlphaBlendEnable = false;
                AlphaTestEnable = false;
                VertexShader = compile vs_2_0 vs_FlatSea();
                PixelShader = compile ps_2_0 ps_FlatSea();
        }
}
