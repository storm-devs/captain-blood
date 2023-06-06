#include "ClothMeshBuilderProxy.h"

ClothMeshBuilderProxy::ClothMeshBuilderProxy(PxPhysics* sdk) :
ClothMeshBuilder(sdk),
IProxy(*((PhysicsScene*)null))
{
}

ClothMeshBuilderProxy::~ClothMeshBuilderProxy(void)
{
}

bool ClothMeshBuilderProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
}

bool ClothMeshBuilderProxy::OnSyncRelease()
{
	for (long i=0; i<IProxy::releaseCount; i++)
		if (ClothMeshBuilder::Release())
		{
			Assert(i == IProxy::releaseCount - 1);
			IProxy::releaseCount = 0;
			return true;
		}

	IProxy::releaseCount = 0;
	return false;
	//if (m_bReleaseCall)
	//	return ClothMeshBuilder::Release();
	//return false;
}

void ClothMeshBuilderProxy::OnSyncTrace()
{
}
