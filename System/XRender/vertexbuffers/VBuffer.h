#ifndef DX8VERTEXBUFFER_HPP
#define DX8VERTEXBUFFER_HPP

#include "..\..\..\common_h\Render.h"
#include "..\D3DConvertor.h"
#include "..\Resource.h"

class CDX8VBuffer : public IVBuffer
{
public:
	static	IDirect3DDevice9	* pD3D8;

	void OnResetDevice();
	void OnLostDevice();


	CDX8VBuffer();
	virtual ~CDX8VBuffer();

	bool	Create(dword Length, dword Usage, RENDERPOOL Pool, dword Stride);

	bool	GetDesc(RENDERVERTEXBUFFER_DESC * pDesc);
	void *	Lock(dword OffsetToLock, dword SizeToLock, dword Flags);
	bool	Unlock();
	bool	IsLocked();

	bool	IsReseted();

	bool 	Copy(const void * pSrc, dword dwSize, dword dwOffset = 0, dword Flags = 0);

	// release inner resource object
	virtual bool Reset();
	// recreate inner resource object
	virtual bool Recreate();

	dword	SetStride(dword _dwStride)
	{ 
		dword dwOldStride = GetStride();
		dwStride = _dwStride;
		return dwOldStride;
	};

	dword				GetLength() const
	{ 
		return Resource.GetSize();
	};

	dword				GetStride() const
	{
		return dwStride;
	};

	RENDERPOOL				GetPool()	const
	{ 
		return Pool;
	};

	dword				GetUsage()	const
	{ 
		return dwUsage;
	};

	IDirect3DVertexBuffer9 * GetDX8VertexBuffer()
	{
		return pBuffer;
	};

	byte* GetMemoryVertexBuffer()
	{
		return pBufferSystemMemory;
	};

private:

	byte* pBufferSystemMemory;
	dword pBufferSystemMemorySize;

	IDirect3DVertexBuffer9		* pBuffer;
	RENDERPOOL					Pool;
	dword						dwUsage, dwStride;
	long						iNumLocks;
	bool						isReseted;

	// Resource implementation
	DX8_RESOURCE_IMPLEMENT			
	virtual bool	Release();
	virtual bool	ForceRelease();
};

#endif
