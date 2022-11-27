#ifndef PARTICLES_STRUCTS
#define PARTICLES_STRUCTS




struct Particles_VS_IN
{
		float3 vOffset : POSITION;
        float4 vColor : TEXCOORD0;
        float2 UV1 : TEXCOORD1;
        float2 UV2 : TEXCOORD2;
        float Angle : TEXCOORD3;
        float Blend : TEXCOORD4;
        float3 vPosition : TEXCOORD5;
        float AddPowerK : TEXCOORD6;
        float2 LightPowerK_and_Ambient : TEXCOORD7;
};




struct Particles_VS_OUT
{
        float4 Position: POSITION0;
        float4 Color : TEXCOORD0;
        float2 UV1 : TEXCOORD1;
        float2 UV2 : TEXCOORD2;
        float4 Params : TEXCOORD3;
        float3 LightDir : TEXCOORD4;
        float4 PositionShader: TEXCOORD5;
};


struct Particles_PS_IN
{
        float4 DiffuseColor : TEXCOORD0;
        float2 UV1 : TEXCOORD1;
        float2 UV2 : TEXCOORD2;
        float4 inParams : TEXCOORD3;
        float3 LightDir : TEXCOORD4;
        float4 PositionShader: TEXCOORD5;
};



#endif