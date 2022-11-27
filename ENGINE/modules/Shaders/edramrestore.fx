#include "technique.h"
//

struct EDRAMVertex
{
 float4 pos: POSITION;
 float2 uv : TEXCOORD0;
};

struct ZSPRITE_PIXEL
{
    float4 Color : COLOR0;
    float  Depth : DEPTH0;
};



texture EDRAM_Color;
sampler EDRAM_ColorSampler =
sampler_state
{
    Texture = <EDRAM_Color>;
    MipFilter = POINT;
    MinFilter = POINT;
    MagFilter = POINT;
};

texture EDRAM_Depth;
sampler EDRAM_DepthSampler =
sampler_state
{
    Texture = <EDRAM_Depth>;
    MipFilter = POINT;
    MinFilter = POINT;
    MagFilter = POINT;
};







void vsX360RestoreEDRAM( float4 vPos : POSITION,
                         float2 vTex0 : TEXCOORD0,
                         out float4 oPos : POSITION,
                         out float2 oTex0 : TEXCOORD0 )
{
    oPos = vPos;
    oTex0 = vTex0; 
}



ZSPRITE_PIXEL psX360RestoreEDRAM( in float2 vScreenPosition : TEXCOORD0)
{
  ZSPRITE_PIXEL o;
  
  o.Color = tex2D( EDRAM_ColorSampler, vScreenPosition);
  o.Depth = tex2D( EDRAM_DepthSampler, vScreenPosition);
  
  return o;
}




Technique(X360RestoreEDRAM, EDRAMVertex)
{
 pass P0
  {
   CullMode = NONE;
   AlphaBlendEnable = FALSE;
   AlphaTestEnable = FALSE;
   ZEnable = TRUE;
   ZFUNC = ALWAYS;

   VertexShader = compile vs_2_0 vsX360RestoreEDRAM();
   PixelShader = compile ps_2_0 psX360RestoreEDRAM();
  }
}




float4 psX360RestoreEDRAM_ColorOnly( in float2 vScreenPosition : TEXCOORD0) : COLOR0
{
  float4 clr = tex2D( EDRAM_ColorSampler, vScreenPosition);
  return clr;
}




Technique(X360RestoreEDRAM_ColorOnly, EDRAMVertex)
{
 pass P0
  {
   CullMode = NONE;
   AlphaBlendEnable = FALSE;
   AlphaTestEnable = FALSE;
   ZEnable = false;

   VertexShader = compile vs_2_0 vsX360RestoreEDRAM();
   PixelShader = compile ps_2_0 psX360RestoreEDRAM_ColorOnly();
  }
}

// Vano added: depth only restore

ZSPRITE_PIXEL psX360RestoreEDRAM_DepthOnly( in float2 vScreenPosition : TEXCOORD0)
{
  ZSPRITE_PIXEL o;
  o.Color = 0.0f;
  o.Depth = tex2D( EDRAM_DepthSampler, vScreenPosition);
  return o;
}

Technique(X360RestoreEDRAM_DepthOnly, EDRAMVertex)
{
 pass P0
  {
   CullMode = NONE;
   AlphaBlendEnable = FALSE;
   AlphaTestEnable = FALSE;
   ColorWriteEnable = 0;
   ZEnable = TRUE;
   ZFUNC = ALWAYS;

   VertexShader = compile vs_2_0 vsX360RestoreEDRAM();
   PixelShader = compile ps_2_0 psX360RestoreEDRAM_DepthOnly();
  }
}



//------------------------------------------------------------------------


sampler X360Upscale3DTo2DSampler =
sampler_state
{
    Texture = <EDRAM_Color>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

float4 psX360Upscale3DTo2D( in float2 vScreenPosition : TEXCOORD0) : COLOR0
{
  float4 clr = tex2D( X360Upscale3DTo2DSampler, vScreenPosition);
  return clr;
}

Technique(X360Upscale3DTo2D, EDRAMVertex)
{
 pass P0
  {
   CullMode = NONE;
   AlphaBlendEnable = FALSE;
   AlphaTestEnable = FALSE;
   ZEnable = false;

   VertexShader = compile vs_2_0 vsX360RestoreEDRAM();
   PixelShader = compile ps_2_0 psX360Upscale3DTo2D();
  }
}
