#ifndef NX_PHYSICS_NXJOINTLIMITDESC
#define NX_PHYSICS_NXJOINTLIMITDESC
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
 
 <h3>Example</h3>

\include NxSpringDesc_NxJointLimitDesc_Example.cpp

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxRevoluteJoint NxSphericalJoint NxJointLimitPairDesc
*/
class NxJointLimitDesc
	{
	public:
	/**
	\brief The angle / position beyond which the limit is active.
	
	Which side the limit restricts depends on whether this is a high or low limit.

	<b>Unit:</b> Angular: Radians
	<b>Range:</b> Angular: (-PI,PI)<br>
	<b>Range:</b> Positional: (-inf,inf)<br>
	<b>Default:</b> 0.0
	*/
	NxReal value;

	/**
	\brief limit bounce

	<b>Range:</b> [0,1]<br>
	<b>Default:</b> 0.0
	*/
	NxReal restitution;

	/**
	\brief [not yet implemented!] limit can be made softer by setting this to less than 1.

	<b>Range:</b> [0,1]<br>
	<b>Default:</b> 1.0
	*/
	NxReal hardness;

	/**
	\brief Constructor, sets members to default values.
	*/
	NX_INLINE NxJointLimitDesc();

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

NX_INLINE NxJointLimitDesc::NxJointLimitDesc()
	{
	setToDefault();
	}

NX_INLINE void NxJointLimitDesc::setToDefault()
	{
	value = 0;
	restitution = 0;
	hardness = 1;
	}

NX_INLINE bool NxJointLimitDesc::isValid() const
	{
	return (restitution >= 0 && restitution <= 1 && hardness >= 0 && hardness <= 1);
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
