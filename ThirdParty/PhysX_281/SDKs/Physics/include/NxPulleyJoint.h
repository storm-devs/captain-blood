#ifndef NX_PHYSICS_NXPULLEYJOINT
#define NX_PHYSICS_NXPULLEYJOINT
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

class NxPulleyJointDesc;
class NxMotorDesc;

/**
 \brief A pulley joint simulates a rope between two objects passing over 2 pulleys.

 \image html pulleyJoint.png
 <h3>Creation</h3>

 Example:

 \include NxPulleyJoint_Create.cpp

 <h3>Visualizations:</h3>
\li #NX_VISUALIZE_JOINT_LOCAL_AXES
\li #NX_VISUALIZE_JOINT_WORLD_AXES
\li #NX_VISUALIZE_JOINT_LIMITS

 @see NxJoint NxPulleyJointDesc NxScene.createJoint()
*/
class NxPulleyJoint: public NxJoint
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

	@see saveToDesc NxPulleyJointDesc
	*/
	virtual void loadFromDesc(const NxPulleyJointDesc& desc) = 0;

	/**
	\brief Writes all of the object's attributes to the desc struct.  

	\param[out] desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see loadFromDesc() NxPulleyJointDesc
	*/
	virtual void saveToDesc(NxPulleyJointDesc& desc) = 0;

	/**
	\brief Sets motor parameters for the joint.
	
	For a positive velTarget, the motor pulls the first body towards its pulley, 
	for a negative velTarget, the motor pulls the second body towards its pulley.
	
	<ul>
	<li>velTarget - the relative velocity the motor is trying to achieve. The motor will only be able
				to reach this velocity if the maxForce is sufficiently large. If the joint is 
				moving faster than this velocity, the motor will actually try to brake. If you set this
				to infinity then the motor will keep speeding up, unless there is some sort of resistance
				on the attached bodies. </li>
	<li>maxForce -  the maximum force the motor can exert. Zero disables the motor.
				Default is 0, should be >= 0. Setting this to a very large value if velTarget is also 
				very large may not be a good idea.</li>
	<li>freeSpin -  if this flag is set, and if the joint is moving faster than velTarget, then neither
				braking nor additional acceleration will result.
				default: false.</li>
	</ul>
	This automatically enables the motor.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] motorDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMotorDesc getMotor()
	*/
	virtual void setMotor(const NxMotorDesc &motorDesc) = 0;

	/**
	\brief Reads back the motor parameters. Returns true if it is enabled.

	\param[out] motorDesc Used to retrieve the settings for this joint.
	\return True if the motor is enabled.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setMotor NxMotorDesc
	*/
	virtual bool getMotor(NxMotorDesc &motorDesc) = 0;

	/**
	\brief Sets the flags.	This is a combination of the ::NxPulleyJointFlag bits.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] flags New set of flags for this joint. See #NxPulleyJointFlag

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPulleyJointFlag getFlags()
	*/
	virtual void setFlags(NxU32 flags) = 0;

	/**
	\brief returns the current flag settings. see #NxPulleyJointFlag

	\return The flag settings for this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFlags() NxPulleyJointFlag
	*/
	virtual NxU32 getFlags() = 0;

	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
