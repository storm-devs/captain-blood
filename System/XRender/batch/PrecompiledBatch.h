#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#ifndef PRECOMPILED_BATCH_H____
#define PRECOMPILED_BATCH_H____

#include <stdio.h>
#include <xtl.h>
#include <xgraphics.h>
#include <xboxmath.h>


#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\defines.h"
#include "..\..\..\common_h\math3D.h"
#include "..\..\..\common_h\templates.h"
#include "..\..\..\common_h\render.h"
#include "..\commandBuffers\CommandBufferCompiler.h"

#include "gpu_structs.h"


#define MAX_STREAMS_COUNT 3
#define MAX_TEXTURES_COUNT 6
#define MAX_ALU_CONST_COUNT 512

//В каждом dword их 32 штуки, 4 dword первые vs, 4 еще dword это ps
#define MAX_ALU_CONST_COUNT_BOOL 8  


#define GPU_INVALID_OFFSET 0xFFFFFFFF

struct TStateHack;

class VariablesDB;

class precompiledBatch
{
private:

	DWORD dwBytesAllocated;

	VariablesDB * m_pDB;

	bool bTemp_VertexShaderNotSet;
	bool bTemp_PixelShaderNotSet;

private:

	void TracePrimitiveType (DWORD dwPrimType);

	void ParseVertexFetchConst(DWORD dwStreamIndex, DWORD dwRegisterElementIndex, DWORD regData);
	void ParseTextureFetchConst(DWORD dwRegIndex, DWORD dwRegisterElementIndex, DWORD regData);

	void ParseType0Packet (const DWORD * dataStream, DWORD count, DWORD gpuRegisterBase);
	void ParseType3Packet (DWORD opCode, const DWORD * dataStream, DWORD count);

	void parseCommandBuffer(BYTE * memoryPtr);

public:

	precompiledBatch(const char * szShdrName);
	~precompiledBatch();

	DWORD getBytesAllocated();

	static void DeleteTraceFile();
	static void Trace (const char * str, ...);

	void debugOutCommandBuffer(BYTE * bDataPtr, bool bDumpToLog);

	void Cleanup();


	void offlineCreate (D3DCommandBuffer * cmdPrecompiledBuffer, D3DVertexShader* vertexShaderToCreateFixup, D3DPixelShader* pixelShaderToCreateFixup, bool bDumpToLog);


	void SetVertexShaderPtr (IDirect3DVertexShader9 *pVertexShader);
	void SetPixelShaderPtr (IDirect3DPixelShader9 *pPixelShader);
	void Freeze();

	//GPUCULL_NONE_FRONTFACE_CCW, GPUCULL_BACK_FRONTFACE_CCW, GPUCULL_BACK_FRONTFACE_CW
	void SetCullMode (GPUCULL cullMode);


	void SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, DWORD dwOffsetInBytes);


	void SetDrawIndexedParameters (GPUPRIMTYPE primType, IDirect3DIndexBuffer9 *pIndexData, DWORD trianglesCount, dword StartIndex);
	void SetDrawParameters (GPUPRIMTYPE primType, DWORD trianglesCount, DWORD dwStartVertex);


	void ApplyStateHacksInplace(IDirect3DDevice9* device, TStateHack * hacks, bool bDisableHacks);
	void RestoreStateHacksInplace(IDirect3DDevice9* device, TStateHack * hacks, bool bDisableHacks);
	void DynamicDraw_ApplyStateHacks(IDirect3DDevice9* device, TStateHack * hacks, bool bDisableHacks);
	void DynamicDraw_RestoreStateHacks(IDirect3DDevice9* device, TStateHack * hacks, bool bDisableHacks);
	



	void SetColorMask (DWORD dwMask);
	void SetZEnable(bool bDepthEnabled, bool bDepthWriteEnabled);
	void SetBlendOp (GPUBLEND dwSrcBlendOp, GPUBLEND dwDestBlendOp);
	void SetHyperZEnable (bool bHyperZEnable, bool bHyperZWriteEnable);


	__forceinline void SetAluConstVector4 (void * __restrict fixupPtrInCommandBuffer, const Vector4 * __restrict vector4Data, DWORD dwDataCount)
	{
		//float * aluConstComponent = (float *)(dwCommandBufferData + dwFixupOffsetInCB);
		Vector4 * __restrict aluConstComponent = (Vector4 *)fixupPtrInCommandBuffer;

		for (DWORD i = 0; i < dwDataCount; i++)
		{
			*aluConstComponent = *vector4Data;
			aluConstComponent++;
			vector4Data++;
		}
	}

	__forceinline void SetAluConstsBool32 (void * __restrict fixupPtrInCommandBuffer, DWORD dwConstants0_31)
	{
		//DWORD * aluConstBoolComponent = (DWORD *)(dwCommandBufferData + dwFixupOffsetInCB);
		DWORD * __restrict aluConstBoolComponent = (DWORD *)fixupPtrInCommandBuffer;

		*aluConstBoolComponent = dwConstants0_31;
	}

	__forceinline void SetTexture (void * __restrict fixupPtrInCommandBuffer, IDirect3DBaseTexture9 *pTexture, GPUMINMAGFILTER minFilter, GPUMINMAGFILTER magFilter, GPUMIPFILTER mipFilter, GPUCLAMP clampU, GPUCLAMP clampV)
	{
		if (pTexture == NULL)
		{
			pTexture = pWhiteTexture;
		}

		GPUTEXTURE_FETCH_CONSTANT * __restrict TextureStageData = (GPUTEXTURE_FETCH_CONSTANT*)fixupPtrInCommandBuffer;
		*TextureStageData = pTexture->Format;

		TextureStageData->MinFilter = minFilter;
		TextureStageData->MagFilter = magFilter;
		TextureStageData->MipFilter = mipFilter;
		TextureStageData->ClampX = clampU;
		TextureStageData->ClampY = clampV;
		TextureStageData->BaseAddress = (GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(pTexture->Format.BaseAddress << 12)) >> 12);
		TextureStageData->MipAddress = (GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(pTexture->Format.MipAddress << 12)) >> 12);
	}


	__forceinline void PrefetchCommandBuffer()
	{
		//8 линеек в кеш грузим...
		__dcbt((128*0), dwCommandBufferData);

		__dcbt((128*1), dwCommandBufferData);

		__dcbt((128*2), dwCommandBufferData);

		__dcbt((128*3), dwCommandBufferData);

		__dcbt((128*4), dwCommandBufferData);

		__dcbt((128*5), dwCommandBufferData);

		__dcbt((128*6), dwCommandBufferData);

		__dcbt((128*7), dwCommandBufferData);
	}




	void AddTextureSampler (const char * szName, DWORD dwSamplerIdx, D3DTEXTUREFILTERTYPE minFilter, D3DTEXTUREFILTERTYPE magFilter, D3DTEXTUREFILTERTYPE mipFilter, D3DTEXTUREADDRESS clampU, D3DTEXTUREADDRESS clampV);
	
	void AddPixelShaderConstVector (const char * szName, DWORD dwIndex, DWORD dwRegCount);
	void AddVertexShaderConstVector (const char * szName, DWORD dwIndex, DWORD dwRegCount);


	void AddPixelShaderConstBool (const char * szName, DWORD dwIndex, DWORD dwRegCount);
	void AddVertexShaderConstBool (const char * szName, DWORD dwIndex, DWORD dwRegCount);


	void SetVariablesDatabase(VariablesDB * pDB);
	void UpdateVariablesFromDB();



	DWORD getPhysicalSize();
	BYTE* getPhysicalBytes();

	DWORD getHeaderSize();
	BYTE* getHeaderBytes();

	DWORD getInitializationSize();
	BYTE* getInitializationBytes();

	DWORD getSizeInDwords();


	void CleanupTempData();

	D3DCommandBuffer * getCommandBuffer();
	D3DCommandBuffer * getSourceCommandBuffer();


	__forceinline void fixupHyperZState (bool bHyperZDisabled)
	{
		if (bHyperZDisabled == false)
		{
			return;
		}

		GPU_HICONTROL * hiZControl = (GPU_HICONTROL *)(dwCommandBufferData + dwHiZControl_Offset);
		hiZControl->HiZWriteEnable = false;
		hiZControl->HiZEnable = false;
		hiZControl->HiZFunc = GPUHIZFUNC_LESS_EQUAL;
		hiZControl->HiBaseAddr = 0x0;

		hiZControl->HiStencilEnable = false;
		hiZControl->HiStencilWriteEnable = false;
		hiZControl->HiStencilFunc = GPUHISTENCILFUNC_EQUAL;
		hiZControl->HiStencilRef = 0x0;
	}

	__forceinline void fixupDepthState (bool bDepthDisabled)
	{
		if (bDepthDisabled == false)
		{
			return;
		}

		bNeedFlushHiperZ = true;
		GPU_DEPTHCONTROL * depthControl = (GPU_DEPTHCONTROL *)(dwCommandBufferData + dwDepthControl_Offset);
		depthControl->ZEnable = false;
		depthControl->ZWriteEnable = false;
	}

	__forceinline void restoreHyperZState ()
	{
		SetHyperZEnable(bHiZEnable, bHiZWriteEnable);
	}

	__forceinline void restoreDepthState ()
	{
		SetZEnable(bDepthEnable, bDepthWriteEnable);
	}



// для отрисовки динамической, через dpup геометрии...
//----------------------------------------------
	void DynamicDraw_SetVertexShader(IDirect3DVertexShader9 *pBoundedVS);
	void DynamicDraw_SetPixelShader(IDirect3DPixelShader9 *pPS);
	void DynamicDraw_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
	void DynamicDraw_SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	
	void DynamicDraw_SetupGPU(IDirect3DDevice9* device);

	void InsertPrefetchFiller();

	const char * GetShaderName();

	__forceinline bool isNeedFlushHiZ()
	{
		return bNeedFlushHiperZ;
	}


private:

	bool bLogOnlyWhenParse;


	IDirect3DBaseTexture9* pWhiteTexture;

	DWORD dwIndxOffset_Offset;
	DWORD dwHiZControl_Offset;
	DWORD dwColorMask_Offset;
	DWORD dwDepthControl_Offset;
	DWORD dwBlendControl0_Offset;
	DWORD dwBlendControl1_Offset;
	DWORD dwBlendControl2_Offset;
	DWORD dwBlendControl3_Offset;
	DWORD dwParser_Offset;
	DWORD dwSetCullMode_Offset;
	DWORD dwDrawToken_Offset;
	DWORD dwSetVertexShader_Offset_;
	DWORD dwSetPixelShader_Offset_;
	DWORD dwEndCommandBuffer_Offset;
	DWORD dwStreamSource_Offsets[MAX_STREAMS_COUNT];
	DWORD dwTexture_Offsets[MAX_TEXTURES_COUNT];
	DWORD dwALUConst_Offsets[MAX_ALU_CONST_COUNT];
	DWORD dwALUConstBool_Offsets[MAX_ALU_CONST_COUNT_BOOL];
	

	WORD dwSizeInDwords;
	WORD dwCurrentSizeInDwords;


	bool bNeedFlushHiperZ;

	bool bHiZEnable;
	bool bHiZWriteEnable;

	bool bDepthEnable;
	bool bDepthWriteEnable;



	DWORD dwInitializationSize_temp;
	BYTE* pInitializationBytes_temp;
	
	DWORD dwHeaderSize_temp;
	BYTE* pHeaderBytes_temp;
	D3DCommandBuffer * commandBuffer_temp;
	D3DCommandBuffer * sourcePrecompiledBuffer;
	

	DWORD dwCommandBufferDataSize;
	BYTE* dwCommandBufferData;


	const BYTE* baseData;

	GPUCOMMANDBUFFER_HEADER_PACKET3 dipPacketHeader;
	GPUCOMMAND_DRAW_INDEX dipPacket;
	
	
	GPUCOMMANDBUFFER_HEADER_PACKET3 dpPacketHeader;
	GPUCOMMAND_DRAW_AUTO dpPacket;


	DWORD dwVSO_Temp;
	DWORD dwPSO_Temp;

	//--------------------------
	struct constVector4
	{
		DWORD dwStartIdx;
		void * fixupPtrInCommandBuffer;
		DWORD dwRegCount;
		Vector4 * sourceData;
	};

	struct constBool
	{
		DWORD dwStartIdx;
		void * fixupPtrInCommandBuffer;
		DWORD dwRegCount;
		boolVector32 * sourceData;
	};
	struct textureSampler
	{
		DWORD dwStage;
		void * fixupPtrInCommandBuffer;
		IDirect3DBaseTexture9 ** tex;

		GPUMINMAGFILTER minFilter;
		GPUMINMAGFILTER magFilter;
		GPUMIPFILTER mipFilter;
		GPUCLAMP clampU;
		GPUCLAMP clampV;
	};


	array<constVector4> ps_consts_v4;
	array<constVector4> vs_consts_v4;
	array<constBool> ps_consts_b;
	array<constBool> vs_consts_b;
	array<textureSampler> tex_consts;

	IDirect3DVertexShader9 * m_pVertexShader;
	IDirect3DPixelShader9 * m_pPixelShader;


	
	//dynamic section
	//---------------
	IDirect3DVertexShader9 *pDynamicDrawVS;
	IDirect3DPixelShader9 *pDynamicDrawPS;

	struct dynamicDrawRState
	{
		D3DRENDERSTATETYPE State;
		DWORD Value;
	};

	struct dynamicDrawSState
	{
		DWORD Sampler;
		D3DSAMPLERSTATETYPE Type;
		DWORD Value;
	};

	string shaderName;

	array<dynamicDrawRState> dynamicDrawRS;
	array<dynamicDrawSState> dynamicDrawSS;


	void SetHyperZWriteEnable (bool bHyperZWriteEnable);
	void SetZWriteEnable(bool bDepthWriteEnabled);

	GPUCULL originalGpuCull;
	DWORD originalZEnable;
	DWORD originalHyperZEnable;
	DWORD originalColorMask;
};


#endif

#endif