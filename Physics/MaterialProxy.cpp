#include "MaterialProxy.h"
#include "PhysicsScene.h"
#include "PhysMaterial.h"

MaterialProxy::MaterialProxy(PhysicsScene& scene, float statFriction, float dynFriction, float restitution) :
IPhysMaterial(&scene),
IProxy(scene),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__)
m_realMaterial(NULL),
m_statFriction(statFriction),
m_dynFriction(dynFriction),
m_restitution(restitution)
{
}

MaterialProxy::~MaterialProxy(void)
{
	if (m_realMaterial)
		m_realMaterial->Release(), m_realMaterial = NULL;

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysMaterial
//////////////////////////////////////////////////////////////////////////
void MaterialProxy::SetRestitution(float restitution)
{
	m_restitution = restitution;
	m_calls.Add
	(
		MakeDeferrer<IPhysMaterial, void, float>(DEF_FREF(IPhysMaterial::SetRestitution), restitution)
	);
}

float MaterialProxy::GetRestitution() const
{
	if (!m_realMaterial)
		return m_restitution;
	return m_realMaterial->GetRestitution();
}

void MaterialProxy::SetDynamicFriction(float friction)
{
	m_dynFriction = friction;
	m_calls.Add
	(
		MakeDeferrer<IPhysMaterial, void, float>(DEF_FREF(IPhysMaterial::SetDynamicFriction), friction)
	);
}

float MaterialProxy::GetDynamicFriction() const
{
	if (!m_realMaterial)
		return m_dynFriction;
	return m_realMaterial->GetDynamicFriction();
}

void MaterialProxy::SetStaticFriction(float friction)
{
	m_statFriction = friction;
	m_calls.Add
	(
		MakeDeferrer<IPhysMaterial, void, float>(DEF_FREF(IPhysMaterial::SetStaticFriction), friction)
	);
}

float MaterialProxy::GetStaticFriction() const
{
	if (!m_realMaterial)
		return m_statFriction;
	return m_realMaterial->GetStaticFriction();
}

unsigned int MaterialProxy::GetIndex() const
{
	if (!m_realMaterial)
		throw "Invalid call";
	return m_realMaterial->GetIndex();
}


//////////////////////////////////////////////////////////////////////////
// IPhysBase
//////////////////////////////////////////////////////////////////////////
bool MaterialProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
	/*if (!m_releaseCall)
		m_releaseCall = MakeDeferrer<IPhysMaterial, void>(DEF_FREF(IPhysMaterial::Release));
	else
	{
		m_realMaterial->Release(), m_realMaterial = NULL;
		def_delete(m_releaseCall), m_releaseCall = NULL;
		IPhysBase::Release();
	}*/
}


//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void MaterialProxy::OnSyncCreate()
{
	if (m_realMaterial)
		return;

	NxMaterialDesc mtlDesc;
	mtlDesc.staticFriction = m_statFriction;
	mtlDesc.dynamicFriction = m_dynFriction;
	mtlDesc.restitution = m_restitution;
	NxMaterial * mtl = m_scene.Scene().createMaterial(mtlDesc);

	m_realMaterial = NEW PhysMaterial(GetFileName(), GetFileLine(), &m_scene, mtl);

	Assert(m_realMaterial);

	SetRealPhysObject(m_realMaterial);
}

void MaterialProxy::OnSyncCalls()
{
	Assert(m_realMaterial);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realMaterial);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();
}

bool MaterialProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void MaterialProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
