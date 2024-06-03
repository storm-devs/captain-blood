#ifndef NX_COLLISION_NXBOXSHAPE
#define NX_COLLISION_NXBOXSHAPE
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
#include "NxShape.h"

class NxBox;
class NxBoxShapeDesc;

/**
\brief A box shaped collision detection primitive.

Each shape is owned by the actor which it is attached to.

<h3>Creation</h3>

An instance can be created by calling the createShape() method of the NxActor object
that will own it, with a NxBoxShapeDesc object as the parameter, or by adding the 
shape descriptor to the NxActorDesc class before creating the actor.

Example (adding shape descriptor to an NxActorDesc):

\include NxBoxShape_CreateDesc.cpp

Example (creating for an already existing actor):

\include NxBoxShape_CreateWithActor.cpp

The shape is deleted by calling NxActor::releaseShape() on the owning actor.

<h3>Visualizations</h3>
\li NX_VISUALIZE_COLLISION_AABBS
\li NX_VISUALIZE_COLLISION_SHAPES
\li NX_VISUALIZE_COLLISION_AXES

@see NxBoxShapeDesc NxShape NxActor.createShape()
*/
class NxBoxShape: public NxShape
	{
	public:
	/**
	\brief Sets the box dimensions.

	The dimensions are the 'radii' of the box, meaning 1/2 extents in x dimension, 
	1/2 extents in y dimension, 1/2 extents in z dimension. 

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] vec The new 'radii' of the box. <b>Range:</b> direction vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxShapeDesc.dimensions getDimensions()
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

	@see NxBoxShapeDesc.dimensions setDimensions()
	*/
	virtual NxVec3 getDimensions() const = 0;

	/**
	\brief Gets the box represented as a world space OBB.

	\param[out] obb The orientated bounding box in the global frame. See #NxBox.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBox
	*/
	virtual void getWorldOBB(NxBox& obb) const = 0;

	/**
	\brief Saves the state of the shape object to a descriptor.

	\param[out] desc Descriptor to save to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxShapeDesc
	*/
	virtual	void saveToDesc(NxBoxShapeDesc& desc) const = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
