#include "..\..\..\common_h\render.h"

#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#include "CommandBufferExecutor.h"
#include "PrecompiledBatch.h"
#include "..\Render.h"


batchExecutor::batchExecutor()
{
	dwCurrentBufferIndex = 0;
	maxSizedBatch = NULL;
	dwMaxSize = 0;
	srcBuffer = NULL;
}

batchExecutor::~batchExecutor()
{
	for (DWORD i = 0; i < MAX_CB_BUFFERS_POOL; i++)
	{
		cbExecutor & cb = cbBuffers[i];

		if (cb.pHeaderBytes)
		{
			free(cb.pHeaderBytes);
			cb.pHeaderBytes = NULL;
		}

		if (cb.pInitializationBytes)
		{
			free(cb.pInitializationBytes);
			cb.pInitializationBytes = NULL;
		}

		if (cb.pPhysicalBytes)
		{
			XPhysicalFree(cb.pPhysicalBytes);
			cb.pPhysicalBytes = NULL;
		}

	}

}

void batchExecutor::calculateMaxSize (precompiledBatch * batch)
{
	DWORD dwBatchMaxSize = batch->getPhysicalSize();

	if (dwBatchMaxSize > dwMaxSize)
	{
		dwMaxSize = dwBatchMaxSize;
		maxSizedBatch = batch;

		if (srcBuffer)
		{
			srcBuffer->Release();
			srcBuffer = NULL;
		}

		srcBuffer = maxSizedBatch->getSourceCommandBuffer();
		srcBuffer->AddRef();
	}
}


void batchExecutor::create()
{
	//Создаем командных буферов для исполнения...
	for (DWORD i = 0; i < MAX_CB_BUFFERS_POOL; i++)
	{
		createCommandBuffer(cbBuffers[i]);
	}

	//Отпускаем матер буфер он под 3 мега...
	srcBuffer->Release();
	srcBuffer = NULL;
}

void batchExecutor::createCommandBuffer (cbExecutor & cbExec)
{
	Assert(srcBuffer);


	cbExec.dwHeaderSize = 0;
	cbExec.dwPhysicalSize = 0;
	cbExec.dwInitializationSize = 0;
	srcBuffer->Deconstruct( 0, NULL, &cbExec.dwHeaderSize, NULL, &cbExec.dwPhysicalSize, NULL, &cbExec.dwInitializationSize );


	cbExec.pHeaderBytes = ( BYTE* )malloc( cbExec.dwHeaderSize );
	cbExec.pPhysicalBytes = ( BYTE* )XPhysicalAlloc( cbExec.dwPhysicalSize, MAXULONG_PTR, 4096, PAGE_READWRITE | PAGE_WRITECOMBINE);
	cbExec.pInitializationBytes = ( BYTE* )malloc( cbExec.dwInitializationSize );

	Assert( cbExec.pHeaderBytes != NULL && cbExec.pPhysicalBytes != NULL && cbExec.pInitializationBytes != NULL );

	Assert(!((DWORD)cbExec.pPhysicalBytes & 3) && ((DWORD)cbExec.pPhysicalBytes >= GPU_CPU_VIRTUAL_ADDRESS_64KB_START));

	srcBuffer->Deconstruct( 0, ( D3DCommandBuffer* )cbExec.pHeaderBytes, &cbExec.dwHeaderSize, cbExec.pPhysicalBytes , &cbExec.dwPhysicalSize, cbExec.pInitializationBytes, &cbExec.dwInitializationSize);

	cbExec.cbToExecute = (D3DCommandBuffer*)cbExec.pHeaderBytes;


	cbExec.cbToExecute->BeginReconstruction( 0, cbExec.pPhysicalBytes, cbExec.pInitializationBytes);
	cbExec.cbToExecute->EndReconstruction();

}

/*
void fastMemoryCopyDword(DWORD * volatile dest, DWORD * volatile src)
{
	*dest = *src;
}
*/

void batchExecutor::Run (precompiledBatch * batch, IDirect3DDevice9* device, bool bDepthDisabled, bool bHyperZDisabled)
{
	batch->fixupHyperZState(bHyperZDisabled);
	batch->fixupDepthState(bDepthDisabled);
	
	

	cbExecutor & cbDesc = cbBuffers[dwCurrentBufferIndex % MAX_CB_BUFFERS_POOL];
	dwCurrentBufferIndex++;

	D3DCommandBuffer * cb = cbDesc.cbToExecute;

	BYTE * srcCommandBufferData = batch->getPhysicalBytes();
	DWORD dwSizeInDwords = batch->getSizeInDwords();
	DWORD dwOffsetToEndToken = dwSizeInDwords * sizeof(DWORD);
	DWORD dwSizeInBytes = dwOffsetToEndToken + (sizeof(DWORD) * 2);

	D3DCommandBuffer * cmdBuffer = batch->getCommandBuffer();
	
	
	
	
	cb->m_Inherited = cmdBuffer->m_Inherited;
	cb->m_Persisted = cmdBuffer->m_Persisted;

	cb->BeginDynamicFixups();

	//Правим новый размер
	GPUSetCommandBufferSizeInDwords(cb, dwSizeInDwords);

	NGRender::RenderStat & stats = NGRender::pRS->renderStats();
	stats.dwWCBytesTraffic += dwOffsetToEndToken;

	//Копируем содержимое (не надо endToken мы его сгенерируем и так...)
	XMemCpyStreaming_WriteCombined(cbDesc.pPhysicalBytes, srcCommandBufferData, dwOffsetToEndToken);


	batch->restoreDepthState();
	batch->restoreHyperZState();

/*
	DWORD * __restrict dest = (DWORD *)cbDesc.pPhysicalBytes;
	DWORD * __restrict src = (DWORD *)srcCommandBufferData;

	for (DWORD s = 0; s < dwSizeInDwords; s++)
	{
		fastMemoryCopyDword(dest, src);
		src++;
		dest++;
	}
*/

	//Маркируем окончание command buffer
	GPUCOMMAND_BUFFER_END * cmdEnd = (GPUCOMMAND_BUFFER_END*)(cbDesc.pPhysicalBytes + dwOffsetToEndToken);
	cmdEnd->Place();

	cb->EndDynamicFixups();



/*

	//------------------------------------------------------------------------------------------
	BYTE * memory = GPUGetCommandBufferMemoryAddr(cb);
	batch->debugOutCommandBuffer(memory, true);



	
	GPUCOMMAND_BUFFER_END * cmdEndStatic = (GPUCOMMAND_BUFFER_END*)(srcCommandBufferData + dwOffsetToEndToken);
	cmdEndStatic->Place();
	//batch->tempFixupVertexShaderAndPixelShader();
	batch->debugOutCommandBuffer(srcCommandBufferData, true);
	
	

	int a = 0;
	*/
	device->RunCommandBuffer(cb, 0);

/*
	if (batch->isNeedFlushHiZ())
	{
		device->FlushHiZStencil(D3DFHZS_ASYNCHRONOUS);
	}
*/

	

}

#endif

