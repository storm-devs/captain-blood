#ifndef NX_FOUNDATION_NXI16VEC3
#define NX_FOUNDATION_NXI16VEC3
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
//Exclude file from docs
/** \cond */

#include "Nx.h"
#include "NxVec3.h"

class NxI16Vec3
{

public:

	NxI16 x;
	NxI16 y;
	NxI16 z;

	NX_INLINE NxI16Vec3()
	{
	}

	NX_INLINE NxI16Vec3(const NxI16Vec3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	NX_INLINE NxI16Vec3(NxI16 _x, NxI16 _y, NxI16 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	NX_INLINE bool operator==(const NxI16Vec3& v) const
	{
		return ((x == v.x) && (y == v.y) && (z == v.z));
	}

	NX_INLINE bool operator!=(const NxI16Vec3& v) const
	{
		return ((x != v.x) || (y != v.y) || (z != v.z));
	}

	NX_INLINE const NxI16Vec3 operator+(const NxI16Vec3& v)
	{
		NxI16Vec3 res;
		res.x = x + v.x;
		res.y = y + v.y;
		res.z = z + v.z;
		return res;
	}

	NX_INLINE const NxI16Vec3& operator+=(const NxI16Vec3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	NX_INLINE const NxI16Vec3& operator=(const NxI16Vec3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}
	
	NX_INLINE const NxI16Vec3 operator << ( const NxU32 shift) const
	{
		NxI16Vec3 res;
		res.x = x << shift;
		res.y = y << shift;
		res.z = z << shift;
		return res;
	}

	NX_INLINE const NxI16Vec3 operator >> ( const NxU32 shift) const
	{
		NxI16Vec3 res;
		res.x = x >> shift;
		res.y = y >> shift;
		res.z = z >> shift;
		return res;
	}

	NX_INLINE const NxI16Vec3& operator <<= ( const NxU32 shift)
	{
		x <<= shift;
		y <<= shift;
		z <<= shift;
		return *this;
	}

	NX_INLINE const NxI16Vec3& operator >>= ( const NxU32 shift)
	{
		x >>= shift;
		y >>= shift;
		z >>= shift;
		return *this;
	}

	NX_INLINE void set( const NxVec3& realVec, NxReal scale)
	{
		x = static_cast<NxI16>(NxMath::floor(realVec.x * scale));
		y = static_cast<NxI16>(NxMath::floor(realVec.y * scale));
		z = static_cast<NxI16>(NxMath::floor(realVec.z * scale));
	}

	NX_INLINE void set( const NxVec3& realVec)
	{
		x = static_cast<NxI16>(NxMath::floor(realVec.x));
		y = static_cast<NxI16>(NxMath::floor(realVec.y));
		z = static_cast<NxI16>(NxMath::floor(realVec.z));
	}

	NX_INLINE void set( NxI16 _x, NxI16 _y, NxI16 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	NX_INLINE void zero()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	NX_INLINE bool isZero()
	{
		return x == 0 && y == 0 && z == 0;
	}
};

/** \endcond */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND

