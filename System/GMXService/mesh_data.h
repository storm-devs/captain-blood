/********************************************************************
created:	2009 dec
author:		Sergey Makeev
purpose:	
*********************************************************************/
#ifndef _MESH_DATA_STRUCTURES___
#define _MESH_DATA_STRUCTURES___




#define MESH_INLINE __forceinline 

#ifndef STOP_DEBUG
#define MESH_ALIGN_16_CHECK(val) Assert((((dword)(val)) & (15)) == 0);
#else
#define MESH_ALIGN_16_CHECK(val) 
#endif









MESH_INLINE void * meshAlloc (dword dwBytes, const char * fileName, long fileLine)
{
	void * memory = api->Reallocate(NULL, dwBytes, fileName, fileLine);
	Assert(memory);
	return memory;
}

MESH_INLINE void meshFree(void * ptr, const char * fileName, long fileLine)
{
	if (ptr == NULL) return;
	api->Free(ptr, fileName, fileLine);
}

#endif