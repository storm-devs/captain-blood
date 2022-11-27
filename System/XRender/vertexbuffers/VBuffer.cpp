#include "VBuffer.h"
#include "..\Render.h"

CDX8VBuffer::CDX8VBuffer() : Resource(DX8TYPE_VERTEXBUFFER, 0)
{
	iNumLocks = 0;
	pBuffer = null;
	pBufferSystemMemory = null;
	pBufferSystemMemorySize = 0;
	isReseted = true;
}

CDX8VBuffer::~CDX8VBuffer()
{
/* JOKER FIX:
#ifdef _XBOX
	if (pBufferSystemMemory)
	{
		XPhysicalFree(pBufferSystemMemory);
	}
#endif
*/
	DELETE(pBufferSystemMemory);

	NGRender::pRS->D3D()->SetStreamSource(0, NULL, 0, 0);
	NGRender::pRS->D3D()->SetStreamSource(1, NULL, 0, 0);
	NGRender::pRS->D3D()->SetStreamSource(2, NULL, 0, 0);
	NGRender::pRS->D3D()->SetStreamSource(3, NULL, 0, 0);

	RELEASE_D3D(pBuffer, 0);
}

bool CDX8VBuffer::Release() 
{
	if (Resource.Release()) return ForceRelease();
	return false;
}

bool CDX8VBuffer::ForceRelease() 
{ 
	Resource.ForceRelease(); 
	NGRender::pRS->ReleaseResource(this);
	delete this;
	return true;
}

bool CDX8VBuffer::Create(dword _Length, dword _Usage, RENDERPOOL _Pool, dword _Stride)
{
	//Assert(_Pool == POOL_MANAGED);

	RELEASE(pBuffer);
/* JOKER FIX:
	#ifdef _XBOX
	if (pBufferSystemMemory)
	{
	XPhysicalFree(pBufferSystemMemory);
	}
	#endif
*/
	DELETE(pBufferSystemMemory);

	if (_Length > 64 * 1024 * 1024)
	{
		api->Trace("DXERROR: try to create vertex buffer with size more than 64Mb !!!");
		return false;
	}

	if (_Length <= 0)
	{
		api->Trace("DXERROR: try to create vertex buffer with size 0");
		return false;
	}



#if defined(_XBOX) && defined(USED_X360_DYNAMIC_BUFFER_HACK)

	if ((_Usage & USAGE_DYNAMIC) != 0)
	{
/* JOKER FIX:
		pBufferSystemMemory = (byte*)XPhysicalAlloc( _Length, MAXULONG_PTR, 0, PAGE_READWRITE | PAGE_WRITECOMBINE);
		pBufferSystemMemorySize = _Length;
*/

		pBufferSystemMemory = new(Resource.GetFileName(),Resource.GetFileLine()) byte[_Length];
		pBuffer = NULL;
	} else
	{
		pBufferSystemMemory = NULL;
#endif



#if defined(_XBOX) && defined(USED_X360_DYNAMIC_BUFFER_HACK)
		_Usage &= ~USAGE_DYNAMIC;
		_Usage &= ~USAGE_WRITEONLY;
#else
		if (_Pool == POOL_DEFAULT)
		{
			_Usage = _Usage | USAGE_WRITEONLY;
			_Usage = _Usage | USAGE_DYNAMIC;
		}
#endif


	HRESULT hr = pD3D8->CreateVertexBuffer(_Length, UsageToDX (_Usage), 0, PoolToDX (_Pool), &pBuffer, NULL);
	Assert(hr == D3D_OK);

#if defined(_XBOX) && defined(USED_X360_DYNAMIC_BUFFER_HACK)
	}
#endif





	Resource.SetSize(_Length);
	SetStride(_Stride);
	dwUsage = _Usage;
	Pool = _Pool;

	if (!_Length)
	{
		api->Trace("DX8Error: Trying create vertex buffer with size = 0, file: %s, line: %d", GetFileName(), GetFileLine());
	}

	return true;
}

bool CDX8VBuffer::IsReseted()
{
	bool reseted = isReseted;
	isReseted = false;
	return reseted;
}

bool CDX8VBuffer::Reset()
{
	if (Pool == POOL_DEFAULT)
		isReseted = true;

	RELEASE(pBuffer);

	return true;
}

bool CDX8VBuffer::Recreate()
{
	return Create(Resource.GetSize(), GetUsage(), GetPool(), GetStride());
}

bool CDX8VBuffer::GetDesc(RENDERVERTEXBUFFER_DESC * pDesc)
{
	D3DVERTEXBUFFER_DESC tempDesc;
	if (!pBuffer) return false;
	
	HRESULT hr = pBuffer->GetDesc(&tempDesc);
	Assert(hr == D3D_OK);

	*pDesc = VertexDescFromDX (tempDesc);

	return true;
}

void * CDX8VBuffer::Lock(dword OffsetToLock, dword SizeToLock, dword Flags)
{
	isReseted = false;
	void * pData = null;

#ifdef _XBOX
	if (pBufferSystemMemory)
	{
		Assert(OffsetToLock == 0);
		Assert(SizeToLock == 0);
		iNumLocks++;


		//JOKER: Хотел сделать защиту на чтение, но можно только на запись защитить :(
		//XPhysicalProtect(pBufferSystemMemory, pBufferSystemMemorySize, );

		return pBufferSystemMemory;
	}
#endif

	if (!pBuffer)
	{
		api->Trace("DX8Error: Trying lock vertex buffer with null pointer, file: %s, line: %d", GetFileName(), GetFileLine());
		return null;
	}

	HRESULT hr = pBuffer->Lock(OffsetToLock, SizeToLock, (void**)&pData, LockFlagsToDX(Flags));
	Assert(hr == D3D_OK);

	iNumLocks++;
	return pData;
}

bool CDX8VBuffer::Unlock()
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

bool CDX8VBuffer::IsLocked()
{
	return iNumLocks > 0;
}

bool CDX8VBuffer::Copy(const void * pSrc, dword dwSize, dword dwOffset, dword Flags)
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


void CDX8VBuffer::OnResetDevice()
{
	if (Pool != POOL_DEFAULT)  return;

	Recreate();
}

void CDX8VBuffer::OnLostDevice()
{
	if (Pool != POOL_DEFAULT)  return;

	Reset();
}
