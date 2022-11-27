//SeaTrack

#include "technique.h"
#include "stdVariables.h"
#include "gmxService\gmx_Pack.h"
#include "gmxService\gmx_Variables.h"
#include "gmxService\gmx_Structs.h"



float TrackBlend = 0;

texture TrackTexture1;
texture TrackTexture2;


sampler TrackSampler1 =
sampler_state
{
    Texture = <TrackTexture1>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    
	AddressU = Clamp;
	AddressV = Wrap;
	AddressW = Wrap;
    
};

sampler TrackSampler2 =
sampler_state
{
    Texture = <TrackTexture2>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    
	AddressU = Clamp;
	AddressV = Wrap;
	AddressW = Wrap;
};




struct SeaTrackInput
{
        float3 vPos : POSITION;
        float4 vColor : COLOR;
        float2 vTexCoord0 : TEXCOORD0;
};


struct SeaTrackOut
{
        float4 vPos : POSITION;
        float4 vColor : COLOR;
        float2 vTexCoord0 : TEXCOORD0;
};

struct SeaTrackPSInput
{
        float4 vColor : COLOR;
        float2 vTexCoord0 : TEXCOORD0;
};



SeaTrackOut vs_SeaTrack(SeaTrackInput v)
{
        SeaTrackOut o;
        
        o.vPos = mul( float4(v.vPos, 1.0), mWorldViewProj);
        o.vColor = v.vColor;
        o.vTexCoord0 = v.vTexCoord0;
        
        return o;
}

float4 ps_SeaTrack(SeaTrackPSInput pnt) : COLOR
{
 float3 tex1 = tex2D (TrackSampler1, pnt.vTexCoord0.xy).rgb;
 float3 tex2 = tex2D (TrackSampler2, pnt.vTexCoord0.xy).rgb;
 
 float3 tex = lerp (tex1, tex2, TrackBlend) * pnt.vColor.a;

 return float4(tex, 1.0);
}


Technique(SeaTrack, SeaTrackInput)
{
        pass P0
        {
      
        ZWriteEnable = false;
        CullMode = none;

        AlphaBlendEnable = true;
        SrcBlend = one;
        DestBlend = one;
        
        AlphaTestEnable = false;
        
        AlphaRef = 16;
                
        VertexShader = compile vs_2_0 vs_SeaTrack();
        PixelShader = compile ps_2_0 ps_SeaTrack();
        }
}
