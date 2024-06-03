#ifndef NX_INTERSECTION_POINT_TRIANGLE
#define NX_INTERSECTION_POINT_TRIANGLE
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

//namespace NxCollision {

	/**
	\brief Point-in-triangle test.
	
	We use the edges as parameters in case the user has access to edges directly
	This is actually a "point-in-prism" test since it returns true as long as the point is bound by the edge planes.

	\param[in] p Point to test. <b>Range:</b> position vector
	\param[in] p0 Triangle vertex. <b>Range:</b> position vector
	\param[in] edge10 Triangle edge. <b>Range:</b> direction vector
	\param[in] edge20 Second triangle edge. <b>Range:</b> direction vector
	
	\return True on intersection

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPointTriangleIntersect2D
	*/
	NX_INLINE NX_BOOL NxPointTriangleIntersect(const NxVec3& p, const NxVec3& p0, const NxVec3& edge10, const NxVec3& edge20)
		{ 
		NxReal a = edge10|edge10;
		NxReal b = edge10|edge20;
		NxReal c = edge20|edge20;
		NxReal ac_bb = (a*c)-(b*b);

		NxVec3 vp = p - p0;

		NxReal d = vp|edge10;
		NxReal e = vp|edge20;

		NxReal x = (d*c) - (e*b);
		NxReal y = (e*a) - (d*b);
		NxReal z = x + y - ac_bb;

		if ((x > 0.0f) && (y > 0.0f) && (z < 0.0f))
			return NX_TRUE;
		else
			return NX_FALSE;
		}

	/**
	\brief Dedicated 2D version of previous test

	\param[in] px Point to test, X
	\param[in] pz Point to test, Z
	\param[in] p0x Vertex of triangle to test, X
	\param[in] p0z Vertex of triangle to test, Z
	\param[in] e10x Edge of triangle to test, X
	\param[in] e10z Edge of triangle to test, Y
	\param[in] e20x Second edge of triangle to test, X
	\param[in] e20z Second edge of triangle to test, Y

	\return True on intersection

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPointTriangleIntersect
	*/
	NX_INLINE NX_BOOL NxPointTriangleIntersect2D(
		NxReal px, NxReal pz,
		NxReal p0x, NxReal p0z,
		NxReal e10x, NxReal e10z,
		NxReal e20x, NxReal e20z)
		{ 
		NxReal a = e10x*e10x + e10z*e10z;
		NxReal b = e10x*e20x + e10z*e20z;
		NxReal c = e20x*e20x + e20z*e20z;
		NxReal ac_bb = (a*c)-(b*b);

		NxReal vpx = px - p0x;
		NxReal vpz = pz - p0z;

		NxReal d = vpx*e10x + vpz*e10z;
		NxReal e = vpx*e20x + vpz*e20z;

		NxReal x = (d*c) - (e*b);
		NxReal y = (e*a) - (d*b);
		NxReal z = x + y - ac_bb;

		if ((x > 0.0f) && (y > 0.0f) && (z < 0.0f))
			return NX_TRUE;
		else
			return NX_FALSE;
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
