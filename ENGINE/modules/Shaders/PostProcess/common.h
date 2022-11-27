#ifndef POSTPROCESS_COMMON_HEADER
#define POSTPROCESS_COMMON_HEADER  

texture PP_CommonTexture;

sampler PP_CommonTexture_Wrap =
sampler_state
{
    Texture = <PP_CommonTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
////AddressU = Wrap;
////AddressV = Wrap;
	AddressU = Clamp;
	AddressV = Clamp;
};

sampler PP_CommonTexture_Prec =
sampler_state
{
    Texture = <PP_CommonTexture>;
    MinFilter = Point;
    MagFilter = Point;
////AddressU = Wrap;
////AddressV = Wrap;
	AddressU = Clamp;
	AddressV = Clamp;
};

/*
sampler PP_CommonTexture_Clamp =
sampler_state
{
    Texture = <PP_CommonTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = Clamp;
    AddressV = Clamp;
};
*/

texture PP_ScreenTexture;

sampler PP_ScreenTexture_Wrap =
sampler_state
{
    Texture = <PP_ScreenTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
////AddressU = Wrap;
////AddressV = Wrap;
	AddressU = Clamp;
	AddressV = Clamp;
};

sampler PP_ScreenTexture_Prec =
sampler_state
{
    Texture = <PP_ScreenTexture>;
    MinFilter = Point;
    MagFilter = Point;
////AddressU = Wrap;
////AddressV = Wrap;
	AddressU = Clamp;
	AddressV = Clamp;
};

/*
sampler PP_ScreenTexture_Clamp =
sampler_state
{
    Texture = <PP_ScreenTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = Clamp;
    AddressV = Clamp;
};
*/

struct PP_FullScreenQuad_VSIN
{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

struct PP_FullScreenQuad_VSOUT
{
	float4 p   : POSITION;
	float2 uv  : TEXCOORD0;
	float4 pos : TEXCOORD1;
};

struct PP_FullScreenQuad_PSIN
{
	float2 uv  : TEXCOORD0;
	float4 pos : TEXCOORD1;
};
/*
float2 clampUV(float2 uv)
{
	return clamp(uv,float2(0.0,0.0),float2(1.0,1.0));
}
*/
PP_FullScreenQuad_VSOUT PP_StdVertexShader(PP_FullScreenQuad_VSIN v)
{
	PP_FullScreenQuad_VSOUT res;

	res.p = v.pos;
	res.uv = v.uv;
	res.pos = v.pos;

	return res;
}

#endif
