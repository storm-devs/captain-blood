#ifndef _XBOX

#include "technique.h"
#include "stdVariables.h"

struct RagdollVertex
{
	float3 pos : POSITION;
	float4 col : COLOR;
};

struct Ragdoll_VS_OUTPUT
{
	float4 pos : POSITION;
	float4 col : COLOR0;
};

struct Ragdoll_PS_INPUT
{
	float4 col: COLOR0;
};

Ragdoll_VS_OUTPUT vs_RagdollPolygon(RagdollVertex vrx)
{
	Ragdoll_VS_OUTPUT res;

	res.pos = mul(float4(vrx.pos,1),mWorldViewProj);
	res.col = vrx.col;

	return res;
}

float4 ps_RagdollPolygon(Ragdoll_PS_INPUT pnt) : COLOR
{
	return pnt.col;
}

Technique(RagdollPolygon, RagdollVertex)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		VertexShader = compile vs_2_0 vs_RagdollPolygon();
		PixelShader = compile ps_2_0 ps_RagdollPolygon();
	}
}

struct Sector_VS_OUTPUT
{
	float4 pos : POSITION;
	float2 tuv : TEXCOORD;
	float4 col : COLOR0;
};

struct Sector_PS_INPUT
{
	float2 tuv : TEXCOORD;
	float4 col : COLOR0;
};

Sector_VS_OUTPUT vs_RagdollSector(RagdollVertex vrx)
{
	Sector_VS_OUTPUT res;

	res.pos = float4(vrx.pos,1);
	res.col = vrx.col;

	res.tuv.x = 10*fmod(vrx.pos.z,0.2);
	res.tuv.y = 10* max(0.0,min(0.1,vrx.pos.z - 0.1));

	return res;
}

float4 SectorRange;

float4 ps_RagdollSector(Sector_PS_INPUT pnt) : COLOR
{
	float x = 2*(pnt.tuv.x - 0.5);
	float y = 2*(0.5 - pnt.tuv.y);

	float k = atan2(x,y);

	if( k < 0 )
		k += 3.1415926*2;

	float beg = SectorRange.x;
	float end = SectorRange.y;

	float f = 1;

	if( beg < end )
	{
		if( k < beg ) f = -1;
		if( k > end ) f = -1;

		if( end - beg < 0.01 )
			f = -1;
	}
	else
	{
		if( k > end && k < beg ) f = -1;

		if( beg - end < 0.01 )
			f = -1;
	}

	clip(f);

	x *= 1.03;
	y *= 1.03;

	float a = 0;

//	float d = 0.0370;
	float d = 0.0185;

	x += d; y -= d;

	a += floor(x*x + y*y);

	x -= d; y -= d;

	a += floor(x*x + y*y); y += d + d;
	a += floor(x*x + y*y); x += d + d;
	a += floor(x*x + y*y); y -= d + d;
	a += floor(x*x + y*y);

	float4 col = pnt.col;

	col.a = 1 - 0.2*a;

	return col;
}

Technique(RagdollSector, RagdollVertex)
{
	pass P0
	{
		CullMode = none;
		AlphaBlendEnable = true;
		VertexShader = compile vs_2_0 vs_RagdollSector();
		PixelShader = compile ps_3_0 ps_RagdollSector();
	}
}

#endif
