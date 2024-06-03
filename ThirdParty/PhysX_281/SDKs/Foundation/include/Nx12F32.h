#ifndef NX_FOUNDATION_NX12F32
#define NX_FOUNDATION_NX12F32
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

//the file name of this header is legacy due to pain of renaming file in repository.

#include "Nxf.h"

class Nx12Real
	{
	public:
    union 
		{
        struct S
			{
            NxReal        _11, _12, _13, _14;
            NxReal        _21, _22, _23, _24;
            NxReal        _31, _32, _33, _34;
			} s;
		NxReal m[3][4];
		};
	};

/** \endcond */
 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
