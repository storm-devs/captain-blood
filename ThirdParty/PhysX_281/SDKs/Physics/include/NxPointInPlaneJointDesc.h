#ifndef NX_PHYSICS_NXPOINTINPLANEJOINTDESC
#define NX_PHYSICS_NXPOINTINPLANEJOINTDESC
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
\brief Desc class for point-in-plane joint. See #NxPointInPlaneJoint.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxPointInPlaneJoint NxJointDesc NxScene.createJoint
*/
class NxPointInPlaneJointDesc : public NxJointDesc
	{
	public:
	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxPointInPlaneJointDesc();	
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

NX_INLINE NxPointInPlaneJointDesc::NxPointInPlaneJointDesc() : NxJointDesc(NX_JOINT_POINT_IN_PLANE)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxPointInPlaneJointDesc::setToDefault()
	{
	NxJointDesc::setToDefault();
	}

NX_INLINE bool NxPointInPlaneJointDesc::isValid() const
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
