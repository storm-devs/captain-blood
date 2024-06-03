#ifndef NX_FOUNDATION_NXUSERALLOCATOR
#define NX_FOUNDATION_NXUSERALLOCATOR
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

/** \addtogroup foundation
  @{
*/

#include "Nx.h"

class NxUserAllocator;

/**
\brief Enum tp identify memory allocations within the SDK.
*/
enum NxMemoryType
{
	NX_MEMORY_PERSISTENT,
	NX_MEMORY_TEMP,
  NX_MEMORY_Articulation,
  NX_MEMORY_ArticulationMaximal,
  NX_MEMORY_AxisConstraint,
  NX_MEMORY_Body,
  NX_MEMORY_BoxShape,
  NX_MEMORY_CapsuleShape,
  NX_MEMORY_Cloth,
  NX_MEMORY_ClothManager,
  NX_MEMORY_ClothMesh,
  NX_MEMORY_WheelShape,
  NX_MEMORY_CollisionMap,
  NX_MEMORY_ConvexMeshBuilder,
  NX_MEMORY_ConvexMeshRuntime,
  NX_MEMORY_ConvexShape,
  NX_MEMORY_DebugRenderable,
  NX_MEMORY_Edge,
  NX_MEMORY_Fluid,
  NX_MEMORY_FluidEmitter,
  NX_MEMORY_FluidEmitterPressure,
  NX_MEMORY_FluidEmitterRate,
  NX_MEMORY_FluidPacketShape,
  NX_MEMORY_FluidElementRbElementInteraction,
  NX_MEMORY_FluidIdToIndexMap,
  NX_MEMORY_FluidParticleQueue,
  NX_MEMORY_FrictionPatch,
  NX_MEMORY_ExtraTrigData,
  NX_MEMORY_JointBreakEvent,
  NX_MEMORY_KinematicTarget,
  NX_MEMORY_LimitPlane,
  NX_MEMORY_Material,
  NX_MEMORY_NpActor,
  NX_MEMORY_NpMaterial,
  NX_MEMORY_NpCompartment,
  NX_MEMORY_NvJointBreakEvent,
  NX_MEMORY_SortedSet_NameBinding,
  NX_MEMORY_FoundationSDK,
  NX_MEMORY_NxMutex,
  NX_MEMORY_PenetrationMap,
  NX_MEMORY_PersistentPairData,
  NX_MEMORY_PhysicsSDK,
  NX_MEMORY_PlaneShape,
  NX_MEMORY_Scene,
  NX_MEMORY_SceneObjectPools,
  NX_MEMORY_NPPoolManager,
  NX_MEMORY_SphereShape,
  NX_MEMORY_SpringAndDamperEffector,
  NX_MEMORY_TriangleMesh,
  NX_MEMORY_TriangleMeshBuilder,
  NX_MEMORY_TriangleMeshShape,
  NX_MEMORY_HeightField,
  NX_MEMORY_HeightFieldShape,
  NX_MEMORY_NpPlaneShape,
  NX_MEMORY_NpSphereShape,
  NX_MEMORY_NpBoxShape,
  NX_MEMORY_NpCapsuleShape,
  NX_MEMORY_NpConvexShape,
  NX_MEMORY_NpTriangleMeshShape,
  NX_MEMORY_NpHeightFieldShape,
  NX_MEMORY_NpWheelShape,
  NX_MEMORY_NpScene,
  NX_MEMORY_NpTriangleMesh,
  NX_MEMORY_NpConvexMesh,
  NX_MEMORY_NpHeightField,
  NX_MEMORY_NpPhysicsSDK,
  NX_MEMORY_PhysicsThread,
  NX_MEMORY_AsyncSceneThread,
  NX_MEMORY_NpPrismaticJoint,
  NX_MEMORY_AsyncScene,
  NX_MEMORY_NpRevoluteJoint,
  NX_MEMORY_NpCylindricalJoint,
  NX_MEMORY_NpSphericalJoint,
  NX_MEMORY_NpPointOnLineJoint,
  NX_MEMORY_NpPointInPlaneJoint,
  NX_MEMORY_NpFixedJoint,
  NX_MEMORY_NpDistanceJoint,
  NX_MEMORY_NpPulleyJoint,
  NX_MEMORY_NpD6Joint,
  NX_MEMORY_NpSpringAndDamperEffector,
  NX_MEMORY_NpForceField,
  NX_MEMORY_NpForceFieldLinearKernel,
  NX_MEMORY_NpForceFieldShapeGroup,
  NX_MEMORY_ForceFieldShapeGroups,
  NX_MEMORY_NpFluid,
  NX_MEMORY_PrismaticJoint,
  NX_MEMORY_RevoluteJoint,
  NX_MEMORY_CylindricalJoint,
  NX_MEMORY_SphericalJoint,
  NX_MEMORY_PointOnLineJoint,
  NX_MEMORY_PointInPlaneJoint,
  NX_MEMORY_DistanceJoint,
  NX_MEMORY_PulleyJoint,
  NX_MEMORY_FixedJoint,
  NX_MEMORY_D6Joint,
  NX_MEMORY_NpFluidEmitter,
  NX_MEMORY_NpCloth,
  NX_MEMORY_NpClothMesh,
  NX_MEMORY_NpSoftBody,
  NX_MEMORY_NpSoftBodyMesh,
  NX_MEMORY_NpCCDSkeleton,
  NX_MEMORY_SourceCCDSkeleton,
  NX_MEMORY_CCDTest,
  NX_MEMORY_Array,
  NX_MEMORY_Profiler,
  NX_MEMORY_ProfilerManager,
  NX_MEMORY_NxTriangle,
  NX_MEMORY_CachedMesh,
  NX_MEMORY_BoundsMirror,
  NX_MEMORY_MirroredActor,
  NX_MEMORY_DynamicMirror,
  NX_MEMORY_HashCell,
  NX_MEMORY_RigidSceneZoner,
  NX_MEMORY_MirroredForceFieldGroup,

  NX_MEMORY_SolverCoreGeneral,
  NX_MEMORY_SolverCoreSSE,
  NX_MEMORY_NPhaseCore,
  NX_MEMORY_BroadPhase,
  NX_MEMORY_GroupSolveCoreSingle,

  //Low level
  NX_MEMORY_LowLevel,
  NX_MEMORY_LowLevelDebug,
  NX_MEMORY_LowLevelThreadingTask,
  NX_MEMORY_LowLevelThreadingThunk,
 

  NX_MEMORY_NPhaseContext,
  NX_MEMORY_GroupSolveContext,
  NX_MEMORY_NPhaseContextWithContainer,
  NX_MEMORY_NxTask,

  NX_MEMORY_NpSchedulerItem,
  NX_MEMORY_NpInternalThread,

  NX_MEMORY_RemoteDebugger,
  NX_MEMORY_VRDProfiling,
  NX_MEMORY_VRDContacts,
  NX_MEMORY_VRDData,

  NX_MEMORY_PxScene,
  NX_MEMORY_PxActor,
  NX_MEMORY_PxInteraction,
  NX_MEMORY_PxElement,
  NX_MEMORY_StaticActor,
  NX_MEMORY_InactiveActor,
  NX_MEMORY_DynamicActor,
  NX_MEMORY_JointInteraction,
  NX_MEMORY_RawBoundsElement,
  NX_MEMORY_RawBoundsInteraction,
  NX_MEMORY_PageBoundsElement,
  NX_MEMORY_PageBoundsInteraction,
  
  NX_MEMORY_SoftwareSceneGraphUpdatePCM,
  NX_MEMORY_SoftwarePoseChangeNotifyPCM,
  NX_MEMORY_SoftwareBroadPhasePCM,
  NX_MEMORY_SoftwarePairCachePCM,
  NX_MEMORY_SoftwareNarrowPhasePCM,
  NX_MEMORY_SoftwareIslandDetectionPCM,
  NX_MEMORY_SoftwareBodyPrepPCM,
  NX_MEMORY_SoftwareConstraintPrepPCM,
  NX_MEMORY_SoftwarePoseIntegratePCM,
  NX_MEMORY_SoftwareConstraintSolvePCM,
  NX_MEMORY_SoftwarePackPCM,
  
  /* FW DDI objects */
  NX_MEMORY_PPUDataStream,
  NX_MEMORY_NfArticulation,
  NX_MEMORY_NfArticulationMaximal,
  NX_MEMORY_NfBody,
  NX_MEMORY_NfConvexMapper,
  NX_MEMORY_NfJoint,
  NX_MEMORY_NfMaterial,
  NX_MEMORY_NfPPUDebug,
  NX_MEMORY_NfPage,
  NX_MEMORY_NfPageHash,
  NX_MEMORY_NfPageInstanceMapper,
  NX_MEMORY_NfPageMapper,
  NX_MEMORY_NfPhysicsSDK,
  NX_MEMORY_NfProfileOutput,
  NX_MEMORY_NfProjectionTree,
  NX_MEMORY_NfRBFwPipeline,
  NX_MEMORY_NfRBSwPipeline,
  NX_MEMORY_NfRBDebugPipeline,
  NX_MEMORY_NfScene,
  NX_MEMORY_NfRBSceneImage,
  NX_MEMORY_NfShape,
  NX_MEMORY_NfSyncManager,
  NX_MEMORY_NfProfile,
  NX_MEMORY_NfVRDBint,
  NX_MEMORY_NfRawTriangleMesh,
  NX_MEMORY_FluidMeshPacketHash,
  NX_MEMORY_NfFluidMeshCuisine,
  NX_MEMORY_NfParticleQueue,

  /* Fw DDI mallocs */
  NX_MEMORY_NfShapePageTable,
  NX_MEMORY_NfRBSwPipelinePCMTable,
  NX_MEMORY_NfRBSwPipelineSendBuf,
  NX_MEMORY_NfRBSwPipelineRecvBuf,
  NX_MEMORY_NfSyncManagerMaterialBM,
  NX_MEMORY_NfSyncManagerPrevAwakeBM,
  NX_MEMORY_NfSyncManagerCurrAwakeBM,
  NX_MEMORY_NfPageInstanceMapperInstancePool,
  NX_MEMORY_NfConvexMapperConvexPool,
  NX_MEMORY_NfRBDebugPipelinePCMTable,
  NX_MEMORY_NFRBDebugPipelineComparatorTable,
  NX_MEMORY_NvVRDContactBuffer,
  
  NX_MEMORY_NfRBDebugPipelineComparatorTable,
  NX_MEMORY_NfConvexHash,
  NX_MEMORY_FwConvex,

  /* Fluid-DDI objects*/
  NX_MEMORY_NvFluidEmitterEvent,
  NX_MEMORY_NvFluidEvent,
  NX_MEMORY_NfFluid,
  NX_MEMORY_NfFluidTriangleMeshPPUInterfaceSw,
  NX_MEMORY_NfFluidTriangleMeshPPUInterfaceHw,
  NX_MEMORY_NfFluidPPUInterfaceSw,
  NX_MEMORY_NfFluidPPUInterfaceHw,
  NX_MEMORY_NfFluidPPUInterfaceSwRef,
  NX_MEMORY_FluidCollisionSwRef,			
  NX_MEMORY_FluidDynamicsSwRef,				
  NX_MEMORY_FluidSwRef,						
  NX_MEMORY_NeighborSearchSwRef,			
  NX_MEMORY_ParticlePairStreamSwRef,		
  NX_MEMORY_SpatialHashSwRef,
  NX_MEMORY_NfFluidEmitter,
  NX_MEMORY_NfFluidEmitterPressure,
  NX_MEMORY_NfFluidEmitterRate,
  NX_MEMORY_ParticleArray,
  NX_MEMORY_SiteData,
  NX_MEMORY_NfFluidDataPrint,
  NX_MEMORY_NfShapeList,
  NX_MEMORY_NxShapeList,
  NX_MEMORY_DebugPacketHash,
  NX_MEMORY_FluidTriangle,
  NX_MEMORY_FluidMeshPktNode,
  NX_MEMORY_FluidMeshPktBucket,
  NX_MEMORY_FluidMeshCellNode,
  NX_MEMORY_FluidMeshCellBucket,
  NX_MEMORY_NfMeshPacketManager,
  NX_MEMORY_NfHeapManager,
  NX_MEMORY_Heap,
  NX_MEMORY_Buddy,
  NX_MEMORY_BuddyList,

  /* Fluid DDI mallocs */
  NX_MEMORY_NfFluidControllerFW,
  NX_MEMORY_NfFluidMeshPacketHashPPU,
  NX_MEMORY_NfFluidMeshTriangleVector,
  NX_MEMORY_NfCollisionTable,
  NX_MEMORY_NfFluidParticleBuffer,
  NX_MEMORY_NfFluidPacketTable,
  NX_MEMORY_NfFluidVpePacketTable,
  NX_MEMORY_NfFluidCellHashTable,
  NX_MEMORY_NfFluidCellSumHashTable,
  NX_MEMORY_NfFluidCellHashNode,
  NX_MEMORY_NfFluidPacketHashNode,
  NX_MEMORY_NfFluidUpdateBuffer,
  NX_MEMORY_NfFluidMeshTriangleVectorElts,
  NX_MEMORY_NfFluidMeshTriangleVectorElt,
  NX_MEMORY_NfFluidMeshPktBucketVectorElts,
  NX_MEMORY_NfFluidMeshPktNodeVectorElts,
  NX_MEMORY_NfFluidPacketHashPackedData,
  NX_MEMORY_NfFluidMemCategory,
  NX_MEMORY_NfFluidTransientBufferSet,
  NX_MEMORY_NfFluidShapes,
  
  NX_MEMORY_NfFluidTriangleList,
  NX_MEMORY_NfTriangleList,

  /* Cloth-DDI objects*/
  NX_MEMORY_NvClothEvent,
  NX_MEMORY_NfCloth,
  NX_MEMORY_NfClothPPUInterfaceSw,
  NX_MEMORY_NfClothPPUInterfaceHw,

  /* Cloth DDI mallocs */
  NX_MEMORY_NfClothDataBuffer,
  NX_MEMORY_NfClothCookBuffer,

// ** NX_ALLOC calls
  NX_MEMORY_CCD_TEMP,
  NX_MEMORY_CONVEX_TEMP,
  NX_MEMORY_TriangleMesh_TEMP,
  NX_MEMORY_MINIBALL_TEMP,
  NX_MEMORY_SMOOTH_NORMALS_TEMP,
  NX_MEMORY_NxTriangle32,
  NX_MEMORY_CCD_BUFFER,
  NX_MEMORY_CCD_edgeDegenMap,
	NX_MEMORY_InternalTriangleMesh_NxPoint,
	NX_MEMORY_InternalTriangleMesh_NxTriangle32,
	NX_MEMORY_InternalTriangleMesh_NxMaterialIndex,
	NX_MEMORY_InternalTriangleMesh_faceRemap,
  NX_MEMORY_InternalTriangelMesh_Plane,
	NX_MEMORY_InternalTriangleMesh_Normals,
  NX_MEMORY_InternalTriangleMesh_NxU8,
  NX_MEMORY_InternalTriangleMesh_NxU16,
  NX_MEMORY_InternalTriangleMesh_NxU32,
  NX_MEMORY_NxSpringDesc,
	NX_MEMORY_Scene_vertexTagStamps,
	NX_MEMORY_Scene_featureCache,
	NX_MEMORY_Scene_featureCacheExternal,
	NX_MEMORY_TriangleMesh_NxU16,
	NX_MEMORY_TriangleMesh_NxU8,
	NX_MEMORY_PMAP,
	NX_MEMORY_GenericCache_NxU8,
  NX_MEMORY_PairManager_NxU32,
	NX_MEMORY_PairManager_userPair,
	NX_MEMORY_Utilities_FaceNormals,
	NX_MEMORY_NxMutexRep,
	NX_MEMORY_Generic_Array_Container,
	NX_MEMORY_ConvexMesh,
	NX_MEMORY_RawBounds,
	NX_MEMORY_ClothSolverPacket,
	NX_MEMORY_PMapSample,
// ** Ice Calls!
	ICE_MEMORY_PERSISTENT,
	ICE_MEMORY_TEMP,
	ICE_MEMORY_SweepAndPrune,
	ICE_MEMORY_Valencies,
	ICE_MEMORY_ValenciesBuilder,
	ICE_MEMORY_IndexedTriangle16,
	ICE_MEMORY_HullPolygon,
	ICE_MEMORY_Edge,
	ICE_MEMORY_EdgeDesc,
	ICE_MEMORY_CollisionHull,
	ICE_MEMORY_SupportVertexMap,
	ICE_MEMORY_RaycastMap,
	ICE_MEMORY_MeshModel,
	ICE_MEMORY_EdgeTriangle,
	ICE_MEMORY_AdjTriangle,
	ICE_MEMORY_AdjacenciesBuilder,
	ICE_MEMORY_EdgeListBuilder,
	ICE_MEMORY_LinearLooseOctree,
	ICE_MEMORY_OctreeCell,
	ICE_MEMORY_FreePruner,
	ICE_MEMORY_DynamicPruner,
	ICE_MEMORY_StaticPruner,
	ICE_MEMORY_DynamicPruner2,
	ICE_MEMORY_AABBTree,
	ICE_MEMORY_AABBNoLeafTree,
	ICE_MEMORY_AABBTreeNode,
	ICE_MEMORY_AABBQuantizedNoLeafTree,
	ICE_MEMORY_AABBCollisionTree,
	ICE_MEMORY_PRUNING_SORTER,
	ICE_MEMORY_AABBCollisionNode,
	ICE_MEMORY_AABBNoLeafNode,
	ICE_MEMORY_AABBQuantizedNode,
	ICE_MEMORY_SAP_Element,
	ICE_MEMORY_SAP_Box,
	ICE_MEMORY_SAP_EndPoint,
	ICE_MEMORY_CustomCell,
	ICE_MEMORY_HandleManager,
	ICE_MEMORY_AABBQuantizedNoLeafNode,
	ICE_MEMORY_LeafTriangles,
	ICE_MEMORY_Pair,
// Ice discrete allocations
	ICE_MEMORY_ConvexDecomposer_FlatTags,
	ICE_MEMORY_ConvexDecomposer_ConvexTags,
	ICE_MEMORY_CookingUtils_XRef,
	ICE_MEMORY_CookingUtils_RVerts,
	ICE_MEMORY_CookingUtils_FaceNormals,
	ICE_MEMORY_CookingUtils_VertexNormals,
	ICE_MEMORY_ConvexHull_HullVertices,
	ICE_MEMORY_ConvexHull_HullNormals,
	ICE_MEMORY_ConvexHull_VertexData,
	ICE_MEMORY_ConvexHull_EdgeData,
	ICE_MEMORY_ConvexHull_EdgeNormals,
	ICE_MEMORY_ConvexHull_FacesByEdges,
	ICE_MEMORY_EdgeList_FacesByEdges,
	ICE_MEMORY_EdgeList_ActiveEdges,
	ICE_MEMORY_IceHullGaussMaps_Samples,
	ICE_MEMORY_Valency_Valencies,
	ICE_MEMORY_Valency_Offsets,
	ICE_MEMORY_Valency_AdjecentVerts,
	ICE_MEMORY_IcePruningPool_WorldBoxes,
	ICE_MEMORY_IcePruningPool_Objects,
	ICE_MEMORY_OPC_AABBTree_Indices,
	ICE_MEMORY_OPC_HybridModel_Indices,
	ICE_MEMORY_OPC_SweepAndPrune_Array,
	ICE_MEMORY_IceContainer_NewEntries,
	ICE_MEMORY_IceCustomArray_CellAddy,
	ICE_MEMORY_IceHandleManager_Objects,
	ICE_MEMORY_IceHandleManager_OutToIn,
	ICE_MEMORY_IceHandleManager_InToOut,
	ICE_MEMORY_IceHandleManager_Stamps,
	ICE_MEMORY_IceRevisedRadix_Histogram,
	ICE_MEMORY_IceRevistedRadix_Offset,
	ICE_MEMORY_IceRevistedRadix_Ranks,
	ICE_MEMORY_IceRevistedRadix_Ranks2,

  /* GPU DDI objects */
  NX_MEMORY_NgArticulation,
  NX_MEMORY_NgArticulationMaximal,
  NX_MEMORY_NgBody,
  NX_MEMORY_NgConvexMapper,
  NX_MEMORY_NgJoint,
  NX_MEMORY_NgMaterial,
  NX_MEMORY_NgPPUDebug,
  NX_MEMORY_NgPage,
  NX_MEMORY_NgPageHash,
  NX_MEMORY_NgPageInstanceMapper,
  NX_MEMORY_NgPageMapper,
  NX_MEMORY_NgPhysicsSDK,
  NX_MEMORY_NgCudaHeapData,
  NX_MEMORY_NgProfileOutput,
  NX_MEMORY_NgProjectionTree,
  NX_MEMORY_NgRBFwPipeline,
  NX_MEMORY_NgRBSwPipeline,
  NX_MEMORY_NgRBDebugPipeline,
  NX_MEMORY_NgScene,
  NX_MEMORY_NgRBSceneImage,
  NX_MEMORY_NgShape,
  NX_MEMORY_NgSyncManager,
  NX_MEMORY_NgProfile,
  NX_MEMORY_NgVRDBint,
  NX_MEMORY_NgRawTriangleMesh,
  NX_MEMORY_NgFluidMeshPacketHash,
  NX_MEMORY_NgFluidMeshCuisine,
  NX_MEMORY_NgParticleQueue,
  NX_MEMORY_NgShapePageTable,
  NX_MEMORY_NgRBSwPipelinePCMTable,
  NX_MEMORY_NgRBSwPipelineSendBuf,
  NX_MEMORY_NgRBSwPipelineRecvBuf,
  NX_MEMORY_NgSyncManagerMaterialBM,
  NX_MEMORY_NgSyncManagerPrevAwakeBM,
  NX_MEMORY_NgSyncManagerCurrAwakeBM,
  NX_MEMORY_NgPageInstanceMapperInstancePool,
  NX_MEMORY_NgConvexMapperConvexPool,
  NX_MEMORY_NgRBDebugPipelinePCMTable,
  NX_MEMORY_NgRBDebugPipelineComparatorTable,
  NX_MEMORY_NgConvexHash,
  NX_MEMORY_NgFluid,
  NX_MEMORY_NgFluidIdToIndexMap,
  NX_MEMORY_NgFluidEmitter,
  NX_MEMORY_NgFluidEmitterPressure,
  NX_MEMORY_NgFluidEmitterRate,
  NX_MEMORY_NgParticleArray,
  NX_MEMORY_NgSiteData,
  NX_MEMORY_NgFluidDataPrint,
  NX_MEMORY_NgShapeList,
  NX_MEMORY_NgDebugPacketHash,
  NX_MEMORY_NgFluidTriangle,
  NX_MEMORY_NgFluidMeshPktNode,
  NX_MEMORY_NgFluidMeshPktBucket,
  NX_MEMORY_NgFluidMeshCellNode,
  NX_MEMORY_NgFluidMeshCellBucket,
  NX_MEMORY_NgMeshPacketManager,
  NX_MEMORY_PxcHeapManager,
  NX_MEMORY_NgHeap,
  NX_MEMORY_NgBuddy,
  NX_MEMORY_NgBuddyList,
  NX_MEMORY_NgImplicitScreenMesh,
  NX_MEMORY_NgImplicitScreenMeshPPUInterfaceHw,
  NX_MEMORY_NgFluidControllerFW,
  NX_MEMORY_NgFluidMeshPacketHashPPU,
  NX_MEMORY_NgFluidMeshTriangleVector,
  NX_MEMORY_NgCollisionTable,
  NX_MEMORY_NgFluidParticleBuffer,
  NX_MEMORY_NgFluidPacketTable,
  NX_MEMORY_NgFluidVpePacketTable,
  NX_MEMORY_NgFluidCellHashTable,
  NX_MEMORY_NgFluidCellSumHashTable,
  NX_MEMORY_NgFluidCellHashNode,
  NX_MEMORY_NgFluidPacketHashNode,
  NX_MEMORY_NgFluidUpdateBuffer,
  NX_MEMORY_NgFluidMeshTriangleVectorElts,
  NX_MEMORY_NNgFluidMeshTriangleVectorElt,
  NX_MEMORY_NgFluidMeshPktBucketVectorElts,
  NX_MEMORY_NgFluidMeshPktNodeVectorElts,
  NX_MEMORY_NgFluidPacketHashPackedData,
  NX_MEMORY_NgFluidMemCategory,
  NX_MEMORY_NgFluidTransientBufferSet,
  NX_MEMORY_NgFluidShapes,
  NX_MEMORY_NgFluidTriangleList,
  NX_MEMORY_NgTriangleList,
  NX_MEMORY_NgCloth,
  NX_MEMORY_NgClothGPUInterfaceSw,
  NX_MEMORY_NgClothGPUInterfaceHw,
  NX_MEMORY_NgClothDataBuffer,
  NX_MEMORY_NgClothCookBuffer,

  NX_MEMORY_LAST
};

#if defined(_DEBUG)
	#define NX_NEW_TMP(x)		new((const char *)__FILE__, __LINE__, #x, NX_MEMORY_##x) x
	#define NX_NEW(x)			new((const char *)__FILE__, __LINE__, #x, NX_MEMORY_##x) x
	#define NX_NEW_MEM(x,y)		new((const char *)__FILE__, __LINE__, #x, NX_MEMORY_##y) x
#else
	#define NX_NEW_TMP(x)		new(NX_MEMORY_##x) x
	#define NX_NEW(x)			new(NX_MEMORY_##x) x
	#define NX_NEW_MEM(x,y)		new(NX_MEMORY_##y) x
#endif

/**
 \brief Abstract base class for an application defined memory allocator that can be used by the Nx library.

 \note The SDK state should not be modified from within any allocation/free function.

 <b>Threading:</b> All methods of this class should be thread safe as it can be called from the user thread or the physics processing thread(s).

 <h2>Example</h2>

 \include NxUserAllocator_Example.cpp
*/
class NxUserAllocator
	{
	public:
	/**
	\brief Allocates size bytes of memory.

	Same as simple #malloc below, but with extra debug info fields.

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
	and physics processing thread(s).

	\param size Number of bytes to allocate.
	\param fileName File which is allocating the memory.
	\param line Line which is allocating the memory.
	\return The allocated block of memory.
	*/
	virtual void* mallocDEBUG(size_t size, const char* fileName, int line) = 0;

	/**
	\brief Allocates size bytes of memory.

	Same as simple #malloc below, but with extra debug info fields.

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
	and physics processing thread(s).

	\param size Number of bytes to allocate.
	\param fileName File which is allocating the memory.
	\param line Line which is allocating the memory.
	\param className Name of the class which is allocating the memory.
	\param type A hint about what the memory will be used for. See #NxMemoryType.
	\return The allocated block of memory.
	*/
	virtual void* mallocDEBUG(size_t size, const char* fileName, int line, const char* className, NxMemoryType type)
		{
		NX_UNREFERENCED_PARAMETER(type);
		NX_UNREFERENCED_PARAMETER(className);
		return mallocDEBUG(size, fileName, line);	// Just so we don't break user code
		}

	/**
	\brief Allocates size bytes of memory.

	Compatible with the standard C malloc(), with the exception that
	it should never return NULL.  If you run out of memory, then
	you should terminate the app or take some other appropriate action.

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
	and physics processing thread(s).

	\param size Number of bytes to allocate.
	\return The allocated block of memory.
	*/
	virtual void* malloc(size_t size) = 0;

	/**
	\brief Allocates size bytes of memory.

	Compatible with the standard C malloc(), with the exception that
	it should never return NULL.  If you run out of memory, then
	you should terminate the app or take some other appropriate action.

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
	and physics processing thread(s).

	\param size Number of bytes to allocate.
	\param type A hint about what the memory will be used for. See #NxMemoryType.
	\return The allocated block of memory.
	*/
	virtual void* malloc(size_t size, NxMemoryType type)
		{
		NX_UNREFERENCED_PARAMETER(type);
		return malloc(size);		// Just so we don't break user code
		}

	/**
	\brief Resizes the memory block previously allocated with malloc() or
	realloc() to be size() bytes, and returns the possibly moved memory.

	Compatible with the standard C realloc(), with the exception that
	it should never return NULL.  If you run out of memory, then
	you should terminate the app or take some other appropriate action.

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
	and physics processing thread(s).

	\param memory Memory block to change the size of.
	\param size New size for memory block.
	\return New pointer to the block of memory.
	*/
	virtual void* realloc(void* memory, size_t size) = 0;

	/**
	\brief Frees the memory previously allocated by malloc() or realloc().

	Compatible with the standard C free().

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
	and physics processing thread(s).

	\param memory Memory to free.
	*/
	virtual void free(void* memory) = 0;

	/**
	\brief Verify heap.

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread 
	and physics processing thread(s).
	*/
	virtual void checkDEBUG(void) {};

	virtual ~NxUserAllocator(){};
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
