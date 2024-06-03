#ifndef NX_FOUNDATION_NXCAPSULE
#define NX_FOUNDATION_NXCAPSULE
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
#include "NxSegment.h"
#include "NxSphere.h"

class NxCapsule;
class NxBox;

/**
\brief Represents a capsule.
*/
class NxCapsule : public NxSegment
	{
	public:
	/**
	\brief Constructor
	*/
	NX_INLINE NxCapsule()
		{
		}

	/**
	\brief Constructor
	
	\param seg Line segment to create capsule from.
	\param _radius Radius of the capsule.
	*/
	NX_INLINE NxCapsule(const NxSegment& seg, NxF32 _radius) : NxSegment(seg), radius(_radius)
		{
		}

	/**
	\brief Destructor
	*/
	NX_INLINE ~NxCapsule()
		{
		}

#ifdef FOUNDATION_EXPORTS

	/**
	 Computes an OBB surrounding the capsule.
	 \param		box		[out] the OBB
	 */
	NX_INLINE void computeOBB(NxBox& box) const
		{
		NxComputeBoxAroundCapsule(*this, box);
		}
	/**
	 Tests if a point is contained within the capsule.
	 \param		pt	[in] the point to test
	 \return	true if inside the capsule
	 \warning	point and capsule must be in same space
	 */
	NX_INLINE bool contains(const NxVec3& pt) const
		{
		return squareDistance(pt) <= radius*radius;
		}

	/**
	 Tests if a sphere is contained within the capsule.
	 \param		sphere	[in] the sphere to test
	 \return	true if inside the capsule
	 \warning	sphere and capsule must be in same space
	 */
	NX_INLINE bool contains(const NxSphere& sphere) const
		{
		NxF32 d = radius - sphere.radius;
		if(d>=0.0f)	return squareDistance(sphere.center) <= d*d;
		else		return false;
		}

	/**
	 Tests if a capsule is contained within the capsule.
	 \param		capsule	[in] the capsule to test
	 \return	true if inside the capsule
	 \warning	both capsule must be in same space
	 */
	NX_INLINE bool contains(const NxCapsule& capsule) const
		{
		// We check the capsule contains the two spheres at the start and end of the sweep
		return contains(NxSphere(capsule.p0, capsule.radius)) && contains(NxSphere(capsule.p1, capsule.radius));
		}
#endif

	NxF32	radius;
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
