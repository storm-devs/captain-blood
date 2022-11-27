#include "..\..\common_h\render.h"

#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#include "VariablesDB.h"
#include "Render.h"

VariablesDB::VariablesDB() : vector4Pool (_FL_), boolPool (_FL_), variables (_FL_), variables_hash (_FL_), texturesAddrPool (_FL_), texture_variables (_FL_)
{
#ifndef STOP_DEBUG
	bDebugIsConst = false;
#endif


	_mWorldViewProj = NULL;
	_mWorld = NULL;
	_mView = NULL;
	_mProjection = NULL;
	_mInverseView = NULL;
	_vWorldPos = NULL;
	_vCamPosRelativeWorld = NULL;
	_vCamDirRelativeWorld = NULL;
	_vCamPos = NULL;



	_fFogHeightDensity = NULL;
	_fFogHeightMin = NULL;
	_fFogHeightMax = NULL;
	_fFogHeightDistanceInv = NULL;
	_fFogDensity = NULL;
	_fFogDistanceMin = NULL;
	_fFogDistanceMax = NULL;
	_fFogDistanceMinSquared = NULL;
	_fFogDistanceLen = NULL;
	_cFogColor = NULL;


	_vAmbientLight = NULL;
	_vAmbientLightSpecular = NULL;
	_vGlobalLightColorSpecular = NULL;
	_vGlobalLightColor = NULL;
	_vGlobalLightDirection = NULL;
	_vGlobalLightDirectionSpecular = NULL;
	_vGlobalLightBackColor = NULL;


}

VariablesDB::~VariablesDB()
{
	varPool.Destroy();
}


void VariablesDB::Init ()
{
	for (DWORD i = 0; i < variables.Size(); i++)
	{
		variables[i]->convertOffsetToPointer(&vector4Pool[0], &boolPool[0], &texturesAddrPool[0]);
	}

#ifndef STOP_DEBUG
	Debug_MakeItConst();
#endif

/*

Ничего особо не дало, т.к. кэш миссы походу в destination

	DWORD dwBytesSize = vector4Pool.GetDataSize();
	BYTE* dwBytesData = (BYTE*)&vector4Pool[0].x;
	XLockL2(XLOCKL2_INDEX_XPS, dwBytesData, dwBytesSize, XLOCKL2_LOCK_SIZE_1_WAY, XLOCKL2_FLAG_SUSPEND_REPLACEMENT);
*/

	BindStandartVariables();
}

#ifndef STOP_DEBUG

void VariablesDB::DebugAssert(bool bExpr)
{
	if (bExpr == false)
	{
		int a = 0;
	}
	Assert(bExpr);
}

void VariablesDB::Debug_MakeItConst()
{
	bDebugIsConst = true;
}
#endif


IVariable * VariablesDB::getShaderVariable (const char* shaderName, const char * srcFile, long srcLine)
{
#ifndef STOP_DEBUG
	DebugAssert(bDebugIsConst == true);
#endif

	IVariable* var = NULL;
	variables_hash.Find(shaderName, var);

	if (var == NULL)
	{
		api->Trace("Variable '%s' not found", shaderName);

		variables_hash.Find(shaderName, var);
		return NULL;
	}

	return var;
}

void VariablesDB::AddVariable (const char * szVariableName, ID3DXEffect* pEffect, D3DXHANDLE paramHandle)
{
#ifndef STOP_DEBUG
	DebugAssert(bDebugIsConst == false);
#endif

	D3DXPARAMETER_DESC constDesc;
	pEffect->GetParameterDesc(paramHandle, &constDesc);

	if ((constDesc.Class == D3DXPC_OBJECT && constDesc.Type != D3DXPT_TEXTURE) || constDesc.Class == D3DXPC_STRUCT)
	{
/*
		OutputDebugString("skip var : ");
		OutputDebugString(szVariableName);
		OutputDebugString("\n");
*/

		return;
	}


	DWORD dwSlotsInPool = 0;
	VariableType type = VT_UNKNOWN;

	if (constDesc.Class == D3DXPC_OBJECT && constDesc.Type == D3DXPT_TEXTURE && constDesc.Elements == 0) 
	{
		type = VT_TEXTURE;
		dwSlotsInPool = 1;
	}

	if (constDesc.Class == D3DXPC_VECTOR && constDesc.Type == D3DXPT_FLOAT)
	{
		if (constDesc.Columns == 3)
		{
			type = VT_VECTOR3;
			dwSlotsInPool = 1;
		}

		if (constDesc.Columns == 4)
		{
			type = VT_VECTOR4;
			dwSlotsInPool = 1;
		}
	}

	if (constDesc.Class == D3DXPC_SCALAR && constDesc.Type == D3DXPT_FLOAT && constDesc.Columns == 1)
	{
		type = VT_FLOAT;
		dwSlotsInPool = 1;
	}

	if (constDesc.Class == D3DXPC_MATRIX_COLUMNS && constDesc.Type == D3DXPT_FLOAT && constDesc.Columns == 4 && constDesc.Rows == 4)
	{
		type = VT_MATRIX4x4;
		dwSlotsInPool = 4;
	}

	if (constDesc.Class == D3DXPC_SCALAR && constDesc.Type == D3DXPT_BOOL && constDesc.Columns == 1)
	{
		type = VT_BOOL;
		dwSlotsInPool = 1;
	}

	DWORD dwElementsCount = constDesc.Elements;
	if (dwElementsCount <= 0)
	{
		dwElementsCount = 1;
	}
	

	if (type == VT_BOOL)
	{
		dwElementsCount = ((dwElementsCount + 31) >> 5);
	}

/*
	OutputDebugString("var : ");
	OutputDebugString(szVariableName);
	OutputDebugString("\n");
*/


	if (type == VT_UNKNOWN)
	{
		OutputDebugString("-- unknown var !!! ---------------- '");
		OutputDebugString(szVariableName);
		OutputDebugString("'----------------\n");
		DebugBreak();
	}
	

	DWORD dwIndexInPool = INVALID_ARRAY_INDEX;
	//void * ptrToPool = NULL;
	DWORD dwElementsToAdd = dwSlotsInPool * dwElementsCount;
	if (type == VT_BOOL)
	{
		dwIndexInPool = boolPool.Size();
		boolPool.AddElements(dwElementsToAdd);
		memset (&boolPool[dwIndexInPool], 0, dwElementsToAdd*sizeof(boolVector32));
		//ptrToPool = &boolPool[dwIndexInPool];
	} else
	{
		if (type == VT_TEXTURE)
		{
			dwIndexInPool = texturesAddrPool.Size();
			texturesAddrPool.AddElements(dwElementsToAdd);
			//ptrToPool = &texturesAddrPool[dwIndexInPool];
		} else
		{
			dwIndexInPool = vector4Pool.Size();
			vector4Pool.AddElements(dwElementsToAdd);
		
			//ptrToPool = &vector4Pool[dwIndexInPool];
		}
	}
	

	Assert(dwIndexInPool != INVALID_ARRAY_INDEX);

	IVariable* varHandle = varPool.Allocate();

	//Передаем index как указатель пока, потом надо будет сделать fixup всем...
	varHandle->Init(type, dwElementsCount, dwSlotsInPool, (void*)dwIndexInPool);

	if (type == VT_TEXTURE)
	{
		texture_variables.Add(varHandle);
	}
	
	variables.Add(varHandle);
	variables_hash.Add(szVariableName, varHandle);
}

/*
void VariablesDB::SetFloat(void * ptrInPool, float val)
{
	((Vector4*)ptrInPool)->x = val;
}

void VariablesDB::SetBool32(void * ptrInPool, boolVector32 val)
{
	*((boolVector32*)ptrInPool) = val;
}

void VariablesDB::SetTexture(void * ptrInPool, IBaseTexture* val)
{
	IDirect3DBaseTexture9** texAddres = ((IDirect3DBaseTexture9**)ptrInPool);
	IDirect3DBaseTexture9 * pDXTexture = NGRender::pRS->GetDXBaseTexture(val);

	*texAddres = pDXTexture;
}

void VariablesDB::SetMatrix(void * ptrInPool, const Matrix& val)
{
	Vector4* vecPtr = (Vector4*)ptrInPool;

	vecPtr->x = val.m[0][0];
	vecPtr->y = val.m[1][0];
	vecPtr->z = val.m[2][0];
	vecPtr->w = val.m[3][0];

	vecPtr++;

	vecPtr->x = val.m[0][1];
	vecPtr->y = val.m[1][1];
	vecPtr->z = val.m[2][1];
	vecPtr->w = val.m[3][1];

	vecPtr++;

	vecPtr->x = val.m[0][2];
	vecPtr->y = val.m[1][2];
	vecPtr->z = val.m[2][2];
	vecPtr->w = val.m[3][2];

	vecPtr++;

	vecPtr->x = val.m[0][3];
	vecPtr->y = val.m[1][3];
	vecPtr->z = val.m[2][3];
	vecPtr->w = val.m[3][3];
}

void VariablesDB::SetVector(void * ptrInPool, const Vector & val)
{
	((Vector4*)ptrInPool)->v = val;
}

void VariablesDB::SetVector4(void * ptrInPool, const Vector4 & val)
{
	*((Vector4*)ptrInPool) = val;
}

void VariablesDB::SetVectorAsVector4(void * ptrInPool, const Vector & val)
{
	((Vector4*)ptrInPool)->v = val;
	((Vector4*)ptrInPool)->w = 1.0f;
}

void VariablesDB::SetVector4Array(void * ptrInPool, const Vector4* val, dword dwCount)
{
	XMemCpy(ptrInPool, val, sizeof(Vector4) * dwCount);
}
*/

IDirect3DBaseTexture9** VariablesDB::getTextureOutput (const char * szName)
{
#ifndef STOP_DEBUG
	DebugAssert(bDebugIsConst == true);
#endif

	IVariable* var = NULL;
	variables_hash.Find(szName, var);

	if (var == NULL)
	{
		return NULL;
	}

	IDirect3DBaseTexture9** retVal = (IDirect3DBaseTexture9**)var->getVariableInplacePointer();
	return retVal;
}

boolVector32* VariablesDB::getBool32Output (const char * szName)
{
#ifndef STOP_DEBUG
	DebugAssert(bDebugIsConst == true);
#endif

	IVariable* var = NULL;
	variables_hash.Find(szName, var);

	if (var == NULL)
	{
		return NULL;
	}

	boolVector32* retVal = (boolVector32*)var->getVariableInplacePointer();
	return retVal;
}

Vector4* VariablesDB::getVectorOutput (const char * szName)
{
#ifndef STOP_DEBUG
	DebugAssert(bDebugIsConst == true);
#endif

	IVariable* var = NULL;
	variables_hash.Find(szName, var);

	if (var == NULL)
	{
		return NULL;
	}

	Vector4* retVal = (Vector4*)var->getVariableInplacePointer();
	return retVal;
}

void VariablesDB::ResetAllTextureVariables()
{
	for (DWORD i = 0; i < texture_variables.Size(); i++)
	{
		texture_variables[i]->ResetTexture();
	}
}

void VariablesDB::ApplyFogVariables (ShaderFogVariables& vars)
{
	if (_fFogHeightDensity)
	{
		_fFogHeightDensity->SetFloat(vars.fHeight_Density);
	}

	if (_fFogHeightMin)
	{
		_fFogHeightMin->SetFloat(vars.fHeight_Min);
	}

	if (_fFogHeightMax)
	{
		_fFogHeightMax->SetFloat(vars.fHeight_Max);
	}

	if (_fFogHeightDistanceInv)
	{
		_fFogHeightDistanceInv->SetFloat(vars.fHeight_DistanceInv);
	}

	if (_fFogDensity)
	{
		_fFogDensity->SetFloat(vars.fDistance_Density);
	}


	if (_fFogDistanceMin)
	{
		_fFogDistanceMin->SetFloat(vars.fDistance_Min);
	}


	if (_fFogDistanceMax)
	{
		_fFogDistanceMax->SetFloat(vars.fDistance_Max);
	}


	if (_fFogDistanceMinSquared)
	{
		_fFogDistanceMinSquared->SetFloat(vars.fDistance_MinSquared);
	}

	if (_fFogDistanceLen)
	{
		_fFogDistanceLen->SetFloat(vars.fDistance_DistanceInv);
	}

	if (_cFogColor)
	{
		_cFogColor->SetVector4(vars.fogColor.v4);
	}
}

void VariablesDB::ApplyLightingVariables30 (const ShaderLightingVariables& vars, dword lightCount)
{
	
	boolVector32 lightMask;


	if (lightCount == 0)
	{
		lightMask.value = 0x0;
		_bLightEnabled30_count->SetBool32(lightMask);
		return;
	}

	//Надо сформировать масочку, в один dword
	Assert(MAX_LIGHTS_IN_30_MODEL <= 32);

	lightMask.value = (1 << lightCount)-1;
	_bLightEnabled30_count->SetBool32(lightMask);


	_fPointLightParams_array->SetVector4Array((const Vector4 *)vars.pointLights.vPointLightsParams, lightCount);
	_vPointLightPos30_array->SetVector4Array((const Vector4 *)vars.pointLights.vPointLightPos, lightCount);
	_vPointLightColor30_array->SetVector4Array((const Vector4 *)vars.pointLights.vPointLightColor, lightCount);
	_vPointLightColorSpecular30_array->SetVector4Array((const Vector4 *)vars.pointLights.vPointLightColorSpecular, lightCount);


}

void VariablesDB::ApplyEnvironmentLightingVariables (const ShaderLightingVariables& vars, Vector4 vCamDirRelativeWorld)
{
	if (_vAmbientLight)
	{
		_vAmbientLight->SetVector4(vars.ambientLight.vAmbientLight.v4);
	}

	if (_vAmbientLightSpecular)
	{
		_vAmbientLightSpecular->SetVector4(vars.ambientLight.vAmbientLightSpecular.v4);
	}


	if (_vGlobalLightDirection)
	{
		_vGlobalLightDirection->SetVectorAsVector4(vars.dirLight.vGlobalLightDirection);
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

		_vGlobalLightDirectionSpecular->SetVector4(specDir);
	}



	if (_vGlobalLightColor)
	{
		_vGlobalLightColor->SetVector4(vars.dirLight.vGlobalLightColor.v4);
	}

	if (_vGlobalLightColorSpecular)
	{
		_vGlobalLightColorSpecular->SetVector4(vars.dirLight.vGlobalLightColorSpecular.v4);
	}

	if (_vGlobalLightBackColor)
	{
		_vGlobalLightBackColor->SetVector4(vars.dirLight.vGlobalLightBackColor.v4);
	}
}


void VariablesDB::ApplyStandartVariables (ShaderStandartVariables & vars)
{
	if (_mWorldViewProj)
	{
		_mWorldViewProj->SetMatrix(*vars.mWorldViewProj);
	}

	if (_mWorld)
	{
		_mWorld->SetMatrix(*vars.mWorld);
	}

	if (_vWorldPos)
	{
		_vWorldPos->SetVector(vars.mWorld->pos);
	}


	if (_mView)
	{
		_mView->SetMatrix(*vars.mView);
	}

	if (_mProjection)
	{
		_mProjection->SetMatrix(*vars.mProjection);
	}

	if (_mInverseView)
	{
		_mInverseView->SetMatrix(*vars.mInverseView);
	}

	if (_vCamPosRelativeWorld)
	{
		_vCamPosRelativeWorld->SetVector(vars.vCamPosRelativeWorld->v);
	}

	if (_vCamDirRelativeWorld)
	{
		_vCamDirRelativeWorld->SetVector(vars.vCamDirRelativeWorld->v);
	}

	if (_vCamPos)
	{
		Vector camPos_ObjectSpace;
		camPos_ObjectSpace = vars.vCamPosRelativeWorld->v * *vars.mInverseWorld;
		_vCamPos->SetVector(camPos_ObjectSpace);
	}




}

void VariablesDB::BindStandartVariables ()
{
	_mWorldViewProj = getShaderVariable("mWorldViewProj", _FL_);
	_mWorld = getShaderVariable("mWorld", _FL_);
	_mView = getShaderVariable("mView", _FL_);
	_mProjection = getShaderVariable("mProjection", _FL_);
	_mInverseView = getShaderVariable("mInverseView", _FL_);
	_vWorldPos = getShaderVariable("vWorldPos", _FL_);
	_vCamPosRelativeWorld = getShaderVariable("vCamPosRelativeWorld", _FL_);
	_vCamDirRelativeWorld = getShaderVariable("vCamDirRelativeWorld", _FL_);
	_vCamPos = getShaderVariable("vCamPos", _FL_);

	_fFogHeightDensity = getShaderVariable("fFogHeightDensity", _FL_);
	_fFogHeightMin = getShaderVariable("fFogHeightMin", _FL_);
	_fFogHeightMax = getShaderVariable("fFogHeightMax", _FL_);
	_fFogHeightDistanceInv = getShaderVariable("fFogHeightDistanceInv", _FL_);
	_fFogDensity = getShaderVariable("fFogDensity", _FL_);
	_fFogDistanceMin = getShaderVariable("fFogDistanceMin", _FL_);
	_fFogDistanceMax = getShaderVariable("fFogDistanceMax", _FL_);
	_fFogDistanceMinSquared = getShaderVariable("fFogDistanceMinSquared", _FL_);
	_fFogDistanceLen = getShaderVariable("fFogDistanceLen", _FL_);
	_cFogColor = getShaderVariable("cFogColor", _FL_);


	_vAmbientLight = getShaderVariable("vAmbientLight", _FL_);
	_vAmbientLightSpecular = getShaderVariable("vAmbientLightSpecular", _FL_);
	
	_vGlobalLightColor = getShaderVariable("vGlobalLightColor", _FL_);
	_vGlobalLightDirection = getShaderVariable("vGlobalLightDirection", _FL_);
	_vGlobalLightDirectionSpecular = getShaderVariable("vGlobalLightDirectionSpec", _FL_);

	_vGlobalLightBackColor = getShaderVariable("vGlobalLightBackColor", _FL_);
	_vGlobalLightColorSpecular = getShaderVariable("vGlobalLightColorSpecular", _FL_);


	_bLightEnabled30_count = getShaderVariable("bLightEnabled30", _FL_);
	_fPointLightParams_array = getShaderVariable("fPointLightParams", _FL_);
	_vPointLightPos30_array = getShaderVariable("vPointLightPos30", _FL_);
	_vPointLightColor30_array = getShaderVariable("vPointLightColor30", _FL_);
	_vPointLightColorSpecular30_array = getShaderVariable("vPointLightColorSpecular30", _FL_);



}


//=============================================================================================
//=============================================================================================
//=============================================================================================
//=============================================================================================
//=============================================================================================
/*


VariableHandle::VariableHandle()
{
	db = NULL;
	type = VT_UNKNOWN;
	dwElementsCount = 0;
	dwSlotsInPool = 0;
	ptrInPool = NULL;
}

VariableHandle::~VariableHandle()
{

}

void VariableHandle::Init (VariablesDB * _db, VariableType _type, DWORD _dwElementsCount, DWORD _dwSlotsInPool, void * _ptrInPool)
{
	db = _db;
	type = _type;
	dwElementsCount = _dwElementsCount;
	dwSlotsInPool = _dwSlotsInPool;
	ptrInPool = _ptrInPool;
}

void VariableHandle::SetFloat(float val)
{
	Assert (type == VT_FLOAT && dwElementsCount == 1);
	db->SetFloat(ptrInPool, val);
}

void VariableHandle::SetBool32(boolVector32 val)
{
	Assert (type == VT_BOOL && dwElementsCount == 1);
	db->SetBool32(ptrInPool, val);
}

void VariableHandle::SetTexture(IBaseTexture* val)
{
	Assert (type == VT_TEXTURE && dwElementsCount == 1);
	db->SetTexture(ptrInPool, val);
}

void VariableHandle::SetMatrix(const Matrix& val)
{
	Assert (type == VT_MATRIX4x4 && dwElementsCount == 1);
	db->SetMatrix(ptrInPool, val);
}

void VariableHandle::SetVector(const Vector & val)
{
	Assert (type == VT_VECTOR3 && dwElementsCount == 1);
	db->SetVector(ptrInPool, val);
}

void VariableHandle::SetVector4(const Vector4 & val)
{
	Assert (type == VT_VECTOR4 && dwElementsCount == 1);
	db->SetVector4(ptrInPool, val);
}

void VariableHandle::SetVectorAsVector4(const Vector & val)
{
	Assert (type == VT_VECTOR4 && dwElementsCount == 1);
	db->SetVectorAsVector4(ptrInPool, val);
}


void VariableHandle::SetVector4Array(const Vector4* val, dword dwCount)
{
	Assert (type == VT_VECTOR4 && dwElementsCount >= 1);
	Assert(dwCount <= dwElementsCount);

	db->SetVector4Array(ptrInPool, val, dwCount);
}


void VariableHandle::ResetTexture()
{

}

void * VariableHandle::getVariableInplacePointer ()
{
	return ptrInPool;
}

void VariableHandle::convertOffsetToPointer(Vector4 * v4Pool, boolVector32 * bPool, IDirect3DBaseTexture9** tPool)
{
	Assert(v4Pool != NULL);
	Assert(bPool != NULL);
	Assert(tPool != NULL);

	DWORD dwOffset = (DWORD)ptrInPool;

	if (type == VT_BOOL)
	{
		ptrInPool = (bPool + dwOffset);
	} else
	{
		if (type == VT_TEXTURE)
		{
			ptrInPool = (tPool + dwOffset);
		} else
		{
			ptrInPool = (v4Pool + dwOffset);
		}
	}


	Assert(ptrInPool != NULL);
}

*/

#endif