#include "SphereProxy.h"
#include "PhysicsScene.h"

SphereProxy::SphereProxy(float radius, const Matrix& transform, bool isDynamic, float density, PhysicsScene& scene):
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_helper(m_calls),
m_realSphere(NULL),
IPhysSphere(&scene),
IProxy(scene),
m_radius(radius),
m_isDynamic(isDynamic),
m_density(density)
{
	m_helper.m_transform = transform;
}

SphereProxy::~SphereProxy(void)
{
	if (m_realSphere)
		m_realSphere->Release(), m_realSphere = NULL;

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysSphere
//////////////////////////////////////////////////////////////////////////
//Установить радиус
void SphereProxy::SetRadius(float radius)
{
	m_radius = radius;
	m_calls.Add
	(
		MakeDeferrer<IPhysSphere, void, float>(DEF_FREF(IPhysSphere::SetRadius), radius)
	);
}

//Получить радиус
float SphereProxy::GetRadius()
{
	if (!m_realSphere)
		return m_radius;
	return m_realSphere->GetRadius();
}

//////////////////////////////////////////////////////////////////////////
// IPhysRigidBody
//////////////////////////////////////////////////////////////////////////
//Получить позицию и ориентацию в мире
void SphereProxy::GetTransform(Matrix & mtx)
{
	if (!m_realSphere)
		m_helper.GetTransform(mtx);
	else
		m_realSphere->GetTransform(mtx);
}

//Получить массу
float SphereProxy::GetMass()
{
	if (!m_realSphere)
		return m_helper.GetMass();

	return m_realSphere->GetMass();
}

//Установить центр масс
Vector SphereProxy::GetCenterMass()
{
	if (!m_realSphere)
		return m_helper.GetCenterMass();

	return m_realSphere->GetCenterMass();
}

//Получить группы
PhysicsCollisionGroup SphereProxy::GetGroup()
{
	if (!m_realSphere)
		return m_helper.GetGroup();

	return m_realSphere->GetGroup();
}

// получить внутренности - !только для использования внутри сервиса
void SphereProxy::GetInternals(PhysInternal & internals) const
{
	if (!m_realSphere)
		throw "invalid call";
	return m_realSphere->GetInternals(internals);
}

// получить материал
IPhysMaterial * SphereProxy::GetPhysMaterial() const
{
	if (!m_realSphere)
		return NULL;
	return m_realSphere->GetPhysMaterial();
}

// получить линейную составляющую скорости
Vector SphereProxy::GetLinearVelocity() const
{
	if (!m_realSphere)
		return Vector(0.0f);
	return m_realSphere->GetLinearVelocity();
}

//////////////////////////////////////////////////////////////////////////
// IPhysBase
//////////////////////////////////////////////////////////////////////////
bool SphereProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
}

//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void SphereProxy::OnSyncCreate()
{
	if (m_realSphere)
		return;

	m_realSphere = NEW PhysSphere(GetFileName(), GetFileLine(), m_radius, m_helper.m_transform, m_isDynamic, m_density, &m_scene);

	Assert(m_realSphere);

	SetRealPhysObject(m_realSphere);

	PhysInternal internals;
	m_realSphere->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;
}

void SphereProxy::OnSyncCalls()
{
	Assert(m_realSphere);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realSphere);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();
}

bool SphereProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void SphereProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
