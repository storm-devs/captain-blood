#include "stdVariables.h"

float4 Phys_v4Color;

struct PhysPolyVertexIn
{
	float3 pos : POSITION;
};

struct PhysPolyVertexOut
{
	float4 pos: POSITION;
    float1 tc1 : TEXCOORD1;
};

struct PhysPolyPSINPUT
{
    float1 tc1 : TEXCOORD1;
};

PhysPolyVertexOut vs_PhysPoly(PhysPolyVertexIn _in)
{
	PhysPolyVertexOut _out;
	_out.pos = mul(float4(_in.pos, 1), mWorldViewProj);
	_out.pos.z = _out.pos.z - 0.001;
	_out.tc1.x = 1.0;
	if (_out.pos.z < 0.01)
	{
		_out.tc1.x = saturate(0.1 + _out.pos.z / 0.01);
	}
	return _out;
}

float4 ps_PhysPoly(PhysPolyPSINPUT inp) : COLOR
{
	return float4(Phys_v4Color.rgb, inp.tc1.x);
}

struct pLineVertex
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct pLine_VS_OUTPUT
{
	float4 pos: POSITION;
	float4 color : COLOR0;
};

struct pLine_PS_INPUT
{
	float4 color : COLOR0;
};

pLine_VS_OUTPUT vs_pLine( pLineVertex vrx)
{
	pLine_VS_OUTPUT res;
	res.pos = mul(float4(vrx.pos, 1), mWorldViewProj);
	res.pos.z = res.pos.z - 0.002;
	res.color = vrx.color;
	return res;
}

float4 ps_pLine(pLine_PS_INPUT pnt) : COLOR
{
	return pnt.color;
}

Technique(dbgPhysPolygons, PhysPolyVertexIn)
{
	pass P0
	{
		ZWriteEnable = false;
		AlphaBlendEnable = false;
		SrcBlend = srcalpha;
		DestBlend = one;
		
		VertexShader = compile vs_2_0 vs_PhysPoly();
		PixelShader = compile ps_2_0 ps_PhysPoly();
	}
}

Technique(dbgPhysLine, pLineVertex)
{
 pass P0
  {
    CullMode = none;
    AlphaBlendEnable = true;
    VertexShader = compile vs_2_0 vs_pLine();
    PixelShader = compile ps_2_0 ps_pLine();
  }
}
