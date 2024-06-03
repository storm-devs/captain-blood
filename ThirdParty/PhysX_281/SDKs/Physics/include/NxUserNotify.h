#ifndef NX_PHYSICS_NXUSERNOTIFY
#define NX_PHYSICS_NXUSERNOTIFY
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

class NxActor;
class NxJoint;

/**
 \brief An interface class that the user can implement in order to receive simulation events.

  <b>Threading:</b> It is not necessary to make this class thread safe as it will only be called in the context of the
  user thread.

 <h3>Example</h3>

 \include NxUserNotify_Example.cpp

 @see NxScene.setUserNotify() NxScene.getUserNotify()
*/
class NxUserNotify
	{
	public:
	/**
	\brief This is called when a breakable joint breaks.
	
	The user should not release the joint inside this call! 
	Instead, if the user would like to have the joint
	released and no longer holds any referenced to it, he should return true.
	In this case the joint will be released by the system. Otherwise the user should return false, and 
	release the joint manually to free the resources associated with it (otherwise it will be released 
	with the scene).

	\note SDK state should not be modified from within onJointBreak(). In particular objects should not
	be created or destroyed. If state modification is needed then the changes should be stored to a buffer
	and performed after the simulation step.

	\note The behavior of this callback changed in version 2.5. The breakingForce parameter now supplies the 
	impulse applied, clamped to the maximum break impulse.

	\param[in] breakingImpulse The impulse which caused the joint to break.
	\param[in] brokenJoint The joint which has been broken.
	\return True to have the system release the joint now. False to keep the joint.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJoint.setBreakable() NxJointDesc.maxForce NxJointDesc.maxTorque
	*/
	virtual bool onJointBreak(NxReal breakingImpulse, NxJoint& brokenJoint) = 0;

	/**
	\brief This is called during NxScene::fetchResults with the actors which have just been woken up.

	\param[in] actors - The actors which just woke up.
	\param[in] count  - The number of actors

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.setUserNotify() NxSceneDesc.userNotify
	*/
	virtual void onWake(NxActor** actors, NxU32 count) = 0;

	/**
	\brief This is called during NxScene::fetchResults with the actors which have just been put to sleep.

	\param[in] actors - The actors which have just been put to sleep.
	\param[in] count  - The number of actors

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.setUserNotify() NxSceneDesc.userNotify
	*/
	virtual void onSleep(NxActor** actors, NxU32 count) = 0;

	protected:
	virtual ~NxUserNotify(){};
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
