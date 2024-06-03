#ifndef NX_PHYSICS_NX_PHYSICS_SDK
#define NX_PHYSICS_NX_PHYSICS_SDK
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
#include "NxInterface.h"
#include "NxUserAllocator.h"

class NxScene;
class NxSceneDesc;
class NxUserDebugRenderer;
class NxTriangleMesh;
class NxTriangleMeshDesc;
class NxConvexMesh;
class NxConvexMeshDesc;
class NxUserOutputStream;
class NxUserAllocator;
class NxActor;
class NxJoint;
class NxStream;
class NxFoundationSDK;
class NxCCDSkeleton;			//this class doesn't actually exist.
class NxSimpleTriangleMesh;
class NxHeightField;
class NxHeightFieldDesc;
#if NX_USE_CLOTH_API
class NxClothMesh;
#endif
#if NX_USE_SOFTBODY_API
class NxSoftBodyMesh;
#endif

enum NxCookingValue
	{
	/**
	Version numbers follow this format:

		Version = 16bit|16bit

	The high part is increased each time the format changes so much that
	pre-cooked files become incompatible with the system (and hence must
	be re-cooked)

	The low part is increased each time the format changes but the code
	can still read old files. You don't need to re-cook the data in that
	case, unless you want to make sure cooked files are optimal.
	*/
	NX_COOKING_CONVEX_VERSION_PC,
	NX_COOKING_MESH_VERSION_PC,
	NX_COOKING_CONVEX_VERSION_XENON,
	NX_COOKING_MESH_VERSION_XENON,
	NX_COOKING_CONVEX_VERSION_PLAYSTATION3,
	NX_COOKING_MESH_VERSION_PLAYSTATION3,
	};

/**
\brief SDK creation flags
*/
enum NxSDKCreationFlag
{
	/**
	\brief Disallows the use of the hardware for the application.
	
	A good example of when this flag is useful is when a client and server app must be run on the same
	machine. Under normal circumstances the SDK will lock the use of the hardware to the first application
	which attempts to use it. In the case of a client and server it is desirable to have the server run in 
	software mode and allow the client to use the hardware.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No

	@see NxPhysicsSDKDesc
	*/
	NX_SDKF_NO_HARDWARE							= (1<<0),
	NX_SDKF_EXTENDED_DESCRIPTOR					= (1<<31),
};

/**
\brief Reads an internal value (cooking format version).

\param[in] cookValue See #NxCookingValue
*/
NX_C_EXPORT NXP_DLL_EXPORT NxU32 NX_CALL_CONV NxGetValue(NxCookingValue cookValue);

/**
\brief Descriptor class for NxPhysicsSDK, primarily used for defining PhysX hardware limits
       for data shared between scenes.
*/
class NxPhysicsSDKDesc
	{
	public:
	NxU32 hwPageSize;		//!< size of hardware mesh pages. Currently only the value 65536 is supported.
	NxU32 hwPageMax;		//!< maximum number of hardware pages supported concurrently on hardware. The valid value must be power of 2.
	NxU32 hwConvexMax;		//!< maximum number of convex meshes which will be resident on hardware. The valid value must be power of 2.
	NxU32 cookerThreadMask;	//!< Thread affinity mask for the background cooker thread.  Defaults to 0 which means the SDK determines the affinity.

	/**
	\brief SDK creation flags.

	@see NxSDKCreationFlag
	*/
	NxU32 flags;

	/**
	\brief Sets the amount of GPU memory which will be reserved for the SDK. 
	
	The valid value must be power of 2. 
	Unit is megabyte.
	Default value is 128 MB. 
	*/
	NxU32 gpuHeapSize;
	
	/**
	\brief Amount of SDK GPU heap memory used by the mesh cache for fluids. 

	The valid value must be power of 2 and smaller than the GPU heap size.
	Unit is megabyte.
	The default value (0xffffffff) sets the mesh cache size to 1/8 of the GPU heap size.
	*/
	NxU32 meshCacheSize;

	/**
	\brief (re)sets the structure to the default.	
	*/

	NX_INLINE void setToDefault()
		{
		hwPageSize = 65536;
		hwConvexMax = 2048;
		hwPageMax = 256;
		flags = (NxU32) NX_SDKF_EXTENDED_DESCRIPTOR;
		cookerThreadMask = 0;
		gpuHeapSize = 128;
		meshCacheSize = 0xffffffff;
		}

	/**
	\brief Returns true if the descriptor is valid.

	\return return true if the current settings are valid
	*/

	NX_INLINE bool isValid() const
		{
		if ( hwPageSize != 65536 ) return false;
		if ( hwConvexMax & (hwConvexMax - 1) ) return false; //check if power of two
		if ( hwPageMax & (hwPageMax - 1) ) return false; //check if power of two
		if( flags & NX_SDKF_EXTENDED_DESCRIPTOR)
			{
			if ( gpuHeapSize & (gpuHeapSize - 1) ) return false; //check if power of two
			if ( (meshCacheSize != 0xffffffff) && (meshCacheSize & (meshCacheSize - 1)) ) return false;  //check if power of two
			if ( (meshCacheSize != 0xffffffff) && (meshCacheSize >= gpuHeapSize) ) return false; //check if smaller than heap
			}
		return true;
		}

	NxPhysicsSDKDesc()
		{
		setToDefault();
		}
	};


/**
	\brief Abstract singleton factory class used for instancing objects in the Physics SDK.

	In addition you can use NxPhysicsSDK to set global parameters which will effect all scenes,
	create triangle meshes and CCD skeletons.
	
	You can get an instance of this class by calling NxCreatePhysicsSDK().

	@see NxCreatePhysicsSDK() NxScene NxParameter NxTriangleMesh NxConvexMesh NxPhysicsSDK.createCCDSkeleton()
*/
class NxPhysicsSDK
	{
	protected:
			NxPhysicsSDK()	{}
	virtual	~NxPhysicsSDK()	{}

	public:

	/**
	\brief Destroys the instance it is called on.

	Use this release method to destroy an instance of this class. Be sure
	to not keep a reference to this object after calling release.
	Avoid release calls while a scene is simulating (in between simulate() and fetchResults() calls).

	Releasing an SDK will also release any scenes, triangle meshes, convex meshes, heightfields, CCD skeletons, and cloth
	meshes created through it, provided the user hasn't already done so.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCreatePhysicsSDK()
	*/
	virtual	void release() = 0;

	/**
	\brief Function that lets you set global simulation parameters.

	Returns false if the value passed is out of range for usage specified by the enum.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors which may be affected.

	See #NxParameter for a description of parameters support by hardware.

	\param[in] paramEnum Parameter to set. See #NxParameter
	\param[in] paramValue The value to set, see #NxParameter for allowable values.
	\return False if the parameter is out of range.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Partial
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParameter getParameter
	*/
	virtual bool setParameter(NxParameter paramEnum, NxReal paramValue) = 0;

	/**
	\brief Function that lets you query global simulation parameters.

	See #NxParameter for a description of parameters support by hardware.

	\param[in] paramEnum The Parameter to retrieve.
	\return The value of the parameter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Partial
	\li PS3  : Yes
	\li XB360: Yes

	@see setParameter NxParameter
	*/
	virtual NxReal getParameter(NxParameter paramEnum) const = 0;

	/**
	\brief Creates a scene.

	The scene can then create its contained entities.

	See #NxSceneDesc::simType to choose if to create a hardware or software scene.

	\param[in] sceneDesc Scene descriptor. See #NxSceneDesc
	\return The new scene object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	<b>Limitations:</b>

	The number of scenes that can be created is limited by the amount of memory available, and since this 
	amount varies dynamically as memory is allocated and deallocated by the PhysX SDK and by other software 
	components there is in general no way to statically determine the maximum number of scenes which can be 
	created at a given point in the simulation.

	However, scenes are built from lower-level objects called contexts. There is a limit of 64 contexts; a scene will 
	take 1 or 2 contexts depending on flags upon creation. A software scene (NX_SIMULATION_SW) will always take 1 context, 
	a hardware scene will take 1 context if NX_SF_RESTRICTED_SCENE is set, otherwise 2 contexts. This places an
	absolute limit on the maximum number of scenes regardless of the memory available.

	@see NxScene NxSceneDesc releaseScene()
	*/
	virtual NxScene* createScene(const NxSceneDesc& sceneDesc) = 0;

	/**
	\brief Deletes the instance passed.

	Also releases any actors, sweep caches, fluids, fluid surfaces, cloths, joints, effectors and materials created in this scene
	(if the user hasn't already done so), but not meshes or other items created via the SDK itself.

	Be sure	to not keep a reference to this object after calling release.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).
	
	Make sure to call #NxScene::shutdownWorkerThreads before releasing the scene, if you have user
	threads that poll for work (see #NxScene::pollForWork).

	\param[in] scene The scene to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene createScene() shutdownWorkerThreads()
	*/
	virtual void releaseScene(NxScene& scene) = 0;

	/**
	\brief Gets number of created scenes.

	\return The number of scenes created.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getScene()
	*/
	virtual NxU32 getNbScenes()			const	= 0;

	/**
	\brief Retrieves pointer to created scenes.

	\param[in] i The index for the scene.
	\return The scene at index i.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbScenes() NxScene
	*/
	virtual NxScene* getScene(NxU32 i)			= 0;

	/**
	\brief Creates a triangle mesh object.
	
	This can then be instanced into #NxTriangleMeshShape objects.

	\param[in] stream The triangle mesh stream.
	\return The new triangle mesh.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMesh NxStream releaseTriangleMesh() createConvexMesh()
	*/
	virtual NxTriangleMesh* createTriangleMesh(const NxStream& stream) = 0;

	/**
	\brief Destroys the instance passed.

	Be sure	to not keep a reference to this object after calling release.
	Do not release the triangle mesh before all its instances are released first!
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param[in] mesh Triangle mesh to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see createTriangleMesh NxTriangleMesh
	*/
	virtual	void	releaseTriangleMesh(NxTriangleMesh& mesh) = 0;

	/**
	\brief Number of triangle meshes.
	
	\return the number of triangle meshes.
	*/
	virtual	NxU32	getNbTriangleMeshes() const = 0;

	/**
	\brief Creates a NxHeightField object.
	
	This can then be instanced into #NxHeightFieldShape objects.
	
	\param[in] desc The descriptor to load the object from.
	\return The new height field object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see releaseHeightField() NxHeightField NxHeightFieldDesc NxHeightFieldShape
	*/
	virtual NxHeightField* createHeightField(const NxHeightFieldDesc& desc) = 0;
	
	/**
	\brief Destroys the instance passed.
	
	Be sure to not keep a reference to this object after calling release.
	Do not release the height field before all its shape instances are released first!
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).
	
	\param[in] heightField The height field to release.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see createHeightField() NxHeightField NxHeightFieldDesc NxHeightFieldShape
	*/
	virtual	void	releaseHeightField(NxHeightField& heightField) = 0;

	/**
	\brief Number of heightfields.
	
	\return the number of heightfields.
	*/
	virtual	NxU32	getNbHeightFields() const = 0;

	/**
	\brief Creates a CCD Skeleton mesh object.  

	CCD is performed with a skeleton(mesh) embedded within the object, this can be simpler than the geometry 
	used for discrete collision detection. 

	NxShape::setCCDSkeleton() should be used to associate a CCD skeleton with shapes.

	\note that stray vertices are permitted (in other words, vertices not referenced by any triangles), 
	but degenerate triangles (triangles that have a triangle index 2 or 3 times) are not. Stray vertices 
	are supported so that you can use a single vertex to do a raycast style CCD test. 

	\note The CCD skeleton should be scaled so that it is smaller than the geometry it is embedded within.
	This allows regular discrete collision detection to handle resting contact. Making the CCD skeleton too
	large in comparison to the shape it is embedded within can cause erratic behavior.
	
	\note CCDSkeletons currently must contain at most 64 vertices!

	<b>Limitations</b>
	<p>
	\li 64 vertex limit on CCD Skeletons (restriction probably lifted in a future version)
	\li Doesn't work when the static shape is a NxSphereShape, NxCapsuleShape, NxPlaneShape,NxBoxShape.
	</p>

	<h3>Visualizations:</h3>
	\li #NX_VISUALIZE_COLLISION_CCD
	\li #NX_VISUALIZE_COLLISION_SKELETONS

	\param[in] mesh The triangle mesh from which to create the CCD skeleton.
	\return The new CCD skeleton.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSimpleTriangleMesh releaseCCDSkeleton() NxShape.setCCDSkeleton()
	*/
	virtual NxCCDSkeleton* createCCDSkeleton(const NxSimpleTriangleMesh& mesh) = 0;


	/**
	\brief Creates a CCD Skeleton mesh object.  

	Same as createCCDSkeleton(NxSimpleTriangleMesh), but it creates from a memory buffer that was previously created
	with NxCCDSkeleton::save().


	\param[in] memoryBuffer the buffer to read from.
	\param[in] bufferSize size of the buffer.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see createCCDSkeleton()
	*/
	virtual NxCCDSkeleton* createCCDSkeleton(const void* memoryBuffer, NxU32 bufferSize) = 0;

	/**
	\brief Destroys the instance passed.

	Be sure	to not keep a reference to this object after calling release.
	Do not release the object before all its users are released first!
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param[in] skel The CCD Skeleton to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see createCCDSkeleton() NxShape.setCCDSkeleton()
	*/
	virtual	void	releaseCCDSkeleton(NxCCDSkeleton& skel) = 0;

	/**
	\brief Number of CCD skeletons.
	
	\return the number of CCD skeletons.
	*/
	virtual	NxU32	getNbCCDSkeletons() const = 0;

	/**
	\brief Creates a convex mesh object.
	
	This can then be instanced into #NxConvexShape objects.

	\param[in] mesh The stream to load the convex mesh from.
	\return The new convex mesh.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback for > 32 vertices or faces)
	\li PS3  : Yes
	\li XB360: Yes

	@see releaseConvexMesh() NxConvexMesh NxStream createTriangleMesh() NxConvexShape
	*/
	virtual NxConvexMesh* createConvexMesh(const NxStream& mesh) = 0;

	/**
	\brief Destroys the instance passed.

	Be sure	to not keep a reference to this object after calling release.
	Do not release the convex mesh before all its instances are released first!
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param[in] mesh The convex mesh to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see createConvexMesh() NxConvexMesh NxConvexShape
	*/
	virtual	void	releaseConvexMesh(NxConvexMesh& mesh) = 0;

	/**
	\brief Number of convex meshes.
	
	\return the number of convex meshes.
	*/
	virtual	NxU32	getNbConvexMeshes() const = 0;

#if NX_USE_CLOTH_API

	/**
	\brief Creates a cloth mesh from a cooked cloth mesh stored in a stream.

	Stream has to be created with NxCookClothMesh(). 

	\return The new cloth mesh.
	*/
	virtual NxClothMesh*				createClothMesh(NxStream& stream) = 0;

	/**
	\brief Deletes the specified cloth mesh. The cloth mesh must be in this scene.

	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param cloth Cloth to release.
	*/
	virtual void						releaseClothMesh(NxClothMesh& cloth) = 0;

	/**
	\brief Number of cloth meshes.
	
	\return the number of cloth meshes.
	*/
	virtual	NxU32						getNbClothMeshes() const = 0;

	/**
	\brief Retrieve an array of cloth meshes.

	\return an array of cloth mesh pointers with size getNbClothMeshes().
	*/
	virtual	NxClothMesh**				getClothMeshes() = 0;

#endif

#if NX_USE_SOFTBODY_API

	/**
	\brief Creates a soft body mesh from a cooked soft body mesh stored in a stream.

	Stream has to be created with NxCookSoftBodyMesh(). 

	\return The new soft body mesh.
	*/
	virtual NxSoftBodyMesh*				createSoftBodyMesh(NxStream& stream) = 0;

	/**
	\brief Deletes the specified soft body mesh. The soft body mesh must be in this scene.

	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param softBodyMesh Soft body mesh to release.
	*/
	virtual void						releaseSoftBodyMesh(NxSoftBodyMesh& softBodyMesh) = 0;

	/**
	\brief Number of soft body meshes.
	
	\return the number of soft body meshes.
	*/
	virtual	NxU32						getNbSoftBodyMeshes() const = 0;

	/**
	\brief Retrieve an array of soft body meshes.

	\return an array of soft body mesh pointers with size getNbSoftBodyMeshes().
	*/
	virtual	NxSoftBodyMesh**				getSoftBodyMeshes() = 0;

#endif

	/**
	\brief Reports the internal API version number of the SDK

	\return The internal API version information.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxU32 getInternalVersion(NxU32& apiRev, NxU32& descRev, NxU32& branchId)			const	= 0;
	virtual NxInterface* getInterface(NxInterfaceType type, int versionNumber) = 0;

    /** 
	\brief Reports the available revision of the PhysX Hardware

	\return 0 if there is no hardware present in the machine, 1 for the PhysX Athena revision 1.0 card. 
	*/

    virtual NxHWVersion getHWVersion() const = 0;

	/**
	\brief Reports the number of PPUs installed in the host system

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32 getNbPPUs() const = 0;

	/**
	\brief Retrieves the FoundationSDK instance.
	\return A reference to the Foundation SDK object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxFoundationSDK& getFoundationSDK() const = 0;

	};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright ?2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
