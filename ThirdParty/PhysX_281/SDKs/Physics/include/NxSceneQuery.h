#ifndef NX_PHYSICS_NX_SCENEQUERY
#define NX_PHYSICS_NX_SCENEQUERY
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
#include "NxUserRaycastReport.h"

struct NxSweepQueryHit;
class NxRay;
class NxSphere;
class NxBounds3;
class NxBox;
class NxCapsule;
class NxPlane;
enum NxShapesType;

/**
\brief Specifies the nature of the query object.

In synchronous mode, a call to NxSceneQuery::execute() will block and batched queries are executed
immediately.

In asynchronous mode, a call to NxSceneQuery::execute() will not block and batched queries are executed
at any point between calling NxSceneQuery::execute() and NxSceneQuery::finish() returning true.

Note: This is a new feature in 2.7.0, and there are still some issues that will be fixed in a future version:
* cullShapes() does not work in asynchronous mode
* the return value from overlapXShapes() might not be correct in synchronous mode
* linearOBBSweep() and linearCapsuleSweep() always returns 0 when NX_SF_ALL_HITS is used.

@see NxSceneQueryDesc NxSceneQuery
*/
enum NxSceneQueryExecuteMode
	{
	NX_SQE_SYNCHRONOUS,			//!< Execute queries in synchronous mode.
	NX_SQE_ASYNCHRONOUS,		//!< Execute queries in asynchronous mode. WARNING: NxSceneDesc::backgroundThreadCount must be at least 1 for this to work.

	NX_SQE_FORCE_DWORD	=	0x7fffffff
	};

/**
\brief Specifies the behaviour after a query result.

@see NxSceneQueryReport NxSceneQuery
*/
enum NxQueryReportResult 
	{
	NX_SQR_CONTINUE,			//!< Continue reporting more results
	NX_SQR_ABORT_QUERY,			//!< Stop reporting results for current query
	NX_SQR_ABORT_ALL_QUERIES,	//!< Stop reporting results for all queries

	NX_SQR_FORCE_DWORD	=	0x7fffffff
	};

/**
\brief User-defined object needed to get back query results from NxSceneQuery objects.

@see NxSceneQuery NxQueryReportResult
*/
class NxSceneQueryReport
	{
	public:
	
	/**
	\brief Callback function used to return boolean query results.

	This function reports results from the following functions:
	
	\li #NxSceneQuery::raycastAnyShape()
	\li #NxSceneQuery::checkOverlapSphere()
	\li #NxSceneQuery::checkOverlapAABB()
	\li #NxSceneQuery::checkOverlapOBB()
	\li #NxSceneQuery::checkOverlapCapsule()

	\param[in] userData User data pointer passed to the query function.
	\param[in] result True if there is an intersection/overlap
	\return Specifies the action the SDK should take, eg continue or abort the query.

	@see NxQueryReportResult
	*/
	virtual	NxQueryReportResult	onBooleanQuery(void* userData, bool result)								= 0;
	
	/**
	\brief Callback function used to return raycast query results.

	This function reports results from the following functions:
	
	\li #NxSceneQuery::raycastClosestShape()
	\li #NxSceneQuery::raycastAllShapes()

	\param[in] userData User data pointer passed to the query function.
	\param[in] nbHits Number of hit shapes
	\param[in] hits Array of hit descriptors (size nbHits)
	\return Specifies the action the SDK should take, eg continue or abort the query.

	@see NxQueryReportResult
	*/
	virtual	NxQueryReportResult	onRaycastQuery(void* userData, NxU32 nbHits, const NxRaycastHit* hits)	= 0;

	/**
	\brief Callback function used to return shape query results.

	This function reports results from the following functions:
	
	\li #NxSceneQuery::overlapSphereShapes()
	\li #NxSceneQuery::overlapAABBShapes()
	\li #NxSceneQuery::overlapOBBShapes()
	\li #NxSceneQuery::overlapCapsuleShapes()
	\li #NxSceneQuery::cullShapes()

	\param[in] userData User data pointer passed to the query function.
	\param[in] nbHits Number of hits returned.
	\param[in] hits Array of shape pointers.
	\return Specifies the action the SDK should take, e.g. continue or abort the query.

	@see NxQueryReportResult
	*/
	virtual	NxQueryReportResult	onShapeQuery(void* userData, NxU32 nbHits, NxShape** hits)				= 0;

	/**
	\brief Callback function used to return sweep query results.

	This function reports results from the following functions:
	
	\li #NxSceneQuery::linearOBBSweep()
	\li #NxSceneQuery::linearCapsuleSweep()

	\param[in] userData User data pointer passed to the query function.
	\param[in] nbHits Number of sweep hits.
	\param[in] hits Array of sweep hits (size nbHits)
	\return Specifies the action the SDK should take, eg continue or abort the query.

	@see NxQueryReportResult
	*/
	virtual	NxQueryReportResult	onSweepQuery(void* userData, NxU32 nbHits, NxSweepQueryHit* hits)		= 0;

	protected:
	virtual ~NxSceneQueryReport(){};
	};

/**
\brief Descriptor class for #NxSceneQuery.

@see NxSceneQuery NxSceneQueryReport NxSceneQueryExecuteMode
*/
class NxSceneQueryDesc
	{
	public:
	/**
	\brief The callback class used to return results from the batched queries.

	@see NxSceneQueryReport NxSceneQuery
	*/
	NxSceneQueryReport*			report;

	/**
	\brief The method used to execute the queries. ie synchronous or asynchronous.

	WARNING: NxSceneDesc::backgroundThreadCount must be at least 1 for the asynchronous mode to work,
	         else the queries will be processed in synchronous mode.

	@see NxSceneQueryExecuteMode NxSceneQuery
	*/
	NxSceneQueryExecuteMode		executeMode;

								NxSceneQueryDesc()		{ setToDefault();									}

	void						setToDefault()			{ report = NULL; executeMode = NX_SQE_SYNCHRONOUS;	}
	bool						isValid()		const	{ return report && executeMode < 2;					}
	};

/**
\brief Batched queries object. This is used to perform several queries at the same time. The queries are the same as the
previously available scene queries in #NxScene.

@see NxSceneQueryReport NxSceneQueryExecuteMode
*/
class NxSceneQuery
	{
	public:

	/**
	\brief Gets report object

	\return The query report callback associated with this object.

	@see NxSceneQueryReport
	*/
	virtual	NxSceneQueryReport*		getQueryReport()			= 0;


	/**
	\brief Gets the execution mode

	\return The execution mode for this query object.

	@see NxSceneQueryExecuteMode
	*/
	virtual	NxSceneQueryExecuteMode	getExecuteMode()			= 0;

	/**
	\brief Executes batched queries.

	In the synchronous mode the batched queries are executed immediately. In the asynchronous mode
	they can be executed at any point between calling execute() and finish() returning true.

	WARNING: NxSceneDesc::backgroundThreadCount must be at least 1 for the asynchronous mode to work,
	         else the queries will be processed in synchronous mode.

	@see finish()
	*/
	virtual	void					execute()					= 0;

	/**
	\brief Used to determine if an execute call has completed.
	
	You should not call this without calling the execute function.
	
	When calling this method with the block parameter set to true, the method will not return until 
	all batched queries have been executed.

	\param[in] block Specifies if this function should wait until queries are complete.
	\return true when all queries have executed.

	@see execute()
	*/
	virtual	bool					finish(bool block=false)	= 0;

	/**
	\brief Check if a ray intersects any shape.

	This function returns false and calls the query report callback when the batched queries are executed.

	See #NxScene::raycastAnyShape() for more details.
	*/
	virtual bool					raycastAnyShape			(const NxRay& worldRay, NxShapesType shapesType, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, const NxGroupsMask* groupsMask=NULL, NxShape** cache=NULL, void* userData=NULL)	const = 0;
	
	/**
	\brief Check if a sphere overlaps shapes.

	This function returns false and calls the query report callback when the batched queries are executed.

	See #NxScene::checkOverlapSphere() for more details.
	*/
	virtual bool					checkOverlapSphere		(const NxSphere& worldSphere, NxShapesType shapesType=NX_ALL_SHAPES, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)							const = 0;
	
	/**
	\brief Check if a AABB overlaps shapes.

	This function returns false and calls the query report callback when the batched queries are executed.

	See #NxScene::checkOverlapAABB() for more details.
	*/
	virtual bool					checkOverlapAABB		(const NxBounds3& worldBounds, NxShapesType shapesType=NX_ALL_SHAPES, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)							const = 0;
	
	/**
	\brief Check if an OBB overlaps shapes.
	
	This function returns false and calls the query report callback when the batched queries are executed.

	See #NxScene::checkOverlapOBB() for more details.
	*/
	virtual bool					checkOverlapOBB			(const NxBox& worldBox, NxShapesType shapesType=NX_ALL_SHAPES, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)									const = 0;
	
	/**
	\brief Check if a capsule overlaps shapes.
	
	This function returns false and calls the query report callback when the batched queries are executed.

	See #NxScene::checkOverlapCapsule() for more details.
	*/
	virtual bool					checkOverlapCapsule		(const NxCapsule& worldCapsule, NxShapesType shapesType=NX_ALL_SHAPES, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)							const = 0;

	/**
	\brief Find the closest ray/shape intersection.

	This function returns NULL and calls the query report callback when the batched queries are executed.

	See #NxScene::raycastClosestShape() for more details.
	*/
	virtual NxShape*				raycastClosestShape		(const NxRay& worldRay, NxShapesType shapesType, NxRaycastHit& hit, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask* groupsMask=NULL, NxShape** cache=NULL, void* userData=NULL) const = 0;

	/**
	\brief Find all the shapes which a ray intersects.

	This function returns 0 and	calls the query report callback when the batched queries are executed.

	See #NxScene::raycastAllShapes() for more details.
	*/
	virtual NxU32					raycastAllShapes		(const NxRay& worldRay, NxShapesType shapesType, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL) const = 0;

	/**
	\brief Find all shapes which overlap a sphere.

	This function returns 0 and	calls the query report callback when the batched queries are executed.

	See #NxScene::overlapSphereShapes() for more details.
	*/
	virtual	NxU32					overlapSphereShapes		(const NxSphere& worldSphere, NxShapesType shapesType, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL) const	= 0;
	
	/**
	\brief Find all shapes which overlap an AABB.
	
	This function returns 0 and	calls the query report callback when the batched queries are executed.

	See #NxScene::overlapAABBShapes() for more details.
	*/
	virtual	NxU32					overlapAABBShapes		(const NxBounds3& worldBounds, NxShapesType shapesType, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL) const = 0;
	
	/**
	\brief Find all shapes which overlap an OBB.

	This function returns 0 and calls the query report callback when the batched queries are executed.


	See #NxScene::overlapOBBShapes() for more details.
	*/
	virtual	NxU32					overlapOBBShapes		(const NxBox& worldBox, NxShapesType shapesType, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)			const = 0;
	
	/**
	\brief Find all shapes which overlap a capsule.

	This function returns 0 and calls the query report callback when the batched queries are executed.
	
	See #NxScene::overlapCapsuleShapes() for more details.
	*/
	virtual	NxU32					overlapCapsuleShapes	(const NxCapsule& worldCapsule, NxShapesType shapesType, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)	const = 0;
	
	/**
	\brief Find the set of shapes which are in the negative half space of a number of planes.

	This function returns 0 and	calls the query report callback when the batched queries are executed.

	See #NxScene::cullShapes() for more details.
	*/
	virtual	NxU32					cullShapes				(NxU32 nbPlanes, const NxPlane* worldPlanes, NxShapesType shapesType, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)	const = 0;

	
	/**
	\brief Perform a linear OBB sweep.
	
	This function returns 0 and calls the query report callback when the batched queries are executed.

	See #NxScene::linearOBBSweep() for more details.
	*/
	virtual NxU32					linearOBBSweep			(const NxBox& worldBox, const NxVec3& motion, NxU32 flags, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)			const 	= 0;
	
	/**
	\brief Perform a linear capsule sweep.
	
	This function returns 0 and	calls the query report callback when the batched queries are executed.

	See #NxScene::linearCapsuleSweep() for more details.
	*/
	virtual	NxU32					linearCapsuleSweep		(const NxCapsule& worldCapsule, const NxVec3& motion, NxU32 flags, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, void* userData=NULL)	const	= 0;

	protected:
	virtual ~NxSceneQuery(){};
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
