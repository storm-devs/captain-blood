#ifndef NX_PHYSICS_NXSPHERICALJOINT
#define NX_PHYSICS_NXSPHERICALJOINT
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

#include "NxSphericalJointDesc.h"

#include "NxJoint.h"

/**
 \brief A sphere joint constrains two points on two bodies to coincide.

 This point, specified in world space (this guarantees that the points coincide 
 to start with) is the only parameter that has to be specified.

 \image html sphericalJoint.png

 <h3>Creation</h3>

 Example:

 \include NxSphericalJoint_Create.cpp

 <h3>Visualizations:</h3>
\li #NX_VISUALIZE_JOINT_LOCAL_AXES
\li #NX_VISUALIZE_JOINT_WORLD_AXES
\li #NX_VISUALIZE_JOINT_LIMITS

 @see NxSphericalJointDesc NxScene.createJoint() NxJoint
*/

class NxSphericalJoint: public NxJoint
	{
	public:

	/**
	\brief Use this for changing a significant number of joint parameters at once.

	Use the set() methods for changing only a single property at once.

	Please note that you can not change the actor pointers using this function, if you do so the joint will be marked as broken and will stop working.

	Calling the loadFromDesc() method on a broken joint will result in an error message.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] desc The descriptor used to set the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphericalJointDesc saveToDesc()
	*/
	virtual void loadFromDesc(const NxSphericalJointDesc &desc) = 0;

	/**
	\brief Writes all of the object's attributes to the desc struct  

	\param[out] desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphericalJointDesc loadFromDesc()
	*/
	virtual void saveToDesc(NxSphericalJointDesc &desc) = 0;

	/**
	\brief Sets the flags to enable/disable the spring/motor/limit.
	
	This is a combination of the bits defined by ::NxSphericalJointFlag.

	\param[in] flags The new value for the joint flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphericalJointFlag getFlags()
	*/
	virtual void setFlags(NxU32 flags) = 0;

	/**
	\brief Returns the current flag settings.

	\return The flags associated with the joint.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getFlags() NxSphericalJointFlag
	*/
	virtual NxU32 getFlags() = 0;

	/**
	\brief Sets the joint projection mode.

	\param[in] projectionMode The new projection mode for the joint. See #NxJointProjectionMode.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getProjectionMode() NxJointProjectionMode
	*/
	virtual void setProjectionMode(NxJointProjectionMode projectionMode) = 0;

	/**
	\brief Returns the current flag settings.

	\return The joints projection mode. See #NxJointProjectionMode.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setProjectionMode() NxJointProjectionMode
	*/
	virtual NxJointProjectionMode getProjectionMode() = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
