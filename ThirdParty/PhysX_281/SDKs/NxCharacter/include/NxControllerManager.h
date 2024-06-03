#ifndef NX_CHARACTER_NXCONTROLLERMANAGER
#define NX_CHARACTER_NXCONTROLLERMANAGER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxCharacter.h"

#include "Nxp.h"
#include "NxArray.h"
#include "NxDebugRenderable.h"

class NxScene;
class Controller;
class NxController;
class NxControllerDesc;
class NxControllerManager;
class ControllerArray;


NX_C_EXPORT NXCHARACTER_API NxControllerManager* NX_CALL_CONV NxCreateControllerManager(NxUserAllocator* allocator);
NX_C_EXPORT NXCHARACTER_API void NX_CALL_CONV NxReleaseControllerManager(NxControllerManager* manager);

/**
\brief Manages an array of character controllers.

@see NxController NxBoxController NxCapsuleController
*/
class NXCHARACTER_API NxControllerManager {
public:
	/**
	\brief Returns the number of controllers that are being managed.

	\return The number of controllers.
	*/
	virtual NxU32			getNbControllers() const = 0;

	/**
	\brief Retrieve one of the controllers in the manager.

	\param index the index of the controller to return
	\return an array of controller pointers with size getNbControllers().
	*/
	virtual NxController*	getController(NxU32 index) = 0;

	/**
	\brief Creates a new character controller.

	\param[in] scene The scene that the controller will belong to.
	\param[in] desc The controllers descriptor
	\return The new controller

	@see NxController NxControllerDesc
	*/
	virtual NxController*	createController(NxScene* scene, const NxControllerDesc& desc) = 0;

	/**
	\brief Releases a controller.

	\param[in] controller The controller to release.

	@see NxController
	*/
	virtual void			releaseController(NxController& controller) = 0;

	/**
	\brief Releases all the controllers that are being managed.
	*/
	virtual void			purgeControllers() = 0;

	/**
	\brief Updates the exposed position from the filtered position of all controllers.
	*/
	virtual void			updateControllers() = 0;

	/**
	\brief Retrieves debug data. Note that debug rendering is not enabled until this method is called.
	*/
	virtual	NxDebugRenderable	getDebugData()		= 0;

	/**
	\brief Resets debug data
	*/
	virtual	void				resetDebugData()	= 0;

protected:
	NxControllerManager() {}
	virtual ~NxControllerManager() {}

	friend NXCHARACTER_API void NxReleaseControllerManager(NxControllerManager* manager);
	virtual void release() = 0;
};


#endif //NX_CHARACTER_NXCONTROLLERMANAGER
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
