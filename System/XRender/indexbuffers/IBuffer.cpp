#include "IBuffer.h"
#include "..\Render.h"

CDX8IBuffer::CDX8IBuffer() : Resource(DX8TYPE_INDEXBUFFER, 0)
{
	iNumLocks = 0;
	pBuffer = null;
	pBufferSystemMemory = NULL;
	isReseted = true;
}

CDX8IBuffer::~CDX8IBuffer()
{
	DELETE (pBufferSystemMemory);

	NGRender::pRS->D3D()->SetIndices(NULL);
	
	RELEASE_D3D(pBuffer, 0);
}

bool CDX8IBuffer::Release() 
{
	if (Resource.Release()) return ForceRelease();
	return false;
}

bool CDX8IBuffer::ForceRelease() 
{ 
	Resource.ForceRelease(); 
	NGRender::pRS->ReleaseResource(this);
	delete this;
	return true;
}

bool CDX8IBuffer::Create(dword _Length, dword _Usage, RENDERFORMAT _Format, RENDERPOOL _Pool)
{
	//Assert(_Pool == POOL_MANAGED);

	RELEASE(pBuffer);
	DELETE (pBufferSystemMemory);

	if (_Length > 64 * 1024 * 1024)
	{
		api->Trace("DXERROR: try to create index buffer with size more than 64Mb !!!");
		return false;
	}


	if (_Length <= 0)
	{
		api->Trace("DXERROR: try to create index buffer with size 0");
		return false;
	}



#if defined(_XBOX) && defined(USED_X360_DYNAMIC_BUFFER_HACK)


	if ((_Usage & USAGE_DYNAMIC) != 0)
	{
		pBufferSystemMemory = new(Resource.GetFileName(),Resource.GetFileLine()) byte[_Length];
		pBuffer = NULL;
	} else
	{
		pBufferSystemMemory = NULL;
#endif


		//FIXME, убрать это дерьмо и попарвить весь код
#ifndef _XBOX
	if (_Pool == POOL_MANAGED) 
	{
		_Usage &= ~USAGE_DYNAMIC;
		_Usage &= ~USAGE_WRITEONLY;
	}
#else
	_Usage &= ~USAGE_DYNAMIC;
	_Usage &= ~USAGE_WRITEONLY;
#endif
	
	HRESULT hr = pD3D8->CreateIndexBuffer(_Length, UsageToDX (_Usage), FormatToDX (_Format), PoolToDX (_Pool), &pBuffer, NULL);
	Assert(hr == D3D_OK);

#if defined(_XBOX) && defined(USED_X360_DYNAMIC_BUFFER_HACK)
	}
#endif


	Resource.SetSize(_Length);
	dwUsage = _Usage;
	Format = _Format; 
	Pool = _Pool;

	if (!_Length)
	{
		api->Trace("DX8Error: Trying create vertex buffer with size = 0, file: %s, line: %d", GetFileName(), GetFileLine());
	}

	return true;
}

bool CDX8IBuffer::IsReseted()
{
	bool reseted = isReseted;
	isReseted = false;
	return reseted;
}

bool CDX8IBuffer::Reset()
{
	if (Pool == POOL_DEFAULT)
		isReseted = true;

	RELEASE(pBuffer);
	return true;
}

bool CDX8IBuffer::Recreate()
{
	return Create(GetLength(), GetUsage(), GetFormat(), GetPool());
}

bool CDX8IBuffer::GetDesc(RENDERINDEXBUFFER_DESC * pDesc)
{
	D3DINDEXBUFFER_DESC tempDesc;
	if (!pBuffer) return false;
	
	HRESULT hr = pBuffer->GetDesc(&tempDesc);
	Assert(hr == D3D_OK);

	*pDesc = IndexDescFromDX(tempDesc);
	return true;
}

void * CDX8IBuffer::Lock(dword OffsetToLock, dword SizeToLock, dword Flags)
{
	isReseted = false;
#ifdef _XBOX
	if (pBufferSystemMemory)
	{
		Assert(OffsetToLock == 0);
		Assert(SizeToLock == 0);
		iNumLocks++;
		return pBufferSystemMemory;
	}
#endif

	if (!pBuffer)
	{
		api->Trace("DX8Error: Trying lock index buffer with null pointer, file: %s, line: %d", GetFileName(), GetFileLine());
		return null;
	}


	void * pData = null;
	HRESULT hr = pBuffer->Lock( OffsetToLock, SizeToLock, (void**)&pData, LockFlagsToDX(Flags));
	Assert(hr == D3D_OK);
	iNumLocks++;
	return pData;
}

bool CDX8IBuffer::Unlock()
{
#ifdef _XBOX
	if (pBufferSystemMemory)
	{
		iNumLocks--;
		return pBufferSystemMemory;
	}
#endif

	HRESULT hr = pBuffer->Unlock();
	Assert(hr == D3D_OK);
	iNumLocks--;
	return true;
}

bool CDX8IBuffer::IsLocked()
{
	return iNumLocks > 0;
}

bool CDX8IBuffer::Copy(const void * pSrc, dword dwSize, dword dwOffset, dword Flags)
{
	if (!pSrc) return false;
	isReseted = false;

	// ВАНО поменял, на xbox'e нельзя лочить кусочки буфферов
#ifdef _XBOX
	Assert(dwOffset == 0);
	void * pDst = Lock(0, 0, LockFlagsToDX(Flags));
#else
	void * pDst = Lock(dwOffset, dwSize, LockFlagsToDX(Flags));
#endif
	if (!pDst) return false;
	memcpy(pDst, pSrc, dwSize);
	return Unlock();
}


void CDX8IBuffer::OnResetDevice()
{
	if (Pool != POOL_DEFAULT)  return;

	Recreate();

}

void CDX8IBuffer::OnLostDevice()
{
	if (Pool != POOL_DEFAULT)  return;

	Reset();
}
