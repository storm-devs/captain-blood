#ifndef NX_PHYSICS_NXFIXEDJOINTDESC
#define NX_PHYSICS_NXFIXEDJOINTDESC
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
/**
\brief Desc class for fixed joint.


<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxFixedJoint NxScene.createJoint()
*/
class NxFixedJointDesc : public NxJointDesc
	{
	public:
	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxFixedJointDesc();	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	\brief Return true if the descriptor is valid.

	\return true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxFixedJointDesc::NxFixedJointDesc() : NxJointDesc(NX_JOINT_FIXED)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxFixedJointDesc::setToDefault()
	{
	NxJointDesc::setToDefault();
	}

NX_INLINE bool NxFixedJointDesc::isValid() const
	{
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
