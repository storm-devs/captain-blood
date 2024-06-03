#ifndef NX_INTERSECTION_RAY_PLANE
#define NX_INTERSECTION_RAY_PLANE
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
class NxRay;
class NxPlane;

	/**
	\brief Segment-plane intersection test.
	
	Returns distance between v1 and impact point, as well as impact point on plane.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] v1 First vertex of segment. <b>Range:</b> position vector
	\param[in] v2 Second vertex of segment. <b>Range:</b> position vector
	\param[in] plane Plane to test against. <b>Range:</b> See #NxPlane
	\param[out] dist Distance from v1 to impact point (so pointOnPlane=Normalize(v2-v1)*dist).
	\param[out] pointOnPlane Imapct point on plane.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE	void NxSegmentPlaneIntersect(const NxVec3& v1, const NxVec3& v2, 
		const NxPlane& plane, NxReal& dist, NxVec3& pointOnPlane)
		{
		return NxGetUtilLib()->NxSegmentPlaneIntersect(v1,v2,plane,dist,pointOnPlane);
		}

	/**
	\brief Ray-plane intersection test.
	
	Returns distance between ray origin and impact point, as well as impact point on plane.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE	bool NxRayPlaneIntersect(const NxRay& ray, const NxPlane& plane, 
		NxReal& dist, NxVec3& pointOnPlane)
		{
		return NxGetUtilLib()->NxRayPlaneIntersect(ray,plane,dist,pointOnPlane);
		}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
