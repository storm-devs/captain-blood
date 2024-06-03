#ifndef NX_INTERSECTION_SEGMENT_CAPSULE
#define NX_INTERSECTION_SEGMENT_CAPSULE
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
#include "NxCapsule.h"
#include "NxSegment.h"
#include "PhysXLoader.h"

//namespace NxCollision {

	/**
	\brief Ray-capsule intersection test.
	
	Returns number of intersection points (0,1 or 2) and corresponding parameters along the ray.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] origin Origin of ray. <b>Range:</b> position vector
	\param[in] dir Direction of ray. <b>Range:</b> direction vector
	\param[in] capsule Capsule to test. <b>Range:</b> see #NxCapsule
	\param[out] t Parameter of intersection on the ray.

	\return Number of intersection points.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxRay NxCapsule
	*/
	NX_INLINE NxU32 NxRayCapsuleIntersect(const NxVec3& origin, const NxVec3& dir, 
		const NxCapsule& capsule, NxReal t[2])
		{
		return NxGetUtilLib()->NxRayCapsuleIntersect(origin,dir,capsule,t);
		}

	/**
	\brief Segment-capsule intersection test.
	
	Returns number of intersection points (0,1 or 2) and corresponding parameters along the ray.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] segment Line segment. <b>Range:</b> See #NxSegment
	\param[in] capsule Capsule to test. <b>Range:</b> See #NxCapsule
	\param[out] nbImpacts Number of intersection point (0, 1 or 2)
	\param[out] t Parameter of intersection on the ray.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSegement NxCapsule
	*/
	NX_INLINE void NxSegmentCapsuleIntersect(const NxSegment& segment, const NxCapsule& capsule, NxU32* nbImpacts, 
		NxReal t[2])
		{
		NxReal s[2];
		NxU32 numISec = NxRayCapsuleIntersect(segment.p0, segment.computeDirection(), capsule,s);

		NxU32 numClip = 0;
		for(NxU32 i = 0; i < numISec; i++)
			{
			if ( 0.0f <= s[i] && s[i] <= 1.0f ) t[numClip++] = s[i];
			}

		*nbImpacts = numClip;
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
