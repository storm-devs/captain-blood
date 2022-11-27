#ifndef STDFONT
#define STDFONT

#include "technique.h"

float4 FontColor;
texture FontTexture;
sampler FontTexSampler =
sampler_state
{
    Texture = <FontTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
};



struct FontVertex
{
 float3 pos : POSITION;
 float2 uv : TEXCOORD0;
};



struct Font_VS_OUTPUT
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct Font_PS_INPUT
{
    float2 uv : TEXCOORD0;
};



Font_VS_OUTPUT FontVertexShader( FontVertex vrx)
{
 Font_VS_OUTPUT res;
 res.pos = float4(vrx.pos, 1);
 res.uv = vrx.uv;
 return res;
}


float4 FontPixelShader(Font_PS_INPUT pnt) : COLOR
{
	float4 tex = tex2D(FontTexSampler, pnt.uv);
	return float4(tex.rgb * FontColor.rgb, tex.a);
}

float4 FontPixelShaderAlpha(Font_PS_INPUT pnt) : COLOR
{	
	return  tex2D(FontTexSampler, pnt.uv) * FontColor; 
}








Technique(dbgFont, FontVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = TRUE;
   AlphaBlendEnable = true;
   ZEnable = FALSE;
   
   AlphaRef = 1;

   VertexShader = compile vs_2_0 FontVertexShader();
   PixelShader = compile ps_2_0 FontPixelShader();
  }
}

Technique(dbgFontAlpha, FontVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = TRUE;
   AlphaBlendEnable = true;
   ZEnable = FALSE;
   
   AlphaRef = 1;

   VertexShader = compile vs_2_0 FontVertexShader();
   PixelShader = compile ps_2_0 FontPixelShaderAlpha();
  }
}






#include "stdVariables.h"



float4 ls_FontPixelShaderAlpha(Font_PS_INPUT pnt) : COLOR
{	
	return GammaCorrection(tex2D(FontTexSampler, pnt.uv) * FontColor); 
}


Technique(ls_dbgFontAlpha, FontVertex)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = TRUE;
   AlphaBlendEnable = true;
   ZEnable = FALSE;
   
   AlphaRef = 1;

   VertexShader = compile vs_2_0 FontVertexShader();
   PixelShader = compile ps_2_0 ls_FontPixelShaderAlpha();
  }
}


#endif