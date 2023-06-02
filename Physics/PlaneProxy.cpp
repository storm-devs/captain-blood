#include "PlaneProxy.h"
#include "PhysicsScene.h"

PlaneProxy::PlaneProxy(const Vector & n, float d, PhysicsScene& scene) :
IPhysPlane(&scene),
IProxy(scene),
m_helper(m_calls),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_planeNormal(n),
m_planeDist(d),
m_realPlane(NULL)
{
	m_plane.d = m_planeDist;
	m_plane.n = m_planeNormal;
}

PlaneProxy::~PlaneProxy(void)
{
	if (m_realPlane)
		m_realPlane->Release(), m_realPlane = NULL;

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysPlane
//////////////////////////////////////////////////////////////////////////
//Установить	лоскость
void PlaneProxy::SetPlane(Plane & p)
{
	m_plane = p;
	m_calls.Add
	(
		MakeDeferrer<IPhysPlane, void, Plane&>(DEF_FREF(IPhysPlane::SetPlane), p)
	);
}

//Получить плоскость
Plane PlaneProxy::GetPlane()
{
	if (!m_realPlane)
	{
		return m_plane;
		/*Plane pln;
		pln.d = m_planeDist;
		pln.n = m_planeNormal;
		return pln;*/
	}
	return m_realPlane->GetPlane();
}


//////////////////////////////////////////////////////////////////////////
// IPhysRigidBody
//////////////////////////////////////////////////////////////////////////

//Получить позицию и ориентацию в мире
void PlaneProxy::GetTransform(Matrix & mtx)
{
	if (!m_realPlane)
		m_helper.GetTransform(mtx);
	else
		m_realPlane->GetTransform(mtx);
}

//Получить массу
float PlaneProxy::GetMass()
{
	if (!m_realPlane)
		return m_helper.GetMass();

	return m_realPlane->GetMass();
}

//Получить группы
PhysicsCollisionGroup PlaneProxy::GetGroup()
{
	if (!m_realPlane)
		return m_helper.GetGroup();

	return m_realPlane->GetGroup();
}

// получить внутренности - !только для использования внутри сервиса
void PlaneProxy::GetInternals(PhysInternal & internals) const
{
	if (!m_realPlane)
		throw "invalid call";
	return m_realPlane->GetInternals(internals);
}

// получить материал
IPhysMaterial * PlaneProxy::GetPhysMaterial() const
{
	if (!m_realPlane)
		return NULL;
	return m_realPlane->GetPhysMaterial();
}

// получить линейную составляющую скорости
Vector PlaneProxy::GetLinearVelocity() const
{
	if (!m_realPlane)
		return Vector(0.0f);
	return m_realPlane->GetLinearVelocity();
}


//Удалить
bool PlaneProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
}

//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void PlaneProxy::OnSyncCreate()
{
	if (m_realPlane)
		return;

	m_realPlane = NEW PhysPlane(GetFileName(), GetFileLine(), m_planeNormal, m_planeDist, &m_scene); 

	Assert(m_realPlane);

	SetRealPhysObject(m_realPlane);

	PhysInternal internals;
	m_realPlane->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;
}


void PlaneProxy::OnSyncCalls()
{
	Assert(m_realPlane);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realPlane);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();
}

bool PlaneProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void PlaneProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
