#ifndef NX_COLLISION_NXCAPSULESHAPEDESC
#define NX_COLLISION_NXCAPSULESHAPEDESC
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

#include "NxCapsuleShape.h"
#include "NxShapeDesc.h"
/**
\brief Descriptor class for #NxCapsuleShape.

@see NxCapsuleShape NxShapeDesc NxActor.createShape() NxCapsule
*/
class NxCapsuleShapeDesc : public NxShapeDesc
	{
	public:
	/**
	\brief Radius of the capsule's hemispherical ends and its trunk.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCapsuleShape.setRadius() NxCapsuleShape.setDimensions()
	*/
	NxReal		radius;

	/**
	\brief The distance between the two hemispherical ends of the capsule.
	
	The height is along the capsule's Y axis. 

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCapsuleShape.setHeight() NxCapsuleShape.setDimensions()
	*/
	NxReal		height;

	/**
	\brief Combination of ::NxCapsuleShapeFlag

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCapsuleShapeFlag
	*/
	NxU32		flags;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE					NxCapsuleShapeDesc();	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	/**
	\brief Returns true if the descriptor is valid.

	\return True if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};

NX_INLINE NxCapsuleShapeDesc::NxCapsuleShapeDesc() : NxShapeDesc(NX_SHAPE_CAPSULE)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxCapsuleShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	radius = 1.0f;
	height = 1.0f;
	flags  = 0;
	}

NX_INLINE bool NxCapsuleShapeDesc::isValid() const
	{
	if(!NxMath::isFinite(radius))	return false;
	if(radius<=0.0f)				return false;
	if(!NxMath::isFinite(height))	return false;
	if(height<=0.0f)				return false;
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
