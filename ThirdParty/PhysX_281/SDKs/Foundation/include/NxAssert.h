#ifndef NX_FOUNDATION_NXASSERT
#define NX_FOUNDATION_NXASSERT
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


/**
This ANSI assert is included so that NX_ASSERTs can continue to appear in user side
code, where the custom assert in Assert.h would not work.
*/

#include <assert.h>
#ifndef NX_ASSERT
  #ifdef _DEBUG
    #define NX_ASSERT(x) assert(x)
  #else
    #define NX_ASSERT(x) {}
  #endif
#endif

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
