#ifndef NX_FLUIDS_NXPARTICLEUPDATEDATA
#define NX_FLUIDS_NXPARTICLEUPDATEDATA
/** \addtogroup fluids
  @{
*/

/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

/**
Particle update flags are used specify flags which can be updated on the particles.
*/
enum NxParticleDataFlag
{
	NX_FP_DELETE		= (1<<0),	
};

/**
\brief Data structure to define particle update data.

There are two different types of updates, id based updates and particle order based updates.
Id based updates may contain an arbitrary number of elements, each paired with a particle id. 
Particle order based updates always need to have the elements matching the particles as received 
with the previous call to NxScene::fetchResults().

The id based update can be specified by passing particle ids per update element with 
NxParticleUpdateData::bufferId, NxParticleUpdateData::bufferIdByteStride and NxParticleUpdateData::numUpdates

The particle order based update is specified with setting NxParticleUpdateData::bufferId to NULL.
NxParticleUpdateData::numUpdates is ignored in this case.

@see NxFluid::updateParticles(const NxParticleUpdateData&)
*/
class NxParticleUpdateData
	{
	public:

	/**
	\brief Defines how the "force" buffer is interpreted.

	Supported:
		NX_FORCE,                   
		NX_IMPULSE,                 
		NX_VELOCITY_CHANGE,			
		NX_ACCELERATION		

	@see NxForceMode
	*/
	NxForceMode				forceMode;

	/**
	\brief Number of elements stored in the update buffers. 
	
	When passing id based updates to the SDK, the number is used to determine 
	how many ids and update forces or flags need to be read from the buffers. If passing particle order based 
	updates this parameter is ignored. 
	*/
	NxU32					numUpdates;

	/**
	\brief The pointer to the user-allocated buffer for particle forces.
	
	A force consists of three consecutive 32-bit floats. If set to NULL, forces are not read.
	*/
	NxF32*					bufferForce;
	
	/**
	\brief The pointer to the user-allocated buffer for particle update flags.

	Particle update flags are represented as a 32-bit unsigned integer. If set the NULL, flags are not read from.
	Use NxParticleDataFlag to set the information.
	*/
	NxU32*					bufferFlag;

	/**
	\brief The pointer to the user-allocated buffer for id based updates.

	Particle ids are represented as a 32-bit unsigned integer. If set the NULL, NxFluid::updateParticles(...) 
	will assume an index based update on all particles.
	*/
	NxU32*					bufferId;

	/**
	\brief The separation (in bytes) between consecutive particle forces.

	The force of the first particle is found at location <tt>bufferForce</tt>;
	the second is at <tt>bufferForce + bufferForceByteStride</tt>;
	and so on.
	*/
	NxU32					bufferForceByteStride;

	/**
	\brief The separation (in bytes) between consecutive particle update flags.

	The update flags of the first particle is found at location <tt>bufferFlag</tt>;
	the second is at <tt>bufferFlag + bufferFlagByteStride</tt>;
	and so on.
	*/
	NxU32					bufferFlagByteStride;

	/**
	\brief The separation (in bytes) between consecutive particle ids.

	The id of the first particle is found at location <tt>bufferId</tt>;
	the second is at <tt>bufferId + bufferIdByteStride</tt>;
	and so on.
	*/
	NxU32					bufferIdByteStride;

	NX_INLINE ~NxParticleUpdateData();
	/**
	\brief (Re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	\brief Returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE	NxParticleUpdateData();
	};

NX_INLINE NxParticleUpdateData::NxParticleUpdateData()
	{
	setToDefault();
	}

NX_INLINE NxParticleUpdateData::~NxParticleUpdateData()
	{
	}

NX_INLINE void NxParticleUpdateData::setToDefault()
	{
	forceMode				= NX_FORCE;
	numUpdates				= 0;
	bufferForce				= NULL;
	bufferFlag				= NULL;
	bufferId				= NULL;
	bufferForceByteStride	= 0;
	bufferFlagByteStride	= 0;
	bufferIdByteStride		= 0;
	}

NX_INLINE bool NxParticleUpdateData::isValid() const
	{
	if (!(bufferForce || bufferFlag)) return false;
	if (bufferForce && !bufferForceByteStride) return false;
	if (bufferFlag && !bufferFlagByteStride) return false;
	if (bufferId && !bufferIdByteStride) return false;
	if (bufferId && !numUpdates) return false;
	return true;
	}

/** @} */
#endif


//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND

