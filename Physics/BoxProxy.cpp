#include "BoxProxy.h"
#include "PhysicsScene.h"

extern bool PhysRigidBodyActorCheckMtx(const Matrix & mtx, NxActor * actor, const char * filename, long fileline);

BoxProxy::BoxProxy(const Vector & size, const Matrix & transform, bool isDynamic, float density, PhysicsScene& scene) :
IProxy(scene),
IPhysBox(&scene),
m_helper(m_calls),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_realBox(NULL),
m_size(size),
m_isDynamic(isDynamic),
m_density(density)
{
//#ifndef STOP_DEBUG
	Assert(PhysRigidBodyActorCheckMtx(transform, null, null, 0));
//#endif
	m_helper.m_transform = transform;
}

BoxProxy::~BoxProxy(void)
{
	if (m_realBox)
		m_realBox->Release(), m_realBox = NULL;

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysBox
//////////////////////////////////////////////////////////////////////////
//Установить размер ящика
void BoxProxy::SetSize(const Vector & size)
{
	m_size = size;
	m_calls.Add
	(
		MakeDeferrer<IPhysBox, void, const Vector&>("SetSize", &IPhysBox::SetSize, size)
	);
}

//Получить размер ящика
Vector BoxProxy::GetSize()
{
	if (m_realBox)
		return m_realBox->GetSize();
	return m_size;
}

//////////////////////////////////////////////////////////////////////////
// IPhysRigidBody
//////////////////////////////////////////////////////////////////////////

//Получить позицию и ориентацию в мире
void BoxProxy::GetTransform(Matrix & mtx)
{
	if (m_realBox)
		m_realBox->GetTransform(mtx);
	else
		m_helper.GetTransform(mtx);
}

//Получить массу
float BoxProxy::GetMass()
{
	if (m_realBox)
		return m_realBox->GetMass();

	return m_helper.GetMass();
}

//Установить центр масс
Vector BoxProxy::GetCenterMass()
{
	if (m_realBox)
		return m_realBox->GetCenterMass();

	return m_helper.GetCenterMass();
}

//Получить группы
PhysicsCollisionGroup BoxProxy::GetGroup()
{
	if (m_realBox)
		return m_realBox->GetGroup();
	return m_helper.GetGroup();
}

// получить внутренности - !только для использования внутри сервиса
void BoxProxy::GetInternals(PhysInternal & internals) const
{
	if (!m_realBox)
		throw "invalid call";
	m_realBox->GetInternals(internals);
}

// получить материал
IPhysMaterial * BoxProxy::GetPhysMaterial() const
{
	if (m_realBox)
		return m_realBox->GetPhysMaterial();
	return NULL;
}

// получить линейную составляющую скорости
Vector BoxProxy::GetLinearVelocity() const
{
	if (m_realBox)
		return m_realBox->GetLinearVelocity();
	return Vector(0.0f);
}

//////////////////////////////////////////////////////////////////////////
// IPhysBase
//////////////////////////////////////////////////////////////////////////
bool BoxProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
}

//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void BoxProxy::OnSyncCreate()
{
	if (m_realBox)
		return;

	m_realBox = NEW PhysBox(GetFileName(), GetFileLine(), m_size, m_helper.m_transform, m_isDynamic, m_density, &m_scene);

	Assert(m_realBox);

	SetRealPhysObject(m_realBox);

	PhysInternal internals;
	m_realBox->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;
}

void BoxProxy::OnSyncCalls()
{
	Assert(m_realBox);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realBox);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();
}

bool BoxProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void BoxProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
