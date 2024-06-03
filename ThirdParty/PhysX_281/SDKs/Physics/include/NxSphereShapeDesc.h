#ifndef NX_COLLISION_NXSPHERESHAPEDESC
#define NX_COLLISION_NXSPHERESHAPEDESC
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

#include "NxShapeDesc.h"

/**
\brief Descriptor class for #NxSphereShape.
  
@see NxSphereShape NxShapeDesc

*/
class NxSphereShapeDesc : public NxShapeDesc
	{
	public:
	/**
	\brief radius of shape. Must be positive.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxReal		radius;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE					NxSphereShapeDesc();	
	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	
	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};

NX_INLINE NxSphereShapeDesc::NxSphereShapeDesc() : NxShapeDesc(NX_SHAPE_SPHERE)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxSphereShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	radius = 1.0f;
	}

NX_INLINE bool NxSphereShapeDesc::isValid() const
	{
	if(!NxMath::isFinite(radius))	return false;
	if(radius<=0.0f)				return false;
	return NxShapeDesc::isValid();
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
