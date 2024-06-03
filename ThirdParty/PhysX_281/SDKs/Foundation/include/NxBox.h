#ifndef NX_FOUNDATION_NXBOX
#define NX_FOUNDATION_NXBOX
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

#include "Nx.h"
#include "NxVec3.h"
#include "NxMat33.h"
#include "NxMat34.h"

class NxCapsule;
class NxPlane;
class NxBox;
class NxBounds3;


/**
\brief Represents an oriented bounding box. 

As a center point, extents(radii) and a rotation. i.e. the center of the box is at the center point, 
the box is rotated around this point with the rotation and it is 2*extents in width, height and depth.
*/
class NxBox
	{
	public:
	/**
	\brief Constructor
	*/
	NX_INLINE NxBox()
		{
		}

	/**
	\brief Constructor

	\param _center Center of the OBB
	\param _extents Extents/radii of the obb.
	\param _rot rotation to apply to the obb.
	*/
	NX_INLINE NxBox(const NxVec3& _center, const NxVec3& _extents, const NxMat33& _rot) : center(_center), extents(_extents), rot(_rot)
		{
		}

	/**
	\brief Destructor
	*/
	NX_INLINE ~NxBox()
		{
		}

	/**
	 \brief Setups an empty box.
	*/
	NX_INLINE void setEmpty()
		{
		center.zero();
		extents.set(NX_MIN_REAL, NX_MIN_REAL, NX_MIN_REAL);
		rot.id();
		}
#ifdef FOUNDATION_EXPORTS

	/**
	 \brief Tests if a point is contained within the box

	 See #NxBoxContainsPoint().

	 \param		p	[in] the world point to test
	 \return	true if inside the box
	*/
	NX_INLINE bool containsPoint(const NxVec3& p) const
		{
		return NxBoxContainsPoint(*this, p);
		}

	/**
	 \brief Builds a box from AABB and a world transform.

	 See #NxCreateBox().

	 \param		aabb	[in] the aabb
	 \param		mat		[in] the world transform
	*/
	NX_INLINE void create(const NxBounds3& aabb, const NxMat34& mat)
		{
		NxCreateBox(*this, aabb, mat);
		}
#endif
	/**
	 \brief Recomputes the box after an arbitrary transform by a 4x4 matrix.

	 \param		mtx		[in] the transform matrix
	 \param		obb		[out] the transformed OBB
	*/
	NX_INLINE void rotate(const NxMat34& mtx, NxBox& obb) const
		{		
		obb.extents = extents;// The extents remain constant
		obb.center = mtx.M * center + mtx.t;
		obb.rot.multiply(mtx.M, rot);
		}

	/**
	 \brief Checks the box is valid.

	 \return	true if the box is valid
	*/
	NX_INLINE bool isValid() const
		{
		// Consistency condition for (Center, Extents) boxes: Extents >= 0.0f
		if(extents.x < 0.0f)	return false;
		if(extents.y < 0.0f)	return false;
		if(extents.z < 0.0f)	return false;
		return true;
		}
#ifdef FOUNDATION_EXPORTS

	/**
	 \brief Computes the obb planes.

	 See #NxComputeBoxPlanes().

	 \param		planes	[out] 6 box planes
	 \return	true if success
	*/
	NX_INLINE bool computePlanes(NxPlane* planes) const
		{
		return NxComputeBoxPlanes(*this, planes);
		}

	/**
	 \brief Computes the obb points.

	 See #NxComputeBoxPoints().

	 \param		pts	[out] 8 box points
	 \return	true if success
	*/
	NX_INLINE bool computePoints(NxVec3* pts) const
		{
		return NxComputeBoxPoints(*this, pts);
		}

	/**
	 \brief Computes vertex normals.

	 See #NxComputeBoxVertexNormals().

	 \param		pts	[out] 8 box points
	 \return	true if success
	*/
	NX_INLINE bool computeVertexNormals(NxVec3* pts) const
		{
		return NxComputeBoxVertexNormals(*this, pts);
		}

	/**
	 \brief Returns edges.

	 See #NxGetBoxEdges().

	 \return	24 indices (12 edges) indexing the list returned by ComputePoints()
	*/
	NX_INLINE const NxU32* getEdges() const
		{
		return NxGetBoxEdges();
		}

		/**
		\brief Return edge axes indices.

		 See #NxgetBoxEdgeAxes().

		 \return Array of edge axes indices.
		*/

	NX_INLINE const NxI32* getEdgesAxes() const
		{
		return NxGetBoxEdgesAxes();
		}

	/**
	 \brief Returns triangles.

	 See #NxGetBoxTriangles().


	 \return 36 indices (12 triangles) indexing the list returned by ComputePoints()
	*/
	NX_INLINE const NxU32* getTriangles() const
		{
		return NxGetBoxTriangles();
		}

	/**
	 \brief Returns local edge normals.

	 See #NxGetBoxLocalEdgeNormals().

	 \return edge normals in local space
	*/
	NX_INLINE const NxVec3* getLocalEdgeNormals() const
		{
		return NxGetBoxLocalEdgeNormals();
		}

	/**
	 \brief Returns world edge normal

	 See #NxComputeBoxWorldEdgeNormal().

	 \param		edge_index		[in] 0 <= edge index < 12
	 \param		world_normal	[out] edge normal in world space
	*/
	NX_INLINE void computeWorldEdgeNormal(NxU32 edge_index, NxVec3& world_normal) const
		{
		NxComputeBoxWorldEdgeNormal(*this, edge_index, world_normal);
		}

	/**
	 \brief Computes a capsule surrounding the box.

	 See #NxComputeCapsuleAroundBox().

	 \param		capsule	[out] the capsule
	*/
	NX_INLINE void computeCapsule(NxCapsule& capsule) const
		{
		NxComputeCapsuleAroundBox(*this, capsule);
		}

	/**
	 \brief Checks the box is inside another box

	 See #NxIsBoxAInsideBoxB().

	 \param		box		[in] the other box
	 \return	TRUE if we're inside the other box
	*/
	NX_INLINE bool isInside(const NxBox& box) const
		{
		return NxIsBoxAInsideBoxB(*this, box);
		}
#endif
	// Accessors

		/**
		\brief Return center of box.

		\return Center of box.
		*/
	NX_INLINE const NxVec3& GetCenter() const
		{
		return center;
		}

		/**
		\brief Return extents(radii) of box.

		\return Extents of box.
		*/

	NX_INLINE const NxVec3& GetExtents() const
		{
		return extents;
		}

		/**
		\brief return box rotation.

		\return Box Rotation.
		*/

	NX_INLINE const NxMat33& GetRot() const
		{
		return rot;
		}

/*	NX_INLINE	void GetRotatedExtents(NxMat33& extents) const
		{
		extents = mRot;
		extents.Scale(mExtents);
		}*/

	NxVec3	center;
	NxVec3	extents;
	NxMat33	rot;
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
