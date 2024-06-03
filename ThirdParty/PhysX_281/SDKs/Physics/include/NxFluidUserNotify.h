#ifndef NX_PHYSICS_NXFLUIDUSERNOTIFY
#define NX_PHYSICS_NXFLUIDUSERNOTIFY
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

class NxFluidEmitter;
class NxFluid;

/**
\brief Event types for fluid events.
@see NxFluidUserNotify::onEvent(NxFluid& fluid, NxFluidEventType eventType)
*/
enum NxFluidEventType
{
	NX_FET_NO_PARTICLES_LEFT,		//!< There are no particles left.
};


/**
\brief Event types for emitter events.
@see NxUserNotify::onEmitterEvent(NxFluidEmitter& emitter, NxFluidEmitterEventType eventType)
*/
enum NxFluidEmitterEventType
{
	NX_FEET_EMITTER_EMPTY,		//!< The emitter has reached is emission limit NxEmitterDesc::maxParticles.
};


/**
\brief An interface class that the user can implement in order to receive simulation events.

<b>Threading:</b> It is not necessary to make this class thread safe as it will only be called in the context of the
user thread.

See the NxUserNotify documentation for an example of how to use notifications.

@see NxScene.setFluidUserNotify() NxScene.getFluidUserNotify() NxUserNotify
*/
class NxFluidUserNotify
{
public:
	/**
	\brief This is called during NxScene::fetchResults with fluid emitters that have events. 

	\param[in] emitter - The emitter which had the event.
	\param[in] eventType  - The event type.
	\return True to have the system release the emitter now. False to keep the emitter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No

	@see NxScene.setUserNotify() NxSceneDesc.userNotify
	@see NxFluidEmitterEventType
	*/
	virtual bool onEmitterEvent(NxFluidEmitter& emitter, NxFluidEmitterEventType eventType) = 0;

	/**
	\brief This is called during NxScene::fetchResults with fluids that have events. 

	\param[in] fluid - The fluid which had the event.
	\param[in] eventType  - The event type.
	\return True to have the system release the fluid now. False to keep the fluid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No

	@see NxScene.setUserNotify() NxSceneDesc.userNotify
	@see NxFluidEventType
	*/
	virtual bool onEvent(NxFluid& fluid, NxFluidEventType eventType) = 0;

protected:
	virtual ~NxFluidUserNotify(){};
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
