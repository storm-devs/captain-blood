#ifndef NX_PHYSICS_NXJOINTLIMITSOFTPAIRDESC
#define NX_PHYSICS_NXJOINTLIMITSOFTPAIRDESC
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

#include "NxJointLimitSoftDesc.h"

/**
\brief Describes a pair of joint limits

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxD6Joint NxD6JointDesc
*/
class NxJointLimitSoftPairDesc
	{
	public:

	/**
	\brief The low limit (smaller value)

	<b>Range:</b> See #NxJointLimitSoftDesc<br>
	<b>Default:</b> See #NxJointLimitSoftDesc

	@see NxJointLimitSoftDesc
	*/
	NxJointLimitSoftDesc low;
	
	/**
	\brief the high limit (larger value)

	<b>Range:</b> See #NxJointLimitSoftDesc<br>
	<b>Default:</b> See #NxJointLimitSoftDesc

	@see NxJointLimitSoftDesc
	*/
	NxJointLimitSoftDesc high;		

	/**
	\brief Constructor, sets members to default values.
	*/
	NX_INLINE NxJointLimitSoftPairDesc();

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

NX_INLINE NxJointLimitSoftPairDesc::NxJointLimitSoftPairDesc()
	{
	setToDefault();
	}

NX_INLINE void NxJointLimitSoftPairDesc::setToDefault()
	{
	//nothing
	}

NX_INLINE bool NxJointLimitSoftPairDesc::isValid() const
	{
	return (low.isValid() && high.isValid() && low.value <= high.value);
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
