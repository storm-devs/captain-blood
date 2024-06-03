#ifndef NX_PHYSICS_NXRAYCAST
#define NX_PHYSICS_NXRAYCAST
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
class NxShape;


/**
Used to specify which types(static or dynamic) of shape to test against when used with raycasting and 
overlap test methods in #NxScene.

@see NxScene NxScene.raycastAllBounds() NxScene.raycastAllShapes() NxScene.raycastClosestBounds()
NxScene.raycastClosestShape() NxScene.raycastAnyBounds() NxScene.raycastAnyShape() NxScene.overlapSphereShapes()
NxScene.overlapAABBShapes()
*/
enum NxShapesType
	{
	NX_STATIC_SHAPES		= (1<<0),								//!< Hits static shapes
	NX_DYNAMIC_SHAPES		= (1<<1),								//!< Hits dynamic shapes
	NX_ALL_SHAPES			= NX_STATIC_SHAPES|NX_DYNAMIC_SHAPES	//!< Hits both static & dynamic shapes
	};

/**

Specifies which members of #NxRaycastHit which should be generated(when used as hint flags for raycasting methods)
or which members have been generated when checking the flags member of #NxRaycastHit.

@see NxRaycastHit
*/
enum NxRaycastBit
	{
	NX_RAYCAST_SHAPE		= (1<<0),								//!< "shape" member of #NxRaycastHit is valid
	NX_RAYCAST_IMPACT		= (1<<1),								//!< "worldImpact" member of #NxRaycastHit is valid
	NX_RAYCAST_NORMAL		= (1<<2),								//!< "worldNormal" member of #NxRaycastHit is valid
	NX_RAYCAST_FACE_INDEX	= (1<<3),								//!< "faceID" member of #NxRaycastHit is valid
	NX_RAYCAST_DISTANCE		= (1<<4),								//!< "distance" member of #NxRaycastHit is valid
	NX_RAYCAST_UV			= (1<<5),								//!< "u" and "v" members of #NxRaycastHit are valid
	NX_RAYCAST_FACE_NORMAL	= (1<<6),								//!< Same as NX_RAYCAST_NORMAL but computes a non-smoothed normal
	NX_RAYCAST_MATERIAL		= (1<<7),								//!< "material" member of #NxRaycastHit is valid
	};

/**
\brief This structure captures results for a single raycast query.

All members of the NxRaycastHit structure are not always available. For example when the ray hits a sphere,
the faceID member is not computed. Also, when raycasting against bounds (AABBs) instead of actual shapes,
some members are not available either. 

Some members like barycentric coordinates are currently only computed for triangle meshes and convexes, but next versions
might provide them in other cases. The client code should check #flags to make sure returned values are
relevant.

When used as hint flags in raycasting queries, those bits control what the user would like to see computed
in the NxRaycastHit structure. For example, you might tell the SDK to compute impact normals using the
NX_RAYCAST_NORMAL hint flag. It's usually faster to let the SDK do it, instead of fetching relevant data
again in the client app. However all users may not need this information. In this case, just omit this flag
and corresponding member of ::NxRaycastHit will not be computed internally - saving processing time. 

Note that NX_RAYCAST_NORMAL computes a smoothed normal, while NX_RAYCAST_FACE_NORMAL only returns the usual
normal of a triangle (i.e. the normalized cross product of two edges).

See #NxScene and #NxShape for raycasting methods.

 <b>Threading:</b> It is not necessary to make this class thread safe as it will only be called in the context of the
 user thread.


@see NxScene.raycastAllBounds
@see NxScene.raycastAllShapes
@see NxScene.raycastClosestBounds
@see NxScene.raycastClosestShape
*/
struct NxRaycastHit
	{
	/**
	Touched shape (associated flags: NX_RAYCAST_SHAPE)

	@see flags
	*/
	NxShape*		shape;

	/**
	Impact point in world space (associated flags: NX_RAYCAST_IMPACT)

	@see flags
	*/
	NxVec3			worldImpact;
					
	/**
	Impact normal in world space (associated flags: NX_RAYCAST_NORMAL / NX_RAYCAST_FACE_NORMAL)

	For #NxConvexShape and #NxTriangleMeshShape NX_RAYCAST_NORMAL generates a smooth normal. If a true
	face normal is required use NX_RAYCAST_FACE_NORMAL instead when specifying hint flags to a raycast method.

	@see flags
	*/
	NxVec3			worldNormal;

	/**
	Index of touched face (associated flags: NX_RAYCAST_FACE_INDEX)

	The face index is for the mesh <b>before</b> cooking. During the cooking process faces are moved 
	around which changes there index. However the SDK stores a table which is used to map the index to the original
	mesh before it is returned for a raycast hit.

	@see flags
	*/
	NxU32			faceID;	
	NxTriangleID	internalFaceID;	

	/**
	Distance from ray start to impact point (associated flags: NX_RAYCAST_DISTANCE)

	@see flags
	*/
	NxReal			distance;
	
	/**
	Impact barycentric coordinates (associated flags: NX_RAYCAST_UV)

	@see flags
	*/
	NxReal			u,v;

	/**
	Index of touched material (associated flags: NX_RAYCAST_MATERIAL)

	@see flags
	*/
	NxMaterialIndex	materialIndex;

	/**
	Combination of ::NxRaycastBit, when a corresponding flag is set, then the member is valid.

	::NxRaycastBit flags can be passed to raycasting functions, as an optimization, to cause the SDK to
	only generate specific members of this structure.
	*/
	NxU32			flags;
	};

/**
\brief The user needs to pass an instance of this class to several of the ray casting routines in
NxScene.

Its onHit method will be called for each shape that the ray intersects.

Example:

\include NxUserRaycastReport_Usage.cpp

@see NxScene.raycastAllBounds
@see NxScene.raycastAllShapes
*/
class NxUserRaycastReport
	{
	public:
	/**
	\brief This method is called for each shape hit by the raycast.
	
	If onHit returns true, it may be called again with the next shape that was stabbed.
	If it returns false, no further shapes are returned, and the raycast is concluded.

	\note SDK state should not be modified from within onHit(). In particular objects should not
	be created or destroyed. If state modification is needed then the changes should be stored to a buffer
	and performed after the raycast.

	\param[in] hits The data corresponding to the ray intersection. See #NxRaycastHit.
	\return True to continue the raycast. False to abort.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	}{

	@see NxScene.raycastAllBounds
    @see NxScene.raycastAllShapes
	*/
	virtual bool onHit(const NxRaycastHit& hits) = 0;

	protected:
	virtual ~NxUserRaycastReport(){};
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
