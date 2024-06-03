#ifndef NX_PHYSICS_NXHINGEJOINTDESC
#define NX_PHYSICS_NXHINGEJOINTDESC
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
#include "NxJointLimitPairDesc.h"
#include "NxSpringDesc.h"
#include "NxMotorDesc.h"

/**
\brief Desc class for #NxRevoluteJoint.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxRevoluteJoint
*/
class NxRevoluteJointDesc : public NxJointDesc
	{
	public:
	/**
	\brief Optional limits for the angular motion of the joint. 

	<b>Range:</b> See #NxJointLimitPairDesc<br>
	<b>Default:</b> See #NxJointLimitPairDesc

	@see NxJointLimitPairDesc NxJointLimitDesc NxRevoluteJointFlag
	*/
	NxJointLimitPairDesc limit;

	/**
	\brief Optional motor.

	<b>Range:</b> See #NxMotorDesc<br>
	<b>Default:</b> See #NxMotorDesc

	@see NxMotorDesc NxRevoluteJointFlag
	*/
	NxMotorDesc			 motor; 

	/**
	\brief Optional spring.

	<b>Range:</b> See #NxSpringDesc<br>
	<b>Default:</b> See #NxSpringDesc

	@see NxSpringDesc NxRevoluteJointFlag
	*/
	NxSpringDesc		 spring;

	/**
	\brief The distance beyond which the joint is projected.
	
	projectionMode is NX_JPM_POINT_MINDIST, the joint gets artificially projected together when it drifts more than this distance. Sometimes it is not possible to project (for example when the joints form a cycle)
	Should be nonnegative. However, it may be a bad idea to always project to a very small or zero distance because the solver *needs* some error in order to produce correct motion.

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 1.0

	@see projectionMode projectionAngle
    */
	NxReal projectionDistance;	

	/**
	\brief The angle beyond which the joint is projected.
	
	This similar to #projectionDistance, except this is an angle (in radians) to which angular drift is 
	projected.

	<b>Unit:</b> Radians
	<b>Range:</b> (0.2,PI)<br>
	<b>Default:</b> 0.0872 (about 5 degrees in radians)

	@see projectionDistance projectionMode
	*/
	NxReal projectionAngle;

	/**
	\brief This is a combination of the bits defined by ::NxRevoluteJointFlag.

	<b>Default:</b> 0

	@see NxRevoluteJointFlag
	*/
	NxU32 flags;

	/**
	\brief use this to enable joint projection

	<b>Default:</b> NX_JPM_NONE

	@see NxJointProjectionMode projectionDistance projectionAngle NxRevoluteJoint.setProjectionMode()
	*/
	NxJointProjectionMode projectionMode;

	/**
	\brief constructor sets to default.
	*/

	NX_INLINE NxRevoluteJointDesc();	
	/**
	\brief (re)sets the structure to the default.	

	\param[in] fromCtor Avoid redundant work if called from constructor.
	*/
	NX_INLINE void setToDefault(bool fromCtor = false);
	
	/**
	\brief Returns true if the descriptor is valid.
	
	\return true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxRevoluteJointDesc::NxRevoluteJointDesc() : NxJointDesc(NX_JOINT_REVOLUTE)	//constructor sets to default
	{
	setToDefault(true);
	}

NX_INLINE void NxRevoluteJointDesc::setToDefault(bool fromCtor)
	{
	NxJointDesc::setToDefault();
	projectionDistance = 1.0f;
	projectionAngle = 0.0872f;	//about 5 degrees in radians.

	if (!fromCtor)
		{
		limit.setToDefault();
		motor.setToDefault();
		spring.setToDefault();
		}

	flags = 0;
	projectionMode = NX_JPM_NONE;
	}

NX_INLINE bool NxRevoluteJointDesc::isValid() const
	{
	if (projectionDistance < 0.0f) return false;
	if (projectionAngle < 0.02f) return false;	//if its smaller then current algo gets too close to a singularity.
	

	if (!limit.isValid()) return false;
	if (!motor.isValid()) return false;
	if (!spring.isValid()) return false;


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
