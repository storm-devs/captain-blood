

#ifndef RENDER_STATE_FILTER
#define RENDER_STATE_FILTER

#include "..\..\GraphicsApi.h"
#include "..\..\..\..\common_h\core.h"

#define RENDER_STATES_ARRAY_SIZE 500

//Кэшируем только первые 32 константы
#define VERTEX_SHADER_CONST_COUNT 32

//Кэшируем только первые 16 констант
#define PIXEL_SHADER_CONST_COUNT 16


class StateFilter : public ID3DXEffectStateManager
{

	LONG m_lRef;

public:

	static IDirect3DDevice9* pD3D8;

	struct Info
	{
		dword totalRequestToChangeStates;
		dword changeStates_saved;
		
		
		dword renderstate_changes_count;
		dword samplerstate_changes_count;
		dword texture_changes_count;
		dword vertexshader_changes_count;
		dword pixelshader_changes_count;
		dword setvsconstF_changes_count;
		dword setvsconstI_changes_count;
		dword setvsconstB_changes_count;
		dword setpsconstF_changes_count;
		dword setpsconstI_changes_count;
		dword setpsconstB_changes_count;

		dword setvsconstF_saved_count;
		dword setpsconstF_saved_count;

		dword vs_const_time;
		dword ps_const_time;

		dword vs_const_count;
		dword ps_const_count;
	};
    
	StateFilter();
	virtual ~StateFilter();



	void ResetInfo();
	void GetInfo (StateFilter::Info &info);


	
	virtual void DirtyCachedValues();
	void SetDefaultStates ();




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


	void DirtyState (D3DRENDERSTATETYPE d3dRenderState);


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


	DWORD GetCullState ();

   
private:


	Info workInfo;



	struct RenderState
	{
		DWORD dwValue;
		bool dirty;
	};
	RenderState cachedRenderStates[RENDER_STATES_ARRAY_SIZE];


	struct VertexShaderCache
	{
		LPDIRECT3DVERTEXSHADER9 VertexShader;
		bool dirty;
	};
	struct PixelShaderCache
	{
		LPDIRECT3DPIXELSHADER9 PixelShader;
		bool dirty;
	};
	VertexShaderCache cached_VS;
	PixelShaderCache cached_PS;


	struct ConstantCacheF
	{
		Vector4 v;
		bool dirty;
	};
	ConstantCacheF cachedVertexConstantsF[VERTEX_SHADER_CONST_COUNT];
	ConstantCacheF cachedPixelConstantsF[PIXEL_SHADER_CONST_COUNT];


};

#endif

