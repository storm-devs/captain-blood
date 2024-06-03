#ifndef NX_COLLISION_NXSHAPE
#define NX_COLLISION_NXSHAPE
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
#include "NxPhysicsSDK.h"

class NxBounds3;
class NxBoxShape;
class NxBoxShapeOffCenter;
class NxPlaneShape;
class NxSphereShape;
class NxCapsuleShape;
class NxCollisionSpace;
class NxConvexShape;
class NxTriangleMeshShape;
class NxHeightFieldShape;
class NxActor;
class NxRay;
class NxSphere;
class NxBox;
class NxCapsule;
class NxWheelShape;
struct NxRaycastHit;

/**
\brief Abstract base class for the various collision shapes.

An instance of a subclass can be created by calling the createShape() method of the NxActor class,
or by adding the shape descriptors into the NxActorDesc class before creating the actor.

Note: in order to avoid a naming conflict, downcast operators are isTYPE(), while up casts are getTYPE().

<h3>Visualizations</h3>
\li NX_VISUALIZE_COLLISION_AABBS
\li NX_VISUALIZE_COLLISION_SHAPES
\li NX_VISUALIZE_COLLISION_AXES

The AGEIA PhysX SDK users guide describes which shapes that can collide with each other (direct link: <a href="PhysXDocumentation.chm::/Guide/collision_Interactions.html">users guide</a>)

@see NxActor.createShape() NxSphereShape NxPlaneShape NxConvexShape NxTriangleMeshShape NxCapsuleShape NxBoxShape
*/
class NxShape
	{
	protected:
	NX_INLINE							NxShape() : userData(NULL), appData(NULL)
													{}
	virtual								~NxShape()	{}

	public:

	/**
	\brief Retrieves the actor which this shape is associated with.

	\return The actor this shape is associated with.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor
	*/
	virtual		NxActor&				getActor() const = 0;

	/**
	\brief Sets which collision group this shape is part of.
	
	Default group is 0. Maximum possible group is 31.
	Collision groups are sets of shapes which may or may not be set
	to collision detect with each other; this can be set using NxScene::setGroupCollisionFlag()

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] collisionGroup The collision group for this shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGroup() NxCollisionGroup
	*/
	virtual		void					setGroup(NxCollisionGroup collisionGroup) = 0;

	/**
	\brief Retrieves the value set with #setGroup().

	NxCollisionGroup is an integer between 0 and 31.

	\return The collision group this shape belongs to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGroup() NxCollisionGroup
	*/
	virtual		NxCollisionGroup		getGroup() const = 0;

	/**
	\brief Returns a world space AABB enclosing this shape.

	\param[out] dest Retrieves the world space bounds.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3
	*/
	virtual		void					getWorldBounds(NxBounds3& dest) const = 0;	

	/**
	\brief Sets shape flags

	The shape may be turned into a trigger by setting one or more of the
	above TriggerFlag-s to true. A trigger shape will not collide
	with other shapes. Instead, if a shape enters the trigger's volume, 
	a trigger event will be sent to the user via the NxUserTriggerReport::onTrigger method.
	You can set a NxUserTriggerReport object with NxScene::setUserTriggerReport().

	Since version 2.1.1 this is also used to setup generic (non-trigger) flags.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] flag The new shape flags to set for this shape. See #NxShapeFlag.
	\param[in] value True to set the flags. False to clear the flags specified in flag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeFlag getFlag()
	*/
	virtual		void					setFlag(NxShapeFlag flag, bool value) = 0;

	/**
	\brief Retrieves shape flags.

	\param[in] flag The flag to retrieve.
	\return The value of the flag specified.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeFlag setFlag()
	*/
	virtual		NX_BOOL					getFlag(NxShapeFlag flag) const = 0;

/************************************************************************************************/

/** @name Pose Manipulation
*/
//@{

	/**
	\brief The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by.
	
	This transformation is identity by default.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	<i>Note:</i> Does not automatically update the inertia properties of the owning actor (if applicable); use NxActor::updateMassFromShapes() to do this.
	\param[in] mat The new transform from the actor frame to the shape frame. <b>Range:</b> rigid body transform

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getLocalPose() NxShapeDesc.localPose()
	*/
	virtual		void					setLocalPose(const NxMat34& mat)			= 0;

	/**
	\brief The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by.
	
	This transformation is identity by default.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	<i>Note:</i> Does not automatically update the inertia properties of the owning actor (if applicable); use NxActor::updateMassFromShapes() to do this.
	\param[in] vec The new position of the shape relative to the actor frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLocalPose() NxShapeDesc.localPose getLocalPosition()
	*/
	virtual		void					setLocalPosition(const NxVec3& vec)			= 0;

	/**
	\brief The setLocal*() methods set the pose of the shape in actor space, i.e. relative to the actor they are owned by.

	This transformation is identity by default.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	<i>Note:</i> Does not automatically update the inertia properties of the owning actor (if applicable); use NxActor::updateMassFromShapes() to do this.
	\param[in] mat The new orientation relative to the actor frame.	<b>Range:</b> rotation matrix

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLocalPose() NxShapeDesc.localPose getLocalOrientation()
	*/
	virtual		void					setLocalOrientation(const NxMat33& mat)		= 0;

	/**
	\brief The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by.
	This transformation is identity by default.

	\return Pose of shape relative to the actors frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLocalPose() NxShapeDesc.localPose
	*/
	virtual		NxMat34					getLocalPose()					const	= 0;

	/**
	\brief The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by.

	This transformation is identity by default.

	\return Position of shape relative to actors frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLocalPosition() getLocalPose() NxShapeDesc.localPose
	*/
	virtual		NxVec3					getLocalPosition()				const	= 0;

	/**
	\brief The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative to the actor they are owned by.
	This transformation is identity by default.

	\return Orientation of shape relative to actors frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLocalOrientation() getLocalPose() NxShapeDesc.localPose
	*/
	virtual		NxMat33					getLocalOrientation()			const	= 0;

	/**
	\brief The setGlobal() calls are convenience methods which transform the passed parameter
	into the current local space of the actor and then call setLocalPose().

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	<i>Note:</i> Does not automatically update the inertia properties of the owning actor (if applicable); use NxActor::updateMassFromShapes() to do this.
	\param[in] mat The new shape pose, relative to the global frame. <b>Range:</b> rigid body transform

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLocalPose() getGlobalPose() setGlobalPosition() setGlobalOrientation()
	*/
	virtual		void					setGlobalPose(const NxMat34& mat)			= 0;

	/**
	\brief The setGlobal() calls are convenience methods which transform the passed parameter
	into the current local space of the actor and then call setLocalPose().

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	<i>Note:</i> Does not automatically update the inertia properties of the owning actor (if applicable); use NxActor::updateMassFromShapes() to do this.
	\param[in] vec The new shape position, relative to the global frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLocalPose() getGlobalPosition() setGlobalPosition() setGlobalOrientation()
	*/
	virtual		void					setGlobalPosition(const NxVec3& vec)		= 0;

	/**
	\brief The setGlobal() calls are convenience methods which transform the passed parameter
	into the current local space of the actor and then call setLocalPose().

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	<i>Note:</i> Does not automatically update the inertia properties of the owning actor (if applicable); use NxActor::updateMassFromShapes() to do this.
	\param[in] mat The new shape orientation relative to the global frame. <b>Range:</b> orientation matrix

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLocalPose() getGlobalOrientation() setGlobalPosition() setGlobalOrientation()
	*/
	virtual		void					setGlobalOrientation(const NxMat33& mat)	= 0;

	/**
	\brief The getGlobal*() methods retrieve the shape's current world space pose. This is 
	the local pose multiplied by the actor's current global pose.

	\return Pose of shape relative to the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalPose() getGlobalPosition() getGlobalOrientation()
	*/
	virtual		NxMat34					getGlobalPose()					const	= 0;

	/**
	\brief The getGlobal*() methods retrieve the shape's current world space pose. This is 
	the local pose multiplied by the actor's current global pose.

	\return Position of the shape relative to the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalPosition() getGlobalPose() getGlobalOrientation()
	*/
	virtual		NxVec3					getGlobalPosition()				const	= 0;

	/**
	\brief The getGlobal*() methods retrieve the shape's current world space pose. This is 
	the local pose multiplied by the actor's current global pose.

	\return Orientation of the shape realative to the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalOrientation() getGlobalPose() getGlobalPosition()
	*/
	virtual		NxMat33					getGlobalOrientation()			const	= 0;
//@}
/************************************************************************************************/

	/**
	\brief Assigns a material index to the shape.
	
	The material index can be retrieved by calling NxMaterial::getMaterialIndex().
	If the material index is invalid, it will still be recorded, but 
	the default material (at index 0) will effectively be used for simulation.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] matIndex The material index to assign to the shape. See #NxMaterial

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.createMaterial() getMaterial() NxMaterialIndex NxMaterial.getMaterialIndex()
	*/
	virtual		void					setMaterial(NxMaterialIndex matIndex)	= 0;

	/**
	\brief Retrieves the material index currently assigned to the shape.

	\return The material index of the material associated with the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see setMaterial() NxMaterialIndex
	*/
	virtual		NxMaterialIndex			getMaterial() const				= 0;

	/**
	\brief Sets the skin width. See #NxShapeDesc::skinWidth.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] skinWidth The new skin width. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getSkinWidth
	@see NxParameter
	*/
	virtual		void					setSkinWidth(NxReal skinWidth)	= 0;
	/**
	\brief Retrieves the skin width. See #NxShapeDesc::skinWidth.

	\return The skin width of the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getSkinWidth() NxParameter
	*/
	virtual		NxReal					getSkinWidth() const	= 0;

	/**
	\brief returns the type of shape.

	\return The shape type of the shape. See #NxShapeType.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType
	*/
	virtual		NxShapeType				getType() const = 0;
	
	/**
	\brief Assigns a CCD Skeleton mesh.  

	Note how CCDSkeletons can be shared between shapes.  

	<h3>Visualizations:</h3>
	\li #NX_VISUALIZE_COLLISION_CCD
	\li #NX_VISUALIZE_COLLISION_SKELETONS

	\param[in] ccdSkel The CCDSkeleton to assign to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeDesc.ccdSkeleton NxPhysicsSDK.createCCDSkeleton() getCCDSkeleton()
	*/
	virtual		void					setCCDSkeleton(NxCCDSkeleton *ccdSkel) = 0;

	/**
	\brief Retrieves the CCDSkeleton for this shape.

	\return The CCD skeleton associated with this shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeDesc.ccdSkeleton NxPhysicsSDK.createCCDSkeleton() setCCDSkeleton()
	*/
	virtual		NxCCDSkeleton *			getCCDSkeleton() const = 0;

/************************************************************************************************/

/** @name Is... Shape Type
*/
//@{
	/**
	\brief Type casting operator. The result may be cast to the desired subclass type.

	\param[in] type The type of shape to attempt a cast to.
	\return NULL if the shape is not type. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType
	*/
	virtual void*					is(NxShapeType type)		= 0;
	virtual	const void*				is(NxShapeType type) const	= 0;

	/**
	\brief Attempts to cast to an #NxPlaneShape.

	Returns NULL if this object is not of the appropriate type.

	\return NULL if the shape is not an #NxPlaneShape. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType NxPlaneShape
	*/
	NX_INLINE	NxPlaneShape*			isPlane()			{ return (NxPlaneShape*)		is(NX_SHAPE_PLANE);		}
	NX_INLINE	const NxPlaneShape*		isPlane() const 	{ return (const NxPlaneShape*)	is(NX_SHAPE_PLANE);		}
	
	/**
	\brief Attempts to cast to an #NxSphereShape

	Returns NULL if this object is not of the appropriate type.

	\return NULL if the shape is not an #NxSphereShape. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType NxSphereShape
	*/
	NX_INLINE	NxSphereShape*			isSphere()			{ return (NxSphereShape*)		is(NX_SHAPE_SPHERE);	}
	NX_INLINE	const NxSphereShape*	isSphere() const	{ return (const NxSphereShape*)	is(NX_SHAPE_SPHERE);	}
	
	/**
	\brief Attempts to cast to an #NxBoxShape

	Returns NULL if this object is not of the appropriate type.

	\return NULL if the shape is not an #NxBoxShape. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType NxBoxShape
	*/
	NX_INLINE	NxBoxShape*				isBox()				{ return (NxBoxShape*)			is(NX_SHAPE_BOX);		}
	NX_INLINE	const NxBoxShape*		isBox() const 		{ return (const NxBoxShape*)	is(NX_SHAPE_BOX);		}
	
	/**
	\brief Attempts to cast to an #NxCapsuleShape

	Returns NULL if this object is not of the appropriate type.

	\return NULL if the shape is not an #NxCapsuleShape. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType NxCapsuleShape
	*/
	NX_INLINE	NxCapsuleShape*			isCapsule()			{ return (NxCapsuleShape*)		is(NX_SHAPE_CAPSULE);	}
	NX_INLINE	const NxCapsuleShape*	isCapsule() const	{ return (const NxCapsuleShape*)is(NX_SHAPE_CAPSULE);	}

	/**
	\brief Attempts to cast to an #NxWheelShape

	Returns NULL if this object is not of the appropriate type.

	\return NULL if the shape is not an #NxWheelShape. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType NxWheelShape
	*/
	NX_INLINE	NxWheelShape*			isWheel()			{ return (NxWheelShape*)		is(NX_SHAPE_WHEEL);		}
	NX_INLINE	const NxWheelShape*		isWheel()	const	{ return (const NxWheelShape*)	is(NX_SHAPE_WHEEL);		}

	/**
	\brief Attempts to cast to an #NxConvexShape

	Returns NULL if this object is not of the appropriate type.

	\return NULL if the shape is not an #NxConvexShape. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType NxConvexShape
	*/
	NX_INLINE	NxConvexShape*			isConvexMesh()		{ return (NxConvexShape*)		is(NX_SHAPE_CONVEX);	}
	NX_INLINE	const NxConvexShape*	isConvexMesh() const{ return (const NxConvexShape*) is(NX_SHAPE_CONVEX);	}
	
	/**
	\brief Attempts to cast to an #NxTriangleMeshShape

	Returns NULL if this object is not of the appropriate type.

	\return NULL if the shape is not an #NxTriangleMeshShape. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType NxTriangleMeshShape
	*/
	NX_INLINE	NxTriangleMeshShape*		isTriangleMesh()		{ return (NxTriangleMeshShape*)	is(NX_SHAPE_MESH);			}
	NX_INLINE	const NxTriangleMeshShape*	isTriangleMesh() const	{ return (const NxTriangleMeshShape*)	is(NX_SHAPE_MESH);	}

	/**
	\brief Attempts to cast to an #NxHeightFieldShape

	Returns NULL if this object is not of the appropriate type.

	\return NULL if the shape is not an #NxHeightFieldShape. Otherwise a pointer to the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType NxHeightFieldShape
	*/
	NX_INLINE	NxHeightFieldShape*			isHeightField()			{ return (NxHeightFieldShape*)		is(NX_SHAPE_HEIGHTFIELD);	}
	NX_INLINE	const NxHeightFieldShape*	isHeightField() const	{ return (const NxHeightFieldShape*)is(NX_SHAPE_HEIGHTFIELD);	}
//@}
/************************************************************************************************/

	/**
	\brief Sets a name string for the object that can be retrieved with #getName().
	
	This is for debugging and is not used by the SDK.
	The string is not copied by the SDK, only the pointer is stored.

	\param[in] name The name string to set the objects name to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getName()
	*/
	virtual		void					setName(const char* name)		= 0;

	/**
	\brief retrieves the name string set with setName().
	\return The name associated with the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setName()
	*/
	virtual		const char*				getName()			const	= 0;

/************************************************************************************************/

/** @name Raycasting and Overlap Testing
*/
//@{

	/**
	\brief casts a world-space ray against the shape.

	maxDist is the maximum allowed distance for the ray. You can use this for segment queries.
	hintFlags is a combination of ::NxRaycastBit flags.
	firstHit is a hint saying you're only interested in a boolean answer.

	Note: Make certain that the direction vector of NxRay is normalized.

	\param[in] worldRay The ray to intersect against the shape in the global frame. <b>Range</b> See #NxRay
	\param[in] maxDist The maximum distance to check along the ray. <b>Range:</b> (0,inf)
	\param[in] hintFlags a combination of ::NxRaycastBit flags. Specifies which members of NxRaycastHit the user is interested in(eg normal, material etc)
	\param[out] hit Retrieves the information computed from a ray intersection
	\param[in] firstHit is a hint saying you're only interested in a boolean answer.
	\return True if the ray intersects the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.raycastAnyShape() NxScene.raycastAnyBounds()
	*/
	virtual		bool					raycast(const NxRay& worldRay, NxReal maxDist, NxU32 hintFlags, NxRaycastHit& hit, bool firstHit)	const = 0;

	/**
	\brief Checks whether the shape overlaps a world-space sphere or not.

	\param[in] worldSphere The sphere description in the global frame to test against. <b>Range:</b> See #NxSphere
	\return True if the sphere overlaps the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.overlapSphereShapes()
	*/
	virtual		bool					checkOverlapSphere(const NxSphere& worldSphere)														const = 0;

	/**
	\brief Checks whether the shape overlaps a world-space OBB or not.

	\param[in] worldBox The world space oriented box to check against. <b>Range:</b> See #NxBox
	\return True if the Oriented Bounding Box overlaps the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.overlapOBBShapes()
	*/
	virtual		bool					checkOverlapOBB(const NxBox& worldBox)																const = 0;

	/**
	\brief Checks whether the shape overlaps a world-space AABB or not.

	\param[in] worldBounds The world space axis aligned box to check against. <b>Range:</b> See #NxBounds3
	\return True if the Axis Aligned Bounding Box overlaps the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.overlapAABBShapes()
	*/
	virtual		bool					checkOverlapAABB(const NxBounds3& worldBounds)														const = 0;

	/**
	\brief Checks whether the shape overlaps a world-space capsule or not.

	\param[in] worldCapsule The world space capsule to check against. <b>Range:</b> See #NxCapsule
	\return True if the capsule overlaps the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.overlapCapsuleShapes()
	*/
	virtual		bool					checkOverlapCapsule(const NxCapsule& worldCapsule)														const = 0;
//@}
/************************************************************************************************/

	/**
	\brief Sets 128-bit mask used for collision filtering. See comments for ::NxGroupsMask

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] mask The group mask to set for the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGroupsMask()
	*/
	virtual		void					setGroupsMask(const NxGroupsMask& mask)	= 0;

	/**
	\brief Gets 128-bit mask used for collision filtering. See comments for ::NxGroupsMask

	\return The group mask for the shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGroupsMask()
	*/
	virtual		const NxGroupsMask		getGroupsMask()	const = 0;

		/**
	\brief Returns which compartment types the shape should not interact with.

	\return A combination of ::NxShapeCompartmentType values.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see setNonInteractingCompartmentTypes() NxShapeCompartmentType
	*/
	virtual		NxU32				getNonInteractingCompartmentTypes() const = 0;

	/**
	\brief Sets which compartment types the shape should not interact with.

	The shape will not interact with objects that belong to a compartment of the specified types.

	\note See #NxShapeDesc::nonInteractingCompartmentTypes for limitations.

	\param[in] compartmentTypes A combination of ::NxShapeCompartmentType values.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see getNonInteractingCompartmentTypes() NxShapeCompartmentType
	*/
	virtual		void				setNonInteractingCompartmentTypes(NxU32 compartmentTypes) = 0;


				void*					userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
				void*					appData;	//!< used internally, do not change.
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
