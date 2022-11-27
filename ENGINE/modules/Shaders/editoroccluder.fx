#include "technique.h"
struct OccluderRecVertexIN
{
 float3 pos : POSITION;
 float4 color : COLOR;
};

struct OccluderRecVertexOUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
};

struct OccluderRecVertexPSIN
{
 float4 color : COLOR0;
};



OccluderRecVertexOUT OccluderRect_vs( OccluderRecVertexIN vrx)
{
 OccluderRecVertexOUT res;
 res.pos = float4(vrx.pos, 1);
 res.color = vrx.color;
 return res;
}


float4 OccluderRect_ps(OccluderRecVertexPSIN pnt) : COLOR
{
 return pnt.color;
}

Technique(OccluderRec, OccluderRecVertexIN)
{
 pass P0
  {
   cullmode = NONE;
   AlphaTestEnable = false;
   AlphaBlendEnable = false;
   
   ColorWriteEnable = 0;
   ZWriteEnable = false;

   VertexShader = compile vs_2_0 OccluderRect_vs();
   PixelShader = compile ps_2_0 OccluderRect_ps();
  }
}
