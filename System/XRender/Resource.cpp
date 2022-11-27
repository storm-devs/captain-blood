#include "Resource.h"
#include "Render.h"

CDX8Resource::CDX8Resource(DX8RESOURCETYPE _Type, dword _dwSize)
{
	Type = _Type;
	dwSize = _dwSize;
	iRefCount = 1;
	iLoadRef = 0;
	bError = false;
}

CDX8Resource::~CDX8Resource()
{
	Type = DX8TYPE_ALREADYRELEASED;
}

bool CDX8Resource::Release()
{
	iRefCount--;

	if (iRefCount == 0)
	{
		return true;
	}

	return false;
};

bool CDX8Resource::ForceRelease()
{
	iRefCount = 0;
	return true;
};

void CDX8Resource::AddLoadRef()
{
	iLoadRef++;
}

void CDX8Resource::DecLoadRef()
{
	Assert(iLoadRef);
	iLoadRef--;
}
