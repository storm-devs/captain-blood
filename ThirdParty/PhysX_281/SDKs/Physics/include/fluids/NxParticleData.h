#ifndef NX_FLUIDS_NXPARTICLEDATA
#define NX_FLUIDS_NXPARTICLEDATA
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

/**
Particle flags are used to give some additional information about the particles.
*/
enum NxParticleFlag
{
	NX_FP_COLLISION_WITH_STATIC		= (1<<0),	
	NX_FP_COLLISION_WITH_DYNAMIC	= (1<<1),
	NX_FP_SEPARATED					= (1<<2),
	NX_FP_MOTION_LIMIT_REACHED		= (1<<3),
};

/**
\brief Descriptor-like user-side class describing a set of fluid particles.

NxParticleData is used to submit particles to the simulation and
to retrieve information about particles in the simulation. 

Each particle is described by its position, velocity, lifetime, density, and a set of (NxParticleFlag) flags.
The memory for the particle data is allocated by the application,
making this class a "user buffer wrapper".
*/
class NxParticleData
	{
	public:

	/**
	\brief Points to the user-allocated memory holding the number of elements stored in the buffers. 
	
	If the SDK writes to a given particle buffer, it also sets the numbers of elements written. If 
	numParticlesPtr is set to NULL, the SDK can't write to the given buffer. 
	When passing particles to the SDK, the number stored at the location is used to determine 
	how many particles need to be read from the buffer. Therefore it is critical to set the value
	numParticlesPtr is pointing to, to the correct number of particles which are prepared to be added 
	to the simulation. Otherwise erroneous data gets processed which has undefined results.
	*/
	NxU32*					numParticlesPtr;

	/**
	\brief The pointer to the user-allocated buffer for particle positions.

	A position consists of three consecutive 32-bit floats. If set to NULL, positions are not read or written to.
	*/
	NxF32*					bufferPos;
	
	/**
	\brief The pointer to the user-allocated buffer for particle velocities.

	A velocity consists of three consecutive 32-bit 
	floats. If set to NULL, velocities are not read or written to.
	*/
	NxF32*					bufferVel;
	
	/**
	\brief The pointer to the user-allocated buffer for particle lifetimes.

	A particle lifetime consists of one 32-bit 
	float. If set to NULL, lifetimes are not read or written to.
	*/
	NxF32*					bufferLife;
    
	/**
	\brief The pointer to the user-allocated buffer for particle densities.

	A particle density consists of one 32-bit float. If set to NULL, densities are not written to.
	Densities are never read from the user, they are always defined by the fluid simulation.
	*/
	NxF32*					bufferDensity;

	/**
	\brief The pointer to the user-allocated buffer for particle IDs.

	A particle id is represented as a 32-bit unsigned integer. If set to NULL, IDs are not written to.
	IDs are never read from the user, they are always defined by the SDK.
	*/
	NxU32*					bufferId;

	/**
	\brief The pointer to the user-allocated buffer for particle flags.

	A particle flags are represented as a 32-bit unsigned integer. If set to NULL, flags are not written to.
	Flags are never read from the user, they are always defined by the SDK. 
	Use NxParticleFlag to interpret this data.
	*/
	NxU32*					bufferFlag;

	/**
	\brief The pointer to the user-allocated buffer for particle collision normals.

	A collision normal consists of three consecutive 32-bit 
	floats. If set to NULL, normals are not read or written to.
	
	Limitation: Collision normals can only be received if 
	NxFluidDesc.flags.NX_FF_COLLISION_TWOWAY is NOT set.
	*/
	NxF32*					bufferCollisionNormal;

	/**
	\brief The separation (in bytes) between consecutive particle positions.

	The position of the first particle is found at location <tt>bufferPos</tt>;
	the second is at <tt>bufferPos + bufferPosByteStride</tt>;
	and so on.
	*/
	NxU32					bufferPosByteStride;

	/**
	\brief The separation (in bytes) between consecutive particle velocities.

	The velocity of the first particle is found at location <tt>bufferVel</tt>;
	the second is at <tt>bufferVel + bufferVelByteStride</tt>;
	and so on.
	*/
	NxU32					bufferVelByteStride;

	/**
	\brief The separation (in bytes) between consecutive particle lifetimes.

	The lifetime of the first particle is found at location <tt>bufferLife</tt>;
	the second is at <tt>bufferLife + bufferLifeByteStride</tt>;
	and so on.
	*/
	NxU32					bufferLifeByteStride;

	/**
	\brief The separation (in bytes) between consecutive particle densities.

	The density of the first particle is found at location <tt>bufferDensity</tt>;
	the second is at <tt>bufferDensity + bufferDensityByteStride</tt>;
	and so on.
	*/
	NxU32					bufferDensityByteStride;

	/**
	\brief The separation (in bytes) between consecutive particle IDs.

	The ID of the first particle is found at location <tt>bufferId</tt>;
	the second is at <tt>bufferId + bufferIdByteStride</tt>;
	and so on.
	*/
	NxU32					bufferIdByteStride;

	/**
	\brief The separation (in bytes) between consecutive particle flags.

	The flags of the first particle is found at location <tt>bufferFlag</tt>;
	the second is at <tt>bufferFlag + bufferFlagByteStride</tt>;
	and so on.
	*/
	NxU32					bufferFlagByteStride;

	/**
	\brief The separation (in bytes) between consecutive particle collision normals.

	The normal of the first particle is found at location <tt>bufferCollisionNormal</tt>;
	the second is at <tt>bufferCollisionNormal + bufferCollisionNormalByteStride</tt>;
	and so on.
	*/
	NxU32					bufferCollisionNormalByteStride;

	const char*				name;			//!< Possible debug name. The string is not copied by the SDK, only the pointer is stored.

	NX_INLINE ~NxParticleData();
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
	NX_INLINE	NxParticleData();
	};

NX_INLINE NxParticleData::NxParticleData()
	{
	setToDefault();
	}

NX_INLINE NxParticleData::~NxParticleData()
	{
	}

NX_INLINE void NxParticleData::setToDefault()
	{
	numParticlesPtr						= NULL;
	bufferPos							= NULL;
	bufferVel							= NULL;
	bufferLife							= NULL;
	bufferDensity						= NULL;
	bufferId							= NULL;
	bufferFlag							= NULL;
	bufferCollisionNormal				= NULL;
	bufferPosByteStride					= 0;
	bufferVelByteStride					= 0;
	bufferLifeByteStride				= 0;
	bufferDensityByteStride				= 0;
	bufferIdByteStride					= 0;
	bufferFlagByteStride				= 0;
	bufferCollisionNormalByteStride		= 0;	
	name								= NULL;
	}

NX_INLINE bool NxParticleData::isValid() const
	{
	if (numParticlesPtr && !(bufferPos || bufferVel || bufferLife || bufferDensity || bufferId || bufferCollisionNormal)) return false;
	if ((bufferPos || bufferVel || bufferLife || bufferDensity || bufferId || bufferCollisionNormal) && !numParticlesPtr) return false;
	if (bufferPos && !bufferPosByteStride) return false;
	if (bufferVel && !bufferVelByteStride) return false;
	if (bufferLife && !bufferLifeByteStride) return false;
	if (bufferDensity && !bufferDensityByteStride) return false;
	if (bufferId && !bufferIdByteStride) return false;
    if (bufferFlag && !bufferFlagByteStride) return false;
	if (bufferCollisionNormal && !bufferCollisionNormalByteStride) return false;
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

