#include "CapsuleProxy.h"
#include "PhysicsScene.h"

CapsuleProxy::CapsuleProxy(float radius, float height, const Matrix& transform, bool isDynamic, float density, PhysicsScene& scene) :
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_helper(m_calls),
m_realCapsule(NULL),
IPhysCapsule(&scene),
IProxy(scene),
m_radius(radius),
m_height(height),
m_isDynamic(isDynamic),
m_density(density)
{
	m_helper.m_transform = transform;
}

CapsuleProxy::~CapsuleProxy(void)
{
	if (m_realCapsule)
		m_realCapsule->Release(), m_realCapsule = NULL;

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysCapsule
//////////////////////////////////////////////////////////////////////////
//Установить радиус
void CapsuleProxy::SetRadius(float radius)
{
	m_radius = radius;
	m_calls.Add
	(
		MakeDeferrer<IPhysCapsule, void, float>(DEF_FREF(IPhysCapsule::SetRadius), radius)
	);
}

//Получить радиус
float CapsuleProxy::GetRadius()
{
	if (!m_realCapsule)
		return m_radius;
	return m_realCapsule->GetRadius();
}

//Установить высоту
void CapsuleProxy::SetHeight(float height)
{
	m_height = height;
	m_calls.Add
	(
		MakeDeferrer<IPhysCapsule, void, float>(DEF_FREF(IPhysCapsule::SetHeight), height)
	);
}

//Получить высоту
float CapsuleProxy::GetHeight()
{
	if (!m_realCapsule)
		return m_height;
	return m_realCapsule->GetHeight();
}

//////////////////////////////////////////////////////////////////////////
// IPhysRigidBody
//////////////////////////////////////////////////////////////////////////
//Получить позицию и ориентацию в мире
void CapsuleProxy::GetTransform(Matrix & mtx)
{
	if (!m_realCapsule)
		m_helper.GetTransform(mtx);
	else
		m_realCapsule->GetTransform(mtx);
}

//Получить массу
float CapsuleProxy::GetMass()
{
	if (!m_realCapsule)
		return m_helper.GetMass();

	return m_realCapsule->GetMass();
}

//Получить группы
PhysicsCollisionGroup CapsuleProxy::GetGroup()
{
	if (!m_realCapsule)
		return m_helper.GetGroup();

	return m_realCapsule->GetGroup();
}

// получить внутренности - !только для использования внутри сервиса
void CapsuleProxy::GetInternals(PhysInternal & internals) const
{
	if (!m_realCapsule)
		throw "invalid call";
	return m_realCapsule->GetInternals(internals);
}

// получить материал
IPhysMaterial * CapsuleProxy::GetPhysMaterial() const
{
	if (!m_realCapsule)
		return NULL;
	return m_realCapsule->GetPhysMaterial();
}

// получить линейную составляющую скорости
Vector CapsuleProxy::GetLinearVelocity() const
{
	if (!m_realCapsule)
		return Vector(0.0f);
	return m_realCapsule->GetLinearVelocity();
}

//////////////////////////////////////////////////////////////////////////
// IPhysBase
//////////////////////////////////////////////////////////////////////////
bool CapsuleProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
}

//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void CapsuleProxy::OnSyncCreate()
{
	if (m_realCapsule)
		return;

	m_realCapsule = NEW PhysCapsule(GetFileName(), GetFileLine(), m_radius, m_height, m_helper.m_transform, m_isDynamic, m_density, &m_scene);

	Assert(m_realCapsule);

	SetRealPhysObject(m_realCapsule);

	PhysInternal internals;
	m_realCapsule->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;
}

void CapsuleProxy::OnSyncCalls()
{
	Assert(m_realCapsule);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realCapsule);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();
}

bool CapsuleProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void CapsuleProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
