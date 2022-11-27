

#ifndef FX_SHADER_VARIABLE
#define FX_SHADER_VARIABLE


#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\Render.h"
#include "..\GraphicsApi.h"


class Shader;



class ShaderVariable : public IVariable
{
	string m_Name;

	ID3DXEffect* m_effect;
	D3DXHANDLE m_handle;

	IBaseTexture * activeTexture;
	IDirect3DBaseTexture9* pDXTexture;



public:

	void SetFileLine(const char* fileName, dword line);

	long GetLine();
	const char* GetFile ();
	

	ShaderVariable();
	virtual ~ShaderVariable();

	void Init(ID3DXEffect* pEffect, D3DXHANDLE var_handle, const char* szName);

	const char* GetName ();


	virtual void	SetFloat(float val);
	//virtual void	SetDword(dword val);

	virtual void	SetBool32(boolVector32 val);

	virtual void	ResetTexture();

	virtual void	SetTexture(IBaseTexture* val);
	virtual void	SetMatrix(const Matrix& val);
	virtual void	SetVector(const Vector & val);
	virtual void	SetVector4(const Vector4 & val);


	virtual void	SetVector4Array(const Vector4* val, dword dwCount);
	//virtual void	SetBool32Array(const boolVector32* val, dword dwCount);

};

#endif

