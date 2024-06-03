#ifndef NX_PHYSICS_NX_COMPARTMENT_DESC
#define NX_PHYSICS_NX_COMPARTMENT_DESC
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

enum NxCompartmentType
	{
	NX_SCT_RIGIDBODY = 0,
	NX_SCT_FLUID = 1,
	NX_SCT_CLOTH = 2,
	NX_SCT_SOFTBODY = 2	//!< Currently SoftBody and Cloth use the same compartments, so you can create SoftBodies in a Cloth compartment. This might change in future versions.
	};

enum NxDeviceCode
	{
	NX_DC_PPU_0	= 0,							//!< Explicit PPU index corresponds to index numeric value (reserved for future use, currently only PPU 0 is valid)
	NX_DC_PPU_1 = 1,
	NX_DC_PPU_2 = 2,
	NX_DC_PPU_3 = 3,
	NX_DC_PPU_4 = 4,
	NX_DC_PPU_5 = 5,
	NX_DC_PPU_6 = 6,
	NX_DC_PPU_7 = 7,
	NX_DC_PPU_8 = 8,
	// ...

	NX_DC_CPU = 0xffff0000,						//!< Compartment is to be simulated on CPU
	NX_DC_PPU_AUTO_ASSIGN = 0xffff0001,			//!< Compartment is to be simulated on a processor (PPU or CPU) chosen by the HSM for best performance (CPU is only used in the absence of a PPU).
	};


/**
\brief Descriptor class for NxCompartment.  A compartment is a portion of the scene that can
be simulated on a different hardware device than other parts of the scene.
*/
class NxCompartmentDesc
	{
	public:
	NxCompartmentType type;						//!< Compartment meant for this type of simulation.  Will be removed in 3.0, as comparments will become type-neutral. Cloth type compartments must have a non-CPU device code.

	/**
	\brief A NxDeviceCode, incl. a PPU index from 0 to 31.
	@see NxDeviceCode
	*/
	NxU32			deviceCode;	
	NxReal			gridHashCellSize;			//!< Size in distance units of a single cell in the paging grid.  Should be set to the size of the largest common dynamic object in this compartment.
	NxU32			gridHashTablePower;			//!< 2-power used to determine size of the hash table that geometry gets hashed into.  Hash table size is (1 << gridHashTablePower).
	NxU32			flags;						//!< Combination of ::NxCompartmentFlag values
	NxU32			threadMask;					//!< Thread affinity mask for the compartment thread.  Defaults to 0 which means the SDK determines the affinity.

	/**
	\brief It is possible to scale the simulation time steps of this compartment relative to the primary scene using this nonnegative value.  A scale of zero does not simulate the compartment. The default value is 1.0f.

	Note: The time scale is taken into account after the number of substeps has been calculated. This means that the maximum timestep setting of the compartment can be exceeded when timeScale is above 1.0f.
	*/
	NxReal			timeScale;

	NX_INLINE void setToDefault()
		{
		type = NX_SCT_RIGIDBODY;
		deviceCode = (NxU32) NX_DC_CPU;
		gridHashCellSize = 100.0f;				//was 2.0f in 2.5, bumped up.
		gridHashTablePower = 8;
		flags = NX_CF_INHERIT_SETTINGS;
		timeScale = 1.0f;
		threadMask = 0;
		}

	/**
	\brief Returns true if the descriptor is valid.

	\return return true if the current settings are valid
	*/

	NX_INLINE bool isValid() const
		{
		if (deviceCode != NX_DC_PPU_0 && 
			deviceCode != NX_DC_PPU_AUTO_ASSIGN &&
			deviceCode != NX_DC_CPU)
			return false;
		if (timeScale < 0.0f)
			return false;
		return (type <= NX_SCT_CLOTH) && (gridHashCellSize > 0.0f);
		}

	NxCompartmentDesc()
		{
		setToDefault();
		}
	};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
