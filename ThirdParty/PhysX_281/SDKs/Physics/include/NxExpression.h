#ifndef NX_PHYSICS_NXEXPRESSION
#define NX_PHYSICS_NXEXPRESSION

#include <iostream>

#include "NxHwParser.h"
#if NX_ENABLE_HW_PARSER

namespace NxForceFieldInternals 
{

class NxFloatExp;
class NxVecExp;

class NxExp
{
	friend std::ostream& operator<<(std::ostream& stream, const NxExp& );
public:
	enum Type
	{
		BOOL, FLOAT, VECTOR
	};
	Type mType;
protected:
	NxExp(Type t): mType(t) {}	
};

class NxBoolExp: public NxExp
{
	friend std::ostream& operator<<(std::ostream& stream, const NxBoolExp& );

	NxHwParser::Op mOp;
	union
	{
		bool mValue;
		int mOffset;
		int mId;
		struct { NxFloatExp *mFloat1, *mFloat2;	};
		struct { NxBoolExp *mBool1, *mBool2;		};
	};

public:
	NxBoolExp(bool val): NxExp(BOOL), mOp(NxHwParser::INLINECONSTANTBOOL), mValue(val)							{}
	NxBoolExp(NxHwParser::Op op, int offsetOrId): NxExp(BOOL), mOp(op), mOffset(offsetOrId)					{}
	NxBoolExp(NxHwParser::Op op, NxBoolExp *a1, NxBoolExp *a2): NxExp(BOOL), mOp(op), mBool1(a1), mBool2(a2)		{}
	NxBoolExp(NxHwParser::Op op, NxFloatExp *a1, NxFloatExp *a2): NxExp(BOOL), mOp(op), mFloat1(a1), mFloat2(a2)	{}
};

class NxFloatExp: public NxExp
{
	friend std::ostream& operator<<(std::ostream& stream, const NxFloatExp& );

	NxHwParser::Op mOp;
	union
	{
		int mOffset;
		int mId;
		NxReal mValue;
		struct	{ NxBoolExp *mBool; NxFloatExp *mFloat1, *mFloat2; };
		struct	{ NxVecExp *mVec1, *mVec2;	};
	};
public:	
	NxFloatExp(NxReal val): NxExp(FLOAT), mOp(NxHwParser::INLINECONSTANTFLOAT), mValue(val)										{}
	NxFloatExp(NxHwParser::Op op, int offsetOrId): NxExp(FLOAT), mOp(op), mOffset(offsetOrId)									{}
	NxFloatExp(NxHwParser::Op op, NxBoolExp *b, NxFloatExp *a1, NxFloatExp *a2): NxExp(FLOAT), mOp(op), mBool(b), mFloat1(a1), mFloat2(a2)	{}
	NxFloatExp(NxHwParser::Op op, NxFloatExp *a1, NxFloatExp *a2 = 0): NxExp(FLOAT), mOp(op), mFloat1(a1), mFloat2(a2)			{}
	NxFloatExp(NxHwParser::Op op, NxVecExp *a1, NxVecExp *a2 = 0): NxExp(FLOAT), mOp(op), mVec1(a1), mVec2(a2)					{}
};

class NxVecExp: public NxExp
{
	friend std::ostream& operator<<(std::ostream& stream, const NxVecExp& );

	NxHwParser::Op mOp;
	union
	{
		int mOffset;
		int mId;
		struct	{	NxBoolExp *mBool;	NxFloatExp *mFloat1; NxVecExp *mVec1, *mVec2;	};
	};
public:
	
	NxVecExp(NxHwParser::Op op, int offsetOrId): NxExp(VECTOR), mOp(op), mOffset(offsetOrId)									{}
	NxVecExp(NxHwParser::Op op, NxVecExp *a1, NxVecExp *a2 = 0): NxExp(VECTOR), mOp(op), mVec1(a1), mVec2(a2)					{}
	NxVecExp(NxHwParser::Op op, NxVecExp *a1, NxFloatExp *a2): mOp(op), NxExp(VECTOR), mVec1(a1), mFloat1(a2)					{}
	NxVecExp(NxHwParser::Op op, NxBoolExp *b, NxVecExp *a1, NxVecExp *a2): NxExp(VECTOR), mOp(op), mBool(b), mVec1(a1), mVec2(a2)	{}

};

class NxCmd
{
	friend std::ostream& operator<<(std::ostream& stream, const NxCmd& );

	NxHwParser::CmdOp mOp;
	int mVarId;
	NxExp *mExp;
public:
	NxCmd(NxHwParser::CmdOp op, NxBoolExp *e): mOp(op), mExp(e)						{}
	NxCmd(NxHwParser::CmdOp op, int v, NxExp *e): mOp(op), mVarId(v), mExp(e)			{}
};

} // namespace
#endif
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
