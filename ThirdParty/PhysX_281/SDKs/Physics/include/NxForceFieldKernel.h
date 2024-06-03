#ifndef NX_PHYSICS_NXFORCEFIELDKERNEL
#define NX_PHYSICS_NXFORCEFIELDKERNEL
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

enum NxForcFieldKernelType
	{
	NX_FFK_LINEAR_KERNEL, //!< The kernel type is linear kernel. For internal use only.
	NX_FFK_CUSTOM_KERNEL  //!< The kernel type is custom kernel. For internal use only.
	};

class NxForceFieldKernel
{
public:
	virtual						~NxForceFieldKernel(){};																			 //!< For internal use only.
	virtual	void				parse()																					const	= 0; //!< For internal use only.
	virtual	bool				evaluate(NxVec3 &force,	NxVec3 &torque, const NxVec3 &position, const NxVec3 &velocity) const	= 0; //!< For internal use only.
	virtual	NxU32				getType()																				const	= 0; //!< For internal use only.
	virtual NxForceFieldKernel*	clone()																					const	= 0; //!< For internal use only.
	virtual void				update(NxForceFieldKernel& in)															const	= 0; //!< For internal use only.
	virtual	void				setEpsilon(NxReal eps)																			= 0; //!< For internal use only.

	void*						userData;																							//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
