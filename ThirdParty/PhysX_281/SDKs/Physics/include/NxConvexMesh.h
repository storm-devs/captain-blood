#ifndef NX_COLLISION_NXCONVEXMESH
#define NX_COLLISION_NXCONVEXMESH
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

#include "Nx.h"
#include "NxTriangleMesh.h"

class NxSimpleTriangleMesh;
class NxConvexMeshDesc;
class NxStream;

/**
\brief A Convex Mesh.

Internally represented as a list of convex polygons. The number
of polygons is limited to 256.

To avoid duplicating data when you have several instances of a particular 
mesh positioned differently, you do not use this class to represent a 
convex object directly. Instead, you create an instance of this mesh via
the NxConvexShape class.

<h3>Creation</h3>

<h3>Creation</h3>

To create an instance of this class call NxPhysicsSDK::createConvexMesh(),
and NxPhysicsSDK::releaseConvexMesh() to delete it. This is only possible
once you have released all of its #NxConvexShape instances.

Example:

\include NxConvexMesh_Create.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_COLLISION_AABBS
\li #NX_VISUALIZE_COLLISION_SHAPES
\li #NX_VISUALIZE_COLLISION_AXES
\li #NX_VISUALIZE_COLLISION_VNORMALS
\li #NX_VISUALIZE_COLLISION_FNORMALS
\li #NX_VISUALIZE_COLLISION_EDGES
\li #NX_VISUALIZE_COLLISION_SPHERES

@see NxConvexMeshDesc NxPhysics.createConvexMesh()
*/

class NxConvexMesh
	{
	public:
	/**
	\brief Saves the mesh to a descriptor.

	\param[out] desc Descriptor to store the state of the convex mesh into.
	
	\return true on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxConvexMeshDesc 
	*/
	virtual	bool				saveToDesc(NxConvexMeshDesc& desc)	const	= 0;

	/**
	\brief Not used.

	*/
	virtual NxU32				getSubmeshCount()							const	= 0;

	/**
	\brief Retrieves the number of elements of a given internal array.

	\param[in] submeshIndex Reserved for future use, must be 0.
	\param[in] intArray The internal array size to retrieve (e.g. triangles, vertices etc). See #NxInternalArray.
	
	\return The number of elements in the internal array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see getFormat() getBase() getStride()
	*/
	virtual NxU32				getCount(NxSubmeshIndex submeshIndex, NxInternalArray intArray)	const	= 0;

	/**
	\brief Retrieves the format of a given internal array.

	\param[in] submeshIndex Reserved for future use, must be 0.
	\param[in] intArray The internal array format to retrieve (e.g. triangles, vertices etc). See #NxInternalArray.
	
	\return The format of the internal array. See #NxInternalFormat.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCount() getBase() getStride()
	*/
	virtual NxInternalFormat	getFormat(NxSubmeshIndex submeshIndex, NxInternalArray intArray)	const	= 0;

	/**
	\brief Retrieves the base pointer of a given internal array.

	Make sure you take into account the given format and stride.

	@see getStride
	@see getFormat

	\param[in] submeshIndex Reserved for future use, must be 0.
	\param[in] intArray The internal array pointer to retrieve (e.g. triangles, vertices etc). See #NxInternalArray.
	
	\return A pointer to the first element of the internal array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCount() getFormat() getStride()
	*/
	virtual const void*			getBase(NxSubmeshIndex submeshIndex, NxInternalArray intArray)	const	= 0;

	/**
	\brief Retrieves the stride value of a given internal array.

	The stride value is always a number of bytes. You have to skip this number of bytes
	to go from one element to the other in an array, starting from the base.

	\param[in] submeshIndex Reserved for future use, must be 0.
	\param[in] intArray The internal array stride to retrieve (e.g. triangles, vertices etc). See #NxInternalArray.
	
	\return The stride(number of bytes from one element to the next) for the internal array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see getCount() getFormat() getBase()
	*/
	virtual NxU32				getStride(NxSubmeshIndex submeshIndex, NxInternalArray intArray)	const	= 0;

	/**
	\brief Load the contents of this mesh from the provided stream.

	A stream of an appropriate format can be creating with the cooking library.

	\param[in] stream Stream to load this mesh from. See #NxStream.
	\return True if successfully loaded. Otherwise False.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback for > 32 faces or vertices)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	bool				load(const NxStream& stream)		= 0;

	/**
	\brief Returns the reference count for shared meshes.

	\return the current reference count, not used in any shapes if the count is 0.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32				getReferenceCount()					= 0;

	/**
	\brief Returns the mass properties of the mesh.

	\param[out] mass The mass of the mesh.
	\param[out] localInertia The inertia tensor in mesh local space.
	\param[out] localCenterOfMass Position of center of mass in mesh local space.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void				getMassInformation(NxReal& mass, NxMat33& localInertia, NxVec3& localCenterOfMass)		const	= 0;
	

	// TODO(djs): remove
	virtual void *				getInternal()						= 0;

	protected:
	virtual ~NxConvexMesh(){};
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
