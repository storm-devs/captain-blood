#ifndef NX_PHYSICS_ALLOCATEABLE
#define NX_PHYSICS_ALLOCATEABLE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup physics
  @{
*/

#include "Nxp.h"
#include "NxUserAllocator.h"
#include "PhysXLoader.h"


/*
Should be called Allocateable but then we collide with Ice::Allocateable.
*/

/**
\brief Subclasses of this base class automatically take part in user memory management.
*/
class NxAllocateable
	{
	public:
	NX_INLINE void* operator new(size_t size, NxMemoryType type);
	NX_INLINE void* operator new[](size_t size, NxMemoryType type);
	NX_INLINE void  operator delete(void* p);
	NX_INLINE void  operator delete[](void* p);
#ifdef _DEBUG
	NX_INLINE void* operator new(size_t size, const char* fileName, int line, const char* className, NxMemoryType type);
	NX_INLINE void* operator new[](size_t size, const char* fileName, int line, const char* className, NxMemoryType type);
	NX_INLINE void  operator delete(void* p, const char*, int, const char*, NxMemoryType type);
	NX_INLINE void  operator delete[](void* p, const char*, int, const char*, NxMemoryType type);
#endif
	// PT: delete parameters have been added to fix warning C4291 in VC7. But they're not used.
	};

NX_INLINE void* ::NxAllocateable::operator new(size_t size, NxMemoryType type)
	{	
	return NxGetPhysicsSDKAllocator()->malloc(size, type);
	}

NX_INLINE void* ::NxAllocateable::operator new[](size_t size, NxMemoryType type)
	{	
	return NxGetPhysicsSDKAllocator()->malloc(size, type);
	}

NX_INLINE void ::NxAllocateable::operator delete(void* p)
	{
	NxGetPhysicsSDKAllocator()->free(p);
	}

NX_INLINE void ::NxAllocateable::operator delete[](void* p)
	{
	NxGetPhysicsSDKAllocator()->free(p);
	}

#ifdef _DEBUG
NX_INLINE void* ::NxAllocateable::operator new(size_t size, const char* fileName, int line, const char* className, NxMemoryType type)
	{	
	return NxGetPhysicsSDKAllocator()->mallocDEBUG(size, fileName, line, className, type);
	}

NX_INLINE void* ::NxAllocateable::operator new[](size_t size, const char* fileName, int line, const char* className, NxMemoryType type)
	{
	return NxGetPhysicsSDKAllocator()->mallocDEBUG(size, fileName, line, className, type);
	}

NX_INLINE void ::NxAllocateable::operator delete(void* p, const char*, int, const char*, NxMemoryType)
	{
	NxGetPhysicsSDKAllocator()->free(p);
	}

NX_INLINE void ::NxAllocateable::operator delete[](void* p, const char*, int, const char*, NxMemoryType)
	{
	NxGetPhysicsSDKAllocator()->free(p);
	}
#endif



#ifndef NX_PHYSICS_DLL			//only define for user code, our internal versions are defined in foundation/src/include/Allocateable.h
#ifndef NX_USE_SDK_STATICLIBS
#ifndef CORELIB					//TODO: this macro needs an NX prefix!
#ifdef NX_FOUNDATION_ALLOCATEABLE
#error  Something went wrong, duplicate macro def!
#endif
#ifdef _DEBUG
	#define NX_ALLOC_TMP(x)		NxGetPhysicsSDKAllocator()->mallocDEBUG(x, (const char *)__FILE__, __LINE__, #x, NX_MEMORY_TEMP)
	#define NX_ALLOC(x)			NxGetPhysicsSDKAllocator()->mallocDEBUG(x, (const char *)__FILE__, __LINE__, #x, NX_MEMORY_PERSISTENT)
#else
	#define NX_ALLOC_TMP(x)		NxGetPhysicsSDKAllocator()->malloc(x, NX_MEMORY_TEMP)
	#define NX_ALLOC(x)			NxGetPhysicsSDKAllocator()->malloc(x, NX_MEMORY_PERSISTENT)
#endif
	#define	NX_FREE(x)	if(x)	{ NxGetPhysicsSDKAllocator()->free(x); x = NULL;	}

#endif
#endif
#endif

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
