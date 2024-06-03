#ifndef NX_FOUNDATION_NXSIMPLETRIANGLEMESH
#define NX_FOUNDATION_NXSIMPLETRIANGLEMESH
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup foundation
  @{
*/

#include "NxVec3.h"

/**
\brief Enum with flag values to be used in NxSimpleTriangleMesh::flags.
*/
enum NxMeshFlags
	{
	/**
	\brief Specifies if the SDK should flip normals.

	The Nx libraries assume that the face normal of a triangle with vertices [a,b,c] can be computed as:
	edge1 = b-a
	edge2 = c-a
	face_normal = edge1 x edge2.

	Note: This is the same as a counterclockwise winding in a right handed coordinate system or
	alternatively a clockwise winding order in a left handed coordinate system.

	If this does not match the winding order for your triangles, raise the below flag.
	*/
	NX_MF_FLIPNORMALS		=	(1<<0),
	NX_MF_16_BIT_INDICES	=	(1<<1),	//<! Denotes the use of 16-bit vertex indices
	NX_MF_HARDWARE_MESH		=	(1<<2),	//<! The mesh will be used in hardware scenes
	};

typedef NxVec3 NxPoint;

/**
\brief A structure describing a triangle mesh.
*/
class NxSimpleTriangleMesh
	{
	public:
	NxU32 numVertices;			//!< Number of vertices.
	NxU32 numTriangles;			//!< Number of triangles.
	NxU32 pointStrideBytes;		//!< Offset between vertex points in bytes.
	NxU32 triangleStrideBytes;	//!< Offset between triangles in bytes.

	/**
	\brief Pointer to first vertex point.
	
	Caller may add pointStrideBytes bytes to the pointer to access the next point.
	*/
	const void* points;

	/**
	\brief Pointer to first triangle.
	
	Caller may add triangleStrideBytes bytes to the pointer to access the next triangle.
	
	These are triplets of 0 based indices:
	vert0 vert1 vert2
	vert0 vert1 vert2
	vert0 vert1 vert2
	...

	where vertex is either a 32 or 16 bit unsigned integer. There are numTriangles*3 indices.

	This is declared as a void pointer because it is actually either an NxU16 or a NxU32 pointer.
	*/
	const void* triangles;

	/**
	\brief Flags bits, combined from values of the enum ::NxMeshFlags
	*/
	NxU32 flags;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxSimpleTriangleMesh();	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	\brief returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};


NX_INLINE NxSimpleTriangleMesh::NxSimpleTriangleMesh()
	{
	setToDefault();
	}

NX_INLINE void NxSimpleTriangleMesh::setToDefault()
	{
	numVertices			= 0;
	numTriangles		= 0;
	pointStrideBytes	= 0;
	triangleStrideBytes	= 0;
	points				= NULL;
	triangles			= NULL;
	flags				= 0;
	}

NX_INLINE bool NxSimpleTriangleMesh::isValid() const
	{
	// Check geometry
	if(numVertices > 0xffff && flags & NX_MF_16_BIT_INDICES)
		return false;
	if(!points)
		return false;
	if(pointStrideBytes < sizeof(NxPoint))	//should be at least one point's worth of data
		return false;

	// Check topology
	// The triangles pointer is not mandatory
	if(triangles)
		{
		// Indexed mesh
		if(flags & NX_MF_16_BIT_INDICES)
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
