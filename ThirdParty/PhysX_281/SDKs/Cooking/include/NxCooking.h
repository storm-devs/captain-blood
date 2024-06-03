#ifndef NX_COOKING_H
#define NX_COOKING_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "Nxc.h"
#include "Nxf.h"
#include "NxArray.h"
class NxUserAllocator;
class NxUserOutputStream;
class NxTriangleMeshDesc;
class NxConvexMeshDesc;
class NxStream;
class NxTriangleMeshShape;
class NxClothMeshDesc;
class NxSoftBodyMeshDesc;
class NxTriangle;

	class NxVec3;
	class NxPlane;
	class NxConvexMeshDesc2;

	enum NxPlatform
		{
		PLATFORM_PC,
		PLATFORM_XENON,
		PLATFORM_PLAYSTATION3
		};

	/**

	\brief Structure describing parameters affecting mesh cooking.

	@see NxSetCookingParams() NxGetCookingParams()
	*/
	struct NxCookingParams
		{
		/**
		\brief Target platform

		Should be set to the platform which you intend to load the cooked mesh data on. This allows
		the SDK to optimize the mesh data in an appropriate way for the platform and make sure that
		endianness issues are accounted for correctly.

		<b>Default value:</b> Same as the platform on which the SDK is running.
		*/
		NxPlatform	targetPlatform;

		/**
		\brief Skin width for convexes

		Specifies the amount to inflate the convex mesh by when the new convex hull generator is used.

		Inflating the mesh allows the user to hide interpenetration errors by increasing the size of the
		collision mesh with respect to the size of the rendered geometry.

		<b>Default value:</b> 0.025f
		*/
		float		skinWidth;

		/**
		\brief Hint to choose speed or less memory for collision structures

		<b>Default value:</b> false
		*/
		bool		hintCollisionSpeed;
		};

	/**
	\brief Sets cooking parameters

	\note #NxInitCooking() sets the parameters to their default values.

	\param[in] params Cooking parameters

	\return true on success.

	@see NxCookingParams NxGetCookingParams()
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NX_CALL_CONV NxSetCookingParams(const NxCookingParams& params);

	/**
	\brief Gets cooking parameters

	\return Current cooking parameters.

	@see NxCookingParams NxSetCookingParams()
	*/
	NX_C_EXPORT NXC_DLL_EXPORT const NxCookingParams& NX_CALL_CONV NxGetCookingParams();

	/**
	\brief Checks endianness is the same between cooking & target platforms

	\return True if there is and endian mismatch.
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NX_CALL_CONV NxPlatformMismatch();

	/**
	\brief Initializes cooking.

	This must be called at least once, before any cooking method is called (otherwise cooking fails) and
	should be matched with a call to NxCloseCooking() before you remove the allocator or output stream
	objects.

	Please note that this also initializes the Foundation SDK. This is not an issue, unless you are linking
	the AGEIA PhysX SDK using static libraries. When you are using static libraries, the cooking library
	will share the same Foundation SDK as the rest of the application and this you will have to consider
	when initializing the cooking library. In statically linked applications, you should specify the same
	allocator and error stream to NxInitCooking() and NxCreatePhysicsSDK(). You are not compelled to
	do so, but should be aware of the possible issues that could arise if you specify different values.
	A common error would be to specify a user error stream when creating the Physics SDK, but omit it
	when initializing the cooking, thus resulting in no error stream if the cooking is initialized last.

	The previous state of the cooking initialization is stored in a stack each time you call NxInitCooking()
	and when you call NxCloseCooking() the previous state is activated again. The "state" that is saved is the 
	allocator and output stream settings. The stack size is currently 32 states, which means that you can 
	not call NxInitCooking() 33 consecutive times without at least one call to NxCloseCooking() in between.

	Note: Cooking parameters (as set by NxSetCookingParams) are reset by this function. You should call NxSetCookingParams
	after this function, not before.

	\param[in] allocator The memory allocator to use.
	\param[in] outputStream The output stream to use.
	\return true on success.

	@see NxCloseCooking()
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NX_CALL_CONV NxInitCooking(NxUserAllocator* allocator = NULL, NxUserOutputStream* outputStream = NULL);


	/**
	\brief Closes cooking.

	This must be called at the end of your app, to release cooking-related data.

	@see NxInitCooking()
	*/
	NX_C_EXPORT NXC_DLL_EXPORT void NX_CALL_CONV NxCloseCooking();

	/**
	\brief Cooks a triangle mesh. The results are written to the stream.

	To create a triangle mesh object(unlike previous versions) it is necessary to first 'cook' the mesh data into
	a form which allows the SDK to perform efficient collision detection.

	NxCookTriangleMesh() and NxCookConvexMesh() allow a mesh description to be cooked into a binary stream
	suitable for loading and performing collision detection at runtime.

	\note #NxInitCooking() must be called before attempting to cook a mesh. NxCloseCooking() should be called
	when the application has finished using the cooking library.

	Example

	\include NxCookTriangleMesh_Example.cpp

	\param[in] desc The triangle mesh descriptor to read the mesh from.
	\param[in] stream User stream to output the cooked data.
	\return true on success

	@see NxCookTriangleMesh() NxInitCooking() NxSetCookingParams()
	*/


	NX_C_EXPORT NXC_DLL_EXPORT bool NX_CALL_CONV NxCookTriangleMesh(const NxTriangleMeshDesc& desc, NxStream& stream);

	/**
	\brief Cooks a convex mesh. The results are written to the stream.

	To create a triangle mesh object(unlike previous versions) it is necessary to first 'cook' the mesh data into
	a form which allows the SDK to perform efficient collision detection.

	NxCookTriangleMesh() and NxCookConvexMesh() allow a mesh description to be cooked into a binary stream
	suitable for loading and performing collision detection at runtime.

	NxCookConvex requires the input mesh to form a closed convex volume. This allows more efficient and robust
	collision detection. The input mesh is not validated to make sure that the mesh is convex.

	\note #NxInitCooking() must be called before attempting to cook a mesh. NxCloseCooking() should be called
	when the application has finished using the cooking library.

	Example

	\include NxCookConvexMesh_Example.cpp

	\param[in] desc The convex mesh descriptor to read the mesh from.
	\param[in] stream User stream to output the cooked data.
	\return true on success

	@see NxCookTriangleMesh() NxInitCooking() NxSetCookingParams()
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NX_CALL_CONV NxCookConvexMesh(const NxConvexMeshDesc& desc, NxStream& stream);
	
	/**
	\brief Scales an existing cooked convex mesh and outputs it into another stream.

	\note #NxInitCooking() must be called before attempting to cook a mesh. #NxCloseCooking() should be called
	when the application has finished using the cooking library.

	\param[in] source The source cooked convex mesh to scale.
	\param[in] scale The uniform scale factor to apply to the convex mesh.
	\param[in] dest User stream to output the cooked data.
	\return true on success

	@see NxCookConvexMesh() NxCookTriangleMesh() NxInitCooking() NxSetCookingParams()
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NX_CALL_CONV NxScaleCookedConvexMesh(const NxStream& source, NxReal scale, NxStream& dest);


	/**
	\brief Report state of cooking memory usage.
	*/
	NX_C_EXPORT NXC_DLL_EXPORT void NX_CALL_CONV NxReportCooking();


	/**
	\brief Cooks a triangle mesh to a ClothMesh.

	\param desc The cloth mesh descriptor on which the generation of the cooked mesh depends.
	\param stream The stream the cooked mesh is written to.
	\return True if cooking was successful
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NX_CALL_CONV NxCookClothMesh(const NxClothMeshDesc& desc, NxStream& stream);

	/**
	\brief Cooks a tetrahedral mesh to a SoftBodyMesh.

	\param desc The soft body mesh descriptor on which the generation of the cooked mesh depends.
	\param stream The stream the cooked mesh is written to.
	\return True if cooking was successful
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NX_CALL_CONV NxCookSoftBodyMesh(const NxSoftBodyMeshDesc& desc, NxStream& stream);

class NxPMap;
class NxTriangleMesh;
class NxUserOutputStream;

class NxCookingInterface
{
public:
	/**
	\brief Sets cooking parameters

	\note #NxInitCooking() sets the parameters to their default values.

	\param[in] params Cooking parameters

	\return true on success.

	@see NxCookingParams NxGetCookingParams()
	*/
	virtual bool  NxSetCookingParams(const NxCookingParams& params) = 0;

	/**
	\brief Gets cooking parameters

	\return Current cooking parameters.

	@see NxCookingParams NxSetCookingParams()
	*/
	virtual const NxCookingParams& NxGetCookingParams() = 0;

	/**
	\brief Checks endianness is the same between cooking & target platforms

	\return True if there is and endian mismatch.
	*/
	virtual bool  NxPlatformMismatch() = 0;

	/**
	\brief Initializes cooking.

	This must be called at least once, before any cooking method is called (otherwise cooking fails) and
	should be matched with a call to NxCloseCooking() before you remove the allocator or output stream
	objects.

	Please note that this also initializes the Foundation SDK. This is not an issue, unless you are linking
	the AGEIA PhysX SDK using static libraries. When you are using static libraries, the cooking library
	will share the same Foundation SDK as the rest of the application and this you will have to consider
	when initializing the cooking library. In statically linked applications, you should specify the same
	allocator and error stream to NxInitCooking() and NxCreatePhysicsSDK(). You are not compelled to
	do so, but should be aware of the possible issues that could arise if you specify different values.
	A common error would be to specify a user error stream when creating the Physics SDK, but omit it
	when initializing the cooking, thus resulting in no error stream if the cooking is initialized last.

	The previous state of the cooking initialization is stored in a stack each time you call NxInitCooking()
	and when you call NxCloseCooking() the previous state is activated again. The "state" that is saved is the 
	allocator and output stream settings. The stack size is currently 32 states, which means that you can 
	not call NxInitCooking() 33 consecutive times without at least one call to NxCloseCooking() in between.

	Note: Cooking parameters (as set by NxSetCookingParams) are reset by this function. You should call NxSetCookingParams
	after this function, not before.

	\param[in] allocator The memory allocator to use.
	\param[in] outputStream The output stream to use.
	\return true on success.

	@see NxCloseCooking()
	*/
	virtual bool  NxInitCooking(NxUserAllocator* allocator = NULL, NxUserOutputStream* outputStream = NULL) = 0;

	/**
	\brief Closes cooking.

	This must be called at the end of your app, to release cooking-related data.

	@see NxInitCooking()
	*/
	virtual void  NxCloseCooking() = 0;

	/**
	\brief Cooks a triangle mesh. The results are written to the stream.

	To create a triangle mesh object(unlike previous versions) it is necessary to first 'cook' the mesh data into
	a form which allows the SDK to perform efficient collision detection.

	NxCookTriangleMesh() and NxCookConvexMesh() allow a mesh description to be cooked into a binary stream
	suitable for loading and performing collision detection at runtime.

	NxCookConvex requires the input mesh to form a closed convex volume. This allows more efficient and robust
	collision detection.

	\note #NxInitCooking() must be called before attempting to cook a mesh. NxCloseCooking() should be called
	when the application has finished using the cooking library.

	Example

	\include NxCookTriangleMesh_Example.cpp

	\param[in] desc The triangle mesh descriptor to read the mesh from.
	\param[in] stream User stream to output the cooked data.
	\return true on success

	@see NxCookTriangleMesh() NxInitCooking() NxSetCookingParams()
	*/
	virtual bool  NxCookTriangleMesh(const NxTriangleMeshDesc& desc, NxStream& stream) = 0;

	/**
	\brief Cooks a convex mesh. The results are written to the stream.

	To create a triangle mesh object(unlike previous versions) it is necessary to first 'cook' the mesh data into
	a form which allows the SDK to perform efficient collision detection.

	NxCookTriangleMesh() and NxCookConvexMesh() allow a mesh description to be cooked into a binary stream
	suitable for loading and performing collision detection at runtime.

	\note #NxInitCooking() must be called before attempting to cook a mesh. NxCloseCooking() should be called
	when the application has finished using the cooking library.

	Example

	\include NxCookConvexMesh_Example.cpp

	\param[in] desc The convex mesh descriptor to read the mesh from.
	\param[in] stream User stream to output the cooked data.
	\return true on success

	@see NxCookTriangleMesh() NxInitCooking() NxSetCookingParams()
	*/
	virtual bool  NxCookConvexMesh(const NxConvexMeshDesc& desc, NxStream& stream)= 0;


	/**
	\brief Cooks a triangle mesh to a ClothMesh.

	\param desc The cloth mesh descriptor on which the generation of the cooked mesh depends.
	\param stream The stream the cooked mesh is written to.
	\return True if cooking was successful
	*/
	virtual bool  NxCookClothMesh(const NxClothMeshDesc& desc, NxStream& stream) = 0;

	/**
	\brief Cooks a tetrahedral mesh to a SoftBodyMesh.

	\param desc The soft body mesh descriptor on which the generation of the cooked mesh depends.
	\param stream The stream the cooked mesh is written to.
	\return True if cooking was successful
	*/
	virtual bool  NxCookSoftBodyMesh(const NxSoftBodyMeshDesc& desc, NxStream& stream) = 0;

	/**
	\brief Creates a PMap from a triangle mesh.

	\warning Legacy function

	A PMap is an optional data structure which makes mesh-mesh collision 
	detection more robust at the cost of higher	memory consumption.

	This structure can then be assigned to NxTriangleMeshDesc::pmap or passed to NxTriangleMesh::loadPMap().

	You may wish to store the PMap on disk (just write the above data block to a file of your choice) after
	computing it because the creation process can be quite expensive. Then you won't have to create it the next time
	you need it.

	\param[out] pmap Used to store details of the created PMap.
	\param[in] mesh Mesh to create PMap from.
	\param[in] density The density(resolution) of the PMap.
	\param[in] outputStream User supplied interface for reporting errors and displaying messages(see NxUserOutputStream)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPMap NxTriangleMesh.loadPMap() NxConvexShape.loadPMap() NxReleasePMap
	*/
	virtual bool  NxCreatePMap(NxPMap& pmap, const NxTriangleMesh& mesh, NxU32 density, NxUserOutputStream* outputStream = NULL)=0;

	/**
	\brief Releases PMap previously created with NxCreatePMap.

	\warning Legacy function

	You should not call this on PMap data you have loaded from
	disc yourself. Don't release a PMap while it is being used by a NxTriangleMesh object.

	\param[in] pmap Pmap to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPMap NxTriangleMesh.loadPMap() NxConvexShape.loadPMap() NxCreatePMap
	*/
	virtual bool  NxReleasePMap(NxPMap& pmap)=0;

	/**
	\brief Scales an existing cooked convex mesh and outputs it into another stream.

	\param[in] source The source cooked convex mesh to scale.
	\param[in] scale The uniform scale factor to apply to the convex mesh.
	\param[in] dest User stream to output the cooked data.
	\return true on success

	\note #NxInitCooking() must be called before attempting to cook a mesh. #NxCloseCooking() should be called
	when the application has finished using the cooking library.

	@see NxCookConvexMesh() NxCookTriangleMesh() NxInitCooking() NxSetCookingParams()
	*/
	virtual bool  NxScaleCookedConvexMesh(const NxStream& source, NxReal scale, NxStream& dest)=0;

	/**
	\brief Report state of cooking memory usage.
	*/
	virtual void  NxReportCooking()=0;

	protected:
	virtual ~NxCookingInterface(){};
};


#endif

//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
