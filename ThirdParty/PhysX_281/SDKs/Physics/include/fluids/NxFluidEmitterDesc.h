#ifndef NX_FLUIDS_NXFLUIDEMITTERDESC
#define NX_FLUIDS_NXFLUIDEMITTERDESC
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
\brief Flags which control the behavior of fluid emitters.

@see NxFluidEmitter
*/
enum NxFluidEmitterFlag
	{
	/**
	\brief Flags whether the emitter should be visualized for debugging or not.
	*/
	NX_FEF_VISUALIZATION		= (1<<0),

	/**
	\brief This flag specifies whether the emission should cause a force on 
	the shapes body that the emitter is attached to.
	*/
	NX_FEF_FORCE_ON_BODY		= (1<<2),
	
	/**
	\brief If set, the velocity of the shapes body is added to the emitted particle velocity.
	
	This is the default behaviour.
	*/
	NX_FEF_ADD_BODY_VELOCITY	= (1<<3),
	
	/**
	\brief Flag to start and stop the emission. On default the emission is enabled.
	*/
	NX_FEF_ENABLED				= (1<<4),
	};

/**
\brief Flags to specify the shape of the area of emission.

Exactly one flag should be set at any time.

*/
enum NxEmitterShape
	{
	NX_FE_RECTANGULAR		= (1<<0),
	NX_FE_ELLIPSE			= (1<<1)
	};

/**
\brief Flags to specify the emitter's type of operation.
Exactly one flag should be set at any time.

@see NxFluidEmitter
*/
enum NxEmitterType
	{
	NX_FE_CONSTANT_PRESSURE		= (1<<0),
	NX_FE_CONSTANT_FLOW_RATE	= (1<<1)
	};

#include "fluids/NxFluidEmitter.h"

/**
\brief Descriptor for NxFluidEmitter class.  Used for saving and loading the emitter state.
*/
class NxFluidEmitterDesc
	{
	public:

	/**
	\brief The emitter's pose relative to the frameShape.

	relPose is relative to the shape's frame. If frameShape is NULL then relPose is relative to the world frame.

	The direction of the flow is the direction of the third (z) axis of the emitter frame.
	*/
    NxMat34					relPose;
	
	/**
	\brief A pointer to the NxShape to which the emitter is attached to.
	
	If this pointer is set to NULL, the emitter is attached to the world frame. The shape 
	must be in the same scene as the emitter. 
	*/
	NxShape*				frameShape;

	/**
	\brief The emitter's mode of operation.

	Either the simulation enforces constant pressure or constant flow rate at the emission site, 
	given the velocity of emitted particles.

	@see NxEmitterType
	*/
	NxU32					type;

	/**
	\brief The maximum number of particles which are emitted from this emitter.
	
	If the total number of particles in the fluid already hit the maxParticles parameter of the fluid, 
	this maximal values can't be reached.
	
	If set to 0, the number of emitted particles is unrestricted.
	*/
	NxU32					maxParticles;

	/**
	\brief The emitter's shape can either be rectangular or elliptical.

	@see NxEmitterShape
	*/
	NxU32					shape;

	/**
	\brief The sizes of the emitter in the directions of the first and the second axis of its orientation 
	frame (relPose).

	The dimensions are actually the radii of the size.

	*/
	NxReal					dimensionX;
	NxReal					dimensionY;

	/**
	\brief Random vector with values for each axis direction of the emitter orientation.
	
	The values have to be positive and describe the maximal random particle displacement in each dimension.
	
	The z value describes the randomization in emission direction. The emission direction 
	is specified by the third orientation axis of relPose.

	*/
    NxVec3					randomPos;

	/**
	\brief Random angle deviation from emission direction.
	
	The emission direction is specified by the third orientation axis of relPose.
	
	<b>Unit:</b> Radians

	*/
	NxReal					randomAngle;

	/**
	\brief The velocity magnitude of the emitted fluid particles.

	*/
	NxReal					fluidVelocityMagnitude;

	/**
	\brief The rate specifies how many particles are emitted per second.
	
	The rate is only considered in the simulation if the type is set to NX_FE_CONSTANT_FLOW_RATE.

	@see NxEmitterType
	*/
	NxReal					rate;
	
	/**
	\brief This specifies the time in seconds an emitted particle lives.

	If set to 0, each particle will live until it collides with a drain.
	*/
	NxReal					particleLifetime;

	/**
	\brief Defines a factor for the impulse transfer from attached emitter to body.

	Only has an effect if NX_FEF_FORCE_ON_BODY is set.

	<b>Default:</b> 1.0 <br>
	<b>Range:</b> [0,inf)

	@see NX_FEF_FORCE_ON_BODY NxFluidEmitter.setRepulsionCoefficient()
	*/
	NxReal					repulsionCoefficient;

	/**
	\brief A combination of NxFluidEmitterFlags.

	@see NxFluidEmitterFlag
	*/
	NxU32					flags;

	void*					userData;		//!< Will be copied to NxShape::userData.
	const char*				name;			//!< Possible debug name. The string is not copied by the SDK, only the pointer is stored.

	NX_INLINE ~NxFluidEmitterDesc();
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
	NX_INLINE	NxFluidEmitterDesc();
	};


NX_INLINE NxFluidEmitterDesc::NxFluidEmitterDesc()
	{
	setToDefault();
	}

NX_INLINE NxFluidEmitterDesc::~NxFluidEmitterDesc()
	{
	}

NX_INLINE void NxFluidEmitterDesc::setToDefault()
	{
    relPose							.id();
    frameShape						= NULL;
	type							= NX_FE_CONSTANT_PRESSURE;
	maxParticles					= 0;
	shape							= NX_FE_RECTANGULAR;
	dimensionX						= 0.25f;
	dimensionY						= 0.25f;
	randomPos						.zero();
	randomAngle						= 0.0f;
	fluidVelocityMagnitude			= 1.0f;
	rate							= 100.0f;
	particleLifetime				= 0.0f;
	repulsionCoefficient			= 1.0f;
	flags							= NX_FEF_ENABLED|NX_FEF_VISUALIZATION|NX_FEF_ADD_BODY_VELOCITY;
	
	userData						= NULL;
	name							= NULL;
	}

NX_INLINE bool NxFluidEmitterDesc::isValid() const
	{
	if (!relPose.isFinite()) return false;
	
	if (dimensionX < 0.0f) return false;
	if (dimensionY < 0.0f) return false;

	if (randomPos.x < 0.0f) return false;
	if (randomPos.y < 0.0f) return false;
	if (randomPos.z < 0.0f) return false;
	if (!randomPos.isFinite()) return false;

	if (randomAngle < 0.0f) return false;

	if (!(((shape & NX_FE_ELLIPSE) > 0) ^ ((shape & NX_FE_RECTANGULAR) > 0))) return false;
	if (!(((type & NX_FE_CONSTANT_FLOW_RATE) > 0) ^ ((type & NX_FE_CONSTANT_PRESSURE) > 0))) return false;

	if (rate < 0.0f) return false;
	if (fluidVelocityMagnitude < 0.0f) return false;
	if (particleLifetime < 0.0f) return false;
	if (repulsionCoefficient < 0.0f) return false;

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

