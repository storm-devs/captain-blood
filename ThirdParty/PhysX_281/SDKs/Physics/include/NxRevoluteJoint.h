#ifndef NX_PHYSICS_NXHINGEJOINT
#define NX_PHYSICS_NXHINGEJOINT
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
#include "NxRevoluteJointDesc.h"

static const NxReal NX_NO_LOW_LIMIT  = -16;
static const NxReal NX_NO_HIGH_LIMIT =  16;


/**

\brief A joint which behaves in a similar way to a hinge or axel.

 A hinge joint removes all but a single rotational degree of freedom from two objects.
 The axis along which the two bodies may rotate is specified with a point and a direction
 vector.

 \image html revoluteJoint.png

 <h3>Creation</h3>

 Example:

 \include NxRevoluteJoint_Create.cpp

 A revolute joint can be given a motor, so that it can apply a force to rotate the attached actors.

 Example:

 \include NxRevoluteJoint_Motor.cpp

 <h3>Visualizations:</h3>
\li #NX_VISUALIZE_JOINT_LOCAL_AXES
\li #NX_VISUALIZE_JOINT_WORLD_AXES
\li #NX_VISUALIZE_JOINT_LIMITS

 @see NxRevoluteJointDesc NxScene.createJoint() NxJoint
*/
class NxRevoluteJoint : public NxJoint
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

	@see saveToDesc() NxRevoluteJointDesc
	*/
	virtual void loadFromDesc(const NxRevoluteJointDesc& desc) = 0;

	/**
	\brief Writes all of the object's attributes to the desc struct  .

	\param[out] desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see loadFromDesc() NxRevoluteJointDesc
	*/
	virtual void saveToDesc(NxRevoluteJointDesc& desc) = 0;

	/**
	\brief Sets angular joint limits.

	If either of these limits are set, any planar limits in NxJoint are ignored.
	The limits are angles defined the same way as the values getAngle() returns.
	
	The following has to hold:
	- Pi < lowAngle < highAngle < Pi
	Both limits are disabled by default.

	Also sets coefficients of restitutions for the low and high angular limits.
	These settings are only used if valid limits are set using setLimits().
	These restitution coefficients work the same way as for contacts.

	The coefficient of restitution determines whether a collision with the joint limit is
	completely elastic (like pool balls, restitution = 1, no energy is lost in the collision),
	completely inelastic (like putty, restitution = 0, no rebound after collision) or
	somewhere in between. The default is 0 for both.

	This automatically enables the limit.

	\param[in] pair The new joint limit settings. <b>Range:</b> See #NxJointLimitPairDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointLimitDesc NxJointLimitPairDesc getLimits()
	*/
	virtual void setLimits(const NxJointLimitPairDesc & pair) = 0;

	/**
	\brief Retrieves the joint limits.
	
	Returns true if it is enabled.

	Also returns the limit restitutions.

	\param[in] pair Used to retrieve the joint limit settings.
	\return True if the limit is enabled.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLimits() NxJointLimitPairDesc NxJointLimitDesc
	*/
	virtual bool getLimits(NxJointLimitPairDesc & pair) = 0;

	/**
	\brief Sets motor parameters for the joint.
	
	The motor rotates the bodies relative to each other along the hinge axis. The motor has these parameters:
	
	<ul>
	<li>velTarget - the relative velocity the motor is trying to achieve. The motor will only be able
				to reach this velocity if the maxForce is sufficiently large. If the joint is 
				spinning faster than this velocity, the motor will actually try to brake. If you set this
				to infinity then the motor will keep speeding up, unless there is some sort of resistance
				on the attached bodies. The sign of this variable determines the rotation direction,
				with positive values going the same way as positive joint angles.
				Default is infinity.</li>
	<li>maxForce -  the maximum force (torque in this case) the motor can exert. Zero disables the motor.
				Default is 0, should be >= 0. Setting this to a very large value if velTarget is also 
				very large may not be a good idea.</li>
	<li>freeSpin -  if this flag is set, and if the joint is spinning faster than velTarget, then neither
				braking nor additional acceleration will result.
				default: false.</li>
	</ul>

	This automatically enables the motor.

	\param[in] motorDesc The new motor parameters for the joint. <b>Range:</b> See #NxMotorDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMotorDesc getMotor()
	*/
	virtual void setMotor(const NxMotorDesc &motorDesc) = 0;
	
	/**
	\brief Reads back the motor parameters.
	
	Returns true if it is enabled.

	\param[out] motorDesc Used to store the motor parameters of the joint. See #NxMotorDesc
	\return True if the motor is enabled.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setMotor() NxMotorDesc
	*/
	virtual bool getMotor(NxMotorDesc &motorDesc) = 0;

	/**
	\brief Sets spring parameters.
	
	The spring is implicitly integrated so no instability should result for arbitrary
	spring and damping constants. Using these settings together with a motor is not possible -- the motor will have
	priority and the spring settings are ignored.
	If you would like to simulate your motor's internal friction, do this by altering the motor parameters directly.

	spring - The rotational spring acts along the hinge axis and tries to force
				the joint angle to zero. A setting of zero disables the spring. Default is 0, should be >= 0.
	damper - Damping coefficient; acts against the hinge's angular velocity. A setting of zero disables
				the damping. The default is 0, should be >= 0.
	targetValue - The angle at which the spring is relaxed. In [-Pi,Pi]. Default is 0.

	This automatically enables the spring.

	\param[in] springDesc The new spring parameters for the joint. <b>Range:</b> See #NxSpringDesc.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getSpring() NxSpringDesc
	*/
	virtual void setSpring(const NxSpringDesc &springDesc) = 0;

	/**
	\brief Retrieves spring settings.
	
	Returns true if it is enabled.

	@see setSpring

	\param[out] springDesc Used to retrieve the spring parameters for the joint. See #NxSpringDesc.
	\return True if the spring is enabled.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringDesc setSpring()
	*/
	virtual bool getSpring(NxSpringDesc &springDesc) = 0;

	/**
	\brief Retrieves the current revolute joint angle.
	
	The relative orientation of the bodies is stored when the joint is created, or when setAxis() 
	or setAnchor() is called.
	This initial orientation returns an angle of zero, and joint angles are measured
	relative to this pose.
	The angle is in the range [-Pi, Pi], with positive angles CCW around the axis, measured
	from body2 to body1.

	<b>Unit:</b> Radians
	<b>Range:</b> [-PI,PI]

	\return The current hinge angle.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getVelocity()
	*/
	virtual NxReal getAngle() = 0;

	/**
	\brief Retrieves the revolute joint angle's rate of change (angular velocity).

	It is the angular velocity of body1 minus body2 projected along the axis.

	\return The hinge velocity.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getAngle()
	*/
	virtual NxReal getVelocity() = 0;

	/**
	\brief Sets the flags to enable/disable the spring/motor/limit.
	
	This is a combination of the ::NxRevoluteJointFlag bits.

	\param[in] flags A combination of NxRevoluteJointFlag flags to set for this joint

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxRevoluteJointFlag getFlags()
	*/
	virtual void setFlags(NxU32 flags) = 0;

	/**
	\brief Retrieve the revolute joints flags.

	\return the current flag settings. See #NxRevoluteJointFlag

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFlags() NxRevoluteJointFlag
	*/
	virtual NxU32 getFlags() = 0;

	/**
	\brief Sets the joint projection mode.

	\param[in] projectionMode The new projection mode. See #NxJointProjectionMode.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getProjectionMode() NxJointProjectionMode NxRevoluteJointDesc.projectionMode
	*/
	virtual void setProjectionMode(NxJointProjectionMode projectionMode) = 0;

	/**
	\brief Retrieves the joints projection mode.

	\return the current flag settings.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setProjectionMode() NxJointProjectionMode NxRevoluteJointDesc.projectionMode
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
