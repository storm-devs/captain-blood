#ifndef NX_COLLISION_CONTROLLERMANAGER
#define NX_COLLISION_CONTROLLERMANAGER

/*----------------------------------------------------------------------------*\
|
|		This is a deprecated interface, use NxControllerManager instead.
|
\*----------------------------------------------------------------------------*/

#include "NxControllerManager.h"

//A simple allocator using malloc() - The old ControllerManager used global allocation, so does the wrapper.
class ControllerManagerAllocator : public NxUserAllocator {
public:
	virtual void*	mallocDEBUG(size_t size, const char* fileName, int line)	{ return ::malloc(size); }
	virtual void*	malloc(size_t size)											{ return ::malloc(size); }
	virtual void*	realloc(void* memory, size_t size)							{ return ::realloc(memory, size); }
	virtual void	free(void* memory)											{ ::free(memory); }
};

/**
\brief Legacy class for supporting old interfaces, 
please use NxControllerManager through NxCreateControllerManager instead.

\note Deprecated

@see NxControllerManager NxCreateControllerManager
*/
class ControllerManager {
public:

	ControllerManager() 
		{
		mAllocator = new ControllerManagerAllocator();
		mManager = NxCreateControllerManager(mAllocator);
		}

	~ControllerManager() 
		{
		mArray.clear();
		NxReleaseControllerManager(mManager);
		delete mAllocator;
		}

	NxU32			getNbControllers() { return mManager->getNbControllers(); }
	NxController*	getController(NxU32 index) { return mManager->getController(index); }
	NxController*	createController(NxScene* scene, const NxControllerDesc& desc) { return mManager->createController(scene, desc); }
	void			releaseController(NxController& controller) { return mManager->releaseController(controller); }
	void			purgeControllers() { return mManager->purgeControllers(); }
	void			updateControllers() { return mManager->updateControllers(); }
	NxController**	getControllers() //The new interface does not contain a getControllers() method - workaround
	{
		mArray.clear();
		NxU32 count = mManager->getNbControllers();
		for (NxU32 i = 0; i < count; i++) {
			mArray.pushBack(mManager->getController(i));
		}
		return mArray.begin();
	}
	NxDebugRenderable	getDebugData()		{ return mManager->getDebugData();	}
	void				resetDebugData()	{ mManager->resetDebugData();		}
protected:
	NxControllerManager* mManager;
	ControllerManagerAllocator* mAllocator;
	NxArray<NxController*> mArray;
};


#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
