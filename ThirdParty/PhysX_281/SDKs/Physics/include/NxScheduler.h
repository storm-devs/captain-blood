#ifndef NX_PHYSICS_NX_SCHEDULER
#define NX_PHYSICS_NX_SCHEDULER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

/**
\brief Interface to an SDK task.

The SDK submits NxTasks to users custom scheduler for execution on an appropriate thread.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : No
\li XB360: Yes

@see NxScheduler NxTask.execute()
*/
class NxTask
	{
	protected:

	virtual	~NxTask()	{}

	public:

    /**
    \brief Execute the task supplied by the SDK

	This method can be called from an arbitary thread. Note however that each additional thread
	calling this function will incur a memory overhead for thread specific data.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see NxScheduler
	*/
    virtual void execute() = 0;
	};

/**
\brief A user defined callback class to allow arbitary scheduling of work between threads.

The user can supply a pointer to a derived class when creating a scene. Subsequently the SDK will
call methods of this class with tasks(#NxTask) which the application should execute on an appropriate thread.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : No
\li XB360: Yes

@see NxSceneDesc.customScheduler NX_SF_ENABLE_MULTITHREAD NxTask
*/
class NxUserScheduler
	{
	public:
    /**
	\brief The SDK calls this method to add a task to the applications work queue

	When a task has been added to the work queue the application should execute the task as soon as possible,
	from an arbitary thread context.

	\param[in] task for the application to execute

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see NxTask
     */
    virtual void addTask(NxTask *task) = 0;

   /**
	\brief The SDK calls this method to add a background task to the applications work queue

	The timing of background tasks is not as critical as other tasks. The application
	is also not required to execute background tasks when waitTasksComplete() is called.

	\note At present background tasks are not dispatched on any platform(this may change in the future).

	\param[in] task for the application to execute

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see NxTask
     */
	virtual void addBackgroundTask(NxTask *task)=0;

    /**
	\brief The SDK thread calls this function to suspend.
	
	When the SDK calls this method the user should block until all the tasks which have previously been submitted
	with addTask() are complete.

	A typical implementation of this method begins executing tasks in the work queue. Then when the queue is complete,
	it blocks until all other threads have finished executing their tasks.

	Note: Tasks submitted by addBackgroundTask() need not be completed before this method returns.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see NxTask
     */
    virtual void waitTasksComplete() = 0;

	protected:
	virtual ~NxUserScheduler(){};
	};
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
