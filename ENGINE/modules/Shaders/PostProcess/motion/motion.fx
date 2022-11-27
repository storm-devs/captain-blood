//#include "technique.h"
//#include "postprocess\common.h"
//#include "stdVariables.h"
//
//#include "postprocess\glow\glow.fx"
//
//texture MotionAccuBuffer;
//
//sampler MotionAccuBuffer_Wrap =
//sampler_state
//{
//    Texture = <MotionAccuBuffer>;
//    MinFilter = Linear;
//    MagFilter = Linear;
//////AddressU = Wrap;
//////AddressV = Wrap;
//	AddressU = Clamp;
//	AddressV = Clamp;
//};
//
////float BlurOriginalK;
////float AfterImageK;
//
////int MotionBlendType;
//
//float MotionBlendFactor;
//
//float4 MotionBlur_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 src = tex2D(PP_ScreenTexture_Wrap,p.uv);
//	float4 acc = tex2D(MotionAccuBuffer_Wrap,p.uv);
//
////	float q = AfterImageK;
////	float c = BlurOriginalK;
//
////	if( MotionBlendType < 1 )
//	//	return lerp(src,acc,0.9);
//		return lerp(src,acc,MotionBlendFactor);
////	else
////		return acc*q + src*c - 1.0/255/c;
//}
//
//Technique(MotionBlurShow,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 MotionBlur_PS();
//	}
//}
//
//float4 AfterBlur_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 src = tex2D(PP_ScreenTexture_Wrap,p.uv);
//	float4 acc = tex2D(MotionAccuBuffer_Wrap,p.uv);
///*
////	float q = AfterImageK;
////	float c = BlurOriginalK;
//
////	if( MotionBlendType < 1 )
//	//	return lerp(src,acc,0.9);
//		return lerp(src,acc,MotionBlendFactor);
////	else
////		return acc*q + src*c - 1.0/255/c;*/
//
//	float4 res = lerp(src,acc,MotionBlendFactor);
//
//	res.a = length(src.xyz - acc.xyz)*0.57735;
//
//	return res;
//}
//
//Technique(AfterImageShow,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 AfterBlur_PS();
//	}
//}
//
//float4 AfterMix_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 src = tex2D(PP_ScreenTexture_Wrap,p.uv);
//	float4 acc = tex2D(MotionAccuBuffer_Wrap,p.uv);
//
//	float4 res = lerp(src,acc,MotionBlendFactor*sqrt(acc.a));
//
//	return res;
////	return acc.a;
//}
//
//Technique(AfterImageMix,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 AfterMix_PS();
//	}
//}
///*
//texture MotionTexture;
//
//sampler MotionTexture_Wrap =
//sampler_state
//{
//    Texture = <MotionTexture>;
//    MinFilter = Linear;
//    MagFilter = Linear;
//    AddressU = Wrap;
//    AddressV = Wrap;
//};
//
//sampler MotionTexture_Prec =
//sampler_state
//{
//    Texture = <MotionTexture>;
//    MinFilter = Point;
//    MagFilter = Point;
//    AddressU = Wrap;
//    AddressV = Wrap;
//};
//*/
//float4 MotionCopy_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 acc = tex2D(MotionTexture_Wrap,p.uv);
//
////	;
//	acc.a = 0.0;
//
////	return acc.a;
//	return acc;
//}
//
//float4 MotionPaste_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 acc = tex2D(MotionTexture_Wrap,p.uv);
//
//	return acc;
//}
//
//Technique(MotionBlurCopy,PP_FullScreenQuad_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//		AlphaBlendEnable = false;
//		AlphaTestEnable = false;
//
//	//	ZWriteEnable = ;
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 PP_StdVertexShader();
//		PixelShader = compile ps_2_0 MotionCopy_PS();
//	}
//}
///*
//float4 AfterCopy_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 acc = tex2D(MotionTexture_Wrap,p.uv);
//
//	return acc;
//}
//
//Technique(AfterImageCopy,PP_FullScreenQuad_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//		AlphaBlendEnable = false;
//		AlphaTestEnable = false;
//
//	//	ZWriteEnable = ;
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 PP_StdVertexShader();
//		PixelShader = compile ps_2_0 AfterCopy_PS();
//	}
//}
//*/
//#ifdef _XBOX
//
//float4 MotionCopy_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 col = tex2D(PP_CommonTexture_Wrap,p.uv);
//
///*	float z = tex2D(MotionTexture_Wrap,p.uv).r;
//
//	float m32 = mProjection[3][2];
//	float m22 = mProjection[2][2];
//
//	float k = m32*(1.0 + m22/(z - m22));
//
//	float a = 1.0/(FocusDist);
//	float b = 1.0/(FocusDist*6);
//
//	k = k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b;*/
//
///*	float z = tex2D(MotionTexture_Wrap,p.uv).r;
//
//	const float m32 = mProjection[3][2];
//	const float m22 = mProjection[2][2];
//
//	float k = m32*(1.0 + m22/(z - m22));
//
//	float a = ForeScale/(FocusDist);
//	float b = BackScale/(FocusDist*5);
//
//	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);*/
//
////	float z = tex2D(MotionTexture_Wrap,p.uv).r;
//	float z = tex2D(MotionTexture_Prec,p.uv).r;
//
//	const float m32 = mProjection[3][2];
//	const float m22 = mProjection[2][2];
//
//	z = m32*(1.0 + m22/(z - m22));
//
//	float k = 1.0/z;
//
////	float m = z < FocusDist ? ForeScale*0.4*(FocusDist - z)*k : BackScale*(z - FocusDist)*k;
//	float m = z < FocusDist ? ForeScale*(1.0 - z/FocusDist) : BackScale*(z - FocusDist)*k;
//
//	col.a = m*BlurFactor;
////	col.rgb = m*BlurFactor;
////	col.a = 1.0;
//
//	return col;
//
///*	float4 col = tex2D(PP_CommonTexture_Wrap,p.uv);
//	float4 src = tex2D(   MotionTexture_Prec,p.uv);
//
//	return lerp(src,col,col.a);*/
//
////	float4 col = tex2D(PP_CommonTexture_Wrap,p.uv);
//
////	return col;
//}
//
//Technique(MotionBlurCopy_BOX,PP_FullScreenQuad_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//		AlphaBlendEnable = true;
//	//	AlphaBlendEnable = false;
//
//		AlphaTestEnable = false;
//
//		SrcBlend = srcalpha;
//		DestBlend = invsrcalpha;
//
//		ZWriteEnable = false;
//		ZEnable = false;
//
//		VertexShader = compile vs_2_0 PP_StdVertexShader();
//		PixelShader = compile ps_2_0 MotionCopy_BOX_PS();
//	}
//}
//
//#endif
//
//struct MotionCopy_VSIN
//{
//	float4 pos : POSITION;
//	float2 uv  : TEXCOORD0;
//	float  alp : TEXCOORD1;
//};
//
//struct MotionCopy_VSOUT
//{
//	float4 p   : POSITION;
//	float2 uv  : TEXCOORD0;
//	float  alp : TEXCOORD1;
//};
//
//MotionCopy_VSOUT MotionCopy_Z_VS(MotionCopy_VSIN v)
//{
//	MotionCopy_VSOUT res;
//
//	float4 p = mul(v.pos,mProjection);
//
//	p.z = p.z/p.w;
//	p.w = 1.0;
//
//	p.xy = v.pos.xy;
//
//	res.p = p;
//
//	res.uv	= v.uv;
//	res.alp = v.alp;
//
//	return res;
//}
///*
//float4 MotionCopy_Z_PS(MotionCopy_VSIN p) : COLOR
//{
//	float4 acc = tex2D(MotionTexture_Wrap,p.uv);
//
////	acc.a *= p.alp;
//	acc.a  = p.alp;
//
//	return acc;
//}
//*/
//float4 MotionCopy_Z_PS(MotionCopy_VSIN p) : COLOR
//{
//	float4 src = tex2D(PP_CommonTexture_Prec,p.uv);
////	float4 acc = tex2D(   MotionTexture_Wrap,p.uv);
///*
////	acc.a *= p.alp;
//	acc.a  = p.alp;
//
//	return acc;
//*/
////	return lerp(src,acc,p.alp);
//
//	src.a = p.alp;
////	src	  = p.alp;
//
//	return src;
//}
//
//struct MotionDir_VSIN
//{
//	float4 pos : POSITION;
//	float2 uv  : TEXCOORD0;
//	float  alp : TEXCOORD1;
//};
//
//struct MotionDir_VSOUT
//{
//	float4 p   : POSITION;
//	float2 uv  : TEXCOORD0;
//	float  alp : TEXCOORD1;
//	float4 ppp : TEXCOORD2;
//};
//
//struct MotionDir_PSIN
//{
//	float2 uv  : TEXCOORD0;
//	float  alp : TEXCOORD1;
//	float4 ppp : TEXCOORD2;
//};
//
//
//MotionDir_VSOUT MotionCopy_Z_Dir_VS(MotionDir_VSIN v)
//{
//	MotionDir_VSOUT res;
//
//	float4 p = mul(v.pos,mProjection);
//
//	res.ppp = v.pos*0.005;
//
//	p.z = p.z/p.w;
//	p.w = 1.0;
//
//	p.xy = v.pos.xy;
//
//	res.p	= p;
//
//	res.uv	= v.uv;
//	res.alp = v.alp;
//
//	return res;
//}
//
//float4 MotionCopy_Z_Dir_PS(MotionDir_PSIN p) : COLOR
//{
//	float4 src = tex2D(PP_CommonTexture_Prec,p.uv);
//
//	src.a = p.ppp.z;
//
//	return src;
//}
//
//Technique(MotionBlurCopy_Z,MotionCopy_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//	//	AlphaBlendEnable = true;
//		AlphaBlendEnable = false;
//
//		AlphaTestEnable = false;
//
//	//	SrcBlend = srcalpha;
//	//	DestBlend = invsrcalpha;
//
//		ZEnable = true;
//		ZFunc = Less;
//
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 MotionCopy_Z_VS();
//		PixelShader = compile ps_2_0 MotionCopy_Z_PS();
//	}
//}
//
//Technique(MotionBlurCopy_Z_Inv,MotionCopy_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//	//	AlphaBlendEnable = true;
//		AlphaBlendEnable = false;
//
//		AlphaTestEnable = false;
//
//	//	SrcBlend = srcalpha;
//	//	DestBlend = invsrcalpha;
//
//		ZEnable = true;
//		ZFunc = Greater;
//
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 MotionCopy_Z_VS();
//		PixelShader = compile ps_2_0 MotionCopy_Z_PS();
//	}
//}
//
//struct DOFCopy_VSIN
//{
//	float4 pos : POSITION;
//	float2 uv  : TEXCOORD0;
//};
//
//struct DOFCopy_VSOUT
//{
//	float4 p  : POSITION;
//	float2 uv : TEXCOORD0;
//	float  d  : TEXCOORD1;
////	float3 d  : TEXCOORD1;
//};
//
//DOFCopy_VSOUT DOFCopy_Z_VS(DOFCopy_VSIN v)
//{
//	DOFCopy_VSOUT res;
//
//	float4 p = mul(v.pos,mProjection);
//
//	p.z = p.z/p.w;
//	p.w = 1.0;
//
//	p.xy = v.pos.xy;
//
//	res.p = p;
//
//	res.uv = v.uv;
//
//	float dz = FocusDist*0.2;
//
////	res.d = v.pos.z < FocusDist ? v.pos.z/FocusDist*0.5 : 0.5 + (v.pos.z - FocusDist)/(FocusDist*5.0)*0.5;
///*	res.d = v.pos.z < FocusDist ?
//		(1.0 - (v.pos.z - dz)/(FocusDist - dz))*ForeScale :
//		(v.pos.z - FocusDist)/(FocusDist*6.0)*BackScale;*/
//	float k = 1.0/v.pos.z;
//	if( v.pos.z < FocusDist )
//	{
//	//	res.d = 1.0 - (v.pos.z - dz)/(FocusDist - dz);
//		res.d = (FocusDist - v.pos.z)*k;
//
//	//	res.d *= 0.5;
//		res.d *= 0.4;
//
//		res.d *= ForeScale;
//	}
//	else
//	{
//	//	res.d = (v.pos.z - FocusDist)/(FocusDist*6.0)*BackScale;
//		res.d = (v.pos.z - FocusDist)*k;
//		
//		res.d *= BackScale;
//	}
////	res.d = v.pos;
//
//	return res;
//}
//
//float4 DOFCopy_Z_PS(DOFCopy_VSOUT p) : COLOR
//{
//	float4 src = tex2D(PP_CommonTexture_Prec,p.uv);
//
///*	float dz = FocusDist*0.2;
//
////	float d = p.d.z;
//	float d = length(p.d);
//
////	src.a = p.d;
//	float k = d < FocusDist ?
//		(1.0 - (d - dz)/(FocusDist - dz))*ForeScale :
//		(d - FocusDist)/(FocusDist*4.0)*BackScale;
//
////	k = 1.0 - k;
////	k = k*k*k*k;
////	k = 1.0 - k;
//
////	src.a = k*BlurFactor;
//	src.a = k;*/
//	src.a = p.d;
//
//	return src;
////	return src.a;
//}
//
//Technique(DOFCopy_Z,DOFCopy_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//	//	AlphaBlendEnable = true;
//		AlphaBlendEnable = false;
//
//		AlphaTestEnable = false;
//
//	//	SrcBlend = srcalpha;
//	//	DestBlend = invsrcalpha;
//
//		ZEnable = true;
//		ZFunc = Less;
//
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 DOFCopy_Z_VS();
//		PixelShader = compile ps_2_0 DOFCopy_Z_PS();
//	}
//}
//
//Technique(DOFCopy_Z_Inv,MotionCopy_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//	//	AlphaBlendEnable = true;
//		AlphaBlendEnable = false;
//
//		AlphaTestEnable = false;
//
//	//	SrcBlend = srcalpha;
//	//	DestBlend = invsrcalpha;
//
//		ZEnable = true;
//		ZFunc = Greater;
//
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 DOFCopy_Z_VS();
//		PixelShader = compile ps_2_0 DOFCopy_Z_PS();
//	}
//}
//
//Technique(MotionBlurCopy_Z_Dir,MotionCopy_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//	//	AlphaBlendEnable = true;
//		AlphaBlendEnable = false;
//
//		AlphaTestEnable = false;
//
//	//	SrcBlend = srcalpha;
//	//	DestBlend = invsrcalpha;
//
//		ZEnable = true;
//		ZFunc = Less;
//
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 MotionCopy_Z_Dir_VS();
//		PixelShader = compile ps_2_0 MotionCopy_Z_Dir_PS();
//	}
//}
//
////// Final blur paste ////
//
//Technique(MotionBlurPaste,MotionCopy_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//		AlphaBlendEnable = true;
//	//	AlphaBlendEnable = false;
//
//		AlphaTestEnable = false;
//
//	//	SrcBlend = destalpha;
//	//	DestBlend = invdestalpha;
//		SrcBlend = srcalpha;
//		DestBlend = invsrcalpha;
//
//		ZEnable = false;
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 PP_StdVertexShader();
//		PixelShader = compile ps_2_0 MotionPaste_PS();
//	}
//}
//
////////////////////////////
//
//float4 MotionEffectPaste_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
////	float4 acc = tex2D(	  MotionTexture_Prec,p.uv);
////	float4 acc;
//	float4 dir = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	dir.a = dir.a*dir.a;
//
///*	float z = acc.a*200.0;
//
//	float4 et = mul(float3(p.pos.xy,z),mProjection);
//
//	float4 pos = float4(p.pos.xy,z,1.0);
//
//	pos.xy *= et.w;
//
//	float4 r = mul(pos,MotionMatrix); r.w = 1.0;
//
//	et = mul(float4(r.xyz,1.0),mProjection);
//
//	r.xy /= et.w;
//
//	float2 d = float2(p.pos.xy - r.xy);*/
//
////	acc = lerp(acc,dir,1.0 - 1.0/(1.0 + length(d)*MotionFactor));
////	acc = lerp(acc,dir,dir.a);
//
////	acc = dir;
////	acc = lerp(acc,dir,dir.a);
//
////	return acc;
//	return dir;
//}
//
//Technique(MotionEffectPaste,PP_FullScreenQuad_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//	//	AlphaBlendEnable = false;
//		AlphaBlendEnable = true;
//
//		AlphaTestEnable = false;
//
//		SrcBlend = srcalpha;
//		DestBlend = invsrcalpha;
//
//		ZEnable = false;
//		ZWriteEnable = false;
//
//		VertexShader = compile vs_2_0 PP_StdVertexShader();
//		PixelShader = compile ps_2_0 MotionEffectPaste_PS();
//	}
//}
