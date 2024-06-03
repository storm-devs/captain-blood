#ifndef NX_PHYSICS_NXBOXFORCEFIELDSHAPE
#define NX_PHYSICS_NXBOXFORCEFIELDSHAPE
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

class NxBoxForceFieldShapeDesc;

/**
 \brief Box shaped region used to define force field.
 

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

 @see NxForceFieldShape
*/
class NxBoxForceFieldShape  : public NxForceFieldShape
{
	public:
	/**
	\brief Sets the box dimensions.

	The dimensions are the 'radii' of the box, meaning 1/2 extents in x dimension, 
	1/2 extents in y dimension, 1/2 extents in z dimension. 

	\param[in] vec The new 'radii' of the box. <b>Range:</b> direction vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxForceFieldShapeDesc.dimensions getDimensions()
	*/
	virtual void setDimensions(const NxVec3& vec) = 0;

	/**
	\brief Retrieves the dimensions of the box.

	The dimensions are the 'radii' of the box, meaning 1/2 extents in x dimension, 
	1/2 extents in y dimension, 1/2 extents in z dimension.

	\return The 'radii' of the box.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxForceFieldShapeDesc.dimensions setDimensions()
	*/
	virtual NxVec3 getDimensions() const = 0;

	/**
	\brief Saves the state of the shape object to a descriptor.

	\param[out] desc Descriptor to save to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxForceFieldShapeDesc
	*/
	virtual	void saveToDesc(NxBoxForceFieldShapeDesc& desc) const=0;
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
