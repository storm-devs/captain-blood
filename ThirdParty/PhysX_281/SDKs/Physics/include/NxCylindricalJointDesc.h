#ifndef NX_PHYSICS_NXSLIDINGJOINTDESC
#define NX_PHYSICS_NXSLIDINGJOINTDESC
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
\brief Desc class for sliding joint. See #NxCylindricalJoint.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxCylindricalJoint NxJointDesc NxScene.createJoint()
*/
class NxCylindricalJointDesc : public NxJointDesc
	{
	public:
	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxCylindricalJointDesc();	
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

NX_INLINE NxCylindricalJointDesc::NxCylindricalJointDesc() : NxJointDesc(NX_JOINT_CYLINDRICAL)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxCylindricalJointDesc::setToDefault()
	{
	NxJointDesc::setToDefault();
	}

NX_INLINE bool NxCylindricalJointDesc::isValid() const
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
