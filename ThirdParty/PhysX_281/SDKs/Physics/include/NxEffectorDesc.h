#ifndef NX_PHYSICS_NXEFFECTORDESC
#define NX_PHYSICS_NXEFFECTORDESC
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


enum NxEffectorType
	{
	NX_EFFECTOR_SPRING_AND_DAMPER,
	};

/**
 \brief Descriptor class for NxEffector class.
 
 Effector descriptors for all types are derived from this class.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

 @see NxSpringAndDamperEffectorDesc
*/
class NxEffectorDesc
	{
	protected:
	/**
	\brief The type of effector. This is set by the c'tor of the derived class.
	*/
	NxEffectorType type;

	public:
	
	/**
	\brief Will be copied to NxEffector::userData.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	void* userData;

	/**
	\brief Possible debug name. The string is not copied by the SDK, only the pointer is stored.

	<b>Default</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	const char* name;

	/**
	\brief Constructor sets to default.
	*/
	virtual NX_INLINE ~NxEffectorDesc();
	
	/**
	\brief (re)sets the structure to the default.	
	*/
	virtual NX_INLINE void setToDefault();
	
	/**
	\brief Returns true if the descriptor is valid.

	\return true if the current settings are valid
	*/
	virtual NX_INLINE bool isValid() const;

	/**
	\brief Retrieves the effector type.

	\return The type of effector this descriptor describes. 

	@see NxEffectorType
	*/
	NX_INLINE NxEffectorType   getType() const;

	protected:
	/**
	\brief Constructor sets to default.

	\param type effector type
	*/
	NX_INLINE NxEffectorDesc(NxEffectorType type);
	};

NX_INLINE NxEffectorDesc::NxEffectorDesc(NxEffectorType t) : type(t)
	{
	setToDefault();
	}

NX_INLINE NxEffectorDesc::~NxEffectorDesc()
	{
	//nothing
	}

NX_INLINE void NxEffectorDesc::setToDefault()
	{
	userData = NULL;
	name = NULL;
	}

NX_INLINE bool NxEffectorDesc::isValid() const
	{
	//nothing
	return true;	
	}
NX_INLINE NxEffectorType   NxEffectorDesc::getType()   const   
	{ 
	return type; 
	}



/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
