#include "stdVariables.h"



struct PolygonVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
};



struct Poly_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
};

struct Poly_PS_INPUT
{
 float4 color : COLOR0;
};



Poly_VS_OUTPUT vs_stdPolygon( PolygonVertex vrx)
{
 Poly_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
 res.color = vrx.color;
 return res;
}


float4 ps_stdPolygon(Poly_PS_INPUT pnt) : COLOR
{
 return pnt.color;
}









Technique(stdPolygon, PolygonVertex)
{
 pass P0
  {
    AlphaBlendEnable = true;
    VertexShader = compile vs_2_0 vs_stdPolygon();
    PixelShader = compile ps_2_0 ps_stdPolygon();
  }
}

Technique(stdPolygonNoCull, PolygonVertex)
{
 pass P0
  {
    AlphaBlendEnable = true;
    CullMode = none;
    VertexShader = compile vs_2_0 vs_stdPolygon();
    PixelShader = compile ps_2_0 ps_stdPolygon();
  }
}
