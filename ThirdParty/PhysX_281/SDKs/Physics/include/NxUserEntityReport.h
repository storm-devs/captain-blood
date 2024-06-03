#ifndef NX_PHYSICS_NXUSERENTITYREPORT
#define NX_PHYSICS_NXUSERENTITYREPORT
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
	\brief The user needs to pass an instance of this class to several of the scene collision
	routines in NxScene.
	
	There are usually two ways to report a variable set of entities
	to the user: using a dynamic array (e.g. an STL vector) or using a callback (called
	for each entity). The first way is fast, but can lead to dynamic allocations and/or
	wasted memory when it would just be possible to handle each entity on-the-fly. The
	second way provides this, but suffers from call overhead. This class combines best
	of both worlds by reporting a small number of entities to the user at the same time,
	via a callback (the onEvent function). This number of entities is user-defined, hence
	it is possible to fallback to the usual array or callback behaviours by adjusting it.
	(Please refer to the comments below for details).

	Returning true lets the SDK continue the collision query, returning false stops it.

	NxUserEntityReport usage:

	A typical collision function will look like this:

	NxU32 someCollisionFunc(NxU32 nbEntities, const Entity** entities, NxUserEntityReport<const Entity*>* callback);

	You have multiple ways to use this:

	1) Using a fixed-size array in your app. This can happen for example when you query
	some entities, and you already have a buffer big enough to handle all of them. In
	that case you don't have to worry about dynamic allocations or callbacks, just do:

	// maxEntities = size of entity buffer
	// entityBuffer = a buffer large enough to contain all entities
	NxU32 nbEntities = someCollisionFunc(maxEntities, entityBuffer, NULL);

	If the buffer is not large enough, the SDK will keep writing entities to the buffer
	until it is full, and then return. You might miss some entities in this case.

	2) Using a callback. This is the default way to use NxUserEntityReport.

	// First define your callback object

	class UserEntityReport : public NxUserEntityReport<Entity*>
		{
		public:

		virtual bool onEvent(NxU32 nbEntities, Entity** entities);
		};

	UserEntityReport myReport;

	// Then later, do the collision calls

	NxU32 nbEntities = someCollisionFunc(0, NULL, &myReport);

	In this case, the SDK will report a limited set of entities at the same time
	(usually 64), through the onEvent() function. The memory used to store
	those entities is internally allocated on the stack.

	3) Using a callback and your own memory buffers. this is the same as the previous
	version, except you can customize the query by specifying both a memory buffer
	and a callback:

	NxU32 nbEntities = someCollisionFunc(maxEntities, myBuffer, &myReport);

	In this case, entities are reported through the onEvent() function as in case 2),
	but those differences apply:

	- the number of entities reported at the same time ("nbEntities" parameter in
	onEvent) is lesser or equal to "maxEntities".

	- the memory used to store the entities ("entities" parameter in onEvent) is the
	same as the user's input buffer ("myBuffer").

	This 3rd way to use NxUserEntityReport is only provided in sake of flexibility,
	but should not be needed in most cases.

	 <b>Threading:</b> It is not necessary to make this class thread safe as it will only be called in the context of the
     user thread.

*/

template<class T>
class NxUserEntityReport
	{
	public:

	/**
	\brief This is called to report a number of entities to the user.

	'nbEntities' is the number of returned entities, which are stored in the
	'entities' memory buffer.

	After processing the entities in your application, return true to continue
	the query (in which case onEvent might get called again), or false to end it.

	\note SDK state should not be modified from within onEvent(). In particular objects should not
	be created or destroyed. If state modification is needed then the changes should be stored to a buffer
	and performed after the query.

	\param[in] nbEntities The number of returned entities, which are stored in the 'entities' memory buffer.
	\param[in] entities Array of entities.
	\return true to continue processing, false to end processing.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool onEvent(NxU32 nbEntities, T* entities) = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
