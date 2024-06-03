#ifndef NX_INTERSECTION_SEGMENT_BOX
#define NX_INTERSECTION_SEGMENT_BOX
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
#include "NxBox.h"
#include "PhysXLoader.h"

class NxRay;

//namespace NxCollision
//{
	/**
	\brief Segment-AABB intersection test.
	
	Also computes intersection point.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE bool NxSegmentBoxIntersect(const NxVec3& p1, const NxVec3& p2,
		const NxVec3& bbox_min,const NxVec3& bbox_max, NxVec3& intercept)
		{
		return NxGetUtilLib()->NxSegmentBoxIntersect(p1,p2,bbox_min,bbox_max,intercept);
		}

	/**
	\brief Ray-AABB intersection test.
	
	Also computes intersection point.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE bool NxRayAABBIntersect(const NxVec3& min, const NxVec3& max, 
		const NxVec3& origin, const NxVec3& dir, NxVec3& coord)
		{
		return NxGetUtilLib()->NxRayAABBIntersect(min,max,origin,dir,coord);
		}

	/**
	\brief Extended Ray-AABB intersection test.
	
	Also computes intersection point, and parameter and returns contacted box axis index+1. Rays starting from inside the box are ignored.
	
	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE NxU32 NxRayAABBIntersect2(const NxVec3& min, const NxVec3& max, 
		const NxVec3& origin, const NxVec3& dir, NxVec3& coord, NxReal & t)
		{
		return NxGetUtilLib()->NxRayAABBIntersect2(min,max,origin,dir,coord,t);
		}

	/**
	\brief Boolean segment-OBB intersection test.
	
	Based on separating axis theorem.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE bool NxSegmentOBBIntersect(const NxVec3& p0, const NxVec3& p1, 
		const NxVec3& center, const NxVec3& extents, const NxMat33& rot)
		{
		return NxGetUtilLib()->NxSegmentOBBIntersect(p0,p1,center,extents,rot);
		}

	/**
	\brief Boolean segment-AABB intersection test.
	
	Based on separating axis theorem.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE bool NxSegmentAABBIntersect(const NxVec3& p0, const NxVec3& p1, 
		const NxVec3& min, const NxVec3& max)
		{
		return NxGetUtilLib()->NxSegmentAABBIntersect(p0,p1,min,max);
		}

	/**
	\brief Boolean ray-OBB intersection test.
	
	Based on separating axis theorem.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE bool NxRayOBBIntersect(const NxRay& ray, const NxVec3& center, 
		const NxVec3& extents, const NxMat33& rot)
		{
		return NxGetUtilLib()->NxRayOBBIntersect(ray,center,extents,rot);
		}
//}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
