#ifndef NX_PHYSICS_NXBOXFORCEFIELDSHAPEDESC
#define NX_PHYSICS_NXBOXFORCEFIELDSHAPEDESC
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


/**
 \brief A descriptor for NxBoxForceFieldShape
 

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

 @see NxBoxForceFieldShape NxForceFieldShapeDesc NxBoxForceField
*/
class NxBoxForceFieldShapeDesc : public NxForceFieldShapeDesc
	{
	public:
	/** 
	\brief Dimensions of the box.
	
	The dimensions are the 'radii' of the box, meaning 1/2 extents in x dimension, 
	1/2 extents in y dimension, 1/2 extents in z dimension. All three must be positive.

	<b>Range:</b> .x (0,inf)
	<b>Range:</b> .y (0,inf)
	<b>Range:</b> .z (0,inf)
	<b>Default:</b> Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxForceFieldShape.setDimensions() NxBoxForceFieldShape.getDimensions()
	*/
	NxVec3 dimensions;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxBoxForceFieldShapeDesc();

	/**
	\brief (re)sets the structure to the default.	
	*/
	virtual NX_INLINE void setToDefault();

	/**
	\brief Returns true if the descriptor is valid.

	\return True if the current settings are valid
	*/
	virtual NX_INLINE bool isValid() const;


	};

NX_INLINE NxBoxForceFieldShapeDesc::NxBoxForceFieldShapeDesc() : NxForceFieldShapeDesc(NX_SHAPE_BOX)
	{
	setToDefault();
	}

NX_INLINE void NxBoxForceFieldShapeDesc::setToDefault()
	{
	NxForceFieldShapeDesc::setToDefault();
	dimensions.set(1.0f, 1.0f, 1.0f);	//note: NxBoxShape defaults to x0x0x but this is inconsistent with other shapes that are unity-sized by default.
	}

NX_INLINE bool NxBoxForceFieldShapeDesc::isValid() const
	{
	if(!dimensions.isFinite())			return false;
	if(dimensions.x<0.0f)				return false;
	if(dimensions.y<0.0f)				return false;
	if(dimensions.z<0.0f)				return false;
	return NxForceFieldShapeDesc::isValid();
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
