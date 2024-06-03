#ifndef NX_COLLISION_NXSPHERESHAPE
#define NX_COLLISION_NXSPHERESHAPE
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

class NxSphereShapeDesc;

/**
\brief A sphere shaped collision detection primitive.

Each shape is owned by an actor that it is attached to.

<h3>Creation</h3>

An instance can be created by calling the createShape() method of the NxActor object
that should own it, with a NxSphereShapeDesc object as the parameter, or by adding the 
shape descriptor into the NxActorDesc class before creating the actor.

The shape is deleted by calling NxActor::releaseShape() on the owning actor.

Example:

\include NxSphereShape_Create.cpp

<h3>Visualizations</h3>
\li NX_VISUALIZE_COLLISION_AABBS
\li NX_VISUALIZE_COLLISION_SHAPES
\li NX_VISUALIZE_COLLISION_AXES

@see NxSphereShapeDesc NxShape
*/

class NxSphereShape : public NxShape
	{
	public:
	/**
	\brief Sets the sphere radius.

	Call this to initialize or alter the sphere. If this is not called,
	then the default settings create a unit sphere at the origin.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] radius The new radius for the sphere. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void setRadius(NxReal radius) = 0;

	/**
	\brief Retrieves the radius of the sphere.

	\return The radius of the sphere.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/

	virtual NxReal getRadius()					const = 0;

	/**
	\brief Gets the sphere data in world space.

	\param[out] worldSphere Retrieves the description of the sphere in world space.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphere
	*/
	virtual void getWorldSphere(NxSphere& worldSphere)		const = 0;

	/*
	\brief Saves the state of the shape object to a descriptor.

	\param[out] desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphereShapeDesc
	*/
	virtual void saveToDesc(NxSphereShapeDesc& desc)	const = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
