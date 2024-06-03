#ifndef NX_PHYSICS_NX_FLUIDACTOR
#define NX_PHYSICS_NX_FLUIDACTOR
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
#include "NxArray.h"
#include "NxBounds3.h"
#include "NxFluidDesc.h"
#include "NxPhysicsSDK.h"

class NxFluidEmitterDesc;
class NxFluidEmitter;
class NxCompartment;

/**
\brief The fluid class represents the main module for the particle based fluid simulation.
SPH (Smoothed Particle Hydrodynamics) is used to animate the particles.

There are two kinds of particle interaction forces which govern the behaviour of the fluid:
<ol>
<li>
	Pressure forces: These forces result from particle densities higher than the
	"rest density" of the fluid.  The rest density is given by specifying the inter-particle
	distance at which the fluid is in its relaxed state.  Particles which are closer than
	the rest spacing are pushed away from each other.
<li>
	Viscosity forces:  These forces act on neighboring particles depending on the difference
	of their velocities.  Particles drag other particles with them which is used to simulate the
	viscous behaviour of the fluid.
</ol>
The fluid class manages a set of particles.
Particles can be created in two ways:
<ol>
<li>
	Particles can be added by the user directly.
<li>
	The user can add emitters to the fluid and configure the parameters of the emission.
	(See NxFluidEmitter)
</ol>
Particles can be removed in two ways as well:
<ol>
<li>
	The user can specify a lifetime for the particles.  When its lifetime expires, a particle is deleted.
<li>
	Shapes can be configured to act as drains.  When a particle intersects with a drain, the particle is deleted.
	(See NxShapeFlag)
</ol>
Particles collide with static and dynamic shapes. Particles are also affected by the scene gravity and a user force, 
as well as global velocity damping.  In order to render a fluid, the user can supply the fluid instance with a 
user buffer into which the particle state is written after each simuation step.

For a good introduction to SPH fluid simulation,
see http://graphics.ethz.ch/~mattmuel/publications/sca03.pdf

@see NxFluidDesc, NxFluidEmitter, NxFluidEmitterDesc, NxMeshData, NxParticleData, NxShapeFlag
*/
class NxFluid
	{
	protected:
	NX_INLINE					NxFluid() : userData(NULL)	{}
	virtual						~NxFluid()	{}

	public:

/************************************************************************************************/

/** @name Emitters
*/
//@{

	/**
	\brief Creates an emitter for this fluid.
	
	NxFluidEmitterDesc::isValid() must return true.

	\param desc The fluid emitter desciptor. See #NxFluidEmitterDesc.
	\return The new fluid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitter
	*/
	virtual		NxFluidEmitter*		createEmitter(const NxFluidEmitterDesc& desc)				= 0;

	/**
	\brief Deletes the specified emitter.
	
	The emitter must belong to this fluid. Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param emitter The emitter to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		void				releaseEmitter(NxFluidEmitter& emitter)							= 0;

	/**
	\brief Returns the number of emitters.

	\return The number of emitters.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxU32				getNbEmitters()									const	= 0;

	/**
	\brief Returns an array of emitter pointers with size getNbEmitters().

	\return An array of fluid emitter pointers.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxFluidEmitter**	getEmitters()									const	= 0;
//@}

/** @name Particle Manipulation
*/
//@{

	/**
	\brief Adds particles to the simulation specified with the user buffer wrapper.
	
	The SDK only accesses the wrapped buffers until the function returns.
	The limit for adding particles is 4096 new particles per timestep.
	This method can be called several times. Each call writes particle ids to the 
	particle creation id buffer, if the user has registered one with either 
	NxFluidDesc::particleCreationIdWriteData or 
	NxFluid::setParticleCreationIdWriteData(const NxParticleIdData&). User particle and packet 
	buffers are also updated accordingly. By default particle ids get overwritten with each call. 
	Optionally the caller can choose to have the ids be appended instead. Note that the particle 
	creation id buffer gets always overwritten on NxScene::fetchResults().

	\param pData Structure describing the particles to add.
	\param appendIds selects particle appending or overwriting.
	\return number of successfully created particles. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParticleData
	*/
	virtual		NxU32 				addParticles(const NxParticleData& pData, bool appendIds = false)						= 0;

	/**
	\brief Removes all particles from the simulation.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		void				removeAllParticles()											= 0;

 	/**
	\brief Sets the wrapper for user buffers, which configure where particle data is written to.

	\param pData The descriptor for the buffers to write the particle data to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParticleData
	*/
	virtual		void 				setParticlesWriteData(const NxParticleData& pData)			= 0;

 	/**
	\brief Returns a copy of the wrapper which was set by setParticlesWriteData().

	\return The particle write data.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParticleData  The descriptor for the buffers to write the particle data to.
	*/
	virtual		NxParticleData 		getParticlesWriteData()							const	= 0;

 	/**
	\brief Sets the wrapper for user ID buffers, which configure where IDs of deleted particles are written to.

	\param iData The descriptor for the buffers to write the particle IDs to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParticleIdData
	*/
	virtual		void 				setParticleDeletionIdWriteData(const NxParticleIdData& iData)	= 0;

 	/**
	\brief Returns a copy of the wrapper which was set by setParticleDeletionIdWriteData().

	\return The particle ID write data.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParticleIdData  The descriptor for the buffers to write the particle IDs to.
	*/
	virtual		NxParticleIdData 		getParticleDeletionIdWriteData()						const	= 0;

 	/**
	\brief Sets the wrapper for user ID buffers, which configure where IDs of created particles are written to.

	\param iData The descriptor for the buffers to write the particle IDs to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParticleIdData
	*/
	virtual		void 				setParticleCreationIdWriteData(const NxParticleIdData& iData)	= 0;

 	/**
	\brief Returns a copy of the wrapper which was set by setParticleCreationIdWriteData().

	\return The particle ID write data.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParticleIdData  The descriptor for the buffers to write the particle IDs to.
	*/
	virtual		NxParticleIdData 	getParticleCreationIdWriteData()						const	= 0;

 	/**
	\brief Gets the number of particles which are reserved for creation at runtime.

	@see NxFluidDesc.numReserveParticles
	*/
	virtual		NxU32 				getNumReserveParticles()								const	= 0;

 	/**
	\brief Sets the number of particles which are reserved for creation at runtime.

	@see NxFluidDesc.numReserveParticles
	*/
	virtual		void 				setNumReserveParticles(NxU32)									= 0;


 	/**
	\brief Gets the bound on the maximum number of particles currently allowed in the fluid. 
	  
	@see setCurrentParticleLimit
	*/
	virtual		NxU32 				getCurrentParticleLimit()								const	= 0;

 	/**
	\brief Sets a bound on the maximum number of particles in the fluid.

	The value defaults to maxParticles, which is its maximum legal value. If it is lowered below
	the number of particles currently in the fluid, the oldest particles will be deleted to bring down 
	the number. This attribute is only effective if the NX_FF_PRIORITY_MODE flag is set on the fluid.

	*/
	virtual		void 				setCurrentParticleLimit(NxU32)									= 0;



	/**
	\brief Updates particles for one simulation frame.
	
	@see NxParticleUpdateData
	*/
	virtual		void				updateParticles(const NxParticleUpdateData&)					= 0;

	/**
	\brief Sets the wrapper for user buffers, which configure where fluid packet data is written to.

	\param pData The descriptor for the buffers to write the fluid packet data to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidPacketData
	*/
	virtual		void 				setFluidPacketData(const NxFluidPacketData& pData)			= 0;

	/**
	\brief Returns a copy of the wrapper which was set by setFluidPacketData().

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidPacketData
	*/
	virtual		NxFluidPacketData 		getFluidPacketData()								const	= 0;

//@}
/************************************************************************************************/

/** @name Fluid Parameters
*/
//@{

	/**
	\brief Returns the simulation method of the fluid.


	\return The simulation method.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidSimulationMethod
	*/
	virtual		NxU32				getSimulationMethod()									const	= 0;

	/**
	\brief Sets the simulation method of the fluid.

	Can either be set to NX_F_SPH, NX_F_NO_PARTICLE_INTERACTION or NX_F_MIXED_MODE.
	Note that depending on the spatial arrangement of the particles, switching from 
	NX_F_NO_PARTICLE_INTERACTION or NX_F_MIXED_MODE to NX_F_SPH might lead to an 
	unstable simulation state.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidSimulationMethod
	*/
	virtual		void				setSimulationMethod(NxU32 simMethod)							= 0;

	/**
	\brief Returns the fluid stiffness.

	\return The fluid stiffness.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.stiffness
	*/
	virtual		NxReal				getStiffness()									const	= 0;

	/**
	\brief Sets the fluid stiffness (must be positive).

	\param stiff The new fluid stiffness.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.stiffness
	*/
	virtual		void 				setStiffness(NxReal stiff)									= 0;

	/**
	\brief Returns the fluid viscosity.

	\return The viscosity  of the fluid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.viscosity
	*/
	virtual		NxReal				getViscosity()									const	= 0;

	/**
	\brief Sets the fluid viscosity (must be positive).

	\param visc The new viscosity of the fluid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.viscosity
	*/
	virtual		void 				setViscosity(NxReal visc)									= 0;

	/**
	\brief Returns the fluid surfaceTension.

	\return The surfaceTension  of the fluid.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxFluidDesc.surfaceTension
	*/
	virtual		NxReal				getSurfaceTension()								const	= 0;

	/**
	\brief Sets the fluid surfaceTension (must be nonnegative).

	\param surfaceTension The new surfaceTension of the fluid.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxFluidDesc.surfaceTension
	*/
	virtual		void 				setSurfaceTension(NxReal surfaceTension)				= 0;

	/**
	\brief Returns the fluid damping.

	\return The fluid damping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.damping
	*/
	virtual		NxReal				getDamping()									const	= 0;

	/**
	\brief Sets the fluid damping (must be nonnegative).

	\param damp The new fluid damping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.damping
	*/
	virtual		void 				setDamping(NxReal damp)										= 0;

	/**
	\brief Returns the fluid fadeInTime.

	\return The fluid fadeInTime.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	Experimental feature.

	@see NxFluidDesc.fadeInTime
	*/
	virtual		NxReal				getFadeInTime()										const	= 0;

	/**
	\brief Sets the fluid fadeInTime (must be nonnegative).

	\param fadeIn The new fadeInTime.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	Experimental feature.

	@see NxFluidDesc.fadeInTime
	*/
	virtual		void 				setFadeInTime(NxReal fadeIn)								= 0;

	/**
	\brief Returns the external acceleration applied to each particle at each time step.

	\return The external acceleration applied to particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.externalAcceleration
	*/
	virtual		NxVec3				getExternalAcceleration()								const	= 0;

	/**
	\brief Sets the external acceleration applied to each particle at each time step.

	\param acceleration External acceleration to apply to particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.externalAcceleration getExternalAcceleration()
	*/
	virtual		void 				setExternalAcceleration(const NxVec3&acceleration)				= 0;

	/**
	\brief Returns the plane the fluid particles are projected to.

	\return The particle projection plane.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NX_FF_PROJECT_TO_PLANE NxFluidDesc.projectionPlane
	*/
	virtual		NxPlane				getProjectionPlane()									const	= 0;

	/**
	\brief Sets the plane the fluid particles are projected to.

	\param plane Particle projection plane.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NX_FF_PROJECT_TO_PLANE NxFluidDesc.projectionPlane
	*/
	virtual		void 				setProjectionPlane(const NxPlane& plane)						= 0;
//@}
/************************************************************************************************/

/** @name Collisions
*/
//@{

	/**
	\brief Returns the collision method of the fluid.

	\return The collision method.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidCollisionMethod
	*/
	virtual		NxU32				getCollisionMethod()						const	= 0;

	/**
	\brief Sets the collision method of the fluid.

	Can be set to a combination of NX_F_STATIC and NX_F_DYNAMIC.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidCollisionMethod
	*/
	virtual		void				setCollisionMethod(NxU32 collMethod)				= 0;

	/**
	\brief Returns the restitution used for collision with static shapes.

	\return The static collision restitution.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.restitutionForStaticShapes
	*/
	virtual		NxReal				getRestitutionForStaticShapes()					const	= 0;

	/**
	\brief Sets the restitution used for collision with static shapes.
	
	Must be between 0 and 1.

	\param rest The new restitution for static shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.restitutionForStaticShapes
	*/
	virtual		void 				setRestitutionForStaticShapes(NxReal rest)					= 0;

	/**
	\brief Returns the dynamic friction used for collision with static shapes.

	\return The dynamic friction used for static shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.dynamicFrictionForStaticShapes
	*/
	virtual		NxReal				getDynamicFrictionForStaticShapes()					const	= 0;

	/**
	\brief Sets the dynamic friction used for collision with static actors.
	
	Must be between 0 and 1.

	\param adhesion The new dynamic friction used for static shapes

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.dynamicFrictionForStaticShapes
	*/
	virtual		void 				setDynamicFrictionForStaticShapes(NxReal friction)			= 0;

	/**
	\brief Returns the static friction used for collision with static shapes.

	This feature is currently unimplemented! 

	*/
	virtual		NxReal				getStaticFrictionForStaticShapes()					const	= 0;

	/**
	\brief Sets the static friction used for collision with static actors.
	
	This feature is currently unimplemented! 

	*/
	virtual		void 				setStaticFrictionForStaticShapes(NxReal friction)			= 0;

	/**
	\brief Returns the attraction used for collision with static actors.

	This feature is currently unimplemented! 
	
	*/
	virtual		NxReal				getAttractionForStaticShapes()						const	= 0;

	/**
	\brief Sets the attraction used for collision with static actors.
	
	This feature is currently unimplemented! 

	*/
	virtual		void 				setAttractionForStaticShapes(NxReal attraction)					= 0;

	/**
	\brief Returns the restitution used for collision with dynamic actors.

	\return The collision resitution for dynamic shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.restitutionForDynamicShapes
	*/
	virtual		NxReal				getRestitutionForDynamicShapes()				const	= 0;

	/**
	\brief Sets the restitution used for collision with dynamic actors.
	
	Must be between 0 and 1.

	\param rest The new collision restitution for dynamic shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.restitutionForDynamicShapes
	*/
	virtual		void 				setRestitutionForDynamicShapes(NxReal rest)				= 0;

	/**
	\brief Returns the dynamic friction used for collision with dynamic shapes.

	\return The dynamic friction used for dynamic shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.dynamicFrictionForDynamicShapes
	*/
	virtual		NxReal				getDynamicFrictionForDynamicShapes()			const	= 0;

	/**
	\brief Sets the dynamic friction used for collision with dynamic shapes.
	
	Must be between 0 and 1.
	
	\param friction The new dynamic friciton used for dynamic shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.dynamicFrictionForDynamicShapes
	*/
	virtual		void 				setDynamicFrictionForDynamicShapes(NxReal friction)		= 0;

	/**
	\brief Returns the static friction used for collision with dynamic shapes.

	This feature is currently unimplemented! 

	*/
	virtual		NxReal				getStaticFrictionForDynamicShapes()			const	= 0;

	/**
	\brief Sets the static friction used for collision with dynamic shapes.
	
	This feature is currently unimplemented! 

	*/
	virtual		void 				setStaticFrictionForDynamicShapes(NxReal friction)		= 0;

	/**
	\brief Returns the attraction used for collision with dynamic actors.

	This feature is currently unimplemented! 
	
	*/
	virtual		NxReal				getAttractionForDynamicShapes()					const	= 0;

	/**
	\brief Sets the attraction used for collision with dynamic actors.
	
	This feature is currently unimplemented! 

	*/
	virtual		void 				setAttractionForDynamicShapes(NxReal attraction)		= 0;

	/**
	\brief Sets the collision response coefficient.

	\param[in] coefficient The collision response coefficient (0 or greater).

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.collisionResponseCoefficient getCollisionResponseCoefficient()
	*/
	virtual		void				setCollisionResponseCoefficient(NxReal coefficient) = 0;

	/**
	\brief Retrieves the collision response coefficient.

	\return The collision response coefficient.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.collisionResponseCoefficient setCollisionResponseCoefficient()
	*/
	virtual		NxReal				getCollisionResponseCoefficient() const = 0;


//@}
/************************************************************************************************/


	/**
	\brief Sets actor flags.

	\param flag Member of #NxFluidFlag.
	\param val New flag value.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.flags
	*/
	virtual		void				setFlag(NxFluidFlag flag, bool val)								= 0;

	/**
	\brief Returns actor flags.

	\param flag Member of #NxFluidFlag.
	\return The current flag value.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.flags
	*/
	virtual		NX_BOOL				getFlag(NxFluidFlag flag)							const	= 0;

	/**
	\brief Retrieves the scene which this fluid belongs to.

	\return Owner Scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: No

	@see NxScene
	*/
	virtual		NxScene&			getScene()							const	= 0;

/************************************************************************************************/

/** @name Fluid Property Read Back
*/
//@{

	/**
	\brief Returns the maximum number of particles for this fluid.

	\return Max number of particles for this fluid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.maxParticles
	*/
	virtual		NxU32 				getMaxParticles()							const	= 0;

	/**
	\brief Returns the kernel radius multiplier (the particle interact within a radius of
	getRestParticleDistance() * getKernelRadiusMultiplier() ).

	\return The kernel radius multiplier.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.kernelRadiusMultiplier
	*/
	virtual		NxReal				getKernelRadiusMultiplier()					const	= 0;

	/**
	\brief Returns the motion limit multiplier (the particle can move the maximal distance of 
	getRestParticleDistance() * getMotionLimitMultiplier() during one timestep).

	\return motion limit multiplier.

	@see getRestParticleDistance()
	*/
	virtual		NxReal				getMotionLimitMultiplier()					const	= 0;

	/**
	\brief Returns the distance between particles and collision geometry, which is maintained during simulation.

	( distance = getCollisionDistanceMultiplier()/getRestParticlesPerMeter() ).

	\return collision distance multiplier.
	@see getRestParticleDistance()
	*/
	virtual		NxReal				getCollisionDistanceMultiplier()			const	= 0;

	/**
	\brief Returns the fluid packet size used for parallelization of the fluid computations.

	\return The packet size multiplier.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxU32				getPacketSizeMultiplier()					const	= 0;

	/**
	\brief Returns the number of particles per meter in the relaxed state of the fluid.

	\return Rest particles per meter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.restParticlesPerMeter
	*/
	virtual		NxReal				getRestParticlesPerMeter()					const	= 0;

	/**
	\brief Returns the density in the relaxed state of the fluid.

	\return Rest density.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc.restDensity
	*/
	virtual		NxReal				getRestDensity()							const	= 0;

	/**
	\brief Returns the inter-particle distance in the relaxed state of the fluid.

	This is the reciprocal of the value given by getRestParticlesPerMeter()

	\return Rest particle distance.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxReal				getRestParticleDistance()					const	= 0;

	/**
	\brief Returns the mass of a particle. This value is dependent on the rest inter-particle
	distance and the rest density of the fluid.

	\return Particle mass.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getRestParticleDistance()
	@see getRestDensity()
	*/
	virtual		NxReal				getParticleMass()							const	= 0;

	/**
	\brief Sets which collision group this fluid is part of.

	\param[in] collisionGroup The collision group for this fluid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCollisionGroup
	*/
	virtual void setGroup(NxCollisionGroup collisionGroup) = 0;

	/**
	\brief Retrieves the value set with #setGroup().

	\return The collision group this fluid belongs to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCollisionGroup
	*/
	virtual NxCollisionGroup getGroup() const = 0;

	/**
	\brief Sets 128-bit mask used for collision filtering.

	\param[in] groupsMask The group mask to set for the fluid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGroupsMask() NxGroupsMask
	*/
	virtual void setGroupsMask(const NxGroupsMask& groupsMask) = 0;

	/**
	\brief Sets 128-bit mask used for collision filtering.

	\return The group mask for the fluid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGroupsMask() NxGroupsMask
	*/
	virtual const NxGroupsMask getGroupsMask() const = 0;

	/**
	\brief Returns the minimal (exact) world space axis aligned bounding box (AABB)
	including all simulated particles.

	\param dest Used to store the world bounds.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		void				getWorldBounds(NxBounds3& dest)				const	= 0;
//@}
/************************************************************************************************/


/** @name Fluid Descriptor Operations
*/
//@{

	/**
	\brief Loads the fluid descriptor.

	\param[in] desc The descriptor used to restore the state of the object.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc
	*/
	virtual		bool				loadFromDesc(const NxFluidDescBase& desc)				= 0;

	/**
	\brief Saves the fluid descriptor.

	This method does not save out any emitters to the emitter array of the fluid desctriptor, 
	nor does it store particle data or any other user pointer. You can use NxFluid::saveEmittersToFluidDesc(NxFluidDesc &)
	to store the emitters of a fluid to a fluid descriptor.

	\param[out] desc The descriptor used to retrieve the state of the object.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidDesc
	@see saveEmittersToDesc
	*/
	virtual		bool				saveToDesc(NxFluidDescBase &desc)				const	= 0;

	/**
	\brief Get the PPU simulation time.

	This method returns the time taken to simulate the fluid on the PPU in units
	which are proportional to time but whose units are otherwise unspecified.

	\return the simulation time
	<b>Platform:</b>
	\li SW   : No
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No

	*/

    virtual		NxU32			    getPPUTime()									const	= 0;

//@}
/************************************************************************************************/

	/**
	\brief Sets a name string for the object that can be retrieved with getName().
	
	This is for debugging and is not used by the SDK.  The string is not copied by the
	SDK, only the pointer is stored.

	\param name The new name.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void			setName(const char* name)		= 0;

	/**
	\brief Returns the name string set with setName().

	\return The current name.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	const char*		getName()			const	= 0;


	/**
	\brief Retrieves the fluid's simulation compartment, as specified by the user at creation time.
	\return NULL if the fluid is not simulated in a compartment or if it was specified to run in 
	the default fluid compartment, otherwise the simulation compartment.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCompartment
	*/
	virtual NxCompartment *			getCompartment() const = 0;


/************************************************************************************************/

	/**
	\brief Retrieves the actor's force field material index, default index is 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldMaterial	getForceFieldMaterial() const = 0;

	/**
	\brief Sets the actor's force field material index, default index is 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void					setForceFieldMaterial(NxForceFieldMaterial)  = 0;

/************************************************************************************************/

	/**
	\brief Helper function to save fluidEmitters to a fluidDescriptor.

	\param[out] desc Descriptor to save to.
	\return True if the resulting fluid descriptor is valid. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE	bool		saveEmittersToFluidDesc(NxFluidDesc &desc);

	template<class AllocType>
	NX_INLINE	bool		saveEmittersToFluidDesc_Template(NxFluidDesc_Template<AllocType> &desc);

/************************************************************************************************/


	//public variables:
	void*			userData;	//!< The user can set this to anything, usually to create a 1:1 relationship with a user object.
	};

NX_INLINE	bool		NxFluid::saveEmittersToFluidDesc(NxFluidDesc &desc)
	{
	NxU32				numEmitters = getNbEmitters();
	NxFluidEmitter**	emitter		= getEmitters();
	for(NxU32 i = 0; i < numEmitters; i++)
	{
		NxFluidEmitterDesc emitterDesc;
		emitter[i]->saveToDesc(emitterDesc);
		desc.emitters.pushBack(emitterDesc);
	}
	return desc.isValid();
	}

template<class AllocType> 
NX_INLINE	bool		NxFluid::saveEmittersToFluidDesc_Template(NxFluidDesc_Template<AllocType> &desc)
	{
	NxU32				numEmitters = getNbEmitters();
	NxFluidEmitter**	emitter		= getEmitters();
	for(NxU32 i = 0; i < numEmitters; i++)
		{
		NxFluidEmitterDesc emitterDesc;
		emitter[i]->saveToDesc(emitterDesc);
		desc.emitters.pushBack(emitterDesc);
		}
		return desc.isValid();
	}

/** @} */
#endif


//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND

