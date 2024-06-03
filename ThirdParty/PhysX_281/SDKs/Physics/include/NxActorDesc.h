#ifndef NX_PHYSICS_NXACTORDESC
#define NX_PHYSICS_NXACTORDESC
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

#include "NxBodyDesc.h"
#include "NxShapeDesc.h"

class NxCompartment;

	/**
	\brief Type of actor
	*/
	enum NxActorDescType
		{
		/**
		\brief Not used*/
		NX_ADT_SHAPELESS, 
		NX_ADT_DEFAULT,
		NX_ADT_ALLOCATOR,
		/**
		\brief Not used*/
		NX_ADT_LIST,
		/**
		\brief Not used*/
		NX_ADT_POINTER
	};

/**
	\brief Actor Descriptor. This structure is used to save and load the state of #NxActor objects.

	If the body descriptor contains a zero mass but the actor descriptor contains a non-zero density,
	we compute a new mass automatically from the density and the shapes.

	Static or dynamic actors:

	\li To create a static actor, use a null body descriptor pointer(and not a body with zero mass). Shapes should
	be specified for the static actor when it is created.
	If you want to create a temporarily static actor that can be made dynamic at runtime, create your
    dynamic actor as usual and use NX_BF_KINEMATIC flags in its body descriptor.

	\li To create a dynamic actor, provide a valid body descriptor with or without shape descriptors. The
	  shapes are not mandatory.

	Mass or density:
	
		To simulate a dynamic actor, the SDK needs a mass and an inertia tensor. 
		(The inertia tensor is the combination of bodyDesc.massLocalPose and bodyDesc.massSpaceInertia)

		These can be specified in several different ways:

		1) actorDesc.density == 0,  bodyDesc.mass > 0, bodyDesc.massSpaceInertia.magnitude() > 0

			Here the mass properties are specified explicitly, there is nothing to compute.

		2) actorDesc.density > 0,	actorDesc.shapes.size() > 0, bodyDesc.mass == 0, bodyDesc.massSpaceInertia.magnitude() == 0

			Here a density and the shapes are given. From this both the mass and the inertia tensor is computed.

		3) actorDesc.density == 0,	actorDesc.shapes.size() > 0, bodyDesc.mass > 0, bodyDesc.massSpaceInertia.magnitude() == 0

			Here a mass and shapes are given. From this the inertia tensor is computed.

		Other combinations of settings are illegal.

		When the SDK computes the inertia properties it uses the actor's shapes.  
		For each shape, the shape geometry, shape mass (or density), and shape positioning within the actor 
		are used to compute the body's mass and inertia properties.  
		
		Setting the individual masses or densities of the shapes of an actor is the most intuitive method
		to specify the mass, center of mass, and inertial properties of a multi	shape actor in order to achieve 
		correct behavior.

		If the actor body has a mass, then the computed inertial properties of the actor are scaled to the specified body mass.  
		Even then, the individual shape masses are still useful since they specify how that mass is distributed within the object.
		i.e. When specified, the actor/body mass properties have priority.

		If you specify a shape density and an actor density then the two are multiplied together to obtain the effective density
		of the actor. So if you set the density of the actor to 2 and the density of the shape to 3 then the effective density 
		of that shape will be 6.

		When the sdk computes the inertial properties the default is to also compute the center of mass.
		To specify an actor's center of mass exactly but still let the SDK compute and diagonalize the inertia 
		tensor raise NxActorDescBase::flags | NX_AF_LOCK_COM.  In this case the center of mass (bodydesc.massLocalPose.t)
		will not be overridden and the computed inertial tensor will be transformed correctly so that
		it will be as if the volume integrals for the moments had been done from the specified point.

		The NX_AF_LOCK_COM is only considered when computing the mass properties during creation, it will
		not be considered when calling NxActor::updateMassFromShapes().

	You should not use the NxActorDescBase class directly, instead use one of the derived classes,
	NxActorDesc or NxActorDesc_Template.
*/

class NxActorDescBase
	{
	public:
	/**
	\brief The pose of the actor in the world.

	<b>Range:</b> rigid body transform<br>
	<b>Default:</b> Identity Matrix

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor::setGlobalPose()
	*/
	NxMat34					globalPose; 
	
	/**
	\brief Body descriptor, null for static actors

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc
	*/
	const NxBodyDesc*		body;
	
	/**
	\brief Density used during mass/inertia computation.
	
	We can compute the mass from a density and the shapes mass/density.
	
	See the notes for #NxActorDesc for more details.
	
	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 0.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActorDesc
	*/
	NxReal					density;
	
	/**
	\brief Combination of ::NxActorFlag flags

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor::raiseActorFlag()
	*/
	NxU32					flags;
	
	/**
	\brief The actors group.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor::setGroup()
	*/
	NxActorGroup			group;

	/**
	\brief Dominance group for this actor.
	
	NxDominanceGroup is a 5 bit group identifier (legal range from 0 to 31).
	The NxScene::setDominanceGroupPair() lets you set certain behaviors for pairs of dominance groups.
	By default every actor is created in group 0.  Static actors must stay in group 0.

	<b>Default:</b> 0
	*/
	NxDominanceGroup dominanceGroup;	

	/**
	\brief Combination of ::NxContactPairFlag flags

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor::setContactReportFlags()
	*/
	NxU32					contactReportFlags;
	
	/**
	\brief Force Field Material Index, index != 0 has to be created.

	<b>Default:</b> 0
	*/
	NxU16					forceFieldMaterial;

	/**
	\brief Will be copied to NxActor::userData

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor.userData
	*/
	void*					userData; 
	
	/**
	\brief Possible debug name. The string is not copied by the SDK, only the pointer is stored.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	<b>Default:</b> NULL
	*/
	const char*				name;

	/**
	\brief The compartment to place the actor in. Must be either a pointer to an NxCompartment of type NX_SCT_RIGIDBODY, or NULL.
	A NULL compartment means creating the actor in the scene proper.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	<b>Default:</b> NULL
	*/
	NxCompartment *			compartment;

	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	\brief Returns true if the descriptor is valid.

	\return True if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	/**
	\brief Retrieve the actor type.

	\return The actor desc type. See #NxActorDescType
	*/
	NX_INLINE NxActorDescType getType() const;
	protected:
	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxActorDescBase();	

	NX_INLINE bool isValidInternal(bool hasSolidShape) const;

	NxActorDescType			type;
	};

/**
\brief Actor Descriptor. This structure is used to save and load the state of #NxActor objects.

Legacy implementation that works with existing code but does not permit the user
to supply his own allocator for NxArray<NxShapeDesc*>	shapes.
*/
class NxActorDesc : public NxActorDescBase
	{
	public:

	/**
	\brief Shapes composing the actor.

	Shapes should always be specified for static actors during creation. However it is optional for dynamic actors.

	See #NxActor.createShape() for additional notes and limitations.

	<b>Default:</b> empty

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes 
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxArray<NxShapeDesc*, NxAllocatorDefault>	shapes;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxActorDesc();	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	\brief Returns true if the descriptor is valid.

	\return True if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

/**
\brief Implementation of an actor descriptor that permits the user to supply his own allocator
*/
template<class AllocType = NxAllocatorDefault> class NxActorDesc_Template : public NxActorDescBase
	{
	public:

	/**
	\brief Shapes composing the actor

	Shapes should always be specified for static actors during creation. However it is optional for dynamic actors.

	See #NxActor.createShape() for additional notes and limitations.

	<b>Default:</b> empty

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes 
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxArray<NxShapeDesc*, AllocType>	shapes;



	NX_INLINE NxActorDesc_Template()
		{
		setToDefault();
		type = NX_ADT_ALLOCATOR;
		}

	NX_INLINE void setToDefault()
		{
		NxActorDescBase::setToDefault();
		shapes.clear();
		}

	NX_INLINE bool isValid() const
		{
		if (!NxActorDescBase::isValid())
			return false;

		unsigned int nNonTriggerShapes = 0;

		// Static actors need nothing but a shape
		if (!body && shapes.size() > 0)
			return true;
		for (unsigned i = 0; i < shapes.size(); i++)
			{
			if (!shapes[i]->isValid())
				return false;
			if ((shapes[i]->shapeFlags & NX_TRIGGER_ENABLE) == 0)
				nNonTriggerShapes++;
			}

		// If Actor is dynamic (body && !(body->flags & NX_BF_KINEMATIC) but has no solid shapes,
		// it has to have mass and massSpaceInertia, otherwise NxScene::createActor returns 0
		if (nNonTriggerShapes == 0 && body && (!(body->flags & NX_BF_KINEMATIC)) && (body->mass < 0 || body->massSpaceInertia.isZero()))
			return false;

		if (!NxActorDescBase::isValidInternal(nNonTriggerShapes > 0))
			return false;

		return true;
		}


	};
	


NX_INLINE NxActorDescBase::NxActorDescBase()
	{
	//nothing!  Don't call setToDefault() here!
	}


NX_INLINE void NxActorDescBase::setToDefault()
	{
	body		= NULL;
	density		= 0.0f;
	globalPose	.id();
	flags		= 0;
	userData	= NULL;
	name		= NULL;
	group		= 0;
	dominanceGroup = 0;
	contactReportFlags = 0;
	forceFieldMaterial = 0;
	compartment = NULL;
	}

NX_INLINE bool NxActorDescBase::isValid() const
	{
	if (density < 0)
		return false;

	if (body && !body->isValid())
		return false;
	if(!globalPose.isFinite())
		return false;
	if (!body && dominanceGroup)	//only dynamic actors may have a nonzero dominance group.
		return false;

	return true;
	}

NX_INLINE NxActorDescType NxActorDescBase::getType() const			
	{	
	return type; 
	}

NX_INLINE bool NxActorDescBase::isValidInternal(bool hasSolidShape) const
	{
	bool haveDensity = density!=0.0f;
	bool haveMass = body && body->mass != 0.0f;
	bool haveTensor = body && !(body->massSpaceInertia.isZero() > 0.0f);

	if      (hasSolidShape && haveDensity && !haveMass && !haveTensor) return true;
	else if (hasSolidShape && !haveDensity && haveMass && !haveTensor) return true;
	else if (!haveDensity && haveMass && haveTensor) return true;
	else return false;
	}




NX_INLINE NxActorDesc::NxActorDesc()
	{
		memset(this,0,sizeof(NxActorDesc));
	setToDefault();
	type = NX_ADT_DEFAULT;
	}

NX_INLINE void NxActorDesc::setToDefault()
	{
	NxActorDescBase::setToDefault();
	shapes		.clear();
	}

NX_INLINE bool NxActorDesc::isValid() const
	{
	if (!NxActorDescBase::isValid())
		return false;

	unsigned int nNonTriggerShapes = 0;

	// Static actors need nothing but a shape
	if (!body && shapes.size() > 0)
		return true;
	for (unsigned i = 0; i < shapes.size(); i++)
	{
		if (!shapes[i]->isValid())
			return false;
		if ((shapes[i]->shapeFlags & NX_TRIGGER_ENABLE) == 0)
			nNonTriggerShapes++;
	}

	// If Actor is dynamic (body && !(body->flags & NX_BF_KINEMATIC) but has no solid shapes,
	// it has to have mass and massSpaceInertia, otherwise NxScene::createActor returns 0
	if (nNonTriggerShapes == 0 && body && (!(body->flags & NX_BF_KINEMATIC)) && (body->mass < 0 || body->massSpaceInertia.isZero()))
		return false;

	if (!NxActorDescBase::isValidInternal(nNonTriggerShapes > 0))
		return false;

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
