#ifndef DX8INDEXBUFFER_HPP
#define DX8INDEXBUFFER_HPP

#include "..\..\..\common_h\Render.h"
#include "..\D3DConvertor.h"
#include "..\Resource.h"

class CDX8IBuffer : public IIBuffer
{
public:
	CDX8IBuffer();
	virtual ~CDX8IBuffer();

	void OnResetDevice();
	void OnLostDevice();



	bool	Create(dword Length, dword Usage, RENDERFORMAT Format, RENDERPOOL Pool);

	bool	GetDesc(RENDERINDEXBUFFER_DESC * pDesc);
	void *	Lock(dword OffsetToLock, dword dwSizeToLock, dword Flags);
	bool	Unlock();
	bool	IsLocked();

	bool	IsReseted();

	bool 	Copy(const void * pSrc, dword dwSize, dword dwOffset = 0, dword Flags = 0);

	// release inner resource object
	virtual bool Reset();
	// recreate inner resource object
	virtual bool Recreate();

	RENDERPOOL		GetPool() const { return Pool; };
	dword		GetUsage() const { return dwUsage; };
	RENDERFORMAT	GetFormat() const { return Format; };
	dword		GetLength() const { return GetSize(); };

	IDirect3DIndexBuffer9		* GetDX8IndexBuffer() { return pBuffer; };

	byte* GetMemoryIndexBuffer() { return pBufferSystemMemory; };

	static	IDirect3DDevice9	* pD3D8;
private:
	byte* pBufferSystemMemory;

	long						iNumLocks;
	IDirect3DIndexBuffer9		* pBuffer;

	dword						dwUsage;
	RENDERPOOL					Pool;
	RENDERFORMAT				Format;
	bool						isReseted;

	// Resource implementation
	DX8_RESOURCE_IMPLEMENT			
	virtual bool	Release();
	virtual bool	ForceRelease();
};

#endif