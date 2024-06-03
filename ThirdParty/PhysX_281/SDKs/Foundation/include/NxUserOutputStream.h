#ifndef NX_FOUNDATION_NXUSEROUTPUTSTREAM
#define NX_FOUNDATION_NXUSEROUTPUTSTREAM
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

enum NxAssertResponse
	{
	NX_AR_CONTINUE,			//!continue execution
	NX_AR_IGNORE,			//!continue and don't report this assert from now on
	NX_AR_BREAKPOINT		//!trigger a breakpoint
	};

/**
 \brief User defined interface class.  Used by the library to emit debug information.

 \note The SDK state should not be modified from within any error reporting functions.

 <b>Threading:</b> It is not necessary to make this class thread safe as it will only be called in the context of the
 user thread.
*/
class NxUserOutputStream
	{
	public:

	/**
	\brief Reports an error code.

	\param code Error code, see #NxErrorCode
	\param message Message to display.
	\param file File error occured in.
	\param line Line number error occured on.
	*/
	virtual void reportError(NxErrorCode code, const char * message, const char *file, int line) = 0;
	/**
	\brief Reports an assertion violation.  The user should return 

	\param message Message to display.
	\param file File error occured in.
	\param line Line number error occured on.
	*/
	virtual NxAssertResponse reportAssertViolation(const char * message, const char *file, int line) = 0;
	/**
	\brief Simply prints some debug text

	\param message Message to display.
	*/
	virtual void print(const char * message) = 0;

	protected:
	virtual ~NxUserOutputStream(){};
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
