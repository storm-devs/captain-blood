#include "technique.h"
#include "gmxService\gmx_UberShader.h"
#include "gmxService\gmx_UberGenerator.h"






struct fb_vsout
{
 float4 pos    : POSITION;
 float3 uv_pos : TEXCOORD0;
 float2 uv     : TEXCOORD1;  
};

struct fb_psin
{
 float3 uv_pos : TEXCOORD0;
 float2 uv     : TEXCOORD1;  
};


fb_vsout vs_FoggedBackground(GMX_VS_IN_STATIC v)
{
 fb_vsout o;
 
 float3 vPos = v.vPos.xyz/* + float3(vCamPos.x, 0.0, vCamPos.z)*/;
//float3 vPos = v.vPos;
 
 o.pos = mul(float4(vPos, 1.0), mWorldViewProj);
 o.uv = UnpackUV(v.vPackedUV1);
 o.uv_pos = vPos;
 

 return o;
}



float4 ps_FoggedBackground(fb_psin p) : COLOR
{
// float4 texel = tex2D(SpecularMap, p.uv);
 float4 texel = tex2D(DiffuseMap, p.uv);
 
 
 float3 delta = vCamPos - p.uv_pos;
 float dist = sqrt(dot(delta, delta)) * 0.00025;

 float heightFogK = 1.0 - (clamp(p.uv_pos.y, 0.0, 90.0) * 0.0111111);
 heightFogK = heightFogK * 0.15;


 float fogK = saturate(dist + heightFogK);
 
 fogK = fogK * GMX_ObjectsUserColor.a;


 float4 res = lerp (texel, float4(GMX_ObjectsUserColor.rgb, 1.0), fogK);
// float4 res = texel;
 
 
 //1 - near, 0 - far
 float alpha = (1.0 - saturate(fogK * fogK * fogK * fogK));
 res.a = alpha;


   
 
 
 return res;
// return float4(fogK, fogK, fogK, 1.0);
}




Technique(gmx_FoggedBackground, GMX_VS_IN_STATIC)
{
 pass P0
 {
  AlphaBlendEnable = true;
  AlphaTestEnable = false;
  VertexShader = compile vs_2_0 vs_FoggedBackground();
  PixelShader = compile ps_2_0 ps_FoggedBackground();
 }
}



