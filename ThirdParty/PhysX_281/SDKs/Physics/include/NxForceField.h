#ifndef NX_PHYSICS_NXFORCEFIELD
#define NX_PHYSICS_NXFORCEFIELD
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
#include "NxForceFieldDesc.h"
#include "NxEffector.h"

class NxEffector;
class NxForceFieldShape;
class NxForceFieldShapeDesc;

/**
 \brief A force field effector.

 Instances of this object automate the application of forces onto rigid bodies, fluid, soft bodies and cloth.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes [SW fallback]
\li PS3  : Yes
\li XB360: Yes

 @see NxForceFieldDesc, NxScene::createForceField()
*/

class NxForceField
	{
	protected:
	NX_INLINE					NxForceField() : userData(NULL)	{}
	virtual						~NxForceField()	{}

	public:
	/**
	\brief Writes all of the effector's attributes to the description, as well
	as setting the actor connection point.

	\param[out] desc The descriptor used to retrieve the state of the effector.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void saveToDesc(NxForceFieldDesc &desc) = 0;

	/**
	\brief Retrieves the force field's transform.  
	
	This transform is either from world space or from actor space, depending on whether the actor pointer is set.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see setPose() getActor() NxForceFieldDesc::pose
	*/
	virtual NxMat34  getPose() const = 0;

	/**
	\brief Sets the force field's transform.  
	
	This transform is either from world space or from actor space, depending on whether the actor pointer is set.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see getPose() getActor() NxForceFieldDesc::pose
	*/
	virtual void setPose(const NxMat34  & pose) = 0;

	/**
	\brief Retrieves the actor pointer that this force field is attached to.  
	
	Unattached force fields return NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see setActor() NxForceFieldDesc::actor
	*/
	virtual NxActor * getActor() const = 0; 

	/**
	\brief Sets the actor pointer that this force field is attached to.  
	
	Pass NULL for unattached force fields.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see getActor() NxForceFieldDesc::actor
	*/
	virtual void setActor(NxActor * actor) = 0;
	
	/**
	\brief Sets the kernel function which this field will be using  

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void setForceFieldKernel(NxForceFieldKernel * kernel) = 0;

	/**
	\brief Retrieves the kernel function which this field is using  

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldKernel*			getForceFieldKernel() = 0;

	/**
	\brief Retrieves the include shape group of this forcefield. Shapes in this group will move with the force field.

	\return NxForceFieldShapeGroup The includeGroup of this force field.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldShapeGroup&		getIncludeShapeGroup() = 0; 
	
	/**
	\brief Adds a force field shape group to this force field to define its volume of activity.

	\param[in] group A force field shape group. See #NxForceFieldShapeGroup

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						addShapeGroup(NxForceFieldShapeGroup& group) = 0;

	/**
	\brief Removes a force field shape group from this force field.

	\param[in] group A force field shape group. See #NxForceFieldShapeGroup

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						removeShapeGroup(NxForceFieldShapeGroup &) = 0;

	/**
	\brief Returns the number of force field shape groups of this force field. (not counting the include group)

	\return The Number of force field shape groups. See #NxForceFieldShapeGroup

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32						getNbShapeGroups() const = 0; 

	/**
	\brief Restarts the force field shape groups iterator so that the next call to getNextShape() returns the first shape in the force field.  

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						resetShapeGroupsIterator() = 0; 

	/**
	\brief Retrieves the next FF shape group when iterating.

	\return NxForceFieldShapeGroup See #NxForceFieldShapeGroup

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldShapeGroup*		getNextShapeGroup() = 0; 

	/**
	\brief Retrieves the value set with #setGroup().

	NxCollisionGroup is an integer between 0 and 31.

	\return The collision group this shape belongs to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see setGroup() NxCollisionGroup
	*/
	virtual NxCollisionGroup  getGroup() const = 0; 

	/**
	\brief Sets which collision group this shape is part of.
	
	Default group is 0. Maximum possible group is 31.
	Collision groups are sets of shapes which may or may not be set
	to collision detect with each other; this can be set using NxScene::setGroupCollisionFlag()

	\param[in] collisionGroup The collision group for this shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see getGroup() NxCollisionGroup
	*/
	virtual void setGroup(NxCollisionGroup collisionGroup) = 0; 

	/**
	\brief Gets 128-bit mask used for collision filtering. See comments for ::NxGroupsMask

	\return The group mask for the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see setGroupsMask()
	*/
	virtual NxGroupsMask  getGroupsMask() const = 0; 

	/**
	\brief Sets 128-bit mask used for collision filtering. See comments for ::NxGroupsMask

	\param[in] mask The group mask to set for the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see getGroupsMask()
	*/
	virtual void setGroupsMask(NxGroupsMask  mask) = 0; 

	/**
	\brief Gets the Coordinate space of the field.

	\return NxForceFieldCoordinates See #NxForceFieldCoordinates

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldCoordinates getCoordinates() const					= 0;
	
	/**
	\brief Sets the Coordinate space of the field. Transforms position and velocity of objects into this space prior to kernel evaluation.

	\param[in] coordinates The coordinate system. See #NxForceFieldCoordinates

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void setCoordinates(NxForceFieldCoordinates coordinates)		= 0;

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
	virtual void  setName (const char* name) = 0;

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

	/**
	\brief Gets the force field scaling type for fluids

	\return NxForceFieldType The force field scaling type for fluids. See #NxForceFieldType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxForceFieldType	getFluidType()		const				= 0;
	
	/**
	\brief Sets the force field scaling type for fluids

	\param[in] t NxForceFieldType The force field scaling type for fluids. See #NxForceFieldType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void				setFluidType(NxForceFieldType t)		= 0;
	
	/**
	\brief Gets the force field scaling type for cloths

	\return NxForceFieldType The force field scaling type for fluids. See #NxForceFieldType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxForceFieldType	getClothType()		const				= 0;
	
	/**
	\brief Sets the force field scaling type for cloths

	\param[in] t NxForceFieldType The force field scaling type for fluids. See #NxForceFieldType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void				setClothType(NxForceFieldType t)		= 0;
	
	/**
	\brief Gets the force field scaling type for soft bodies

	\return NxForceFieldType The force field scaling type for fluids. See #NxForceFieldType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxForceFieldType	getSoftBodyType()	const				= 0;

	/**
	\brief Sets the force field scaling type for soft bodies

	\param[in] t NxForceFieldType The force field scaling type for fluids. See #NxForceFieldType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void				setSoftBodyType(NxForceFieldType t)		= 0;

	/**
	\brief Gets the force field scaling type for rigid bodies

	\return NxForceFieldType The force field scaling type for fluids. See #NxForceFieldType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxForceFieldType	getRigidBodyType()	const				= 0;
	
	/**
	\brief Sets the force field scaling type for rigid bodies

	\param[in] t NxForceFieldType The force field scaling type for fluids. See #NxForceFieldType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void				setRigidBodyType(NxForceFieldType t)	= 0;

	/**
	\brief Gets the force field flags @see NxForceFieldFlags

	\return NxForceFieldFlags The force field flags

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32	getFlags () const			= 0;

	/**
	\brief Sets the force field flags @see NxForceFieldFlags

	\param[in] f NxForceFieldFlags The force field flags

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void	setFlags(NxU32 f)				= 0;

	/**
	\brief Samples the force field. Incoming points & velocities must be in world space. The velocities pointer is optional and can be null. 

	\param[in] numPoints Size of the buffers
	\param[in] points Buffer of sample points
	\param[in] velocities Buffer of velocities at the sample points
	\param[out] outForces Buffer for the returned forces
	\param[out] outTorques Buffer for the returned torques

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void				samplePoints(NxU32 numPoints, const NxVec3* points, const NxVec3* velocities, NxVec3* outForces, NxVec3* outTorques)	const = 0;

	/**
	\brief Retrieves the scene which this force field belongs to.

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
	\brief Retrieves the force field variety index, default index is 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldVariety	getForceFieldVariety() const = 0;

	/**
	\brief Sets the force field variety index, default index is 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void				setForceFieldVariety(NxForceFieldVariety)  = 0;


	//public variables:
	void*			userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.

	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
