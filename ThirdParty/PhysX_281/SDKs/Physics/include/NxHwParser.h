#ifndef NX_PHYSICS_NXHWPARSER
#define NX_PHYSICS_NXHWPARSER

#include "Nxp.h"
#if NX_ENABLE_HW_PARSER

namespace NxForceFieldInternals 
{

class NxBoolExp;
class NxFloatExp;
class NxVecExp;
class NxCmd;

#define DllExport __declspec( dllexport ) 
#else
#define DllExport
#endif


class DllExport NxHwParser
{
public:
	static void parseKernel(NxForceFieldKernel& kernel);
	static void acquire();		// can also do locking
	static void release();
	static void dump();

	enum Op
	{
		INLINECONSTANTFLOAT, INLINECONSTANTBOOL, BOOLVAR, FLOATVAR, VECTORVAR, BOOLCONST, FLOATCONST, VECTORCONST,	// leaf
		RECIP, SQRT, RECIPSQRT, NEGATE, MAGNITUDE, VECTORX, VECTORY, VECTORZ,			// unary
		AND, OR, XOR, PLUS, MINUS, TIMES, LT, GT, LE, GE, EQ, NEQ, DOT, CROSS,			// binary
		SELECT																			// ternary
	};

	enum CmdOp
	{
		ASSIGN, PLUSASSIGN, MINUSASSIGN, TIMESASSIGN, FAILIF, FINISHIF, ASSIGNX, ASSIGNY, ASSIGNZ
	};

	static NxBoolExp *	b(bool b);														// NX_INLINE constant
	static NxBoolExp *	b(NxHwParser::Op op, int idOrOffset);							// vars, constants
	static NxBoolExp *	b(NxHwParser::Op op, NxBoolExp *b1, NxBoolExp *b2 = 0);			// boolean ops
	static NxBoolExp *	b(NxHwParser::Op op, NxFloatExp *b1, NxFloatExp *b2 = 0);		// comparisons

	static NxFloatExp *	f(NxReal f);													// NX_INLINE constant
	static NxFloatExp *	f(NxHwParser::Op op, int idOrOffset);							// vars, constants
	static NxFloatExp *	f(NxHwParser::Op, NxFloatExp *f1, NxFloatExp *f2 = 0);			// float ops
	static NxFloatExp *	f(NxHwParser::Op, NxVecExp *f1, NxVecExp *f2 = 0);				// dot product, magnitude
	static NxFloatExp *	f(NxHwParser::Op, NxBoolExp *, NxFloatExp *, NxFloatExp *);		// select

	static NxVecExp *		v(NxHwParser::Op, int offsetOrId);							// vars, constants
	static NxVecExp *		v(NxHwParser::Op, NxVecExp *f1, NxVecExp *f2 = 0);			// vec ops
	static NxVecExp *		v(NxHwParser::Op, NxVecExp *f1, NxFloatExp *f2);			// scaling
	static NxVecExp *		v(NxHwParser::Op, NxBoolExp *, NxVecExp *, NxVecExp *);		// select

	static void			cmd(NxHwParser::CmdOp, NxBoolExp *);							// failif, finishif
	static void			cmd(NxHwParser::CmdOp, int, NxBoolExp *);						// bool assign
	static void			cmd(NxHwParser::CmdOp, int, NxFloatExp *);						// float assign
	static void			cmd(NxHwParser::CmdOp, int, NxVecExp *);						// vec assign

	static int			getBoolVarIndex();
	static int			getFloatVarIndex();
	static int			getVectorVarIndex();
	static int			getBoolConstOffset();
	static int			getFloatConstOffset();
	static int			getVecConstOffset();
private:
	static int			sBoolVarIndex;
	static int			sFloatVarIndex;
	static int			sVectorVarIndex;
	static int			sBoolConstOffset;
	static int			sFloatConstOffset;
	static int			sVectorConstOffset;
	static int			sCmdCount;
	static				NxCmd *sCmd[1000];
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
