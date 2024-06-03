#ifndef NX_PHYSICS_NXFORCEFIELDSHAPEGROUP
#define NX_PHYSICS_NXFORCEFIELDSHAPEGROUP
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

class NxScene;
class NxForceField;
class NxForceFieldShape;
class NxForceFieldShapeDesc;
class NxForceFieldShapeGroupDesc;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class NxForceFieldShapeGroup
	{
	protected:
	NX_INLINE					NxForceFieldShapeGroup() : userData(NULL)	{}
	virtual						~NxForceFieldShapeGroup()	{}

	public:

	/**
	\brief Creates a NxForceFieldShape and adds it to the group. 
	
	The volume of activity of the force field is defined by the union of all of the force field's shapes' volumes created
	here without the NX_FFS_EXCLUDE flag set, minus the union of all of the force field's shapes with the NX_FFS_EXCLUDE flag set.

	The shapes are owned by the force field and released if the force field is released.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see releaseShape() NxForceFieldShapeDesc
	*/
	virtual NxForceFieldShape* createShape(const NxForceFieldShapeDesc &) = 0;
	
	/**
	\brief Releases the passed force field shape. 

	The passed force field shape must previously have been created with this force field's createShape() call.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see createShape() NxForceFieldShapeDesc
	*/	
	virtual void releaseShape(const NxForceFieldShape &) = 0;

	/**
	\brief Returns the number of shapes in the force field group.

	\return The number of shapes in this group.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see createShape() NxForceFieldShapeDesc
	*/	
	virtual NxU32  getNbShapes() const = 0; 

	/**
	\brief Restarts the shape iterator so that the next call to getNextShape() returns the first shape in the force field group.  

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/	
	virtual void  resetShapesIterator() = 0; 

	/**
	\brief Retrieves the next shape when iterating.

	\return NxForceFieldShape See #NxForceFieldShape

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/	
	virtual NxForceFieldShape*  getNextShape() = 0; 

	/**
	\brief If this is an include group, getForceField() will return the force field of this group, else NULL will be returned.

	\return NxForceField See #NxForceField

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/	
	virtual NxForceField*	getForceField() const = 0; 

	/**
	\brief Returns the force field shape group flags. @see NxForceFieldShapeGroupFlags

	\return NxForceFieldShapeGroupFlags See #NxForceFieldShapeGroupFlags

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/	
	virtual	NxU32			getFlags() const = 0;

	/**
	\brief Writes all of the shape groups's attributes to the description

	\param[out] desc The descriptor used to retrieve the state of the shape group.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void saveToDesc(NxForceFieldShapeGroupDesc &desc) = 0;

	/**
	\brief Retrieves the scene which this force field group belongs to.

	\return Owner Scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene
	*/
	virtual NxScene&	getScene()	const = 0;

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
	virtual void  setName (const char* name)= 0;

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
