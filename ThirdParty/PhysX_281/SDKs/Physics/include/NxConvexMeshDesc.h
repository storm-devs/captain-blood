#ifndef NX_COLLISION_NXCONVEXMESHDESC
#define NX_COLLISION_NXCONVEXMESHDESC
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

#include "NxConvexMesh.h"

/**
\brief Flags which describe the format and behavior of a convex mesh.
*/
enum NxConvexFlags
	{
	/**
	\brief Used to flip the normals if the winding order is reversed.

	The Nx libraries assume that the face normal of a triangle with vertices [a,b,c] can be computed as:
	edge1 = b-a
	edge2 = c-a
	face_normal = edge1 x edge2.

	Note: this is the same as counterclockwise winding in a right handed graphics coordinate system.

	If this does not match the winding order for your triangles, raise the below flag.
	*/
	NX_CF_FLIPNORMALS		=	(1<<0),

	/**
	Denotes the use of 16-bit vertex indices in NxConvexMeshDesc::triangles.
	(otherwise, 32-bit indices are assumed)
	@see #NxConvexMeshDesc.triangles
	*/
	NX_CF_16_BIT_INDICES	=	(1<<1),

	/**
	Automatically recomputes the hull from the vertices. If this flag is not set, you must provide the entire geometry manually.
	*/
	NX_CF_COMPUTE_CONVEX	=	(1<<2),	

	/**
	\brief Inflates the convex object according to skin width

	\note This flag is only used in combination with NX_CF_COMPUTE_CONVEX.

	@see NxCookingParams
	*/
	NX_CF_INFLATE_CONVEX	=	(1<<3),

	/**
	\brief Instructs cooking to save normals uncompressed.  The cooked hull data will be larger, but will load faster.

	@see NxCookingParams
	*/
	NX_CF_USE_UNCOMPRESSED_NORMALS	=	(1<<5),
	};

typedef NxVec3 NxPoint;

/**
\brief Descriptor class for #NxConvexMesh.

@see NxConvexMesh NxConvexShape NxPhysicsSDK.createConvexMesh()

*/
class NxConvexMeshDesc
	{
	public:

	/**
	\brief Number of vertices.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Max limit of 32 vertices for hardware rigid bodies)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32 numVertices;
	
	/**
	\brief Number of triangles.

	Hardware rigid body scenes have a limit of 32 faces per convex.
	Fluid scenes have a limit of 64 faces per cooked convex for dynamic actors.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Max 32 faces)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32 numTriangles;
	
	/**
	\brief Offset between vertex points in bytes.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32 pointStrideBytes;
	
	/**
	\brief Offset between triangles in bytes.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32 triangleStrideBytes;

	/**
	\brief Pointer to array of vertex positions.
	Pointer to first vertex point. Caller may add pointStrideBytes bytes to the pointer to access the next point.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	const void* points;

	/**
	\brief Pointer to array of triangle indices.
	<p>Pointer to first triangle. Caller may add triangleStrideBytes bytes to the pointer to access the next triangle.</p>
	<p><pre>These are triplets of 0 based indices:
	vert0 vert1 vert2
	vert0 vert1 vert2
	vert0 vert1 vert2
	...</pre></p>

	<p>Where vertex is either a 32 or 16 bit unsigned integer. There are numTriangles*3 indices.</p>

	<p>This is declared as a void pointer because it is actually either an NxU16 or a NxU32 pointer.</p>

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxConvexFlags.NX_CF_16_BIT_INDICES
	*/
	const void* triangles;

	/**
	\brief Flags bits, combined from values of the enum ::NxConvexFlags

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32 flags;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxConvexMeshDesc();
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	\brief Returns true if the descriptor is valid.

	\return True if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxConvexMeshDesc::NxConvexMeshDesc()	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxConvexMeshDesc::setToDefault()
	{
	numVertices			= 0;
	numTriangles		= 0;
	pointStrideBytes	= 0;
	triangleStrideBytes	= 0;
	points				= NULL;
	triangles			= NULL;
	flags				= 0;
	}

NX_INLINE bool NxConvexMeshDesc::isValid() const
	{
	// Check geometry
	if(numVertices < 3 ||	//at least 1 trig's worth of points
		(numVertices > 0xffff && flags & NX_CF_16_BIT_INDICES))
		return false;
	if(!points)
		return false;
	if(pointStrideBytes < sizeof(NxPoint))	//should be at least one point's worth of data
		return false;

	// Check topology
	// The triangles pointer is not mandatory: the vertex cloud is enough to define the convex hull.
	if(triangles)
		{
		// Indexed mesh
		if(numTriangles < 2)	//some algos require at least 2 trigs
			return false;
		if(flags & NX_CF_16_BIT_INDICES)
			{
			if((triangleStrideBytes < sizeof(NxU16)*3))
				return false;
			}
		else
			{
			if((triangleStrideBytes < sizeof(NxU32)*3))
				return false;
			}
		}
	else
		{
		// We can compute the hull from the vertices
		if(!(flags & NX_CF_COMPUTE_CONVEX))
			return false;	// If the mesh is convex and we're not allowed to compute the hull,
							// you have to provide it completely (geometry & topology).
		}
	return true;
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
