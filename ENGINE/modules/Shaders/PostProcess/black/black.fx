//#include "technique.h"
//#include "postprocess\common.h"
//
//float BlackScale;
//
//float4 PostProcessShowBW_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
////	float4 src = tex2D(PP_CommonTexture_Wrap,p.uv);
//	float4 src = tex2D(PP_CommonTexture_Prec,p.uv);
//
////	float c = dot(src.xyz,float3(0.2125,0.7154,0.0721));
//
////	float4 res = lerp(src,c*float4(0.83,0.88,1.00,0.0),BlackScale);
//
//	float r = length(src.xyz)*0.57735;
//
//	float4 res = lerp(src,float4(r,r,r,0.0),BlackScale);
//
//	return res;
//}
///*
//float4 PostProcessShowBW_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 src = tex2D(PP_CommonTexture_Prec,p.uv);
//
//	const float dx = 1.0/1280;
//	const float dy = 1.0/1024;
//
//	float4 t1 = tex2D(PP_CommonTexture_Prec,p.uv - float2(-dx,-dy));
//	float4 t2 = tex2D(PP_CommonTexture_Prec,p.uv - float2( dx,-dy));
//	float4 t3 = tex2D(PP_CommonTexture_Prec,p.uv - float2( dx, dy));
//	float4 t4 = tex2D(PP_CommonTexture_Prec,p.uv - float2(-dx, dy));
//
//	float4 dif = src - (t1 + t2 + t3 + t4)/4.0;
//
////	средняя конт
////	return sqrt(1.0 - length(dif.xyz)*0.57735);
//
////	сильная конт
//	float d = 1.0 - length(dif.xyz)*0.57735;
////	return (d*d*0.8 + 0.2)*src;
//	return  d*d*src;
//
////	для скетча
////	return 1.0 - sqrt(length(dif.xyz)*0.57735);
//
////	слабая  конт
////	float d = length(dif.xyz)*0.57735;
////	return (1.0 - d*d)*src;
//}
//*/
//Technique(PostProcessShowBW,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 PostProcessShowBW_PS();
//	}
//}
//
//////
//
//struct PostPolygon_VSIN
//{
//	float2 pos : POSITION;
//	float4 col : COLOR0;
//};
//
//struct PostPolygon_VSOUT
//{
//	float4 pos: POSITION;
//	float4 col: COLOR0;
//};
//
//struct PostPolygon_PSIN
//{
//	float4 col : COLOR0;
//};
//
//PostPolygon_VSOUT PostPolygon_VS(PostPolygon_VSIN p)
//{
//	PostPolygon_VSOUT r;
//
//	float4 pos = float4(p.pos,0.0,1.0);
//
//	pos.x = pos.x*2.0 - 1.0;
//	pos.y = 1.0 - pos.y*2.0;
//
//	r.pos = pos;
//	r.col = p.col;
//
//	return r;
//}
//
//float4 PostPolygon_PS(PostPolygon_PSIN p) : COLOR
//{
//	return p.col;
//}
//
//Technique(PostPolygon,PostPolygon_VSIN)
//{
//	pass P0
//	{
//		AlphaBlendEnable = true;
//
//		CullMode = None;
//
//		ZEnable = false;
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 PostPolygon_VS();
//		PixelShader = compile ps_2_0 PostPolygon_PS();
//	}
//}
