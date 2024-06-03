#ifndef NX_FLUIDS_NXFLUIDACTORDESC
#define NX_FLUIDS_NXFLUIDACTORDESC
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

#include "NxMeshData.h"
#include "fluids/NxParticleData.h"
#include "fluids/NxParticleIdData.h"
#include "fluids/NxParticleUpdateData.h"
#include "fluids/NxFluidPacketData.h"
#include "fluids/NxFluidEmitterDesc.h"

#include "NxSceneDesc.h"

#include "NxArray.h"
#include "NxPlane.h"

class NxCompartment;

enum NxFluidDescType
{
	NX_FDT_DEFAULT,
	NX_FDT_ALLOCATOR
};

/**
\brief Describes the particle simulation method

Particles can be treated in two ways: either they are simulated considering
interparticular forces (SPH), or they are simulated independently.
In the latter case (with the simulation method set to NX_F_NO_PARTICLE_INTERACTION),
you still get collision between particles and static/dynamic shapes, damping,
acceleration due to gravity, and the user force.
*/
enum NxFluidSimulationMethod
	{
	/**
	\brief Enable simulation of inter particle forces.
	*/
	NX_F_SPH						= (1<<0),	

	/**
	\brief Do not simulate inter particle forces.
	*/
	NX_F_NO_PARTICLE_INTERACTION	= (1<<1),

	/**
	\brief Alternate between SPH and simple particles
	*/
	NX_F_MIXED_MODE					= (1<<2),
	};

/**
\brief The fluid collision method

The NxFluid instance can be selected for collision with both static and dynamic shapes.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : No
\li XB360: No
*/
enum NxFluidCollisionMethod
	{
	NX_F_STATIC					= (1<<0),	
	NX_F_DYNAMIC 				= (1<<1),
	};

/**
\brief Fluid flags
*/
enum NxFluidFlag
	{
	/**
	\brief Enables debug visualization for the NxFluid.
	*/
	NX_FF_VISUALIZATION							= (1<<0),

	/**
	\brief Disables scene gravity for the NxFluid.
	*/
	NX_FF_DISABLE_GRAVITY						= (1<<1),

	/**
	\brief Enable/disable two way collision of fluid with the rigid body scene.
	In either case, fluid is influenced by colliding rigid bodies.
	If NX_FF_COLLISION_TWOWAY is not set, rigid bodies are not influenced by 
	colliding pieces of fluid. Use NxFluidDesc.collisionResponseCoefficient to
	control the strength of the feedback force on rigid bodies.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluieDesc.collisionResponseCoefficient
	*/
	NX_FF_COLLISION_TWOWAY						= (1<<2),


	/**
	\brief Enable/disable execution of fluid simulation.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_FF_ENABLED								= (1<<3),

	/**
	\brief Defines whether this fluid is simulated on the PPU.
	*/
	NX_FF_HARDWARE								= (1<<4),

	/**
	\brief Enable/disable particle priority mode. 
	If enabled, the oldest particles are deleted to keep a certain budget for 
	new particles. Note that particles which have equal lifetime can get deleted 
	at the same time. In order to avoid this, the particle lifetimes 
	can be varied randomly.

	@see NxFluidDesc.numReserveParticles
	*/
	NX_FF_PRIORITY_MODE							= (1<<5),

	/**
	\brief Defines whether the particles of this fluid should be projected to a plane.
	This can be used to build 2D fluid applications, for instance. The projection
	plane is defined by the parameter NxFluidDesc.projectionPlane.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.projectionPlane
	*/
	NX_FF_PROJECT_TO_PLANE						= (1<<6),

	/**
	\brief Forces fluid static mesh cooking format to parameters given by the fluid descriptor.

	Currently not implemented!
	*/
	NX_FF_FORCE_STRICT_COOKING_FORMAT			= (1<<7),

	};

/**
\brief Describes an NxFluid.
*/
class NxFluidDescBase
	{
	public:

	/**
	\brief 	Describes the particles which are added to the fluid initially.

	The pointers to the buffers are invalidated after the initial particles are generated.

	@see NxParticleData
	*/
	NxParticleData				initialParticleData;
	
	/**
	\brief Sets the maximal number of particles for the fluid used in the simulation.
	
	If more particles are added directly, or more particles are emitted into the
	fluid after this limit is reached, they are simply ignored.

	*/
	NxU32						maxParticles;

	/**
	\brief Defines the number of particles which are reserved for creation at runtime.
    	
	If NxFluidDesc.flags.NX_FF_PRIORITY_MODE is set the oldest particles are removed until 
	there are no more than (maxParticles - numReserveParticles) particles left. This removal
	is carried out for each simulation step, on particles which have a finite life time 
	(i.e. > 0.0). The deletion guarantees a reserve of numReserveParticles particles which 
	can be added for each simulaiton step. Note that particles which have equal lifetime can 
	get deleted at the same time. In order to avoid this, the particle lifetimes 
	can be varied randomly.

	This parameter must be smaller than NxFluidDesc.maxParticles.
	*/
	NxU32						numReserveParticles;

	/**
	\brief The particle resolution given as particles per linear meter measured when the fluid is
	in its rest state (relaxed).
	
	Even if the particle system is simulated without particle interactions, this parameter defines the 
	emission density of the emitters.

	*/
	NxReal						restParticlesPerMeter;

	/**
	\brief Target density for the fluid (water is about 1000).
	
	Even if the particle system is simulated without particle interactions, this parameter defines 
	indirectly in combination with restParticlesPerMeter the mass of one particle 
	( mass = restDensity/(restParticlesPerMeter^3) ).
	
	The particle mass has an impact on the repulsion effect on emitters and actors.

	*/
	NxReal						restDensity;

	/**
	\brief Radius of sphere of influence for particle interaction.
	
	This parameter is relative to the rest spacing of the particles, which is defined by the parameter
	restParticlesPerMeter
	
	( radius = kernelRadiusMultiplier/restParticlesPerMeter ).

	This parameter should be set around 2.0 and definitely below 2.5 for optimal performance and simulation quality.

	@see restParticlesPerMeter
	*/
	NxReal						kernelRadiusMultiplier;

	/**
	\brief Maximal distance a particle is allowed to travel within one timestep.

	This parameter is relative to the rest spacing of the particles, which is defined by the parameter
	restParticlesPerMeter:

	( maximal travel distance = motionLimitMultiplier/restParticlesPerMeter ).

	Default value is 3.6 (i.e., 3.0 * kernelRadiusMultiplier).

	The value must not be higher than the product of packetSizeMultiplier and kernelRadiusMultiplier.

	@see restParticlesPerMeter
	*/
	NxReal						motionLimitMultiplier;

	/**
	\brief Defines the distance between particles and collision geometry, which is maintained during simulation.

	For the actual distance, this parameter is divided by the rest spacing of the particles, which is defined by the parameter
	restParticlesPerMeter:

	( distance = collisionDistanceMultiplier/restParticlesPerMeter ).

	It has to be positive and not higher than packetSizeMultiplier*kernelRadiusMultiplier.
	Default value is 0.12 (i.e., 0.1 * kernelRadiusMultiplier).

	@see restParticlesPerMeter, kernelRadiusMultiplier
	*/
	NxReal						collisionDistanceMultiplier;

	/**
	\brief This parameter controls the parallelization of the fluid.
	
	The spatial domain is divided into so called packets, equal sized cubes, aligned in a grid.
	
	The parameter given defines the edge length of such a packet. This parameter is relative to the interaction 
	radius of the particles, given as kernelRadiusMultiplier/restParticlesPerMeter.

	It has to be a power of two, no less than 4.

	*/
	NxU32						packetSizeMultiplier;

	/**
	\brief The stiffness of the particle interaction related to the pressure.
	
	This factor linearly scales the force which acts on particles which are closer to each other than 
	the rest spacing.
	
	Setting	this parameter appropriately is crucial for the simulation.  The right value depends on many factors
	such as viscosity, damping, and kernelRadiusMultiplier.  Values which are too high will result in an
	unstable simulation, whereas too low values will make the fluid appear "springy" (the fluid
	acts more compressible).

	Must be positive.

	*/
	NxReal						stiffness;

	/**
	\brief 	The viscosity of the fluid defines its viscous behavior.
	
	Higher values will result in a honey-like behavior.  Viscosity is an effect which depends on the 
	relative velocity of neighboring particles; it reduces the magnitude of the relative velocity.
	
	Must be positive.

	*/
	NxReal						viscosity;

	/**
	\brief 	The surfaceTension of the fluid defines an attractive force between particles
	
	Higher values will result in smoother surfaces.
	Must be nonnegative.

	*/
	NxReal						surfaceTension;

	/**
	\brief Velocity damping constant, which is globally applied to each particle.
	
	It generally reduces the velocity of the particles. Setting the damping to 0 will leave the 
	particles unaffected.

	Must be nonnegative.

	*/
	NxReal						damping;

	/**
	\brief Defines a timespan for the particle "fade-in".

	This feature is experimental. When a particle is created it has no influence on the simulation. 
	It takes fadeInTime until the particle has full influence. If set to zero, the particle has full 
	influence on the simulation from start.

	<b>Default:</b> 0.0 <br>
	<b>Range:</b> [0,inf)
	*/
	NxReal						fadeInTime;

	/**
	\brief Acceleration (m/s^2) applied to all particles at all time steps.

	Useful to simulate smoke or fire.
	This acceleration is additive to the scene gravity. The scene gravity can be turned off 
	for the fluid, using the flag NX_FF_DISABLE_GRAVITY.

	@see NxFluid.getExternalAcceleration() NxFluid.setExternalAcceleration()
	*/
	NxVec3						externalAcceleration;

	/**
	\brief Defines the plane the fluid particles are projected to. This parameter is only used if
	NX_FF_PROJECT_TO_PLANE is set.

	<b>Default:</b> XY plane

	@see NX_FF_PROJECT_TO_PLANE NxFluid.getProjectionPlane() NxFluid.setProjectionPlane()
	*/
	NxPlane						projectionPlane;

	/**
	\brief Defines the restitution coefficient used for collisions of the fluid particles with static shapes.

	Must be between 0 and 1.
	
	A value of 0 causes the colliding particle to get a zero velocity component in the
	direction of the surface normal of the static shape at the collision location; i.e.
	it will not bounce.
	
	A value of 1 causes a particle's velocity component in the direction of the surface normal to invert;
	i.e. the particle bounces off the surface with the same velocity magnitude as it had before collision. 
	(Caution: values near 1 may have a negative impact on stability)

	*/
	NxReal						restitutionForStaticShapes;
	
	/**
	\brief Defines the dynamic friction of the fluid regarding the surface of a static shape.

	Must be between 0 and 1.
	
	A value of 1 will cause the particle to lose its velocity tangential to
	the surface normal of the shape at the collision location; i.e. it will not slide
	along the surface.
	
	A value of 0 will preserve the particle's velocity in the tangential surface
	direction; i.e. it will slide without resistance on the surface.

	*/
	NxReal						dynamicFrictionForStaticShapes;
	
	/**
	\brief Defines the static friction of the fluid regarding the surface of a static shape.

	This feature is currently unimplemented! 

	*/
	NxReal						staticFrictionForStaticShapes;

	/**
	\brief Defines the strength of attraction between the particles and static rigid bodies on collision. 

	This feature is currently unimplemented! 

	*/
	NxReal						attractionForStaticShapes;

	/**
	\brief Defines the restitution coefficient used for collisions of the fluid particles with dynamic shapes.
	
	Must be between 0 and 1.

	(Caution: values near 1 may have a negative impact on stability)

	@see restitutionForStaticShapes
	*/
	NxReal						restitutionForDynamicShapes;
	
	/**
	\brief Defines the dynamic friction of the fluid regarding the surface of a dynamic shape.

	Must be between 0 and 1.

	@see dynamicFrictionForStaticShapes
	*/
	NxReal						dynamicFrictionForDynamicShapes;

	/**
	\brief Defines the static friction of the fluid regarding the surface of a dynamic shape.

	This feature is currently unimplemented! 

	*/
	NxReal						staticFrictionForDynamicShapes;

	/**
	\brief Defines the strength of attraction between the particles and the dynamic rigid bodies on collision. 

	This feature is currently unimplemented! 

	*/
	NxReal						attractionForDynamicShapes;

	/**
	\brief Defines a factor for the impulse transfer from fluid to colliding rigid bodies.

	Only has an effect if NX_FF_COLLISION_TWOWAY is set.

	<b>Default:</b> 0.2 <br>
	<b>Range:</b> [0,inf)

	@see NX_FF_COLLISION_TWOWAY NxFluid.setCollisionResponseCoefficient()
	*/
	NxReal						collisionResponseCoefficient;

	/**
	\brief NxFluidSimulationMethod flags. Defines whether or not particle interactions are considered 
	in the simulation.

	@see NxFluidSimulationMethod
	*/
	NxU32						simulationMethod;

	/**
	\brief NxFluidCollisionMethod flags. Selects whether static collision and/or dynamic collision 
	with the environment is performed.
	
	@see NxFluidCollisionMethod
	*/
	NxU32						collisionMethod;

	/**
	\brief Sets which collision group this fluid is part of.

	<b>Default:</b> 0

	NxFluid.setCollisionGroup()
	*/
	NxCollisionGroup collisionGroup;


	/**
	\brief Sets the 128-bit mask used for collision filtering.

	<b>Default:</b> 0

	@see NxGroupsMask NxFluid.setGroupsMask() NxFluid.getGroupsMask()
	*/
	NxGroupsMask groupsMask;

	/**
	\brief Force Field Material Index, index != 0 has to be created.

	<b>Default:</b> 0
	*/
	NxForceFieldMaterial		forceFieldMaterial;

	/**
	\brief Defines the user data buffers which are used to store particle data, which can be used for rendering.

	@see NxParticleData
	*/
	NxParticleData				particlesWriteData;

	/**
	\brief Defines the user data buffer which is used to store IDs of deleted particles.

	@see NxParticleIdData
	*/
	NxParticleIdData			particleDeletionIdWriteData;

	/**
	\brief Defines the user data buffer which is used to store IDs of created particles.

	@see NxParticleIdData
	*/
	NxParticleIdData			particleCreationIdWriteData;

	/**
	\brief Defines the user data buffer which is used to store fluid packets.

	@see NxFluidPacketData
	*/
	NxFluidPacketData			fluidPacketData;
	
	/**
	\brief Flags defining certain properties of the fluid.

	@see NxFluidFlag
	*/
	NxU32						flags;

	void*						userData;	//!< Will be copied to NxFluid::userData
	const char*					name;		//!< Possible debug name. The string is not copied by the SDK, only the pointer is stored.

	/**
	\brief The compartment to place the fluid in. Must be either a pointer to an NxCompartment of type NX_SCT_FLUID, or NULL.
	A NULL compartment means creating the fluid in the first available fluid compartment (a default fluid compartment is created if none exists).

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	<b>Default:</b> NULL
	*/
	NxCompartment *				compartment;

	/**
	\brief Constructor sets to default.

	*/
	NX_INLINE NxFluidDescBase();	
	/**
	\brief (Re)sets the structure to the default.	

	*/
	NX_INLINE void setToDefault();
	/**
	\brief Returns true if the current settings are valid

	*/
	NX_INLINE bool isValid() const;


	/**
	\brief Retrieve the fluid desc type.

	\return The fluid desc type. See #NxFluidDescType
	*/
	NX_INLINE NxFluidDescType getType() const;

	protected:

	NxFluidDescType			type;

	};

/**
\brief Fluid Descriptor. This structure is used to save and load the state of #NxFluid objects.

Legacy implementation that works with existing code but does not permit the user
to supply his own allocator for NxArray<NxEmitterDesc*>	emitters.
*/
class NxFluidDesc : public NxFluidDescBase
	{
	public: 

	/**
	\brief Array of emitter descriptors that describe emitters which emit fluid into this
	fluid actor.

	A fluid actor can have any number of emitters.

	@see NxFluidEmitter
	*/
	NxArray<NxFluidEmitterDesc, NxAllocatorDefault> emitters;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxFluidDesc();

	/**
	\brief (Re)sets the structure to the default.	

	*/
	NX_INLINE void setToDefault();
	/**
	\brief Returns true if the current settings are valid

	*/
	NX_INLINE bool isValid() const;
	};

/**
\brief Implementation of an actor descriptor that permits the user to supply his own allocator
*/
template<class AllocType = NxAllocatorDefault> class NxFluidDesc_Template : public NxFluidDescBase
	{
	public: 
	/**
	\brief Array of emitter descriptors that describe emitters which emit fluid into this
	fluid actor.

	A fluid actor can have any number of emitters.

	@see NxFluidEmitter
	*/
	NxArray<NxFluidEmitterDesc, AllocType> emitters;

	NX_INLINE NxFluidDesc_Template()
		{
		setToDefault();
		type = NX_FDT_ALLOCATOR;
		}

	NX_INLINE void setToDefault()
		{
		NxFluidDescBase::setToDefault();
		emitters.clear();
		}

	NX_INLINE bool isValid() const
		{
		if (!NxFluidDescBase::isValid())
			return false;

		if (emitters.size() > 0xffff) return false;

		for (unsigned i = 0; i < emitters.size(); i++)
			if (!emitters[i].isValid()) return false;

		return true;
		}

	};


NX_INLINE NxFluidDescBase::NxFluidDescBase()
	{
	//nothing!  Don't call setToDefault() here!
	}

NX_INLINE void NxFluidDescBase::setToDefault()
	{
	maxParticles				= 32767;
	numReserveParticles			= 0;
	restParticlesPerMeter		= 50.0f;
	restDensity					= 1000.0f;
	kernelRadiusMultiplier		= 1.2f;
	motionLimitMultiplier		= 3.0f * kernelRadiusMultiplier;
	collisionDistanceMultiplier = 0.1f * kernelRadiusMultiplier;
	packetSizeMultiplier		= 16;
	stiffness					= 20.0f;
	viscosity					= 6.0f;
	surfaceTension				= 0.0f;
	damping						= 0.0f;
	fadeInTime					= 0.0f;
	externalAcceleration.zero();
	projectionPlane.set(NxVec3(0.0f, 0.0f, 1.0f), 0.0f);
	restitutionForStaticShapes	= 0.5f;
	dynamicFrictionForStaticShapes = 0.05f;
	staticFrictionForStaticShapes = 0.05f;
	attractionForStaticShapes	= 0.0f;
	restitutionForDynamicShapes	= 0.5f;
	dynamicFrictionForDynamicShapes = 0.5f;
	staticFrictionForDynamicShapes = 0.5f;
	attractionForDynamicShapes	= 0.0f;
	collisionResponseCoefficient = 0.2f;

	simulationMethod			= NX_F_SPH;
	collisionMethod				= NX_F_STATIC|NX_F_DYNAMIC;
	collisionGroup				= 0;	
	forceFieldMaterial			= 0;
	groupsMask.bits0 = 0;
	groupsMask.bits1 = 0;
	groupsMask.bits2 = 0;
	groupsMask.bits3 = 0;

	particlesWriteData			.setToDefault();
	particleDeletionIdWriteData .setToDefault();
	particleCreationIdWriteData .setToDefault();
	fluidPacketData				.setToDefault();

	flags						= NX_FF_VISUALIZATION|NX_FF_ENABLED|NX_FF_HARDWARE;

	userData					= NULL;
	name						= NULL;
	compartment					= NULL;
	}

NX_INLINE bool NxFluidDescBase::isValid() const
	{
	if (kernelRadiusMultiplier < 1.0f) return false;
	if (restDensity <= 0.0f) return false;
	if (restParticlesPerMeter <= 0.0f) return false;

	if (packetSizeMultiplier < 4) return false;
	if (packetSizeMultiplier & ( packetSizeMultiplier - 1 ) ) return false; 

	if (motionLimitMultiplier <= 0.0f) return false;
	if (motionLimitMultiplier > packetSizeMultiplier*kernelRadiusMultiplier) return false;

	if (collisionDistanceMultiplier <= 0.0f) return false;
	if (collisionDistanceMultiplier > packetSizeMultiplier*kernelRadiusMultiplier) return false;

	if (stiffness <= 0.0f) return false;
	if (viscosity <= 0.0f) return false;
	if (surfaceTension < 0.0f) return false;

	bool isNoInteraction = (simulationMethod & NX_F_NO_PARTICLE_INTERACTION) > 0;
	bool isSPH = (simulationMethod & NX_F_SPH) > 0;
	bool isMixed = (simulationMethod & NX_F_MIXED_MODE) > 0;
	if (!(isNoInteraction || isSPH || isMixed)) return false;
	if (isNoInteraction && (isSPH || isMixed)) return false;
	if (isSPH && (isNoInteraction || isMixed)) return false;
	if (isMixed && (isNoInteraction || isSPH)) return false;
	
	if (damping < 0.0f) return false;
	if (fadeInTime < 0.0f) return false;

	if (projectionPlane.normal.isZero()) return false;

	if (dynamicFrictionForDynamicShapes < 0.0f || dynamicFrictionForDynamicShapes > 1.0f) return false;
	if (staticFrictionForDynamicShapes < 0.0f || staticFrictionForDynamicShapes > 1.0f) return false;
	if (restitutionForDynamicShapes < 0.0f || restitutionForDynamicShapes > 1.0f) return false;
	if (attractionForDynamicShapes < 0.0f) return false;
	if (dynamicFrictionForStaticShapes < 0.0f || dynamicFrictionForStaticShapes > 1.0f) return false;
	if (staticFrictionForStaticShapes < 0.0f || staticFrictionForStaticShapes > 1.0f) return false;
	if (restitutionForStaticShapes < 0.0f || restitutionForStaticShapes > 1.0f) return false;
	if (attractionForStaticShapes < 0.0f) return false;
	if (collisionResponseCoefficient < 0.0f) return false;
	
	if (!initialParticleData.isValid()) return false;
	if (!particlesWriteData.isValid()) return false;
	if (!particleDeletionIdWriteData.isValid()) return false;
	if (!particleCreationIdWriteData.isValid()) return false;
	if (!fluidPacketData.isValid()) return false;
	
	if (maxParticles >= (1<<16)) return false;
	if (maxParticles < 1) return false;
	
	if (numReserveParticles >= maxParticles) return false;

	if(collisionGroup >= 32) return false; // We only support 32 different collision groups

	return true;
	}

NX_INLINE NxFluidDescType NxFluidDescBase::getType() const			
	{	
		return type; 
	}

NX_INLINE NxFluidDesc::NxFluidDesc()
	{
		memset(this,0,sizeof(NxFluidDesc));
		setToDefault();
		type = NX_FDT_DEFAULT;
	}

NX_INLINE void NxFluidDesc::setToDefault()
	{
		NxFluidDescBase::setToDefault();
		emitters		.clear();
	}

NX_INLINE bool NxFluidDesc::isValid() const
	{
		if (!NxFluidDescBase::isValid())
			return false;

		if (emitters.size() > 0xffff) return false;

		for (unsigned i = 0; i < emitters.size(); i++)
			if (!emitters[i].isValid()) return false;

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

