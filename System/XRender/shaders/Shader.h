
#ifndef FX_SHADER
#define FX_SHADER


#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\render.h"
#include "ChangeList/ChangeList.h"

#include "..\MaxLights.h"

#include "..\commandBuffers\CommandBufferCompiler.h"



#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
#include "..\batch\PrecompiledBatch.h"

class batchExecutor;
#endif

struct ShaderFogVariables
{
	float fHeight_Density;
	float fHeight_Min;
	float fHeight_Max;
	float fHeight_DistanceInv;

	float fDistance_Density;
	float fDistance_Min;
	float fDistance_Max;
	float fDistance_MinSquared;
	float fDistance_DistanceInv;

	Color fogColor;
};



struct ShaderStandartVariables
{
	//mWorldViewProj
	const Matrix *mWorldViewProj;

	//mWorld
	const Matrix *mWorld;

	//mView
	const Matrix *mView;

	//mProjection
	const Matrix *mProjection;

	//mInverseView
	const Matrix *mInverseView;


	const Matrix *mInverseWorld;

	//Camera in world space
	const Vector4 * vCamPosRelativeWorld;

	const Vector4 * vCamDirRelativeWorld;
};


struct PointLightParams
{
/*
	float fPointLightShadowAffect[MAX_LIGHTS_IN_30_MODEL];

	//point light near radius
	float fPointLightNear[MAX_LIGHTS_IN_30_MODEL];
	//point light near radius*near radius
	float fOneDivPointLightNearPow[MAX_LIGHTS_IN_30_MODEL];
	//point light far radius
	float fPointLightFar[MAX_LIGHTS_IN_30_MODEL]; 
*/

	/*x: OneDivPointLightNearPow, y:PointLightNear, z:PointLightFar, w:PointLightAffectedShadow*/
	Vector4 vPointLightsParams[MAX_LIGHTS_IN_30_MODEL];

	//point light position
	Vector4 vPointLightPos[MAX_LIGHTS_IN_30_MODEL];
	//point light color
	Vector4 vPointLightColor[MAX_LIGHTS_IN_30_MODEL];

	//point light color specular
	Vector4 vPointLightColorSpecular[MAX_LIGHTS_IN_30_MODEL];

};


struct DirectionalLightParams
{
	//directional light color
	Color vGlobalLightColor;

	//directional light color back component
	Color vGlobalLightBackColor;

	//directional light color specular component
	Color vGlobalLightColorSpecular;

	//directional light dir
	Vector vGlobalLightDirection;

	
	dword dwAttachSpecDirToCam;


};

struct AmbientLightParams
{
	//ambient light color
	Color vAmbientLight;

	Color vAmbientLightSpecular;
};




struct ShaderLightingVariables
{
	PointLightParams pointLights;
	DirectionalLightParams dirLight;
	AmbientLightParams ambientLight;
};


class IFile;
class texNameFinder;


class Shader
{
	string ShaderName;

	dword dwOffsetInStream;
	array<D3DVERTEXELEMENT9> vertexDeclaration;


	array<RenderState> changedStates;
	array<dword> wrapsToRestore;


	


	ID3DXEffect* m_effect;
	D3DXHANDLE m_handle;


	IDirect3DVertexDeclaration9* pVertexDeclaration;


	bool ConvertSemanticToUsage (const char* semantic, dword elementCount, D3DVERTEXELEMENT9& decl);
	bool AddToStream (dword streamIndex, D3DXPARAMETER_DESC declaration);


	ID3DXEffectStateManager* stateManager;

	
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	texNameFinder * m_finder;
	//precompiledBatch batch;
	//VariablesDB * mVarsDatabase;
	array<DWORD> vertexDeclStrides;

public:

/*
	void setVariableDatabase (VariablesDB * pDB)
	{
		mVarsDatabase = pDB;
	}
*/	
	
private:



#endif

public:

	static IDirect3DDevice9* pD3D8;




	Shader(ID3DXEffect* pEffect, D3DXHANDLE tech_handle);
	virtual ~Shader();

	bool CreateDeclaration ();

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	void setTextureFinder (texNameFinder * finder);
//	void TestDraw(batchExecutor* executor, ShaderStandartVariables& vars, IDirect3DVertexBuffer9* pVertexStream0, IDirect3DVertexBuffer9* pVertexStream1, IDirect3DIndexBuffer9 * pIndexData, DWORD trianglesCount);
#endif

	virtual dword Begin ();
	virtual void End ();
	virtual void BeginPass (dword passIndex);
	virtual void EndPass (dword passIndex);
	virtual void RestoreStates ();


	void ApplyFogVariables (ShaderFogVariables& vars);
	void ApplyStandartVariables (ShaderStandartVariables& vars);
	void ApplyLightingVariables30 (const ShaderLightingVariables& vars, dword lightCount);
	void ApplyEnvironmentLightingVariables (const ShaderLightingVariables& vars, Vector4 vCamDirRelativeWorld);

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	void BuildChangeList (D3DDevice* pCommandBufferDevice, VariablesDB * pDB, precompiledBatch * resultBatch);
#else
	void BuildChangeList ();
#endif


	void OnResetDevice();
	void OnLostDevice();

	void SetShaderName (const char * szShaderName);
	const char* GetShaderName ();


	void AttachStandartVariables ();


	

protected:

	D3DXVECTOR4 zero;

	Vector4 camPos_ObjectSpace;

	D3DXHANDLE _mWorldViewProj;
	D3DXHANDLE _mWorld;
	D3DXHANDLE _mView;
	D3DXHANDLE _mProjection;
	D3DXHANDLE _mInverseView;
	D3DXHANDLE _vWorldPos;
	D3DXHANDLE _vCamPosRelativeWorld;
	D3DXHANDLE _vCamDirRelativeWorld;
	D3DXHANDLE _vCamPos;

	D3DXHANDLE _fLightsEnabled_count;
	D3DXHANDLE _bLightEnabled30_count;
	
	/*x: OneDivPointLightNearPow, y:PointLightNear, z:PointLightFar, w:PointLightAffectedShadow*/
	D3DXHANDLE _fPointLightParams_array;
/*	
	D3DXHANDLE _fOneDivPointLightNearPow30_array;
	D3DXHANDLE _fPointLightNear30_array;
	D3DXHANDLE _fPointLightAffectedShadow30_array;
	D3DXHANDLE _fPointLightFar30_array;
*/


	D3DXHANDLE _vPointLightPos30_array;
	D3DXHANDLE _vPointLightColor30_array;
	D3DXHANDLE _vPointLightColorSpecular30_array;

	D3DXHANDLE _vAmbientLight;
	D3DXHANDLE _vAmbientLightSpecular;
	D3DXHANDLE _vGlobalLightColorSpecular;
	D3DXHANDLE _vGlobalLightColor;
	D3DXHANDLE _vGlobalLightDirection;
	D3DXHANDLE _vGlobalLightDirectionSpecular;
	D3DXHANDLE _vGlobalLightBackColor;


	D3DXHANDLE _fFogHeightDensity;
	D3DXHANDLE _fFogHeightMin;
	D3DXHANDLE _fFogHeightMax;
	D3DXHANDLE _fFogHeightDistanceInv;
	D3DXHANDLE _fFogDensity;
	D3DXHANDLE _fFogDistanceMin;
	D3DXHANDLE _fFogDistanceMax;
	D3DXHANDLE _fFogDistanceMinSquared;
	D3DXHANDLE _fFogDistanceLen;
	D3DXHANDLE _cFogColor;


	bool bGlobalDestruction;

};


#endif


