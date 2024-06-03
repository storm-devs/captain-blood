#ifndef NX_COLLISION_NXCAPSULESHAPE
#define NX_COLLISION_NXCAPSULESHAPE
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

class NxShape;
class NxCapsule;
class NxCapsuleShapeDesc;

/**
\brief A capsule shaped collision detection primitive, also known as a line swept sphere.

'radius' is the radius of the capsule's hemispherical ends and its trunk.
'height' is the distance between the two hemispherical ends of the capsule.
The height is along the capsule's Y axis.

Each shape is owned by an actor that it is attached to.

<h3>Creation</h3>

An instance can be created by calling the createShape() method of the NxActor object
that should own it, with a NxCapsuleShapeDesc object as the parameter, or by adding the 
shape descriptor into the NxActorDesc class before creating the actor.

Example:

\include NxCapsuleShape_Create.cpp

The shape is deleted by calling NxActor::releaseShape() on the owning actor.

<h3>Visualizations</h3>
\li NX_VISUALIZE_COLLISION_AABBS
\li NX_VISUALIZE_COLLISION_SHAPES
\li NX_VISUALIZE_COLLISION_AXES

@see NxCapsuleShapeDesc NxShape NxActor.createShape()
*/

class NxCapsuleShape: public NxShape
	{
	public:
	/**
	\brief Call this to initialize or alter the capsule. 

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] radius The new radius of the capsule. <b>Range:</b> (0,inf)
	\param[in] height The new height of the capsule. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setRadius() setHeight()
	*/
	virtual void setDimensions(NxReal radius, NxReal height) = 0;

	/**
	\brief Alters the radius of the capsule.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] radius The new radius of the capsule. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setDimensions() NxCapsuleShapeDesc.radius getRadius()
	*/
	virtual void setRadius(NxReal radius) = 0;

	/**
	\brief Retrieves the radius of the capsule.

	\return The radius of the capsule.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see setRadius() setDimensions() NxCapsuleShapeDesc.radius
	*/
	virtual NxReal getRadius() const = 0;

	/**
	\brief Alters the height of the capsule. 

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] height The new height of the capsule. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see getHeight() NxCapsuleShapeDesc.height getRadius() setDimensions()
	*/
	virtual void setHeight(NxReal height) = 0;	

	/**
	\brief Retrieves the height of the capsule.

	\return The height of the capsule measured from end to end.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see setHeight() setRadius() NxCapsuleShapeDesc.height
	*/
	virtual NxReal getHeight() const = 0;

	/**
	\brief Retrieves the capsule parameters in world space. See #NxCapsule.

	\param[out] worldCapsule Use to retrieve the capsule parameters in world space.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see NxCapsule
	*/
	virtual void getWorldCapsule(NxCapsule& worldCapsule)	const	= 0;

	/*
	\brief Saves the state of the shape object to a descriptor.

	\param[out] desc Descriptor to save the state of the object to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see NxCapsuleShapeDesc
	*/
	virtual	void	saveToDesc(NxCapsuleShapeDesc& desc)		const = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
