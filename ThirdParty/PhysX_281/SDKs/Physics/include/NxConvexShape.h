#ifndef NX_COLLISION_NXCONVEXSHAPE
#define NX_COLLISION_NXCONVEXSHAPE
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

class NxConvexShapeDesc;

/**
\brief Used to represent an instance of an #NxConvexMesh.

<h3>Creation</h3>

Example:

\include NxConvexMesh_Create.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_COLLISION_AABBS
\li #NX_VISUALIZE_COLLISION_SHAPES
\li #NX_VISUALIZE_COLLISION_AXES
\li #NX_VISUALIZE_COLLISION_VNORMALS
\li #NX_VISUALIZE_COLLISION_FNORMALS
\li #NX_VISUALIZE_COLLISION_EDGES
\li #NX_VISUALIZE_COLLISION_SPHERES

@see NxConvexShapeDesc NxConvexMesh NxShape NxPhysicsSDK.createConvexMesh() NxActor.createShape()
*/
class NxConvexShape: public NxShape
	{
	public:

	/**

	\brief Save shape to desc.

	\param[out] desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxConvexShapeDesc
	*/
	virtual	void	saveToDesc(NxConvexShapeDesc& desc)	const = 0;

	/**
	\brief Retrieves the convex mesh data associated with this instance.

	\return The convex mesh associated with this shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxConvexMesh
	*/
	virtual	NxConvexMesh&		getConvexMesh() = 0;

	/**
	\brief Retrieves the convex mesh data associated with this instance.

	Const version.

	\return The convex mesh associated with this shape.

	@see NxConvexMesh
	*/
	virtual	const NxConvexMesh&	getConvexMesh() const = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
