#ifndef NX_COLLISION_NXTRIANGLEMESHSHAPE
#define NX_COLLISION_NXTRIANGLEMESHSHAPE
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
#include "NxUserEntityReport.h"

class NxTriangle;
class NxTriangleMeshShapeDesc;
class NxTriangleMesh;


/**
\brief This class is a shape instance of a triangle mesh object of type NxTriangleMesh.

Each shape is owned by an actor that it is attached to.

<h3>Creation</h3>

An instance can be created by calling the createShape() method of the NxActor object
that should own it, with a NxTriangleMeshShapeDesc object as the parameter, or by adding the 
shape descriptor into the NxActorDesc class before creating the actor.

The shape is deleted by calling NxActor::releaseShape() on the owning actor.

PPU: Collision detection will only be performed against mesh pages which have been mapped into
PPU memory using #mapPageInstance()

Example:

\include NxTriangleMeshShape_Create.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_COLLISION_AABBS
\li #NX_VISUALIZE_COLLISION_SHAPES
\li #NX_VISUALIZE_COLLISION_AXES
\li #NX_VISUALIZE_COLLISION_VNORMALS
\li #NX_VISUALIZE_COLLISION_FNORMALS
\li #NX_VISUALIZE_COLLISION_EDGES
\li #NX_VISUALIZE_COLLISION_SPHERES

@see NxTriangleMeshShapeDesc NxTriangleMesh
*/

class NxTriangleMeshShape: public NxShape
	{
	public:
	/*
	\brief Saves the state of the shape object to a descriptor.

	\param[out]  desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMeshShapeDesc
	*/
	virtual	void				saveToDesc(NxTriangleMeshShapeDesc& desc)	const = 0;

	/**
	\brief Retrieves the triangle mesh data associated with this instance.

	\return The triangle mesh associated with this shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMesh
	*/
	virtual	NxTriangleMesh&		getTriangleMesh() = 0;
	virtual	const NxTriangleMesh& getTriangleMesh() const = 0;

	/**
	\brief Retrieves triangle data from a triangle ID.

	This function can be used together with #overlapAABBTriangles() to retrieve triangle properties.

	\param[out] triangle triangle points in local or world space.
	\param[out] edgeTri World space edge normals for triangle (NULL to not compute).
	\param[out] flags Flags which show if an edge is convex. See #NxTriangleFlags
	\param[in] triangleIndex The index of the triangle to retrieve.
	\param[in] worldSpaceTranslation true to return triangle's position in world space, else false for local space
	\param[in] worldSpaceRotation true to return triangle's orientation in world space, else false for local space
	\return Unused.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangle NxTriangleFlags NxTriangleID overlapAABBTriangles()
	*/
	virtual	NxU32				getTriangle(NxTriangle& triangle, NxTriangle* edgeTri, NxU32* flags, NxTriangleID triangleIndex, bool worldSpaceTranslation=true, bool worldSpaceRotation=true) const	= 0;

	/**
	\brief Finds triangles touching the input bounds.

	\warning This method returns a pointer to an internal structure using the indices member. Hence the
	user should use or copy the indices before calling any other API function.

	\warning This method is deprecated and overlapAABBTriangles(const NxBounds3 bounds, NxU32 flags, NxUserEntityReport<NxU32>* callback) should
	be used in new code as it is more efficient and less error prone.

	The triangle indices returned by overlapAABBTriangles() can be used with #getTriangle() to retrieve the triangle properties.

	\param[in] bounds Bounds to test against. In object or world space depending on #NxQueryFlags. <b>Range:</b> See #NxBounds3
	\param[in] flags Controls if the bounds are in object or world space and if we return first contact only. See #NxQueryFlags.
	\param[out] nb Retrieves the number of triangle indices touching the AABB.
	\param[out] indices Returns an array of touching triangle indices.
	\return True if there is an overlap.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3 NxQueryFlags NxScene.overlapAABBShapes() getTriangle()
	*/
	NX_INLINE bool				overlapAABBTriangles(const NxBounds3& bounds, NxU32 flags, NxU32& nb, const NxU32*& indices)	const
		{
			return overlapAABBTrianglesDeprecated(bounds, flags, nb, indices);
		}

	virtual	bool				overlapAABBTrianglesDeprecated(const NxBounds3& bounds, NxU32 flags, NxU32& nb, const NxU32*& indices)	const	= 0;

	/**
	\brief Finds triangles touching the input bounds.

	The triangle indices returned by overlapAABBTriangles() can be used with #getTriangle() to retrieve the triangle properties.

	\param[in] bounds Bounds to test against. In object or world space depending on #NxQueryFlags. <b>Range:</b> See #NxBounds3
	\param[in] flags Controls if the bounds are in object or world space and if we return first contact only. See #NxQueryFlags.
	\param[in] callback Used to return triangles which intersect the AABB
	\return True if there is an overlap.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3 NxQueryFlags NxScene.overlapAABBShapes() getTriangle() NxUserEntityReport
	*/
	virtual	bool				overlapAABBTriangles(const NxBounds3& bounds, NxU32 flags, NxUserEntityReport<NxU32>* callback) const = 0;

	/**
	\brief Send a page to the PhysX processor.

	Ensure that a mesh instance corresponding to a mesh page is present on the PhysX processor. If the 
	page is not present on the processor, it is also sent. If the PhysX processor is not present, this call has
	no effect and returns false. If the page instance is already on the processor, this call has no effect and
	returns true.

	If the mesh is emulated in software then collision detection will still be performed, even if the pages
	of the mesh are not mapped. Since the software versions of the collision detection routines do not require
	the mesh to be mapped into hardware memory. 
	
	Cases where a mesh will be emulated in software(for 2.5):

	\li Dynamic(movable) triangle meshes.
	\li Triangle meshes with per triangle materials.
	\li mesh-mesh collision (pmaps).
	\li Heightfield triangle meshes.
	
	\param[in] pageIndex the index of the page to map. Pages are indexed sequentially starting from zero.
	\return False if there are insufficient resources available on the processor to accommodate the page or page
	instance. False is also returned when the mesh is being run in software.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see unmapPageInstance() isPageInstanceMapped()
	*/

	virtual bool				mapPageInstance(NxU32 pageIndex) = 0;

	/**
	\brief Release a page instance from the PhysX processor.

	Release a page instance corresponding to a mesh page from the PhysX processor. no other page instances
	reference the mesh page, the page is also released. If the PhysX card is not present, this call has no effect.

	\param[in] pageIndex the index of the page to unmap. Pages are indexed sequentially starting from zero.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isPageInstanceMapped() mapPageInstance()
	*/

	virtual void				unmapPageInstance(NxU32 pageIndex) = 0;

	/**
	\brief Determine whether a page instance is present from the PhysX processor.

	\param[in] pageIndex the index of the page being queried. Pages are indexed sequentially starting from zero.
	\return False if the page is not mapped or if the PhysX processor is not present. False is also returned 
	when the mesh is being emulated in software.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see mapPageInstance() unmapPageInstance()
	*/

	virtual bool				isPageInstanceMapped(NxU32 pageIndex) const = 0;

	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
