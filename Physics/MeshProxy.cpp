#include "MeshProxy.h"
#include "PhysicsScene.h"

extern bool PhysRigidBodyActorCheckMtx(const Matrix & mtx, NxActor * actor, const char * filename, long fileline);

MeshProxy::MeshProxy(const IPhysicsScene::MeshInit * meshes, dword numMeshes, bool bDynamic, float density, PhysicsScene& scene) :
IPhysMesh(&scene),
IProxy(scene),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_helper(m_calls),
m_bDynamic(bDynamic),
m_realMesh(NULL),
m_density(density),
m_meshes(_FL_, 1)
{
	m_meshes.AddElements(numMeshes);
	for (long i=0; i<(long)numMeshes; i++)
	{
		Assert(PhysRigidBodyActorCheckMtx(meshes[i].mtx, null, GetFileName(), GetFileLine()));

		m_meshes[i] = meshes[i];
		m_meshes[i].mesh->AddRef();
	}
}

MeshProxy::~MeshProxy(void)
{
	if (m_realMesh)
		m_realMesh->Release(), m_realMesh = NULL;

	for (int i=0; i<m_meshes.Len(); i++)
		m_meshes[i].mesh->Release();

	m_meshes.DelAll();

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysRigidBody
//////////////////////////////////////////////////////////////////////////
//Получить позицию и ориентацию в мире
void MeshProxy::GetTransform(Matrix & mtx)
{
	if (!m_realMesh)
		m_helper.GetTransform(mtx);
	else
		m_realMesh->GetTransform(mtx);
}

//Получить массу
float MeshProxy::GetMass()
{
	if (!m_realMesh)
		return m_helper.GetMass();

	return m_realMesh->GetMass();
}

//Установить центр масс
Vector MeshProxy::GetCenterMass()
{
	if (!m_realMesh)
		return m_helper.GetCenterMass();

	return m_realMesh->GetCenterMass();
}

//Получить группы
PhysicsCollisionGroup MeshProxy::GetGroup()
{
	if (!m_realMesh)
		return m_helper.GetGroup();

	return m_realMesh->GetGroup();
}

// получить внутренности - !только для использования внутри сервиса
void MeshProxy::GetInternals(PhysInternal & internals) const
{
	if (!m_realMesh)
		throw "invalid call";
	return m_realMesh->GetInternals(internals);
}

// получить материал
IPhysMaterial * MeshProxy::GetPhysMaterial() const
{
	if (!m_realMesh)
		return NULL;
	return m_realMesh->GetPhysMaterial();
}

// получить линейную составляющую скорости
Vector MeshProxy::GetLinearVelocity() const
{
	if (!m_realMesh)
		return Vector(0.0f);
	return m_realMesh->GetLinearVelocity();
}

//////////////////////////////////////////////////////////////////////////
// IPhysBase
//////////////////////////////////////////////////////////////////////////
bool MeshProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
}

//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void MeshProxy::OnSyncCreate()
{
	if (m_realMesh)
		return;

	m_realMesh = NEW PhysMesh(GetFileName(), GetFileLine(), m_meshes.GetBuffer(), m_meshes.Len(), m_bDynamic, m_density, &m_scene);

	for (long i=0; i<m_meshes.Len(); i++)
		m_meshes[i].mesh->Release();

	m_meshes.DelAll();

	Assert(m_realMesh);

	SetRealPhysObject(m_realMesh);

	PhysInternal internals;
	m_realMesh->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;
}

void MeshProxy::OnSyncCalls()
{
	Assert(m_realMesh);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realMesh);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();
}

bool MeshProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void MeshProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
