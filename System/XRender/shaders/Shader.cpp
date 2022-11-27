
#include "Shader.h"
#include "ShaderVar.h"

#include "..\Render.h"
#include "..\DefaultStates\DefaultStates.h"


#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif



static dword TechCount = 0;

Shader::Shader(ID3DXEffect* pEffect, D3DXHANDLE tech_handle) : vertexDeclaration (_FL_, 64),
                                                               changedStates (_FL_, 16),
															   wrapsToRestore(_FL_, 8)
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
															   ,vertexDeclStrides(_FL_, 4)
#endif
{


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	m_finder = NULL;
#endif



	bGlobalDestruction = false;

	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;
	zero.w = 0.0f;

	m_effect = pEffect;
	m_handle = tech_handle;

	m_effect->AddRef();

	pVertexDeclaration = NULL;






	stateManager = NGRender::pRS->getRedundantStatesFilter();
	
#ifndef _XBOX
	m_effect->SetStateManager(stateManager);
#else
	m_effect->SetStateManager(NULL);
#endif

	TechCount++;


	

}


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
void Shader::setTextureFinder (texNameFinder * finder)
{
	m_finder = finder;
}

/*
void Shader::TestDraw(batchExecutor* executor, ShaderStandartVariables& vars, IDirect3DVertexBuffer9* pVertexStream0, IDirect3DVertexBuffer9* pVertexStream1, IDirect3DIndexBuffer9 * pIndexData, DWORD trianglesCount)
{
	batch.UpdateVariablesFromDB();
	batch.SetStreamSource(0, pVertexStream0);
	batch.SetStreamSource(1, pVertexStream1);
	batch.SetDrawIndexedParameters(pIndexData, trianglesCount);

	//batch.Execute(NGRender::pRS->D3D());

	executor->Run(&batch, NGRender::pRS->D3D());
}
*/
#endif


void Shader::SetShaderName (const char * szShaderName)
{
	ShaderName = szShaderName;
}


const char* Shader::GetShaderName ()
{
	return ShaderName.c_str();
}


Shader::~Shader()
{
	NGRender::pRS->D3D()->SetVertexShader(NULL);
	NGRender::pRS->D3D()->SetPixelShader(NULL);

#ifdef _XBOX
	NGRender::pRS->D3D()->SetVertexDeclaration(NULL);
#endif

	bGlobalDestruction = true;


	RELEASE(pVertexDeclaration);
	RELEASE(m_effect);
}



dword Shader::Begin ()
{
	NGRender::pRS->D3D()->SetVertexDeclaration(pVertexDeclaration);
	
	UINT passes = 0;
	m_effect->SetTechnique(m_handle);

	if (stateManager)
	{
		m_effect->Begin(&passes, D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	} else
	{
		m_effect->Begin(&passes, 0);
	}

	return passes;
}

void Shader::End ()
{
	m_effect->End();

	RestoreStates();
}

void Shader::BeginPass (dword passIndex)
{
	m_effect->BeginPass(passIndex);
	
}

void Shader::EndPass (dword passIndex)
{
	m_effect->EndPass();
}

void Shader::ApplyFogVariables (ShaderFogVariables& vars)
{
/*
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	mVarsDatabase->ApplyFogVariables(vars);

#else
*/

	if (_fFogHeightDensity)
	{
		m_effect->SetFloat(_fFogHeightDensity, vars.fHeight_Density);
	}

	if (_fFogHeightMin)
	{
		m_effect->SetFloat(_fFogHeightMin, vars.fHeight_Min);
	}

	if (_fFogHeightMax)
	{
		m_effect->SetFloat(_fFogHeightMax, vars.fHeight_Max);
	}

	if (_fFogHeightDistanceInv)
	{
		m_effect->SetFloat(_fFogHeightDistanceInv, vars.fHeight_DistanceInv);
	}

	if (_fFogDensity)
	{
		m_effect->SetFloat(_fFogDensity, vars.fDistance_Density);
	}


	if (_fFogDistanceMin)
	{
		m_effect->SetFloat(_fFogDistanceMin, vars.fDistance_Min);
	}

	
	if (_fFogDistanceMax)
	{
		m_effect->SetFloat(_fFogDistanceMax, vars.fDistance_Max);
	}

	
	if (_fFogDistanceMinSquared)
	{
		m_effect->SetFloat(_fFogDistanceMinSquared, vars.fDistance_MinSquared);
	}

	if (_fFogDistanceLen)
	{
		m_effect->SetFloat(_fFogDistanceLen, vars.fDistance_DistanceInv);
	}

	if (_cFogColor)
	{
		m_effect->SetVector(_cFogColor, (D3DXVECTOR4*)&vars.fogColor.v4.x);
	}

//#endif
	
	
	

}

void Shader::ApplyStandartVariables (ShaderStandartVariables& vars)
{
/*
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	mVarsDatabase->ApplyStandartVariables(vars);

#else
*/
	if (_mWorldViewProj)
	{
		m_effect->SetMatrix(_mWorldViewProj, *vars.mWorldViewProj);
	}

	if (_mWorld)
	{
		m_effect->SetMatrix(_mWorld, *vars.mWorld);
	}

	if (_vWorldPos)
	{
		m_effect->SetVector(_vWorldPos, (D3DXVECTOR4*)&Vector4(vars.mWorld->pos));
	}


	if (_mView)
	{
		m_effect->SetMatrix(_mView, *vars.mView);
	}

	if (_mProjection)
	{
		m_effect->SetMatrix(_mProjection, *vars.mProjection);
	}

	if (_mInverseView)
	{
		m_effect->SetMatrix(_mInverseView, *vars.mInverseView);
	}

	if (_vCamPosRelativeWorld)
	{
		m_effect->SetVector(_vCamPosRelativeWorld, (D3DXVECTOR4*)&*vars.vCamPosRelativeWorld);
	}

	if (_vCamDirRelativeWorld)
	{
		m_effect->SetVector(_vCamDirRelativeWorld, (D3DXVECTOR4*)&*vars.vCamDirRelativeWorld);
	}


	if (_vCamPos)
	{
		camPos_ObjectSpace.v = vars.vCamPosRelativeWorld->v * *vars.mInverseWorld;
		camPos_ObjectSpace.w = 1.0f;
		m_effect->SetVector(_vCamPos, (D3DXVECTOR4*)&camPos_ObjectSpace);
	}
//#endif


}


void Shader::ApplyEnvironmentLightingVariables (const ShaderLightingVariables& vars, Vector4 vCamDirRelativeWorld)
{
	/*
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	mVarsDatabase->ApplyEnvironmentLightingVariables(vars);

#else
*/
	if (_vAmbientLight)
	{
		m_effect->SetVector(_vAmbientLight, (D3DXVECTOR4*)&vars.ambientLight.vAmbientLight);
	}

	if (_vAmbientLightSpecular)
	{
		m_effect->SetVector(_vAmbientLightSpecular, (D3DXVECTOR4*)&vars.ambientLight.vAmbientLightSpecular);
	}


	if (_vGlobalLightDirection)
	{
		m_effect->SetVector(_vGlobalLightDirection, (D3DXVECTOR4*)&Vector4(vars.dirLight.vGlobalLightDirection));
	}

	if (_vGlobalLightDirectionSpecular)
	{
		Vector4 specDir;
		if (vars.dirLight.dwAttachSpecDirToCam)
		{
			specDir.v = Vector (-vCamDirRelativeWorld.x, vCamDirRelativeWorld.y, -vCamDirRelativeWorld.z);
			specDir.w = 0.0f;
		} else
		{
			specDir = vars.dirLight.vGlobalLightDirection;
		}
		
		m_effect->SetVector(_vGlobalLightDirectionSpecular, (D3DXVECTOR4*)&specDir);
	}


	

	if (_vGlobalLightColor)
	{
		m_effect->SetVector(_vGlobalLightColor, (D3DXVECTOR4*)&vars.dirLight.vGlobalLightColor);
	}

	if (_vGlobalLightColorSpecular)
	{
		m_effect->SetVector(_vGlobalLightColorSpecular, (D3DXVECTOR4*)&vars.dirLight.vGlobalLightColorSpecular);
	}
	
	if (_vGlobalLightBackColor)
	{
		m_effect->SetVector(_vGlobalLightBackColor, (D3DXVECTOR4*)&vars.dirLight.vGlobalLightBackColor);
	}

//#endif
}



void Shader::ApplyLightingVariables30 (const ShaderLightingVariables& vars, dword lightCount)
{
/*
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	mVarsDatabase->ApplyLightingVariables30(vars, lightCount);

#else
*/

	static BOOL enabledArray[MAX_LIGHTS_IN_30_MODEL];
	memset (enabledArray, 0, sizeof(BOOL) * MAX_LIGHTS_IN_30_MODEL);
	for (dword i = 0; i < lightCount; i++)
	{
		enabledArray[i] = TRUE;
	}

	if (_fLightsEnabled_count)
	{
		m_effect->SetFloat(_fLightsEnabled_count, (float)lightCount);
	}

	if (_bLightEnabled30_count)
	{
		 m_effect->SetBoolArray(_bLightEnabled30_count, enabledArray, MAX_LIGHTS_IN_30_MODEL);
	}

	if (_fPointLightParams_array)
	{
		m_effect->SetVectorArray(_fPointLightParams_array, (const D3DXVECTOR4 *)vars.pointLights.vPointLightsParams, lightCount);
	}

	if (_vPointLightPos30_array)
	{
		m_effect->SetVectorArray(_vPointLightPos30_array, (const D3DXVECTOR4 *)vars.pointLights.vPointLightPos, lightCount);
	}


	if (_vPointLightColor30_array)
	{
		m_effect->SetVectorArray(_vPointLightColor30_array, (const D3DXVECTOR4 *)vars.pointLights.vPointLightColor, lightCount);
	}

	

	if (_vPointLightColorSpecular30_array)
	{
		m_effect->SetVectorArray(_vPointLightColorSpecular30_array, (const D3DXVECTOR4 *)vars.pointLights.vPointLightColorSpecular, lightCount);
	}

//#endif

}

bool Shader::CreateDeclaration ()
{
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	vertexDeclStrides.DelAll();
#endif

	D3DXTECHNIQUE_DESC techDesc;
	m_effect->GetTechniqueDesc(m_handle, &techDesc);

	static char tempName[512];
	crt_snprintf(tempName, 511, "x__%s", techDesc.Name);
	D3DXHANDLE declarationHandle = m_effect->GetParameterByName(NULL, tempName);


	D3DXPARAMETER_DESC structDesc;
	m_effect->GetParameterDesc(declarationHandle, &structDesc);


	if (structDesc.Class != D3DXPC_STRUCT || structDesc.StructMembers <= 0)
	{
		api->Trace("Can create vertex declaration only from struct !!");
		return false;
	}


	


	dword dwStreamIndex = 0;
	dwOffsetInStream = 0;

	for (DWORD i = 0; i < structDesc.StructMembers; i++)
	{
		D3DXHANDLE memberHandle = m_effect->GetParameter(declarationHandle, i);
		if (!memberHandle) continue;
		D3DXPARAMETER_DESC memberDesc;            
		m_effect->GetParameterDesc(memberHandle, &memberDesc);

		if (memberDesc.Class == D3DXPC_STRUCT && memberDesc.StructMembers > 0)
		{
			for (DWORD j = 0; j < memberDesc.StructMembers; j++)
			{
				D3DXHANDLE streamStructHandle = m_effect->GetParameter(memberHandle, j);
				if (!streamStructHandle) continue;
				D3DXPARAMETER_DESC streamMemberDesc;            
				m_effect->GetParameterDesc(streamStructHandle, &streamMemberDesc);

				if (!AddToStream(dwStreamIndex, streamMemberDesc)) return false;
			}


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
			//считаем количество стримов и страйды на каждый стрим...
			vertexDeclStrides.Add(dwOffsetInStream);
#endif

			dwStreamIndex++;
			dwOffsetInStream = 0;
		} else
		{
			if (!AddToStream(dwStreamIndex, memberDesc)) return false;
		}
	}


	
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	//считаем количество стримов и страйды на каждый стрим в последнем потоке...
	vertexDeclStrides.Add(dwOffsetInStream);
#endif


/*
	for (dword j = 0; j < vertexDeclaration.Size(); j++)
	{
		api->Trace("[%d] offset:%d  ", vertexDeclaration[j].Stream, vertexDeclaration[j].Offset);
	}
*/


	D3DVERTEXELEMENT9 v;
	v.Stream = 0xFF;
	v.Offset = 0;
	v.Type = D3DDECLTYPE_UNUSED;
	v.Usage = (D3DDECLUSAGE)0;
	v.Method = (D3DDECLMETHOD)0;
	v.UsageIndex = 0;
	vertexDeclaration.Add(v);


	NGRender::pRS->D3D()->CreateVertexDeclaration(&vertexDeclaration[0], &pVertexDeclaration);

	return true;
}

bool Shader::AddToStream (dword streamIndex, D3DXPARAMETER_DESC declaration)
{
	//usage      declaration.Semantic
	//Columns = 4, Rows = 1
	//Class = VECTOR
	//TYPE = Float
	//BYTES = 16

	//api->Trace("[%d], '%s'", streamIndex, declaration.Name);

	D3DVERTEXELEMENT9 decl;
	decl.Method = D3DDECLMETHOD_DEFAULT;
	decl.Offset = dwOffsetInStream;
	decl.Stream = streamIndex;


	if (!ConvertSemanticToUsage(declaration.Semantic, declaration.Columns, decl))
	{
		return false;
	}

	vertexDeclaration.Add(decl);

	return true;
}

bool Shader::ConvertSemanticToUsage (const char* semantic, dword elementCount, D3DVERTEXELEMENT9& decl)
{
	string strSemantic = semantic;
	string strUsageIndex = "0";
	string ins;


	if (strSemantic.Size() > 1)
	{
		char lastChar = strSemantic.Last();
		if (lastChar >= 0x30 && lastChar <= 0x39)
		{
			strUsageIndex = "";
			strUsageIndex += lastChar;
			strSemantic.DeleteLastSymbol(lastChar);

			if (strSemantic.Size() > 1)
			{
				char lastChar2 = strSemantic.Last();
				if (lastChar2 >= 0x30 && lastChar2 <= 0x39)
				{
					ins += lastChar2;
					strUsageIndex.Insert(0, ins);
					strSemantic.DeleteLastSymbol(lastChar2);
				}
			}
		}
	}
	

/*
	Vertex Shader Input Semantic Description 
		BINORMAL[n] Binormal 
		BLENDINDICES[n] Blend indices 
+BLENDWEIGHT[n] Blend weights 
+COLOR[n] Diffuse and specular color 
+NORMAL[n] Normal vector 
+POSITION[n] Vertex position in object space 
+POSITIONT Transformed vertex position. POSITIONT tells the runtime that the vertex is transformed and that the vertex shader should not be executed. 
		PSIZE[n] Point size 
		TANGENT[n] Tangent 
		TESSFACTOR[n] Tessellation factor 
+TEXCOORD[n] Texture coordinates 
*/


/*
	packed_v4 = color
		packed_uv = texcoord, short2
		packed_v2 = blendweight, short2
*/
	

	decl.UsageIndex = atoi(strUsageIndex.c_str());

	if (crt_stricmp (strSemantic.c_str(), "POSITION") == 0)
	{
		decl.Usage = D3DDECLUSAGE_POSITION;

		switch (elementCount)
		{
		case 1:
			decl.Type = D3DDECLTYPE_FLOAT1;
			dwOffsetInStream += (sizeof(float) * 1);
			break;
		case 2:
			decl.Type = D3DDECLTYPE_FLOAT2;
			dwOffsetInStream += (sizeof(float) * 2);
			break;
		case 3:
			decl.Type = D3DDECLTYPE_FLOAT3;
			dwOffsetInStream += (sizeof(float) * 3);
			break;
		case 4:
			decl.Type = D3DDECLTYPE_FLOAT4;
			dwOffsetInStream += (sizeof(float) * 4);
			break;
		}

		return true;
	}

/*
	if (crt_stricmp (strSemantic.c_str(), "POSITIONT") == 0)
	{
		decl.Usage = D3DDECLUSAGE_POSITIONT;

		switch (elementCount)
		{
		case 1:
			decl.Type = D3DDECLTYPE_FLOAT1;
			dwOffsetInStream += (sizeof(float) * 1);
			break;
		case 2:
			decl.Type = D3DDECLTYPE_FLOAT2;
			dwOffsetInStream += (sizeof(float) * 2);
			break;
		case 3:
			decl.Type = D3DDECLTYPE_FLOAT3;
			dwOffsetInStream += (sizeof(float) * 3);
			break;
		case 4:
			decl.Type = D3DDECLTYPE_FLOAT4;
			dwOffsetInStream += (sizeof(float) * 4);
			break;
		}

		return true;
	}
*/

	if (crt_stricmp (strSemantic.c_str(), "TEXCOORD") == 0)
	{
		decl.Usage = D3DDECLUSAGE_TEXCOORD;

		switch (elementCount)
		{
		case 1:
			decl.Type = D3DDECLTYPE_FLOAT1;
			dwOffsetInStream += (sizeof(float) * 1);
			break;
		case 2:
			decl.Type = D3DDECLTYPE_FLOAT2;
			dwOffsetInStream += (sizeof(float) * 2);
			break;
		case 3:
			decl.Type = D3DDECLTYPE_FLOAT3;
			dwOffsetInStream += (sizeof(float) * 3);
			break;
		case 4:
			decl.Type = D3DDECLTYPE_FLOAT4;
			dwOffsetInStream += (sizeof(float) * 4);
			break;
		}

		return true;
	}

	if (crt_stricmp (strSemantic.c_str(), "NORMAL") == 0)
	{
		decl.Usage = D3DDECLUSAGE_NORMAL;

		switch (elementCount)
		{
		case 1:
			decl.Type = D3DDECLTYPE_FLOAT1;
			dwOffsetInStream += (sizeof(float) * 1);
			break;
		case 2:
			decl.Type = D3DDECLTYPE_FLOAT2;
			dwOffsetInStream += (sizeof(float) * 2);
			break;
		case 3:
			decl.Type = D3DDECLTYPE_FLOAT3;
			dwOffsetInStream += (sizeof(float) * 3);
			break;
		case 4:
			decl.Type = D3DDECLTYPE_FLOAT4;
			dwOffsetInStream += (sizeof(float) * 4);
			break;
		}

		return true;
	}

	if (crt_stricmp (strSemantic.c_str(), "COLOR") == 0)
	{
		decl.Usage = D3DDECLUSAGE_COLOR;

		switch (elementCount)
		{
		case 1:
			decl.Type = D3DDECLTYPE_FLOAT1;
			dwOffsetInStream += (sizeof(float) * 1);
			break;
		case 2:
			decl.Type = D3DDECLTYPE_FLOAT2;
			dwOffsetInStream += (sizeof(float) * 2);
			break;
		case 3:
			decl.Type = D3DDECLTYPE_FLOAT3;
			dwOffsetInStream += (sizeof(float) * 3);
			break;
		case 4:
			decl.Type = D3DDECLTYPE_D3DCOLOR;
			dwOffsetInStream += (sizeof(dword));
			break;
		}

		return true;
	}


	if (crt_stricmp (strSemantic.c_str(), "BLENDWEIGHT") == 0)
	{
		decl.Usage = D3DDECLUSAGE_BLENDWEIGHT;

		switch (elementCount)
		{
		case 1:
			decl.Type = D3DDECLTYPE_FLOAT1;
			dwOffsetInStream += (sizeof(float) * 1);
			break;
		case 2:
			decl.Type = D3DDECLTYPE_SHORT2;
			dwOffsetInStream += (sizeof(short) * 2);
			break;
		case 3:
			decl.Type = D3DDECLTYPE_FLOAT3;
			dwOffsetInStream += (sizeof(float) * 3);
			break;
		case 4:
			decl.Type = D3DDECLTYPE_SHORT4;
			dwOffsetInStream += (sizeof(short) * 4);
			break;
		}

		return true;
	}

	api->Trace("Vertex declaration error: %s", semantic);
	return false;

}

void Shader::AttachStandartVariables ()
{
	_mWorldViewProj = m_effect->GetParameterByName(NULL, "mWorldViewProj");
	_mWorld = m_effect->GetParameterByName(NULL, "mWorld");
	_mView = m_effect->GetParameterByName(NULL, "mView");
	_mProjection = m_effect->GetParameterByName(NULL, "mProjection");
	_mInverseView = m_effect->GetParameterByName(NULL, "mInverseView");
	_vWorldPos = m_effect->GetParameterByName(NULL, "vWorldPos");
	_vCamPosRelativeWorld = m_effect->GetParameterByName(NULL, "vCamPosRelativeWorld");
	_vCamDirRelativeWorld = m_effect->GetParameterByName(NULL, "vCamDirRelativeWorld");
	_vCamPos = m_effect->GetParameterByName(NULL, "vCamPos");



	_fLightsEnabled_count = m_effect->GetParameterByName(NULL, "fEnalbedLightsCount");
	_bLightEnabled30_count = m_effect->GetParameterByName(NULL, "bLightEnabled30");

/*
	_fPointLightAffectedShadow30_array = m_effect->GetParameterByName(NULL, "fPointShadowAffect30");
	_fOneDivPointLightNearPow30_array = m_effect->GetParameterByName(NULL, "fOneDivPointLightNearPow30");
	_fPointLightNear30_array = m_effect->GetParameterByName(NULL, "fPointLightFar30");
	_fPointLightFar30_array = m_effect->GetParameterByName(NULL, "fPointLightNear30");
*/

	_fPointLightParams_array = m_effect->GetParameterByName(NULL, "fPointLightParams");



	_vPointLightPos30_array = m_effect->GetParameterByName(NULL, "vPointLightPos30");
	_vPointLightColor30_array = m_effect->GetParameterByName(NULL, "vPointLightColor30");
	_vPointLightColorSpecular30_array = m_effect->GetParameterByName(NULL, "vPointLightColorSpecular30");

	


	_vAmbientLight = m_effect->GetParameterByName(NULL, "vAmbientLight");
	_vAmbientLightSpecular = m_effect->GetParameterByName(NULL, "vAmbientLightSpecular");
	_vGlobalLightColor = m_effect->GetParameterByName(NULL, "vGlobalLightColor");
	_vGlobalLightDirection = m_effect->GetParameterByName(NULL, "vGlobalLightDirection");
	_vGlobalLightDirectionSpecular = m_effect->GetParameterByName(NULL, "vGlobalLightDirectionSpec");
	_vGlobalLightBackColor = m_effect->GetParameterByName(NULL, "vGlobalLightBackColor");
	_vGlobalLightColorSpecular = m_effect->GetParameterByName(NULL, "vGlobalLightColorSpecular");



	_fFogHeightDensity = m_effect->GetParameterByName(NULL, "fFogHeightDensity");
	_fFogHeightMin = m_effect->GetParameterByName(NULL, "fFogHeightMin");
	_fFogHeightMax = m_effect->GetParameterByName(NULL, "fFogHeightMax");
	_fFogHeightDistanceInv = m_effect->GetParameterByName(NULL, "fFogHeightDistanceInv");
	_fFogDensity = m_effect->GetParameterByName(NULL, "fFogDensity");
	_fFogDistanceMin = m_effect->GetParameterByName(NULL, "fFogDistanceMin");
	_fFogDistanceMax = m_effect->GetParameterByName(NULL, "fFogDistanceMax");
	_fFogDistanceMinSquared = m_effect->GetParameterByName(NULL, "fFogDistanceMinSquared");
	_fFogDistanceLen = m_effect->GetParameterByName(NULL, "fFogDistanceLen");
	_cFogColor = m_effect->GetParameterByName(NULL, "cFogColor");





}

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
void Shader::BuildChangeList (D3DDevice* pCommandBufferDevice, VariablesDB * pDB, precompiledBatch * resultBatch)
#else
void Shader::BuildChangeList ()
#endif
{


#ifndef _XBOX

	//FIXME:Тут тормоза из за NEW на PC
	FXChangeList* forSaveStatesChange = NEW FXChangeList(changedStates, wrapsToRestore);
	
	HRESULT hr = m_effect->SetStateManager(forSaveStatesChange);

	UINT passes = 0;
	m_effect->SetTechnique(m_handle);
	m_effect->Begin(&passes, D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);

	//Подддерживаются только однопроходные шейдеры
	if (passes > 1)
	{
		api->Trace("Shader '%s' have more than 1 pass - error !", ShaderName.c_str());
		Assert (false);
	}
	

	for (dword n = 0; n < passes; n++)
	{
		m_effect->BeginPass(n);
		m_effect->EndPass();
	}
	m_effect->End();


	m_effect->SetStateManager(stateManager);
	DELETE(forSaveStatesChange);
#else


	bool bLog = false;

	D3DXPASS_DESC passDesc;

	D3DXTECHNIQUE_DESC tDesc;
	m_effect->GetTechniqueDesc(m_handle, &tDesc);

	
	NGRender::pRS->getRedundantStatesFilter()->SetDefaultStates();

	UINT passes = 0;
	m_effect->SetTechnique(m_handle);
	m_effect->Begin(&passes, D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);

	//Подддерживаются только однопроходные шейдеры
	Assert (passes <= 1);

	for (dword n = 0 ; n < 16; n++)
	{
		NGRender::pRS->D3D()->SetTexture(n, NULL);
	}


#if defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	D3DXTECHNIQUE_DESC tDescForName;
	m_effect->GetTechniqueDesc(m_handle, &tDescForName);


	CommandBufferCompiler * bufCompiler = NEW CommandBufferCompiler();
	bufCompiler->SetName(tDescForName.Name);

	resultBatch->Cleanup();
#endif

	for (dword n = 0; n < passes; n++)
	{
		D3DXHANDLE passHandle = m_effect->GetPass(m_handle, n);
		m_effect->GetPassDesc(passHandle, &passDesc);

#if defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
		bufCompiler->ExtractVertexShader(passDesc.pVertexShaderFunction);
		bufCompiler->ExtractPixelShader(passDesc.pPixelShaderFunction);
		bufCompiler->PatchShadersAndDumpMicrocode(pVertexDeclaration, vertexDeclStrides);
#endif


		m_effect->BeginPass(n);
		m_effect->EndPass();
	}

	DWORD dwDefValue = 0;
	DWORD dwGPUValue = 0;
	for (dword n = 0; n < D3DRS_MAX; n++)
	{
		D3DRENDERSTATETYPE renderState = (D3DRENDERSTATETYPE)n;

		bool bExist = DefaultRenderStates::GetDefaultValue(renderState, dwDefValue);
		if (!bExist) continue;

		dwGPUValue = 0xDEADDEAD;
		NGRender::pRS->D3D()->GetRenderState(renderState, &dwGPUValue);

		if (dwDefValue != dwGPUValue)
		{
			RenderState* rs = &changedStates[changedStates.Add()];
			rs->state = renderState;
			rs->dwValue = dwDefValue;

#if defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
			bufCompiler->ExtractRenderState(renderState, dwGPUValue, bLog);
#endif
		}
	}
	
	DWORD adressU, adressV;
	DWORD minF, magF, MipF;
	for (dword n = 0 ; n < 16; n++)
	{
		IDirect3DBaseTexture9* pTex = NULL;
		NGRender::pRS->D3D()->GetTexture(n, &pTex);

		
		NGRender::pRS->D3D()->GetSamplerState(n, D3DSAMP_ADDRESSU, &adressU);
		NGRender::pRS->D3D()->GetSamplerState(n, D3DSAMP_ADDRESSV, &adressV);
		NGRender::pRS->D3D()->GetSamplerState(n, D3DSAMP_MAGFILTER, &magF);
		NGRender::pRS->D3D()->GetSamplerState(n, D3DSAMP_MINFILTER, &minF);
		NGRender::pRS->D3D()->GetSamplerState(n, D3DSAMP_MIPFILTER, &MipF);

#if defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
		//Если была текстура, добавить к состояниям...
		if (pTex)
		{
			const char* variableName = m_finder->findTextureVariableName(pTex);

			bufCompiler->ExtractTexture(n, variableName);

			bufCompiler->ExtractSamplerState(n, D3DSAMP_ADDRESSU, adressU);
			bufCompiler->ExtractSamplerState(n, D3DSAMP_ADDRESSV, adressV);
			bufCompiler->ExtractSamplerState(n, D3DSAMP_MAGFILTER, magF);
			bufCompiler->ExtractSamplerState(n, D3DSAMP_MINFILTER, minF);
			bufCompiler->ExtractSamplerState(n, D3DSAMP_MIPFILTER, MipF);
		}
#endif


	

		if (adressU != D3DTADDRESS_WRAP || adressV != D3DTADDRESS_WRAP)
		{
			wrapsToRestore.Add(n);
	
		}

		NGRender::pRS->D3D()->SetTexture(n, NULL);
	}

	m_effect->End();



#if defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	resultBatch->SetVariablesDatabase(pDB);
	bufCompiler->Finalize(pCommandBufferDevice, resultBatch);
	delete bufCompiler;
	bufCompiler = (CommandBufferCompiler *)0xDEADBABE;
#endif

#endif


}

void Shader::RestoreStates ()
{
#ifndef _XBOX

	if (!stateManager) return;

	for (dword i = 0; i < changedStates.Size(); i++)
	{
		const RenderState &st = changedStates[i];
		stateManager->SetRenderState(st.state, st.dwValue);
	}

	for (dword i = 0; i < wrapsToRestore.Size(); i++)
	{
		stateManager->SetSamplerState(wrapsToRestore[i], D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		stateManager->SetSamplerState(wrapsToRestore[i], D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}
#else

	for (dword i = 0; i < changedStates.Size(); i++)
	{
		const RenderState &st = changedStates[i];
		NGRender::pRS->D3D()->SetRenderState(st.state, st.dwValue);
	}

	for (dword i = 0; i < wrapsToRestore.Size(); i++)
	{
		NGRender::pRS->D3D()->SetSamplerState(wrapsToRestore[i], D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		NGRender::pRS->D3D()->SetSamplerState(wrapsToRestore[i], D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}


#endif


}


void Shader::OnLostDevice()
{
	if (!m_effect) return;
	m_effect->OnLostDevice();
}

void Shader::OnResetDevice()
{
	if (!m_effect) return;
	m_effect->OnResetDevice();
}


