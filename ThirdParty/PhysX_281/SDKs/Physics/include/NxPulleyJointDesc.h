#ifndef NX_PHYSICS_NXPULLEYJOINTDESC
#define NX_PHYSICS_NXPULLEYJOINTDESC
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
#include "NxJointDesc.h"
#include "NxMotorDesc.h"

/**
\brief Desc class for #NxPulleyJoint.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxPulleyJoint NxJointDesc
*/
class NxPulleyJointDesc : public NxJointDesc
	{
	public:
	/**
	\brief suspension points of two bodies in world space.

	<b>Range:</b> position vector<br>
	<b>Default:</b> [0]Zero<br>
	<b>Default:</b> [1]Zero
	*/
	NxVec3 pulley[2];

	/**
	\brief the rest length of the rope connecting the two objects.
	
	The distance is computed as ||(pulley0 - anchor0)|| +  ||(pulley1 - anchor1)|| * ratio.

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0.0
	*/
	NxReal distance;

	/**
	\brief how stiff the constraint is, between 0 and 1 (stiffest)

	<b>Range:</b> [0,1]<br>
	<b>Default:</b> 1.0
	*/
	NxReal stiffness;

	/**
	\brief transmission ratio

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0
	*/
	NxReal ratio;

	/**
	\brief This is a combination of the bits defined by ::NxPulleyJointFlag.

	<b>Default:</b> 0

	@see NxPulleyJointFlag.
	*/
	NxU32  flags; 

	/**
	\brief Optional joint motor.

	<b>Range:</b> See #NxMotorDesc<br>
	<b>Default:</b> See #NxMotorDesc

	@see NxMotorDesc
	*/
	NxMotorDesc	motor;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxPulleyJointDesc();	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxPulleyJointDesc::NxPulleyJointDesc() : NxJointDesc(NX_JOINT_PULLEY)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxPulleyJointDesc::setToDefault()
	{
	pulley[0].zero();
	pulley[1].zero();

	distance = 0.0f;
	stiffness = 1.0f;
	ratio = 1.0f;
	flags = 0;

	motor.setToDefault();

	NxJointDesc::setToDefault();
	}

NX_INLINE bool NxPulleyJointDesc::isValid() const
	{
	if (distance < 0) return false;
	if (stiffness < 0 || stiffness > 1) return false;
	if (ratio < 0) return false;
	if (!motor.isValid()) return false;

	return NxJointDesc::isValid();
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
