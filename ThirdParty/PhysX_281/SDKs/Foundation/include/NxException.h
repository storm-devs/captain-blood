#ifndef NX_FOUNDATION_NXEXCEPTION
#define NX_FOUNDATION_NXEXCEPTION
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
/**
 \brief Objects of this class are optionally thrown by some classes as part of the error reporting mechanism.
*/
class NxException
	{
	public:
	virtual NxErrorCode getErrorCode() = 0;
	virtual const char * getFile() = 0;
	virtual int getLine() = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
