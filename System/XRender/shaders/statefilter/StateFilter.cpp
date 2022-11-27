
#include "StateFilter.h"
#include "..\..\Render.h"


#include "..\ChangeList\ChangeList.h"
#include "..\..\DefaultStates\DefaultStates.h"


StateFilter::StateFilter()
{
	m_lRef = 1;

	ResetInfo();

	DirtyCachedValues();
}

StateFilter::~StateFilter()
{
}

void StateFilter::DirtyCachedValues()
{
	for (dword n = 0; n < RENDER_STATES_ARRAY_SIZE; n++)
	{
		cachedRenderStates[n].dirty = true;
	}

	for (dword n = 0; n < VERTEX_SHADER_CONST_COUNT; n++)
	{
		cachedVertexConstantsF[n].dirty = true;
	}

	for (dword n = 0; n < PIXEL_SHADER_CONST_COUNT; n++)
	{
		cachedPixelConstantsF[n].dirty = true;
	}

	cached_VS.dirty = true;
	cached_PS.dirty = true;
}


void StateFilter::DirtyState (D3DRENDERSTATETYPE d3dRenderState)
{
	cachedRenderStates[d3dRenderState].dirty = true;
}

HRESULT StateFilter::SetRenderState(THIS_ D3DRENDERSTATETYPE d3dRenderState, DWORD dwValue )
{
	workInfo.totalRequestToChangeStates++;



	Assert(d3dRenderState < RENDER_STATES_ARRAY_SIZE);




	if (cachedRenderStates[d3dRenderState].dirty == false)
	{
		if (cachedRenderStates[d3dRenderState].dwValue == dwValue)
		{
			workInfo.changeStates_saved++;
			return D3D_OK;
		}
	}


	
	HRESULT hr = pD3D8->SetRenderState(d3dRenderState, dwValue);
	cachedRenderStates[d3dRenderState].dirty = false;
	cachedRenderStates[d3dRenderState].dwValue = dwValue;
	workInfo.renderstate_changes_count++;
	return hr;
}

HRESULT StateFilter::SetSamplerState(THIS_ DWORD dwStage, D3DSAMPLERSTATETYPE d3dSamplerState, DWORD dwValue )
{
	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetSamplerState(dwStage, d3dSamplerState, dwValue);
	workInfo.samplerstate_changes_count++;
	return hr;
}



#ifdef _XBOX
HRESULT StateFilter::Do_Not_Use_SetTextureStageState(THIS_ DWORD Stage, DWORD Type, DWORD Value)
{
	return D3D_OK;
}
#else
HRESULT StateFilter::SetTextureStageState(THIS_ DWORD dwStage, D3DTEXTURESTAGESTATETYPE d3dTextureStageState, DWORD dwValue )
{
	return D3D_OK;
}
#endif





HRESULT StateFilter::SetTexture(THIS_ DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture )
{
	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetTexture(dwStage, pTexture);
	workInfo.texture_changes_count++;
	return hr;
}

HRESULT StateFilter::SetVertexShader(THIS_ LPDIRECT3DVERTEXSHADER9 pShader )
{

	workInfo.totalRequestToChangeStates++;

	if (cached_VS.dirty == false)
	{
		if (cached_VS.VertexShader == pShader)
		{
			workInfo.changeStates_saved++;
			return D3D_OK;
		}
	}
	HRESULT hr = pD3D8->SetVertexShader(pShader);
	cached_VS.dirty = false;
	cached_VS.VertexShader = pShader;
	workInfo.vertexshader_changes_count++;
	return hr;


/*
	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetVertexShader(pShader);
	workInfo.vertexshader_changes_count++;
	return hr;
*/
}

HRESULT StateFilter::SetPixelShader(THIS_ LPDIRECT3DPIXELSHADER9 pShader )
{

	workInfo.totalRequestToChangeStates++;

	if (cached_PS.dirty == false)
	{
		if (cached_PS.PixelShader == pShader)
		{
			workInfo.changeStates_saved++;
			return D3D_OK;
		}
	}
	HRESULT hr = pD3D8->SetPixelShader(pShader);
	cached_PS.dirty = false;
	cached_PS.PixelShader = pShader;
	workInfo.pixelshader_changes_count++;
	return hr;

/*
	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetPixelShader(pShader);
	workInfo.pixelshader_changes_count++;
	return hr;
*/
}

HRESULT StateFilter::SetFVF(THIS_ DWORD dwFVF )
{
	return D3D_OK;
}


HRESULT StateFilter::SetTransform(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix )
{
	return D3D_OK;
}

HRESULT StateFilter::SetMaterial(THIS_ CONST D3DMATERIAL9 *pMaterial )
{
	return D3D_OK;
}

HRESULT StateFilter::SetLight(THIS_ DWORD Index, CONST D3DLIGHT9 *pLight )
{
	return D3D_OK;
}

HRESULT StateFilter::LightEnable(THIS_ DWORD Index, BOOL Enable )
{
	return D3D_OK;
}

HRESULT StateFilter::SetNPatchMode(THIS_ FLOAT NumSegments )
{
	return D3D_OK;
}




HRESULT StateFilter::SetVertexShaderConstantF(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount )
{
/*
	dword dwTime = 0;
	RDTSC_B(dwTime)

	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetVertexShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
	workInfo.setvsconstF_changes_count++;

	RDTSC_E(dwTime)
	workInfo.vs_const_time += dwTime;

	return hr;
*/

	dword dwTime = 0;
	RDTSC_B(dwTime)


	workInfo.totalRequestToChangeStates++;
	//if (RegisterIndex + RegisterCount >= VERTEX_SHADER_CONST_COUNT)
	{
		HRESULT hr = pD3D8->SetVertexShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
		workInfo.setvsconstF_changes_count++;

		RDTSC_E(dwTime)
		workInfo.vs_const_time += dwTime;
		workInfo.vs_const_count++;

		return hr;

	}

	bool isEqual = true;
	for (dword n = RegisterIndex; n < RegisterIndex+RegisterCount; n++)
	{
		if (cachedVertexConstantsF[n].dirty == true)
		{
			isEqual = false;
			break;
		}

		if (cachedVertexConstantsF[n].v.x != pConstantData[(n*4)+0])
		{
			isEqual = false;
			break;
		}
		if (cachedVertexConstantsF[n].v.y != pConstantData[(n*4)+1])
		{
			isEqual = false;
			break;
		}
		if (cachedVertexConstantsF[n].v.z != pConstantData[(n*4)+2])
		{
			isEqual = false;
			break;
		}
		if (cachedVertexConstantsF[n].v.w != pConstantData[(n*4)+3])
		{
			isEqual = false;
			break;
		}
	}

	if (isEqual)
	{
		workInfo.changeStates_saved++;

		RDTSC_E(dwTime)
		workInfo.vs_const_time += dwTime;
		workInfo.vs_const_count++;
		workInfo.setvsconstF_saved_count++;

		return D3D_OK;
	}


	HRESULT hr = NGRender::pRS->D3D()->SetVertexShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
	workInfo.setvsconstF_changes_count++;
	for (dword n = 0; n < RegisterCount; n++)
	{
		cachedVertexConstantsF[RegisterIndex+n].dirty = false;
		cachedVertexConstantsF[RegisterIndex+n].v.x = pConstantData[(n*4)+0];
		cachedVertexConstantsF[RegisterIndex+n].v.y = pConstantData[(n*4)+1];
		cachedVertexConstantsF[RegisterIndex+n].v.z = pConstantData[(n*4)+2];
		cachedVertexConstantsF[RegisterIndex+n].v.w = pConstantData[(n*4)+3];
	}


	RDTSC_E(dwTime)
	workInfo.vs_const_time += dwTime;
	workInfo.vs_const_count++;

	return hr;

}

HRESULT StateFilter::SetVertexShaderConstantI(THIS_ UINT RegisterIndex,	CONST INT *pConstantData, UINT RegisterCount )
{
	dword dwTime = 0;
	RDTSC_B(dwTime)

	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetVertexShaderConstantI(RegisterIndex, pConstantData, RegisterCount);
	workInfo.setvsconstI_changes_count++;

	RDTSC_E(dwTime)
	workInfo.vs_const_time += dwTime;
	workInfo.vs_const_count++;

	return hr;
}

HRESULT StateFilter::SetVertexShaderConstantB(THIS_ UINT RegisterIndex,	CONST BOOL *pConstantData, UINT RegisterCount )
{
	dword dwTime = 0;
	RDTSC_B(dwTime)

	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetVertexShaderConstantB(RegisterIndex, pConstantData, RegisterCount);
	workInfo.setvsconstB_changes_count++;

	RDTSC_E(dwTime)
	workInfo.vs_const_time += dwTime;
	workInfo.vs_const_count++;

	return hr;
}

HRESULT StateFilter::SetPixelShaderConstantF(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount )
{
/*
	dword dwTime = 0;
	RDTSC_B(dwTime)

	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetPixelShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
	workInfo.setpsconstF_changes_count++;

	RDTSC_E(dwTime)
	workInfo.ps_const_time += dwTime;

	return hr;
*/

	dword dwTime = 0;
	RDTSC_B(dwTime)


	workInfo.totalRequestToChangeStates++;
	//if (RegisterIndex + RegisterCount >= VERTEX_SHADER_CONST_COUNT)
	{
		HRESULT hr = pD3D8->SetPixelShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
		workInfo.setpsconstF_changes_count++;

		RDTSC_E(dwTime)
		workInfo.ps_const_time += dwTime;
		workInfo.ps_const_count++;

		return hr;

	}

	bool isEqual = true;
	for (dword n = RegisterIndex; n < RegisterIndex+RegisterCount; n++)
	{
		if (cachedPixelConstantsF[n].dirty == true)
		{
			isEqual = false;
			break;
		}

		if (cachedPixelConstantsF[n].v.x != pConstantData[(n*4)+0])
		{
			isEqual = false;
			break;
		}
		if (cachedPixelConstantsF[n].v.y != pConstantData[(n*4)+1])
		{
			isEqual = false;
			break;
		}
		if (cachedPixelConstantsF[n].v.z != pConstantData[(n*4)+2])
		{
			isEqual = false;
			break;
		}
		if (cachedPixelConstantsF[n].v.w != pConstantData[(n*4)+3])
		{
			isEqual = false;
			break;
		}
	}

	if (isEqual)
	{
		workInfo.changeStates_saved++;

		RDTSC_E(dwTime)
		workInfo.ps_const_time += dwTime;
		workInfo.ps_const_count++;
		workInfo.setpsconstF_saved_count++;

		return D3D_OK;
	}


	HRESULT hr = NGRender::pRS->D3D()->SetPixelShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
	workInfo.setpsconstF_changes_count++;
	for (dword n = 0; n < RegisterCount; n++)
	{
		cachedPixelConstantsF[RegisterIndex+n].dirty = false;
		cachedPixelConstantsF[RegisterIndex+n].v.x = pConstantData[(n*4)+0];
		cachedPixelConstantsF[RegisterIndex+n].v.y = pConstantData[(n*4)+1];
		cachedPixelConstantsF[RegisterIndex+n].v.z = pConstantData[(n*4)+2];
		cachedPixelConstantsF[RegisterIndex+n].v.w = pConstantData[(n*4)+3];
	}


	RDTSC_E(dwTime)
	workInfo.ps_const_time += dwTime;
	workInfo.ps_const_count++;

	return hr;

}

HRESULT StateFilter::SetPixelShaderConstantI(THIS_ UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount )
{
	dword dwTime = 0;
	RDTSC_B(dwTime)

	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetPixelShaderConstantI(RegisterIndex, pConstantData, RegisterCount);
	workInfo.setpsconstI_changes_count++;

	RDTSC_E(dwTime)
	workInfo.ps_const_time += dwTime;
	workInfo.ps_const_count++;

	return hr;
}

HRESULT StateFilter::SetPixelShaderConstantB(THIS_ UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount )
{
	dword dwTime = 0;
	RDTSC_B(dwTime)

	workInfo.totalRequestToChangeStates++;
	HRESULT hr = pD3D8->SetPixelShaderConstantB(RegisterIndex, pConstantData, RegisterCount);
	workInfo.setpsconstB_changes_count++;

	RDTSC_E(dwTime)
	workInfo.ps_const_time += dwTime;
	workInfo.ps_const_count++;

	return hr;
}



void StateFilter::ResetInfo()
{
	workInfo.renderstate_changes_count = 0;

	workInfo.changeStates_saved = 0;
	workInfo.samplerstate_changes_count = 0;
	workInfo.texture_changes_count = 0;
	workInfo.vertexshader_changes_count = 0;
	workInfo.pixelshader_changes_count = 0;
	workInfo.setvsconstF_changes_count = 0;
	workInfo.setvsconstI_changes_count = 0;
	workInfo.setvsconstB_changes_count = 0;
	workInfo.setpsconstF_changes_count = 0;
	workInfo.setpsconstI_changes_count = 0;
	workInfo.setpsconstB_changes_count = 0;


	workInfo.setvsconstF_saved_count = 0;
	workInfo.setpsconstF_saved_count = 0;

	workInfo.totalRequestToChangeStates = 0;

	workInfo.vs_const_time = 0;
	workInfo.ps_const_time = 0;

	workInfo.vs_const_count = 0;
	workInfo.ps_const_count = 0;

}

void StateFilter::GetInfo (StateFilter::Info &info)
{
	info = workInfo;
}

void StateFilter::SetDefaultStates ()
{
	DirtyCachedValues();

	DWORD dwValue = 0;
	for (dword n = 0; n < RENDER_STATES_ARRAY_SIZE; n++)
	{
		if (DefaultRenderStates::GetDefaultValue((D3DRENDERSTATETYPE)n, dwValue))
		{
			SetRenderState((D3DRENDERSTATETYPE)n, dwValue);
		}
	}


	for (dword n = 0 ; n < 16; n++)
	{
		SetSamplerState(n, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		SetSamplerState(n, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		SetSamplerState(n, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		SetSamplerState(n, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		SetSamplerState(n, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	}


	SetVertexShader(NULL);
	SetPixelShader(NULL);
}

DWORD StateFilter::GetCullState ()
{
	if (cachedRenderStates[D3DRS_CULLMODE].dirty == true)
	{
		return D3DCULL_CCW;
	}

	return cachedRenderStates[D3DRS_CULLMODE].dwValue;
}

