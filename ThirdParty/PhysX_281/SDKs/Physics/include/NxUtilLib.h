#ifndef NX_UTIL_LIB
#define NX_UTIL_LIB
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxFoundation.h"
#include "Nxp.h"
class NxJointDesc;
class NxTriangle;

// For NxSweepBoxTriangles and NxSweepCapsuleTriangles
enum NxTriangleCollisionFlag
	{
	// Must be the 3 first ones to be indexed by (flags & (1<<edge_index))
	TCF_ACTIVE_EDGE01	= (1<<0),	//!< Enable collision with edge 0-1
	TCF_ACTIVE_EDGE12	= (1<<1),	//!< Enable collision with edge 1-2
	TCF_ACTIVE_EDGE20	= (1<<2),	//!< Enable collision with edge 2-0
	TCF_DOUBLE_SIDED	= (1<<3),	//!< Triangle is double-sided
	};

/**
\brief Abstract interface used to expose utility functions

The functions contained within NxUtil were previously exposed as static exports from NxPhysics.dll.
However to allow a switchable PhysXCore, they are now exposed through this interface.
*/
class NxUtilLib
	{

	public:
/*
** From NxExportedUtils.h
*/
/***************************************************************************/
	/**
	\brief Test if an oriented box contains a point.

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
	virtual bool NxBoxContainsPoint(const NxBox& box, const NxVec3& p)=0;

	/**

	\brief Create an oriented box from an axis aligned box and a transformation.

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
	virtual void NxCreateBox(NxBox& box, const NxBounds3& aabb, const NxMat34& mat)=0;

	/**

	\brief Computes plane equation for each face of an oriented box.

	\param[in] box The oriented box.
	\param[out] planes Array to receive the computed planes (should be large enough to hold 6 planes)

	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBox NxPlane
	*/
	virtual bool NxComputeBoxPlanes(const NxBox& box, NxPlane* planes)=0;

	/**

	\brief Compute the corner points of an oriented box.

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
	virtual bool NxComputeBoxPoints(const NxBox& box, NxVec3* pts)=0;

	/**

	\brief Compute the vertex normals of an oriented box. These are smooth normals, i.e. averaged from the faces of the box.

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
	virtual bool NxComputeBoxVertexNormals(const NxBox& box, NxVec3* pts)=0;

	/**
	\brief Return a list of edge indices.

	\return List of edge indices.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxComputeBoxPoints
	*/
	virtual const NxU32* NxGetBoxEdges()=0;

	/**
	\brief Return a list of box edge axes.

	\return List of box edge axes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxComputeBoxPoints
	*/
	virtual const NxI32* NxGetBoxEdgesAxes()=0;

	/**
	\brief Return a set of triangle indices suitable for use with #NxComputeBoxPoints.

	\return List of box triangles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxComputeBoxPoints
	*/
	virtual const NxU32* NxGetBoxTriangles()=0;

	/**
	\brief Returns a list of local space edge normals.

	\return List of edge normals.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual const NxVec3* NxGetBoxLocalEdgeNormals()=0;

	/**
	\brief Compute and edge normals for an oriented box.

	This is an averaged normal, from the two faces sharing the edge.

	The edge index should be from 0 to 11 (i.e. a box has 12 edges).

	Edge ordering:

	\image html boxEdgeDiagram.png

	\param[in] box The oriented box.
	\param[in] edge_index The index of the edge to compute a normal for.
	\param[out] world_normal The computed normal.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxComputeBoxWorldEdgeNormal(const NxBox& box, NxU32 edge_index, NxVec3& world_normal)=0;

	/**

	\brief Compute a capsule which encloses a box.

	\param box Box to generate capsule for.
	\param capsule Stores the capsule which is generated.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBox NxCapsule NxComputeBoxAroundCapsule
	*/
	virtual void NxComputeCapsuleAroundBox(const NxBox& box, NxCapsule& capsule)=0;

	/**
	\brief Test if box A is inside another box B.

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
	virtual bool NxIsBoxAInsideBoxB(const NxBox& a, const NxBox& b)=0;

	/**
	\brief Get a list of indices representing the box as quads.

	\return List of quad indices.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxComputeBoxPoints()
	*/
	virtual const NxU32* NxGetBoxQuads()=0;

	/**
	\brief Returns a list of quad indices sharing the vertex index.

	\param vertexIndex Vertex Index.
	\return List of quad indices sharing the vertex index.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxComputeBoxPoints() NxGetBoxQuads()
	*/
	virtual const NxU32* NxBoxVertexToQuad(NxU32 vertexIndex)=0;

	/**
	\brief Compute a box which encloses a capsule.

	\param capsule Capsule to generate an enclosing box for.
	\param box Generated box.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxComputeCapsuleAroundBox
	*/
	virtual void NxComputeBoxAroundCapsule(const NxCapsule& capsule, NxBox& box)=0;

	/**
	\brief Set FPU precision.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxSetFPUPrecision24()=0;

	/**
	\brief Set FPU precision.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxSetFPUPrecision53()=0;

	/**
	\brief Set FPU precision

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxSetFPUPrecision64()=0;

	/**
	\brief Set FPU precision.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxSetFPURoundingChop()=0;

	/**
	\brief Set FPU rounding mode.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxSetFPURoundingUp()=0;

	/**
	\brief Set FPU rounding mode.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxSetFPURoundingDown()=0;

	/**
	\brief Set FPU rounding mode.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxSetFPURoundingNear()=0;

	/**
	\brief Enable/Disable FPU exception.

	\param b True to enable exception.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxSetFPUExceptions(bool b)=0;

	/**
	\brief Convert a floating point number to an integer.

	\param f Floating point number.

	\return The result.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual int NxIntChop(const NxF32& f)=0;

	/**
	\brief Convert a floating point number to an integer.

	\param f Floating point number.

	\return The result.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual int NxIntFloor(const NxF32& f)=0;

	/**
	\brief Convert a floating point number to an integer.

	\param f Floating point number.

	\return The result.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual int NxIntCeil(const NxF32& f)=0;

	/**
	\brief Compute the distance squared from a point to a ray.

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
	virtual NxF32 NxComputeDistanceSquared(const NxRay& ray, const NxVec3& point, NxF32* t)=0;

	/**
	\brief Compute the distance squared from a point to a line segment.

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
	virtual NxF32 NxComputeSquareDistance(const NxSegment& seg, const NxVec3& point, NxF32* t)=0;

	/**
	\brief Compute a bounding sphere for a point cloud.

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
	virtual NxBSphereMethod NxComputeSphere(NxSphere& sphere, unsigned nb_verts, const NxVec3* verts)=0;
	/**
	\brief Compute a bounding sphere for a point cloud.

	The sphere may not be as tight as #NxComputeSphere

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
	virtual bool NxFastComputeSphere(NxSphere& sphere, unsigned nb_verts, const NxVec3* verts)=0;

	/**
	\brief Compute an overall bounding sphere for a pair of spheres.

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
	virtual void NxMergeSpheres(NxSphere& merged, const NxSphere& sphere0, const NxSphere& sphere1)=0;

	/**
	\brief Get the tangent vectors associated with a normal.

	\param n Normal vector
	\param t1 First tangent
	\param t2 Second tangent
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxNormalToTangents(const NxVec3 & n, NxVec3 & t1, NxVec3 & t2)=0;

	/**
	\brief Rotates a 3x3 symmetric inertia tensor I into a space R where it can be represented with the diagonal matrix D.

	I = R * D * R'

	Returns false on failure. 

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

	virtual bool NxDiagonalizeInertiaTensor(const NxMat33 & denseInertia, NxVec3 & diagonalInertia, NxMat33 & rotation)=0;

	/**
	\brief  Computes a rotation matrix.

	computes rotation matrix M so that:

	M * x = b

	x and b are unit vectors.

	\param x Vector.
	\param b Vector.
	\param M Computed rotation matrix.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxFindRotationMatrix(const NxVec3 & x, const NxVec3 & b, NxMat33 & M)=0;

	/**
	\brief Computes bounds of an array of vertices

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
	virtual void NxComputeBounds(NxVec3& min, NxVec3& max, NxU32 nbVerts, const NxVec3* verts)=0;


	/**
	\brief Computes CRC of input buffer

	\param buffer Input buffer.
	\param nbBytes Number of bytes in in the input buffer.
	\return The computed CRC.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32 NxCrc32(const void* buffer, NxU32 nbBytes)=0;
/*
** From NxInertiaTensor.h
*/
/***************************************************************************/
	/**
	\brief Computes mass of a homogeneous sphere according to sphere density.

	\param[in] radius Radius of the sphere. <b>Range:</b> (0,inf)
	\param[in] density Density of the sphere. <b>Range:</b> (0,inf)

	\return The mass of the sphere.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeSphereMass			(NxReal radius, NxReal density)=0;

	/**
	\brief Computes density of a homogeneous sphere according to sphere mass.

	\param[in] radius Radius of the sphere. <b>Range:</b> (0,inf)
	\param[in] mass Mass of the sphere. <b>Range:</b> (0,inf)

	\return The density of the sphere.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeSphereDensity		(NxReal radius, NxReal mass)=0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes mass of a homogeneous box according to box density.

	\param[in] extents The extents/radii, that is the full side length along each axis, of the box. <b>Range:</b> direction vector
	\param[in] density The density of the box. <b>Range:</b> (0,inf)

	\return The mass of the box.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeBoxMass			(const NxVec3& extents, NxReal density)=0;
	
	/**
	\brief Computes density of a homogeneous box according to box mass.

	\param[in] extents The extents/radii, that is the full side length along each axis, of the box. <b>Range:</b> direction vector
	\param[in] mass The mass of the box. <b>Range:</b> (0,inf)

	\return  The density of the box.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeBoxDensity			(const NxVec3& extents, NxReal mass)=0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes mass of a homogeneous ellipsoid according to ellipsoid density.

	\param[in] extents The extents/radii of the ellipsoid. <b>Range:</b> direction vector
	\param[in] density The density of the ellipsoid. <b>Range:</b> (0,inf)

	\return The mass of the ellipsoid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeEllipsoidMass		(const NxVec3& extents, NxReal density)=0;
	
	/**
	\brief Computes density of a homogeneous ellipsoid according to ellipsoid mass.

	\param[in] extents The extents/radii of the ellipsoid. <b>Range:</b> direction vector
	\param[in] mass The mass of the ellipsoid. <b>Range:</b> (0,inf)

	\return The density of the ellipsoid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeEllipsoidDensity	(const NxVec3& extents, NxReal mass)=0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes mass of a homogeneous cylinder according to cylinder density.

	\param[in] radius The radius of the cylinder. <b>Range:</b> (0,inf)
	\param[in] length The length. <b>Range:</b> (0,inf)
	\param[in] density The density. <b>Range:</b> (0,inf)

	\return The mass.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeCylinderMass		(NxReal radius, NxReal length, NxReal density)=0;
	
	/**
	\brief Computes density of a homogeneous cylinder according to cylinder mass.

	\param[in] radius The radius of the cylinder. <b>Range:</b> (0,inf)
	\param[in] length The length. <b>Range:</b> (0,inf)
	\param[in] mass The mass. <b>Range:</b> (0,inf)

	\return The density.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeCylinderDensity	(NxReal radius, NxReal length, NxReal mass)=0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes mass of a homogeneous cone according to cone density.

	\param[in] radius The radius of the cone. <b>Range:</b> (0,inf)
	\param[in] length The length. <b>Range:</b> (0,inf)
	\param[in] density The density. <b>Range:</b> (0,inf)

	\return The mass.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeConeMass			(NxReal radius, NxReal length, NxReal density)=0;
	
	/**
	\brief Computes density of a homogeneous cone according to cone mass.

	\param[in] radius The radius of the cone. <b>Range:</b> (0,inf)
	\param[in] length The length. <b>Range:</b> (0,inf)
	\param[in] mass The mass. <b>Range:</b> (0,inf)

	\return The density.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxReal NxComputeConeDensity		(NxReal radius, NxReal length, NxReal mass)=0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes diagonalized inertia tensor for a box.

	\param[out] diagInertia The diagonalized inertia tensor.
	\param[in] mass The mass of the box. <b>Range:</b> (0,inf)
	\param[in] xlength The width of the box. <b>Range:</b> (-inf,inf)
	\param[in] ylength The height. <b>Range:</b> (-inf,inf)
	\param[in] zlength The depth. <b>Range:</b> (-inf,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxComputeBoxInertiaTensor	(NxVec3& diagInertia, NxReal mass, NxReal xlength, NxReal ylength, NxReal zlength)=0;
	
	/**
	\brief Computes diagonalized inertia tensor for a sphere.

	\param[out] diagInertia The diagonalized inertia tensor.
	\param[in] mass The mass. <b>Range:</b> (0,inf)
	\param[in] radius The radius. <b>Range:</b> (-inf,inf)
	\param[in] hollow True to treat the sphere as a hollow shell. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void NxComputeSphereInertiaTensor(NxVec3& diagInertia, NxReal mass, NxReal radius, bool hollow)=0;

/*
** From NxJointDesc.h
*/
/*************************************************************/
	/**
	\brief Set the local anchor stored in a #NxJointDesc from a global anchor point.

	\param dis Joint desc to update.
	\param wsAnchor Anchor point in the global frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.setGlobalAnchor()
	*/
	virtual void NxJointDesc_SetGlobalAnchor(NxJointDesc & dis, const NxVec3 & wsAnchor)=0;

	/**
	\brief Set the local axis stored in a #NxJointDesc from a global axis.

	\param dis Joint desc to update.
	\param wsAxis Axis in the global frame. <b>Range:</b> direction vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.setGlobalAxis()
	*/
	virtual void NxJointDesc_SetGlobalAxis(NxJointDesc & dis, const NxVec3 & wsAxis)=0;


/*
** From NxIntersectionBoxBox.h
*/
/******************************************************************/

		/**
	\brief Boolean intersection test between two OBBs.
	
	Uses the separating axis theorem. Disabling 'full_test' only performs 6 axis tests out of 15.

	\param[in] extents0 Extents/radii of first box before transformation. <b>Range:</b> direction vector
	\param[in] center0 Center of first box. <b>Range:</b> position vector
	\param[in] rotation0 Rotation to apply to first box (before translation). <b>Range:</b> rotation matrix
	\param[in] extents1 Extents/radii of second box before transformation <b>Range:</b> direction vector
	\param[in] center1 Center of second box. <b>Range:</b> position vector
	\param[in] rotation1 Rotation to apply to second box(before translation). <b>Range:</b> rotation matrix
	\param[in] fullTest If false test only the first 6 axis.

	\return true on intersection

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxBoxBoxIntersect(	const NxVec3& extents0, const NxVec3& center0, const NxMat33& rotation0,
														const NxVec3& extents1, const NxVec3& center1, const NxMat33& rotation1,
														bool fullTest)=0;



	/*
	\brief Boolean intersection test between a triangle and an AABB.

	\param[in] vertex0 First vertex of triangle. <b>Range:</b> position vector
	\param[in] vertex1 Second Vertex of triangle. <b>Range:</b> position vector
	\param[in] vertex2 Third Vertex of triangle. <b>Range:</b> position vector
	\param[in] center Center of Axis Aligned bounding box. <b>Range:</b> position vector
	\param[in] extents Extents/radii of AABB. <b>Range:</b> direction vector

	\return true on intersection.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxTriBoxIntersect(const NxVec3 & vertex0, const NxVec3 & vertex1, const NxVec3 & vertex2, const NxVec3 & center, const NxVec3& extents)=0;

	/**
	\brief Computes the separating axis between two OBBs.

	\param[in] extents0 Extents/radii of first box before transformation. <b>Range:</b> direction vector
	\param[in] center0 Center of box first box. <b>Range:</b> position vector
	\param[in] rotation0 Rotation to apply to first box (before translation). <b>Range:</b> rotation matrix
	\param[in] extents1 Extents/radii of second box before transformation. <b>Range:</b> direction vector
	\param[in] center1 Center of second box. <b>Range:</b> position vector
	\param[in] rotation1 Rotation to apply to second box (before translation). <b>Range:</b> rotation matrix
	\param[in] fullTest If false test only the first 6 axis.

	\return The separating axis or NX_SEP_AXIS_OVERLAP for an overlap.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSepAxis
	*/
	virtual NxSepAxis NxSeparatingAxis(	const NxVec3& extents0, const NxVec3& center0, const NxMat33& rotation0,
															const NxVec3& extents1, const NxVec3& center1, const NxMat33& rotation1,
															bool fullTest=true)=0;

	/*
	** From NxIntersectionRayPlane.h
	*/
/*************************************************************************/

	/**
	\brief Segment-plane intersection test.
	
	Returns distance between v1 and impact point, as well as impact point on plane.

	\param[in] v1 First vertex of segment. <b>Range:</b> position vector
	\param[in] v2 Second vertex of segment. <b>Range:</b> position vector
	\param[in] plane Plane to test against. <b>Range:</b> See #NxPlane
	\param[out] dist Distance from v1 to impact point (so pointOnPlane=Normalize(v2-v1)*dist).
	\param[out] pointOnPlane Imapact point on plane.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void NxSegmentPlaneIntersect(const NxVec3& v1, const NxVec3& v2, 
		const NxPlane& plane, NxReal& dist, NxVec3& pointOnPlane)=0;

	/**
	\brief Ray-plane intersection test.
	
	Returns distance between ray origin and impact point, as well as impact point on plane.

	\param[in] ray Ray to test against plane. <b>Range:</b> See #NxRay
	\param[in] plane Plane to test. <b>Range:</b> See #NxPlane
	\param[out] dist Distance along ray to impact point (so pointOnPlane=Normalize(v2-v1)*dist).
	\param[out] pointOnPlane Impact point on the plane.

	\return True on intersection.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	bool NxRayPlaneIntersect(const NxRay& ray, const NxPlane& plane, 
		NxReal& dist, NxVec3& pointOnPlane)=0;

/*
** From NxIntersectionRaySphere.h
*/
/**************************************************************************/

	/**
	\brief Ray-sphere intersection test.
	
	Returns true if the ray intersects the sphere, and the impact point if needed.

	\param[in] origin Origin of the ray. <b>Range:</b> position vector
	\param[in] dir Direction of the ray. <b>Range:</b> direction vector
	\param[in] length Length of the ray. <b>Range:</b> (0,inf)
	\param[in] center Center of the sphere. <b>Range:</b> position vector
	\param[in] radius Sphere radius. <b>Range:</b> (0,inf)
	\param[out] hit_time Distance of intersection between ray and sphere.
	\param[out] hit_pos Point of intersection between ray and sphere.

	\return True on intersection.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxRaySphereIntersect(const NxVec3& origin, const NxVec3& dir, NxReal length, const NxVec3& center, NxReal radius, NxReal& hit_time, NxVec3& hit_pos) = 0;

/*
** From NxIntersectionSegmentBox
*/
/**************************************************************************/

	/**
	\brief Segment-AABB intersection test.
	
	Also computes intersection point.

	\param[in] p1 First point of line segment. <b>Range:</b> position vector
	\param[in] p2 Second point of line segment. <b>Range:</b> position vector
	\param[in] bbox_min Minimum extent of AABB. <b>Range:</b> position vector
	\param[in] bbox_max Max extent of AABB. <b>Range:</b> position vector
	\param[out] intercept Intersection point between segment and box.

	\return True if the segment and AABB intersect.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSegmentBoxIntersect(const NxVec3& p1, const NxVec3& p2,
		const NxVec3& bbox_min,const NxVec3& bbox_max, NxVec3& intercept)=0;

	/**
	\brief Ray-AABB intersection test.
	
	Also computes intersection point.

	\param[in] min Minimum extent of AABB. <b>Range:</b> position vector
	\param[in] max Maximum extent of AABB. <b>Range:</b> position vector
	\param[in] origin Origin of ray. <b>Range:</b> position vector
	\param[in] dir Direction of ray. <b>Range:</b> direction vector
	\param[out] coord Intersection point.

	\return True if the ray and AABB intersect.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxRayAABBIntersect(const NxVec3& min, const NxVec3& max, 
		const NxVec3& origin, const NxVec3& dir, NxVec3& coord)=0;

	/**
	\brief Extended Ray-AABB intersection test.
	
	Also computes intersection point, and parameter and returns contacted box axis index+1. Rays starting from inside the box are ignored.
	

	\param[in] min Minimum extent of AABB. <b>Range:</b> position vector
	\param[in] max Maximum extent of AABB. <b>Range:</b> position vector
	\param[in] origin Origin of ray. <b>Range:</b> position vector
	\param[in] dir Direction of ray. <b>Range:</b> direction vector
	\param[out] coord Intersection point.
	\param[out] t Ray parameter corresponding to contact point.

	\return Box axis index.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32 NxRayAABBIntersect2(const NxVec3& min, const NxVec3& max, 
		const NxVec3& origin, const NxVec3& dir, NxVec3& coord, NxReal & t)=0;

	/**
	\brief Boolean segment-OBB intersection test.
	
	Based on separating axis theorem.

	\param[in] p0 First point of line segment. <b>Range:</b> position vector
	\param[in] p1 Second point of line segment. <b>Range:</b> position vector
	\param[in] center Center point of OBB. <b>Range:</b> position vector
	\param[in] extents Extent/Radii of the OBB. <b>Range:</b> direction vector
	\param[in] rot Rotation of the OBB(applied before translation). <b>Range:</b> rotation matrix

	\return true if the segment and OBB intersect.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSegmentOBBIntersect(const NxVec3& p0, const NxVec3& p1, 
		const NxVec3& center, const NxVec3& extents, const NxMat33& rot)=0;

	/**
	\brief Boolean segment-AABB intersection test.
	
	Based on separating axis theorem.

	\param[in] p0 First point of line segment. <b>Range:</b> position vector
	\param[in] p1 Second point of line segment. <b>Range:</b> position vector
	\param[in] min Minimum extent of AABB. <b>Range:</b> position vector
	\param[in] max Maximum extent of AABB. <b>Range:</b> position vector

	\return True if the segment and AABB intersect.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSegmentAABBIntersect(const NxVec3& p0, const NxVec3& p1, 
		const NxVec3& min, const NxVec3& max)=0;

	/**
	\brief Boolean ray-OBB intersection test.
	
	Based on separating axis theorem.

	\param[in] ray Ray to test against OBB. <b>Range:</b> See #NxRay
	\param[in] center Center point of OBB. <b>Range:</b> position vector
	\param[in] extents Extent/Radii of the OBB. <b>Range:</b> direction vector
	\param[in] rot Rotation of the OBB(applied before translation). <b>Range:</b> rotation matrix

	\return True on intersection.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxRayOBBIntersect(const NxRay& ray, const NxVec3& center, 
		const NxVec3& extents, const NxMat33& rot)=0;

/*
** From NxIntersectionSegmentCapsule.h
*/
/*************************************************************************/

		/**
	\brief Ray-capsule intersection test.
	
	Returns number of intersection points (0,1 or 2) and corresponding parameters along the ray.

	\param[in] origin Origin of ray. <b>Range:</b> position vector
	\param[in] dir Direction of ray. <b>Range:</b> direction vector
	\param[in] capsule Capsule to test. <b>Range:</b> see #NxCapsule
	\param[out] t Parameter of intersection on the ray.

	The actual impact point is given by: 
	impact[i] = origin + t[i] * dir;

	\return Number of intersection points.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxRay NxCapsule
	*/
	virtual  NxU32 NxRayCapsuleIntersect(const NxVec3& origin, const NxVec3& dir, 
		const NxCapsule& capsule, NxReal t[2])=0;

/*
** From NxIntersectionSweptSpheres.h
*/
/***************************************************************************/

	/**
	\brief Sphere-sphere sweep test.
	
	Returns true if spheres intersect during their linear motion along provided velocity vectors.

	\param[in] sphere0 First sphere to test. <b>Range:</b> See #NxSphere
	\param[in] velocity0 Velocity of the first sphere(i.e. the vector to sweep the sphere along). <b>Range:</b> velocity/direction vector
	\param[in] sphere1 Second sphere to test <b>Range:</b> See #NxSphere
	\param[in] velocity1 Velocity of the second sphere(i.e. the vector to sweep the sphere along). <b>Range:</b> velocity/direction vector

	\return True if spheres intersect.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphere
	*/
	virtual bool NxSweptSpheresIntersect(	const NxSphere& sphere0, const NxVec3& velocity0,
																const NxSphere& sphere1, const NxVec3& velocity1)=0;

/*
** From NxRayTryIntersect.h
*/

	/**
	\brief Ray-triangle intersection test.
	
	Returns impact distance (t) as well as barycentric coordinates (u,v) of impact point.
	Use NxComputeBarycentricPoint() in Foundation to compute the impact point from the barycentric coordinates.
	The test performs back face culling or not according to 'cull'.

	\param[in] orig Origin of the ray. <b>Range:</b> position vector
	\param[in] dir Direction of the ray. <b>Range:</b> direction vector
	\param[in] vert0 First vertex of triangle. <b>Range:</b> position vector
	\param[in] vert1 Second vertex of triangle. <b>Range:</b> position vector
	\param[in] vert2 Third vertex of triangle. <b>Range:</b> position vector
	\param[out] t Distance along the ray from the origin to the impact point.
	\param[out] u Barycentric coordinate.
	\param[out] v Barycentric coordinate.
	\param[in] cull Cull backfaces.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxRayTriIntersect(const NxVec3& orig, const NxVec3& dir, const NxVec3& vert0, const NxVec3& vert1, const NxVec3& vert2, float& t, float& u, float& v, bool cull)=0;

	/***************************************************************/
/*
** From NxBuildSmoothNormals.h
*/
	/**
	\brief Builds smooth vertex normals over a mesh.

	- "smooth" because smoothing groups are not supported here
	- takes angles into account for correct cube normals computation

	To use 32bit indices pass a pointer in dFaces and set wFaces to zero. Alternatively pass a pointer to 
	wFaces and set dFaces to zero.

	\param[in] nbTris Number of triangles
	\param[in] nbVerts Number of vertices
	\param[in] verts Array of vertices
	\param[in] dFaces Array of dword triangle indices, or null
	\param[in] wFaces Array of word triangle indices, or null
	\param[out] normals Array of computed normals (assumes nbVerts vectors)
	\param[in] flip Flips the normals or not

	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxBuildSmoothNormals(
		NxU32 nbTris,
		NxU32 nbVerts,
		const NxVec3* verts,
		const NxU32* dFaces,
		const NxU16* wFaces,
		NxVec3* normals,
		bool flip=false
		)=0;




	/**
	\brief Box-vs-capsule sweep test.

	Sweeps a box against a capsule, returns true if box hit the capsule. Also returns contact information.

	\param[in] box Box (source of the sweep)
	\param[in] lss Capsule
	\param[in] dir Unit-length sweep direction
	\param[in] length Length of sweep (i.e. total motion vectoir is dir*length)
	\param[out] min_dist Impact distance
	\param[out] normal Normal at impact point
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSweepBoxCapsule(const NxBox& box, const NxCapsule& lss, const NxVec3& dir, float length, float& min_dist, NxVec3& normal) = 0;

	/**
	\brief Box-vs-sphere sweep test.

	Sweeps a box against a sphere, returns true if box hit the sphere. Also returns contact information.

	\param[in] box Box (source of the sweep)
	\param[in] sphere Sphere
	\param[in] dir Unit-length sweep direction
	\param[in] length Length of sweep (i.e. total motion vectoir is dir*length)
	\param[out] min_dist Impact distance
	\param[out] normal Normal at impact point
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSweepBoxSphere(const NxBox& box, const NxSphere& sphere, const NxVec3& dir, float length, float& min_dist, NxVec3& normal) = 0;

	/**
	\brief Capsule-vs-capsule sweep test.

	Sweeps a capsule against a capsule, returns true if capsule hit the other capsule. Also returns contact information.

	\param[in] lss0 Capsule (source of the sweep)
	\param[in] lss1 Capsule
	\param[in] dir Unit-length sweep direction
	\param[in] length Length of sweep (i.e. total motion vectoir is dir*length)
	\param[out] min_dist Impact distance
	\param[out] ip Impact point
	\param[out] normal Normal at impact point
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSweepCapsuleCapsule(const NxCapsule& lss0, const NxCapsule& lss1, const NxVec3& dir, float length, float& min_dist, NxVec3& ip, NxVec3& normal) = 0;

	/**
	\brief Sphere-vs-capsule sweep test.

	Sweeps a sphere against a capsule, returns true if sphere hit the capsule. Also returns contact information.

	\param[in] sphere Sphere (source of the sweep)
	\param[in] lss Capsule
	\param[in] dir Unit-length sweep direction
	\param[in] length Length of sweep (i.e. total motion vectoir is dir*length)
	\param[out] min_dist Impact distance
	\param[out] ip Impact point
	\param[out] normal Normal at impact point
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSweepSphereCapsule(const NxSphere& sphere, const NxCapsule& lss, const NxVec3& dir, float length, float& min_dist, NxVec3& ip, NxVec3& normal) = 0;

	/**
	\brief Box-vs-box sweep test.

	Sweeps a box against a box, returns true if box hit the other box. Also returns contact information.

	\param[in] box0 Box (source of the sweep)
	\param[in] box1 Box
	\param[in] dir Unit-length sweep direction
	\param[in] length Length of sweep (i.e. total motion vectoir is dir*length)
	\param[out] ip Impact point
	\param[out] normal Normal at impact point
	\param[out] min_dist Impact distance
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSweepBoxBox(const NxBox& box0, const NxBox& box1, const NxVec3& dir, float length, NxVec3& ip, NxVec3& normal, float& min_dist) = 0;

	/**
	\brief Box-vs-triangles sweep test.

	Sweeps a box against a set of triangles, returns true if box hit any triangle. Also returns contact information.

	\param[in] nb_tris Number of triangles
	\param[in] triangles Array of triangles
	\param[in] edge_triangles Array of edge-triangles, whose "vertices" are the edge normals
	\param[in] edge_flags Array of edge flags (NxTriangleCollisionFlag)
	\param[in] box Box (source of the sweep)
	\param[in] dir Unit-length sweep direction
	\param[in] length Length of sweep (i.e. total motion vectoir is dir*length)
	\param[out] hit Impact point
	\param[out] normal Normal at impact point
	\param[out] d Impact distance
	\param[out] index Triangle index (closest hit triangle)
	\param[in/out] cachedIndex Cached triangle index for subsequent calls. Cached triangle is tested first.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSweepBoxTriangles(NxU32 nb_tris, const NxTriangle* triangles, const NxTriangle* edge_triangles, const NxU32* edge_flags,
									const NxBounds3& box,
									const NxVec3& dir, float length,
									NxVec3& hit, NxVec3& normal, float& d, NxU32& index, NxU32* cachedIndex=NULL) = 0;

	/**
	\brief Capsule-vs-triangles sweep test.

	Sweeps a capsule against a set of triangles, returns true if capsule hit any triangle. Also returns contact information.

	\param[in] nb_tris Number of triangles
	\param[in] triangles Array of triangles
	\param[in] edge_flags Array of edge flags (NxTriangleCollisionFlag)
	\param[in] center Center of capsule
	\param[in] radius Capsule radius
	\param[in] height Capsule height
	\param[in] dir Unit-length sweep direction
	\param[in] length Length of sweep (i.e. total motion vectoir is dir*length)
	\param[out] hit Impact point
	\param[out] normal Normal at impact point
	\param[out] d Impact distance
	\param[out] index Triangle index (closest hit triangle)
	\param[in/out] cachedIndex Cached triangle index for subsequent calls. Cached triangle is tested first.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool NxSweepCapsuleTriangles(NxU32 up_direction,
									NxU32 nb_tris, const NxTriangle* triangles, const NxU32* edge_flags,
									const NxVec3& center, const float radius, const float height,
									const NxVec3& dir, float length,
									NxVec3& hit, NxVec3& normal, float& d, NxU32& index, NxU32* cachedIndex=NULL) = 0;

	/**
	\brief Point-vs-OBB distance computation.

	Returns distance between a point and an OBB.

	\param[in] point The point
	\param[in] center OBB center
	\param[in] extents OBB extents
	\param[in] rot OBB rotation
	\param[out] params Closest point on the box, in box space

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual float NxPointOBBSqrDist(const NxVec3& point, const NxVec3& center, const NxVec3& extents, const NxMat33& rot, NxVec3* params) = 0;

	/**
	\brief Segment-vs-OBB distance computation.

	Returns distance between a segment and an OBB.

	\param[in] segment The segment
	\param[in] c0 OBB center
	\param[in] e0 OBB extents
	\param[in] r0 OBB rotation
	\param[out] t Parameter in [0,1] describing the closest point on the segment.
	\param[out] params Closest point on the box, in box space

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual float NxSegmentOBBSqrDist(const NxSegment& segment, const NxVec3& c0, const NxVec3& e0, const NxMat33& r0, float* t, NxVec3* p) = 0;

	protected:
	virtual ~NxUtilLib(){};
	};
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
