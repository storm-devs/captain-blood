#ifndef NX_PHYSICS_NXSWTARGET
#define NX_PHYSICS_NXSWTARGET

#include "Nxp.h"

namespace NxForceFieldInternals 
{

class NxSwFloat;
class NxSwVec;

template <typename T = void> 
struct NxForceFieldEpsHolder
{
	static float epsilon;
};

template <typename T>float NxForceFieldEpsHolder<T>::epsilon = 1e-3f;

class NxSwBool
{
	friend class NxSw;
	friend class NxSwBoolVar;
	friend class NxSwVec;
	friend NxSwBool operator &(const NxSwBool &b1, const NxSwBool &b2);
	friend NxSwBool operator |(const NxSwBool &b1, const NxSwBool &b2);
	friend NxSwBool operator ^(const NxSwBool &b1, const NxSwBool &b2);
private:
protected:
	bool val;
public:
	NX_INLINE NxSwBool()									{}
	NX_INLINE NxSwBool(bool t)								{	val = t;						}
	NX_INLINE NxSwBool(int t)								{	val = !!t;						}	// For "Boolean b = true ^ true;"

	NX_INLINE NxSwFloat select(NxSwFloat f0, NxSwFloat f1);
	NX_INLINE NxSwVec	select(NxSwVec& v0, NxSwVec& v1);
};

NX_INLINE NxSwBool operator &(const NxSwBool &b1, const NxSwBool &b2)
{
	return NxSwBool(b1.val && b2.val);
}
NX_INLINE NxSwBool operator |(const NxSwBool &b1, const NxSwBool &b2)
{
	return NxSwBool(b1.val || b2.val);
}
NX_INLINE NxSwBool operator ^(const NxSwBool &b1, const NxSwBool &b2)
{
	return NxSwBool(!b1.val && b2.val||b1.val && !b2.val);
}

class NxSwBoolVar: public NxSwBool
{
public:
	NX_INLINE NxSwBoolVar(const NxSwBool& b)				{	val = b.val; }
	NX_INLINE void operator=(const NxSwBool& b)				{	val = b.val; }
};

class NxSwFloat
{
	friend class NxSw;
	friend class NxSwVec;
	friend class NxSwFloatVar;
	friend class NxSwVecVar;
	friend NxSwFloat operator +(const NxSwFloat &t1, const NxSwFloat &t2);
	friend NxSwFloat operator -(const NxSwFloat &t1, const NxSwFloat &t2);
	friend NxSwFloat operator *(const NxSwFloat &t1, const NxSwFloat &t2);
	friend NxSwBool operator<(const NxSwFloat &t1, const NxSwFloat &t2);
	friend NxSwBool operator>(const NxSwFloat &t1, const NxSwFloat &t2);
	friend NxSwBool operator==(const NxSwFloat &t1, const NxSwFloat &t2);
	friend NxSwBool operator<=(const NxSwFloat &t1, const NxSwFloat &t2);
	friend NxSwBool operator>=(const NxSwFloat &t1, const NxSwFloat &t2);
	friend NxSwBool operator!=(const NxSwFloat &t1, const NxSwFloat &t2);

protected:
	NxReal val;
public:
	NX_INLINE NxSwFloat()									{}
	NX_INLINE NxSwFloat(NxReal v): val(v)					{}

	NX_INLINE NxSwFloat operator -() const					{	return NxSwFloat(-val);																	}
	NX_INLINE NxSwFloat recip()								{	return fabsf(val) < NxForceFieldEpsHolder<>::epsilon ? 0 : NxSwFloat(1.0f/val);			}
	NX_INLINE NxSwFloat recipSqrt()							{	return fabsf(val) < NxForceFieldEpsHolder<>::epsilon ? 0 : NxSwFloat(1.0f/sqrtf(val));	}
	NX_INLINE NxSwFloat sqrt()								{	return NxSwFloat(sqrtf(val));															}
	NX_INLINE static void setEpsilon(NxReal eps)			{	NxForceFieldEpsHolder<>::epsilon = eps;													}

};

NX_INLINE NxSwFloat operator +(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwFloat(t1.val+t2.val);
}
NX_INLINE NxSwFloat operator -(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwFloat(t1.val-t2.val);
}
NX_INLINE NxSwFloat operator *(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwFloat(t1.val*t2.val);
}
NX_INLINE NxSwBool operator<(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwBool(t1.val<t2.val);
}
NX_INLINE NxSwBool operator>(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwBool(t1.val>t2.val);
}
NX_INLINE NxSwBool operator==(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwBool(t1.val==t2.val);
}
NX_INLINE NxSwBool operator<=(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwBool(t1.val<=t2.val);
}
NX_INLINE NxSwBool operator>=(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwBool(t1.val>=t2.val);
}
NX_INLINE NxSwBool operator!=(const NxSwFloat &t1, const NxSwFloat &t2)
{
	return NxSwBool(t1.val!=t2.val);
}

class NxSwFloatVar: public NxSwFloat
{
public:
	NX_INLINE NxSwFloatVar(NxReal v)						{	val = v;		}
	NX_INLINE NxSwFloatVar(const NxSwFloat& t)				{	val = t.val;	}
	NX_INLINE void operator  =(const NxSwFloat& t)			{	val = t.val;	}
	NX_INLINE void operator +=(const NxSwFloat& t)			{	val += t.val;	}
	NX_INLINE void operator -=(const NxSwFloat& t)			{	val -= t.val;	}
	NX_INLINE void operator *=(const NxSwFloat& t)			{	val *= t.val;	}
};

class NxSwVec
{
	friend class NxSw;
	friend class NxSwVecVar;

private:
	NxVec3 val;
	NX_INLINE NxSwVec(NxReal mx, NxReal my, NxReal mz): val(mx,my,mz) {}
public:
	NX_INLINE NxSwVec()										{}
	NX_INLINE NxSwVec(const NxVec3& t): val(t)				{}

	NX_INLINE NxSwFloat getX() const						{	return NxSwFloat(val.x);			}
	NX_INLINE NxSwFloat getY() const						{	return NxSwFloat(val.y);			}
	NX_INLINE NxSwFloat getZ() const						{	return NxSwFloat(val.z);			}
	NX_INLINE NxSwVec operator +(const NxSwVec& t)	const	{	return NxSwVec(val + t);			}
	NX_INLINE NxSwVec operator -(const NxSwVec& t)	const	{	return NxSwVec(val - t);			}
	NX_INLINE NxSwVec operator -()					const	{	return NxSwVec( -val);				}
	NX_INLINE NxSwVec operator *(NxSwFloat f)		const	{	return NxSwVec(val * f.val);		}
	NX_INLINE NxSwFloat dot(const NxSwVec& t)		const	{	return NxSwFloat(val.dot(t.val));	}
	NX_INLINE NxSwVec cross(const NxSwVec& t)		const	{	return NxSwVec(val.cross(t.val));	}
	NX_INLINE operator NxVec3()						const	{	return val;							}
	NX_INLINE NxSwFloat magnitude()					const	{	return val.magnitude();				}
};

class NxSwVecVar: public NxSwVec
{
public:
	NX_INLINE NxSwVecVar():NxSwVec(0,0,0)					{}
	NX_INLINE NxSwVecVar(const NxSwVec& t)	 				{	val = t.val;}
	NX_INLINE void setX(const NxSwFloat &f)					{	val.x=f.val;	}
	NX_INLINE void setY(const NxSwFloat &f)					{	val.y=f.val;	}
	NX_INLINE void setZ(const NxSwFloat &f)					{	val.z=f.val;	}
	NX_INLINE void operator=(const NxSwVec& t)				{	val = t;	}
	NX_INLINE void operator +=(const NxSwVec& t) 			{	val += t;	}
	NX_INLINE void operator -=(const NxSwVec& t)			{	val -= t;	}
};

class NxSw
{
public:
	typedef NxSwFloat		FConstType;
	typedef NxSwFloatVar	FVarType;

	typedef NxSwVec			VConstType;
	typedef NxSwVecVar		VVarType;

	typedef NxSwBool		BConstType;
	typedef NxSwBoolVar		BVarType;

	static NX_INLINE bool	testFailure(NxSwBool condition)	{	return condition.val; }
	static NX_INLINE bool	testFinish(NxSwBool condition)	{	return condition.val; }
	
	static NX_INLINE bool	getBoolVal(const NxSwBool& b)	{	return b.val; }
	static NX_INLINE NxReal	getFloatVal(const NxSwFloat& f)	{	return f.val; }
	static NX_INLINE NxVec3	getVecVal(const NxSwVec& v)		{	return v.val; }
};

NX_INLINE NxSwFloat NxSwBool::select(NxSwFloat f0, NxSwFloat f1)
	{	
	return NxSwFloat(val ? f0 : f1); 
	}

NX_INLINE NxSwVec	NxSwBool::select(NxSwVec& v0, NxSwVec& v1)
	{	
	return NxSwVec(val ? v0 : v1); 
	}

} // namespace
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
