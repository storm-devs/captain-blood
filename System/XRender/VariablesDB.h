#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#ifndef __COMMAND_BUFFER_GLOBAL_VARIABLES_DATABASE_H
#define __COMMAND_BUFFER_GLOBAL_VARIABLES_DATABASE_H


#include <stdio.h>
#include <xtl.h>
#include <xgraphics.h>
#include <xboxmath.h>


#include "..\..\common_h\core.h"
#include "..\..\common_h\defines.h"
#include "..\..\common_h\math3D.h"
#include "..\..\common_h\templates.h"
#include "..\..\common_h\Render.h"

//for structs ShaderStandartVariables
#include "shaders\Shader.h"


//class VariablesDB;




//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================

class VariablesDB
{

private:

#ifndef STOP_DEBUG
	bool bDebugIsConst;
	void DebugAssert(bool bExpr);
#endif

	array<Vector4> vector4Pool;
	array<boolVector32> boolPool;
	array<IDirect3DBaseTexture9*> texturesAddrPool;

	array<IVariable*> texture_variables;
	array<IVariable*> variables;
	htable<IVariable*> variables_hash; 

	objectsPool<IVariable, 512> varPool;

private:

	IVariable* _mWorldViewProj;
	IVariable* _mWorld;
	IVariable* _mView;
	IVariable* _mProjection;
	IVariable* _mInverseView;
	IVariable* _vWorldPos;
	IVariable* _vCamPosRelativeWorld;
	IVariable* _vCamDirRelativeWorld;
	IVariable* _vCamPos;


	IVariable* _fFogHeightDensity;
	IVariable* _fFogHeightMin;
	IVariable* _fFogHeightMax;
	IVariable* _fFogHeightDistanceInv;
	IVariable* _fFogDensity;
	IVariable* _fFogDistanceMin;
	IVariable* _fFogDistanceMax;
	IVariable* _fFogDistanceMinSquared;
	IVariable* _fFogDistanceLen;
	IVariable* _cFogColor;


	IVariable* _vAmbientLight;
	IVariable* _vAmbientLightSpecular;
	
	IVariable* _vGlobalLightColorSpecular;
	IVariable* _vGlobalLightColor;
	IVariable* _vGlobalLightDirection;
	IVariable* _vGlobalLightDirectionSpecular;
	IVariable* _vGlobalLightBackColor;



	IVariable* _bLightEnabled30_count;
	IVariable* _fPointLightParams_array;
	IVariable* _vPointLightPos30_array;
	IVariable* _vPointLightColor30_array;
	IVariable* _vPointLightColorSpecular30_array;




public:


	VariablesDB();
	~VariablesDB();

	void Init ();


	void AddVariable (const char * szVariableName, ID3DXEffect* pEffect, D3DXHANDLE paramHandle);

/*
	virtual void SetFloat(void * ptrInPool, float val);
	virtual void SetBool32(void * ptrInPool, boolVector32 val);

	virtual void SetTexture(void * ptrInPool, IBaseTexture* val);
	virtual void SetMatrix(void * ptrInPool, const Matrix& val);
	virtual void SetVector(void * ptrInPool, const Vector & val);
	virtual void SetVector4(void * ptrInPool, const Vector4 & val);
	virtual void SetVectorAsVector4(void * ptrInPool, const Vector & val);

	virtual void SetVector4Array(void * ptrInPool, const Vector4* val, dword dwCount);
*/
	

	IVariable * getShaderVariable (const char* shaderName, const char * srcFile, long srcLine);


	IDirect3DBaseTexture9** getTextureOutput (const char * szName);
	boolVector32* getBool32Output (const char * szName);
	Vector4* getVectorOutput (const char * szName);

	void BindStandartVariables ();

#ifndef STOP_DEBUG
	void Debug_MakeItConst();
#endif
	
	
	void ApplyStandartVariables (ShaderStandartVariables & vars);
	void ApplyFogVariables (ShaderFogVariables& vars);
	void ApplyLightingVariables30 (const ShaderLightingVariables& vars, dword lightCount);
	void ApplyEnvironmentLightingVariables (const ShaderLightingVariables& vars, Vector4 vCamDirRelativeWorld);

	void ResetAllTextureVariables();

};





#endif 

#endif