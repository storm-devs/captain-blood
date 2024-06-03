#ifndef NX_FOUNDATION_NXSEGMENT
#define NX_FOUNDATION_NXSEGMENT
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

#include "Nxf.h"
#include "NxVec3.h"


class NxSegment;

/**
\brief Represents a line segment.

*/
class NxSegment
	{
	public:
	/**
	\brief Constructor
	*/
	NX_INLINE NxSegment()
		{
		}

	/**
	\brief Constructor
	*/
	NX_INLINE NxSegment(const NxVec3& _p0, const NxVec3& _p1) : p0(_p0), p1(_p1)
		{
		}

	/**
	\brief Copy constructor
	*/
	NX_INLINE NxSegment(const NxSegment& seg) : p0(seg.p0), p1(seg.p1)
		{
		}

	/**
	\brief Destructor
	*/
	NX_INLINE ~NxSegment()
		{
		}

	NX_INLINE const NxVec3& getOrigin() const
		{
		return p0;
		}

	NX_INLINE NxVec3 computeDirection() const
		{
		return p1 - p0;
		}

	NX_INLINE void computeDirection(NxVec3& dir) const
		{
		dir = p1 - p0;
		}

	NX_INLINE NxF32 computeLength() const
		{
		return p1.distance(p0);
		}

	NX_INLINE NxF32 computeSquareLength() const
		{
		return p1.distanceSquared(p0);
		}

	NX_INLINE void setOriginDirection(const NxVec3& origin, const NxVec3& direction)
		{
		p0 = p1 = origin;
		p1 += direction;
		}

	/**
	\brief Computes a point on the segment

	\param[out] pt point on segment
	\param[in] t point's parameter [t=0 => pt = mP0, t=1 => pt = mP1]
	 */
	NX_INLINE void computePoint(NxVec3& pt, NxF32 t) const
		{
		pt = p0 + t * (p1 - p0);
		}
#ifdef FOUNDATION_EXPORTS

	NX_INLINE NxF32 squareDistance(const NxVec3& point, NxF32* t=NULL) const
		{
		return NxComputeSquareDistance(*this, point, t);
		}

	NX_INLINE NxF32 distance(const NxVec3& point, NxF32* t=NULL) const
		{
		return sqrtf(squareDistance(point, t));
		}
#endif

	NxVec3	p0;		//!< Start of segment
	NxVec3	p1;		//!< End of segment
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
