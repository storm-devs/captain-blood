#include "CombinedProxy.h"
#include "PhysicsScene.h"

CombinedProxy::CombinedProxy(const char * cppfile, long cppline, const Matrix & transform, bool isDynamic, float density, PhysicsScene& _scene) :
IPhysCombined(&_scene),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_helper(m_calls),
IProxy(_scene),
m_realCombined(NULL),
m_isBuilt(false),
m_haveBuildCommand(false),
m_totalMass(0.0f),
m_descs(__FILE__, __LINE__),
m_density(density)
{
	SetFileLine(cppfile, cppline);
	m_helper.m_transform = transform;
	m_realCombined = NEW PhysCombined(GetFileName(), GetFileLine(), transform, isDynamic, m_density, &m_scene);
	SetRealPhysObject(m_realCombined);
	Assert(m_realCombined);	
}	

CombinedProxy::~CombinedProxy(void)
{
	if (m_realCombined)
		m_realCombined->Release(), m_realCombined = NULL;

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysRigidBody
//////////////////////////////////////////////////////////////////////////
//Получить позицию и ориентацию в мире
void CombinedProxy::GetTransform(Matrix & mtx)
{
	if (m_haveBuildCommand && m_isBuilt)
		m_realCombined->GetTransform(mtx);
	else
		m_helper.GetTransform(mtx);
}

//Получить массу
float CombinedProxy::GetMass()
{
	if (m_haveBuildCommand && m_isBuilt)
		return m_realCombined->GetMass();
	else
		return m_totalMass;
}

//Получить центр масс
Vector CombinedProxy::GetCenterMass()
{
	if (m_haveBuildCommand && m_isBuilt)
		return m_realCombined->GetCenterMass();
	else
		return m_helper.GetCenterMass();
}

//Получить группы
PhysicsCollisionGroup CombinedProxy::GetGroup()
{
	if (m_haveBuildCommand && m_isBuilt)
		return m_realCombined->GetGroup();
	else
		return m_helper.GetGroup();
}

//Приложить силу к кости в заданной локальной точке
void CombinedProxy::ApplyForce(const Vector & force, const Vector & localPosition)
{
	m_helper.ApplyForce(force, localPosition);
}

//Приложить импульс в заданной локальной точке
void CombinedProxy::ApplyImpulse(const Vector & force, const Vector & localPosition)
{
	m_helper.ApplyImpulse(force, localPosition);
}

// приложить вращающий момент в СК актера
void CombinedProxy::ApplyLocalTorque(const Vector& torque)
{
	m_helper.ApplyLocalTorque(torque);
}

// установить затухание движения
void CombinedProxy::SetMotionDamping ( float fDamping )
{
	m_helper.SetMotionDamping(fDamping);
}

void CombinedProxy::SetMotionDamping ( float fLinDamping, float fAngDamping )
{
	m_helper.SetMotionDamping(fLinDamping, fAngDamping);
}

// включить-выключить гравитацию
void CombinedProxy::EnableGravity ( bool bEnable )
{
	m_helper.EnableGravity(bEnable);
}

// включить-выключить кинематическое поведение
void CombinedProxy::EnableKinematic(bool bEnable)
{
	m_helper.EnableKinematic(bEnable);
}

//Включить-выключить объект
void CombinedProxy::Activate(bool isActive)
{
	m_helper.Activate(isActive);
}

// получить внутренности - !только для использования внутри сервиса
void CombinedProxy::GetInternals(PhysInternal & internals) const
{
	return m_realCombined->GetInternals(internals);
}

// установить материал
void CombinedProxy::SetPhysMaterial(IPhysMaterial * material)
{
	m_helper.SetPhysMaterial(material);
}

// получить материал
IPhysMaterial * CombinedProxy::GetPhysMaterial() const
{
	return m_realCombined->GetPhysMaterial();
}

// получить линейную составляющую скорости
Vector CombinedProxy::GetLinearVelocity() const
{
	return m_velocity;
	//return m_realCombined->GetLinearVelocity();
}

//////////////////////////////////////////////////////////////////////////
// IPhysCombined
//////////////////////////////////////////////////////////////////////////
//Добавить ящик
void CombinedProxy::AddBox(const Vector & size, const Matrix & transform)
{
	m_realCombined->AddBox(size, transform);

	AddObjectDesc(transform, NxComputeBoxMass(Nx(size), m_density), size * 0.5f);
}

//Добавить шар
void CombinedProxy::AddSphere(float radius, const Matrix & transform)
{
	m_realCombined->AddSphere(radius, transform);

	AddObjectDesc(transform, NxComputeSphereMass(radius, m_density), 0.0f, radius);
}

//Добавить капсулу
void CombinedProxy::AddCapsule(float radius, float height, const Matrix & transform)
{
	m_realCombined->AddCapsule(radius, height, transform);

	AddObjectDesc(transform, NxComputeSphereMass(radius, m_density) + NxComputeCylinderMass(radius, height, m_density), 0.0f, radius, height);
}

void CombinedProxy::AddObjectDesc(const Matrix & transform, float fMass, const Vector & size, float radius, float height)
{
	ObjectDesc & obj = m_descs[m_descs.Add()];

	obj.mTransform = transform;
	obj.fMass = fMass;

	obj.size = size;
	obj.radius = radius;
	obj.height = height;

	m_totalMass += fMass;
}

bool CombinedProxy::SetMass(unsigned int index, float mass)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, bool, unsigned int, float>(DEF_FREF(IPhysCombined::SetMass), index, mass)
	);
	m_totalMass -= m_descs[index].fMass;
	m_descs[index].fMass = mass;
	m_totalMass += mass;
	return true;
}

//Получить текущее число фигур для билда
unsigned int CombinedProxy::GetCountForBuild()
{
	return m_realCombined->GetCountForBuild();
}

//Сконструировать объект
void CombinedProxy::Build()
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, void>(DEF_FREF(IPhysCombined::Build))
	);
	m_haveBuildCommand = true;
}


//Получить количество фигур
long CombinedProxy::GetCount()
{
	return m_realCombined->GetCount();
}

//Установить локальную позицию
bool CombinedProxy::SetLocalTransform(long index, const Matrix & transform)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, bool, long, const Matrix&>(DEF_FREF(IPhysCombined::SetLocalTransform), index, transform)
	);
	m_descs[index].mTransform = transform;
	return true;
}

//Получить локальную позицию
bool CombinedProxy::GetLocalTransform(long index, Matrix & transform)
{
	if (m_haveBuildCommand && m_isBuilt)
		return m_realCombined->GetLocalTransform(index, transform);

	transform = m_descs[index].mTransform;
	return true;
}

//Получить тип
IPhysCombined::Type CombinedProxy::GetType(long index)
{
	return m_realCombined->GetType(index);
}

//Получить параметры ящика
bool CombinedProxy::GetBox(long index, Vector & size)
{
	if (m_haveBuildCommand && m_isBuilt)
		return m_realCombined->GetBox(index, size);

	size = m_descs[index].size;
	return true;
}

//Получить параметры шара
bool CombinedProxy::GetSphere(long index, float & radius)
{
	if (m_haveBuildCommand && m_isBuilt)
		return m_realCombined->GetSphere(index, radius);

	radius = m_descs[index].radius;
	return true;
}

//Получить параметры капсулы
bool CombinedProxy::GetCapsule(long index, float & radius, float & height)
{
	return m_realCombined->GetCapsule(index, radius, height);

	radius = m_descs[index].radius;
	height = m_descs[index].height;
	return true;
}

//Установить глобальную позицию
bool CombinedProxy::SetGlobalTransform(unsigned int index, const Matrix & transform)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, bool, unsigned int, const Matrix&>(DEF_FREF(IPhysCombined::SetGlobalTransform), index, transform)
	);

	if (m_haveBuildCommand && m_isBuilt) return true;

	Matrix mGlobal;
	GetTransform(mGlobal);
	mGlobal.Inverse();
	m_descs[index].mTransform.EqMultiplyFast(transform, mGlobal);
	return true;
}

//Получить глобальную позицию
bool CombinedProxy::GetGlobalTransform(unsigned int index, Matrix & transform)
{
	if (m_haveBuildCommand && m_isBuilt)
		return m_realCombined->GetGlobalTransform(index, transform);

	Matrix mGlobal;
	GetTransform(mGlobal);
	transform.EqMultiplyFast(m_descs[index].mTransform, mGlobal);
	return true;
}

//Включить/выключить коллизии
void CombinedProxy::EnableCollision(bool enable)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, void, bool>(DEF_FREF(IPhysCombined::EnableCollision), enable)
	);
}

void CombinedProxy::EnableCollision(unsigned int index, bool enable)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, void, unsigned int, bool>(DEF_FREF(IPhysCombined::EnableCollision), index, enable)
	);
}

//Включить/выключить реакцию на коллизии
void CombinedProxy::EnableResponse(unsigned int index, bool enable)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, void, unsigned int, bool>(DEF_FREF(IPhysCombined::EnableResponse), index, enable)
	);
}

//Включить/выключить рэйкаст
void CombinedProxy::EnableRaycast(unsigned int index, bool enable)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, void, unsigned int, bool>(DEF_FREF(IPhysCombined::EnableRaycast), index, enable)
	);
}

//Включить/выключить визуализацию
void CombinedProxy::EnableVisualization(unsigned int index, bool enable)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, void, unsigned int, bool>(DEF_FREF(IPhysCombined::EnableVisualization), index, enable)
	);
}

//Включить/выключить визуализацию для актера
void CombinedProxy::EnableVisualization(bool enable)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, void, bool>(DEF_FREF(IPhysCombined::EnableVisualization), enable)
	);
}

// установить кол-во итерация солвера для актера
void CombinedProxy::SetSolverIterations(unsigned int count)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCombined, void, unsigned int>(DEF_FREF(IPhysCombined::SetSolverIterations), count)
	);
}

//Протестировать элемент на пересечение с лучом
bool CombinedProxy::Raycast(unsigned int index, const Vector& from, const Vector& to, RaycastResult * details)
{
	return m_realCombined->Raycast(index, from, to, details);
}

//Протестировать элемент на пересечение с боксом
bool CombinedProxy::OverlapBox(unsigned int index, const Vector& size, const Matrix& transform )
{
	//m_scene.lockQueries();
		bool res = m_realCombined->OverlapBox(index, size, transform);
	//m_scene.unlockQueries();
	return res;
}

//Протестировать элемент на пересечение со сферой
bool CombinedProxy::OverlapSphere(unsigned int index, const Vector& center, float radius)
{
	//m_scene.lockQueries();
		bool res = m_realCombined->OverlapSphere(index, center, radius);
	//m_scene.unlockQueries();
	return res;
}


//////////////////////////////////////////////////////////////////////////
// IPhysBase
//////////////////////////////////////////////////////////////////////////
bool CombinedProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
}


//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void CombinedProxy::OnSyncCreate()
{
	
}

void CombinedProxy::OnSyncCalls()
{
	Assert(m_realCombined);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realCombined);
		def_delete(m_calls[i]);
	}

	if ( m_haveBuildCommand )
		m_isBuilt = true;

	PhysInternal internals;
	m_realCombined->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;

	m_velocity = m_realCombined->GetLinearVelocity();
	m_calls.Empty();

	//float fNewMass = m_realCombined->GetMass();
}

bool CombinedProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void CombinedProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
