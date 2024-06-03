#ifndef NX_PHYSICS_NXMATERIAL
#define NX_PHYSICS_NXMATERIAL
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

#include "NxMaterialDesc.h"

class NxScene;

/**
\brief Class for describing a shape's surface properties.

<h3>Creation</h3>

Example material creation:
\include NxMaterial_Create.cpp

You can create a material which has different friction coefficients depending on the direction that
a body in contact is trying to move in. This is called anisotropic friction.

<h3>Anisotropic Friction</h3>

Anisotropic friction is useful for modeling things like sledges, skis etc

When you create an anisotropic material you specify the default friction parameters and also friction parameters for the V axis.
The friction parameters for the V axis are applied to motion along the direction of anisotropy (dirOfAnisotropy).

Anisotropic Material Example:
\include NxMaterial_Aniso.cpp

<h3>Default Material</h3>

You can change the properties of the default material by querying for material index 0.

Default Material Example:
\include NxMaterial_ChangeDefault.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_CONTACT_POINT
\li #NX_VISUALIZE_CONTACT_NORMAL
\li #NX_VISUALIZE_CONTACT_ERROR
\li #NX_VISUALIZE_CONTACT_FORCE

@see NxMaterialDesc NxScene.createMaterial
*/
class NxMaterial
	{
	protected:
	NX_INLINE					NxMaterial() : userData(NULL)		{}
	virtual						~NxMaterial()	{}

	public:
	/**
	\brief The ID of the material can be retrieved using this function.	  
	
	Materials are associated with mesh faces and shapes using 16 bit identifiers of type NxMaterialIndex rather
	than pointers.

	If you release a material while its material ID is still in use by shapes or meshes, the material usage
	of these objects becomes undefined as the material index gets recycled.

	\return The material index for this material.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterialIndex NxTriangleMeshDesc.materialIndices NxShapeDesc.materialIndex NxShape.setMaterial()
	*/
	virtual		NxMaterialIndex getMaterialIndex() = 0;

	/**
	\brief Loads the entire state of the material from a descriptor with a single call.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] desc The descriptor used to set this objects state.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see saveToDesc NxMaterialDesc
	*/
	virtual		void			loadFromDesc(const NxMaterialDesc& desc) = 0;

	/**
	\brief Saves the state of the material into a descriptor.

	\param[out] desc The descriptor used to retrieve this objects state.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see loadFromDesc NxMaterialDesc
	*/
	virtual		void			saveToDesc(NxMaterialDesc& desc) const	= 0;

	/**
	\brief retrieves owner scene

	\return The scene which this material belongs to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene
	*/
	virtual		NxScene&		getScene() const = 0;

	/**
	\brief Sets the coefficient of dynamic friction.
	
	The coefficient of dynamic friction should be in [0, +inf]. If set to greater than staticFriction, the effective value of staticFriction will be increased to match.
	If the flag NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] coef Coefficient of dynamic friction. <b>Range:</b> [0, +inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterialDesc.dynamicFriction getDynamicFriction()
	*/
	virtual		void			setDynamicFriction(NxReal coef) = 0;

	/**
	\brief Retrieves the DynamicFriction value.

	\return The coefficient of dynamic friction.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setDynamicFriction NxMaterialDesc.dynamicFriction
	*/
	virtual		NxReal			getDynamicFriction() const = 0;

	/**
	\brief Sets the coefficient of static friction
	
	The coefficient of static friction should be in the range [0, +inf]
	if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] coef Coefficient of static friction. <b>Range:</b> [0,inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getStaticFriction() NxMaterialDesc.staticFriction
	*/
	virtual		void			setStaticFriction(NxReal coef) = 0;

	/**
	\brief Retrieves the coefficient of static friction.
	\return The coefficient of static friction.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setStaticFriction NxMaterialDesc.staticFriction
	*/
	virtual		NxReal			getStaticFriction() const = 0;

	/**
	\brief Sets the coefficient of restitution 
	
	A coefficient of 0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] rest Coefficient of restitution. <b>Range:</b> [0,1]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getRestitution() NxMaterialDesc.restitution
	*/
	virtual		void			setRestitution(NxReal rest) = 0;

	/**
	\brief Retrieves the coefficient of restitution. 

	See #setRestitution.

	\return The coefficient of restitution.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setRestitution() NxMaterialDesc.restitution
	*/
	virtual		NxReal			getRestitution() const = 0;

	/**
	\brief Sets the dynamic friction coefficient along the secondary (V) axis. 

	This is used when anisotropic friction is being applied. I.e. the NX_MF_ANISOTROPIC flag is set.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] coef Coefficient of dynamic friction in the V axis. <b>Range:</b> [0, +inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getDynamicFrictionV() NxMaterialDesc.dynamicFrictionV setFlags()
	*/
	virtual		void			setDynamicFrictionV(NxReal coef) = 0;

	/**
	\brief Retrieves the dynamic friction coefficient for the V direction.
	
	See #setDynamicFrictionV.

	\return The coefficient if dynamic friction in the V direction.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setDynamicFrictionV() NxMaterialDesc.dynamicFrictionV
	*/
	virtual		NxReal			getDynamicFrictionV() const = 0;

	/**
	\brief Sets the static friction coefficient along the secondary (V) axis. 

	This is used when anisotropic friction is being applied. I.e. the NX_MF_ANISOTROPIC flag is set.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] coef Coefficient of static friction in the V axis. <b>Range:</b> [0,inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getStaticFrictionV() NxMaterialDesc.staticFrictionV setFlags()
	*/
	virtual		void			setStaticFrictionV(NxReal coef) = 0;

	/**
	\brief Retrieves the static friction coefficient for the V direction.

	\return The coefficient of static friction in the V direction.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setStaticFrictionV() NxMaterialDesc.staticFrictionV
	*/
	virtual		NxReal			getStaticFrictionV() const = 0;

	/**
	\brief Sets the shape space direction (unit vector) of anisotropy.

	This is used when anisotropic friction is being applied. I.e. the NX_MF_ANISOTROPIC flag is set.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] vec Shape space direction of anisotropy. <b>Range:</b> direction vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getDirOfAnisotropy() NxMaterialDesc.dirOfAnisotropy setFlags()
	*/
	virtual		void			setDirOfAnisotropy(const NxVec3 &vec) = 0;

	/**
	\brief Retrieves the direction of anisotropy value.

	\return The direction of anisotropy.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setDirOfAnisotropy() NxMaterialDesc.dirOfAnisotropy setFlags()
	*/
	virtual		NxVec3			getDirOfAnisotropy() const = 0;

	/**
	\brief Sets the flags, a combination of the bits defined by the enum ::NxMaterialFlag . 

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] flags #NxMaterialFlag combination.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getFlags() NxMaterialFlag
	*/
	virtual		void			setFlags(NxU32 flags) = 0;

	/**
	\brief Retrieves the flags. See #NxMaterialFlag.

	\return The material flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterialFlag setFlags()
	*/
	virtual		NxU32			getFlags() const = 0;

	/**
	\brief Sets the friction combine mode.
	
	See the enum ::NxCombineMode .

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] combMode Friction combine mode to set for this material. See #NxCombineMode.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCombineMode getFrictionCombineMode setStaticFriction() setDynamicFriction()
	*/
	virtual		void			setFrictionCombineMode(NxCombineMode combMode) = 0;

	/**
	\brief Retrieves the friction combine mode.
	
	See #setFrictionCombineMode.

	\return The friction combine mode for this material.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCombineMode setFrictionCombineMode() 
	*/
	virtual		NxCombineMode	getFrictionCombineMode() const = 0;

	/**
	\brief Sets the restitution combine mode.
	
	See the enum ::NxCombineMode .

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	\param[in] combMode Restitution combine mode for this material. See #NxCombineMode.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCombineMode getRestitutionCombineMode() setRestitution()
	*/
	virtual		void			setRestitutionCombineMode(NxCombineMode combMode) = 0;

	/**
	\brief Retrieves the restitution combine mode.
	
	See #setRestitutionCombineMode.

	\return The coefficient of restitution combine mode for this material.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCombineMode setRestitutionCombineMode getRestitution()
	*/
	virtual		NxCombineMode	getRestitutionCombineMode() const = 0;

	//public variables:
				void*			userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	};


//typedef NxMaterial * NxMaterialIndex;   //legacy support (problematic because the size used to be 2 bytes)

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
