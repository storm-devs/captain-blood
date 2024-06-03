#ifndef NX_FLUIDS_NXFLUIDEMITTER
#define NX_FLUIDS_NXFLUIDEMITTER
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
#include "NxPhysicsSDK.h"
#include "NxFluidEmitterDesc.h"

class NxFluid;
class NxFluidEmitterDesc;
class NxShape;



/**
\brief The fluid emitter class. It represents an emitter (fluid source) which is associated with one fluid.

The emitter is an alternative to adding particles to the fluid via the NxFluid::addParticles() method.

Emission always takes place in a plane given by the orientation and position of the emitter. The 
shape of the area of emission is either a rectangle or an ellipse. The direction of emission is usually 
perpendicular to the emission plane. However, this can be randomly modulated using the setRandomAngle() 
method. An emitter can have two types of operation:
<ol>
<li>
	<i>Constant pressure.</i>
		In this case the state of the surrounding fluid is taken into account. The emitter tries 
		to match the rest spacing of the particles. Nice rays of water can be generated this way.
<li>
	<i>Constant flow rate.</i>
		In this case the emitter keeps emitting the same number of particles each frame. The rate can 
		be adjusted dynamically.
</ol>
The emitter's pose can be animated directly or attached to a shape which belongs to a 
dynamic actor. This shape is called the frame shape. When attaching an emitter to a shape, one 
has the option of enabling impulse transfer from the emitter to the body of the shape. 
The impulse generated is dependent on the rate, density, 
and velocity of the emitted particles.

*/
class NxFluidEmitter
	{
	protected:
	NX_INLINE				NxFluidEmitter() : userData(NULL)	{}
	virtual					~NxFluidEmitter()	{}

	public:

	/**
	\brief Returns the owner fluid.

	\return The fluid this emitter is associated with.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxFluid &	getFluid() const = 0;

	/**
	\brief Sets the pose of the emitter in world space.

	\param[in] mat New pose of the emitter in world space.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		void		setGlobalPose(const NxMat34& mat)					= 0;

	/**
	\brief Sets the position of the emitter in world space.

	\param[in] vec New positon in world space.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		void		setGlobalPosition(const NxVec3& vec)				= 0;

	/**
	\brief Sets the orientation of the emitter in world space.

	\param[in] mat New orientation in world space.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		void		setGlobalOrientation(const NxMat33& mat)			= 0;

//exclude from documentation
/** \cond */

	/**
	\brief The get*Val() methods work just like the get*() methods, except they return the 
	desired values instead of copying them to destination variables.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxMat34		getGlobalPoseVal()						const	= 0;
	virtual		NxVec3		getGlobalPositionVal()					const	= 0;
	virtual		NxMat33		getGlobalOrientationVal()				const	= 0;
/** \endcond */

	/**
	\brief Returns the pose of the emitter in world space.

	\return The global pose.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE	NxMat34		getGlobalPose()							const	{ return getGlobalPoseVal();		}

	/**
	\brief Returns the position of the emitter in world space.

	\return The world space position.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE	NxVec3		getGlobalPosition()						const	{ return getGlobalPositionVal();	}

	/**
	\brief Returns the orientation of the emitter in world space.

	\return The world space orientation.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE	NxMat33		getGlobalOrientation()					const	{ return getGlobalOrientationVal();	}

	/**
	\brief Sets the pose of the emitter relative to the frameShape. 

	The pose is set relative to the shape frame.

	If the frameShape is NULL, world space is used.

	\param[in] mat The new local pose of the emitter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.relPose
	*/
	virtual		void		setLocalPose(const NxMat34& mat)					= 0;

	/**
	\brief Sets the position of the emitter relative to the frameShape. 

	The pose is relative to the shape frame.

	If the frameShape is NULL, world space is used.

	\param[in] vec The new local position of the emitter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.relPose
	*/
	virtual		void		setLocalPosition(const NxVec3& vec)					= 0;

	/**
	\brief Sets the orientation of the emitter relative to the frameShape. 

	The pose is relative to the shape frame.

	If the frameShape is NULL, world space is used.

	\param[in] mat The new local orientation of the emitter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.relPose
	*/
	virtual		void		setLocalOrientation(const NxMat33& mat)				= 0;

//exclude from documentation
/** \cond */

	/**
	\brief 	The get*Val() methods work just like the get*() methods, except they return the 
	desired values instead of copying them to the destination variables.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxMat34		getLocalPoseVal()						const	= 0;
	virtual		NxVec3		getLocalPositionVal()					const	= 0;
	virtual		NxMat33		getLocalOrientationVal()				const	= 0;
/** \endcond */


	/**
	\brief Returns the pose of the emitter relative to the frameShape. 

	The pose is relative to the shape frame.

	If the frameShape is NULL, world space is used.

	\return The local pose of the emitter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.relPose
	*/
	NX_INLINE	NxMat34		getLocalPose()							const	{ return getLocalPoseVal();		}
	
	/**
	\brief Returns the position of the emitter relative to the frameShape. 

	The pose is relative to the shape frame.

	If the frameShape is NULL, world space is used.

	\return The local position of the emitter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.relPose
	*/

	NX_INLINE	NxVec3		getLocalPosition()						const	{ return getLocalPositionVal();	}

	/**
	\brief Returns the orientation of the emitter relative to the frameShape. 

	The pose is relative to the shape frame.

	If the frameShape is NULL, world space is used.

	\return The local orientation of the emitter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.relPose
	*/
	NX_INLINE	NxMat33		getLocalOrientation()					const	{ return getLocalOrientationVal();	}

	/**
	\brief Sets the frame shape. Can be set to NULL.

	\param[in] shape The frame shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.frameShape
	*/
	virtual		void 		setFrameShape(NxShape* shape)							= 0;

	/**
	\brief Returns the frame shape. May be NULL.

	\return The frame shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.frameShape
	*/
	virtual		NxShape * 	getFrameShape()							const	= 0;

	/**
	\brief Returns the radius of the emitter along the x axis.

	\return Radius of emitter along the X axis.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.dimensionX
	*/
	virtual		NxReal 			getDimensionX()						const	= 0;

	/**
	\brief Returns the radius of the emitter along the y axis.

	\return Radius of emitter along the Y axis.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.dimensionY
	*/
	virtual		NxReal 			getDimensionY()						const	= 0;

	/**
	\brief Sets the maximal random displacement in every dimension.

	\param[in] disp The maximal random displacment of particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.randomPos
	*/
	virtual		void 			setRandomPos(NxVec3 disp)						= 0;

	/**
	\brief Returns the maximal random displacement in every dimension.

	\return The maximal random displacment of particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.randomPos
	*/
	virtual		NxVec3 			getRandomPos()						const	= 0;

	/**
	\brief Sets the maximal random angle offset (in radians). 

	<b>Unit:</b> Radians

	\param[in] angle Maximum random angle for emitted particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.randomAngle
	*/
	virtual		void 			setRandomAngle(NxReal angle)						= 0;

	/**
	\brief Returns the maximal random angle offset (in radians). 

	<b>Unit:</b> Radians

	\return Maximum random angle for emitted particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.randomAngle
	*/
	virtual		NxReal 			getRandomAngle()					const	= 0;

	/**
	\brief Sets the velocity magnitude of the emitted particles. 

	\param[in] vel New velocity magnitude of emitted particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.fluidVelocityMagnitude
	*/
	virtual		void 			setFluidVelocityMagnitude(NxReal vel)			= 0;

	/**
	\brief Returns the velocity magnitude of the emitted particles.

	\return Velocity magnitude of emitted particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.fluidVelocityMagnitude
	*/
	virtual		NxReal 			getFluidVelocityMagnitude()			const	= 0;

	/**
	\brief Sets the emission rate (particles/second).
	
	Only used if the NxEmitterType is NX_FE_CONSTANT_FLOW_RATE.

	\param[in] rate New emission rate.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.rate
	*/
	virtual		void 			setRate(NxReal rate)								= 0;

	/**
	\brief Returns the emission rate.

	\return Emission rate.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.rate
	*/
	virtual		NxReal 			getRate()							const	= 0;

	/**
	\brief Sets the particle lifetime.

	\param[in] life Lifetime of emitted particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.particleLifetime
	*/
	virtual		void 			setParticleLifetime(NxReal life)					= 0;

	/**
	\brief Returns the particle lifetime.

	\return Lifetime of emitted particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.particleLifetime
	*/
	virtual		NxReal 			getParticleLifetime()				const	= 0;

	/**
	\brief Sets the repulsion coefficient.

	\param[in] coefficient The repulsion coefficient in the range from 0 to inf.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.repulsionCoefficient getRepulsionCoefficient()
	*/
	virtual		void				setRepulsionCoefficient(NxReal coefficient) = 0;

	/**
	\brief Retrieves the repulsion coefficient.

	\return The repulsion coefficient.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.repulsionCoefficient setRepulsionCoefficient()
	*/
	virtual		NxReal				getRepulsionCoefficient() const = 0;

	/**
	\brief Resets the particle reservoir. 

	\param[in] new maxParticles value.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.maxParticles
	*/
	virtual		void				resetEmission(NxU32 maxParticles)		= 0;

	/**
	\brief Returns the maximal particle number to be emitted. 

	\return max particles.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.maxParticles
	*/
	virtual		NxU32 				getMaxParticles()				const	= 0;

	/**
	\brief Returns the number of particles that have been emitted already. 

	\return number of particles already emitted.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxU32				getNbParticlesEmitted()			const	= 0;

	/**
	\brief Sets the emitter flags. 

	\param[in] flag Member of #NxFluidEmitterFlag.
	\param[in] val New flag value.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterFlag
	*/
	virtual		void			setFlag(NxFluidEmitterFlag flag, bool val)			= 0;

	/**
	\brief Returns the emitter flags.

	\param[in] flag Member of #NxFluidEmitterFlag.
	\return The current flag value.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterFlag
	*/
	virtual		NX_BOOL			getFlag(NxFluidEmitterFlag flag)			const	= 0;

	/**
	\brief Get the emitter shape.

	\param[in] shape Member of #NxEmitterShape.
	\return True if it is of type shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc.shape
	*/
	virtual		NX_BOOL			getShape(NxEmitterShape shape)			const	= 0;

	/**
	\brief Get the emitter type.

	\param[in] type Member of #NxEmitterType
	\return True if it is of type type.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxEmitterType
	*/
	virtual		NX_BOOL			getType(NxEmitterType type)				const	= 0;

/************************************************************************************************/

/** @name FluidEmitter Descriptor Operations
*/
//@{

	/**
	\brief Loads the FluidEmitter descriptor.

	\param[in] desc The descriptor used to restore the state of the object.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc
	*/
	virtual		bool			loadFromDesc(const NxFluidEmitterDesc& desc)				= 0;

	/**
	\brief Saves the FluidEmitter descriptor.

	\param[out] desc The descriptor used to retrieve the state of the object.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidEmitterDesc
	*/
	virtual		bool			saveToDesc(NxFluidEmitterDesc &desc)				const	= 0;

//@}
/************************************************************************************************/


	/**
	\brief Sets a name string for the object that can be retrieved with getName().
	
	This is for debugging and is not used by the SDK.  The string is not copied by the SDK; 
	only the pointer is stored.

	\param[in] name The new name.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		void			setName(const char* name)		= 0;

	/**
	\brief Retrieves the name string set with setName().

	\return The current name.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		const char*		getName()			const	= 0;

	void*				userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	};
/** @} */
#endif


//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND

