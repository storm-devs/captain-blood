#ifndef NX_PHYSICS_NXHWTARGET
#define NX_PHYSICS_NXHWTARGET

#include "Nxp.h"
#if NX_ENABLE_HW_PARSER
#include "NxForceFieldKernel.h"
#include "NxExpression.h"
#include "NxHwParser.h"

namespace NxForceFieldInternals 
{

class NxHwFloat;
class NxHwVec;

class NxHwBool
{
	friend class NxHwBoolVar;
	friend class NxHwVec;
	friend class NxHwFloat;
	friend NxHwBool operator<(const NxHwFloat &f1, const NxHwFloat &f2);
	friend NxHwBool operator>(const NxHwFloat &f1, const NxHwFloat &f2);
	friend NxHwBool operator==(const NxHwFloat &f1, const NxHwFloat &f2);
	friend NxHwBool operator<=(const NxHwFloat &f1, const NxHwFloat &f2);
	friend NxHwBool operator>=(const NxHwFloat &f1, const NxHwFloat &f2);
	friend NxHwBool operator!=(const NxHwFloat &f1, const NxHwFloat &f2);
	friend NxHwBool operator &(const NxHwBool &b1, const NxHwBool &b2);
	friend NxHwBool operator |(const NxHwBool &b1, const NxHwBool &b2);
	friend NxHwBool operator ^(const NxHwBool &b1, const NxHwBool &b2);

private:
	NxHwBool(NxBoolExp *exp): mExp(exp) {}
protected:
	NxHwBool():	mExp(0) {}
public:
	NxBoolExp *mExp;	// FIXME
	NxHwBool(bool b):	mExp(NxHwParser::b(b))		{ }
	NxHwBool(int b):	mExp(NxHwParser::b(!!b))	{ }	// For "Boolean b = true ^ true;"
	NX_INLINE NxHwFloat	select(NxHwFloat f0, NxHwFloat f1);		
	NX_INLINE NxHwVec	select(NxHwVec& v0, NxHwVec& v1);			
};

NX_INLINE NxHwBool operator &(const NxHwBool &b1, const NxHwBool &b2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::AND,b1.mExp,b2.mExp));
}
NX_INLINE NxHwBool operator |(const NxHwBool &b1, const NxHwBool &b2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::OR,b1.mExp,b2.mExp));
}
NX_INLINE NxHwBool operator ^(const NxHwBool &b1, const NxHwBool &b2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::XOR,b1.mExp,b2.mExp));
}


class NxHwBoolVar: public NxHwBool
{
	int mId;
public:
	NxHwBoolVar(const NxHwBool& b):	mId(NxHwParser::getBoolVarIndex()) 		{	NxHwParser::cmd(NxHwParser::ASSIGN, mId, b.mExp); 
																		mExp = NxHwParser::b(NxHwParser::BOOLVAR, mId);		
																	}
	void operator=(const NxHwBool& b)									{	NxHwParser::cmd(NxHwParser::ASSIGN ,mId, b.mExp);	}
};

class NxHwBoolConst: public NxHwBool
{
	int mOffset;
public:
	NxHwBoolConst(): mOffset(NxHwParser::getBoolConstOffset())		{	mExp = NxHwParser::b(NxHwParser::BOOLCONST, mOffset); }
};

// -------------------------------------------------

class NxHwFloat
{
friend class NxHwVec;
friend class NxHw;
friend class NxHwFloatVar;
friend class NxHwBool;
friend NxHwFloat operator +(const NxHwFloat &f1, const NxHwFloat &f2);
friend NxHwFloat operator -(const NxHwFloat &f1, const NxHwFloat &f2);
friend NxHwFloat operator *(const NxHwFloat &f1, const NxHwFloat &f2);

private:
	NxHwFloat(NxFloatExp *exp): mExp(exp) {}

	static const NxHwFloat EPSILON;

protected:
	NxHwFloat(): mExp(0) {}

public:
	NxFloatExp *mExp;		// FIXME
	NxHwFloat(NxReal f): mExp(NxHwParser::f(f))			{}

	NxHwFloat operator -()						const	{	return NxHwFloat(NxHwParser::f(NxHwParser::NEGATE, mExp));			}
	NxHwFloat recip()							const	{	return NxHwFloat(NxHwParser::f(NxHwParser::RECIP, mExp));			}
	NxHwFloat recipSqrt()						const	{	return NxHwFloat(NxHwParser::f(NxHwParser::RECIPSQRT, mExp));		}
	NxHwFloat sqrt()							const	{	return NxHwFloat(NxHwParser::f(NxHwParser::SQRT, mExp));			}
};

NX_INLINE NxHwFloat operator +(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwFloat(NxHwParser::f(NxHwParser::PLUS, f1.mExp, f2.mExp));
}
NX_INLINE NxHwFloat operator -(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwFloat(NxHwParser::f(NxHwParser::MINUS, f1.mExp, f2.mExp));
}
NX_INLINE NxHwFloat operator *(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwFloat(NxHwParser::f(NxHwParser::TIMES, f1.mExp, f2.mExp));
}
NX_INLINE NxHwBool operator<(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::LT, f1.mExp, f2.mExp));	
}
NX_INLINE NxHwBool operator>(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::GT, f1.mExp, f2.mExp));	
}
NX_INLINE NxHwBool operator==(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::EQ, f1.mExp, f2.mExp));	
}
NX_INLINE NxHwBool operator<=(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::LE, f1.mExp, f2.mExp));	
}
NX_INLINE NxHwBool operator>=(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::GE, f1.mExp, f2.mExp));	
}
NX_INLINE NxHwBool operator!=(const NxHwFloat &f1, const NxHwFloat &f2)
{
	return NxHwBool(NxHwParser::b(NxHwParser::NEQ, f1.mExp, f2.mExp));	
}

class NxHwFloatVar: public NxHwFloat
{
	int mId;
public:

	NxHwFloatVar(NxReal f) 											{	mExp = NxHwParser::f(f);								}
	NxHwFloatVar(const NxHwFloat& f): mId(NxHwParser::getFloatVarIndex())	{	NxHwParser::cmd(NxHwParser::ASSIGN, mId, f.mExp);		
																	mExp = NxHwParser::f(NxHwParser::FLOATVAR, mId);
																}
	void operator  =(const NxHwFloat& f)							{	printf("=\n");NxHwParser::cmd(NxHwParser::ASSIGN, mId, f.mExp);		}
	void operator +=(const NxHwFloat& f)							{	NxHwParser::cmd(NxHwParser::PLUSASSIGN, mId, f.mExp);	}
	void operator -=(const NxHwFloat& f)							{	NxHwParser::cmd(NxHwParser::MINUSASSIGN, mId, f.mExp);	}
	void operator *=(const NxHwFloat& f)							{	NxHwParser::cmd(NxHwParser::TIMESASSIGN, mId, f.mExp);	}
};

class NxHwFloatConst: public NxHwFloat
{
	int mOffset;
public:
	NxHwFloatConst(): mOffset(NxHwParser::getFloatConstOffset())	{	mExp = NxHwParser::f(NxHwParser::FLOATCONST, mOffset); }
};


// -------------------------------------------------

class NxHwVec
{
	friend class NxHw;
	friend class NxHwVecVar;
	friend class NxHwVecConst;
	friend class NxHwBool;

	NxHwVec(NxVecExp *exp): mExp(exp) {}

protected:
	NxHwVec(): mExp(0) {}

public:

	NxVecExp *mExp;	// FIXME

	NxHwFloat getX()								{	return NxHwFloat(NxHwParser::f(NxHwParser::VECTORX, mExp));		}
	NxHwFloat getY()								{	return NxHwFloat(NxHwParser::f(NxHwParser::VECTORY, mExp));		}
	NxHwFloat getZ()								{	return NxHwFloat(NxHwParser::f(NxHwParser::VECTORZ, mExp));		}
	NxHwVec operator +(const NxHwVec& v)	const	{	return NxHwVec(NxHwParser::v(NxHwParser::PLUS, mExp, v.mExp));	}
	NxHwVec operator -(const NxHwVec& v)	const	{	return NxHwVec(NxHwParser::v(NxHwParser::MINUS, mExp, v.mExp));	}
	NxHwVec operator -()					const	{	return NxHwVec(NxHwParser::v(NxHwParser::NEGATE, mExp));		}
	NxHwVec operator *(const NxHwFloat& f)	const	{	return NxHwVec(NxHwParser::v(NxHwParser::TIMES, mExp, f.mExp));	}
	NxHwFloat dot(const NxHwVec& v)			const	{	return NxHwFloat(NxHwParser::f(NxHwParser::DOT, mExp, v.mExp));	}
	NxHwVec cross(const NxHwVec& v)			const	{	return NxHwVec(NxHwParser::v(NxHwParser::CROSS, mExp, v.mExp));	}
	NxHwFloat magnitude()					const	{	return NxHwFloat(NxHwParser::f(NxHwParser::MAGNITUDE, mExp));	}

};

class NxHwVecVar: public NxHwVec
{
	int mId;
public:
	NxHwVecVar(): mId(NxHwParser::getVectorVarIndex())					{	mExp = NxHwParser::v(NxHwParser::VECTORVAR, mId);	}
	NxHwVecVar(const NxHwVec& v): mId(NxHwParser::getVectorVarIndex())	{	NxHwParser::cmd(NxHwParser::ASSIGN, mId, v.mExp);
																	mExp = NxHwParser::v(NxHwParser::VECTORVAR, mId); 
																}
	void setX(const NxHwFloat &f)								{	NxHwParser::cmd(NxHwParser::ASSIGNX, mId, f.mExp);		}
	void setY(const NxHwFloat &f)								{	NxHwParser::cmd(NxHwParser::ASSIGNY, mId, f.mExp);		}
	void setZ(const NxHwFloat &f)								{	NxHwParser::cmd(NxHwParser::ASSIGNZ, mId, f.mExp);		}
	void operator  =(const NxHwVec& v)							{	NxHwParser::cmd(NxHwParser::ASSIGN, mId, v.mExp);		}
	void operator +=(const NxHwVec& v)							{	NxHwParser::cmd(NxHwParser::PLUSASSIGN, mId, v.mExp);	}
	void operator -=(const NxHwVec& v)							{	NxHwParser::cmd(NxHwParser::MINUSASSIGN, mId, v.mExp);	}
};

class NxHwVecConst: public NxHwVec
{
	int mOffset;
public:
	NxHwVecConst(): mOffset(NxHwParser::getVecConstOffset())		{	mExp = NxHwParser::v(NxHwParser::VECTORCONST, mOffset); }
};

class NxHw
{
public:

	typedef NxHwBoolConst	BConstType;
	typedef NxHwBoolVar		BVarType;
	
	typedef NxHwFloatConst	FConstType;
	typedef NxHwFloatVar	FVarType;
	
	typedef NxHwVecConst	VConstType;
	typedef NxHwVecVar		VVarType;
	
	static bool				testFailure(NxHwBool cond)				{	NxHwParser::cmd(NxHwParser::FAILIF,cond.mExp);	return false;	}
	static bool				testFinish(NxHwBool cond)				{	NxHwParser::cmd(NxHwParser::FINISHIF,cond.mExp);	return false;	}
};

//void parseKernel(NxForceFieldKernel& kernel);

NX_INLINE NxHwFloat NxHwBool::select(NxHwFloat f0, NxHwFloat f1)	
	{
	NxFloatExp *f = NxHwParser::f(NxHwParser::SELECT, mExp,f0.mExp,f1.mExp);
	return NxHwFloat(f);
	}

NX_INLINE NxHwVec NxHwBool::select(NxHwVec& v0, NxHwVec& v1)		
	{	
	return NxHwVec(NxHwParser::v(NxHwParser::SELECT, mExp,v0.mExp,v1.mExp));	
	}

} // namespace
#endif
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
