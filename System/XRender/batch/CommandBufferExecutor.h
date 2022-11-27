#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#ifndef __COMMAND_BUFFERS_BATCH_EXECUTOR_____
#define __COMMAND_BUFFERS_BATCH_EXECUTOR_____

#include <stdio.h>
#include <xtl.h>
#include <xgraphics.h>
#include <xboxmath.h>


#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\defines.h"
#include "..\..\..\common_h\math3D.h"
#include "..\..\..\common_h\templates.h"
#include "..\..\..\common_h\render.h"


#define MAX_CB_BUFFERS_POOL 1536

class precompiledBatch;

class batchExecutor
{
	struct cbExecutor
	{
		DWORD dwHeaderSize;
		DWORD dwPhysicalSize;
		DWORD dwInitializationSize;

		BYTE* pHeaderBytes;
		BYTE* pInitializationBytes;
		BYTE* pPhysicalBytes;

		D3DCommandBuffer * cbToExecute;
	};

	void createCommandBuffer (cbExecutor & cbExec);


	D3DCommandBuffer * srcBuffer;
	precompiledBatch * maxSizedBatch;
	DWORD dwMaxSize;


	cbExecutor cbBuffers[MAX_CB_BUFFERS_POOL];


	DWORD dwCurrentBufferIndex;

public:


	batchExecutor();
	~batchExecutor();

	void calculateMaxSize (precompiledBatch * batch);

	void create();

	void Run (precompiledBatch * batch, IDirect3DDevice9* device, bool bDepthDisabled, bool bHyperZDisabled);

};



#endif

#endif