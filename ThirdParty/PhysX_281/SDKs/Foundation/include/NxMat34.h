#ifndef NX_FOUNDATION_NxMat34T
#define NX_FOUNDATION_NxMat34T
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

#include "NxMat33.h"

/**
\brief Combination of a 3x3 rotation matrix and a translation vector.

homogenous transform class composed of a matrix and a vector.
*/

class NxMat34
	{
	public:
	/**
	\brief [ M t ]
	*/
	NxMat33 M;
	NxVec3 t;

	/**
	\brief by default M is inited and t isn't.  Use this ctor to either init or not init in full.
	*/
	NX_INLINE explicit NxMat34(bool init = true);

	NX_INLINE NxMat34(const NxMat33& rot, const NxVec3& trans) : M(rot), t(trans)
		{
		}

	NX_INLINE void zero();

	NX_INLINE void id();

	/**
	\brief returns true for identity matrix
	*/
	NX_INLINE bool isIdentity() const;

	/**
	\brief returns true if all elems are finite (not NAN or INF, etc.)
	*/
	NX_INLINE bool isFinite() const;

	/**
	\brief assigns inverse to dest. 
	
	Returns false if singular (i.e. if no inverse exists), setting dest to identity.  dest may equal this.
	*/
	NX_INLINE bool getInverse(NxMat34& dest) const;

	/**
	\brief same as #getInverse(), but assumes that M is orthonormal
	*/
	NX_INLINE bool getInverseRT(NxMat34& dest) const;

	/**
	\brief dst = this * src
	*/
	NX_INLINE void multiply(const NxVec3 &src, NxVec3 &dst) const;

	/**
	\brief operator wrapper for multiply
	*/
	NX_INLINE NxVec3 operator*  (const NxVec3 & src) const { NxVec3 dest; multiply(src, dest); return dest; }
	/**
	\brief dst = inverse(this) * src	-- assumes M is rotation matrix!!!
	*/
	NX_INLINE void multiplyByInverseRT(const NxVec3 &src, NxVec3 &dst) const;

	/**
	\brief operator wrapper for multiplyByInverseRT
	*/
	NX_INLINE NxVec3 operator%  (const NxVec3 & src) const { NxVec3 dest; multiplyByInverseRT(src, dest); return dest; }

	/**
	\brief this = left * right	
	*/
	NX_INLINE void multiply(const NxMat34& left, const NxMat34& right);

	/**
	\brief this = inverse(left) * right	-- assumes M is rotation matrix!!!
	*/
	NX_INLINE void multiplyInverseRTLeft(const NxMat34& left, const NxMat34& right);

	/**
	\brief this = left * inverse(right)	-- assumes M is rotation matrix!!!
	*/
	NX_INLINE void multiplyInverseRTRight(const NxMat34& left, const NxMat34& right);

	/**
	\brief operator wrapper for multiply
	*/
	NX_INLINE NxMat34 operator*  (const NxMat34 & right) const { NxMat34 dest(false); dest.multiply(*this, right); return dest; }

	/**
	\brief convert from a matrix format appropriate for rendering
	*/
	NX_INLINE void setColumnMajor44(const NxF32 *);
	/**
	\brief convert from a matrix format appropriate for rendering
	*/
	NX_INLINE void setColumnMajor44(const NxF32 d[4][4]);
	/**
	\brief convert to a matrix format appropriate for rendering
	*/
	NX_INLINE void getColumnMajor44(NxF32 *) const;
	/**
	\brief convert to a matrix format appropriate for rendering
	*/
	NX_INLINE void getColumnMajor44(NxF32 d[4][4]) const;
	/**
	\brief set the matrix given a row major matrix.
	*/
	NX_INLINE void setRowMajor44(const NxF32 *);
	/**
	\brief set the matrix given a row major matrix.
	*/
	NX_INLINE void setRowMajor44(const NxF32 d[4][4]);
	/**
	\brief retrieve the matrix in a row major format.
	*/
	NX_INLINE void getRowMajor44(NxF32 *) const;
	/**
	\brief retrieve the matrix in a row major format.
	*/
	NX_INLINE void getRowMajor44(NxF32 d[4][4]) const;
	};


NX_INLINE NxMat34::NxMat34(bool init)
	{
	if (init)
	{
		t.zero();
		M.id();
	}
	}


NX_INLINE void NxMat34::zero()
	{
	M.zero();
	t.zero();
	}


NX_INLINE void NxMat34::id()
	{
	M.id();
	t.zero();
	}


NX_INLINE bool NxMat34::isIdentity() const
	{
	if(!M.isIdentity())	return false;
	if(!t.isZero())		return false;
	return true;
	}


NX_INLINE bool NxMat34::isFinite() const
	{
	if(!M.isFinite())	return false;
	if(!t.isFinite())	return false;
	return true;
	}


NX_INLINE bool NxMat34::getInverse(NxMat34& dest) const
	{
	// inv(this) = [ inv(M) , inv(M) * -t ]
	bool status = M.getInverse(dest.M);
	dest.M.multiply(t * -1.0f, dest.t); 
	return status;
	}


NX_INLINE bool NxMat34::getInverseRT(NxMat34& dest) const
	{
	// inv(this) = [ M' , M' * -t ]
	dest.M.setTransposed(M);
	dest.M.multiply(t * -1.0f, dest.t); 
	return true;
	}



NX_INLINE void NxMat34::multiply(const NxVec3 &src, NxVec3 &dst) const
	{
	dst = M * src + t;
	}


NX_INLINE void NxMat34::multiplyByInverseRT(const NxVec3 &src, NxVec3 &dst) const
	{
	//dst = M' * src - M' * t = M' * (src - t)
	M.multiplyByTranspose(src - t, dst);
	}


NX_INLINE void NxMat34::multiply(const NxMat34& left, const NxMat34& right)
	{
	//[aR at] * [bR bt] = [aR * bR		aR * bt + at]  NOTE: order of operations important so it works when this ?= left ?= right.
	t = left.M * right.t + left.t;
	M.multiply(left.M, right.M);
	}


NX_INLINE void NxMat34::multiplyInverseRTLeft(const NxMat34& left, const NxMat34& right)
	{
	//[aR' -aR'*at] * [bR bt] = [aR' * bR		aR' * bt  - aR'*at]	//aR' ( bt  - at )	NOTE: order of operations important so it works when this ?= left ?= right.
	t = left.M % (right.t - left.t);
	M.multiplyTransposeLeft(left.M, right.M);
	}


NX_INLINE void NxMat34::multiplyInverseRTRight(const NxMat34& left, const NxMat34& right)
	{
	//[aR at] * [bR' -bR'*bt] = [aR * bR'		-aR * bR' * bt + at]	NOTE: order of operations important so it works when this ?= left ?= right.
	M.multiplyTransposeRight(left.M, right.M);
	t = left.t - M * right.t;
	}

NX_INLINE void NxMat34::setColumnMajor44(const NxF32 * d) 
	{
	M.setColumnMajorStride4(d);
    t.x = d[12];
	t.y = d[13];
	t.z = d[14];
	}

NX_INLINE void NxMat34::setColumnMajor44(const NxF32 d[4][4]) 
	{
	M.setColumnMajorStride4(d);
    t.x = d[3][0];
	t.y = d[3][1];
	t.z = d[3][2];
	}

NX_INLINE void NxMat34::getColumnMajor44(NxF32 * d) const
	{
	M.getColumnMajorStride4(d);
    d[12] = t.x;
	d[13] = t.y;
	d[14] = t.z;
	d[3] = d[7] = d[11] = 0.0f;
	d[15] = 1.0f;
	}

NX_INLINE void NxMat34::getColumnMajor44(NxF32 d[4][4]) const
	{
	M.getColumnMajorStride4(d);
    d[3][0] = t.x;
	d[3][1] = t.y;
	d[3][2] = t.z;
	d[0][3] = d[1][3] = d[2][3] = 0.0f;
	d[3][3] = 1.0f;
	}

NX_INLINE void NxMat34::setRowMajor44(const NxF32 * d) 
	{
	M.setRowMajorStride4(d);
    t.x = d[3];
	t.y = d[7];
	t.z = d[11];
	}

NX_INLINE void NxMat34::setRowMajor44(const NxF32 d[4][4])
	{
	M.setRowMajorStride4(d);
    t.x = d[0][3];
	t.y = d[1][3];
	t.z = d[2][3];
	}

NX_INLINE void NxMat34::getRowMajor44(NxF32 * d) const
	{
	M.getRowMajorStride4(d);
    d[3] = t.x;
	d[7] = t.y;
	d[11] = t.z;
	d[12] = d[13] = d[14] = 0.0f;
	d[15] = 1.0f;
	}

NX_INLINE void NxMat34::getRowMajor44(NxF32 d[4][4]) const
	{
	M.getRowMajorStride4(d);
    d[0][3] = t.x;
	d[1][3] = t.y;
	d[2][3] = t.z;
	d[3][0] = d[3][1] = d[3][2] = 0.0f;
	d[3][3] = 1.0f;
	}

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
