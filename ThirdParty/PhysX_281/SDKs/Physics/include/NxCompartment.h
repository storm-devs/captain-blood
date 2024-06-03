#ifndef NX_PHYSICS_NX_COMPARTMENT
#define NX_PHYSICS_NX_COMPARTMENT
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

#include "NxCompartmentDesc.h"

/**
\brief A scene compartment is a portion of the scene that can
be simulated on a different hardware device than other parts of the scene.

Note: You cannot release scene compartments explicitly, they are automatically released when the scene is released.
To create a compartment, call NxScene::createCompartment()
*/
class NxCompartment
	{
	public:
	/**
	\return the compartment simulation type.  Will be removed in 3.0, as compartments will become type-neutral.
	*/
	virtual NxCompartmentType getType() const = 0;

	/**
	\brief The device code that is specified when creating a compartment or the automatically
	assigned device code, if NX_DC_PPU_AUTO_ASSIGN was used.

	\return the ::NxDeviceCode of the compartment.
	*/
	virtual NxU32 getDeviceCode() const = 0;

	/**
	\return the paging grid cell size.
	*/
	virtual NxReal getGridHashCellSize() const = 0;

	/**
	\return the paging grid power.
	*/
	virtual NxU32 gridHashTablePower() const = 0;

	/**
	\brief Sets the time scale for the compartment.

	@see NxCompartmentDesc::timeScale getTimeScale
	*/
	virtual void setTimeScale(NxReal) = 0;

	/**
	\return the time scale for the compartment.

	@see NxCompartmentDesc::timeScale setTimeScale
	*/
	virtual NxReal getTimeScale() const = 0;

	/**
 	\brief Sets simulation timing parameters used to simulate the compartment.  

	The initial default settings are inherited from the primary scene.

	If method is NX_TIMESTEP_FIXED, elapsedTime(simulate() parameter) is internally subdivided into up to
	maxIter substeps no larger than maxTimestep.
	
	If elapsedTime is not a multiple of maxTimestep then any remaining time is accumulated 
	to be added onto the elapsedTime for the next time step.

	If more sub steps than maxIter are needed to advance the simulation by elapsed time, then
	the remaining time is also accumulated for the next call to simulate().
	
	The timestep method of TIMESTEP_FIXED is strongly preferred for stable, reproducible simulation.

	Alternatively NX_TIMESTEP_VARIABLE can be used, in this case the first two parameters
	are not used.	See also ::NxTimeStepMethod.

	\param[in] maxTimestep Maximum size of a substep. <b>Range:</b> (0,inf)
	\param[in] maxIter Maximum number of iterations to divide a timestep into.
	\param[in] method Method to use for timestep (either variable time step or fixed). See #NxTimeStepMethod.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setTimeScale() getTiming()
	*/
	virtual void						setTiming(NxReal maxTimestep=1.0f/60.0f, NxU32 maxIter=8, NxTimeStepMethod method=NX_TIMESTEP_FIXED) = 0;

	/**
	\brief Retrieves simulation timing parameters.

	\param[in] maxTimestep Maximum size to divide a substep into. <b>Range:</b> (0,inf)
	\param[in] maxIter Maximum number of iterations to divide a timestep into.
	\param[in] method Method to use for timestep (either variable time step or fixed). See #NxTimeStepMethod.
	\param[in] numSubSteps The number of sub steps the time step will be divided into.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setTiming() setTimeScale()
	*/
	virtual void						getTiming(NxReal& maxTimestep, NxU32& maxIter, NxTimeStepMethod& method, NxU32* numSubSteps=NULL) const = 0;



	/**
	\brief This checks to see if the simulation of the objects in this compartment has completed.

	This does not cause the data available for reading to be updated with the results of the simulation, it is simply a status check.
	The bool will allow it to either return immediately or block waiting for the condition to be met so that it can return true

	\param[in] block When set to true will block until the condition is met.
	\return True if the results are available.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	bool						checkResults(bool block = false)	= 0;

	/**
	This is the big brother to checkResults() it basically makes the results of the compartment's simulation readable.
	

	The entire scene incl. the compartments will still be locked for writing until you call fetchResults(NX_RIGID_BODY_FINISHED).

	\param[in] block When set to true will block until the condition is met.
	\return True if the results have been fetched.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	bool						fetchResults(bool block = false)	= 0;

	/**
	\brief Saves the compartment to a descriptor. Please note that the device code will be the same as the 
	value returned by getDeviceCode(), i.e. not necessarily the same as the value you assigned when first
	creating the compartment (when using auto-assign).

	\return true
	*/
	virtual bool						saveToDesc(NxCompartmentDesc& desc) const = 0;

	/**
	\brief Sets the compartment flags, a combination of the bits defined by the enum ::NxCompartmentFlag.

	\param[in] flags #NxCompartmentFlag combination.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCompartmentDesc.flags NxCompartmentFlag getFlags()
	*/

	virtual void setFlags(NxU32 flags) = 0;

	/**
	\brief Retrieves the compartment flags.

	\return The compartment flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCompartmentDesc.flags NxCompartmentFlag setFlags()
	*/
	virtual NxU32 getFlags() const = 0;

	protected:
	virtual ~NxCompartment(){};
	};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
