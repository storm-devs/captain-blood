

#include "ShaderVar.h"
#include "..\Render.h"


//#define RENDER_PANIC_CHECK_PARAMS


#ifdef RENDER_PANIC_CHECK_PARAMS
	__forceinline void _CheckFloat (float f)
	{
		Assert(!_isnan(f));
		Assert(fabsf(f) < 100000.0f);
	}
#endif




ShaderVariable::ShaderVariable()
{
}

void ShaderVariable::Init(ID3DXEffect* pEffect, D3DXHANDLE var_handle, const char* szName)
{
	pDXTexture = NULL;

	activeTexture = NULL;

	m_Name = szName;

	m_effect = pEffect;
	m_handle = var_handle;
}

ShaderVariable::~ShaderVariable()
{
	if (activeTexture)
	{
		activeTexture->Release();
		activeTexture = NULL;
	}
}

const char*  ShaderVariable::GetName ()
{
	return m_Name.c_str();
}




void ShaderVariable::SetFloat(float val)
{
#ifdef RENDER_PANIC_CHECK_PARAMS
	_CheckFloat(val);
#endif

	m_effect->SetFloat(m_handle, val);
}

void ShaderVariable::SetBool32(boolVector32 val)
{
	static BOOL tempBoolArray[32];
	for (DWORD i = 0; i < 32; i++)
	{
		tempBoolArray[i] = val.get(i);
	}

	m_effect->SetBoolArray(m_handle, tempBoolArray, 32);
}



void ShaderVariable::ResetTexture()
{
	if (activeTexture)
	{
		activeTexture->Release();
		activeTexture = NULL;
	}

	m_effect->SetTexture(m_handle, NULL);
}



void ShaderVariable::SetTexture(IBaseTexture* val)
{
	Assert(val);

	if (activeTexture)
	{
		activeTexture->Release();
		activeTexture = NULL;
	}

	activeTexture = val;

	activeTexture->AddRef();

	pDXTexture = (IDirect3DBaseTexture9*)val->GetBaseTexture();
	m_effect->SetTexture(m_handle, pDXTexture);
}

void ShaderVariable::SetMatrix(const Matrix& val)
{
#ifdef RENDER_PANIC_CHECK_PARAMS
	_CheckFloat(val.m[0][0]);
	_CheckFloat(val.m[0][1]);
	_CheckFloat(val.m[0][2]);
	_CheckFloat(val.m[0][3]);

	_CheckFloat(val.m[1][0]);
	_CheckFloat(val.m[1][1]);
	_CheckFloat(val.m[1][2]);
	_CheckFloat(val.m[1][3]);

	_CheckFloat(val.m[2][0]);
	_CheckFloat(val.m[2][1]);
	_CheckFloat(val.m[2][2]);
	_CheckFloat(val.m[2][3]);

	_CheckFloat(val.m[3][0]);
	_CheckFloat(val.m[3][1]);
	_CheckFloat(val.m[3][2]);
	_CheckFloat(val.m[3][3]);
#endif

	m_effect->SetMatrix(m_handle, val);
}

void ShaderVariable::SetVector(const Vector & val)
{
#ifdef RENDER_PANIC_CHECK_PARAMS
	_CheckFloat(val.x);
	_CheckFloat(val.y);
	_CheckFloat(val.z);
#endif

	Vector4 v(val);
	m_effect->SetVector(m_handle, (D3DXVECTOR4 *)&v);
}

void ShaderVariable::SetVector4(const Vector4 & val)
{
#ifdef RENDER_PANIC_CHECK_PARAMS
	_CheckFloat(val.x);
	_CheckFloat(val.y);
	_CheckFloat(val.z);
	_CheckFloat(val.w);
#endif


	m_effect->SetVector(m_handle, (D3DXVECTOR4 *)&val);
}



void ShaderVariable::SetVector4Array(const Vector4* val, dword dwCount)
{
#ifdef RENDER_PANIC_CHECK_PARAMS
	for (dword i = 0; i < dwCount; i++)
	{
		_CheckFloat(val[i].x);
		_CheckFloat(val[i].y);
		_CheckFloat(val[i].z);
		_CheckFloat(val[i].w);
	}
#endif

	m_effect->SetVectorArray(m_handle, (D3DXVECTOR4 *)val, dwCount);
}



