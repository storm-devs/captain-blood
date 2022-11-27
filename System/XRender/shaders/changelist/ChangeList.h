
#ifndef CHANGE_LIST_FX
#define CHANGE_LIST_FX

#include "..\..\GraphicsApi.h"
#include "..\..\..\..\common_h\core.h"



struct RenderState
{
	D3DRENDERSTATETYPE state;
	DWORD dwValue;
};



class FXChangeList : public ID3DXEffectStateManager
{

	LONG m_lRef;



	//Тут состояния которые меняет шейдер...
	array<RenderState>& m_changedStates;



	//Стейты у который wrap был сброшен
	array<dword>& m_changedWraps;



public:


	FXChangeList(array<RenderState>& changedStates, array<dword>& changedWraps);
	~FXChangeList();



	//static bool GetDefaultValue (D3DRENDERSTATETYPE state, DWORD &defaultValue);


	virtual void DirtyCachedValues() {};


	//======= IUnknown methods ===========================================
	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv)
	{
		if (iid == IID_IUnknown || iid == IID_ID3DXEffectStateManager)
		{
			*ppv = static_cast<ID3DXEffectStateManager*>(this);
		} 
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}

		reinterpret_cast<IUnknown*>(this)->AddRef();
		return S_OK;
	}
	STDMETHOD_(ULONG, AddRef)(THIS)
	{
		return (ULONG)InterlockedIncrement( &m_lRef );
	}
	STDMETHOD_(ULONG, Release)(THIS)
	{
		if( 0L == InterlockedDecrement( &m_lRef ) )
		{
			delete this;
			return 0L;
		}

		return m_lRef;
	}

	//======= IUnknown methods ===========================================
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE d3dRenderState, DWORD dwValue );
	STDMETHOD(SetSamplerState)(THIS_ DWORD dwStage, D3DSAMPLERSTATETYPE d3dSamplerState, DWORD dwValue );

#ifdef _XBOX
	STDMETHOD(Do_Not_Use_SetTextureStageState)(THIS_ DWORD Stage, DWORD Type, DWORD Value);
#else
	STDMETHOD(SetTextureStageState)(THIS_ DWORD dwStage, D3DTEXTURESTAGESTATETYPE d3dTextureStageState, DWORD dwValue );
#endif
	

	STDMETHOD(SetTexture)(THIS_ DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture );
	STDMETHOD(SetVertexShader)(THIS_ LPDIRECT3DVERTEXSHADER9 pShader );
	STDMETHOD(SetPixelShader)(THIS_ LPDIRECT3DPIXELSHADER9 pShader );
	STDMETHOD(SetFVF)(THIS_ DWORD dwFVF );
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix );
	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9 *pMaterial );
	STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9 *pLight );
	STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable );
	STDMETHOD(SetNPatchMode)(THIS_ FLOAT NumSegments );
	STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount );
	STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT RegisterIndex,	CONST INT *pConstantData, UINT RegisterCount );
	STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT RegisterIndex,	CONST BOOL *pConstantData, UINT RegisterCount );
	STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount );
	STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount );
	STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount );



};

#endif

