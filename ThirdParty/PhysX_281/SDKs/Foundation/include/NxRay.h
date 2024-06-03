#ifndef NX_FOUNDATION_NXRAY
#define NX_FOUNDATION_NXRAY
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

class NxRay;

/**
\brief Represents an infinite ray as an origin and direction.

The direction should be normalized.
*/
class NxRay
	{
	public:
	/**
	Constructor
	*/
	NX_INLINE NxRay()
		{
		}

	/**
	Constructor
	*/
	NX_INLINE NxRay(const NxVec3& _orig, const NxVec3& _dir) : orig(_orig), dir(_dir)
		{
		}

	/**
	Destructor
	*/
	NX_INLINE ~NxRay()
		{
		}
#ifdef FOUNDATION_EXPORTS

	NX_INLINE NxF32 distanceSquared(const NxVec3& point, NxF32* t=NULL) const
		{
		return NxComputeDistanceSquared(*this, point, t);
		}

	NX_INLINE NxF32 distance(const NxVec3& point, NxF32* t=NULL) const
		{
		return sqrtf(distanceSquared(point, t));
		}
#endif

	NxVec3	orig;	//!< Ray origin
	NxVec3	dir;	//!< Normalized direction
	};

	NX_INLINE void ComputeReflexionVector(NxVec3& reflected, const NxVec3& incoming_dir, const NxVec3& outward_normal)
	{
		reflected = incoming_dir - outward_normal * 2.0f * incoming_dir.dot(outward_normal);
	}

	NX_INLINE void ComputeReflexionVector(NxVec3& reflected, const NxVec3& source, const NxVec3& impact, const NxVec3& normal)
	{
		NxVec3 V = impact - source;
		reflected = V - normal * 2.0f * V.dot(normal);
	}

	NX_INLINE void ComputeNormalCompo(NxVec3& normal_compo, const NxVec3& outward_dir, const NxVec3& outward_normal)
	{
		normal_compo = outward_normal * outward_dir.dot(outward_normal);
	}

	NX_INLINE void ComputeTangentCompo(NxVec3& outward_dir, const NxVec3& outward_normal)
	{
		outward_dir -= outward_normal * outward_dir.dot(outward_normal);
	}

	NX_INLINE void DecomposeVector(NxVec3& normal_compo, NxVec3& tangent_compo, const NxVec3& outward_dir, const NxVec3& outward_normal)
	{
		normal_compo = outward_normal * outward_dir.dot(outward_normal);
		tangent_compo = outward_dir - normal_compo;
	}

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
