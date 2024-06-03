#ifndef NX_COLLISION_NXTRIANGLEMESHDESC
#define NX_COLLISION_NXTRIANGLEMESHDESC
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

#include "NxTriangleMesh.h"
#include "NxSimpleTriangleMesh.h"

/**
\brief Descriptor class for #NxTriangleMesh.

Note that this class is derived from NxSimpleTriangleMesh which contains the members that describe the basic mesh.
The mesh data is *copied* when an NxTriangleMesh object is created from this descriptor. After the call the
user may discard the triangle data.

@see NxTriangleMesh NxTriangleMeshShape
*/
class NxTriangleMeshDesc : public NxSimpleTriangleMesh
	{
	public:
	/**
	If materialIndices is NULL (not used) then this should be zero. Otherwise this is the
	offset between material indices in bytes. This is at least sizeof(NxMaterialIndex).

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fall-back)
	\li PS3  : Yes
	\li XB360: Yes

	@see materialIndices
	*/
	NxU32					materialIndexStride;

	/**
	Optional pointer to first material index, or NULL. There are NxSimpleTriangleMesh::numTriangles indices in total.
	Caller may add materialIndexStride bytes to the pointer to access the next triangle.

	When a triangle mesh collides with another object, a material is required at the collision point.
	If materialIndices is NULL, then the material of the NxTriangleMeshShape instance (specified via NxShapeDesc::materialIndex) is used.
	Otherwise, if the point of contact is on a triangle with index i, then the material index is determined as: 
	NxMaterialIndex	index = *(NxMaterialIndex *)(((NxU8*)materialIndices) + materialIndexStride * i);

	If the contact point falls on a vertex or an edge, a triangle adjacent to the vertex or edge is selected, and its index
	used to look up a material. The selection is arbitrary but consistent over time. 

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fall-back)
	\li PS3  : Yes
	\li XB360: Yes

	@see materialIndexStride
	*/
	const void*				materialIndices;

	/**
	\brief Deprecated

	\warning This member is deprecated and will no longer be supported. Use the specialized	#NxHeightField
	class instead.

	The mesh may represent either an arbitrary mesh or a height field. The advantage of a height field
	is that it is assumed to be semi-infinite along one axis, and therefore it doesn't have the problem
	of fast moving objects 'popping' through it due to temporal under sampling.

	However, height fields must be 'flat' in the sense that the projections of all triangles onto the
	height field plane must be disjoint. (If the height field vertical axis is Y, the height field plane is spanned by X and Z.)

	To create a height field, set heightFieldVerticalAxis to NX_X, NX_Y or NX_Z, 
	or leave it set to NX_NOT_HEIGHTFIELD for an arbitrary mesh.

	<b>Default:</b> NX_NOT_HEIGHTFIELD

	<b>Platform:</b>
	\li PC SW: Deprecated
	\li PPU  : Deprecated
	\li PS3  : Deprecated
	\li XB360: Deprecated

	@see NxHeightFieldAxis heightFieldVerticalExtent
	*/
	NxHeightFieldAxis		heightFieldVerticalAxis;

	/**
	\brief Deprecated

	\warning This member is deprecated and will no longer be supported. Use the specialized	#NxHeightField
	class instead.

	If this mesh is a height field, this sets how far 'below ground' the height volume extends.

	In this way even objects which are under the surface of the height field but above
	this cutoff are treated as colliding with the height field. To create a height field with the up axis being
	the Y axis, you need to set the heightFieldVerticalAxis to Y and the heightFieldVerticalExtent to a large negative number. 

	The heightFieldVerticalExtent has to be outside of the vertex coordinate range of the mesh along the heightFieldVerticalAxis.

	You may set this to a positive value, in which case the extent will be cast along the opposite side of the height field.

	You may use a smaller finite value for the extent if you want to put some space under the height field, such as a cave.

	<b>Range:</b> (-inf,inf)<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Deprecated
	\li PPU  : Deprecated
	\li PS3  : Deprecated
	\li XB360: Deprecated

	@see heightFieldVerticalAxis
	*/
	NxReal					heightFieldVerticalExtent;

	/**
	The pmap is an optional data structure which makes mesh-mesh collision detection more robust at the cost of higher
	memory consumption. A pmap can be created with ::NxCreatePMap and released with ::NxReleasePMap.
	You may also save the output of ::NxCreatePMap do disc to avoid this preprocessing step.

	\warning Legacy member

	The pmap data will not be copied. For this reason the caller does not need to keep it around for the lifetime of 
	the NxTriangleMesh object.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMesh.loadPMap
	*/
	NxPMap*					pmap;

	/**
	The SDK computes convex edges of a mesh and use them for collision detection. This parameter allows you to
	setup a tolerance for the convex edge detector.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 0.001

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxReal					convexEdgeThreshold;

	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxTriangleMeshDesc();	
	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	
	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxTriangleMeshDesc::NxTriangleMeshDesc()	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxTriangleMeshDesc::setToDefault()
	{
	NxSimpleTriangleMesh::setToDefault();
	materialIndexStride			= 0;
	materialIndices				= 0;
	heightFieldVerticalAxis		= NX_NOT_HEIGHTFIELD;
	heightFieldVerticalExtent	= 0;
	convexEdgeThreshold			= 0.001f;
	pmap						= NULL;
	}

NX_INLINE bool NxTriangleMeshDesc::isValid() const
	{
	if(numVertices < 3) 	//at least 1 trig's worth of points
		return false;
	if ((!triangles) && (numVertices%3))		// Non-indexed mesh => we must ensure the geometry defines an implicit number of triangles // i.e. numVertices can't be divided by 3
		return false;
	//add more validity checks here
	if (materialIndices && materialIndexStride < sizeof(NxMaterialIndex))
		return false;
	return NxSimpleTriangleMesh::isValid();
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
