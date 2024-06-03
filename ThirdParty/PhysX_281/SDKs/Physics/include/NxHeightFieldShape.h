#ifndef NX_COLLISION_NXHEIGHTFIELDSHAPE
#define NX_COLLISION_NXHEIGHTFIELDSHAPE
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
class NxHeightFieldShapeDesc;
class NxHeightField;


/**
\brief This class is a shape instance of a height field object of type NxHeightField.

Each shape is owned by an actor that it is attached to.

<h3>Creation</h3>

An instance can be created by calling the createShape() method of the NxActor object
that should own it, with a NxHeightFieldShapeDesc object as the parameter, or by adding the 
shape descriptor into the NxActorDesc class before creating the actor.

The shape is deleted by calling NxActor::releaseShape() on the owning actor.

Example:

\include NxHeightFieldShape_Create.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_COLLISION_AABBS
\li #NX_VISUALIZE_COLLISION_SHAPES
\li #NX_VISUALIZE_COLLISION_AXES
\li #NX_VISUALIZE_COLLISION_VNORMALS
\li #NX_VISUALIZE_COLLISION_FNORMALS
\li #NX_VISUALIZE_COLLISION_EDGES
\li #NX_VISUALIZE_COLLISION_SPHERES

@see NxHeightFieldShapeDesc NxHeightField
*/

class NxHeightFieldShape: public NxShape
	{
	public:
	/**
	\brief Saves the state of the shape object to a descriptor.

	\param[out]  desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldShapeDesc
	*/
	virtual	void				saveToDesc(NxHeightFieldShapeDesc& desc)	const = 0;

	/**
	\brief Retrieves the height field data associated with this instance.

	\return The height field associated with this shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightField
	*/
	virtual	NxHeightField&		getHeightField()							const = 0;

	/**
	\brief Retrieves the multiplier to transform sample height values to shape space y coordinates.
	
	\return The multiplier to transform sample height values to shape space y coordinates.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldShapeDesc.heightScale
	*/
	virtual NxReal				getHeightScale()							const = 0;

	/**
	\brief Retrieves the multiplier to transform height field rows to shape space x coordinates.
	
	\return The multiplier to transform height field rows to shape space x coordinates.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldShapeDesc.rowScale
	*/
	virtual NxReal				getRowScale()								const = 0;
	
	/**
	\brief Retrieves the multiplier to transform height field columns to shape space z coordinates.
	
	\return The multiplier to transform height field columns to shape space z coordinates.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes
	
	@see NxHeightFieldShapeDesc.columnScale
	*/
	virtual NxReal				getColumnScale()							const = 0;

	/**
	\brief Sets the multiplier to transform sample height values to shape space y coordinates.
	
	\param scale The multiplier to transform sample height values to shape space y coordinates. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldShapeDesc.heightScale
	*/
	virtual void				setHeightScale(NxReal scale) = 0;

	/**
	\brief Sets the multiplier to transform height field rows to shape space x coordinates.
	
	\param scale The multiplier to transform height field rows to shape space x coordinates. <b>Range:</b> (-inf,0), (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldShapeDesc.rowScale
	*/
	virtual void				setRowScale(NxReal scale) = 0;
	
	/**
	\brief Sets the multiplier to transform height field columns to shape space z coordinates.
	
	\param scale The multiplier to transform height field columns to shape space z coordinates. <b>Range:</b> (-inf,0), (0,inf)
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldShapeDesc.columnScale
	*/
	virtual void				setColumnScale(NxReal scale) = 0;

	/**
	\brief Retrieves triangle data from a triangle ID.

	\param[out] worldTri World space triangle points.
	\param[out] edgeTri World space edge normals for triangle (NULL to not compute).
	\param[out] flags Flags which show if an edge is convex. See #NxTriangleFlags
	\param[in] triangleIndex The index of the triangle to retrieve.
	\param[in] worldSpaceTranslation True if the triangle should be translated to world space.
	\param[in] worldSpaceRotation True if the triangle should be rotated to world space.

	\return Zero if the triangle does not exist or has the hole material. Nonzero otherwise.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangle NxTriangleFlags NxTriangleID
	*/
	virtual	NxU32				getTriangle(NxTriangle& worldTri, NxTriangle* edgeTri, NxU32* flags, NxTriangleID triangleIndex, bool worldSpaceTranslation=true, bool worldSpaceRotation=true) const	= 0;

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
	\brief Checks if the point in shape space projects onto the height field surface.
	\return True if the point projects onto the height field surface, false otherwise.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool				isShapePointOnHeightField(NxReal x, NxReal z) const = 0;

	/**
	\brief Returns the interpolated height at the given point in shape space.
	\return The interpolated height at the given point, or 0 if the point is out of range. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal				getHeightAtShapePoint(NxReal x, NxReal z) const = 0;

	/**
	\brief Returns the material index at the given point in shape space.
	The return value is the 7 low order bits as set in the samples array.
	The value may be compared directly with the hole material to determine
	if the heightfield has a hole at the given point.

	\return The low bits material index, or 0xFFFF if the point is out of range. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldShapeDesc.holeMaterial
	*/
	virtual NxMaterialIndex		getMaterialAtShapePoint(NxReal x, NxReal z) const = 0;

	/**
	\brief Returns the normal of the heightfield surface at the given point in shape space.
	\return The normal at the given point, or <0,0,0> if the point is out of range. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxVec3				getNormalAtShapePoint(NxReal x, NxReal z) const = 0;
	
	/**
	\brief Returns the smoothed normal of the heightfield surface at the given point in shape space.
	\return The smoothed normal at the given point, or <0,0,0> if the point is out of range. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxVec3				getSmoothNormalAtShapePoint(NxReal x, NxReal z) const = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
