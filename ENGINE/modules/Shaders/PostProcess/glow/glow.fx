//#include "technique.h"
//#include "postprocess\common.h"
//#include "stdVariables.h"
//
//float TexelSizeX;
//float TexelSizeY;
//
//float GlowBlurPass;
//float GlowThreshold;
//
//float GlowPower;
//
//float BlurDivider;
//
//float BlurFactor;
//
//////
//
//float4 Blur8;
//float4 Blur9;
//float4 BlurA;
//float4 BlurB;
//float4 BlurC;
//float4 BlurD;
//
//////
//
//float GlowBack;
//float GlowFore;
///*
//float4 BlurWithClampMix_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex1 = tex2D(PP_CommonTexture_Wrap,InTex);
//	float4 tex2 = tex2D(PP_CommonTexture_Wrap,InTex + (float2( TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 tex3 = tex2D(PP_CommonTexture_Wrap,InTex + (float2(-TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 tex4 = tex2D(PP_CommonTexture_Wrap,InTex + (float2( TexelSizeX,-TexelSizeY))*GlowBlurPass);
//	float4 tex5 = tex2D(PP_CommonTexture_Wrap,InTex + (float2(-TexelSizeX,-TexelSizeY))*GlowBlurPass);
//
//	float4 te_1 = tex2D(PP_ScreenTexture_Wrap,InTex);
//	float4 te_2 = tex2D(PP_ScreenTexture_Wrap,InTex + (float2( TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 te_3 = tex2D(PP_ScreenTexture_Wrap,InTex + (float2(-TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 te_4 = tex2D(PP_ScreenTexture_Wrap,InTex + (float2( TexelSizeX,-TexelSizeY))*GlowBlurPass);
//	float4 te_5 = tex2D(PP_ScreenTexture_Wrap,InTex + (float2(-TexelSizeX,-TexelSizeY))*GlowBlurPass);
//
//	tex1.rgb = tex1.rgb*GlowBack + te_1.rgb*GlowFore;
//	tex2.rgb = tex2.rgb*GlowBack + te_2.rgb*GlowFore;
//	tex3.rgb = tex3.rgb*GlowBack + te_3.rgb*GlowFore;
//	tex4.rgb = tex4.rgb*GlowBack + te_4.rgb*GlowFore;
//	tex5.rgb = tex5.rgb*GlowBack + te_5.rgb*GlowFore;
//
//	tex2 = lerp(tex1,tex2,tex1.a*tex2.a + (1.0 - tex1.a));
//	tex3 = lerp(tex1,tex3,tex1.a*tex3.a + (1.0 - tex1.a));
//	tex4 = lerp(tex1,tex4,tex1.a*tex4.a + (1.0 - tex1.a));
//	tex5 = lerp(tex1,tex5,tex1.a*tex5.a + (1.0 - tex1.a));
//
//	float4 summ = (tex1 + tex2 + tex3 + tex4 + tex5)*0.2; // divide by 5
//
//	float max_component = max(max(summ.r, summ.g), summ.b);
//
//	if( max_component < GlowThreshold )
//	{
//		summ = 0.0;
//	}
//
//	return summ;
//}
//*//*
//float4 BlurWithClampMix_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//	float4 gex = tex2D(PP_ScreenTexture_Wrap,p.uv);
//
//	tex.rgb = tex.rgb*GlowBack + gex.rgb*GlowFore;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*GlowBlurPass;
//
//	float4 t00 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-7.5,0.0));
//	float4 t01 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-6.5,0.0));
//	float4 t02 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-5.5,0.0));
//	float4 t03 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-4.5,0.0));
//
//	float4 t04 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-3.5,0.0));
//	float4 t05 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-2.5,0.0));
//	float4 t06 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-1.5,0.0));
//	float4 t07 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-0.5,0.0));
//
//	float4 t08 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 0.5,0.0));
//	float4 t09 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 1.5,0.0));
//	float4 t10 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 2.5,0.0));
//	float4 t11 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 3.5,0.0));
//
//	float4 t12 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 4.5,0.0));
//	float4 t13 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 5.5,0.0));
//	float4 t14 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 6.5,0.0));
//	float4 t15 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 7.5,0.0));
//
//	float4 g00 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2(-7.5,0.0));
//	float4 g01 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2(-6.5,0.0));
//	float4 g02 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2(-5.5,0.0));
//	float4 g03 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2(-4.5,0.0));
//
//	float4 g04 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2(-3.5,0.0));
//	float4 g05 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2(-2.5,0.0));
//	float4 g06 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2(-1.5,0.0));
//	float4 g07 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2(-0.5,0.0));
//
//	float4 g08 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2( 0.5,0.0));
//	float4 g09 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2( 1.5,0.0));
//	float4 g10 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2( 2.5,0.0));
//	float4 g11 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2( 3.5,0.0));
//
//	float4 g12 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2( 4.5,0.0));
//	float4 g13 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2( 5.5,0.0));
//	float4 g14 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2( 6.5,0.0));
//	float4 g15 = tex2D(PP_ScreenTexture_Wrap,p.uv + d*float2( 7.5,0.0));
//
//	t00.rgb = t00.rgb*GlowBack + g00.rgb*GlowFore;
//	t01.rgb = t01.rgb*GlowBack + g01.rgb*GlowFore;
//	t02.rgb = t02.rgb*GlowBack + g02.rgb*GlowFore;
//	t03.rgb = t03.rgb*GlowBack + g03.rgb*GlowFore;
//
//	t04.rgb = t04.rgb*GlowBack + g04.rgb*GlowFore;
//	t05.rgb = t05.rgb*GlowBack + g05.rgb*GlowFore;
//	t06.rgb = t06.rgb*GlowBack + g06.rgb*GlowFore;
//	t07.rgb = t07.rgb*GlowBack + g07.rgb*GlowFore;
//
//	t08.rgb = t08.rgb*GlowBack + g08.rgb*GlowFore;
//	t09.rgb = t09.rgb*GlowBack + g09.rgb*GlowFore;
//	t10.rgb = t10.rgb*GlowBack + g10.rgb*GlowFore;
//	t11.rgb = t11.rgb*GlowBack + g11.rgb*GlowFore;
//
//	t12.rgb = t12.rgb*GlowBack + g12.rgb*GlowFore;
//	t13.rgb = t13.rgb*GlowBack + g13.rgb*GlowFore;
//	t14.rgb = t14.rgb*GlowBack + g14.rgb*GlowFore;
//	t15.rgb = t15.rgb*GlowBack + g15.rgb*GlowFore;
//
//	t00 = lerp(tex,t00,tex.a*t00.a + (1.0 - tex.a));
//	t01 = lerp(tex,t01,tex.a*t01.a + (1.0 - tex.a));
//	t02 = lerp(tex,t02,tex.a*t02.a + (1.0 - tex.a));
//	t03 = lerp(tex,t03,tex.a*t03.a + (1.0 - tex.a));
//
//	t04 = lerp(tex,t04,tex.a*t04.a + (1.0 - tex.a));
//	t05 = lerp(tex,t05,tex.a*t05.a + (1.0 - tex.a));
//	t06 = lerp(tex,t06,tex.a*t06.a + (1.0 - tex.a));
//	t07 = lerp(tex,t07,tex.a*t07.a + (1.0 - tex.a));
//
//	t08 = lerp(tex,t08,tex.a*t08.a + (1.0 - tex.a));
//	t09 = lerp(tex,t09,tex.a*t09.a + (1.0 - tex.a));
//	t10 = lerp(tex,t10,tex.a*t10.a + (1.0 - tex.a));
//	t11 = lerp(tex,t11,tex.a*t11.a + (1.0 - tex.a));
//
//	t12 = lerp(tex,t12,tex.a*t12.a + (1.0 - tex.a));
//	t13 = lerp(tex,t13,tex.a*t13.a + (1.0 - tex.a));
//	t14 = lerp(tex,t14,tex.a*t14.a + (1.0 - tex.a));
//	t15 = lerp(tex,t15,tex.a*t15.a + (1.0 - tex.a));
//
//	float4 blur = 0.0;
//
//	blur +=	t00*BlurC.r;
//	blur +=	t01*BlurC.g;
//	blur +=	t02*BlurC.b;
//	blur +=	t03*BlurC.a;
//
//	blur +=	t04*BlurD.r;
//	blur +=	t05*BlurD.g;
//	blur +=	t06*BlurD.b;
//	blur +=	t07*BlurD.a;
//
//	blur +=	t08*BlurD.a;
//	blur +=	t09*BlurD.b;
//	blur +=	t10*BlurD.g;
//	blur +=	t11*BlurD.r;
//
//	blur +=	t12*BlurC.a;
//	blur +=	t13*BlurC.b;
//	blur +=	t14*BlurC.g;
//	blur +=	t15*BlurC.r;
//
//	float max_component = max(max(blur.r,blur.g),blur.b);
//
//	if( max_component < GlowThreshold )
//	{
//		blur = 0.0;
//	}
//
//	return blur;
//}
//*/
//float4 BlurWithClampMix_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
///*	float tsx = TexelSizeX*0.25;
//	float tsy = TexelSizeY*0.25;
//
//	float2 tc0 = float2(-tsx,-tsy);
//	float2 tc1 = float2( tsx,-tsy);
//	float2 tc2 = float2(-tsx, tsy);
//	float2 tc3 = float2( tsx, tsy);
//
//	float4 tex;
//
//	tex  = tex2D(PP_CommonTexture_Wrap,p.uv + tc0);
//	tex += tex2D(PP_CommonTexture_Wrap,p.uv + tc1);
//	tex += tex2D(PP_CommonTexture_Wrap,p.uv + tc2);
//	tex += tex2D(PP_CommonTexture_Wrap,p.uv + tc3);
//
//	tex *= 0.25;
//
//	float4 te_;
//
//	te_  = tex2D(PP_ScreenTexture_Wrap,p.uv + tc0);
//	te_ += tex2D(PP_ScreenTexture_Wrap,p.uv + tc1);
//	te_ += tex2D(PP_ScreenTexture_Wrap,p.uv + tc2);
//	te_ += tex2D(PP_ScreenTexture_Wrap,p.uv + tc3);
//
//	te_ *= 0.25;*/
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//	float4 te_ = tex2D(PP_ScreenTexture_Wrap,p.uv);
//
//	tex.rgb = tex.rgb*GlowBack + te_.rgb*GlowFore;
//
//	float max_component = max(max(tex.r,tex.g),tex.b);
//
//	if( max_component < GlowThreshold )
//	{
//		tex.rgb = 0.0;
//	}
//
//	return tex;
//}
//
//////
///*
//float4 IterativeBlurWithClamp_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex1 = tex2D(PP_CommonTexture_Wrap,InTex);
//	float4 tex2 = tex2D(PP_CommonTexture_Wrap,InTex + (float2( TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 tex3 = tex2D(PP_CommonTexture_Wrap,InTex + (float2(-TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 tex4 = tex2D(PP_CommonTexture_Wrap,InTex + (float2( TexelSizeX,-TexelSizeY))*GlowBlurPass);
//	float4 tex5 = tex2D(PP_CommonTexture_Wrap,InTex + (float2(-TexelSizeX,-TexelSizeY))*GlowBlurPass);
//
//	tex2 = lerp(tex1,tex2,tex1.a*tex2.a + (1.0 - tex1.a));
//	tex3 = lerp(tex1,tex3,tex1.a*tex3.a + (1.0 - tex1.a));
//	tex4 = lerp(tex1,tex4,tex1.a*tex4.a + (1.0 - tex1.a));
//	tex5 = lerp(tex1,tex5,tex1.a*tex5.a + (1.0 - tex1.a));
//
//	float4 summ = (tex1 + tex2 + tex3 + tex4 + tex5)*0.2; // divide by 5
//
//	float max_component = max(max(summ.r, summ.g), summ.b);
//
//	if( max_component < GlowThreshold )
//	{
//		summ = 0.0;
//	}
//
//	return summ;
////	return 0.0f;
//}
//*/
//float4 IterativeBlurWithClampH_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*GlowBlurPass;
//
//	float4 t00 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-7.5,0.0));
//	float4 t01 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-6.5,0.0));
//	float4 t02 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-5.5,0.0));
//	float4 t03 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-4.5,0.0));
//
//	float4 t04 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-3.5,0.0));
//	float4 t05 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-2.5,0.0));
//	float4 t06 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-1.5,0.0));
//	float4 t07 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-0.5,0.0));
//
//	float4 t08 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 0.5,0.0));
//	float4 t09 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 1.5,0.0));
//	float4 t10 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 2.5,0.0));
//	float4 t11 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 3.5,0.0));
//
//	float4 t12 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 4.5,0.0));
//	float4 t13 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 5.5,0.0));
//	float4 t14 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 6.5,0.0));
//	float4 t15 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 7.5,0.0));
//
///*	t00 = lerp(tex,t00,tex.a*t00.a + (1.0 - tex.a));
//	t01 = lerp(tex,t01,tex.a*t01.a + (1.0 - tex.a));
//	t02 = lerp(tex,t02,tex.a*t02.a + (1.0 - tex.a));
//	t03 = lerp(tex,t03,tex.a*t03.a + (1.0 - tex.a));
//
//	t04 = lerp(tex,t04,tex.a*t04.a + (1.0 - tex.a));
//	t05 = lerp(tex,t05,tex.a*t05.a + (1.0 - tex.a));
//	t06 = lerp(tex,t06,tex.a*t06.a + (1.0 - tex.a));
//	t07 = lerp(tex,t07,tex.a*t07.a + (1.0 - tex.a));
//
//	t08 = lerp(tex,t08,tex.a*t08.a + (1.0 - tex.a));
//	t09 = lerp(tex,t09,tex.a*t09.a + (1.0 - tex.a));
//	t10 = lerp(tex,t10,tex.a*t10.a + (1.0 - tex.a));
//	t11 = lerp(tex,t11,tex.a*t11.a + (1.0 - tex.a));
//
//	t12 = lerp(tex,t12,tex.a*t12.a + (1.0 - tex.a));
//	t13 = lerp(tex,t13,tex.a*t13.a + (1.0 - tex.a));
//	t14 = lerp(tex,t14,tex.a*t14.a + (1.0 - tex.a));
//	t15 = lerp(tex,t15,tex.a*t15.a + (1.0 - tex.a));*/
//
//	float4 blur = 0.0;
//
//	blur +=	t00*BlurC.r;
//	blur +=	t01*BlurC.g;
//	blur +=	t02*BlurC.b;
//	blur +=	t03*BlurC.a;
//
//	blur +=	t04*BlurD.r;
//	blur +=	t05*BlurD.g;
//	blur +=	t06*BlurD.b;
//	blur +=	t07*BlurD.a;
//
//	blur +=	t08*BlurD.a;
//	blur +=	t09*BlurD.b;
//	blur +=	t10*BlurD.g;
//	blur +=	t11*BlurD.r;
//
//	blur +=	t12*BlurC.a;
//	blur +=	t13*BlurC.b;
//	blur +=	t14*BlurC.g;
//	blur +=	t15*BlurC.r;
//
//	float max_component = max(max(blur.r,blur.g),blur.b);
//
//	if( max_component < GlowThreshold )
//	{
//		blur = 0.0;
//	}
//
//	return blur;
//}
///*
//float4 IterativeBlurWithClampV_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*GlowBlurPass;
//	
//	float4 t00 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-7.5));
//	float4 t01 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-6.5));
//	float4 t02 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-5.5));
//	float4 t03 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-4.5));
//
//	float4 t04 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-3.5));
//	float4 t05 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-2.5));
//	float4 t06 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-1.5));
//	float4 t07 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-0.5));
//
//	float4 t08 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 0.5));
//	float4 t09 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 1.5));
//	float4 t10 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 2.5));
//	float4 t11 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 3.5));
//
//	float4 t12 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 4.5));
//	float4 t13 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 5.5));
//	float4 t14 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 6.5));
//	float4 t15 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 7.5));
//
//	t00 = lerp(tex,t00,tex.a*t00.a + (1.0 - tex.a));
//	t01 = lerp(tex,t01,tex.a*t01.a + (1.0 - tex.a));
//	t02 = lerp(tex,t02,tex.a*t02.a + (1.0 - tex.a));
//	t03 = lerp(tex,t03,tex.a*t03.a + (1.0 - tex.a));
//
//	t04 = lerp(tex,t04,tex.a*t04.a + (1.0 - tex.a));
//	t05 = lerp(tex,t05,tex.a*t05.a + (1.0 - tex.a));
//	t06 = lerp(tex,t06,tex.a*t06.a + (1.0 - tex.a));
//	t07 = lerp(tex,t07,tex.a*t07.a + (1.0 - tex.a));
//
//	t08 = lerp(tex,t08,tex.a*t08.a + (1.0 - tex.a));
//	t09 = lerp(tex,t09,tex.a*t09.a + (1.0 - tex.a));
//	t10 = lerp(tex,t10,tex.a*t10.a + (1.0 - tex.a));
//	t11 = lerp(tex,t11,tex.a*t11.a + (1.0 - tex.a));
//
//	t12 = lerp(tex,t12,tex.a*t12.a + (1.0 - tex.a));
//	t13 = lerp(tex,t13,tex.a*t13.a + (1.0 - tex.a));
//	t14 = lerp(tex,t14,tex.a*t14.a + (1.0 - tex.a));
//	t15 = lerp(tex,t15,tex.a*t15.a + (1.0 - tex.a));
//
//	float4 blur = 0.0;
//
//	blur +=	t00*BlurC.r;
//	blur +=	t01*BlurC.g;
//	blur +=	t02*BlurC.b;
//	blur +=	t03*BlurC.a;
//
//	blur +=	t04*BlurD.r;
//	blur +=	t05*BlurD.g;
//	blur +=	t06*BlurD.b;
//	blur +=	t07*BlurD.a;
//
//	blur +=	t08*BlurD.a;
//	blur +=	t09*BlurD.b;
//	blur +=	t10*BlurD.g;
//	blur +=	t11*BlurD.r;
//
//	blur +=	t12*BlurC.a;
//	blur +=	t13*BlurC.b;
//	blur +=	t14*BlurC.g;
//	blur +=	t15*BlurC.r;
//
//	return blur;
//}
//*//*
//float4 IterativeBlur_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex1 = tex2D(PP_CommonTexture_Wrap,InTex);
//	float4 tex2 = tex2D(PP_CommonTexture_Wrap,InTex + (float2( TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 tex3 = tex2D(PP_CommonTexture_Wrap,InTex + (float2(-TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 tex4 = tex2D(PP_CommonTexture_Wrap,InTex + (float2( TexelSizeX,-TexelSizeY))*GlowBlurPass);
//	float4 tex5 = tex2D(PP_CommonTexture_Wrap,InTex + (float2(-TexelSizeX,-TexelSizeY))*GlowBlurPass);
//
//	tex2 = lerp(tex1,tex2,tex1.a*tex2.a + (1.0 - tex1.a));
//	tex3 = lerp(tex1,tex3,tex1.a*tex3.a + (1.0 - tex1.a));
//	tex4 = lerp(tex1,tex4,tex1.a*tex4.a + (1.0 - tex1.a));
//	tex5 = lerp(tex1,tex5,tex1.a*tex5.a + (1.0 - tex1.a));
//
//	float4 blur = (tex1 + tex2 + tex3 + tex4 + tex5)*0.2;
////	float4 blur = (tex2 + tex3 + tex4 + tex5)*0.25;
//
////	return blur;
////	return lerp(tex1,blur,BlurFactor/(0.75 + sqrt(GlowBlurPass)));
//	return lerp(tex1,blur,BlurFactor);
//}
//*/
//float4 IterativeBlurH_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 d = float2(TexelSizeX,TexelSizeY)*GlowBlurPass;
//
//	float2 pos = p.uv + d*float2(-7.5,0.0);
//
//	d.x *= 16.0/48;
//
//	pos.x -= d.x*0.5;
//
//	float4 blur = 0.0;
//
//	blur += Blur8.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur8.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur8.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur8.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += Blur9.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur9.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur9.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur9.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += BlurA.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurA.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurA.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurA.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += BlurB.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurB.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurB.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurB.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += BlurC.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurC.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurC.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurC.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += BlurD.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurD.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurD.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurD.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += BlurD.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurD.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurD.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurD.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += BlurC.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurC.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurC.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurC.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += BlurB.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurB.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurB.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurB.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += BlurA.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurA.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurA.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += BlurA.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += Blur9.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur9.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur9.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur9.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	blur += Blur8.a*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur8.b*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur8.g*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//	blur += Blur8.r*tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//	return blur;//*1.03;
//}
//
//float4 IterativeBlurV_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 d = float2(TexelSizeX,TexelSizeY)*GlowBlurPass;
//
//	float2 pos = p.uv + d*float2(0.0,-7.5);
//
//	d.y *= 16.0/48;
//
//	pos.y -= d.y*0.5;
//
//	float4 blur = 0.0;
//
//	blur += Blur8.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur8.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur8.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur8.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += Blur9.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur9.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur9.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur9.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += BlurA.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurA.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurA.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurA.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += BlurB.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurB.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurB.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurB.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += BlurC.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurC.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurC.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurC.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += BlurD.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurD.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurD.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurD.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += BlurD.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurD.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurD.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurD.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += BlurC.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurC.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurC.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurC.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += BlurB.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurB.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurB.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurB.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += BlurA.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurA.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurA.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += BlurA.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += Blur9.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur9.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur9.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur9.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	blur += Blur8.a*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur8.b*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur8.g*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//	blur += Blur8.r*tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//	return blur;//*1.03;
//}
//
//float4x4 MotionMatrix;
//float	 MotionFactor;
//
//float4 CalculateDir_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,InTex);
//
//	float z = tex.a*200.0;
//
//	z += mProjection[3][2];
//
//	z = clamp(z,1.0,100.0);
//
//	float4 et = mul(float3(p.pos.xy,z),mProjection);
//
//	float4 pos = float4(p.pos.xy,z,1.0);
////	float4 pos = float4(p.pos.xy,et.z,et.w);
////	float4 pos = float4(p.pos.xy,z,et.w);
//
//	pos.xy *= et.w;
////	pos.xy *= mProjection[2][3]*z;
//
//	float4 r = mul(pos,MotionMatrix);// r.w = 1.0;
//
//	r.z = clamp(r.z,1.0,100.0);
//
//	et = mul(float4(r.xyz,1.0),mProjection);
////	r = mul(r,mProjection);
//
////	r.xy /= r.w;
//
//	r.xy /= et.w;
////	r.xy /= mProjection[2][3]*z;
//
//	float2 d = p.pos.xy - r.xy; d.x = -d.x;
//
////	float3 dir = normalize(float3(d,0.0));
//	float2 dir = normalize(d);
//
//	float4 res = 0.0;
//
//	float len = length(d);
//
////	if( len > 0.5 )
////		len = 0.0;
//
////	res.xy = dir;//.xy;
//	res.xy = dir*0.5 + 0.5;
//	res.a  = 1.0 - 1.0/(1.0 + len*MotionFactor);
//
//	res.a = res.a*res.a;
//
//	res.z = tex.a;
//
//	return res;
//}
///*
//float4 CalculateDir_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,InTex);
//
//	float z = tex.a*200.0;
//
//	z = clamp(z,1.0,100.0);
//
//	float3 pos = float3(p.pos.xy,z);
//
//	float m00 = mProjection[0][0];
//	float m11 = mProjection[1][1];
//
//	pos.x = pos.x*z/m00;
//	pos.y = pos.y*z/m11;
//
//	float3 r = mul(pos,MotionMatrix);
//
//	float2 q = r.xy;
//
//	r.z = clamp(r.z,1.0,100.0);
//
//	float2 t;
//
////	t.x = pos.x*z/m00;
////	t.y = pos.y*z/m11;
//	t = pos.xy;
//
//	r.x = r.x*m00/r.z;
//	r.y = r.y*m11/r.z;
//
////	float2 d = pos.xy - r.xy;
//	float2 d = p.pos.xy - r.xy;
//
//	// ???
//	d.x = -d.x;
//
//	float2 dir = normalize(d);
//
//	float4 res = 0.0;
//
////	res.xy = dir*0.5 + 0.5;
//	res.xy = q.xy*0.5 + 0.5;
//
//	res.a  = 1.0 - 1.0/(1.0 + length(d)*MotionFactor);
////	res.a = length(pos.xy - r.xy)*0.1;
//
////	res.z = tex.a;
//	res.z = r.z*0.005;
//
//	return res;
//}
//*/
//texture MotionTexture;
//
//sampler MotionTexture_Wrap =
//sampler_state
//{
//    Texture = <MotionTexture>;
//    MinFilter = Linear;
//    MagFilter = Linear;
//////AddressU = Wrap;
//////AddressV = Wrap;
//	AddressU = Clamp;
//	AddressV = Clamp;
//};
//
//sampler MotionTexture_Prec =
//sampler_state
//{
//    Texture = <MotionTexture>;
//    MinFilter = Point;
//    MagFilter = Point;
//////AddressU = Wrap;
//////AddressV = Wrap;
//	AddressU = Clamp;
//	AddressV = Clamp;
//};
//
//float FirstMotion;
//float MotionScale;
//
//float4 IterativeBlurDir_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex1 = tex2D(PP_CommonTexture_Wrap,InTex);
//	float4 inf1 = tex2D(   MotionTexture_Wrap,InTex);
//
//	float3 scale = float3(TexelSizeX,TexelSizeY,0.0);
//
///*	float k = tex1.a;
//
//	if( FirstMotion > 0.0 )
//		k = 0.0;*/
//
////	float3 dir = scale*float3(inf1.xy*2.0 - 1.0,0.0)*inf1.a*MotionScale;
////	float3 dir = scale*float3(inf1.xy*2.0 - 1.0,0.0)*k*MotionScale;
//	float3 dir = scale*float3(inf1.xy*2.0 - 1.0,0.0)*MotionScale*inf1.a;
//
//	dir *= GlowBlurPass;
//
//	float4 tex2 = tex2D(PP_CommonTexture_Wrap,InTex - dir*4.0);
//	float4 tex3 = tex2D(PP_CommonTexture_Wrap,InTex - dir*3.0);
//	float4 tex4 = tex2D(PP_CommonTexture_Wrap,InTex - dir*2.0);
//	float4 tex5 = tex2D(PP_CommonTexture_Wrap,InTex - dir*1.0);
//	float4 tex6 = tex2D(PP_CommonTexture_Wrap,InTex + dir*1.0);
//	float4 tex7 = tex2D(PP_CommonTexture_Wrap,InTex + dir*2.0);
//	float4 tex8 = tex2D(PP_CommonTexture_Wrap,InTex + dir*3.0);
////	float4 tex9 = tex2D(PP_CommonTexture_Wrap,InTex + dir*4.0);
//
//	float4 inf2 = tex2D(   MotionTexture_Wrap,InTex - dir*4.0);
//	float4 inf3 = tex2D(   MotionTexture_Wrap,InTex - dir*3.0);
//	float4 inf4 = tex2D(   MotionTexture_Wrap,InTex - dir*2.0);
//	float4 inf5 = tex2D(   MotionTexture_Wrap,InTex - dir*1.0);
//	float4 inf6 = tex2D(   MotionTexture_Wrap,InTex + dir*1.0);
//	float4 inf7 = tex2D(   MotionTexture_Wrap,InTex + dir*2.0);
//	float4 inf8 = tex2D(   MotionTexture_Wrap,InTex + dir*3.0);
////	float4 inf9 = tex2D(   MotionTexture_Wrap,InTex + dir*4.0);
//
//	tex2 = lerp(tex1,tex2,inf1.z > inf2.z ? inf2.a : inf1.a);
//	tex3 = lerp(tex1,tex3,inf1.z > inf3.z ? inf3.a : inf1.a);
//	tex4 = lerp(tex1,tex4,inf1.z > inf4.z ? inf4.a : inf1.a);
//	tex5 = lerp(tex1,tex5,inf1.z > inf5.z ? inf5.a : inf1.a);
//	tex6 = lerp(tex1,tex6,inf1.z > inf6.z ? inf6.a : inf1.a);
//	tex7 = lerp(tex1,tex7,inf1.z > inf7.z ? inf7.a : inf1.a);
//	tex8 = lerp(tex1,tex8,inf1.z > inf8.z ? inf8.a : inf1.a);
////	tex9 = lerp(tex1,tex9,inf1.z > inf9.z ? inf9.a : inf1.a);
//
////	float4 blur = tex1*0.2 + (tex5 + tex6)*0.2 + (tex4 + tex7)*0.1 + (tex3 + tex8)*0.05 + (tex2 + tex9)*0.05;
//
//	float4 blur = tex1*0.2 + (tex5 + tex6)*0.2 + (tex4 + tex7)*0.1 + (tex3 + tex8)*0.05 + (tex2)*0.1;
////	float4 blur = (tex2 + tex3 + tex4 + tex5 + tex6 + tex7 + tex8)/7.0;
//
//	float4 res = lerp(tex1,blur,BlurFactor);
//
//	if( FirstMotion > 0.0 )
//		res.a = inf1.a;
////	res = inf1.z;
////	res.rb = inf1.xy;
////	res.g = 0.0;
////	res = inf1.a;
////	res = inf1.z;
////	res.a = 1.0;
//
//	return res;
//}
//
//float FocusDist; // фокусное расстояние
//
//float BackScale;
//float ForeScale;
//
//#ifndef _XBOX
///*
//float4 IterativeBlurDepth_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex1 = tex2D(PP_CommonTexture_Wrap,InTex);
//	float4 tex2 = tex2D(PP_CommonTexture_Wrap,InTex + (float2( TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 tex3 = tex2D(PP_CommonTexture_Wrap,InTex + (float2(-TexelSizeX, TexelSizeY))*GlowBlurPass);
//	float4 tex4 = tex2D(PP_CommonTexture_Wrap,InTex + (float2( TexelSizeX,-TexelSizeY))*GlowBlurPass);
//	float4 tex5 = tex2D(PP_CommonTexture_Wrap,InTex + (float2(-TexelSizeX,-TexelSizeY))*GlowBlurPass);
//
//	tex2 = lerp(tex1,tex2,tex2.a);
//	tex3 = lerp(tex1,tex3,tex3.a);
//	tex4 = lerp(tex1,tex4,tex4.a);
//	tex5 = lerp(tex1,tex5,tex5.a);
//
//	float4 blur = (tex1 + tex2 + tex3 + tex4 + tex5)*0.2;
////	float4 blur = (tex2 + tex3 + tex4 + tex5)*0.25;
//
////	return blur;
//	return lerp(tex1,blur,BlurFactor);
//}
//*/
//float4 IterativeBlurDepthH_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float k = BlurFactor*tex.a*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*k;
//
//	float4 t00 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-7.5,0.0));
//	float4 t01 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-6.5,0.0));
//	float4 t02 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-5.5,0.0));
//	float4 t03 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-4.5,0.0));
//
//	float4 t04 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-3.5,0.0));
//	float4 t05 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-2.5,0.0));
//	float4 t06 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-1.5,0.0));
//	float4 t07 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-0.5,0.0));
//
//	float4 t08 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 0.5,0.0));
//	float4 t09 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 1.5,0.0));
//	float4 t10 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 2.5,0.0));
//	float4 t11 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 3.5,0.0));
//
//	float4 t12 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 4.5,0.0));
//	float4 t13 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 5.5,0.0));
//	float4 t14 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 6.5,0.0));
//	float4 t15 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 7.5,0.0));
//
//	t00 = lerp(tex,t00,t00.a);
//	t01 = lerp(tex,t01,t01.a);
//	t02 = lerp(tex,t02,t02.a);
//	t03 = lerp(tex,t03,t03.a);
//
//	t04 = lerp(tex,t04,t04.a);
//	t05 = lerp(tex,t05,t05.a);
//	t06 = lerp(tex,t06,t06.a);
//	t07 = lerp(tex,t07,t07.a);
//
//	t08 = lerp(tex,t08,t08.a);
//	t09 = lerp(tex,t09,t09.a);
//	t10 = lerp(tex,t10,t10.a);
//	t11 = lerp(tex,t11,t11.a);
//
//	t12 = lerp(tex,t12,t12.a);
//	t13 = lerp(tex,t13,t13.a);
//	t14 = lerp(tex,t14,t14.a);
//	t15 = lerp(tex,t15,t15.a);
//
//	float4 blur = 0.0f;
//
//	blur +=	t00*BlurC.r;
//	blur +=	t01*BlurC.g;
//	blur +=	t02*BlurC.b;
//	blur +=	t03*BlurC.a;
//
//	blur +=	t04*BlurD.r;
//	blur +=	t05*BlurD.g;
//	blur +=	t06*BlurD.b;
//	blur +=	t07*BlurD.a;
//
//	blur +=	t08*BlurD.a;
//	blur +=	t09*BlurD.b;
//	blur +=	t10*BlurD.g;
//	blur +=	t11*BlurD.r;
//
//	blur +=	t12*BlurC.a;
//	blur +=	t13*BlurC.b;
//	blur +=	t14*BlurC.g;
//	blur +=	t15*BlurC.r;
//
//	return blur;
//}
//
//float4 IterativeBlurDepthV_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float k = BlurFactor*tex.a*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*k;
//
//	float4 t00 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-7.5));
//	float4 t01 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-6.5));
//	float4 t02 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-5.5));
//	float4 t03 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-4.5));
//
//	float4 t04 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-3.5));
//	float4 t05 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-2.5));
//	float4 t06 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-1.5));
//	float4 t07 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-0.5));
//
//	float4 t08 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 0.5));
//	float4 t09 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 1.5));
//	float4 t10 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 2.5));
//	float4 t11 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 3.5));
//
//	float4 t12 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 4.5));
//	float4 t13 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 5.5));
//	float4 t14 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 6.5));
//	float4 t15 = tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 7.5));
//
//	t00 = lerp(tex,t00,t00.a);
//	t01 = lerp(tex,t01,t01.a);
//	t02 = lerp(tex,t02,t02.a);
//	t03 = lerp(tex,t03,t03.a);
//
//	t04 = lerp(tex,t04,t04.a);
//	t05 = lerp(tex,t05,t05.a);
//	t06 = lerp(tex,t06,t06.a);
//	t07 = lerp(tex,t07,t07.a);
//
//	t08 = lerp(tex,t08,t08.a);
//	t09 = lerp(tex,t09,t09.a);
//	t10 = lerp(tex,t10,t10.a);
//	t11 = lerp(tex,t11,t11.a);
//
//	t12 = lerp(tex,t12,t12.a);
//	t13 = lerp(tex,t13,t13.a);
//	t14 = lerp(tex,t14,t14.a);
//	t15 = lerp(tex,t15,t15.a);
//
//	float4 blur = 0.0f;
//
//	blur +=	t00*BlurC.r;
//	blur +=	t01*BlurC.g;
//	blur +=	t02*BlurC.b;
//	blur +=	t03*BlurC.a;
//
//	blur +=	t04*BlurD.r;
//	blur +=	t05*BlurD.g;
//	blur +=	t06*BlurD.b;
//	blur +=	t07*BlurD.a;
//
//	blur +=	t08*BlurD.a;
//	blur +=	t09*BlurD.b;
//	blur +=	t10*BlurD.g;
//	blur +=	t11*BlurD.r;
//
//	blur +=	t12*BlurC.a;
//	blur +=	t13*BlurC.b;
//	blur +=	t14*BlurC.g;
//	blur +=	t15*BlurC.r;
//
//	if( GlowBlurPass > 1.0 )
//	{
//	//	if( k > 1.0 )
//	//		k = 1.0;
//	/*	if( k > 2.0 )
//			k = 2.0;
//		k *= 0.5f;*/
//	/*	if( k > 4.0 )
//			k = 4.0;
//		k *= 0.25f;*/
//		if( k > BlurDivider )
//			k = BlurDivider;
//		k /= BlurDivider;
//
//	//	blur.a *= k;
//		blur.a  = k;
//
//	//	blur.rgb = tex.a;
//	//	blur.a *= tex.a;
//	}
//
//	return blur;
//}
//
//#else
///*
//float4 IterativeBlurDepth_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float2 t2 = float2( TexelSizeX, TexelSizeY)*GlowBlurPass;
//	float2 t3 = float2(-TexelSizeX, TexelSizeY)*GlowBlurPass;
//	float2 t4 = float2( TexelSizeX,-TexelSizeY)*GlowBlurPass;
//	float2 t5 = float2(-TexelSizeX,-TexelSizeY)*GlowBlurPass;
//
//	float4 tex1 = tex2D(PP_CommonTexture_Wrap,InTex);
//	float4 tex2 = tex2D(PP_CommonTexture_Wrap,InTex + t2);
//	float4 tex3 = tex2D(PP_CommonTexture_Wrap,InTex + t3);
//	float4 tex4 = tex2D(PP_CommonTexture_Wrap,InTex + t4);
//	float4 tex5 = tex2D(PP_CommonTexture_Wrap,InTex + t5);
//
//	float z1 = tex2D(MotionTexture_Wrap,InTex	  ).r;
//	float z2 = tex2D(MotionTexture_Wrap,InTex + t2).r;
//	float z3 = tex2D(MotionTexture_Wrap,InTex + t3).r;
//	float z4 = tex2D(MotionTexture_Wrap,InTex + t4).r;
//	float z5 = tex2D(MotionTexture_Wrap,InTex + t5).r;
//
//	float m32 = mProjection[3][2];
//	float m22 = mProjection[2][2];
//
//	float k1 = m32*(1.0 + m22/(z1 - m22));
//	float k2 = m32*(1.0 + m22/(z2 - m22));
//	float k3 = m32*(1.0 + m22/(z3 - m22));
//	float k4 = m32*(1.0 + m22/(z4 - m22));
//	float k5 = m32*(1.0 + m22/(z5 - m22));
//
//	float a = ForeScale/(FocusDist);
//
////	float b = BackScale/(FocusDist*6);
//	float b = BackScale/(FocusDist*5);
//
//	k1 = k1 < FocusDist ? (FocusDist - k1)*a : (k1 - FocusDist)*b;
//	k2 = k2 < FocusDist ? (FocusDist - k2)*a : (k2 - FocusDist)*b;
//	k3 = k3 < FocusDist ? (FocusDist - k3)*a : (k3 - FocusDist)*b;
//	k4 = k4 < FocusDist ? (FocusDist - k4)*a : (k4 - FocusDist)*b;
//	k5 = k5 < FocusDist ? (FocusDist - k5)*a : (k5 - FocusDist)*b;
//
//	k1 = clamp(k1,0.0,1.0);
//	k2 = clamp(k2,0.0,1.0);
//	k3 = clamp(k3,0.0,1.0);
//	k4 = clamp(k4,0.0,1.0);
//	k5 = clamp(k5,0.0,1.0);
//
////	k1 *= k1;
////	k2 *= k2;
////	k3 *= k3;
////	k4 *= k4;
////	k5 *= k5;
//
//	if( z2 < z1 ) tex2 = lerp(tex1,tex2,k2);
//	if( z3 < z1 ) tex3 = lerp(tex1,tex3,k3);
//	if( z4 < z1 ) tex4 = lerp(tex1,tex4,k4);
//	if( z5 < z1 ) tex5 = lerp(tex1,tex5,k5);
//
//	float4 blur = (tex1 + tex2 + tex3 + tex4 + tex5)*0.2;
////	float4 blur = (tex2 + tex3 + tex4 + tex5)*0.25;
//
////	return blur;
////	return lerp(tex1,blur,BlurFactor);
//	float4 res = lerp(tex1,blur,BlurFactor);
//	res.a = k1;
//
//	return res;
//}
//*//*
//float4 IterativeBlurDepthH_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float zzz = tex2D(MotionTexture_Wrap,p.uv).r;
//
//	float m32 = mProjection[3][2];
//	float m22 = mProjection[2][2];
//
//	float kkk = m32*(1.0 + m22/(zzz - m22));
//
//	float a = ForeScale/(FocusDist);
//	float b = BackScale/(FocusDist*5);
//
//	kkk = saturate(kkk < FocusDist ? (FocusDist - kkk)*a : (kkk - FocusDist)*b);
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float2 p00 = p.uv + d*float2(-7.5,0.0);
//	float2 p01 = p.uv + d*float2(-6.5,0.0);
//	float2 p02 = p.uv + d*float2(-5.5,0.0);
//	float2 p03 = p.uv + d*float2(-4.5,0.0);
//
//	float2 p04 = p.uv + d*float2(-3.5,0.0);
//	float2 p05 = p.uv + d*float2(-2.5,0.0);
//	float2 p06 = p.uv + d*float2(-1.5,0.0);
//	float2 p07 = p.uv + d*float2(-0.5,0.0);
//
//	float2 p08 = p.uv + d*float2( 0.5,0.0);
//	float2 p09 = p.uv + d*float2( 1.5,0.0);
//	float2 p10 = p.uv + d*float2( 2.5,0.0);
//	float2 p11 = p.uv + d*float2( 3.5,0.0);
//
//	float2 p12 = p.uv + d*float2( 4.5,0.0);
//	float2 p13 = p.uv + d*float2( 5.5,0.0);
//	float2 p14 = p.uv + d*float2( 6.5,0.0);
//	float2 p15 = p.uv + d*float2( 7.5,0.0);
//
//	float4 z00 = float4(tex2D(MotionTexture_Wrap,p00).r,tex2D(MotionTexture_Wrap,p01).r,tex2D(MotionTexture_Wrap,p02).r,tex2D(MotionTexture_Wrap,p03).r);
//	float4 z04 = float4(tex2D(MotionTexture_Wrap,p04).r,tex2D(MotionTexture_Wrap,p05).r,tex2D(MotionTexture_Wrap,p06).r,tex2D(MotionTexture_Wrap,p07).r);
//	float4 z08 = float4(tex2D(MotionTexture_Wrap,p08).r,tex2D(MotionTexture_Wrap,p09).r,tex2D(MotionTexture_Wrap,p10).r,tex2D(MotionTexture_Wrap,p11).r);
//	float4 z12 = float4(tex2D(MotionTexture_Wrap,p12).r,tex2D(MotionTexture_Wrap,p13).r,tex2D(MotionTexture_Wrap,p14).r,tex2D(MotionTexture_Wrap,p15).r);
//
//	float4 k00 = m32*(1.0 + m22/(z00 - m22));
//	float4 k04 = m32*(1.0 + m22/(z04 - m22));
//	float4 k08 = m32*(1.0 + m22/(z08 - m22));
//	float4 k12 = m32*(1.0 + m22/(z12 - m22));
//
//	k00 = saturate(k00 < FocusDist ? (FocusDist - k00)*a : (k00 - FocusDist)*b);
//	k04 = saturate(k04 < FocusDist ? (FocusDist - k04)*a : (k04 - FocusDist)*b);
//	k08 = saturate(k08 < FocusDist ? (FocusDist - k08)*a : (k08 - FocusDist)*b);
//	k12 = saturate(k12 < FocusDist ? (FocusDist - k12)*a : (k12 - FocusDist)*b);
//
////	if( z00 < zzz ) t00 = lerp(tex,t00,k00);
////	if( z04 < zzz ) t04 = lerp(tex,t04,k04);
////	if( z08 < zzz ) t08 = lerp(tex,t08,k08);
////	if( z12 < zzz ) t12 = lerp(tex,t12,k12);
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float4 blur = 0.0f;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p00),k00.r)*BlurC.r;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p01),k00.g)*BlurC.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p02),k00.b)*BlurC.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p03),k00.a)*BlurC.a;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p04),k04.r)*BlurD.r;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p05),k04.g)*BlurD.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p06),k04.b)*BlurD.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p07),k04.a)*BlurD.a;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p08),k08.r)*BlurD.a;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p09),k08.g)*BlurD.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p10),k08.b)*BlurD.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p11),k08.a)*BlurD.r;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p12),k12.r)*BlurC.a;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p13),k12.g)*BlurC.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p14),k12.b)*BlurC.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p15),k12.a)*BlurC.r;
//
//	blur.a = kkk;
//
//	return blur;
//}
//*//*
//float4 IterativeBlurDepthH_BOX_First_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float zzz = tex2D(MotionTexture_Prec,p.uv).r;
//
//	const float m32 = mProjection[3][2];
//	const float m22 = mProjection[2][2];
//
//	float kkk = m32*(1.0 + m22/(zzz - m22));
//
////	float a = ForeScale/(FocusDist);
////	float b = BackScale/(FocusDist*5);
//
////	kkk = saturate(kkk < FocusDist ? (FocusDist - kkk)*a : (kkk - FocusDist)*b);
//	kkk = kkk < FocusDist ? ForeScale*0.4*(FocusDist - kkk)/kkk : BackScale*(kkk - FocusDist)/kkk;
//
//	float a_out = kkk;
//
////	kkk = 0.5 + 0.5*kkk;
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float4 blur = 0.0f;
//
//	float2 pos = p.uv + d*float2(-7.5,0.0);
//	float2 ppp = pos;
//
//	float4 z;
//	float4 k;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
//	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.r)*BlurC.r; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.g)*BlurC.g; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.b)*BlurC.b; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.a)*BlurC.a; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
//	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.r)*BlurD.r; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.g)*BlurD.g; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.b)*BlurD.b; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.a)*BlurD.a; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
//	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.r)*BlurD.a; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.g)*BlurD.b; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.b)*BlurD.g; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.a)*BlurD.r; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
//	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.r)*BlurC.a; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.g)*BlurC.b; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.b)*BlurC.g; ppp.x += d.x;
//	blur +=	lerp(tex,float4(tex2D(PP_CommonTexture_Wrap,ppp).rgb,k.r),k.a)*BlurC.r; ppp.x += d.x;
//
////	blur.a = kkk;
////	blur.a = a_out;
//
//	return blur;
//}
//*/
//float3 DOFPixelSize;
///*
//float4 IterativeBlurDepthH_BOX_First_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float zzz = tex2D(MotionTexture_Prec,p.uv).r;
//
//	const float m32 = mProjection[3][2];
//	const float m22 = mProjection[2][2];
//
//	float kkk = m32*(1.0 + m22/(zzz - m22));
//
////	float a = ForeScale/(FocusDist);
////	float b = BackScale/(FocusDist*5);
//
////	kkk = saturate(kkk < FocusDist ? (FocusDist - kkk)*a : (kkk - FocusDist)*b);
////	kkk = kkk < FocusDist ? ForeScale*0.4*(FocusDist - kkk)/kkk : BackScale*(kkk - FocusDist)/kkk;
//	kkk = kkk < FocusDist ? ForeScale*(1.0 - kkk/FocusDist) : BackScale*(kkk - FocusDist)/kkk;
//
//	kkk = 0.5 + 0.5*kkk;
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float4 blur = 0.0f;
//
//	float2 pos = p.uv + d*float2(-7.5,0.0);
//	float2 ppp = pos;
//
//	float4 z;
//	float4 k;
//
//	float4 t;
//
//	float2 tc0 = float2(-DOFPixelSize.x,-DOFPixelSize.y);
//	float2 tc1 = float2( DOFPixelSize.x,-DOFPixelSize.y);
//	float2 tc2 = float2(-DOFPixelSize.x, DOFPixelSize.y);
//	float2 tc3 = float2( DOFPixelSize.x, DOFPixelSize.y);
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.r)*BlurC.r; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.g)*BlurC.g; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.b)*BlurC.b; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.a)*BlurC.a; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.r)*BlurD.r; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.g)*BlurD.g; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.b)*BlurD.b; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.a)*BlurD.a; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.r)*BlurD.a; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.g)*BlurD.b; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.b)*BlurD.g; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.a)*BlurD.r; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.r)*BlurC.a; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.g)*BlurC.b; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.b)*BlurC.g; ppp.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,ppp + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,ppp + tc3);
//
//	blur +=	lerp(tex,t*0.25,k.a)*BlurC.r; ppp.x += d.x;
//
////	blur.a = kkk;
//
//	return blur;
//}
//*/
//float4 ResizeRectSimple_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 tc0 = float2(-DOFPixelSize.x,-DOFPixelSize.y);
//	float2 tc1 = float2( DOFPixelSize.x,-DOFPixelSize.y);
//	float2 tc2 = float2(-DOFPixelSize.x, DOFPixelSize.y);
//	float2 tc3 = float2( DOFPixelSize.x, DOFPixelSize.y);
//
//	float4 t;
//	
//	t  = tex2D(PP_CommonTexture_Wrap,p.uv + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,p.uv + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,p.uv + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,p.uv + tc3);
//
//	return t*0.25;
//
///*	float2 lt = p.uv + float2(-DOFPixelSize.x,-DOFPixelSize.y);
//	float2 rb = p.uv + float2( DOFPixelSize.x, DOFPixelSize.y);
//
//	if( lt.x < 0.0 ) lt.x = p.uv.x;
//	if( lt.y < 0.0 ) lt.y = p.uv.y;
//
//	if( rb.x > 1.0 ) rb.x = p.uv.x;
//	if( rb.y > 1.0 ) rb.y = p.uv.y;
//
//	float4 t;
//	
//	t  = tex2D(PP_CommonTexture_Wrap,float2(lt.x,lt.y));
//	t += tex2D(PP_CommonTexture_Wrap,float2(rb.x,lt.y));
//	t += tex2D(PP_CommonTexture_Wrap,float2(lt.x,rb.y));
//	t += tex2D(PP_CommonTexture_Wrap,float2(rb.x,rb.y));
//
//	return t*0.25;*/
//}
//
//float4 ResizeRect_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	const float m32 = mProjection[3][2];
//	const float m22 = mProjection[2][2];
//
//	float2 tc0 = float2(-DOFPixelSize.x,-DOFPixelSize.y);
//	float2 tc1 = float2( DOFPixelSize.x,-DOFPixelSize.y);
//	float2 tc2 = float2(-DOFPixelSize.x, DOFPixelSize.y);
//	float2 tc3 = float2( DOFPixelSize.x, DOFPixelSize.y);
//
//	float4 t; float k;
//
//	k = tex2D(MotionTexture_Prec,p.uv + tc0).r;
//	k = m32*(1.0 + m22/(k - m22));
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t  = lerp(tex,tex2D(PP_CommonTexture_Wrap,p.uv + tc0),k);
//
//	k = tex2D(MotionTexture_Prec,p.uv + tc1).r;
//	k = m32*(1.0 + m22/(k - m22));
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t += lerp(tex,tex2D(PP_CommonTexture_Wrap,p.uv + tc1),k);
//
//	k = tex2D(MotionTexture_Prec,p.uv + tc2).r;
//	k = m32*(1.0 + m22/(k - m22));
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t += lerp(tex,tex2D(PP_CommonTexture_Wrap,p.uv + tc2),k);
//
//	k = tex2D(MotionTexture_Prec,p.uv + tc3).r;
//	k = m32*(1.0 + m22/(k - m22));
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t += lerp(tex,tex2D(PP_CommonTexture_Wrap,p.uv + tc3),k);
//
//	t *= 0.25;
//
//	////
//
//	k = tex2D(MotionTexture_Prec,p.uv).r;
//	k = m32*(1.0 + m22/(k - m22));
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	t.a = k;
//
//	////
//
//	return t;
//}
///*
//float4 IterativeBlurDepthH_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float zzz = tex2D(MotionTexture_Prec,p.uv).r;
//
//	const float m32 = mProjection[3][2];
//	const float m22 = mProjection[2][2];
//
//	float kkk = m32*(1.0 + m22/(zzz - m22));
//
////	float a = ForeScale/(FocusDist);
////	float b = BackScale/(FocusDist*5);
//
////	kkk = saturate(kkk < FocusDist ? (FocusDist - kkk)*a : (kkk - FocusDist)*b);
////	kkk = kkk < FocusDist ? ForeScale*0.4*(FocusDist - kkk)/kkk : BackScale*(kkk - FocusDist)/kkk;
//	kkk = kkk < FocusDist ? ForeScale*(1.0 - kkk/FocusDist) : BackScale*(kkk - FocusDist)/kkk;
//
//	kkk = 0.5 + 0.5*kkk;
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float4 blur = 0.0f;
//
//	float2 pos = p.uv + d*float2(-7.5,0.0);
//	float2 ppp = pos;
//
//	float4 z;
//	float4 k;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurC.r; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurC.g; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurC.b; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurC.a; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurD.r; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurD.g; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurD.b; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurD.a; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurD.a; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurD.b; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurD.g; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurD.r; ppp.x += d.x;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.x += d.x;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurC.a; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurC.b; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurC.g; ppp.x += d.x;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurC.r; ppp.x += d.x;
//
////	blur.a = kkk;
//
//	return blur;
//}*/
//float4 IterativeBlurDepthH_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float k = tex.a;
//
//	k = 0.5 + 0.5*k;
//
//	k *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*k;
//
//	float2 pos = p.uv + d*float2(-7.5,0.0);
//
//	float4 blur = 0.0f;
//
//	float4 t00;
//	float4 t01;
//	float4 t02;
//	float4 t03;
//
//	[isolate]
//	{
//		t00 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t01 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t02 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t03 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//		blur +=	lerp(tex,t00,t00.a)*BlurC.r;
//		blur +=	lerp(tex,t01,t01.a)*BlurC.g;
//		blur +=	lerp(tex,t02,t02.a)*BlurC.b;
//		blur +=	lerp(tex,t03,t03.a)*BlurC.a;
//	}
//
//	[isolate]
//	{
//		t00 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t01 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t02 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t03 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//		blur +=	lerp(tex,t00,t00.a)*BlurD.r;
//		blur +=	lerp(tex,t01,t01.a)*BlurD.g;
//		blur +=	lerp(tex,t02,t02.a)*BlurD.b;
//		blur +=	lerp(tex,t03,t03.a)*BlurD.a;
//	}
//
//	[isolate]
//	{
//		t00 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t01 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t02 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t03 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//		blur +=	lerp(tex,t00,t00.a)*BlurD.a;
//		blur +=	lerp(tex,t01,t01.a)*BlurD.b;
//		blur +=	lerp(tex,t02,t02.a)*BlurD.g;
//		blur +=	lerp(tex,t03,t03.a)*BlurD.r;
//	}
//
//	[isolate]
//	{
//		t00 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t01 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t02 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//		t03 = tex2D(PP_CommonTexture_Wrap,pos); pos.x += d.x;
//
//		blur +=	lerp(tex,t00,t00.a)*BlurC.a;
//		blur +=	lerp(tex,t01,t01.a)*BlurC.b;
//		blur +=	lerp(tex,t02,t02.a)*BlurC.g;
//		blur +=	lerp(tex,t03,t03.a)*BlurC.r;
//	}
//
//	blur.a = tex.a;
//
//	return blur;
//}
///*
//float4 IterativeBlurDepthH_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float kkk = tex.a;
//
////	kkk = 0.5 + 0.5*kkk;
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float4 blur = 0.0f;
//
//	float2 ppp = p.uv + d*float2(-7.5,0.0);
//
//	float4 t0;
//	float4 t1;
//	float4 t2;
//	float4 t3;
//
//	t0 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t1 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t2 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t3 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//
//	blur +=	lerp(tex,t0,t0.a)*BlurC.r;
//	blur +=	lerp(tex,t1,t1.a)*BlurC.g;
//	blur +=	lerp(tex,t2,t2.a)*BlurC.b;
//	blur +=	lerp(tex,t3,t3.a)*BlurC.a;
//
//	t0 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t1 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t2 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t3 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//
//	blur +=	lerp(tex,t0,t0.a)*BlurD.r;
//	blur +=	lerp(tex,t1,t1.a)*BlurD.g;
//	blur +=	lerp(tex,t2,t2.a)*BlurD.b;
//	blur +=	lerp(tex,t3,t3.a)*BlurD.a;
//
//	t0 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t1 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t2 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t3 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//
//	blur +=	lerp(tex,t0,t0.a)*BlurD.a;
//	blur +=	lerp(tex,t1,t1.a)*BlurD.b;
//	blur +=	lerp(tex,t2,t2.a)*BlurD.g;
//	blur +=	lerp(tex,t3,t3.a)*BlurD.r;
//
//	t0 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t1 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t2 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//	t3 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.x += d.x;
//
//	blur +=	lerp(tex,t0,t0.a)*BlurC.a;
//	blur +=	lerp(tex,t1,t1.a)*BlurC.b;
//	blur +=	lerp(tex,t2,t2.a)*BlurC.g;
//	blur +=	lerp(tex,t3,t3.a)*BlurC.r;
//
////	blur.a = kkk;
////	blur.a = tex.a;
//
//	return blur;
//}
//*//*
//float4 IterativeBlurDepthV_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float zzz = tex2D(MotionTexture_Wrap,p.uv).r;
//
//	float m32 = mProjection[3][2];
//	float m22 = mProjection[2][2];
//
//	float kkk = m32*(1.0 + m22/(zzz - m22));
//
//	float a = ForeScale/(FocusDist);
//	float b = BackScale/(FocusDist*5);
//
//	kkk = saturate(kkk < FocusDist ? (FocusDist - kkk)*a : (kkk - FocusDist)*b);
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float2 p00 = p.uv + d*float2(0.0,-7.5);
//	float2 p01 = p.uv + d*float2(0.0,-6.5);
//	float2 p02 = p.uv + d*float2(0.0,-5.5);
//	float2 p03 = p.uv + d*float2(0.0,-4.5);
//
//	float2 p04 = p.uv + d*float2(0.0,-3.5);
//	float2 p05 = p.uv + d*float2(0.0,-2.5);
//	float2 p06 = p.uv + d*float2(0.0,-1.5);
//	float2 p07 = p.uv + d*float2(0.0,-0.5);
//
//	float2 p08 = p.uv + d*float2(0.0, 0.5);
//	float2 p09 = p.uv + d*float2(0.0, 1.5);
//	float2 p10 = p.uv + d*float2(0.0, 2.5);
//	float2 p11 = p.uv + d*float2(0.0, 3.5);
//
//	float2 p12 = p.uv + d*float2(0.0, 4.5);
//	float2 p13 = p.uv + d*float2(0.0, 5.5);
//	float2 p14 = p.uv + d*float2(0.0, 6.5);
//	float2 p15 = p.uv + d*float2(0.0, 7.5);
//
//	float4 z00 = float4(tex2D(MotionTexture_Wrap,p00).r,tex2D(MotionTexture_Wrap,p01).r,tex2D(MotionTexture_Wrap,p02).r,tex2D(MotionTexture_Wrap,p03).r);
//	float4 z04 = float4(tex2D(MotionTexture_Wrap,p04).r,tex2D(MotionTexture_Wrap,p05).r,tex2D(MotionTexture_Wrap,p06).r,tex2D(MotionTexture_Wrap,p07).r);
//	float4 z08 = float4(tex2D(MotionTexture_Wrap,p08).r,tex2D(MotionTexture_Wrap,p09).r,tex2D(MotionTexture_Wrap,p10).r,tex2D(MotionTexture_Wrap,p11).r);
//	float4 z12 = float4(tex2D(MotionTexture_Wrap,p12).r,tex2D(MotionTexture_Wrap,p13).r,tex2D(MotionTexture_Wrap,p14).r,tex2D(MotionTexture_Wrap,p15).r);
//
//	float4 k00 = m32*(1.0 + m22/(z00 - m22));
//	float4 k04 = m32*(1.0 + m22/(z04 - m22));
//	float4 k08 = m32*(1.0 + m22/(z08 - m22));
//	float4 k12 = m32*(1.0 + m22/(z12 - m22));
//
//	k00 = saturate(k00 < FocusDist ? (FocusDist - k00)*a : (k00 - FocusDist)*b);
//	k04 = saturate(k04 < FocusDist ? (FocusDist - k04)*a : (k04 - FocusDist)*b);
//	k08 = saturate(k08 < FocusDist ? (FocusDist - k08)*a : (k08 - FocusDist)*b);
//	k12 = saturate(k12 < FocusDist ? (FocusDist - k12)*a : (k12 - FocusDist)*b);
//
////	if( z00 < zzz ) t00 = lerp(tex,t00,k00);
////	if( z04 < zzz ) t04 = lerp(tex,t04,k04);
////	if( z08 < zzz ) t08 = lerp(tex,t08,k08);
////	if( z12 < zzz ) t12 = lerp(tex,t12,k12);
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float4 blur = 0.0f;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p00),k00.r)*BlurC.r;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p01),k00.g)*BlurC.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p02),k00.b)*BlurC.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p03),k00.a)*BlurC.a;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p04),k04.r)*BlurD.r;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p05),k04.g)*BlurD.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p06),k04.b)*BlurD.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p07),k04.a)*BlurD.a;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p08),k08.r)*BlurD.a;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p09),k08.g)*BlurD.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p10),k08.b)*BlurD.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p11),k08.a)*BlurD.r;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p12),k12.r)*BlurC.a;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p13),k12.g)*BlurC.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p14),k12.b)*BlurC.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p15),k12.a)*BlurC.r;
//
//	blur.a = kkk;
//
//	return blur;
//}
//*//*
//float4 IterativeBlurDepthV_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float zzz = tex2D(MotionTexture_Wrap,p.uv).r;
//
//	const float m32 = mProjection[3][2];
//	const float m22 = mProjection[2][2];
//
//	float kkk = m32*(1.0 + m22/(zzz - m22));
//
////	float a = ForeScale/(FocusDist);
////	float b = BackScale/(FocusDist*5);
//
////	kkk = saturate(kkk < FocusDist ? (FocusDist - kkk)*a : (kkk - FocusDist)*b);
////	kkk = kkk < FocusDist ? ForeScale*0.4*(FocusDist - kkk)/kkk : BackScale*(kkk - FocusDist)/kkk;
//	kkk = kkk < FocusDist ? ForeScale*(1.0 - kkk/FocusDist) : BackScale*(kkk - FocusDist)/kkk;
//
//	kkk = 0.5 + 0.5*kkk;
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float4 blur = 0.0f;
//
//	float2 pos = p.uv + d*float2(0.0,-7.5);
//	float2 ppp = pos;
//
//	float4 z;
//	float4 k;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurC.r; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurC.g; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurC.b; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurC.a; ppp.y += d.y;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurD.r; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurD.g; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurD.b; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurD.a; ppp.y += d.y;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurD.a; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurD.b; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurD.g; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurD.r; ppp.y += d.y;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//
//	k = m32*(1.0 + m22/(z - m22));
////	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
////	k = k < FocusDist ? ForeScale*0.4*(FocusDist - k)/k : BackScale*(k - FocusDist)/k;
//	k = k < FocusDist ? ForeScale*(1.0 - k/FocusDist) : BackScale*(k - FocusDist)/k;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurC.a; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurC.b; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurC.g; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurC.r; ppp.y += d.y;
//
////	blur.a = kkk;
//
//	return blur;
//}
//*/
//float4 IterativeBlurDepthV_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float k = tex.a;
//
//	k = 0.5 + 0.5*k;
//
//	k *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*k;
//
//	float2 pos = p.uv + d*float2(0.0,-7.5);
//
//	float4 blur = 0.0f;
//
//	float4 t00;
//	float4 t01;
//	float4 t02;
//	float4 t03;
//
//	[isolate]
//	{
//		t00 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t01 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t02 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t03 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//		blur +=	lerp(tex,t00,t00.a)*BlurC.r;
//		blur +=	lerp(tex,t01,t01.a)*BlurC.g;
//		blur +=	lerp(tex,t02,t02.a)*BlurC.b;
//		blur +=	lerp(tex,t03,t03.a)*BlurC.a;
//	}
//
//	[isolate]
//	{
//		t00 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t01 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t02 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t03 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//		blur +=	lerp(tex,t00,t00.a)*BlurD.r;
//		blur +=	lerp(tex,t01,t01.a)*BlurD.g;
//		blur +=	lerp(tex,t02,t02.a)*BlurD.b;
//		blur +=	lerp(tex,t03,t03.a)*BlurD.a;
//	}
//
//	[isolate]
//	{
//		t00 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t01 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t02 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t03 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//		blur +=	lerp(tex,t00,t00.a)*BlurD.a;
//		blur +=	lerp(tex,t01,t01.a)*BlurD.b;
//		blur +=	lerp(tex,t02,t02.a)*BlurD.g;
//		blur +=	lerp(tex,t03,t03.a)*BlurD.r;
//	}
//
//	[isolate]
//	{
//		t00 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t01 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t02 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//		t03 = tex2D(PP_CommonTexture_Wrap,pos); pos.y += d.y;
//
//		blur +=	lerp(tex,t00,t00.a)*BlurC.a;
//		blur +=	lerp(tex,t01,t01.a)*BlurC.b;
//		blur +=	lerp(tex,t02,t02.a)*BlurC.g;
//		blur +=	lerp(tex,t03,t03.a)*BlurC.r;
//	}
//
//	blur.a = tex.a;
//
//	return blur;
//}
///*
//float4 IterativeBlurDepthV_BOX_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float kkk = tex.a;
//
////	kkk = 0.5 + 0.5*kkk;
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float4 blur = 0.0f;
//
//	float2 ppp = p.uv + d*float2(0.0,-7.5);
//
//	float4 t0;
//	float4 t1;
//	float4 t2;
//	float4 t3;
//
//	t0 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t1 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t2 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t3 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//
//	blur +=	lerp(tex,t0,t0.a)*BlurC.r;
//	blur +=	lerp(tex,t1,t1.a)*BlurC.g;
//	blur +=	lerp(tex,t2,t2.a)*BlurC.b;
//	blur +=	lerp(tex,t3,t3.a)*BlurC.a;
//
//	t0 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t1 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t2 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t3 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//
//	blur +=	lerp(tex,t0,t0.a)*BlurD.r;
//	blur +=	lerp(tex,t1,t1.a)*BlurD.g;
//	blur +=	lerp(tex,t2,t2.a)*BlurD.b;
//	blur +=	lerp(tex,t3,t3.a)*BlurD.a;
//
//	t0 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t1 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t2 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t3 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//
//	blur +=	lerp(tex,t0,t0.a)*BlurD.a;
//	blur +=	lerp(tex,t1,t1.a)*BlurD.b;
//	blur +=	lerp(tex,t2,t2.a)*BlurD.g;
//	blur +=	lerp(tex,t3,t3.a)*BlurD.r;
//
//	t0 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t1 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t2 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//	t3 = tex2D(PP_CommonTexture_Wrap,ppp); ppp.y += d.y;
//
//	blur +=	lerp(tex,t0,t0.a)*BlurC.a;
//	blur +=	lerp(tex,t1,t1.a)*BlurC.b;
//	blur +=	lerp(tex,t2,t2.a)*BlurC.g;
//	blur +=	lerp(tex,t3,t3.a)*BlurC.r;
//
////	blur.a = kkk;
////	blur.a = tex.a;
//
//	return blur;
//}
//*//*
//float4 IterativeBlurDepthV_BOX_Last_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float zzz = tex2D(MotionTexture_Wrap,p.uv).r;
//
//	float m32 = mProjection[3][2];
//	float m22 = mProjection[2][2];
//
//	float kkk = m32*(1.0 + m22/(zzz - m22));
//
//	float a = ForeScale/(FocusDist);
//	float b = BackScale/(FocusDist*5);
//
//	kkk = saturate(kkk < FocusDist ? (FocusDist - kkk)*a : (kkk - FocusDist)*b);
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float2 p00 = p.uv + d*float2(0.0,-7.5);
//	float2 p01 = p.uv + d*float2(0.0,-6.5);
//	float2 p02 = p.uv + d*float2(0.0,-5.5);
//	float2 p03 = p.uv + d*float2(0.0,-4.5);
//
//	float2 p04 = p.uv + d*float2(0.0,-3.5);
//	float2 p05 = p.uv + d*float2(0.0,-2.5);
//	float2 p06 = p.uv + d*float2(0.0,-1.5);
//	float2 p07 = p.uv + d*float2(0.0,-0.5);
//
//	float2 p08 = p.uv + d*float2(0.0, 0.5);
//	float2 p09 = p.uv + d*float2(0.0, 1.5);
//	float2 p10 = p.uv + d*float2(0.0, 2.5);
//	float2 p11 = p.uv + d*float2(0.0, 3.5);
//
//	float2 p12 = p.uv + d*float2(0.0, 4.5);
//	float2 p13 = p.uv + d*float2(0.0, 5.5);
//	float2 p14 = p.uv + d*float2(0.0, 6.5);
//	float2 p15 = p.uv + d*float2(0.0, 7.5);
//
//	float4 z00 = float4(tex2D(MotionTexture_Wrap,p00).r,tex2D(MotionTexture_Wrap,p01).r,tex2D(MotionTexture_Wrap,p02).r,tex2D(MotionTexture_Wrap,p03).r);
//	float4 z04 = float4(tex2D(MotionTexture_Wrap,p04).r,tex2D(MotionTexture_Wrap,p05).r,tex2D(MotionTexture_Wrap,p06).r,tex2D(MotionTexture_Wrap,p07).r);
//	float4 z08 = float4(tex2D(MotionTexture_Wrap,p08).r,tex2D(MotionTexture_Wrap,p09).r,tex2D(MotionTexture_Wrap,p10).r,tex2D(MotionTexture_Wrap,p11).r);
//	float4 z12 = float4(tex2D(MotionTexture_Wrap,p12).r,tex2D(MotionTexture_Wrap,p13).r,tex2D(MotionTexture_Wrap,p14).r,tex2D(MotionTexture_Wrap,p15).r);
//
//	float4 k00 = m32*(1.0 + m22/(z00 - m22));
//	float4 k04 = m32*(1.0 + m22/(z04 - m22));
//	float4 k08 = m32*(1.0 + m22/(z08 - m22));
//	float4 k12 = m32*(1.0 + m22/(z12 - m22));
//
//	k00 = saturate(k00 < FocusDist ? (FocusDist - k00)*a : (k00 - FocusDist)*b);
//	k04 = saturate(k04 < FocusDist ? (FocusDist - k04)*a : (k04 - FocusDist)*b);
//	k08 = saturate(k08 < FocusDist ? (FocusDist - k08)*a : (k08 - FocusDist)*b);
//	k12 = saturate(k12 < FocusDist ? (FocusDist - k12)*a : (k12 - FocusDist)*b);
//
////	if( z00 < zzz ) t00 = lerp(tex,t00,k00);
////	if( z04 < zzz ) t04 = lerp(tex,t04,k04);
////	if( z08 < zzz ) t08 = lerp(tex,t08,k08);
////	if( z12 < zzz ) t12 = lerp(tex,t12,k12);
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float4 blur = 0.0f;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p00),k00.r)*BlurC.r;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p01),k00.g)*BlurC.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p02),k00.b)*BlurC.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p03),k00.a)*BlurC.a;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p04),k04.r)*BlurD.r;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p05),k04.g)*BlurD.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p06),k04.b)*BlurD.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p07),k04.a)*BlurD.a;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p08),k08.r)*BlurD.a;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p09),k08.g)*BlurD.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p10),k08.b)*BlurD.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p11),k08.a)*BlurD.r;
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p12),k12.r)*BlurC.a;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p13),k12.g)*BlurC.b;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p14),k12.b)*BlurC.g;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,p15),k12.a)*BlurC.r;
//
//	kkk = clamp(kkk,0.0,BlurDivider);
//
//	kkk /= BlurDivider;
//
//	blur.a = kkk;
//
//	return blur;
//}
//*//*
//float4 IterativeBlurDepthV_BOX_Last_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float zzz = tex2D(MotionTexture_Wrap,p.uv).r;
//
//	const float m32 = mProjection[3][2];
//	const float m22 = mProjection[2][2];
//
//	float kkk = m32*(1.0 + m22/(zzz - m22));
//
//	float a = ForeScale/(FocusDist);
//	float b = BackScale/(FocusDist*5);
//
//	kkk = saturate(kkk < FocusDist ? (FocusDist - kkk)*a : (kkk - FocusDist)*b);
//
//	kkk *= BlurFactor*GlowBlurPass*2.0*3.0;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*kkk;
//
//	float4 tex = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float4 blur = 0.0f;
//
//	float2 pos = p.uv + d*float2(0.0,-7.5);
//	float2 ppp = pos;
//
//	float4 z;
//	float4 k;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//
//	k = m32*(1.0 + m22/(z - m22));
//	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurC.r; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurC.g; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurC.b; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurC.a; ppp.y += d.y;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//
//	k = m32*(1.0 + m22/(z - m22));
//	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurD.r; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurD.g; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurD.b; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurD.a; ppp.y += d.y;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//
//	k = m32*(1.0 + m22/(z - m22));
//	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurD.a; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurD.b; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurD.g; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurD.r; ppp.y += d.y;
//
//	z.r = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.g = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.b = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//	z.a = tex2D(MotionTexture_Wrap,pos).r; pos.y += d.y;
//
//	k = m32*(1.0 + m22/(z - m22));
//	k = saturate(k < FocusDist ? (FocusDist - k)*a : (k - FocusDist)*b);
//
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.r)*BlurC.a; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.g)*BlurC.b; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.b)*BlurC.g; ppp.y += d.y;
//	blur +=	lerp(tex,tex2D(PP_CommonTexture_Wrap,ppp),k.a)*BlurC.r; ppp.y += d.y;
//
//	kkk = clamp(kkk,0.0,BlurDivider);
//
//	kkk /= BlurDivider;
//
//	blur.a = kkk;
//
//	return blur;
//}*/
//
//#endif // _XBOX
//
//float InnerFactor;
//float OuterFactor;
//
//struct IterativeBlur_VSOUT
//{
//	float4 p   : POSITION;
//	float2 uv  : TEXCOORD0;
//	float4 pos : TEXCOORD1;
///*	float4 a   : TEXCOORD2;
//	float4 b   : TEXCOORD3;
//	float4 c   : TEXCOORD4;
//	float4 d   : TEXCOORD5;*/
//};
//
//struct IterativeBlur_PSIN
//{
//	float2 uv  : TEXCOORD0;
//	float4 pos : TEXCOORD1;
///*	float4 a   : TEXCOORD2;
//	float4 b   : TEXCOORD3;
//	float4 c   : TEXCOORD4;
//	float4 d   : TEXCOORD5;*/
//};
//
//IterativeBlur_VSOUT IterativeRadialBlur_VS(PP_FullScreenQuad_VSIN v)
//{
//	IterativeBlur_VSOUT res;
//
//	res.p = v.pos;
//	res.uv = v.uv;
//	res.pos = v.pos;
//
///*	const float t = 4.5f;
//
//	const float a = 0.39894228;
//	const float b = 1.0/(2*t*t);
//
//	res.a.r = a/t*exp(-15.5*15.5*b);
//	res.a.g = a/t*exp(-14.5*14.5*b);
//	res.a.b = a/t*exp(-13.5*13.5*b);
//	res.a.a = a/t*exp(-12.5*12.5*b);
//
//	res.b.r = a/t*exp(-11.5*11.5*b);
//	res.b.g = a/t*exp(-10.5*10.5*b);
//	res.b.b = a/t*exp(- 9.5* 9.5*b);
//	res.b.a = a/t*exp(- 8.5* 8.5*b);
//
//	res.c.r = a/t*exp(- 7.5* 7.5*b);
//	res.c.g = a/t*exp(- 6.5* 6.5*b);
//	res.c.b = a/t*exp(- 5.5* 5.5*b);
//	res.c.a = a/t*exp(- 4.5* 4.5*b);
//
//	res.d.r = a/t*exp(- 3.5* 3.5*b);
//	res.d.g = a/t*exp(- 2.5* 2.5*b);
//	res.d.b = a/t*exp(- 1.5* 1.5*b);
//	res.d.a = a/t*exp(- 0.5* 0.5*b);*/
//
//	return res;
//}
///*
//#ifndef _XBOX
//
//float4 IterativeRadialBlurH_First_Low_PS(IterativeBlur_PSIN p) : COLOR
//{
//	float r = length(p.pos.xy);
//	float k = clamp(InnerFactor + OuterFactor*r*r,0.0,1.0);
//
//	float ttt = GlowBlurPass;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*ttt*k*2.0f*3.0;
//
//	float4 blur = 0.0f;
//
//	float2 pos = p.uv + d*float2(-7.5,0.0);
//
//	float2 ps = float2(TexelSizeX*2.0,TexelSizeY*2.0);
//
//	float2 tc0 = float2(-ps.x,-ps.y);
//	float2 tc1 = float2( ps.x,-ps.y);
//	float2 tc2 = float2(-ps.x, ps.y);
//	float2 tc3 = float2( ps.x, ps.y);
//
//	float4 t;
//
//	////
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurC.r; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurC.g; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurC.b; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurC.a; pos.x += d.x;
//
//	////
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurD.r; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurD.g; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurD.b; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurD.a; pos.x += d.x;
//
//	////
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurD.a; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurD.b; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurD.g; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurD.r; pos.x += d.x;
//
//	////
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurC.a; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurC.b; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurC.g; pos.x += d.x;
//
//	t  = tex2D(PP_CommonTexture_Wrap,pos + tc0);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc1);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc2);
//	t += tex2D(PP_CommonTexture_Wrap,pos + tc3);
//
//	blur +=	t*0.25*BlurC.r; pos.x += d.x;
//
//	////
//
//	return blur;
//}
//
//#endif
//*/
//float4 IterativeRadialBlurH_PS(IterativeBlur_PSIN p) : COLOR
//{
//	float r = length(p.pos.xy);
//	float k = clamp(InnerFactor + OuterFactor*r*r,0.0,1.0);
//
//	float t = GlowBlurPass;
//
////	if( t%1.0 < 0.5 )
////		t -= 0.5;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*t*k*2.0f*3.0;
//
//	float4 blur = 0.0f;
//
//	/*	blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-15.5,0.0))*p.a.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-14.5,0.0))*p.a.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-13.5,0.0))*p.a.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-12.5,0.0))*p.a.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-11.5,0.0))*p.b.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-10.5,0.0))*p.b.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 9.5,0.0))*p.b.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 8.5,0.0))*p.b.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 7.5,0.0))*p.c.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 6.5,0.0))*p.c.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 5.5,0.0))*p.c.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 4.5,0.0))*p.c.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 3.5,0.0))*p.d.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 2.5,0.0))*p.d.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 1.5,0.0))*p.d.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 0.5,0.0))*p.d.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  0.5,0.0))*p.d.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  1.5,0.0))*p.d.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  2.5,0.0))*p.d.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  3.5,0.0))*p.d.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  4.5,0.0))*p.c.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  5.5,0.0))*p.c.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  6.5,0.0))*p.c.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  7.5,0.0))*p.c.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  8.5,0.0))*p.b.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  9.5,0.0))*p.b.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 10.5,0.0))*p.b.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 11.5,0.0))*p.b.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 12.5,0.0))*p.a.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 13.5,0.0))*p.a.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 14.5,0.0))*p.a.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 15.5,0.0))*p.a.r;*/
//
//	/*	blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-15.5,0.0))*BlurA.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-14.5,0.0))*BlurA.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-13.5,0.0))*BlurA.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-12.5,0.0))*BlurA.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-11.5,0.0))*BlurB.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(-10.5,0.0))*BlurB.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 9.5,0.0))*BlurB.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 8.5,0.0))*BlurB.a;*/
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 7.5,0.0))*BlurC.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 6.5,0.0))*BlurC.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 5.5,0.0))*BlurC.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 4.5,0.0))*BlurC.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 3.5,0.0))*BlurD.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 2.5,0.0))*BlurD.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 1.5,0.0))*BlurD.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(- 0.5,0.0))*BlurD.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  0.5,0.0))*BlurD.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  1.5,0.0))*BlurD.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  2.5,0.0))*BlurD.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  3.5,0.0))*BlurD.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  4.5,0.0))*BlurC.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  5.5,0.0))*BlurC.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  6.5,0.0))*BlurC.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  7.5,0.0))*BlurC.r;
//
//	/*	blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  8.5,0.0))*BlurB.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(  9.5,0.0))*BlurB.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 10.5,0.0))*BlurB.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 11.5,0.0))*BlurB.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 12.5,0.0))*BlurA.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 13.5,0.0))*BlurA.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 14.5,0.0))*BlurA.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2( 15.5,0.0))*BlurA.r;*/
//
//	return blur;
//}
//
//float4 IterativeRadialBlurV_PS(IterativeBlur_PSIN p) : COLOR
//{
//	float r = length(p.pos.xy);
//	float k = clamp(InnerFactor + OuterFactor*r*r,0.0,1.0);
//
//	float t = GlowBlurPass;
//
////	if( t%1.0 < 0.5 )
////		t -= 0.5;
//
//	float2 d = float2(TexelSizeX,TexelSizeY)*t*k*2.0f*3.0;
//
//	float4 blur = 0.0f;
//	
//	/*	blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-15.5))*p.a.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-14.5))*p.a.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-13.5))*p.a.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-12.5))*p.a.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-11.5))*p.b.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-10.5))*p.b.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 9.5))*p.b.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 8.5))*p.b.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 7.5))*p.c.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 6.5))*p.c.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 5.5))*p.c.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 4.5))*p.c.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 3.5))*p.d.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 2.5))*p.d.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 1.5))*p.d.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 0.5))*p.d.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  0.5))*p.d.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  1.5))*p.d.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  2.5))*p.d.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  3.5))*p.d.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  4.5))*p.c.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  5.5))*p.c.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  6.5))*p.c.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  7.5))*p.c.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  8.5))*p.b.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  9.5))*p.b.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 10.5))*p.b.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 11.5))*p.b.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 12.5))*p.a.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 13.5))*p.a.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 14.5))*p.a.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 15.5))*p.a.r;*/
//
//	/*	blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-15.5))*BlurA.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-14.5))*BlurA.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-13.5))*BlurA.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-12.5))*BlurA.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-11.5))*BlurB.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,-10.5))*BlurB.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 9.5))*BlurB.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 8.5))*BlurB.a;*/
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 7.5))*BlurC.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 6.5))*BlurC.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 5.5))*BlurC.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 4.5))*BlurC.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 3.5))*BlurD.r;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 2.5))*BlurD.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 1.5))*BlurD.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,- 0.5))*BlurD.a;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  0.5))*BlurD.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  1.5))*BlurD.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  2.5))*BlurD.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  3.5))*BlurD.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  4.5))*BlurC.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  5.5))*BlurC.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  6.5))*BlurC.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  7.5))*BlurC.r;
//
//	/*	blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  8.5))*BlurB.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0,  9.5))*BlurB.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 10.5))*BlurB.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 11.5))*BlurB.r;
//
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 12.5))*BlurA.a;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 13.5))*BlurA.b;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 14.5))*BlurA.g;
//		blur +=	tex2D(PP_CommonTexture_Wrap,p.uv + d*float2(0.0, 15.5))*BlurA.r;*/
//
//	return blur;
//}
//
//float3 RadialBlurCenter;
//float  RadialBlurLinear;
//
//float3 RadialFocusDir;
//float  RadialFocusAsp;
//float  RadialFocusRad;
//
//float4 NativeRadialBlur_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float2 InTex = p.uv;
//
//	float4 tex1 = tex2D(PP_CommonTexture_Wrap,InTex);
//
//	float3 scale = float3(TexelSizeX,TexelSizeY,0.0);
//
////	float3 dir = scale*p.pos*MotionScale;
////	float3 dir = scale*float3(p.pos.x,-p.pos.y,0.0)*MotionScale;
//
////	float3 pos = float3(p.pos.x,-p.pos.y,0.0) - float3(RadialBlurCenter.x,-RadialBlurCenter.y,0.0);
//	float3 pos = float3(p.pos.x + 1.0f,1.0f - p.pos.y,0.0) - RadialBlurCenter;
//
////	if( RadialBlurLinear == 0.0 )
////		pos *= abs(pos);
//	pos *= lerp(abs(pos),1.0,RadialBlurLinear);
//
//	float3 dir =
//		scale*pos*MotionScale;
//
//	dir *= GlowBlurPass;
//	dir *= BlurFactor;
//
//	float k = dot(normalize(dir),RadialFocusDir);
//
///*	if( k >= 0.0 )
//	{
//		k = (1.0 - k*k)*0.5;
//	}
//	else
//	{
//	//	k = k*k*0.5 + 0.5;
//		k = -k*0.5 + 0.5;
//	}*/
//
//	k = k*0.5 + 0.5;
//
//	k = 1.0 - k*RadialFocusAsp;
//
//	k = min(1.0,k/RadialFocusRad);
//
//	dir *= k;
//
//	float4 tex2 = tex2D(PP_CommonTexture_Wrap,InTex - dir*4.0);
//	float4 tex3 = tex2D(PP_CommonTexture_Wrap,InTex - dir*3.0);
//	float4 tex4 = tex2D(PP_CommonTexture_Wrap,InTex - dir*2.0);
//	float4 tex5 = tex2D(PP_CommonTexture_Wrap,InTex - dir*1.0);
//	float4 tex6 = tex2D(PP_CommonTexture_Wrap,InTex + dir*1.0);
//	float4 tex7 = tex2D(PP_CommonTexture_Wrap,InTex + dir*2.0);
//	float4 tex8 = tex2D(PP_CommonTexture_Wrap,InTex + dir*3.0);
////	float4 tex9 = tex2D(PP_CommonTexture_Wrap,InTex + dir*4.0);
//
////	float4 blur = tex1*0.2 + (tex5 + tex6)*0.2 + (tex4 + tex7)*0.1 + (tex3 + tex8)*0.05 + (tex2 + tex9)*0.05;
//
//	float4 blur = tex1*0.2 + (tex5 + tex6)*0.2 + (tex4 + tex7)*0.1 + (tex3 + tex8)*0.05 + (tex2)*0.1;
////	float4 blur = (tex2 + tex3 + tex4 + tex5 + tex6 + tex7 + tex8)/7.0;
//
//	////
//
///*	float k = max(0.0,dot(normalize(dir),RadialFocusDir));
//
//	k = 1.0 - k*k*RadialFocusAsp;*/
///*	float k = dot(normalize(dir),RadialFocusDir);
//
////	if( k >= 0.0 )
////	{
////		k = (1.0 - k*k)*0.5;
////	}
////	else
////	{
////	//	k = k*k*0.5 + 0.5;
////		k = -k*0.5 + 0.5;
////	}
//
//	k = k*0.5 + 0.5;
//
//	k = 1.0 - k*RadialFocusAsp;
//
//	k = min(1.0,k/RadialFocusRad);*/
//
//	////
//
////	float4 res = lerp(tex1,blur,/*BlurFactor*/k);
//	float4 res = blur;
//
//	return res;
//}
//
//float4 ShowGlow_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 glow = tex2D(PP_CommonTexture_Wrap,p.uv);
////	float4 glow = tex2D(PP_CommonTexture_Prec,p.uv);
//
////	glow = glow - float4(0.5,0.5,0.5,0.5);
//	glow = glow - 0.5;
//
//	glow = glow*2.0;
//	glow = glow*glow*glow;
//
////	glow = glow*float4(0.5,0.5,0.5,0.5);
//	glow = glow*0.5;
//
////	glow = glow + float4(0.5,0.5,0.5,0.5);
//	glow = glow + 0.5;
//
//	float4 screen = tex2D(PP_ScreenTexture_Wrap,p.uv);
////	float4 screen = tex2D(PP_ScreenTexture_Prec,p.uv);
//
//	return screen + glow*GlowPower;
//}
//
//float4 ShowGlowPrec_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 glow = tex2D(PP_CommonTexture_Wrap,p.uv); //return glow;/*
////	float4 glow = tex2D(PP_CommonTexture_Prec,p.uv);
//
//	glow = length(glow.xyz)*0.57735;
//
////	glow = glow - float4(0.5,0.5,0.5,0.5);
//	glow = glow - 0.5;
//
//	glow = glow*2.0;
//	glow = glow*glow*glow;
//
////	glow = glow*float4(0.5,0.5,0.5,0.5);
//	glow = glow*0.5;
//
////	glow = glow + float4(0.5,0.5,0.5,0.5);
//	glow = glow + 0.5;
//
////	float4 screen = tex2D(PP_ScreenTexture_Wrap,p.uv);
//	float4 screen = tex2D(PP_ScreenTexture_Prec,p.uv);
//
//	return screen + glow*GlowPower;//*/
////	return glow*GlowPower;
////	return glow;
//}
//
//////
//
//Technique(BlurWithClampMix,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 BlurWithClampMix_PS();
//	}
//}
//
//////
//
//Technique(IterativeBlurWithClampH,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurWithClampH_PS();
//	}
//}
//
//Technique(IterativeBlurH,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurH_PS();
//	}
//}
//
//Technique(IterativeBlurV,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurV_PS();
//	}
//}
//
//Technique(CalculateDir,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 CalculateDir_PS();
//	}
//}
//
//Technique(IterativeBlurDir,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurDir_PS();
//	}
//}
//
//#ifndef _XBOX
//
//Technique(IterativeBlurDepthH,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurDepthH_PS();
//	}
//}
//
//Technique(IterativeBlurDepthV,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurDepthV_PS();
//	}
//}
//
//#else
///*
//Technique(IterativeBlurDepthH_BOX_First,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurDepthH_BOX_First_PS();
//	}
//}
//*/
//Technique(ResizeRectSimple,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 ResizeRectSimple_PS();
//	}
//}
//
//Technique(ResizeRect_BOX,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 ResizeRect_BOX_PS();
//	}
//}
//
//Technique(IterativeBlurDepthH_BOX,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurDepthH_BOX_PS();
//	}
//}
//
//Technique(IterativeBlurDepthV_BOX,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurDepthV_BOX_PS();
//	}
//}
///*
//Technique(IterativeBlurDepthV_BOX_Last,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 IterativeBlurDepthV_BOX_Last_PS();
//	}
//}
//*/
//#endif
///*
//#ifndef _XBOX
//
//Technique(IterativeRadialBlurH_First_Low,PP_FullScreenQuad_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//		AlphaBlendEnable = false;
//		AlphaTestEnable = false;
//
//		VertexShader = compile vs_2_0 IterativeRadialBlur_VS();
//		PixelShader = compile ps_2_0 IterativeRadialBlurH_First_Low_PS();
//	}
//}
//
//#endif
//*/
//Technique(IterativeRadialBlurH,PP_FullScreenQuad_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//		AlphaBlendEnable = false;
//		AlphaTestEnable = false;
//
//		VertexShader = compile vs_2_0 IterativeRadialBlur_VS();
//		PixelShader = compile ps_2_0 IterativeRadialBlurH_PS();
//	}
//}
//
//Technique(IterativeRadialBlurV,PP_FullScreenQuad_VSIN)
//{
//	pass P0
//	{
//		CullMode = None;
//
//		AlphaBlendEnable = false;
//		AlphaTestEnable = false;
//
//		VertexShader = compile vs_2_0 IterativeRadialBlur_VS();
//		PixelShader = compile ps_2_0 IterativeRadialBlurV_PS();
//	}
//}
//
//Technique(NativeRadialBlur,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 NativeRadialBlur_PS();
//	}
//}
//
//float4 RadialBlurPaste_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
////	float4 orig = tex2D(PP_ScreenTexture_Prec,p.uv);
//	float4 blur = tex2D(PP_CommonTexture_Wrap,p.uv);
//
///*	float r = length(float3(p.pos.x + 1.0f,1.0f - p.pos.y,0.0) - RadialBlurCenter);
//
//	if( RadialBlurLinear == 0.0 )
//		r *= r*0.5;
//	else
//		r *= 0.8;
//
//	float k = clamp(r*BlurFactor,0.0,1.0);
//
//	return lerp(orig,blur,k);*/
//
//	return blur;
//}
//
//Technique(RadialBlurPaste,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 RadialBlurPaste_PS();
//	}
//}
//
//float4 RadialBlurPaste_Low_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 orig = tex2D(PP_ScreenTexture_Prec,p.uv);
//	float4 blur = tex2D(PP_CommonTexture_Wrap,p.uv);
//
//	float3 dir = float3(p.pos.x + 1.0f,1.0f - p.pos.y,0.0) - RadialBlurCenter;
//
//	float r = length(dir);
//
///*	if( RadialBlurLinear == 0.0 )
//		r *= r*0.5;
//	else
//		r *= 0.8;*/
////	if( RadialBlurLinear == 0.0 )
////		r *= r;
//	r *= lerp(r,1.0,RadialBlurLinear);
//
//	r *= MotionScale*GlowBlurPass*BlurFactor;
//
////	float k = clamp(r*BlurFactor,0.0,1.0);
//	float k = r;
//
///*	if( k > 4.0 )
//		k = 4.0;
//	k *= 0.25f;*/
///*	const float t = 4.0*1.5;
//	if( k > t )
//		k = t;
//	k /= t;*/
//
///*	float m = max(0.0,dot(normalize(dir),RadialFocusDir));
//
//	m = 1.0 - m*m**RadialFocusAsp;*/
//	float m = dot(normalize(dir),RadialFocusDir);
//
//	m = m*0.5 + 0.5;
//
//	m = 1.0 - m*RadialFocusAsp;
//
//	m = min(1.0,m/RadialFocusRad);
//
//	k *= m;
//
//	const float t = 4.0*1.5*3.0;
//	if( k > t )
//		k = t;
//	k /= t;
//
////	return lerp(orig,blur,k*m);
//	return lerp(orig,blur,k*k);
//}
//
//Technique(RadialBlurPaste_Low,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 RadialBlurPaste_Low_PS();
//	}
//}
///*
//float4 BlurPaste_Low_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 orig = tex2D(PP_CommonTexture_Prec,p.uv);
//	float4 blur = tex2D(PP_ScreenTexture_Wrap,p.uv);
//
//	float k = clamp(BlurFactor,0.0,1.0);
//
//	return lerp(orig,blur,k);
//}
//
//Technique(BlurPaste_Low,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 BlurPaste_Low_PS();
//	}
//}
//*/
//float4 BlurPaste_Low_Rad_PS(PP_FullScreenQuad_PSIN p) : COLOR
//{
//	float4 orig = tex2D(PP_CommonTexture_Prec,p.uv);
//	float4 blur = tex2D(PP_ScreenTexture_Wrap,p.uv);
//
//	float r = length(p.pos.xy);
//
///*	float k = clamp(r*r*BlurFactor,0.0,1.0);
//
//	return lerp(orig,blur,k);*/
//
//	float k = InnerFactor + OuterFactor*r*r;
//
///*	k = clamp(k,0.0,1.0);
//
//	k = 1.0 - k;
//	k = k*k*k*k;
//	k = 1.0 - k;*/
//
//	float t = GlowBlurPass;
//
////	if( t%1.0 < 0.5 )
////		t -= 0.5;
//
//	k *= t*2.0;
//
//	if( k < 1.0 )
//	{
//	//	k = k*2.0;
//
//	//	k *= k;
//	}
//	else
//		k = 1.0;
//
//	return lerp(orig,blur,k);
//}
//
//Technique(BlurPaste_Low_Rad,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 BlurPaste_Low_Rad_PS();
//	}
//}
//
//Technique(ShowGlow,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 ShowGlow_PS();
//	}
//}
//
//Technique(ShowGlowPrec,PP_FullScreenQuad_VSIN)
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
//		PixelShader = compile ps_2_0 ShowGlowPrec_PS();
//	}
//}
