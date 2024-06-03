#ifndef NX_PHYSICS_NXJOINTLIMITSOFTDESC
#define NX_PHYSICS_NXJOINTLIMITSOFTDESC
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

/**
\brief Describes a joint limit.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxD6Joint NxD6JointDesc NxJointSoftLimitPairDesc
*/
class NxJointLimitSoftDesc
	{
	public:
	/**
	\brief the angle / position beyond which the limit is active.
	
	Which side the limit restricts depends on whether this is a high or low limit.

	<b>Unit:</b> Angular: Radians
	<b>Range:</b> Angular: (-PI,PI)<br>
	<b>Range:</b> Positional: [0.0,inf)<br>
	<b>Default:</b> 0.0
	*/
	NxReal value;
	
	/**
	\brief Controls the amount of bounce when the joint hits a limit.

	A restitution value of 1.0 causes the joint to bounce back with the velocity which it hit the limit.
	A value of zero causes the joint to stop dead.

	In situations where the joint has many locked DOFs (e.g. 5) the restitution may not be applied 
	correctly. This is due to a limitation in the solver which causes the restitution velocity to become zero 
	as the solver enforces constraints on the other DOFs.

	This limitation applies to both angular and linear limits, however it is generally most apparent with limited
	angular DOFs.

	Disabling joint projection and increasing the solver iteration count may improve this behavior to some extent.

	Also, combining soft joint limits with joint motors driving against those limits may affect stability.

	<b>Range:</b> [0,1]<br>
	<b>Default:</b> 0.0
	*/
	NxReal restitution;
	
	/**
	\brief if greater than zero, the limit is soft, i.e. a spring pulls the joint back to the limit

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0.0
	*/
	NxReal spring;

	/**
	\brief if spring is greater than zero, this is the damping of the spring
	
	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0.0
	*/
	NxReal damping;

	/**
	\brief Constructor, sets members to default values.
	*/
	NX_INLINE NxJointLimitSoftDesc();

	/** 
	\brief Sets members to default values.
	*/
	NX_INLINE void setToDefault();

	/**
	\brief Returns true if the descriptor is valid.

	\return true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxJointLimitSoftDesc::NxJointLimitSoftDesc()
	{
	setToDefault();
	}

NX_INLINE void NxJointLimitSoftDesc::setToDefault()
	{
	value = 0;
	restitution = 0;
	spring = 0;
	damping = 0;
	}

NX_INLINE bool NxJointLimitSoftDesc::isValid() const
	{
	return (restitution >= 0 && restitution <= 1);
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
