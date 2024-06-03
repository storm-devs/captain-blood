#ifndef NX_PHYSICS_NXCONVEXFORCEFIELDSHAPE
#define NX_PHYSICS_NXCONVEXFORCEFIELDSHAPE
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

class NxConvexForceFieldShapeDesc;

/**
 \brief Convex shaped region used to define force field.
 

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

 @see NxForceFieldShape
*/
class NxConvexForceFieldShape  : public NxForceFieldShape
{
public:
	/**
	\brief Saves the state of the shape object to a descriptor.

	\param[out] desc Descriptor to save to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxConvexForceFieldShapeDesc
	*/
	virtual	void saveToDesc(NxConvexForceFieldShapeDesc& desc) const=0;
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
