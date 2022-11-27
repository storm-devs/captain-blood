#ifndef GUICOMMON
#define GUICOMMON


texture GUITex;
sampler GUISampler =
sampler_state
{
    Texture = <GUITex>;
    MinFilter = Point;
    MagFilter = Point;
};





float4x4 GUI_CliperMatrix;
float4 GUI_LinesColor;


struct GUIVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
};


struct GUI_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
};

struct GUI_PS_INPUT
{
 float4 color : COLOR0;
};







struct GUISpriteVertex
{
 float3 pos : POSITION;
 float4 color : COLOR;
 float2 uv : TEXCOORD0;
};

struct GUISPR_VS_OUTPUT
{
 float4 pos: POSITION;
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
};

struct GUISPR_PS_INPUT
{
 float4 color : COLOR0;
 float2 uv : TEXCOORD0;
};



#endif