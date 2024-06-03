#ifndef NX_CHARACTER_CONTROLLERMANAGER
#define NX_CHARACTER_CONTROLLERMANAGER

//Exclude file from docs
/** \cond */

#include "NxControllerManager.h"
#include "CCTDebugRenderer.h"

//Implements the NxControllerManager interface, this class used to be called ControllerManager
class CharacterControllerManager: public NxControllerManager
{
public:
						CharacterControllerManager(NxUserAllocator* userAlloc);
	virtual				~CharacterControllerManager();

	NxU32				getNbControllers()	const;
	NxController*		getController(NxU32 index);
	Controller**		getControllers();
	NxController*		createController(NxScene* scene, const NxControllerDesc& desc);
	void				releaseController(NxController& controller);
	void				purgeControllers();
	void				updateControllers();
	void				release();
	NxDebugRenderable	getDebugData();
	void				resetDebugData();

	void				printStats();

	CCTDebugData*		debugData;
protected:
	ControllerArray*	controllers;
	NxUserAllocator*	allocator;
};

/** \endcond */

#endif //NX_CHARACTER_CONTROLLERMANAGER
