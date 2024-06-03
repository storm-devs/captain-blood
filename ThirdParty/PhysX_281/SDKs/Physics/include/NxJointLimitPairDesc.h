#ifndef NX_PHYSICS_NXJOINTLIMITPAIRDESC
#define NX_PHYSICS_NXJOINTLIMITPAIRDESC
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

#include "NxJointLimitDesc.h"

/**
\brief Describes a pair of joint limits

<h3>Example</h3>

\include NxSpringDesc_NxJointLimitDesc_Example.cpp

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxRevoluteJoint NxSphericalJoint NxJointLimitDesc
*/
class NxJointLimitPairDesc
	{
	public:

	/**
	\brief The low limit (smaller value)

	<b>Range:</b> See #NxJointLimitDesc<br>
	<b>Default:</b> See #NxJointLimitDesc

	@see NxJointLimitDesc
	*/
	NxJointLimitDesc low;
	
	/**
	\brief the high limit (larger value)

	<b>Range:</b> See #NxJointLimitDesc<br>
	<b>Default:</b> See #NxJointLimitDesc

	@see NxJointLimitDesc
	*/
	NxJointLimitDesc high;

	/**
	\brief Constructor, sets members to default values.
	*/
	NX_INLINE NxJointLimitPairDesc();

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

NX_INLINE NxJointLimitPairDesc::NxJointLimitPairDesc()
	{
	setToDefault();
	}

NX_INLINE void NxJointLimitPairDesc::setToDefault()
	{
	//nothing
	}

NX_INLINE bool NxJointLimitPairDesc::isValid() const
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
