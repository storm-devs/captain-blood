#ifndef PARTICLES_VERTEX_SHADER
#define PARTICLES_VERTEX_SHADER


#ifdef _XBOX
	#define USE_4X_LESS_BUFFER (1)
#endif




#include "stdVariables.h"
#include "Particles\ParticlesCommon.h"
#include "Particles\ParticlesStruct.h"

#ifdef USE_4X_LESS_BUFFER
Particles_VS_OUT vs_Particles(int Index : INDEX)
#else
Particles_VS_OUT vs_Particles(Particles_VS_IN v)
#endif
{
        Particles_VS_OUT o;

#ifdef USE_4X_LESS_BUFFER
		int fetchIndex = Index / 4;
		int vIdx = Index - ( fetchIndex * 4 );

		float3 _vOffsetAndUVSize;
		float2 _UV1;
		float2 _UV2;
		float _Angle;
		float3 _vPosition;
		float4 _vColor;
		float _Blend;
		float _AddPowerK;
		float2 _LightPowerK_and_Ambient;

		asm
		{
			vfetch _vOffsetAndUVSize.xyz, fetchIndex, position;
			vfetch _vColor, fetchIndex, texcoord0;
			vfetch _UV1.xy, fetchIndex, texcoord1;
			vfetch _UV2.xy, fetchIndex, texcoord2;
			vfetch _Angle.x, fetchIndex, texcoord3;
			vfetch _Blend.x, fetchIndex, texcoord4;
			vfetch _vPosition.xyz, fetchIndex, texcoord5;
			vfetch _AddPowerK.x, fetchIndex, texcoord6;
			vfetch _LightPowerK_and_Ambient.xy, fetchIndex, texcoord7;
		};


		float4 offsetVector = float4( -1.0, -1.0, -1.0, 1.0);
		[flatten]
		if (vIdx == 1)
		{
			offsetVector = float4( -1.0,  1.0, -1.0, -1.0);
		} else
		{
			[flatten]
			if (vIdx == 2)
			{
				offsetVector = float4( 1.0,  1.0, 1.0, -1.0);
			} else
			{
				[flatten]
				if (vIdx == 3)
				{
					offsetVector = float4( 1.0, -1.0, 1.0,  1.0);
				}
			}
		}


		float2 _vOffset = _vOffsetAndUVSize.xx * offsetVector.xy;
		_UV1 += _vOffsetAndUVSize.y * offsetVector.zw;
		_UV2 += _vOffsetAndUVSize.z * offsetVector.zw;

#else

		float _Angle = v.Angle;
		float2 _vOffset = v.vOffset.xy;
		float3 _vPosition = v.vPosition;

		float2 _UV1 = v.UV1;
		float2 _UV2 = v.UV2;

		float4 _vColor = v.vColor;

		float _Blend = v.Blend;
		float _AddPowerK = v.AddPowerK;
		float2 _LightPowerK_and_Ambient = v.LightPowerK_and_Ambient;

#endif


        float vSin;
        float vCos;
        sincos(_Angle, vSin, vCos);

        float2 ViewOffset;
        ViewOffset.x = (_vOffset.x * vCos) + (_vOffset.y * vSin);
        ViewOffset.y = ((_vOffset.y * vCos) - (_vOffset.x * vSin));

        float4 ViewSpace = mul(float4(_vPosition, 1.0), mView);
        ViewSpace.xy = ViewSpace.xy + ViewOffset;
        o.Position = mul(ViewSpace, mProjection);

        o.PositionShader = mul( ViewSpace, mProjection);

        //   fBlend; // К - блендинга между кадрами
        //   AddPowerK; // K - силы выжигания

        o.UV1 = _UV1;
        o.UV2 = _UV2;
        o.Color = _vColor;

        o.Params.x = _Blend;
        o.Params.y = _AddPowerK;
        o.Params.z = _LightPowerK_and_Ambient.x;
        o.Params.w = _LightPowerK_and_Ambient.y;


        float3 Light;
        Light.x = (LightDirProjection.x * vCos) + (LightDirProjection.y * vSin);
        Light.y = ((LightDirProjection.y * vCos) - (LightDirProjection.x * vSin));
        Light.z = LightDirProjection.z;
        o.LightDir = normalize(Light);

        return o;

}



#ifdef USE_4X_LESS_BUFFER
Particles_VS_OUT vs_ParticlesXZ(int Index : INDEX)
#else
Particles_VS_OUT vs_ParticlesXZ(Particles_VS_IN v)
#endif
{
        Particles_VS_OUT o;

#ifdef USE_4X_LESS_BUFFER
		int fetchIndex = Index / 4;
		int vIdx = Index - ( fetchIndex * 4 );

		float3 _vOffsetAndUVSize;
		float2 _UV1;
		float2 _UV2;
		float _Angle;
		float3 _vPosition;
		float4 _vColor;
		float _Blend;
		float _AddPowerK;
		float2 _LightPowerK_and_Ambient;

		asm
		{
			vfetch _vOffsetAndUVSize.xyz, fetchIndex, position;
			vfetch _vColor, fetchIndex, texcoord0;
			vfetch _UV1.xy, fetchIndex, texcoord1;
			vfetch _UV2.xy, fetchIndex, texcoord2;
			vfetch _Angle.x, fetchIndex, texcoord3;
			vfetch _Blend.x, fetchIndex, texcoord4;
			vfetch _vPosition.xyz, fetchIndex, texcoord5;
			vfetch _AddPowerK.x, fetchIndex, texcoord6;
			vfetch _LightPowerK_and_Ambient.xy, fetchIndex, texcoord7;
		};


		float4 offsetVector = float4( -1.0, -1.0, -1.0, 1.0);
		[flatten]
		if (vIdx == 1)
		{
			offsetVector = float4( -1.0,  1.0, -1.0, -1.0);
		} else
		{
			[flatten]
			if (vIdx == 2)
			{
				offsetVector = float4( 1.0,  1.0, 1.0, -1.0);
			} else
			{
				[flatten]
				if (vIdx == 3)
				{
					offsetVector = float4( 1.0, -1.0, 1.0,  1.0);
				}
			}
		}


		float2 _vOffset = _vOffsetAndUVSize.xx * offsetVector.xy;
		_UV1 += _vOffsetAndUVSize.y * offsetVector.zw;
		_UV2 += _vOffsetAndUVSize.z * offsetVector.zw;

#else

		float _Angle = v.Angle;
		float2 _vOffset = v.vOffset.xy;
		float3 _vPosition = v.vPosition;

		float2 _UV1 = v.UV1;
		float2 _UV2 = v.UV2;

		float4 _vColor = v.vColor;

		float _Blend = v.Blend;
		float _AddPowerK = v.AddPowerK;
		float2 _LightPowerK_and_Ambient = v.LightPowerK_and_Ambient;

#endif


        float vSin;
        float vCos;
        sincos(_Angle, vSin, vCos);

        float3 ViewOffset;
        ViewOffset.x = (_vOffset.x * vCos) + (_vOffset.y * vSin);
        ViewOffset.y = 0.0;
        ViewOffset.z = ((_vOffset.y * vCos) - (_vOffset.x * vSin));

        float3 vPosition = _vPosition + ViewOffset;

        o.Position = mul(float4(vPosition, 1.0), mWorldViewProj);
        o.PositionShader = mul(float4(vPosition, 1.0), mWorldViewProj);

        //   fBlend; // К - блендинга между кадрами
        //   AddPowerK; // K - силы выжигания


        o.UV1 = _UV1;
        o.UV2 = _UV2;
        o.Color = _vColor;

        o.Params.x = _Blend;
        o.Params.y = _AddPowerK;
        o.Params.z = _LightPowerK_and_Ambient.x;
        o.Params.w = _LightPowerK_and_Ambient.y;


        float3 Light;
        Light.x = (LightDirProjection.x * vCos) + (LightDirProjection.y * vSin);
        Light.y = ((LightDirProjection.y * vCos) - (LightDirProjection.x * vSin));
        Light.z = LightDirProjection.z;

        o.LightDir = normalize(Light);

        return o;
}




#endif