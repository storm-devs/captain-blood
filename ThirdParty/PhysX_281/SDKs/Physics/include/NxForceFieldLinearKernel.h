#ifndef NX_PHYSICS_NXFORCEFIELDLINEARKERNEL
#define NX_PHYSICS_NXFORCEFIELDLINEARKERNEL
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
#include "NxForceFieldKernel.h"
#include "NxForceFieldLinearKernelDesc.h"


class NxForceFieldLinearKernel : public NxForceFieldKernel
{
protected:
	NX_INLINE	NxForceFieldLinearKernel()	{}
	virtual		~NxForceFieldLinearKernel()	{}

public:

	/**
	\brief Gets the constant part of force field function.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxVec3 getConstant () const= 0;
	
	/**
	\brief Sets the constant part of force field function.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setConstant (const NxVec3 &) = 0;						
	
	/**
	\brief Gets the coefficient of force field function position term.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxMat33 getPositionMultiplier () const = 0;
	
	/**
	\brief Sets the coefficient of force field function position term.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setPositionMultiplier (const NxMat33 & ) = 0;
	
	/**
	\brief Gets the coefficient of force field function velocity term.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxMat33 getVelocityMultiplier () const = 0;
	

	/**
	\brief Sets the coefficient of force field function velocity term.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setVelocityMultiplier (const NxMat33 & ) = 0;

	/**
	\brief Gets the force field position target.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxVec3 getPositionTarget () const = 0;
	
	/**
	\brief  Sets the force field position target.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setPositionTarget (const NxVec3 & ) = 0;
	
	/**
	\brief Gets the force field velocity target.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxVec3 getVelocityTarget () const = 0;
	
	/**
	\brief Sets the force field velocity target.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setVelocityTarget (const NxVec3 & ) = 0;
	
	/**
	\brief Sets the linear falloff term.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxVec3 getFalloffLinear() const = 0;

	/**
	\brief Sets the linear falloff term.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setFalloffLinear(const NxVec3 &) = 0;
	
	/**
	\brief Sets the quadratic falloff term.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxVec3 getFalloffQuadratic() const = 0;
	
	/**
	\brief Sets the quadratic falloff term.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setFalloffQuadratic(const NxVec3 &) = 0;
	
	/**
	\brief Gets the force field noise.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxVec3 getNoise () const = 0;
	
	/**
	\brief Sets the force field noise.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setNoise (const NxVec3 & ) = 0;

	/** 
	\brief ets the toroidal radius.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxReal getTorusRadius () const = 0;

	/** 
	\brief ets the toroidal radius.
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void setTorusRadius(NxReal) = 0;

	/**
	\brief Retrieves the scene which this kernel belongs to.

	\return Owner Scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene
	*/
	virtual		NxScene&		getScene()	const = 0;

	/**
	\brief Writes all of the kernel's attributes to the description, as well
	as setting the actor connection point.

	\param[out] desc The descriptor used to retrieve the state of the kernel.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void saveToDesc(NxForceFieldLinearKernelDesc &desc) = 0;

	/**
	\brief Sets a name string for the object that can be retrieved with getName().
	
	This is for debugging and is not used by the SDK. The string is not copied by the SDK, only the pointer is stored.
	
	\param[in] name String to set the objects name to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see getName()
	*/
	virtual void  setName (const char* name)= 0;

	/**
	\brief Retrieves the name string set with setName().

	\return The name string for this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see setName()
	*/
	virtual const char*  getName () const = 0;
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
