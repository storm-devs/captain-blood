#ifndef NX_SCENE_STATS2
#define NX_SCENE_STATS2
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

#include "NxArray.h"
#include "NxFoundationSDK.h"
#include "Nxp.h"

#define NX_ENABLE_SCENE_STATS2
/**
\brief Scene statistic counters
*/

struct NxSceneStatistic
{
	/** \brief Current value of the statistic
	*/
	NxI32 curValue;
	/** \brief Maximum value of the statistic over the entire lifetime of the scene
	*/
	NxI32 maxValue;
	/** \brief Pointer to the name of the statistic
	*/
	const char *name;
	/** \brief The index into the array of NxSceneStatistics (see #NxSceneStats2) of the parent of the statistic; if no parent exists, the value is 0xFFFFFFFF).
	*/
	NxU32 parent;
};

/**
\brief Class used to retrieve statistics for a scene.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxScene::getStats2()
*/
class NxSceneStats2
{
	public:
		/**\brief The number of #NxSceneStatistic structures stored.
		*/
		NxU32				numStats;
		/**\brief Array of #NxSceneStatistic structures.
		*/
		NxSceneStatistic	*stats;
};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
