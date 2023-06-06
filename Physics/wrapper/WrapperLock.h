#pragma once

#include <foundation/PxPreprocessor.h>
#include <PxScene.h>

namespace physx
{
class PhysX3LockRead
{
public:
	PhysX3LockRead(PxScene *scene,const char *fileName,int lineno) : mScene(scene)
	{
		if (mScene)
		{
			mScene->lockRead(fileName, (physx::PxU32)lineno);
		}
	}
	~PhysX3LockRead()
	{
		if (mScene)
		{
			mScene->unlockRead();
		}
	}
private:
	PxScene* mScene;
};

class PhysX3LockWrite
{
public:
	PhysX3LockWrite(PxScene *scene,const char *fileName,int lineno) : mScene(scene)
	{
		if (mScene)
		{
			mScene->lockWrite(fileName, (physx::PxU32)lineno);
		}
	}
	~PhysX3LockWrite()
	{
		if ( mScene )
		{
			mScene->unlockWrite();
		}
	}
private:
	PxScene* mScene;
};

}; // end physx namespace

#if defined(_DEBUG) || defined(PX_CHECKED)
#define PHYSX3_LOCK_WRITE(x) physx::PhysX3LockWrite _wlock(x,__FILE__,__LINE__);
#else
#define PHYSX3_LOCK_WRITE(x) physx::PhysX3LockWrite _wlock(x,"",__LINE__);
#endif

#if defined(_DEBUG) || defined(PX_CHECKED)
#define PHYSX3_LOCK_READ(x) physx::PhysX3LockRead _rlock(x,__FILE__,__LINE__);
#else
#define PHYSX3_LOCK_READ(x) physx::PhysX3LockRead _rlock(x,"",__LINE__);
#endif
