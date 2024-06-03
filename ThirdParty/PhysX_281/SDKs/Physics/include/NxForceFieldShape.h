#ifndef NX_PHYSICS_NXFORCEFIELDSHAPE
#define NX_PHYSICS_NXFORCEFIELDSHAPE
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
#include "NxForceFieldShapeDesc.h"

class NxSphereForceFieldShape;
class NxBoxForceFieldShape;
class NxCapsuleForceFieldShape;
class NxConvexForceFieldShape;

/**
 \brief A shape that represents a volume in which the force field acts on objects.
 

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes [SW fallback]
\li PS3  : Yes
\li XB360: Yes

 @see NxForceFieldShapeDesc, NxForceField
*/
class NxForceFieldShape 
	{
	protected:
	NX_INLINE							NxForceFieldShape() : userData(NULL), appData(NULL)		{}
	virtual								~NxForceFieldShape()	{}
	public:

	/**
	\brief Retrieves the force field shape's transform.  
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see setPose() getFlags() NxForceFieldShapeDesc::pose
	*/
	virtual NxMat34  getPose() const = 0;

	/**
	\brief Sets the force field shape's transform.  
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see getPose() getFlags() NxForceFieldShapeDesc::pose
	*/
	virtual void setPose(const NxMat34 &) = 0;

	/**
	\brief Returns the owning force field if this is a shape of an include group, else NULL will be returned

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback] 
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceField
	*/
	virtual NxForceField * getForceField() const = 0;

	/**
	\brief Returns the owning force field shape group.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback] 
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceField
	*/
	virtual NxForceFieldShapeGroup & getShapeGroup() const = 0;

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
	virtual void  setName (const char *name)= 0;

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
	virtual const char *  getName () const = 0;

	/**
	\brief Retrieve the type of this force field shape.
	\return The type of force field shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback] 
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType
	*/
	virtual NxShapeType  getType () const = 0; 

	/**
	\brief Type casting operator. The result may be cast to the desired subclass type.

	\param[in] type Used to query for a specific effector type.
	\return NULL if the object if not of type(see #NxShapeType). Otherwise a pointer to this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType
	*/
	NX_INLINE void *  is (NxShapeType type);
	 
	/**
	\brief Type casting operator. The result may be cast to the desired subclass type.

	\param[in] type Used to query for a specific effector type.
	\return NULL if the object if not of type(see #NxShapeType). Otherwise a pointer to this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeType
	*/
	NX_INLINE const void *  is (NxShapeType type) const;
	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a NxSphereForceFieldShape a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphereForceFieldShape
	*/
	NX_INLINE NxSphereForceFieldShape *  isSphere ();

	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a NxSphereForceFieldShape a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphereForceFieldShape()
	*/
	NX_INLINE const NxSphereForceFieldShape *  isSphere () const;

	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a NxBoxForceFieldShape a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxForceFieldShape()
	*/
	NX_INLINE NxBoxForceFieldShape *  isBox ();

	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a NxBoxForceFieldShape a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBoxForceFieldShape
	*/
	NX_INLINE const NxBoxForceFieldShape *  isBox () const;

	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a NxCapsuleForceFieldShape a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCapsuleForceFieldShape
	*/
	NX_INLINE NxCapsuleForceFieldShape *  isCapsule ();

	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a NxCapsuleForceFieldShape a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCapsuleForceFieldShape
	*/
	NX_INLINE const NxCapsuleForceFieldShape *  isCapsule () const;

	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a NxConvexForceFieldShape a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxConvexForceFieldShape
	*/
	NX_INLINE NxConvexForceFieldShape *  isConvex ();

	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a NxConvexForceFieldShape a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxConvexForceFieldShape
	*/
	NX_INLINE const NxConvexForceFieldShape *  isConvex () const;

	void*					userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	void*					appData;	//!< used internally, do not change.
	};

NX_INLINE void *  NxForceFieldShape::is (NxShapeType type)
	{
	return (type == getType()) ? (void*)this : NULL;
	}

NX_INLINE const void *  NxForceFieldShape::is (NxShapeType type) const  
	{
	return (type == getType()) ? (const void*)this : NULL;
	}

NX_INLINE NxSphereForceFieldShape *  NxForceFieldShape::isSphere ()
	{
	return (NxSphereForceFieldShape*)is(NX_SHAPE_SPHERE);
	}

NX_INLINE const NxSphereForceFieldShape *  NxForceFieldShape::isSphere () const  
	{
	return (const NxSphereForceFieldShape*)is(NX_SHAPE_SPHERE);
	}

NX_INLINE NxBoxForceFieldShape *  NxForceFieldShape::isBox ()
	{
	return (NxBoxForceFieldShape*)is(NX_SHAPE_BOX);
	}

NX_INLINE const NxBoxForceFieldShape *  NxForceFieldShape::isBox () const   
	{
	return (const NxBoxForceFieldShape*)is(NX_SHAPE_BOX);
	}

NX_INLINE NxCapsuleForceFieldShape *  NxForceFieldShape::isCapsule ()
	{
	return (NxCapsuleForceFieldShape*)is(NX_SHAPE_CAPSULE);
	}

NX_INLINE const NxCapsuleForceFieldShape *  NxForceFieldShape::isCapsule () const
	{
	return (const NxCapsuleForceFieldShape*)is(NX_SHAPE_CAPSULE);
	}

NX_INLINE NxConvexForceFieldShape *  NxForceFieldShape::isConvex ()
	{
	return (NxConvexForceFieldShape*)is(NX_SHAPE_CONVEX);
	}

NX_INLINE const NxConvexForceFieldShape *  NxForceFieldShape::isConvex () const
	{
	return (const NxConvexForceFieldShape*)is(NX_SHAPE_CONVEX);
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
