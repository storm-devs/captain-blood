#include "technique.h"
#include "ocean\ocean.h"

struct OceanVSIn3
{
	float3 pos : POSITION;
	float2 normal : BLENDWEIGHT0;
};

struct OceanVSOut3
{
	float4 pos : POSITION;
	float4 normal : TEXCOORD0;		// x,y,z - normal, w = wave fade k
	float4 uv_fog : TEXCOORD1;		// x,y = uv, z = fogK, w = bump.z
	float4 cam2Pos : TEXCOORD2;		// xyz - cam2Pos, w - reflection k from distance
	float4 scr_uv : TEXCOORD3;		// xy = 0.0-1.0f, zw = refract viewport shrink
	float4 transDir : TEXCOORD4;	// xyz - dir, w - translucence coeff
	float4 wpos : TEXCOORD5;		// xyz - position, w - foam parameter
};

struct OceanPSIn3
{
	float4 normal : TEXCOORD0;		// x,y,z - normal, w = wave fade k
	float4 uv_fog : TEXCOORD1;		// x,y = uv, z = fogK
	float4 cam2Pos : TEXCOORD2;		// xyz - cam2Pos, w - reflection k from distance
	float4 scr_uv : TEXCOORD3;		// xy = 0.0-1.0f, zw = refract viewport shrink
	float4 transDir : TEXCOORD4;	// xyz - dir, w - translucence coeff
	float4 wpos : TEXCOORD5;		// xyz - position, w - foam parameter
};

OceanVSOut3 vs_Ocean3(OceanVSIn3 v)
{
	OceanVSOut3 o;
	
	float4 pos = float4(v.pos, 1.0f);
	//pos.x = v.pos.x;
	//pos.z = v.pos.y;
	//pos.w = 1.0f;
	//const float3 factor = float3(255.0f * 256.0f * 256.0f / 4096.0f, 255.0f * 256.0f / 4096.0f, 255.0f / 4096.0f);
	//pos.y = dot(v.color.rgb, factor) - 300.0f;
	
	float3 cam2Pos = vCamPos.xyz - pos.xyz;
	float camDist = dot(cam2Pos, cam2Pos);
	
	float2 fnrm = floor(v.normal * (1.0f / 2048.0f));
	float alpha = (fnrm.y * 8.0f + fnrm.x) * (1.0f / 63.0f);
	float2 vnorm = v.normal - fnrm * 2048.0f;

	float2 nrm = vnorm * (2.0f / 2048.0f) - 1.0f;// * 2.0f - 1.0f; 
	//float4 norms2 = nrm * nrm;
	float3 tmp;// = sqrt(float3(1.0f - (norms2.x + norms2.z), 1.0f - (norms2.y + norms2.w), camDist));
	tmp.z = sqrt(camDist);

	cam2Pos /= tmp.z;
	
	//
	float4 normal;
	normal.w = 1.0f;//max(0.01f, 1.0f - pow(saturate(tmp.z * ocean2DistFade), 2.0f));

	float fade = 1.0f - min(1.0f, pow(tmp.z * ocean2DistFade, 1.5f));
	float faden = 1.0f - min(1.0f, pow(tmp.z * ocean2DistFade, 1.0f));

	normal.xz = float2(nrm.x, nrm.y ) * faden;
	normal.y = sqrt(1.0f - (nrm.x * nrm.x + nrm.y * nrm.y));

	float2 pp = pos.xz - 0.25f * normal.xz * ocean2Shifts.xx * fade;

	o.uv_fog.xy = (pos.xz + ocean2BumpScaleMove.zw) * ocean2BumpScaleMove.xy;
	
	pos.y = pos.y * fade + ocean2FakeHeight.y;

	pos.xz -= normal.xz * ocean2Shifts.xx * fade;

	normal.xz *= ocean2BumpPower.z * fade;
	//normal.xz *= fade;
	normal.xyz = normalize(normal.xyz);

	o.pos = mul(pos, mWorldViewProj);
	o.scr_uv = o.pos;
	// FIX-ME 
	o.scr_uv.z = alpha;
	o.normal = normal;
	o.uv_fog.z = tmp.z;
	o.wpos.xyz = pos.xyz;
	o.wpos.w = fade * pow(saturate((pos.y - ocean2FoamParams1.x) * ocean2FoamParams1.y), ocean2FoamParams1.z);

	float3 transDir = normalize(float3(cam2Pos.x, 0.0f, cam2Pos.z));
	transDir = normalize(float3(transDir.x, ocean2TranslucenceAngle.x, transDir.z));
	o.transDir.xyz = transDir;
	o.transDir.w = 0.1f + 0.9f * pow(1.0f - cam2Pos.y * cam2Pos.y, 2.0f);

	//float3 kb = 1.0f - tmp.z * ocean2BumpDistance.xyz; 
	//kb.z = max(kb.z, ocean2BumpDistance.w);	// немного бампа оставляем на горизонте
	//kb = saturate(kb);
	o.uv_fog.w = 1.0f;//kb.z;
	o.cam2Pos.xyz = cam2Pos;
	o.cam2Pos.w = max(0.1f, 1.0f - tmp.z * 0.001f); 

	return o;
}

float4 ps_Ocean3(OceanPSIn3 p) : COLOR
{
	float3 bump, bump1;

	float4 tt = tex2D(Ocean2BumpSampler, p.uv_fog.xy);
	bump.xz = (tt.ba * 2.0f - 1.0f);
	bump.y = sqrt(1.0f - (bump.x * bump.x + bump.z * bump.z));// * (ocean2BumpPower.x + p.scr_uv.z * ocean2BumpPower.y);

	/*float4 tt1 = tex2D(Ocean2BumpSampler, p.uv_fog.xy * 0.125f);
	bump1.xz = (tt1.ar * 2.0f - 1.0f);
	bump1.y = sqrt(1.0f - (bump.x * bump.x + bump.z * bump.z));*/

	// получаем новую нормаль с учетом бампа
	float3 normal;
	normal = (bump/* + bump1 * 0.3f*/) * ocean2BumpPower.x + p.normal;
	//normal = bump * ocean2BumpPower.x + p.normal;
	normal.xz *= saturate(1.0f - ocean2BumpPower.y * p.scr_uv.z);
	normal = normalize(normal);

	float3 cam2Pos = normalize(vCamPos.xyz - p.wpos.xyz);

	// достаем френеля
	float doter = abs(dot(cam2Pos.xyz, normal));
	float k = pow(1.0f - doter, 6.0f);

	// рассчитываем коэфф. отражения и преломления
	// x = reflection, y = alpha
	float2 ra = saturate(lerp(ocean2ReflTrans.xz, ocean2ReflTrans.yw, min(1.0, k)));

	// достаем отражение неба
	float3 vReflect = reflect(cam2Pos.xyz, normal);
	float3 vReflect2D = normalize(float3(vReflect.x, 0.0f, vReflect.z));
	//float v = acos(dot(vReflect2D, vReflect)) / (3.14159265f / 2.0f);
	float v = sqrt(1.0f - dot(vReflect2D, vReflect));
	//float u = (atan2(vReflect2D.x, vReflect2D.z) + 3.14159265f) / (3.14159265f * 2.0f);
	float u = dot(vReflect2D, float3(1.0f, 0.0f, 0.0f));
	u = sqrt((1.0f - u) * 0.5f) * 0.5f;
	u = (vReflect2D.z >= 0.0f) ? u : 1.0f - u;
	float d = 1.0f / 32.0f;
	float3 reflTex = 1.0 * tex2D(Ocean2SphereMapSampler, float2(u, v));

	float2 ruv;
	ruv.xy = p.scr_uv.xy / p.scr_uv.w * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

	// достаем отражение окружения
	float4 reflEnvTex = tex2D(Ocean2ReflSampler, normal.xz * ocean2RefrParams.yy + ruv.xy);
	float tmpdot = dot(reflEnvTex.rgb, float3(0.3f, 0.59f, 0.11f));
	float3 addReflColor = reflEnvTex.rgb * saturate((tmpdot - ocean2RefrParams2.x) * ocean2RefrParams.z) * reflEnvTex.a * p.cam2Pos.w;
	float blendSkyObj = saturate(min(ocean2RefrParams2.y, reflEnvTex.a) * p.cam2Pos.w);
	reflTex = lerp(reflTex, ocean2RefrParams.w * reflEnvTex.rgb, blendSkyObj);

	float4 color;
	
	// считаем свет
	k = dot(normal, ocean2SunLight.xyz) * ocean2SunLightParams.x + ocean2SunLightParams.y;
	color.rgb = k * lerp(ocean2WaterColor.rgb, reflTex * ocean2SkyColor.rgb, ra.x);
	
	// достаем преломление
	float4 refrTex = tex2D(Ocean2RefrSampler, normal.xz * ocean2RefrParams.xx + ruv.xy);	// WAS: + ruv.zw
	float4 refrTex2 = tex2D(Ocean2RefrSampler, ruv.xy);
	refrTex.rgb = lerp(refrTex2, refrTex, refrTex.a);

	// прозрачность
	color.rgb = lerp(color, refrTex, saturate(ra.y + p.scr_uv.z) );

	// пена
	if (ocean2BoolParams[OC2_FOAM_ENABLE])
	{
		float2 fuv = p.wpos.xx * ocean2FoamRotate.xy + p.wpos.zz * ocean2FoamRotate.zw;
		color.rgb += p.wpos.w * ocean2FoamColor.rgb * tex2D(Ocean2FoamSampler, ocean2FoamParams2.zw + fuv - normal.xz * ocean2FoamParams2.y).rgb;
	}
	
	// транслюценция
	if (ocean2BoolParams[OC2_TRANSLUCENCE_ENABLE])
	{
		float dok = dot(normal, p.transDir.xyz);
		dok = saturate(ocean2TranslucenceParams.y * pow(dok, ocean2TranslucenceParams.x));
		color += ocean2TranslucenceColor * dok * p.transDir.w;
	}
	
	float fogFade = max(0.0f, (p.uv_fog.z - ocean2FogParams.z) * ocean2FogParams.y);
	float fogk = saturate(ocean2FogParams.x * (1.0f - exp(-fogFade)));

	// солнечная дорожка
	// После нее повреждаются vReflect и normal - так что ОСТОРОЖНО
	if (ocean2BoolParams[OC2_SUNROAD_ENABLE])
	{
		normal = normalize(normal * ocean2SunBumpScale.xyz);
		vReflect = reflect(-cam2Pos.xyz, normal);
		k = dot(vReflect, ocean2SunRoadParams.xyz);
		k = pow(saturate(k), ocean2SunRoadParams.a);
		color.rgb = color.rgb + ocean2SunRoadColor.rgb * k;
	}

	// добавляем немного отражения
	color.rgb += addReflColor;

	return float4(lerp(color.rgb, ocean2FogColor.rgb, fogk), 1.0f);
}

Technique(NewOcean3, OceanVSIn3)
{
	pass P0
	{
		CullMode = cw;
		ZEnable = true;
		ZWriteEnable = true;
	
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_Ocean3();
		PixelShader = compile ps_3_0 ps_Ocean3();
	}
}

Technique(NewWater3, OceanVSIn3)
{
	pass P0
	{
		CullMode = none;
		ZEnable = true;
		//ZWriteEnable = false;
	
		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		VertexShader = compile vs_3_0 vs_Ocean3();
		PixelShader = compile ps_3_0 ps_Ocean3();
	}
}

float4 ps_Ocean3Mirror(OceanPSIn3 p) : COLOR
{
	// достаем френеля
	float k = tex1D(Ocean2FrenelSampler, dot(p.cam2Pos.xyz, p.normal) * 0.5f).r;

	// рассчитываем коэфф. отражения и преломления
	// x = reflection, y = alpha
	float2 ra = saturate(lerp(ocean2ReflTrans.xz, ocean2ReflTrans.yw, k));

	float2 ruv;
	ruv.xy = p.scr_uv.xy / p.scr_uv.w * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

	// достаем отражение окружения
	float4 reflTex = tex2D(Ocean2ReflSampler, ruv);

	float fFogK = CalculateFogFactor(p.wpos.xyz);

	return ApplyFogColor(float4(reflTex.rgb * (ocean2SkyColor.rgb * ra.x), ra.y * saturate(1.0f - fFogK)), fFogK);
}

Technique(MirrorOcean3, OceanVSIn3)
{
	pass P0
	{
		CullMode = none;
		ZEnable = true;
		ZWriteEnable = true;

		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_Ocean3();
		PixelShader = compile ps_2_0 ps_Ocean3Mirror();
	}
}
