#include "technique.h"
#ifndef CameraFrustum
#define CameraFrustum

#include "stdVariables.h"

struct Frustum_Vertex
{
	float3 pos : POSITION;	
};

struct FRST_VS_OUTPUT
{
    float4 pos : POSITION;    
};
	
FRST_VS_OUTPUT vs_ShowCameraFrustum(Frustum_Vertex In)
{
 FRST_VS_OUTPUT res;    
    
 res.pos = mul( float4(In.pos, 1.0), mWorldViewProj);   
    
 return res;
}

float4 ps_ShowCameraFrustum_Blend(FRST_VS_OUTPUT In) : COLOR
{ 	    
	return float4(0.15, 1.0, 0.15, 0.35);
}

float4 ps_ShowCameraFrustum_Lines(FRST_VS_OUTPUT In) : COLOR
{ 	    
	return float4(1.0f,1.0, 1.0, 1.0);
}

Technique(ShowCameraFrustum, Frustum_Vertex)
{	
/* multipass no longer supported!
  pass P0
  {	    
		zenable = true;
		zwriteenable = false;
		CullMode = none;

		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		srcblend = one;
		destblend = one;

		fillmode = wireframe;

		VertexShader = compile vs_2_0 vs_ShowCameraFrustum();
		PixelShader = compile ps_2_0 ps_ShowCameraFrustum_Lines();
  }
*/  
	
  pass P0	
  {	    
		zenable = true;
		zwriteenable = false;
		CullMode = none;

		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		srcblend = srcalpha;
		destblend = invsrcalpha;

		fillmode = solid;
		AlphaBlendEnable = true;

		VertexShader = compile vs_2_0 vs_ShowCameraFrustum();
		PixelShader = compile ps_2_0 ps_ShowCameraFrustum_Blend();
  }		
}

#endif

