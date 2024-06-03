#ifndef NX_FOUNDATION_NXPROFILER
#define NX_FOUNDATION_NXPROFILER
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

//#define NX_ENABLE_PROFILER		//!< Switch indicating if profiler is active.  Customer requested that this define be exposed so that user code can be conditionally compiled.
//#define NX_ENABLE_PROFILER_COUNTER //!< Enable an additional performance counter(eg L2 cache misses)

/**
\brief A profiling zone. 

A profiling zone is a named piece of code whose performance characteristics have been measured.
*/
class NxProfileZone
	{
	public:
	const char * name;			//!< Name of the zone.
	NxU32  callCount;			//!< The number of times this zone was executed over the last profiling run (since readProfileData(true) was called.)
	NxU32  hierTime;			//!< Time in micro seconds that it took to execute the total of the calls to this zone.
	NxU32  selfTime;			//!< Time in micro seconds that it took to execute the total of the calls to this zone, minus the time it took to execute the zones called from this zone.  
	NxU32  recursionLevel;		//!< The number of parent zones this zone has, each of which called the next until this zone was called.  Can be used to indent a tree display of the zones.  Sometimes a zone could have multiple rec. levels as it was called from different places.  In this case the first encountered rec level is displayed.
	NxReal percent;				//!< The percentage time this zone took of its parent zone's time.  If this zone has multiple parents (the code was called from multiple places), this is zero. 

#ifdef NX_ENABLE_PROFILER_COUNTER
	NxU32 counter;
#endif
	};


/**
\brief Array of profiling data. 

 profileZones points to an array of numZones profile zones.  Zones are sorted such that the parent zones always come before their children.  
 Some zones have multiple parents (code called from multiple places) in which case only the relationship to the first parent is displayed.
 returned by NxScene::readProfileData().
*/
class NxProfilerData
	{
	public:
	NxU32 numZones;
	NxProfileZone * profileZones;
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
