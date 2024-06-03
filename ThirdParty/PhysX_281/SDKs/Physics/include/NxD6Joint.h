#ifndef NX_PHYSICS_NXD6Joint
#define NX_PHYSICS_NXD6Joint
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
#include "NxJoint.h"

class NxD6JointDesc;

/**
 \brief A D6 joint is a general constraint between two actors.
 
 It allows the user to individually define the linear and rotational degrees of freedom. 
 It also allows the user to configure the joint with limits and driven degrees of freedom as they wish.

 For example to create a fixed joint we would need to do:

 \code
	...
	d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

    d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
	...
 \endcode

 Or a Revolute joint:

 \code
	...
	d6Desc.twistMotion = NX_D6JOINT_MOTION_FREE;
    d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

    d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
	...

 \endcode

 And a spherical joint:

 \code
	...
	d6Desc.twistMotion = NX_D6JOINT_MOTION_FREE;
    d6Desc.swing1Motion = NX_D6JOINT_MOTION_FREE;
    d6Desc.swing2Motion = NX_D6JOINT_MOTION_FREE;

    d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
    d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
	...
 \endcode


<h3>Creation</h3>

Example:

\include NxD6Joint_Create.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_JOINT_LOCAL_AXES
\li #NX_VISUALIZE_JOINT_WORLD_AXES
\li #NX_VISUALIZE_JOINT_LIMITS

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Partial(some features are not supported. Hardware D6 joints have different properties to 
software joints so may behave slightly differently. See the user guide for details)
\li PS3  : Yes
\li XB360: Yes

@see NxD6JointDesc NxJoint NxScene.createJoint()
*/
class NxD6Joint: public NxJoint
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

	@see saveToDesc NxD6JointDesc
	*/
	virtual void loadFromDesc(const NxD6JointDesc& desc) = 0;
 
	/**
	\brief Writes all of the object's attributes to the desc struct  

	\param[out] desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see loadFromDesc NxD6JointDesc
	*/
	virtual void saveToDesc(NxD6JointDesc& desc) = 0;
 
	 
	/**
	\brief Set the drive position goal position when it is being driven.

	The goal position is specified relative to the joint frame corresponding to actor[0].

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param position The goal position if NX_D6JOINT_DRIVE_POSITION is set for xDrive,yDrive or zDrive. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setDriveOrientation() NxD6JointDesc.drivePosition
	@see NxD6JointDesc
	*/
	virtual void setDrivePosition(const NxVec3 &position) = 0;
 
	/**
	\brief Set the drive goal orientation when it is being driven.

	The goal orientation is specified relative to the joint frame corresponding to actor[0].

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param orientation The goal orientation if NX_D6JOINT_DRIVE_POSITION is set for swingDrive or
	twistDrive. <b>Range:</b> unit quaternion

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setDrivePosition NxD6JointDesc.driveOrientation
	*/
	virtual void setDriveOrientation(const NxQuat &orientation) = 0; 
 
	/**
	\brief Set the drive goal linear velocity when it is being driven.

	The drive linear velocity is specified relative to the actor[0] joint frame.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param linVel The goal velocity if NX_D6JOINT_DRIVE_VELOCITY is set for xDrive,yDrive or zDrive.
	See #NxD6JointDesc. <b>Range:</b> velocity vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setDriveAngularVelocity NxD6JointDesc.driveLinearVelocity
	*/
	virtual void setDriveLinearVelocity(const NxVec3 &linVel) = 0;
 
	/**
	\brief Set the drive angular velocity goal when it is being driven.

	The drive angular velocity is specified relative to the drive orientation target in the case of a slerp drive.

	The drive angular velocity is specified in the actor[0] joint frame in all other cases.
	
	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param angVel The goal angular velocity if NX_D6JOINT_DRIVE_VELOCITY is set for swingDrive or 
	twistDrive. <b>Range:</b> angular velocity vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setDriveLinearVelocity() NxD6JointDesc.driveAngularVelocity
	*/
	virtual void setDriveAngularVelocity(const NxVec3 &angVel) = 0;
	};
/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
