#ifndef NX_PHYSICS_NXLEGACYEXPORTS
#define NX_PHYSICS_NXLEGACYEXPORTS
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
#include "PhysXLoader.h"

struct NxIntegrals;
/*
These are functions that used to be exported for the user from the foundation DLL, which doesn't exist anymore.
They are kept here and exported from the physics SDK DLL for backwards compatibility.
*/

/**

\brief Test if an oriented box contains a point.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param[in] box Oriented Box to test point against.
\param[in] p Point to test. 

\return True if the box contains p.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBox
*/
NX_INLINE bool NxBoxContainsPoint(const NxBox& box, const NxVec3& p)
	{
	return NxGetUtilLib()->NxBoxContainsPoint(box,p);
	}

/**

\brief Create an oriented box from an axis aligned box and a transformation.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param[out] box Used to store the oriented box.
\param[in] aabb Axis aligned box.
\param[in] mat Transformation to apply to the axis aligned box.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBox NxBounds3
*/
NX_INLINE void NxCreateBox(NxBox& box, const NxBounds3& aabb, const NxMat34& mat)
	{
	NxGetUtilLib()->NxCreateBox(box,aabb,mat);
	}

/**

\brief Computes plane equation for each face of an oriented box.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param[in] box The oriented box.
\param[out] planes Array to receive the computed planes(should be large enough to hold 6 planes)

\return True on success.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBox NxPlane
*/
NX_INLINE bool NxComputeBoxPlanes(const NxBox& box, NxPlane* planes)
	{
	return NxGetUtilLib()->NxComputeBoxPlanes(box,planes);
	}

/**

\brief Compute the corner points of an oriented box.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param[in] box The oriented box.
\param[out] pts Array to receive the box point (should be large enough to hold 8 points)

\return True on success.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBox
*/
NX_INLINE bool NxComputeBoxPoints(const NxBox& box, NxVec3* pts)
	{
	return NxGetUtilLib()->NxComputeBoxPoints(box,pts);
	}

/**

\brief Compute the vertex normals of an oriented box. These are smooth normals, i.e. averaged from the faces of the box.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param[in] box The oriented box.
\param[out] pts The normals for each vertex(should be large enough to hold 8 normals).

\return True on success.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBox
*/
NX_INLINE bool NxComputeBoxVertexNormals(const NxBox& box, NxVec3* pts)
	{
	return NxGetUtilLib()->NxComputeBoxVertexNormals(box,pts);
	}

/**
\brief Return a list of edge indices.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\return List of edge indices.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxComputeBoxPoints
*/
NX_INLINE const NxU32* NxGetBoxEdges()
	{
	return NxGetUtilLib()->NxGetBoxEdges();
	}

/**
\brief Return a list of box edge axes.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\return List of box edge axes.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxComputeBoxPoints
*/
NX_INLINE const NxI32* NxGetBoxEdgesAxes()
	{
	return NxGetUtilLib()->NxGetBoxEdgesAxes();
	}

/**
\brief Return a set of triangle indices suitable for use with #NxComputeBoxPoints.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\return List of box triangles.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxComputeBoxPoints
*/
NX_INLINE const NxU32* NxGetBoxTriangles()
	{
	return NxGetUtilLib()->NxGetBoxTriangles();
	}

/**
\brief Returns a list of local space edge normals.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\return List of edge normals.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE const NxVec3* NxGetBoxLocalEdgeNormals()
	{
	return NxGetUtilLib()->NxGetBoxLocalEdgeNormals();
	}

/**
\brief Compute and edge normals for an oriented box.

This is an averaged normal, from the two faces sharing the edge.

The edge index should be from 0 to 11 (i.e. a box has 12 edges).

Edge ordering:

\image html boxEdgeDiagram.png

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param[in] box The oriented box.
\param[in] edge_index The index of the edge to compute a normal for.
\param[out] world_normal The computed normal.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxComputeBoxWorldEdgeNormal(const NxBox& box, NxU32 edge_index, NxVec3& world_normal)
	{
	NxGetUtilLib()->NxComputeBoxWorldEdgeNormal(box,edge_index,world_normal);
	}

/**

\brief Compute a capsule which encloses a box.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param box Box to generate capsule for.
\param capsule Stores the capsule which is generated.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBox NxCapsule NxComputeBoxAroundCapsule
*/
NX_INLINE void NxComputeCapsuleAroundBox(const NxBox& box, NxCapsule& capsule)
	{
	NxGetUtilLib()->NxComputeCapsuleAroundBox(box,capsule);
	}

/**
\brief Test if box A is inside another box B.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param a Box A
\param b Box B

\return True if box A is inside box B.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBox
*/
NX_INLINE bool NxIsBoxAInsideBoxB(const NxBox& a, const NxBox& b)
	{
	return NxGetUtilLib()->NxIsBoxAInsideBoxB(a,b);
	}

/**
\brief Get a list of indices representing the box as quads.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\return List of quad indices.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxComputeBoxPoints()
*/
NX_INLINE const NxU32* NxGetBoxQuads()
	{
	return NxGetUtilLib()->NxGetBoxQuads();
	}

/**
\brief Returns a list of quad indices sharing the vertex index.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param vertexIndex Vertex Index.
\return List of quad indices sharing the vertex index.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxComputeBoxPoints() NxGetBoxQuads()
*/
NX_INLINE const NxU32* NxBoxVertexToQuad(NxU32 vertexIndex)
	{
	return NxGetUtilLib()->NxBoxVertexToQuad(vertexIndex);
	}

/**
\brief Compute a box which encloses a capsule.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param capsule Capsule to generate an enclosing box for.
\param box Generated box.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxComputeCapsuleAroundBox
*/
NX_INLINE void NxComputeBoxAroundCapsule(const NxCapsule& capsule, NxBox& box)
	{
	NxGetUtilLib()->NxComputeBoxAroundCapsule(capsule,box);
	}

/**
\brief Set FPU precision.

\warning #NxCreatePhysicsSDK() must be called before using this function.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxSetFPUPrecision24()
	{
	NxGetUtilLib()->NxSetFPUPrecision24();
	}

/**
\brief Set FPU precision.

\warning #NxCreatePhysicsSDK() must be called before using this function.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxSetFPUPrecision53()
	{
	NxGetUtilLib()->NxSetFPUPrecision53();
	}

/**
\brief Set FPU precision

\warning #NxCreatePhysicsSDK() must be called before using this function.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxSetFPUPrecision64()
	{
	NxGetUtilLib()->NxSetFPUPrecision64();
	}

/**
\brief Set FPU precision.

\warning #NxCreatePhysicsSDK() must be called before using this function.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxSetFPURoundingChop()
	{
	NxGetUtilLib()->NxSetFPURoundingChop();
	}

/**
\brief Set FPU rounding mode.

\warning #NxCreatePhysicsSDK() must be called before using this function.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxSetFPURoundingUp()
	{
	NxGetUtilLib()->NxSetFPURoundingUp();
	}

/**
\brief Set FPU rounding mode.

\warning #NxCreatePhysicsSDK() must be called before using this function.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxSetFPURoundingDown()
	{
	NxGetUtilLib()->NxSetFPURoundingDown();
	}

/**
\brief Set FPU rounding mode.

\warning #NxCreatePhysicsSDK() must be called before using this function.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxSetFPURoundingNear()
	{
	NxGetUtilLib()->NxSetFPURoundingNear();
	}

/**
\brief Enable/Disable FPU exception.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param b True to enable exception.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxSetFPUExceptions(bool b)
	{
	NxGetUtilLib()->NxSetFPUExceptions(b);
	}

/**
\brief Convert a floating point number to an integer.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param f Floating point number.

\return The result.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE int NxIntChop(const NxF32& f)
	{
	return NxGetUtilLib()->NxIntChop(f);
	}

/**
\brief Convert a floating point number to an integer.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param f Floating point number.

\return The result.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE int NxIntFloor(const NxF32& f)
	{
	return NxGetUtilLib()->NxIntFloor(f);
	}

/**
\brief Convert a floating point number to an integer.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param f Floating point number.

\return The result.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE int NxIntCeil(const NxF32& f)
	{
	return NxGetUtilLib()->NxIntCeil(f);
	}

/**
\brief Compute the distance squared from a point to a ray.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param ray The ray.
\param point The point.
\param t Used to retrieve the closest parameter value on the ray.

\return The squared distance.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxRay
*/
NX_INLINE NxF32 NxComputeDistanceSquared(const NxRay& ray, const NxVec3& point, NxF32* t)
	{
	return NxGetUtilLib()->NxComputeDistanceSquared(ray,point,t);
	}

/**
\brief Compute the distance squared from a point to a line segment.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param seg The line segment.
\param point The point.
\param t Used to retrieve the closest parameter value on the line segment.

\return The squared distance.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxSegment
*/
NX_INLINE NxF32 NxComputeSquareDistance(const NxSegment& seg, const NxVec3& point, NxF32* t)
	{
	return NxGetUtilLib()->NxComputeSquareDistance(seg,point,t);
	}

/**
\brief Compute a bounding sphere for a point cloud.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param sphere The computed sphere.
\param nb_verts Number of points.
\param verts Array of points.

\return The method used to compute the sphere, see #NxBSphereMethod.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxSphere NxFastComputeSphere
*/
NX_INLINE NxBSphereMethod NxComputeSphere(NxSphere& sphere, unsigned nb_verts, const NxVec3* verts)
	{
	return NxGetUtilLib()->NxComputeSphere(sphere,nb_verts,verts);
	}
/**
\brief Compute a bounding sphere for a point cloud.

The sphere may not be as tight as #NxComputeSphere

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param sphere The computed sphere.
\param nb_verts Number of points.
\param verts Array of points.

\return True on success.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxSphere NxComputeSphere
*/
NX_INLINE bool NxFastComputeSphere(NxSphere& sphere, unsigned nb_verts, const NxVec3* verts)
	{
	return NxGetUtilLib()->NxFastComputeSphere(sphere,nb_verts,verts);
	}

/**
\brief Compute an overall bounding sphere for a pair of spheres.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param merged The computed sphere.
\param sphere0 First sphere.
\param sphere1 Second sphere.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxSphere NxComputeSphere
*/
NX_INLINE void NxMergeSpheres(NxSphere& merged, const NxSphere& sphere0, const NxSphere& sphere1)
	{
	NxGetUtilLib()->NxMergeSpheres(merged,sphere0,sphere1);
	}

/**
\brief Get the tangent vectors associated with a normal.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param n Normal vector
\param t1 First tangent
\param t2 Second tangent

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxNormalToTangents(const NxVec3 & n, NxVec3 & t1, NxVec3 & t2)
	{
	NxGetUtilLib()->NxNormalToTangents(n,t1,t2);
	}

/**
\brief Rotates a 3x3 symmetric inertia tensor I into a space R where it can be represented with the diagonal matrix D.

I = R * D * R'

Returns false on failure. 

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param denseInertia The dense inertia tensor.
\param diagonalInertia The diagonalized inertia tensor.
\param rotation Rotation for the frame of the diagonalized inertia tensor.

\return True if the inertia tensor can be diagonalized.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/

NX_INLINE bool NxDiagonalizeInertiaTensor(const NxMat33 & denseInertia, NxVec3 & diagonalInertia, NxMat33 & rotation)
	{
	return NxGetUtilLib()->NxDiagonalizeInertiaTensor(denseInertia,diagonalInertia,rotation);
	}

/**
\brief  Computes a rotation matrix.

computes rotation matrix M so that:

M * x = b

x and b are unit vectors.

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param x Vector.
\param b Vector.
\param M Computed rotation matrix.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE void NxFindRotationMatrix(const NxVec3 & x, const NxVec3 & b, NxMat33 & M)
	{
	NxGetUtilLib()->NxFindRotationMatrix(x,b,M);
	}

/**
\brief Computes bounds of an array of vertices

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param min Computed minimum of the bounds.
\param max Maximum
\param nbVerts Number of input vertices.
\param verts Array of vertices.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBounds3
*/
NX_INLINE void NxComputeBounds(NxVec3& min, NxVec3& max, NxU32 nbVerts, const NxVec3* verts)
	{
	NxGetUtilLib()->NxComputeBounds(min,max,nbVerts,verts);
	}

/**
\brief Computes bounds of an array of vertices

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param bounds Computed bounds.
\param nbVerts Number of input vertices.
\param verts Array of vertices.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxBounds3
*/
NX_INLINE void NxComputeBounds(NxBounds3& bounds, NxU32 nbVerts, const NxVec3* verts)
	{
	NxVec3 min, max;
	NxComputeBounds(min, max, nbVerts, verts);
	bounds.set(min, max);
	}

/**
\brief Computes CRC of input buffer

\warning #NxCreatePhysicsSDK() must be called before using this function.

\param buffer Input buffer.
\param nbBytes Number of bytes in in the input buffer.
\return The computed CRC.

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
NX_INLINE NxU32 NxCrc32(const void* buffer, NxU32 nbBytes)
	{
	return NxGetUtilLib()->NxCrc32(buffer,nbBytes);
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
