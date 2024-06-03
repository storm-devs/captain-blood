#ifndef NX_COLLISION_NXTRIANGLEMESH
#define NX_COLLISION_NXTRIANGLEMESH
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
#include "NxVec3.h"
#include "NxBounds3.h"

class NxTriangleMeshShape;
class NxSimpleTriangleMesh;
class NxTriangleMeshDesc;
class NxTriangleMeshShapeDesc;
class NxPMap;
class NxStream;


/**

\brief A triangle mesh, also called a 'polygon soup'.

It is represented as an indexed triangle list. There are no restrictions on the
triangle data. 

To avoid duplicating data when you have several instances of a particular 
mesh positioned differently, you do not use this class to represent a 
mesh object directly. Instead, you create an instance of this mesh via
the NxTriangleMeshShape class.

<h3>Creation</h3>

To create an instance of this class call NxPhysicsSDK::createTriangleMesh(),
and NxPhysicsSDK::releaseTriangleMesh() to delete it. This is only possible
once you have released all of its NxTriangleMeshShape instances.


Example:

\include NxTriangleMesh_Create.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_COLLISION_AABBS
\li #NX_VISUALIZE_COLLISION_SHAPES
\li #NX_VISUALIZE_COLLISION_AXES
\li #NX_VISUALIZE_COLLISION_VNORMALS
\li #NX_VISUALIZE_COLLISION_FNORMALS
\li #NX_VISUALIZE_COLLISION_EDGES
\li #NX_VISUALIZE_COLLISION_SPHERES

@see NxTriangleMeshDesc NxTriangleMeshShape NxPhysicsSDK.createTriangleMesh()
*/

class NxTriangleMesh
	{
	public:
	/**
	\brief Saves the Mesh descriptor.

	\param[out] desc The descriptor used to retrieve the state of the object.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	bool				saveToDesc(NxTriangleMeshDesc& desc)	const	= 0;

	/**
	\brief Not used.
	*/
	virtual NxU32				getSubmeshCount()							const	= 0;

	/**
	\brief Retrieves the number of elements of a given internal array.

	\param[in] submeshIndex Reserved for future use, must be 0.
	\param[in] intArray The internal array size to retrieve(e.g. triangles, points etc). See #NxInternalArray.
	\return The number of elements in the internal array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getFormat() getStride() getBase()
	*/
	virtual NxU32				getCount(NxSubmeshIndex submeshIndex, NxInternalArray intArray)	const	= 0;

	/**
	\brief Retrieves the format of a given internal array.

	\param[in] submeshIndex Reserved for future use, must be 0.
	\param[in] intArray The internal array format to retrieve(e.g. triangles, points etc). See #NxInternalArray.
	\return The format of the internal array. See #NxInternalFormat

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCount() getStride() getBase()
	*/
	virtual NxInternalFormat	getFormat(NxSubmeshIndex submeshIndex, NxInternalArray intArray)	const	= 0;

	/**
	\brief Retrieves the base pointer of a given internal array.

	Example:

	\include NxTriangleMesh_getBase.cpp

	
	\param[in] submeshIndex Reserved for future use, must be 0.
	\param[in] intArray The internal array pointer to retrieve(e.g. triangles, points etc). See #NxInternalArray.
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
	\param[in] intArray The internal array stride to retrieve(e.g. triangles, points etc). See #NxInternalArray.
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
	\brief Retrieves the number of PhysX processor pages into which a mesh is divided.
	
	Meshes used for PhysX processor collision are divided into a number of fixed-size pages. This 
	returns the number of such pages for this mesh.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32				getPageCount() const = 0;

	/**
	\brief Retrieves the bounding box of a PhysX processor mesh page in the local space of the mesh

	\param pageIndex The index of the mesh page. Pages are indexed sequentially starting from 0.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/

	virtual NxBounds3			getPageBBox(NxU32 pageIndex) const = 0;

	/**
	\brief This call lets you supply a pmap if you have not done so at creation time.

	\warning Legacy function

	<h3>Example</h3>
	
	\include NxPMap_Create.cpp

	\param[in] pmap The PMap to apply.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMeshDesc.pmap NxPMap
	*/
	virtual	bool				loadPMap(const NxPMap& pmap) = 0;

	/**
	\brief Checks the mesh has a pmap or not.

	\warning Legacy function

	\return True if the mesh has a PMap associated with it.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMeshDesc.pmap NxPMap loadPMap()
	*/
	virtual	bool				hasPMap()					const	= 0;

	/**
	\brief Gets the size of the pmap.

	\warning Legacy function

	\return The size (in bytes) necessary to store the associated PMap.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMeshDesc.pmap NxPMap loadPMap() hasPMap()
	*/
	virtual	NxU32				getPMapSize()				const	= 0;

	/**
	\brief Gets pmap data.
	
	\warning Legacy function

	You must first query expected size with getPmapSize(), then allocate a buffer large
	enough to contain that amount of bytes, then call this function to dump data in preallocated buffer.
	The system checks that pmap.dataSize is equal to expected data size, so you must initialize that
	member as well before the query.

	\param[out] pmap Descriptor for PMap to retrieve.
	\return True if the PMap is successfully extracted.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes
	
	@see NxTriangleMeshDesc.pmap NxPMap loadPMap() hasPMap() getPMapSize()
	*/
	virtual	bool				getPMapData(NxPMap& pmap)	const	= 0;

	/**
	\brief Gets the density of the pmap.

	\warning Legacy function

	\return The density(resolution) of the PMap, on all axis.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMeshDesc.pmap NxPMap loadPMap() hasPMap() getPMapSize() hasPMap()
	*/
	virtual	NxU32				getPMapDensity()			const	= 0;

	/**
	\brief Load the triangle mesh from a stream.

	You can create an appropriate stream using the cooking library.

	\param[in] stream Stream to load triangle mesh from.
	\return True if the triangle mesh was successfully loaded.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxStream
	*/
	virtual	bool				load(const NxStream& stream)		= 0;

	/**
	\brief Returns material index of given triangle

	This function takes a post cooking triangle index.

	\param[in] triangleIndex (internal) index of desired triangle
	\return Material index, or 0xffff if no per-triangle materials are used

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxMaterialIndex		getTriangleMaterial(NxTriangleID triangleIndex)	const	= 0;

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

	protected:
	virtual ~NxTriangleMesh(){};
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
