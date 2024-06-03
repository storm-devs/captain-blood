#ifndef NX_INTERSECTION_RAY_SPHERE
#define NX_INTERSECTION_RAY_SPHERE
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

//namespace NxCollision
//{
	/**
	\brief Ray-sphere intersection test.
	
	Returns true if the ray intersects the sphere, and the impact point if needed.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE bool NxRaySphereIntersect(const NxVec3& origin, const NxVec3& dir, NxReal length, const NxVec3& center, NxReal radius, NxReal& hit_time, NxVec3& hit_pos)
		{
		return NxGetUtilLib()->NxRaySphereIntersect(origin, dir, length, center, radius, hit_time, hit_pos);
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
