#ifndef NX_PHYSICS_NXSCENEDESC
#define NX_PHYSICS_NXSCENEDESC
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

/**
\brief Used to specify the timestepping behavior.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxSceneDesc
*/
enum NxTimeStepMethod
	{
	NX_TIMESTEP_FIXED,				//!< The simulation automatically subdivides the passed elapsed time into maxTimeStep-sized substeps.
	NX_TIMESTEP_VARIABLE,			//!< The simulation uses the elapsed time that the user passes as-is, substeps (maxTimeStep, maxIter) are not used.
	NX_TIMESTEP_INHERIT,			//!< Inherit timing settings from primary scene.  Only valid for compartments.
	NX_NUM_TIMESTEP_METHODS,

	NX_TSM_FORCE_DWORD = 0x7fffffff	//!< Just to make sure sizeof(enum) == 4, not a valid value.
	};

/**
\brief Used to choose between a hardware and software master scene.

This enum is used with the NxSceneDesc::simType member.

\note The master scene is a rigid body scene (which can also contain cloth); compartments are used for fluid.
Compartments can run in hardware even if the master scene is a software scene.

@see NxSceneDesc
*/
enum NxSimulationType
	{
	NX_SIMULATION_SW	= 0,		//!< Create a software master scene.
	NX_SIMULATION_HW	= 1,		//!< Create a hardware master scene.

	NX_STY_FORCE_DWORD = 0x7fffffff			//!< Just to make sure sizeof(enum) == 4, not a valid value.
	};

/**
\brief Pruning structure used to accelerate scene queries (raycast, sweep tests, etc)

	NX_PRUNING_NONE can be used without defining extra parameters. It typically doesn't provide
	fast scene queries, but on the other hand it doesn't consume much memory. It is useful when
	you don't use the SDK's scene queries at all.

	NX_PRUNING_OCTREE usually provides fast queries and cheap per-object updates. You need
	to define "maxBounds" and "subdivisionLevel" to use this structure.

	NX_PRUNING_QUADTREE is the 2D version of NX_PRUNING_OCTREE. It is usually a better choice
	when your world is mostly flat. It is sometimes a better choice for non-flat worlds as well.
	You need to define "maxBounds", "subdivisionLevel" and "upAxis" to use this structure.

	NX_PRUNING_DYNAMIC_AABB_TREE usually provides the fastest queries. However there is a
	constant per-frame management cost associated with this structure. You have the option to
	give a hint on how much work should be done per frame by setting the parameter
	#NxSceneDesc::dynamicTreeRebuildRateHint.

	NX_PRUNING_STATIC_AABB_TREE is typically used for static objects. It is the same as the
	dynamic AABB tree, without the per-frame overhead. This is the default choice for static
	objects. However, if you are streaming parts of the world in and out, you may want to use
	the dynamic version even for static objects.
*/
enum NxPruningStructure
	{
	NX_PRUNING_NONE,				//!< No structure, using a linear list of objects
	NX_PRUNING_OCTREE,				//!< Using a preallocated loose octree
	NX_PRUNING_QUADTREE,			//!< Using a preallocated loose quadtree
	NX_PRUNING_DYNAMIC_AABB_TREE,	//!< Using a dynamic AABB tree
	NX_PRUNING_STATIC_AABB_TREE,	//!< Using a static AABB tree
	};

/**
\brief Selects a broadphase type.
*/
enum NxBroadPhaseType
{
	/**
	\brief A sweep-and-prune (SAP) algorithm to find pairs of potentially colliding shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
    NX_BP_TYPE_SAP_SINGLE,

	/**
	\brief A multi sweep-and-prune algorithm to find pairs of potentially colliding shapes.
	
	Uses a configurable 2D grid to divide the scene space into cells. The potentially overlapping 
	shape pairs are detected in each cell and the information is merged together. This approach
	is usually faster than NX_BP_TYPE_SAP_SINGLE in scenarios with many shapes and a high creation/deletion
	rate of shapes. However, the amount of memory required is considerably higher depending on the
	number of grid cells used.

	\note The following extra parameters need to be defined:
	\li NxSceneDesc.maxBounds
	\li NxSceneDesc.upAxis
	\li NxSceneDesc.nbGridCellsX
	\li NxSceneDesc.nbGridCellsY

	\n

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : No
	\li XB360: Yes

	@see NxSceneDesc.bpType
	*/
    NX_BP_TYPE_SAP_MULTI,
};

enum NxSceneFlags
	{
	/**
	\brief Used to disable use of SSE in the solver.

	SSE is detected at runtime(on appropriate platforms) and used if present by default.

	However use of SSE can be disabled, even if present, using this flag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : N/A
	\li PS3  : N/A
	\li XB360: N/A
	*/
	NX_SF_DISABLE_SSE	=0x1,

	/**
	\brief Disable all collisions in a scene. Use the flags NX_AF_DISABLE_COLLISION and NX_SF_DISABLE_COLLISION for disabling collisions between specific actors and shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NX_AF_DISABLE_COLLISION, NX_SF_DISABLE_COLLISION
	*/
	NX_SF_DISABLE_COLLISIONS	=0x2,

	/**
	\brief Perform the simulation in a separate thread.

	By default the SDK runs the physics on a separate thread to the user thread(i.e. the thread which
	calls the API).

	However if this flag is disabled, then the simulation is run in the thread which calls #NxScene::simulate()

	<b>Default:</b> True

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.simulate()
	*/
	NX_SF_SIMULATE_SEPARATE_THREAD	=0x4,

	/**
	\brief Enable internal multi threading.

	This flag enables the multi threading code within the SDK which allows the simulation to
	be divided into tasks for execution on an arbitrary number of threads. 
	
	This is an orthogonal feature to running the simulation in a separate thread, see #NX_SF_SIMULATE_SEPARATE_THREAD.

	\note There may be a small performance penalty for enabling the multi threading code, hence this flag should
	only be enabled if the application intends to use the feature.

	<b>Default:</b> False

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see NxSceneDesc NX_SF_SIMULATE_SEPARATE_THREAD
	*/
	NX_SF_ENABLE_MULTITHREAD		=0x8,

	/**
	\brief Enable Active Transform Notification.

	This flag enables the the Active Transform Notification feature for a scene.  This
	feature defaults to disabled.  When disabled, the function
	NxScene::getActiveTransforms() will always return a NULL list.

	\note There may be a performance penalty for enabling the Active Transform Notification, hence this flag should
	only be enabled if the application intends to use the feature.

	<b>Default:</b> False

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_SF_ENABLE_ACTIVETRANSFORMS	=0x10,

	/**
	\brief Enable Restricted Scene.

	\note Only applies to hardware scenes.

	This flag creates a restricted scene, running the broadphase collision detection on hardware, 
	while limiting the number of actors (see "AGEIA PhysX Hardware Scenes" in the Guide).

	<b>Default:</b> False

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No
	*/
	NX_SF_RESTRICTED_SCENE				=0x20,

	/**
	\brief Disable the mutex which serializes scene execution

	Under normal circumstances scene execution is serialized by a mutex. This flag
	can be used to disable this serialization.

	\warning This flag is _experimental_ and in future versions is likely be removed. In favour of 
	completely removing	the mutex.

	If this flag is used the recommended scenario is for use with a software scene and hardware fluid/cloth scene.

	<b>Default:</b> True (change from earlier beta versions)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_SF_DISABLE_SCENE_MUTEX			=0x40,


	/**
	\brief Force the friction model to cone friction

	This ensures that all contacts in the scene will use cone friction, rather than the default
	simplified scheme. This will however have a negative impact on performance in software scenes. Use this
	flag if sliding objects show an affinity for moving along the world axes.

	\note Only applies to software scenes; hardware scenes always force cone friction.
	
	Cone friction may also be activated on an actor-by-actor basis using the NX_AF_FORCE_CONE_FRICTION flag, see #NxActorFlag.

	<b>Default:</b> False

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (always active)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_SF_FORCE_CONE_FRICTION			=0x80,


	/**
	\brief When set to 1, the compartments are all executed before the primary scene is executed.  This may lower performance
	but it improves interaction quality between compartments and the primary scene.

	<b>Default:</b> False

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: No
	*/
	NX_SF_SEQUENTIAL_PRIMARY			=0x80*2,

	/**
	\brief Enables faster but less accurate fluid collision with static geometry.

	If the flag is set static geometry is considered one simulation step late, which 
	can cause particles to leak through static geometry. In order to prevent this, 
	NxFluidDesc.collisionDistanceMultiplier can be increased.
	
	<b>Default:</b> False

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_SF_FLUID_PERFORMANCE_HINT		=0x80*4,
	
	};

class NxUserNotify;
class NxFluidUserNotify;
class NxClothUserNotify;
class NxSoftBodyUserNotify;
class NxUserContactModify;
class NxUserTriggerReport;
class NxUserContactReport;
class NxUserActorPairFiltering;
class NxBounds3;
class NxUserScheduler;

/**
\brief Class used to retrieve limits(e.g. max number of bodies) for a scene. The limits
are used as a hint to the size of the scene, not as a hard limit (i.e. it will be possible
to create more objects than specified in the scene limits).

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
*/
class NxSceneLimits
	{
	public:
	NxU32					maxNbActors;		//!< Expected max number of actors
	NxU32					maxNbBodies;		//!< Expected max number of bodies
	NxU32					maxNbStaticShapes;	//!< Expected max number of static shapes
	NxU32					maxNbDynamicShapes;	//!< Expected max number of dynamic shapes
	NxU32					maxNbJoints;		//!< Expected max number of joints

	NX_INLINE NxSceneLimits();
	};

NX_INLINE NxSceneLimits::NxSceneLimits()	//constructor sets to default
	{
	maxNbActors			= 0;
	maxNbBodies			= 0;
	maxNbStaticShapes	= 0;
	maxNbDynamicShapes	= 0;
	maxNbJoints			= 0;
	}

/**
\brief Descriptor class for scenes. See #NxScene.

@see NxScene NxPhysicsSDK.createScene maxBounds
*/

class NxSceneDesc
	{
	public:

	/**
	\brief Gravity vector

	<b>Range:</b> force vector<br>
	<b>Default:</b> Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.setGravity()
	*/
	NxVec3					gravity;

	/**
	\brief Possible notification callback

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserNotify NxScene.setUserNotify() NxScene.getUserNotify()
	*/
	NxUserNotify*			userNotify;

	/**
	\brief Possible notification callback for fluids

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxFluidUserNotify NxScene.setFluidUserNotify() NxScene.getFluidUserNotify()
	*/
	NxFluidUserNotify*		fluidUserNotify;

	/**
	\brief Possible notification callback for cloths

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxClothUserNotify NxScene.setClothUserNotify() NxScene.getClothUserNotify()
	*/
	NxClothUserNotify*		clothUserNotify;

	/**
	\brief Possible notification callback for softBodys

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxSoftBodyUserNotify NxScene.setSoftBodyUserNotify() NxScene.getSoftBodyUserNotify()
	*/
	NxSoftBodyUserNotify*	softBodyUserNotify;

	/**
	\brief Possible asynchronous callback for contact modification

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserContactModify NxScene.setUserContactModify() NxScene.getUserContactModify()
	*/
	NxUserContactModify*		userContactModify;

	/**
	\brief Possible trigger callback

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserTriggerReport NxScene.setUserTriggerReport() NxScene.getUserTriggerReport()
	*/
	NxUserTriggerReport*	userTriggerReport;

	/**
	\brief Possible contact callback

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserContactReport NxScene.setUserContactReport() NxScene.getUserContactReport()
	*/
	NxUserContactReport*	userContactReport;

	/**
	\brief Internal. Do not use!
	*/	
	NxUserActorPairFiltering*	userActorPairFiltering;

	/**
	\brief Maximum substep size.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0/60.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.setTiming() maxIter
	*/
	NxReal					maxTimestep;

	/**
	\brief Maximum number of substeps to take

	<b>Default:</b> 8

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.setTiming() maxTimestep
	*/
	NxU32					maxIter;

	/**
	\brief Integration method.

	<b>Default:</b> NX_TIMESTEP_FIXED

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTimeStepMethod NxScene.setTiming() maxTimestep maxIter
	*/
	NxTimeStepMethod		timeStepMethod;

	/**
	\brief Max scene bounds.

	If scene bounds are provided (maxBounds in the descriptor), the SDK takes advantage of this information
	to accelerate scene-level collision queries (e.g. raycasting). When using maxBounds, you have to make
	sure created objects stay within the scene bounds. In particular, the position of dynamic shapes should
	stay within the provided bounds. Otherwise the shapes outside the bounds will not be taken into account
	by all scene queries (raycasting, sweep tests, overlap tests, etc). They will nonetheless still work
	correctly for the main physics simulation.

	<b>Range:</b> See #NxBounds3<br>
	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxBounds3*				maxBounds;

	/**
	\brief Expected scene limits (or NULL)

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSceneLimits
	*/
	NxSceneLimits*			limits;

	/**
	\brief Used to specify if the scene is a master hardware or software scene.

	<b>Default:</b> NX_SIMULATION_SW

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSimulationType
	*/
	NxSimulationType		simType;

	/**
	\brief Enable/disable default ground plane

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	<b>Default:</b> false
	*/
	NX_BOOL					groundPlane;

	/**
	\brief Enable/disable 6 planes around maxBounds (if available)

	<b>Default:</b> false

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see maxBounds
	*/
	NX_BOOL					boundsPlanes;

	/**
	\brief Flags used to select scene options.

	<b>Default:</b> NX_SF_SIMULATE_SEPARATE_THREAD | NX_SF_DISABLE_SCENE_MUTEX

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Partial
	\li PS3  : Yes (however, default value is NX_SF_SIMULATE_SEPARATE_THREAD)
	\li XB360: Yes

	@see NxSceneFlags
	*/
	NxU32					flags;

	/**
	\brief Defines a custom scheduler.

	The application can define a custom scheduler to completely take over the allocation of work items among threads.

	An alternative is for the application to use the built in scheduler. See #internalThreadCount

	<b>Default:</b> NULL (disabled)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see NxScheduler NX_SF_ENABLE_MULTITHREAD internalThreadCount
	*/
	NxUserScheduler*			customScheduler;

	/**
	\brief Allows the user to specify the stack size for the main simulation thread.

	The value is specified in bytes and rounded to an appropriate size by the operating system.

	NOTE: If you increase the stack size for all threads that call the SDK, you may want to call
	#NxFoundationSDK::setAllocaThreshold to prevent unnecessary heap allocations.

	Specifying a value of zero will cause the SDK to choose a platform specific default value:

	\li PC SW - OS default
	\li PPU - OS default
	\li PS3 - 256k
	\li XBox 360 - OS default

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32					simThreadStackSize;
	
	/**
	\brief Sets the thread priority of the main simulation thread.

	The default is normal priority.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: No
	*/
	NxThreadPriority		simThreadPriority;

	/**
	\brief Allows the user to specify which (logical) processor to allocate the simulation thread to.

	The sim thread will be allocated to processors corresponding to bits which are set. Starting from
	the least significant bit.

	This flag is ignored for platforms which do not associate threads exclusively to specific processors.

	\note The XBox 360 associates threads with specific processors.
	\note The special value of 0 lets the SDK determine the thread affinity.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes
	*/
	NxU32					simThreadMask;

	/**
	\brief Sets the number of SDK managed worker threads used when running the simulation in parallel.

	When internal multi threading is enabled(see #NX_SF_ENABLE_MULTITHREAD) the SDK creates a number of
	threads to run the simulation. This member controls the number of threads.

	This member is ignored if the application takes over control of the work allocation with a custom scheduler.

	\note This is poorly named; should be renamed workerThreadCount for consistency.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see customScheduler
	*/
	NxU32					internalThreadCount;

	/**
	\brief Allows the user to specify the stack size for the worker threads created by the SDK.

	The value is specified in bytes and rounded to an appropriate size by the operating system.

	NOTE: If you increase the stack size for all threads that call the SDK, you may want to call
	#NxFoundationSDK::setAllocaThreshold to prevent unnecessary heap allocations.

	Specifying a value of zero will cause the SDK to choose a platform specific default value.

	
	\li PC SW - OS default
	\li PPU - OS default
	\li XBox 360 - OS default

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes
	*/
	NxU32					workerThreadStackSize;

	/**
	\brief Sets the thread priority of the SDK created worker threads

	The default is normal priority.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No
	*/
	NxThreadPriority		workerThreadPriority;

	/**
	\brief Allows the user to specify which (logical) processor to allocate SDK internal worker threads to.

	The SDK Will allocate internal threads to processors corresponding to bits which are set. Starting from
	the least significant bit.

	This flag is ignored for platforms which do not associate threads exclusively to specific processors.

	\note The XBox 360 associates threads with specific processors.
	\note This is poorly named; should be renamed workerThreadMask for consistency.
	\note The special value of 0 lets the SDK determine the thread affinity.
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes
	*/
	NxU32					threadMask;

	/**
	\brief Sets the number of SDK managed threads which will be processing background tasks.

	For example scene queries can run on these threads or the SDK may need to preprocess data to be sent to the 
	PhysX card.

	This member must be set to 0 if the application takes over control of the work allocation with a custom scheduler.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see customScheduler
	*/
	NxU32					backgroundThreadCount;

	/**
	\brief Sets the thread priority of the SDK created background threads

	The default is normal priority.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No
	*/
	NxThreadPriority		backgroundThreadPriority;

	/**
	\brief Allows the user to specify which (logical) processor to allocate SDK background threads.

	The SDK Will allocate internal threads to processors corresponding to bits which are set. Starting from
	the least significant bit.

	This flag is ignored for platforms which do not associate threads exclusively to specific processors.

	\note The XBox 360 associates threads with specific processors.
	\note The special value of 0 lets the SDK determine the thread affinity.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes
	*/
	NxU32					backgroundThreadMask;



	/**
	\brief Defines the up axis for your world. This is used to accelerate scene queries like
	raycasting or sweep tests. Internally, a 2D structure is used instead of a 3D one whenever
	an up axis is defined. This saves memory and is usually faster.

	Use 1 for Y = up, 2 for Z = up, or 0 to disable this feature.
	It is not possible to use X = up.

	\note WARNING: this is only used when maxBounds are defined.
	*/
	NxU32					upAxis;

	/**
	\brief Defines the subdivision level for acceleration structures used for scene queries.

	\note WARNING: this is only used when maxBounds are defined.
	*/
	NxU32					subdivisionLevel;

	/**
	\brief Defines the structure used to store static objects.

	\note Only NX_PRUNING_STATIC_AABB_TREE and NX_PRUNING_DYNAMIC_AABB_TREE are allowed here.
	*/
	NxPruningStructure		staticStructure;

	/**
	\brief Defines the structure used to store dynamic objects.
	*/
	NxPruningStructure		dynamicStructure;

	/**
	\brief Hint for how much work should be done per simulation frame to rebuild the pruning structure.

	This parameter gives a hint on the distribution of the workload for rebuilding the dynamic AABB tree
	pruning structure #NX_PRUNING_DYNAMIC_AABB_TREE. It specifies the desired number of simulation frames
	the rebuild process should take. Higher values will decrease the workload per frame but the pruning
	structure will get more and more outdated the longer the rebuild takes (which can make
	scene queries less efficient).

	\note Only used for #NX_PRUNING_DYNAMIC_AABB_TREE pruning structure.

	\note This parameter gives only a hint. The rebuild process might still take more or less time depending on the
	      number of objects involved.

	<b>Range:</b> [5, inf]<br>
	<b>Default:</b> 100

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32					dynamicTreeRebuildRateHint;

	/**
	\brief Will be copied to NxScene::userData

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	void*					userData;

	/**
	\brief Defines which type of broadphase to use.

	<b>Default:</b> NX_BP_TYPE_SAP_SINGLE

	@see NxBroadPhaseType
	*/
	NxBroadPhaseType		bpType;

	/**
	\brief Defines the number of broadphase cells along the grid x-axis.

	\note Must be power of two. Max is 8 at the moment. The broadphase type must be set to NX_BP_TYPE_SAP_MULTI 
	for this parameter to have an effect.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : No
	\li XB360: Yes

	@see NxSceneDesc.bpType
	*/
	NxU32					nbGridCellsX;

	/**
	\brief Defines the number of broadphase cells along the grid y-axis.

	\note Must be power of two. Max is 8 at the moment. The broadphase type must be set to NX_BP_TYPE_SAP_MULTI 
	for this parameter to have an effect.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : No
	\li XB360: Yes

	@see NxSceneDesc.bpType
	*/
	NxU32					nbGridCellsY;

	/**
	\brief Defines the number of actors required to spawn a separate rigid body solver thread.

	\note Internal multi threading must be enabled (see #NX_SF_ENABLE_MULTITHREAD) for this member to have
	any effect.

	<b>Default:</b> 32

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Not applicable
	\li PS3  : Not applicable
	\li XB360: Yes

	@see NxScene.setSolverBatchSize() NxScene.getSolverBatchSize()
	*/
	NxU32					solverBatchSize;

	/**
	\brief constructor sets to default (no gravity, no ground plane, collision detection on).
	*/
	NX_INLINE NxSceneDesc();

	/**
	\brief (re)sets the structure to the default (no gravity, no ground plane, collision detection on).	
	*/
	NX_INLINE void setToDefault();

	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid (returns always true).
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxSceneDesc::NxSceneDesc()	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxSceneDesc::setToDefault()
	{
	gravity.zero();
	userNotify				= NULL;
	fluidUserNotify			= NULL;
	clothUserNotify			= NULL;
	softBodyUserNotify		= NULL;
	userTriggerReport		= NULL;
	userContactReport		= NULL;
	userContactModify		= NULL;
	userActorPairFiltering	= NULL;

	maxTimestep				= 1.0f/60.0f;
	maxIter					= 8;
	timeStepMethod			= NX_TIMESTEP_FIXED;

	maxBounds				= NULL;
	limits					= NULL;
	simType					= NX_SIMULATION_SW;
	groundPlane				= false;
	boundsPlanes			= false;
	userData				= NULL;
#ifdef __CELLOS_LV2__
	flags				= NX_SF_SIMULATE_SEPARATE_THREAD | NX_SF_SEQUENTIAL_PRIMARY;
#else
	flags					= NX_SF_SIMULATE_SEPARATE_THREAD | NX_SF_DISABLE_SCENE_MUTEX;
#endif
	upAxis					= 0;
	subdivisionLevel		= 5;
	staticStructure			= NX_PRUNING_STATIC_AABB_TREE;
	dynamicStructure		= NX_PRUNING_NONE;
	dynamicTreeRebuildRateHint = 100;

	internalThreadCount		= 0;
	backgroundThreadCount	= 0;
	customScheduler			= NULL;

	simThreadStackSize		= 0;
	simThreadPriority		= NX_TP_NORMAL;

	workerThreadStackSize	= 0;
	workerThreadPriority	= NX_TP_NORMAL;
	backgroundThreadPriority= NX_TP_NORMAL;

	simThreadMask			= 0;
	threadMask				= 0;
	backgroundThreadMask	= 0;

	bpType					= NX_BP_TYPE_SAP_SINGLE;
	nbGridCellsX			= 0;
	nbGridCellsY			= 0;

	solverBatchSize			= 32;
	}

NX_INLINE bool NxIsPowerOfTwo(NxU32 n)	{ return ((n&(n-1))==0);	}

NX_INLINE bool NxSceneDesc::isValid() const
	{
	if(bpType==NX_BP_TYPE_SAP_MULTI)
		{
		if(!nbGridCellsX || !NxIsPowerOfTwo(nbGridCellsX) || nbGridCellsX>8)
			return false;
		if(!nbGridCellsY || !NxIsPowerOfTwo(nbGridCellsY) || nbGridCellsY>8)
			return false;
		if(!maxBounds)
			return false;
		}

	if(maxTimestep <= 0 || maxIter < 1 || timeStepMethod > NX_NUM_TIMESTEP_METHODS)
		return false;
	if(boundsPlanes && !maxBounds)
		return false;

	if(staticStructure!=NX_PRUNING_STATIC_AABB_TREE && staticStructure!=NX_PRUNING_DYNAMIC_AABB_TREE)
		return false;

	if(dynamicStructure==NX_PRUNING_OCTREE || dynamicStructure==NX_PRUNING_QUADTREE)
		{
		if(!maxBounds)
			return false;
		if(dynamicStructure==NX_PRUNING_QUADTREE)
			{
			if(upAxis!=1 && upAxis!=2)
				return false;
			}
		}

	if (dynamicTreeRebuildRateHint < 5)
		return false;

	if((customScheduler!=NULL)&&(internalThreadCount>0))
		return false;

	if((customScheduler!=NULL)&&(backgroundThreadCount>0))
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
