#include "technique.h"
#include "stdVariables.h"
#include "gmxService\gmx_Variables.h"
#include "shadows_pssm\pssm_vars.h"

float4  grass2Temp[4];			// tex_shift_x, tex_shift_y
float4	grass2TexPatches[4];	// x, y, width, height
float4	grass2PatchesParams[4];	// width multiplier, height multiplier, lodx ^ 2, 
float4	grass2Color;
float4	grass2Angles;			// x - start angle, y - angles range(0..1)
float4	grass2Distances;		// x - distance^2 to start transparency, y - (1.0 / distance^2) of transparency part
float4	grass2Player;			// x,y,z - player pos, w - power

float4	grass2WindParams;	// x, y - wind dir, z - windpower, 0
float4	grass2WindParams2;	// x - color ambient, y - color coeff for wind


// 70 blocks, 2 float4 per block
// pos.x, pos.y, pos.z, w
// scale.x, scale.y, scale.z, w
// first 4 .w - wind power multiplicator, all others .w - 21 2*float3(vx, vz) matrices
float4	grass2PosVectors[68 * 2];

texture grass2ShadowTexture;
sampler grass2ShadowSampler =
sampler_state
{
        Texture = <grass2ShadowTexture>;
        MinFilter = Linear;
        MagFilter = Linear;

        AddressU = Clamp;
        AddressV = Clamp;
};

texture grass2Texture;
sampler grass2Sampler =
sampler_state
{
	Texture = <grass2Texture>;

#ifdef _XBOX
	TrilinearThreshold = THREEEIGHTHS;
#endif

	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#ifdef _XBOX
	struct GVS_S0
	{
		float4 pos : COLOR0;
		float4 color : COLOR1;
	};

	struct GVS_S1
	{
		float4	grass2TexPatches : TEXCOORD0;		// x, y, width, height
		float3  grass2Temp : TEXCOORD1;				// tex_shift_x, tex_shift_y, wind_power
		float2	grass2PatchesParams : TEXCOORD2;	// width multiplier, height multiplier
		float2	grass2Lod : TEXCOORD3;				// x, y
	};

	struct GVS_IN
	{
		GVS_S0 s0;
		GVS_S1 s1;
	};
#else
	struct GVS_IN
	{
		float4 pos : COLOR0;
		float4 color : COLOR1;
	};
#endif

struct GVS_OUT2
{
	float4 pos : POSITION;
	float4 TexCoord0 : TEXCOORD0;
	float4 color : TEXCOORD1;
};

struct GVS_OUT
{
	GVS_OUT2 out2;
	//float4 pos : POSITION;
	//float4 TexCoord0 : TEXCOORD0;
	//float4 color : TEXCOORD1;
	float4 ScreenPos : TEXCOORD2;
};

struct GPS_IN
{
	float4 TexCoord0 : TEXCOORD0;
	float4 color : TEXCOORD1;
	float4 ScreenPos : TEXCOORD2;
};

struct GPS_IN2
{
	float4 TexCoord0 : TEXCOORD0;
	float4 color : TEXCOORD1;
};

void CalcPointLight2 (int index, float3 TexelWorldPos, in out float3 DiffuseColor)
{
	float3 d = vPointLightPos30[index] - TexelWorldPos;
	float Attenuation = saturate(1.0 - dot (d, d) * fPointLightParams[index].x); 
	float3 lightColor = vPointLightColor30[index].rgb * max(0.0f, fPointLightParams[index].w);
	DiffuseColor.rgb += saturate(Attenuation * lightColor);
}

void CalculatePointLights2 (float3 vWorldPos, in out float3 DiffuseColor)
{
	//------- calculate lighting -----------------
	if (bLightEnabled30[0])
	{
		CalcPointLight2(0, vWorldPos, DiffuseColor);

		if (bLightEnabled30[1])
		{
			CalcPointLight2(1, vWorldPos, DiffuseColor);

			if (bLightEnabled30[2])
			{
				CalcPointLight2(2, vWorldPos, DiffuseColor);

				if (bLightEnabled30[3])
				{
					CalcPointLight2(3, vWorldPos, DiffuseColor);

					if (bLightEnabled30[4])
					{
						CalcPointLight2(4, vWorldPos, DiffuseColor);

						if (bLightEnabled30[5])
						{
							CalcPointLight2(5, vWorldPos, DiffuseColor);

							if (bLightEnabled30[6])
							{
								CalcPointLight2(6, vWorldPos, DiffuseColor);

								if (bLightEnabled30[7])
								{
									CalcPointLight2(7, vWorldPos, DiffuseColor);
								}
							}
						}
					}
				}
			}
		}
	}


	if (bLightEnabled30[8])
	{
		CalcPointLight2(8, vWorldPos, DiffuseColor);

		if (bLightEnabled30[9])
		{
			CalcPointLight2(9, vWorldPos, DiffuseColor);

			if (bLightEnabled30[10])
			{
				CalcPointLight2(10, vWorldPos, DiffuseColor);

				if (bLightEnabled30[11])
				{
					CalcPointLight2(11, vWorldPos, DiffuseColor);

					if (bLightEnabled30[12])
					{
						CalcPointLight2(12, vWorldPos, DiffuseColor);

						if (bLightEnabled30[13])
						{
							CalcPointLight2(13, vWorldPos, DiffuseColor);

							if (bLightEnabled30[14])
							{
								CalcPointLight2(14, vWorldPos, DiffuseColor);

								if (bLightEnabled30[15])
								{
									CalcPointLight2(15, vWorldPos, DiffuseColor);
								}
							}
						}
					}
				}
			}
		}
	}
}

void GrassVsBody(out GVS_OUT2 res, out float4 pos, in int vertex, in float4 Inpos, in float4 Incolor, uniform bool bLightEnable, uniform bool bShadowRecv)
{
	int BoneIndexes[4] = (int[4])D3DCOLORtoUBYTE4( Incolor );

	float4 block_pos = grass2PosVectors[2 * BoneIndexes[1] + 0];
	float4 block_size = grass2PosVectors[2 * BoneIndexes[1] + 1];

	pos = float4(Inpos.xyz * block_size + block_pos, 1.0f);

	static const float grassSize = grass2Temp[1].w;

	//float texAndSlope = BoneIndexes[2];
	//float slope = floor(texAndSlope * 0.25f); /* typeAndSlope / 4 */
	//float curtex = texAndSlope;//floor(texAndSlope - slope * 4);
	float lod = floor(BoneIndexes[0] * 0.25f);
	//float lod = 0;
#ifndef _XBOX
	vertex = floor(BoneIndexes[0] - lod * 4);
	//vertex = BoneIndexes[0];
#endif

#ifdef _XBOX
	float4 tex_patch;
	float2 patchesParams;
	float windPowerK;
	float2 rot;
	float2 lodParams;
	float3 tmp;
    asm
    {
       vfetch tex_patch, BoneIndexes[2], texcoord0, UseTextureCache = true;
       vfetch tmp.xyz, BoneIndexes[2], texcoord1, UseTextureCache = true;
       vfetch rot.xy, vertex, texcoord2, UseTextureCache = true;
       vfetch lodParams.xy, lod, texcoord3, UseTextureCache = true;
	};
	
	patchesParams.xy = tmp.xy;
	windPowerK = tmp.z;

#else
	float windPowerK = grass2PosVectors[ BoneIndexes[2] ].w;
	float4 tex_patch = grass2TexPatches[ BoneIndexes[2] ];
	float2 patchesParams = grass2PatchesParams[ BoneIndexes[2] ].xy;

	float2 rot = grass2Temp[ vertex ].xy;

	float2 lodParams = grass2PatchesParams[lod].zw;
#endif

	/* зависимость силы ветра от типа травинки */
	float wind_power_multiplicator = windPowerK * grass2WindParams.z;

	/* угол поворота этой травы, в Incolor.a 0..1 значение */
	float3 vsincos;
	sincos(grass2Angles.x + grass2Angles.y * Incolor.a * 3.14159f * 2.5f, vsincos.x, vsincos.z);
	vsincos.y = 1.0f;

	float kwave = sin(grass2Temp[0].w * (0.45f + Inpos.w) + Incolor.a * 3.14150f * 2.0f);
	float windPower = kwave * wind_power_multiplicator * (2.0f + vsincos.x + vsincos.z);
	float3 windDir = rot.y * windPower * float3(vsincos.x, 0.0f, vsincos.z);

	/* рандомный размер травы, rand = 0..1 в Inpos.w */
	/* grass2Temp[ curtex ].z - размер данного типа травы в Texture Patches */
	float scale = max(0.02f, grassSize + grass2Temp[2].w * (Inpos.w * 2.0f - 1.0f));

	/* отгибаем от плеера */
	/* FIX-ME убрать sqrt и divide */
	float3 plr2pos = (grass2Player.xyz  - pos.xyz);
	float len = sqrt(dot(plr2pos.xz, plr2pos.xz));
	plr2pos /= len;
	float power = 1.0f - saturate(len *  0.45f);
	pos.xz -= plr2pos.xz * rot.y * scale * power * grass2Player.w;

	/* отгибаем от ветра */						
	float3 vec;
	vec = scale * patchesParams.xyx * (tex_patch.zwz * 2.0f * rot.xyx * vsincos.xyz);

	pos.xyz += vec + scale * windDir.xyz;

	float4 vPos = mul( pos, mWorldViewProj);

	res.pos = vPos;								

	/* текстурные координаты */
	res.TexCoord0.xy = tex_patch.xy + (rot.xy + float2(0.5f, 0.0f)) * tex_patch.zw;
	res.TexCoord0.zw = 0.0f;

	if (bShadowRecv)
	{
		res.color.x = vPos.z * 0.01f;
		res.color.y = fPSSMShadowPow;
		res.color.zw = 0.0f;
	}
	else
	{
		/* считаем цвет травы от источников света */
		float3 outcolor;
		if (bLightEnable)
		{
			outcolor = float3(0.0f, 0.0f, 0.0f);
			CalculatePointLights2(pos.xyz, outcolor);
		}
		else
		{
			outcolor = float3(1.0f, 1.0f, 1.0f);
		}

		/* затемнение цвета травы от симуляции ветра */
		float angc = saturate(grass2WindParams2.x - grass2WindParams2.y * windPower);
		res.color.xyz = (outcolor + vGlobalLightColor.xyz) * vAmbientLight.xyz * grass2Color.xyz * angc;
													
		/* вычисляем альфу от расстояния */
		float d = dot(vCamPos.xyz - pos.xyz, vCamPos.xyz - pos.xyz);
		float klod = saturate((lodParams.x - sqrt(d)) * lodParams.y);
		res.color.w = klod * saturate(1.0 - max(0.0f, d - grass2Distances.x) * grass2Distances.y);
		res.TexCoord0.z = CalculateFogFactor(pos);
	}
}

#ifdef _XBOX
GVS_OUT Grass2VShader(int Index : INDEX, uniform bool bLightEnable)
#else
GVS_OUT Grass2VShader(GVS_IN In, uniform bool bLightEnable)
#endif
{
	GVS_OUT res;
	float4 Inpos, Incolor;
	int vertex = 0;

#ifdef _XBOX
    int fetchIndex = Index / 4;

	vertex = Index - ( fetchIndex * 4 );

    asm
    {
       vfetch Inpos, fetchIndex, color0;
       vfetch Incolor, fetchIndex, color1;
	};
#else
	Inpos = In.pos;
	Incolor = In.color;
#endif

	float4 pos;
	GrassVsBody(res.out2, pos, vertex, Inpos, Incolor, bLightEnable, false);

	// координаты для шадоумапа
	res.ScreenPos = mul(float4(pos.xyz, 1.0), GMX_ShadowMatrix);
	
	return res;
}

#ifdef _XBOX
GVS_OUT2 Grass2VShaderNoShadow(int Index : INDEX, uniform bool bLightEnable, uniform bool bShadowRecv)
#else
GVS_OUT2 Grass2VShaderNoShadow(GVS_IN In, uniform bool bLightEnable, uniform bool bShadowRecv)
#endif
{
	GVS_OUT2 res;
	float4 Inpos, Incolor;
	int vertex = 0;

#ifdef _XBOX
    int fetchIndex = Index / 4;

	vertex = Index - ( fetchIndex * 4 );

    asm
    {
       vfetch Inpos, fetchIndex, color0;
       vfetch Incolor, fetchIndex, color1;
	};
#else
	Inpos = In.pos;
	Incolor = In.color;
#endif

	float4 pos;
	if (bShadowRecv)
	{
		GrassVsBody(res, pos, vertex, Inpos, Incolor, false, true);
	}
	else
	{
		GrassVsBody(res, pos, vertex, Inpos, Incolor, bLightEnable, false);
	}

	return res;
}

float4 Grass2PSShadowRecv(GPS_IN2 In) : COLOR
{
	float4 tex = tex2D(grass2Sampler, In.TexCoord0.xy);
	clip(tex.a - 0.5f);

	return float4(In.color.x, In.color.y, 0.0f, 1.0f);
}

float4 Grass2PShader(GPS_IN In) : COLOR
{
	float4 tex = tex2D(grass2Sampler, In.TexCoord0.xy);
	clip(tex.a - 0.5f);
	float4 res = tex * In.color;
	//clip(res.a - 0.001f);

	float4 shadow = tex2Dproj(grass2ShadowSampler, In.ScreenPos);
	res.rgb *= shadow.rgb;

	return ApplyFogColor(res, In.TexCoord0.z);
}

float4 Grass2PShaderNoShadow(GPS_IN2 In) : COLOR
{
	float4 tex = tex2D(grass2Sampler, In.TexCoord0.xy);
	clip(tex.a - 0.5f);
	float4 res = tex * In.color;
	//clip(res.a - 0.001f);

	return ApplyFogColor(res, In.TexCoord0.z);
}

Technique(Grass2, GVS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_3_0 Grass2VShader(false);
		PixelShader = compile ps_3_0 Grass2PShader();
	}
}

Technique(LIGHT_Grass2, GVS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_3_0 Grass2VShader(true);
		PixelShader = compile ps_3_0 Grass2PShader();
	}
}

Technique(Grass2NoShadow, GVS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_3_0 Grass2VShaderNoShadow(false, false);
		PixelShader = compile ps_3_0 Grass2PShaderNoShadow();
	}
}

Technique(LIGHT_Grass2NoShadow, GVS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_3_0 Grass2VShaderNoShadow(true, false);
		PixelShader = compile ps_3_0 Grass2PShaderNoShadow();
	}
}

Technique(Grass2ShadowRecv, GVS_IN)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		
		VertexShader = compile vs_3_0 Grass2VShaderNoShadow(false, true);
		PixelShader = compile ps_3_0 Grass2PSShadowRecv();
	}
}
