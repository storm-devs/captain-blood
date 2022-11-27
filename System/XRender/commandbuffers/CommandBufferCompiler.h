#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)


#ifndef _COMMAND_BUFFER_COMPILER___
#define _COMMAND_BUFFER_COMPILER___


#include "..\..\..\common_h\core.h"
#include "..\GraphicsApi.h"

class IFileService;
class precompiledBatch;

class CommandBufferCompiler
{


	struct renderState
	{
		D3DRENDERSTATETYPE state;
		DWORD val;
	};



	struct microcode
	{
		char* pMicrocodeBody;
		XGMICROCODESHADERPARTS parts;

		microcode()
		{
			pMicrocodeBody = NULL;
		}

		~microcode()
		{
			if (pMicrocodeBody)
			{
				delete [] pMicrocodeBody;
				pMicrocodeBody = NULL;
			}
		}

		char* Allocate (dword dwSize)
		{
			pMicrocodeBody = new char[dwSize];
			return pMicrocodeBody;
		}

	};


	struct texture
	{
		D3DTEXTUREFILTERTYPE minFilter;
		D3DTEXTUREFILTERTYPE magFilter;
		D3DTEXTUREFILTERTYPE mipFilter;
		D3DTEXTUREADDRESS clampU;
		D3DTEXTUREADDRESS clampV;

		DWORD dwStage;
		string variableName;

		texture()
		{
			dwStage = 0;
			minFilter = D3DTEXF_LINEAR;
			magFilter = D3DTEXF_LINEAR;
			mipFilter = D3DTEXF_LINEAR;
			clampU = D3DTADDRESS_WRAP;
			clampV = D3DTADDRESS_WRAP;
		}
	};


public:

	enum ConstantType
	{
		CT_UNKNOWN = 0,

		//матрица 4x4
		CT_MATRIX4x4 = 1,

		//float4
		CT_FLOAT4 = 2,

		//float3 -> float4
		CT_FLOAT3 = 3,

		//float -> float4
		CT_FLOAT = 4,

		//bool
		CT_BOOL = 5,

		//int
		CT_INT4 = 6,


		CT_FORCE_DWORD               = 0x7fffffff, /* force 32-bit size enum */
	};

private:

	struct shaderConstant
	{
		bool bPredefinedInShaderBody;
		string szName;
		CommandBufferCompiler::ConstantType type;
		DWORD dwRegisterIndex;
		DWORD dwRegisterCounts;
		DWORD dwElements;

		shaderConstant()
		{
			bPredefinedInShaderBody = false;
			type = CT_UNKNOWN;
			dwRegisterIndex = 0;
			dwRegisterCounts = 0;
			dwElements = 0;
		}

		shaderConstant(const char * _name, CommandBufferCompiler::ConstantType _type, DWORD _dwRegisterIndex, DWORD _dwRegisterCounts, DWORD _dwElements)
		{
			bPredefinedInShaderBody = false;
			szName = _name;
			type = _type;
			dwRegisterIndex = _dwRegisterIndex;
			dwRegisterCounts = _dwRegisterCounts;
			dwElements = _dwElements;
		}
	};


	struct pass
	{
		array<renderState> r_states;

		array<shaderConstant> vs_consts;
		array<shaderConstant> ps_consts;

		array<texture> textures;

		CONST DWORD *pPixelShaderFunc;
		CONST DWORD *pVertexShaderFunc;
		
		microcode microcodeVS;
		microcode microcodePS;

		IDirect3DVertexShader9 *pBoundedVS;
		IDirect3DPixelShader9 * pBoundedPS;
		

		pass() : r_states(_FL_),
				 textures(_FL_),
				 vs_consts(_FL_),
				 ps_consts(_FL_)
		{
			pBoundedVS = NULL;
			pBoundedPS = NULL;
		}

		~pass()
		{
			if (pBoundedVS)
			{
				pBoundedVS->Release();
			}
			pBoundedVS = NULL;

			if (pBoundedPS)
			{
				pBoundedPS->Release();
			}
			pBoundedPS = NULL;
		}

	};



	pass shaderStates;



	


	enum MicrocodeType
	{
		MT_PIXEL_SHADER = 0,
		MT_VERTEX_SHADER = 1,

		MT_FORCE_DWORD = 0x7fffffff
	};

	string techName;
	IFileService * pFS;


	D3DCommandBuffer* commandBuffer;


	DWORD dwHeaderSize;
	DWORD dwPhysicalSize;
	DWORD dwInitializationSize;

	DWORD dwUsedSpaceInCmdBuffer;


	bool ExtractConstants (CONST DWORD *pShaderFunction, MicrocodeType type);
	

	void ExtractMicrocode(CONST DWORD *pShaderFunction, MicrocodeType type);


	void ResetToDefaultStates(D3DDevice* pCommandBufferDevice);

	void ParsePredefinedConstants (const char* szShaderBody, array<shaderConstant> & shaderConsts, bool bPixelShader);

public:

	CommandBufferCompiler();
	~CommandBufferCompiler();

	void SetName (const char* szTechName);



	void ExtractPixelShader(CONST DWORD *pShaderFunction);
	void ExtractVertexShader(CONST DWORD *pShaderFunction);

	void ExtractRenderState (D3DRENDERSTATETYPE state, DWORD dwVal, bool bLog);
	void ExtractSamplerState (DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD dwVal);

	
	void PatchShadersAndDumpMicrocode(IDirect3DVertexDeclaration9 * pVDecl, array<DWORD> & vertexDeclStrides);

	void Finalize(D3DDevice* pCommandBufferDevice, precompiledBatch * pBatch);


	void ExtractTexture (DWORD dwIndex, const char* texvariableName);


	

	

};


#endif


#endif