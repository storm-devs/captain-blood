
#include "ChangeList.h"
#include "..\..\Render.h"
#include "..\..\Defaultstates\DefaultStates.h"



FXChangeList::FXChangeList(array<RenderState>& changedStates, array<dword>& changedWraps) : m_changedStates(changedStates),
                                                                                            m_changedWraps(changedWraps)
{
	m_lRef = 1;
}

FXChangeList::~FXChangeList()
{
}


/*
bool FXChangeList::GetDefaultValue (D3DRENDERSTATETYPE state, DWORD &defaultValue)
{
	float fFogStart = 0.0f;
	float fFogEnd = 1.0f;
	float fFogDensity = 1.0f;

	float fPointScaleA = 1.0;
	float fPointScaleB = 1.0;
	float fPointScaleC = 1.0;

	float TweenFactor = 0.0f;

	float MinTesselationLevel = 1.0f;
	float MaxTesselationLevel = 1.0f;
	float AdaptiveTessX = 0.0f;
	float AdaptiveTessY = 0.0f;
	float AdaptiveTessZ = 1.0f;
	float AdaptiveTessW = 0.0f;

	float fPointSize = 64.0;
	float fPointSizeMin = 1.0;
	float fPointSizeMax = 64.0;

	switch(state)
	{
#ifndef _XBOX


				case D3DRS_SHADEMODE:
					defaultValue = D3DSHADE_GOURAUD;
					break;
				case D3DRS_DITHERENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_FOGENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_FOGCOLOR:
					defaultValue = 0;
					break;
				case D3DRS_SPECULARENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_FOGTABLEMODE:
					defaultValue = D3DFOG_NONE;
					break;
				case D3DRS_FOGSTART:
					defaultValue = *((DWORD*) (&fFogStart));
					break;
				case D3DRS_FOGEND:
					defaultValue = *((DWORD*) (&fFogEnd));
					break;
				case D3DRS_FOGDENSITY:
					defaultValue = *((DWORD*) (&fFogDensity));
					break;
				case D3DRS_RANGEFOGENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_TEXTUREFACTOR:
					defaultValue = 0xFFFFFFFF;
					break;
				case D3DRS_CLIPPING:
					defaultValue = TRUE;
					break;
				case D3DRS_LIGHTING:
					defaultValue = FALSE;
					break;
				case D3DRS_AMBIENT:
					defaultValue = 0;
					break;
				case D3DRS_FOGVERTEXMODE:
					defaultValue = D3DFOG_NONE;
					break;
				case D3DRS_COLORVERTEX:
					defaultValue = FALSE;
					break;
				case D3DRS_LOCALVIEWER:
					defaultValue = TRUE;
					break;
				case D3DRS_NORMALIZENORMALS:
					defaultValue = FALSE;
					break;
				case D3DRS_DIFFUSEMATERIALSOURCE:
					defaultValue = D3DMCS_COLOR1;
					break;
				case D3DRS_SPECULARMATERIALSOURCE:
					defaultValue = D3DMCS_COLOR2;
					break;
				case D3DRS_AMBIENTMATERIALSOURCE:
					defaultValue = D3DMCS_MATERIAL;
					break;
				case D3DRS_EMISSIVEMATERIALSOURCE:
					defaultValue = D3DMCS_MATERIAL;
					break;
				case D3DRS_VERTEXBLEND:
					defaultValue = D3DVBF_DISABLE;
					break;
				case D3DRS_POINTSCALEENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_POINTSCALE_A:
					defaultValue = *((DWORD*)&fPointScaleA);
					break;
				case D3DRS_POINTSCALE_B:
					defaultValue = *((DWORD*)&fPointScaleB);
					break;
				case D3DRS_POINTSCALE_C:
					defaultValue = *((DWORD*)&fPointScaleC);
					break;
				case D3DRS_PATCHEDGESTYLE:
					defaultValue = D3DPATCHEDGE_DISCRETE;
					break;
				case D3DRS_DEBUGMONITORTOKEN:
					defaultValue = D3DDMT_ENABLE;
					break;
				case D3DRS_INDEXEDVERTEXBLENDENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_TWEENFACTOR:
					defaultValue = *((DWORD*)&TweenFactor);
					break;
				case D3DRS_POSITIONDEGREE:
					defaultValue = D3DDEGREE_CUBIC;
					break;
				case D3DRS_NORMALDEGREE:
					defaultValue = D3DDEGREE_LINEAR;
					break;
				case D3DRS_ANTIALIASEDLINEENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_ADAPTIVETESS_X:
					defaultValue = *((DWORD*)&AdaptiveTessX);
					break;
				case D3DRS_ADAPTIVETESS_Y:
					defaultValue = *((DWORD*)&AdaptiveTessY);
					break;
				case D3DRS_ADAPTIVETESS_Z:
					defaultValue = *((DWORD*)&AdaptiveTessZ);
					break;
				case D3DRS_ADAPTIVETESS_W:
					defaultValue = *((DWORD*)&AdaptiveTessW);
					break;
				case D3DRS_ENABLEADAPTIVETESSELLATION:
					defaultValue = FALSE;
					break;
				case D3DRS_SRGBWRITEENABLE:
					defaultValue = 0;
					break;
				case D3DRS_LASTPIXEL:
					defaultValue = TRUE;
					break;
#endif



				case D3DRS_ZENABLE:
					defaultValue = D3DZB_TRUE;
					break;
				case D3DRS_FILLMODE:
					defaultValue = D3DFILL_SOLID;
					break;
				case D3DRS_ZWRITEENABLE:
					defaultValue = TRUE;
					break;
				case D3DRS_ALPHATESTENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_SRCBLEND:
					defaultValue = D3DBLEND_SRCALPHA;
					break;
				case D3DRS_DESTBLEND:
					defaultValue = D3DBLEND_INVSRCALPHA;
					break;
				case D3DRS_CULLMODE:
					defaultValue = D3DCULL_CCW;
					break;
				case D3DRS_ZFUNC:
					defaultValue = D3DCMP_LESSEQUAL;
					break;
				case D3DRS_ALPHAREF:
					defaultValue = 0;
					break;
				case D3DRS_ALPHAFUNC:
					defaultValue = D3DCMP_GREATER;
					break;
				case D3DRS_ALPHABLENDENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_STENCILENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_STENCILFAIL:
					defaultValue = D3DSTENCILOP_KEEP;
					break;
				case D3DRS_STENCILZFAIL:
					defaultValue = D3DSTENCILOP_KEEP;
					break;
				case D3DRS_STENCILPASS:
					defaultValue = D3DSTENCILOP_KEEP;
					break;
				case D3DRS_STENCILFUNC:
					defaultValue = D3DCMP_ALWAYS;
					break;
				case D3DRS_STENCILREF:
					defaultValue = 0;
					break;
				case D3DRS_STENCILMASK:
					defaultValue = 0xFFFFFFFF;
					break;
				case D3DRS_STENCILWRITEMASK:
					defaultValue = 0xFFFFFFFF;
					break;
				case D3DRS_WRAP0:
					defaultValue = 0;
					break;
				case D3DRS_WRAP1:
					defaultValue = 0;
					break;
				case D3DRS_WRAP2:
					defaultValue = 0;
					break;
				case D3DRS_WRAP3:
					defaultValue = 0;
					break;
				case D3DRS_WRAP4:
					defaultValue = 0;
					break;
				case D3DRS_WRAP5:
					defaultValue = 0;
					break;
				case D3DRS_WRAP6:
					defaultValue = 0;
					break;
				case D3DRS_WRAP7:
					defaultValue = 0;
					break;
				case D3DRS_CLIPPLANEENABLE:
					defaultValue = 0;
					break;
				case D3DRS_POINTSIZE:
					defaultValue = *((DWORD*)&fPointSize);
					break;
				case D3DRS_POINTSIZE_MIN:
					defaultValue = *((DWORD*)&fPointSizeMin);
					break;
				case D3DRS_POINTSPRITEENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_MULTISAMPLEANTIALIAS:
					defaultValue = TRUE;
					break;
				case D3DRS_MULTISAMPLEMASK:
					defaultValue = 0xFFFFFFFF;
					break;
				case D3DRS_POINTSIZE_MAX:
					defaultValue = *((DWORD*)&fPointSizeMax);
					break;
				case D3DRS_COLORWRITEENABLE:
					defaultValue = 0x0000000F;
					break;
				case D3DRS_BLENDOP:
					defaultValue = D3DBLENDOP_ADD;
					break;
				case D3DRS_SCISSORTESTENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_SLOPESCALEDEPTHBIAS:
					defaultValue = 0;
					break;
				case D3DRS_MINTESSELLATIONLEVEL:
					defaultValue = *((DWORD*)&MinTesselationLevel);
					break;
				case D3DRS_MAXTESSELLATIONLEVEL:
					defaultValue = *((DWORD*)&MaxTesselationLevel);
					break;
				case D3DRS_TWOSIDEDSTENCILMODE:
					defaultValue = FALSE;
					break;
				case D3DRS_CCW_STENCILFAIL:
					defaultValue = D3DSTENCILOP_KEEP;
					break;
				case D3DRS_CCW_STENCILZFAIL:
					defaultValue = D3DSTENCILOP_KEEP;
					break;
				case D3DRS_CCW_STENCILPASS:
					defaultValue = D3DSTENCILOP_KEEP;
					break;
				case D3DRS_CCW_STENCILFUNC:
					defaultValue = D3DCMP_ALWAYS;
					break;
				case D3DRS_COLORWRITEENABLE1:
					defaultValue = 0x0000000f;
					break;
				case D3DRS_COLORWRITEENABLE2:
					defaultValue = 0x0000000f;
					break;
				case D3DRS_COLORWRITEENABLE3:
					defaultValue = 0x0000000f;
					break;
				case D3DRS_BLENDFACTOR:
					defaultValue = 0xffffffff;
					break;
				case D3DRS_DEPTHBIAS:
					defaultValue = 0;
					break;
				case D3DRS_WRAP8:
					defaultValue = 0;
					break;
				case D3DRS_WRAP9:
					defaultValue = 0;
					break;
				case D3DRS_WRAP10:
					defaultValue = 0;
					break;
				case D3DRS_WRAP11:
					defaultValue = 0;
					break;
				case D3DRS_WRAP12:
					defaultValue = 0;
					break;
				case D3DRS_WRAP13:
					defaultValue = 0;
					break;
				case D3DRS_WRAP14:
					defaultValue = 0;
					break;
				case D3DRS_WRAP15:
					defaultValue = 0;
					break;
				case D3DRS_SEPARATEALPHABLENDENABLE:
					defaultValue = FALSE;
					break;
				case D3DRS_SRCBLENDALPHA:
					defaultValue = D3DBLEND_ONE;
					break;
				case D3DRS_DESTBLENDALPHA:
					defaultValue = D3DBLEND_ONE;
					break;
				case D3DRS_BLENDOPALPHA:
					defaultValue = D3DBLENDOP_ADD;
					break;

				default:
					return false;
	}

	return true;

}
*/

HRESULT FXChangeList::SetRenderState (THIS_ D3DRENDERSTATETYPE d3dRenderState, DWORD dwValue )
{
	DWORD dwDefaultValue = 0;
	bool found = DefaultRenderStates::GetDefaultValue(d3dRenderState, dwDefaultValue);

	if (found)
	{
		RenderState* st = &m_changedStates[m_changedStates.Add()];
		st->state = d3dRenderState;
		st->dwValue = dwDefaultValue;
	}

	return D3D_OK;
}

HRESULT FXChangeList::SetSamplerState(THIS_ DWORD dwStage, D3DSAMPLERSTATETYPE d3dSamplerState, DWORD dwValue )
{
	if ((d3dSamplerState == D3DSAMP_ADDRESSU) || (d3dSamplerState == D3DSAMP_ADDRESSV))
	{
		if (dwValue != D3DTADDRESS_WRAP)
		{
			m_changedWraps.Add(dwStage);
		}
	}

	return D3D_OK;
}


#ifdef _XBOX
HRESULT FXChangeList::Do_Not_Use_SetTextureStageState(THIS_ DWORD Stage, DWORD Type, DWORD Value)
{
	return D3D_OK;
}
#else
HRESULT FXChangeList::SetTextureStageState(THIS_ DWORD dwStage, D3DTEXTURESTAGESTATETYPE d3dTextureStageState, DWORD dwValue )
{
	return D3D_OK;
}
#endif




HRESULT FXChangeList::SetTexture(THIS_ DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetVertexShader(THIS_ LPDIRECT3DVERTEXSHADER9 pShader )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetPixelShader(THIS_ LPDIRECT3DPIXELSHADER9 pShader )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetFVF(THIS_ DWORD dwFVF )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetTransform(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetMaterial(THIS_ CONST D3DMATERIAL9 *pMaterial )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetLight(THIS_ DWORD Index, CONST D3DLIGHT9 *pLight )
{
	return D3D_OK;
}

HRESULT FXChangeList::LightEnable(THIS_ DWORD Index, BOOL Enable )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetNPatchMode(THIS_ FLOAT NumSegments )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetVertexShaderConstantF(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetVertexShaderConstantI(THIS_ UINT RegisterIndex,	CONST INT *pConstantData, UINT RegisterCount )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetVertexShaderConstantB(THIS_ UINT RegisterIndex,	CONST BOOL *pConstantData, UINT RegisterCount )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetPixelShaderConstantF(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetPixelShaderConstantI(THIS_ UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount )
{
	return D3D_OK;
}

HRESULT FXChangeList::SetPixelShaderConstantB(THIS_ UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount )
{
	return D3D_OK;
}



