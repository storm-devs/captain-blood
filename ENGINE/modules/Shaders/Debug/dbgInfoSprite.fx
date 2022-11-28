#include "technique.h"


texture Texture;
sampler SprSampler =
sampler_state
{
    Texture = <Texture>;
    MinFilter = Linear;
    MagFilter = Linear;
};



struct SpriteVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
 float2 uv : TEXCOORD0;
};



struct VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
};



VS_OUTPUT vs_dbgInfoSprite( SpriteVertex vrx)
{
 VS_OUTPUT res;
 res.pos = float4(vrx.pos, 1);
 res.color = vrx.color;
 res.uv = vrx.uv;
 return res;
}


float4 ps_dbgInfoSprite(PS_INPUT pnt) : COLOR
{
 return float4(0.37, 0.37, 0.37, 0.45);
}

float4 ps_dbgSprites(PS_INPUT pnt) : COLOR
{
 float4 tex = tex2D(SprSampler, pnt.uv);
 return tex * pnt.color;
}

float4 ps_dbgSpritesColorOnly(PS_INPUT pnt) : COLOR
{
 return pnt.color;
}

Technique(dbgInfoSprite, SpriteVertex)
{
 pass P0
  {
    CullMode = none;
    AlphaBlendEnable = true;
    VertexShader = compile vs_2_0 vs_dbgInfoSprite();
    PixelShader = compile ps_2_0 ps_dbgInfoSprite();
  }
}

Technique(dbgSprites, SpriteVertex)
{
 pass P0
  {
    CullMode = none;
    AlphaBlendEnable = true;

    VertexShader = compile vs_2_0 vs_dbgInfoSprite();
    PixelShader = compile ps_2_0 ps_dbgSprites();
  }
}

Technique(dbgSpritesColorOnly, SpriteVertex)
{
 pass P0
  {
    CullMode = none;
    AlphaBlendEnable = true;

    VertexShader = compile vs_2_0 vs_dbgInfoSprite();
    PixelShader = compile ps_2_0 ps_dbgSpritesColorOnly();
  }
}

Technique(dbgSpritesNoblend, SpriteVertex)
{
 pass P0
  {
    CullMode = none;
    AlphaBlendEnable = false;

    VertexShader = compile vs_2_0 vs_dbgInfoSprite();
    PixelShader = compile ps_2_0 ps_dbgSprites();
  }
}



