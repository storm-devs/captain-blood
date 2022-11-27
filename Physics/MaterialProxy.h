#pragma once
#include "common.h"
#include "IProxy.h"
#include "Deferrer.h"

class PhysicsScene;
class MaterialProxy : public IPhysMaterial, public IProxy
{
	//PhysicsScene&						m_scene;
	IPhysMaterial*						m_realMaterial;
	DECLARE_MCALLS(IPhysMaterial)
	//array<IDeferrer<IPhysMaterial>*>	m_calls;

	float	m_statFriction;
	float	m_dynFriction;
	float	m_restitution;
public:
	
	MaterialProxy(PhysicsScene& scene, float statFriction, float dynFriction, float restitution);
	virtual ~MaterialProxy(void);

	//////////////////////////////////////////////////////////////////////////
	// IPhysMaterial
	//////////////////////////////////////////////////////////////////////////
	virtual void SetRestitution(float restitution);
	virtual float GetRestitution() const;
	virtual void SetDynamicFriction(float friction);
	virtual float GetDynamicFriction() const;
	virtual void SetStaticFriction(float friction);
	virtual float GetStaticFriction() const;
	virtual unsigned int GetIndex() const;

	//////////////////////////////////////////////////////////////////////////
	// IPhysBase
	//////////////////////////////////////////////////////////////////////////
	virtual bool Release();

	//////////////////////////////////////////////////////////////////////////
	// IProxy
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSyncCreate();
	virtual void OnSyncCalls();
	virtual bool OnSyncRelease();
	virtual void OnSyncTrace();
};
