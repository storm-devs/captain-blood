#pragma once

#include "ClothMeshBuilder.h"
#include "IProxy.h"

class ClothMeshBuilderProxy : public ClothMeshBuilder, public IProxy
{
public:
	ClothMeshBuilderProxy(PxPhysics* sdk);
	virtual ~ClothMeshBuilderProxy(void);

	//////////////////////////////////////////////////////////////////////////
	// ClothMeshBuilder
	//////////////////////////////////////////////////////////////////////////

	virtual bool Release();

	//////////////////////////////////////////////////////////////////////////
	// IProxy
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSyncCreate() {}
	virtual void OnSyncCalls() {}
	virtual bool OnSyncRelease();
	virtual void OnSyncTrace();
};
