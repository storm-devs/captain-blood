#include "stdVariables.h"


float4 SphereColor;


struct SphereVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
};



struct Sphere_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
};

struct Sphere_PS_INPUT
{
 float4 color : COLOR0;
};



Sphere_VS_OUTPUT vs_Sphere( SphereVertex vrx)
{
 Sphere_VS_OUTPUT res;
 res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
 res.color = vrx.color;
 return res;
}


float4 ps_Sphere(Sphere_PS_INPUT pnt) : COLOR
{
 return (SphereColor * pnt.color);
}









Technique(dbgSphere, SphereVertex)
{
 pass P0
  {
    AlphaBlendEnable = true;
    ZWriteEnable = false;
    VertexShader = compile vs_2_0 vs_Sphere();
    PixelShader = compile ps_2_0 ps_Sphere();
  }
}


