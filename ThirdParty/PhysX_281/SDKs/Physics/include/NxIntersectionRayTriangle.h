#ifndef NX_INTERSECTION_RAY_TRIANGLE
#define NX_INTERSECTION_RAY_TRIANGLE
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
	\brief Ray-triangle intersection test.
	
	Returns impact distance (t) as well as barycentric coordinates (u,v) of impact point.
	Use NxComputeBarycentricPoint() in Foundation to compute the impact point from the barycentric coordinates.
	The test performs back face culling or not according to 'cull'.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

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
	NX_INLINE bool NxRayTriIntersect(const NxVec3& orig, const NxVec3& dir, const NxVec3& vert0, const NxVec3& vert1, const NxVec3& vert2, float& t, float& u, float& v, bool cull)
		{
		return NxGetUtilLib()->NxRayTriIntersect(orig,dir,vert0,vert1,vert2,t,u,v,cull);
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
