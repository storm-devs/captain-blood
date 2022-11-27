#include "technique.h"
texture VideoTex;
sampler VideoSampler =
sampler_state
{
    Texture = <VideoTex>;
    MinFilter = Point;
    MagFilter = Point;
};


struct VideoVertex
{
 float3 pos : POSITION;
 float2 uv : TEXCOORD0;
};


struct Video_VS_OUTPUT
{
 float4 pos: POSITION;
 float2 uv : TEXCOORD0;
};

struct Video_PS_INPUT
{
 float2 uv : TEXCOORD0;
};




Video_VS_OUTPUT VideoVertexShader( VideoVertex vrx)
{
 Video_VS_OUTPUT res;
 res.pos = float4(vrx.pos, 1);
 res.uv = vrx.uv;
 return res;
}


float4 VideoPixelShader( Video_PS_INPUT pnt) : COLOR
{
 float Y = tex2D( VideoSampler, float2(pnt.uv.x, 0.6666 - pnt.uv.y * 0.6666) ).x;
 float V = tex2D( VideoSampler, float2(pnt.uv.x * 0.5, 1 - pnt.uv.y * 0.3333) ).x;
 float U = tex2D( VideoSampler, float2(0.5 + pnt.uv.x * 0.5, 1 - pnt.uv.y * 0.3333) ).x;

 float R = 1.164 * ( Y - 0.0625 ) + 1.596 * ( V - 0.5 );
 float G = 1.164 * ( Y - 0.0625 ) - 0.391 * ( U - 0.5 ) - 0.813 * ( V - 0.5 );
 float B = 1.164 * ( Y - 0.0625 ) + 2.018 * ( U - 0.5 );

 float4 ret = float4(R,G,B, 1);

 return ret;
}






Technique(VideoSprite, VideoVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = false;
   AlphaBlendEnable = false;
   ZEnable = FALSE;

   VertexShader = compile vs_2_0 VideoVertexShader();
   PixelShader = compile ps_2_0 VideoPixelShader();
  }
}
