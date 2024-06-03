#ifndef NX_FOUNDATION_NXBOUNDS3
#define NX_FOUNDATION_NXBOUNDS3
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
#include "NxMat33.h"

/**
	\brief Class representing 3D range or axis aligned bounding box.

	Stored as minimum and maximum extent corners. Alternate representation
	would be center and dimensions.
	May be empty or nonempty. If not empty, min <= max has to hold.
*/
class NxBounds3
	{
	public:
	NX_INLINE NxBounds3();
	NX_INLINE ~NxBounds3();

	/**
	\brief Sets empty to true
	*/
	NX_INLINE void setEmpty();

	/**
	\brief Sets infinite bounds
	*/
	NX_INLINE void setInfinite();
	
	/**
	\brief low level assignment.

	\param minx Minimum X value
	\param miny Minimum Y value
	\param minz Minimum Z value
	\param maxx Maximum X value
	\param maxy Maximum Y value
	\param maxz Maximum Z value
	*/
	NX_INLINE void set(NxReal minx, NxReal miny, NxReal minz, NxReal maxx, NxReal maxy,NxReal maxz);

	/**
	\brief vector assignment.

	\param min Minimum point of bounds.
	\param max Maximum point of bounds.
	*/
	NX_INLINE void set(const NxVec3& min, const NxVec3& max);

	/**
	\brief expands the volume to include v

	\param v Point to expand to.
	*/
	NX_INLINE void include(const NxVec3& v);

	/**
	\brief sets this to the union of this and b2.

	\param b2 Bounds to perform union with.
	*/
	NX_INLINE void combine(const NxBounds3& b2);

	/**
	\brief sets this to the AABB of the OBB passed.

	\param orientation Orientation of the OBB.
	\param translation Translation of the OBB.
	\param halfDims radii of the OBB.
	*/
	NX_INLINE void boundsOfOBB(const NxMat33& orientation, const NxVec3& translation, const NxVec3& halfDims);

	/**
	\brief transforms this volume as if it was an axis aligned bounding box, and then assigns the results' bounds to this.

	\param orientation Orientation to apply.
	\param translation Translation to apply(applied after orientation transform)
	*/
	NX_INLINE void transform(const NxMat33& orientation, const NxVec3& translation);
	
	NX_INLINE bool isEmpty() const;

	/**
	\brief indicates whether the intersection of this and b is empty or not.

	\param b Bounds to test for intersection.
	*/
	NX_INLINE bool intersects(const NxBounds3& b) const;

	/**
	\brief indicates whether the intersection of this and b is empty or not in the plane orthogonal to the axis passed (X = 0, Y = 1 or Z = 2).

	\param b Bounds to test for intersection.
	\param axisToIgnore Axis to ignore when performing the intersection test.
	*/
	NX_INLINE bool intersects2D(const NxBounds3& b, unsigned axisToIgnore) const;

	/**
	\brief indicates if these bounds contain v.

	\param v Point to test against bounds.
	*/
	NX_INLINE bool contain(const NxVec3& v) const;

	/**
	\brief returns the center of this axis aligned box.

	\param center The center of the bounds.
	*/
	NX_INLINE void getCenter(NxVec3& center) const;

	/**
	\brief returns the dimensions (width/height/depth) of this axis aligned box.

	\param dims The dimensions of the bounds.
	*/
	NX_INLINE void getDimensions(NxVec3& dims) const;

	/**
	\brief returns the extents, which are half of the width/height/depth.

	\param extents The extents/radii of the bounds.
	*/
	NX_INLINE void getExtents(NxVec3& extents) const;

	/**
	\brief setups an AABB from center & extents vectors.

	\param c Center vector
	\param e Extents vector
	*/
	NX_INLINE void setCenterExtents(const NxVec3& c, const NxVec3& e);

	/**
	\brief scales the AABB.

	\param scale Factor to scale AABB by.
	*/
	NX_INLINE void scale(NxF32 scale);

	/** 
	fattens the AABB in all 3 dimensions by the given distance. 
	*/
	NX_INLINE void fatten(NxReal distance);


	//NX_INLINE void combine(NxReal extension);

	NxVec3 min, max;
	};


NX_INLINE NxBounds3::NxBounds3()
	{
	// Default to empty boxes for compatibility TODO: PT: remove this if useless
	setEmpty();
	}


NX_INLINE NxBounds3::~NxBounds3()
	{
	//nothing
	}


NX_INLINE void NxBounds3::setEmpty()
	{
	// We know use this particular pattern for empty boxes
	set(NX_MAX_REAL, NX_MAX_REAL, NX_MAX_REAL,
		NX_MIN_REAL, NX_MIN_REAL, NX_MIN_REAL);
	}

NX_INLINE void NxBounds3::setInfinite()
	{
	set(NX_MIN_REAL, NX_MIN_REAL, NX_MIN_REAL,
		NX_MAX_REAL, NX_MAX_REAL, NX_MAX_REAL);
	}

NX_INLINE void NxBounds3::set(NxReal minx, NxReal miny, NxReal minz, NxReal maxx, NxReal maxy,NxReal maxz)
	{
	min.set(minx, miny, minz);
	max.set(maxx, maxy, maxz);
	}

NX_INLINE void NxBounds3::set(const NxVec3& _min, const NxVec3& _max)
	{
	min = _min;
	max = _max;
	}

NX_INLINE void NxBounds3::include(const NxVec3& v)
	{
	max.max(v);
	min.min(v);
	}

NX_INLINE void NxBounds3::combine(const NxBounds3& b2)
	{
	// - if we're empty, min = MAX,MAX,MAX => min will be b2 in all cases => it will copy b2, ok
	// - if b2 is empty, the opposite happens => keep us unchanged => ok
	// => same behavior as before, automatically
	min.min(b2.min);
	max.max(b2.max);
	}

NX_INLINE void NxBounds3::boundsOfOBB(const NxMat33& orientation, const NxVec3& translation, const NxVec3& halfDims)
	{
	NxReal dimx = halfDims[0];
	NxReal dimy = halfDims[1];
	NxReal dimz = halfDims[2];

	NxReal x = NxMath::abs(orientation(0,0) * dimx) + NxMath::abs(orientation(0,1) * dimy) + NxMath::abs(orientation(0,2) * dimz);
	NxReal y = NxMath::abs(orientation(1,0) * dimx) + NxMath::abs(orientation(1,1) * dimy) + NxMath::abs(orientation(1,2) * dimz);
	NxReal z = NxMath::abs(orientation(2,0) * dimx) + NxMath::abs(orientation(2,1) * dimy) + NxMath::abs(orientation(2,2) * dimz);

	set(-x + translation[0], -y + translation[1], -z + translation[2], x + translation[0], y + translation[1], z + translation[2]);
	}

NX_INLINE void NxBounds3::transform(const NxMat33& orientation, const NxVec3& translation)
	{
	// convert to center and extents form
	NxVec3 center, extents;
	getCenter(center);
	getExtents(extents);

	center = orientation * center + translation;
	boundsOfOBB(orientation, center, extents);
	}

NX_INLINE bool NxBounds3::isEmpty() const
	{
	// Consistency condition for (Min, Max) boxes: min < max
	// TODO: PT: should we test against the explicit pattern ?
	if(min.x < max.x)	return false;
	if(min.y < max.y)	return false;
	if(min.z < max.z)	return false;
	return true;
	}

NX_INLINE bool NxBounds3::intersects(const NxBounds3& b) const
	{
	if ((b.min.x > max.x) || (min.x > b.max.x)) return false;
	if ((b.min.y > max.y) || (min.y > b.max.y)) return false;
	if ((b.min.z > max.z) || (min.z > b.max.z)) return false;
	return true;
	}

NX_INLINE bool NxBounds3::intersects2D(const NxBounds3& b, unsigned axis) const
	{
	// TODO: PT: could be static and like this:
	// static unsigned i[3] = { 1,2,0,1 };
	// const unsigned ii = i[axis];
	// const unsigned jj = i[axis+1];
	const unsigned i[3] = { 1,0,0 };
	const unsigned j[3] = { 2,2,1 };
	const unsigned ii = i[axis];
	const unsigned jj = j[axis];
	if ((b.min[ii] > max[ii]) || (min[ii] > b.max[ii])) return false;
	if ((b.min[jj] > max[jj]) || (min[jj] > b.max[jj])) return false;
	return true;
	}

NX_INLINE bool NxBounds3::contain(const NxVec3& v) const
	{
	if ((v.x < min.x) || (v.x > max.x)) return false;
	if ((v.y < min.y) || (v.y > max.y)) return false;
	if ((v.z < min.z) || (v.z > max.z)) return false;
	return true;
	}

NX_INLINE void NxBounds3::getCenter(NxVec3& center) const
	{
	center.add(min,max);
	center *= NxReal(0.5);
	}

NX_INLINE void NxBounds3::getDimensions(NxVec3& dims) const
	{
	dims.subtract(max,min);
	}

NX_INLINE void NxBounds3::getExtents(NxVec3& extents) const
	{
	extents.subtract(max,min);
	extents *= NxReal(0.5);
	}

NX_INLINE void NxBounds3::setCenterExtents(const NxVec3& c, const NxVec3& e)
	{
	min = c - e;
	max = c + e;
	}

NX_INLINE void NxBounds3::scale(NxF32 scale)
	{
	NxVec3 center, extents;
	getCenter(center);
	getExtents(extents);
	setCenterExtents(center, extents * scale);
	}

NX_INLINE void NxBounds3::fatten(NxReal distance)
	{
	min.x -= distance;
	min.y -= distance;
	min.z -= distance;

	max.x += distance;
	max.y += distance;
	max.z += distance;
	}

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
