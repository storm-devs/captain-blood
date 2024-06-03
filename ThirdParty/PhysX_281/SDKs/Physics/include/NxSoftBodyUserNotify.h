#ifndef NX_PHYSICS_NXSOFTBODYUSERNOTIFY
#define NX_PHYSICS_NXSOFTBODYUSERNOTIFY
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup physics
@{
*/


#include "Nxp.h"

class NxSoftBody;

/**
\brief An interface class that the user can implement in order to receive simulation events.

<b>Threading:</b> It is not necessary to make this class thread safe as it will only be called in the context of the
user thread.

See the NxSoftBodyNotify documentation for an example of how to use notifications.

@see NxScene.setSoftBodyUserNotify() NxScene.getSoftBodyUserNotify() NxUserNotify
*/
class NxSoftBodyUserNotify
{
	virtual ~NxSoftBodyUserNotify(){};
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
