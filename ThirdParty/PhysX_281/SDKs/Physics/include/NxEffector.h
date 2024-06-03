#ifndef NX_PHYSICS_NXEFFECTOR
#define NX_PHYSICS_NXEFFECTOR
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
#include "NxEffectorDesc.h"

class NxScene;
class NxSpringAndDamperEffector;

/**
 \brief An effector is a class that gets called before each tick of the
 scene.
 
 At this point it may apply any permissible effect
 to the objects. For example: #NxSpringAndDamperEffector

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

 @see NxSpringAndDamperEffector NxScene.createSpringAndDamperEffector
*/
class NxEffector
	{
	public:
	/**
	\brief Retrieve the type of this effector.
	\return The type of effector.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes 
	\li PS3  : Yes
	\li XB360: Yes

	@see NxEffectorType
	*/
	virtual NxEffectorType  getType() const = 0;

	/**
	\brief Type casting operator. The result may be cast to the desired subclass type.

	\param[in] type Used to query for a specific effector type.
	\return NULL if the object if not of type(see #NxEffectorType). Otherwise a pointer to this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxEffectorType
	*/
	NX_INLINE void* is(NxEffectorType type) { return (type == getType()) ? (void*)this : NULL;		};

	/**
	\brief Type casting operator. The result may be cast to the desired subclass type.

	\param[in] type Used to query for a specific effector type.
	\return NULL if the object if not of type(see #NxEffectorType). Otherwise a pointer to this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxEffectorType
	*/
	NX_INLINE const void* is(NxEffectorType type) const { return (type == getType()) ? (const void*)this : NULL;		};

	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a spring and damper effector a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector
	*/
	NX_INLINE NxSpringAndDamperEffector*	isSpringAndDamperEffector() { return (NxSpringAndDamperEffector*)is(NX_EFFECTOR_SPRING_AND_DAMPER);}


	/**
	\brief Attempts to perform a downcast to the type returned.
	
	Returns 0 if this object is not of the appropriate type.

	\return If this is a spring and damper effector a pointer otherwise NULL.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector
	*/
	NX_INLINE const NxSpringAndDamperEffector*	isSpringAndDamperEffector() const { return (const NxSpringAndDamperEffector*)is(NX_EFFECTOR_SPRING_AND_DAMPER);}

	/**
	\brief Sets a name string for the object that can be retrieved with getName().
	
	This is for debugging and is not used by the SDK. The string is not copied by the SDK, only the pointer is stored.
	
	\param[in] name String to set the objects name to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getName()
	*/
	virtual	void			setName(const char* name)		= 0;

	/**
	\brief Retrieves the name string set with setName().

	\return The name string for this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setName()
	*/
	virtual	const char*		getName()			const	= 0;

	/**
	\brief Retrieves the owner scene

	\return The scene which this effector belongs to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene
	*/
	virtual		NxScene&					getScene() const = 0;


	void * userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	void * appData;		//!< used internally, do not change.

	protected:

	NX_INLINE NxEffector() : userData(NULL), appData(NULL) {}
	virtual NX_INLINE ~NxEffector(){};
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
