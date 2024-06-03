#ifndef NX_COLLISION_NXBOXSHAPEDESC
#define NX_COLLISION_NXBOXSHAPEDESC
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
\brief Descriptor class for #NxBoxShape.

@see NxBox NxShapeDesc NxActor.createShape()
*/
class NxBoxShapeDesc : public NxShapeDesc
	{
	public:
	/** 
	\brief Dimensions of the box.
	
	The dimensions are the 'radii' of the box, meaning 1/2 extents in x dimension, 
	1/2 extents in y dimension, 1/2 extents in z dimension. All three must be positive.

	<b>Range:</b> .x (0,inf)
	<b>Range:</b> .y (0,inf)
	<b>Range:</b> .x (0,inf)
	<b>Default:</b> Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxShape.setDimensions() NxBoxShape.getDimensions()
	*/
	NxVec3	dimensions;			

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE					NxBoxShapeDesc();	
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

NX_INLINE NxBoxShapeDesc::NxBoxShapeDesc() : NxShapeDesc(NX_SHAPE_BOX)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxBoxShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	dimensions.zero();
	}

NX_INLINE bool NxBoxShapeDesc::isValid() const
	{
	if(!dimensions.isFinite())			return false;
	if(dimensions.x<0.0f)				return false;
	if(dimensions.y<0.0f)				return false;
	if(dimensions.z<0.0f)				return false;
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
