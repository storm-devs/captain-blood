#ifndef NX_FOUNDATION_NXSPHERE
#define NX_FOUNDATION_NXSPHERE
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

/**
\brief Enum to control the sphere generation method from a set of points.
*/
enum NxBSphereMethod
	{
		NX_BS_NONE,
		NX_BS_GEMS,
		NX_BS_MINIBALL,

		NX_BS_FORCE_DWORD	= 0x7fffffff
	};

/**
\brief Represents a sphere defined by its center point and radius.
*/
class NxSphere
	{
	public:
	/**
	\brief Constructor
	*/
	NX_INLINE NxSphere()
		{
		}

	/**
	\brief Constructor
	*/
	NX_INLINE NxSphere(const NxVec3& _center, NxF32 _radius) : center(_center), radius(_radius)
		{
		}
#ifdef FOUNDATION_EXPORTS
	/**
	\brief Constructor
	*/
	NX_INLINE NxSphere(unsigned nb_verts, const NxVec3* verts)
		{
		NxComputeSphere(*this, nb_verts, verts);
		}
#endif
	/**
	\brief Copy constructor
	*/
	NX_INLINE NxSphere(const NxSphere& sphere) : center(sphere.center), radius(sphere.radius)
		{
		}
#ifdef FOUNDATION_EXPORTS

	/**
	\brief Union of spheres
	*/
	NX_INLINE NxSphere(const NxSphere& sphere0, const NxSphere& sphere1)
		{
		NxMergeSpheres(*this, sphere0, sphere1);
		}
#endif
	/**
	\brief Destructor
	*/
	NX_INLINE ~NxSphere()
		{
		}
#ifdef FOUNDATION_EXPORTS

	NX_INLINE NxBSphereMethod compute(unsigned nb_verts, const NxVec3* verts)
		{
		return NxComputeSphere(*this, nb_verts, verts);
		}

	NX_INLINE bool fastCompute(unsigned nb_verts, const NxVec3* verts)
		{
		return NxFastComputeSphere(*this, nb_verts, verts);
		}
#endif
	/**
	\brief Checks the sphere is valid.

	\return		true if the sphere is valid
	*/
	NX_INLINE bool IsValid() const
		{
		// Consistency condition for spheres: Radius >= 0.0f
		return radius >= 0.0f;
		}

	/**
	\brief Tests if a point is contained within the sphere.

	\param[in] p the point to test
	\return	true if inside the sphere
	*/
	NX_INLINE bool Contains(const NxVec3& p) const
		{
		return center.distanceSquared(p) <= radius*radius;
		}

	/**
	\brief Tests if a sphere is contained within the sphere.

	\param		sphere	[in] the sphere to test
	\return		true if inside the sphere
	*/
	NX_INLINE bool Contains(const NxSphere& sphere)	const
		{
		// If our radius is the smallest, we can't possibly contain the other sphere
		if(radius < sphere.radius)	return false;
		// So r is always positive or null now
		float r = radius - sphere.radius;
		return center.distanceSquared(sphere.center) <= r*r;
		}

	/**
	\brief Tests if a box is contained within the sphere.

	\param		min		[in] min value of the box
	\param		max		[in] max value of the box
	\return		true if inside the sphere
	*/
	NX_INLINE bool Contains(const NxVec3& min, const NxVec3& max) const
		{
		// I assume if all 8 box vertices are inside the sphere, so does the whole box.
		// Sounds ok but maybe there's a better way?
		NxF32 R2 = radius * radius;
		NxVec3 p;
		p.x=max.x; p.y=max.y; p.z=max.z;	if(center.distanceSquared(p)>=R2)	return false;
		p.x=min.x;							if(center.distanceSquared(p)>=R2)	return false;
		p.x=max.x; p.y=min.y;				if(center.distanceSquared(p)>=R2)	return false;
		p.x=min.x;							if(center.distanceSquared(p)>=R2)	return false;
		p.x=max.x; p.y=max.y; p.z=min.z;	if(center.distanceSquared(p)>=R2)	return false;
		p.x=min.x;							if(center.distanceSquared(p)>=R2)	return false;
		p.x=max.x; p.y=min.y;				if(center.distanceSquared(p)>=R2)	return false;
		p.x=min.x;							if(center.distanceSquared(p)>=R2)	return false;

		return true;
		}

	/**
	\brief Tests if the sphere intersects another sphere

	\param		sphere	[in] the other sphere
	\return		true if spheres overlap
	*/
	NX_INLINE bool Intersect(const NxSphere& sphere) const
		{
		NxF32 r = radius + sphere.radius;
		return center.distanceSquared(sphere.center) <= r*r;
		}

	NxVec3	center;		//!< Sphere's center
	NxF32	radius;		//!< Sphere's radius
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
