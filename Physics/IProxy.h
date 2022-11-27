#pragma once

#include "PhysicsScene.h"
#include "PhysInternal.h"

class IProxy
{
protected:
	PhysicsScene & m_scene;

	~IProxy() 
	{
		m_scene.UnregistryProxyObject(this); 

		if (m_dependences)
		{
			for (unsigned int i = 0; i < m_dependences->Size(); ++i)
				(*m_dependences)[i]->DecRef();

			m_dependences->DelAll();
			delete m_dependences;
		}
	}

public:
	IProxy(PhysicsScene& scene) :
		m_scene(scene)
	{
		releaseCount = 0;
		m_dependences = null;
	}

	virtual void OnSyncCreate() = null;
	virtual void OnSyncCalls()
	{
		if (!m_dependences) return;

		for (unsigned int i = 0; i < m_dependences->Size(); ++i)
			(*m_dependences)[i]->DecRef();

		m_dependences->Empty();
	}
	virtual bool OnSyncRelease() = null;

	// вывод накопленных m_copyCalls в системный лог
	virtual void OnSyncTrace() = null;

	bool AddReleaseCount()
	{
		releaseCount++;
		return false;
	}

	bool ReleaseCounts(IPhysBase * physObj)
	{
		Assert(physObj);

		long count = releaseCount;
		while (count)
		{
			count--;
			if (physObj->IPhysBase::Release())
			{
				Assert(count == 0);
				return true;
			}
		}

		releaseCount = 0;
		return false;
	}

protected:
	//массив объектов у которых был сделан AddRef, и которым надо сделать DecRef после IProxy::OnSyncCalls()
	array<IPhysBase*> * m_dependences;
	long releaseCount;

};