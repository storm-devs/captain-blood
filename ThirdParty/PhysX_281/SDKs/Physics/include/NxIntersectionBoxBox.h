#ifndef NX_INTERSECTION_BOX_BOX
#define NX_INTERSECTION_BOX_BOX
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
	NX_INLINE bool NxBoxBoxIntersect(	const NxVec3& extents0, const NxVec3& center0, const NxMat33& rotation0,
														const NxVec3& extents1, const NxVec3& center1, const NxMat33& rotation1,
														bool fullTest)
		{
		return NxGetUtilLib()->NxBoxBoxIntersect(extents0,center0,rotation0,extents1,center1,rotation1,fullTest);
		}

	/**
	\brief Boolean intersection test between two OBBs.
	
	Uses the separating axis theorem. Disabling 'full_test' only performs 6 axis tests out of 15.

	\param[in] obb0 First Oriented Bounding Box. <b>Range:</b> See #NxBox
	\param[in] obb1 Second Oriented Bounding Box. <b>Range:</b> See #NxBox
	\param[in] fullTest If false test only the first 6 axis.

	\return true on intersection

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBox
	*/
	NX_INLINE bool NxBoxBoxIntersect(const NxBox& obb0, const NxBox& obb1, bool fullTest=true)
		{
		return NxBoxBoxIntersect(
			obb0.extents, obb0.center, obb0.rot,
			obb1.extents, obb1.center, obb1.rot,
			fullTest);
		}

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
	NX_INLINE bool NxTriBoxIntersect(const NxVec3 & vertex0, const NxVec3 & vertex1, const NxVec3 & vertex2, const NxVec3 & center, const NxVec3& extents)
		{
		return NxGetUtilLib()->NxTriBoxIntersect(vertex0,vertex1,vertex2,center,extents);

		}


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
	NX_INLINE NxSepAxis NxSeparatingAxis(	const NxVec3& extents0, const NxVec3& center0, const NxMat33& rotation0,
															const NxVec3& extents1, const NxVec3& center1, const NxMat33& rotation1,
															bool fullTest=true)
		{
		return NxGetUtilLib()->NxSeparatingAxis(extents0,center0,rotation0,extents1,center1,rotation1,fullTest);
		}

	/**
	\brief Computes the separating axis between two OBBs.

	\param[in] obb0 First Oriented Bounding box. <b>Range:</b> See #NxBox
	\param[in] obb1 Second Oriented Bounding box. <b>Range:</b> See #NxBox
	\param[in] fullTest If false test only the first 6 axis.

	\return The separating axis.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSepAxis
	*/
	NX_INLINE NxSepAxis NxSeparatingAxis(const NxBox& obb0, const NxBox& obb1, bool fullTest=true)
	{
		return NxSeparatingAxis(
			obb0.extents, obb0.center, obb0.rot,
			obb1.extents, obb1.center, obb1.rot,
			fullTest);
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
