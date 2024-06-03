#ifndef NX_PHYSICS_NXDISTANCEJOINTDESC
#define NX_PHYSICS_NXDISTANCEJOINTDESC
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
#include "NxSpringDesc.h"

/**
\brief Desc class for distance joint. See #NxDistanceJoint.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxDistanceJoint NxJointDesc NxScene.createJoint()

*/
class NxDistanceJointDesc : public NxJointDesc
	{
	public:

	/**
	\brief The maximum rest length of the rope or rod between the two anchor points.

	<b>Range:</b> [#minDistance,inf)<br>
	<b>Default:</b> 0.0
	*/
	NxReal maxDistance;

	/**
	\brief The minimum rest length of the rope or rod between the two anchor points

	<b>Range:</b> [0,#maxDistance]<br>
	<b>Default:</b> 0.0
	*/
	NxReal minDistance;
	
	/*
	\brief How stiff the constraint is, between 0 and 1 (stiffest)

	<b>Range:</b> [0,1]<br>
	<b>Default:</b> 1.0
	*/
	//NxReal stiffness;

	/**
	\brief makes the joint springy. The spring.targetValue field is not used.

	<b>Range:</b> See #NxSpringDesc<br>
	<b>Default:</b> See #NxSpringDesc
	*/
	NxSpringDesc spring;

	/**
	\brief This is a combination of the bits defined by ::NxDistanceJointFlag. 

	<b>Default:</b> 0
	*/
	NxU32  flags;

	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxDistanceJointDesc();	
	/**
	\brief (re)sets the structure to the default.	

	\param[in] fromCtor skip redundant operations if called from contructor.
	*/
	NX_INLINE void setToDefault(bool fromCtor=false);
	/**
	\brief Returns true if the descriptor is valid.

	\return true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxDistanceJointDesc::NxDistanceJointDesc() : NxJointDesc(NX_JOINT_DISTANCE)	//constructor sets to default
	{
	setToDefault(true);
	}

NX_INLINE void NxDistanceJointDesc::setToDefault(bool fromCtor)
	{
	NxJointDesc::setToDefault();
	maxDistance = 0.0f;
	minDistance = 0.0f;
	//stiffness = 1.0f;
	flags = 0;

	if (!fromCtor)
		{
		//this is redundant if we're being called from the ctor:
		spring.setToDefault();
		}
	}

NX_INLINE bool NxDistanceJointDesc::isValid() const
	{
	if (maxDistance < 0) return false;
	if (minDistance < 0) return false;

	// if both distance constrains are on, the min better be less than or equal to the max.
	if ((minDistance > maxDistance) && (flags == (NX_DJF_MIN_DISTANCE_ENABLED | NX_DJF_MAX_DISTANCE_ENABLED))) return false;
//	if (stiffness < 0 || stiffness > 1) return false;
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
