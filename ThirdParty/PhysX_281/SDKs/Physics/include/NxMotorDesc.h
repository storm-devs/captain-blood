#ifndef NX_PHYSICS_NXMOTORDESC
#define NX_PHYSICS_NXMOTORDESC
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

#include "NxBitField.h"	//just for NxJointDriveDesc's drive type, to be removed.
/**
\brief Describes a joint motor.

Some joints can be motorized, this allows them to apply a force to cause attached actors to move.

Joints which can be motorized:

<ul>
<li> #NxPulleyJoint </li>
<li> #NxRevoluteJoint </li>
</ul>

#NxJointDriveDesc is used for a similar purpose with #NxD6Joint.

 Example (for a revolute joint):

 \include NxRevoluteJoint_Motor.cpp

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxPulleyJoint NxRevoluteJoint
*/
class NxMotorDesc
	{
	public:
	/**
	\brief The relative velocity the motor is trying to achieve.
	
	The motor will only be able to reach this velocity if the maxForce is sufficiently large.
	If the joint is spinning faster than this velocity, the motor will actually try to brake(see #freeSpin). 

	If you set this to infinity then the motor will keep speeding up, unless there is some sort 
	of resistance on the attached bodies. The sign of this variable determines the rotation direction,
	with positive values going the same way as positive joint angles.

	Default is infinity.

	<b>Range:</b> [0,inf]<br>
	<b>Default:</b> NX_MAX_REAL

	@see freeSpin
	*/
	NxReal velTarget;	//target velocity of motor

	/**
	\brief The maximum force (or torque) the motor can exert.
	
	Zero disables the motor.
	Default is 0, should be >= 0. Setting this to a very large value if velTarget is also 
	very large may cause unexpected results.

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0.0
	*/
	NxReal maxForce;	//maximum motor force/torque

	/**
	\brief If true, motor will not brake when it spins faster than velTarget
	
	<b>Default:</b> false
	*/
	NX_BOOL freeSpin;

	/**
	\brief Constructor, sets members to default values.
	*/
	NX_INLINE NxMotorDesc();

	/**
	\brief Constructor, sets members to specified values.

	\param[in] velTarget target velocity of motor. <b>Range:</b> [0,inf]
	\param[in] maxForce maximum motor force/torque. <b>Range:</b> [0,inf)
	\param[in] freeSpin If true, motor will not brake when it spins faster than velTarget.
	*/
	NX_INLINE NxMotorDesc(NxReal velTarget, NxReal maxForce = 0, NX_BOOL freeSpin = 0);

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

NX_INLINE NxMotorDesc::NxMotorDesc()
	{
	setToDefault();
	}

NX_INLINE NxMotorDesc::NxMotorDesc(NxReal v, NxReal m, NX_BOOL f)
	{
	velTarget = v;
	maxForce = m;
	freeSpin = f;
	}

NX_INLINE void NxMotorDesc::setToDefault()
	{
	velTarget = NX_MAX_REAL;
	maxForce = 0;
	freeSpin = 0;
	}

NX_INLINE bool NxMotorDesc::isValid() const
	{
	return (maxForce >= 0);
	}

//TODO: the below class is very similar to the above and NxSpringDesc, so it should be merged...
/**
	\brief Class used to describe drive properties for a #NxD6Joint

	<h3>Default Values</h3>

	\li #driveType - 0
	\li #spring - 0
	\li #damping - 0
	\li #forceLimit - FLT_MAX

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Only D6 joint orientation)
	\li PS3  : Yes
	\li XB360: Yes
*/
class NxJointDriveDesc
	{
	public:
		/**
		Type of drive to apply. See #NxD6JointDriveType.

		<b>Default:</b> 0
		*/
		NxBitField32			driveType;

		/**
		\brief spring coefficient

		<b>Default:</b> 0
		<b>Range:</b> (-inf,inf)
		*/
		NxReal					spring;

		/**
		\brief damper coefficient

		<b>Default:</b> 0
		<b>Range:</b> [0,inf)
		*/
		NxReal					damping;

		/**
		\brief The maximum force (or torque) the drive can exert.

		<b>Default:</b> NX_MAX_REAL
		<b>Range:</b> [0,inf)
		*/
		NxReal					forceLimit;

		NxJointDriveDesc()
			{
			spring = 0;
			damping = 0;
			forceLimit = FLT_MAX;
			driveType = 0;
			}
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
