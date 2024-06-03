#ifndef NX_FOUNDATION_NXPLANE
#define NX_FOUNDATION_NXPLANE
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
#include "NxMat34.h"

/**
\brief Representation of a plane.

 Plane equation used: a*x + b*y + c*z + d = 0
*/
class NxPlane
	{
	public:
	/**
	\brief Constructor
	*/
	NX_INLINE NxPlane()
		{
		}

	/**
	\brief Constructor from a normal and a distance
	*/
	NX_INLINE NxPlane(NxF32 nx, NxF32 ny, NxF32 nz, NxF32 _d)
		{
		set(nx, ny, nz, _d);
		}

	/**
	\brief Constructor from a point on the plane and a normal
	*/
	NX_INLINE NxPlane(const NxVec3& p, const NxVec3& n)
		{
		set(p, n);
		}

	/**
	\brief Constructor from three points
	*/
	NX_INLINE NxPlane(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2)
		{
		set(p0, p1, p2);
		}

	/**
	\brief Constructor from a normal and a distance
	*/
	NX_INLINE NxPlane(const NxVec3& _n, NxF32 _d) : normal(_n), d(_d)
		{
		}

	/**
	\brief Copy constructor
	*/
	NX_INLINE NxPlane(const NxPlane& plane) : normal(plane.normal), d(plane.d)
		{
		}

	/**
	\brief Destructor
	*/
	NX_INLINE ~NxPlane()
		{
		}

	/**
	\brief Sets plane to zero.
	*/
	NX_INLINE NxPlane& zero()
		{
		normal.zero();
		d = 0.0f;
		return *this;
		}

	NX_INLINE NxPlane& set(NxF32 nx, NxF32 ny, NxF32 nz, NxF32 _d)
		{
		normal.set(nx, ny, nz);
		d = _d;
		return *this;
		}

	NX_INLINE NxPlane& set(const NxVec3& _normal, NxF32 _d)
		{
		normal = _normal;
		d = _d;
		return *this;
		}

	NX_INLINE NxPlane& set(const NxVec3& p, const NxVec3& _n)
		{
		normal = _n;
		// Plane equation: a*x + b*y + c*z + d = 0
		// p belongs to plane so:
		//     a*p.x + b*p.y + c*p.z + d = 0
		// <=> (n|p) + d = 0
		// <=> d = - (n|p)
		d = - p.dot(_n);
		return *this;
		}

	/**
	 \brief Computes the plane equation from 3 points.
	 */
	NxPlane& set(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2)
		{
		NxVec3 Edge0 = p1 - p0;
		NxVec3 Edge1 = p2 - p0;

		normal = Edge0.cross(Edge1);
		normal.normalize();

		// See comments in set() for computation of d
		d = -p0.dot(normal);

		return	*this;
		}

	NX_INLINE NxF32 distance(const NxVec3& p) const
		{
		// Valid for plane equation a*x + b*y + c*z + d = 0
		return p.dot(normal) + d;
		}

	NX_INLINE bool belongs(const NxVec3& p) const
		{
		return fabsf(distance(p)) < (1.0e-7f);
		}

	/**
	\brief projects p into the plane
	*/
	NX_INLINE NxVec3 project(const NxVec3 & p) const
		{
		// Pretend p is on positive side of plane, i.e. plane.distance(p)>0.
		// To project the point we have to go in a direction opposed to plane's normal, i.e.:
		return p - normal * distance(p);
//		return p + normal * distance(p);
		}

	/**
	\brief find an arbitrary point in the plane
	*/
	NX_INLINE NxVec3 pointInPlane() const
		{
		// Project origin (0,0,0) to plane:
		// (0) - normal * distance(0) = - normal * ((p|(0)) + d) = -normal*d
		return - normal * d;
//		return normal * d;
		}

	NX_INLINE void normalize()
		{
			NxF32 Denom = 1.0f / normal.magnitude();
			normal.x	*= Denom;
			normal.y	*= Denom;
			normal.z	*= Denom;
			d			*= Denom;
		}

	NX_INLINE operator NxVec3() const
		{
		return normal;
		}

	NX_INLINE void transform(const NxMat34 & transform, NxPlane & transformed) const
		{
		transformed.normal = transform.M * normal;
		transformed.d = d - (transform.t | transformed.normal);
		}

	NX_INLINE void inverseTransform(const NxMat34 & transform, NxPlane & transformed) const
		{
		transformed.normal = transform.M % normal;
		NxVec3 it = transform.M %  transform.t;
		transformed.d = d + (it | transformed.normal);
		}

	NxVec3	normal;		//!< The normal to the plane
	NxF32	d;			//!< The distance from the origin
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
