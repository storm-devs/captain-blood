
#ifndef FX_SHADERS_MANAGER
#define FX_SHADERS_MANAGER


#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\templates.h"
#include "..\..\..\common_h\render.h"
#include "IncludeHandler.h"



class IVariable;
class Shader;
class ShaderVariable;

#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif




#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#include "..\VariablesDB.h"
#include "..\batch\CommandBufferExecutor.h"


//JOKER TODO:
class texNameFinder
{
	struct texKey
	{
		string texName;
		IDirect3DBaseTexture9* ptr;
		ID3DXEffect * pEffect;
		D3DXHANDLE paramHandle;
	};
	array<texKey> texturesMap;

public:

	texNameFinder() : texturesMap (_FL_)
	{

	}
	
	~texNameFinder()
	{

	}


	void Init(ID3DXEffect* pEffect, IDirect3DDevice9* dev)
	{
		D3DXEFFECT_DESC fxDesc;
		pEffect->GetDesc(&fxDesc);
		D3DXPARAMETER_DESC paramDesc;
		for (UINT i = 0; i < fxDesc.Parameters; i++)
		{
			D3DXHANDLE paramHandle = pEffect->GetParameter(NULL, i);
			pEffect->GetParameterDesc(paramHandle, &paramDesc);

			if (paramDesc.Type == D3DXPT_TEXTURE ||
				paramDesc.Type == D3DXPT_TEXTURE1D ||
				paramDesc.Type == D3DXPT_TEXTURE2D ||
				paramDesc.Type == D3DXPT_TEXTURE3D ||
				paramDesc.Type == D3DXPT_TEXTURECUBE)
			{

				//Создаем текстуры и ставим в каждый параметр, что бы потом можно было связать
				//sampler->texture посмотрев с сэмплере указатель на IDirect3DTexture9
				IDirect3DTexture9* tex = NULL; 
				dev->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tex, NULL);

				if (tex == NULL)
				{
					api->Trace("can't create texture 8x8xA8R8G8B8 in default pool !");
					return;
				}

				//api->Trace("tex: '%s' = 0x%08X", paramDesc.Name, tex);
				pEffect->SetTexture(paramHandle, tex);

				//Заносим в map...
				texKey & key = texturesMap[texturesMap.Add()];
				key.texName = paramDesc.Name;
				key.ptr = tex;
				key.pEffect = pEffect;
				key.paramHandle = paramHandle;

				//printf ("texture: %s = 0x%08X\n", paramDesc.Name, (DWORD)tex);
			}

		}
	}


	void Release()
	{
		for (dword i = 0; i < texturesMap.Size(); i++)
		{
			texturesMap[i].pEffect->SetTexture(texturesMap[i].paramHandle, NULL);
			texturesMap[i].ptr->Release();
		}

		texturesMap.DelAll();
	}


	const char* findTextureVariableName (IDirect3DBaseTexture9* tex)
	{
		for (dword i = 0; i < texturesMap.Size(); i++)
		{
			if (texturesMap[i].ptr == tex)
			{
				return texturesMap[i].texName.c_str();
			}
		}

		return NULL;
	}


};


#endif

class ShadersManager
{

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	texNameFinder texFinder;
	batchExecutor batchRunner;
#endif



	

	bool bErrorsWhileValidation;

	dword dwSearchTime;

	RenderIncludeHandler m_includeHandler;


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	htable<precompiledBatch*> shadersArray;
	array<precompiledBatch*> shadersArrayLin;
#else
	htable<Shader*> shadersArray;
	array<Shader*> shadersArrayLin;
#endif


	array<string> fx_files;

	long FindInFiles (const char* fileName);
	void MoveToUp (const char* fileName);


	void Compile (const char * dir, const char * objectFile);

	long systemShadersCount;

	string shadersRootDir;

public:

	DWORD dwPCBtime;


	ShadersManager();
	~ShadersManager();

	bool IsNeedBuild();
	void Build ();
	//void BuildSystemShaders ();

	const char* GetShadersRootDir ();


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)


	__forceinline VariablesDB* getVarDatabase()
	{
		return &varsDatabase;
	}

	__forceinline precompiledBatch* FindShaderByName (const char* shaderName)
	{
		dword dwTime = 0;
		RDTSC_B(dwTime);

		precompiledBatch* shdr = shadersArray[shaderName];

		RDTSC_E(dwTime);

		dwSearchTime+=dwTime;

		return shdr;
	}

	__forceinline precompiledBatch* _GetShaderByIndex (dword dwIndex)
	{
		if (shadersArrayLin.Size() <= 0) return NULL;

		precompiledBatch* shdr = shadersArrayLin[dwIndex];
		return shdr;

	}



#else

	__forceinline Shader* FindShaderByName (const char* shaderName)
	{
		dword dwTime = 0;
		RDTSC_B(dwTime);

		Shader* shdr = shadersArray[shaderName];

		RDTSC_E(dwTime);

		dwSearchTime+=dwTime;

		return shdr;
	}

	__forceinline Shader* _GetShaderByIndex (dword dwIndex)
	{
		if (shadersArrayLin.Size() <= 0) return NULL;

		Shader* shdr = shadersArrayLin[dwIndex];
		return shdr;

	}


#endif


	IVariable * getShaderVariable (const char* shaderName, const char * srcFile, long srcLine);

	void StartFrame();
	dword GetTimeForSearch();


	void Destroy();


	void InsertShaderName (const char* szName, bool bLast);

	static ShadersManager* manager;

	void OnResetDevice();
	void OnLostDevice();



	void SetD3D(IDirect3DDevice9* device);


	void ResetAllTextureVariables();

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	batchExecutor* getBatchExecutor()
	{
		return &batchRunner;
	}
#endif


protected:

	IDirect3DDevice9* d3d;


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	VariablesDB varsDatabase;

#else
	objectsPool<ShaderVariable, 512> varPool;

	array <ShaderVariable*> texture_variables;
	array <ShaderVariable*> variables;
	htable <ShaderVariable*> variables_hash;
#endif

	void CreateShader (char* buffer, dword size, const char* szFileName, bool bWriteObjFile, const char* objFileName);
	void CreateVaribles (ID3DXEffect* pEffect);

	

};




#endif