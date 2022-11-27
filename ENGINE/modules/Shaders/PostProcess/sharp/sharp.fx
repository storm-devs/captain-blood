//#include "technique.h"
//#include "postprocess\common.h"
//
//float SharpScale;
//
//float SharpDX;
//float SharpDY;
//
//float4 PPSharp_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex1 = tex2D(PP_CommonTexture_Wrap,InTex);
//	float4 tex2 = tex2D(PP_CommonTexture_Wrap,InTex + float2( SharpDX, SharpDY));
//	float4 tex3 = tex2D(PP_CommonTexture_Wrap,InTex + float2(-SharpDX, SharpDY));
//	float4 tex4 = tex2D(PP_CommonTexture_Wrap,InTex + float2( SharpDX,-SharpDY));
//	float4 tex5 = tex2D(PP_CommonTexture_Wrap,InTex + float2(-SharpDX,-SharpDY));
//
///*	float4 blur = 4.0*tex1 - (tex2 + tex3 + tex4 + tex5);
//
////	blur = tex1 + blur*SharpScale;
//	blur = tex1 + blur;
//
//	return blur;*/
//
//	float4 blur = (tex1 + tex2 + tex3 + tex4 + tex5)*0.2;
//
//	float4 res = tex1 - blur;
//
//	res = sqrt(abs(res))*sign(res);
//
////	return res*0.5 + 0.5;
//	return tex1 + res*3.0*SharpScale*0.1;
//}
//
//Technique(PPSharp,PP_FullScreenQuad_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//		AlphaBlendEnable = false;
//		AlphaTestEnable = false;
//
//		ZEnable = false;
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 PP_StdVertexShader();
//		PixelShader = compile ps_2_0 PPSharp_PS();
//	}
//}
