#ifndef NX_SCENE_STATS
#define NX_SCENE_STATS
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

/**
\brief Class used to retrieve statistics for a scene.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Partial (Actors, dynamic actors, joints)
\li PS3  : Yes
\li XB360: Yes

@see NxScene::getStats()
*/
class NxSceneStats
	{
	public:
//collisions:
	/**
	\brief Number of contacts present in the scene for the current simulation step. Not supported in 2.6.
	*/
	NxU32   numContacts;
	/**
	\brief Maximum number of contacts present in the scene since the scene was created. Not supported in 2.6.
	*/
	NxU32   maxContacts;
	/**
	\brief Number of shape pairs present in the scene for the current simulation step.
	*/
	NxU32	numPairs;
	/**
	\brief Maximum number of shape pairs present in the scene since the scene was created.
	*/
	NxU32	maxPairs;
//sleep:
	/**
	\brief Number of dynamic actors which are not part of a sleeping group(island)
	*/
	NxU32   numDynamicActorsInAwakeGroups;
	/**
	\brief Maximum number of actors which have been present in a non sleeping island since the scene was created.
	*/
	NxU32   maxDynamicActorsInAwakeGroups;
//solver:
	/**
	\brief The number of constraints(joints+contact) present in the current simulation step. Not supported in 2.6.
	*/
	NxU32	numAxisConstraints;
	/**
	\brief The maximum number of constraints(joints+contacts) present in the scene since creation. Not supported in 2.6.
	*/
	NxU32	maxAxisConstraints;
	/**
	\brief Number of solver bodies present in the current step(i.e. the number of bodies subject to constraints). Not supported in 2.6.
	*/
	NxU32	numSolverBodies;
	/**
	\brief Max number of solver bodies present in the scene since creation. Not supported in 2.6.
	*/
	NxU32	maxSolverBodies;
//scene:
	/**
	\brief Number of actors(static+dynamic) present in the scene for the current simulation step.
	*/
	NxU32   numActors;
	/**
	\brief Max number of actors(static+dynamic) present in the scene since the scene was created.
	*/
	NxU32	maxActors;
	/**
	\brief Number of dynamic actors present in the scene for the current simulation step.
	*/
	NxU32   numDynamicActors;
	/**
	\brief Max number of dynamic actors present in the scene since it was created.
	*/
	NxU32	maxDynamicActors;
	/**
	\brief Number of static shapes present in the scene.
	*/
	NxU32   numStaticShapes;
	/**
	\brief Max number of static shapes present in the scene since it was created.
	*/
	NxU32	maxStaticShapes;
	/**
	\brief Number of dynamic shapes present in the scene for the current simulation step.
	*/
	NxU32	numDynamicShapes;
	/**
	\brief Max number of dynamic actors present in the scene, since it was created.
	*/
	NxU32	maxDynamicShapes;
	/**
	\brief Number of joints in the scene. Note that this number also includes all "dead joints"
	in the scene (see NxScene.releaseActor).
	*/
	NxU32	numJoints;
	/**
	\brief Max number of joints in the scene since it was created.
	*/
	NxU32	maxJoints;


	NxSceneStats()
		{
		reset();
		}

	/**
	\brief Zeros all members.
	*/
	void reset()
		{
		numContacts = 0;
		maxContacts = 0;
		numPairs = 0;
		maxPairs = 0;
		numDynamicActorsInAwakeGroups = 0;
		maxDynamicActorsInAwakeGroups = 0;
		numAxisConstraints = 0;
		maxAxisConstraints = 0;
		numSolverBodies = 0;
		maxSolverBodies = 0;
		numActors = 0;
		maxActors = 0;
		numDynamicActors = 0;
		maxDynamicActors = 0;
		numStaticShapes = 0;
		maxStaticShapes = 0;
		numDynamicShapes = 0;
		maxDynamicShapes = 0;
		numJoints = 0;
		maxJoints = 0;
		}
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
