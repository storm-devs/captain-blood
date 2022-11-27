#include "ClothProxy.h"
#include "PhysicsScene.h"
#include "PhysCloth.h"

ClothProxy::ClothProxy(	PhysicsScene& scene, IClothRenderInfo& renderInfo,
					   IClothMeshBuilder& clothMeshBuilder, SimulationData& clothSimData, float density) :
IPhysCloth(&scene),
IProxy(scene),
m_renderInfo(renderInfo),
m_clothMeshBuilder(clothMeshBuilder),
m_clothSimData(clothSimData),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_realCloth(NULL),
m_tearcoef(0.0f),
m_attachmenttearcoef(0.0f),
m_acceleration(0.0f),
m_density(density),
m_recreateIndexes(_FL_, 32)
{
	m_clothMeshBuilder.AddRef();
	m_clothSimData.AddRef();
	m_renderInfo.AddRef();

	IProxy::m_dependences = NEW array<IPhysBase*>(_FL_);
}

ClothProxy::~ClothProxy(void)
{
	if (m_realCloth)
		m_realCloth->Release(), m_realCloth = NULL;

	// освобождаем все physrigidbody 
	IProxy::OnSyncCalls();

	m_clothMeshBuilder.Release();
	m_clothSimData.Release();
	m_renderInfo.Release();

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}


// присоединить ткань ко всем объектам которые она пересекает
void ClothProxy::Attach(bool twoWayInteraction, bool tearable)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, bool, bool>(DEF_FREF(IPhysCloth::Attach), twoWayInteraction, tearable)
	);
}

// присоединить ткань ближайшим вертексом к мировой точке
unsigned int ClothProxy::Attach(const Vector& pos)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, unsigned int, const Vector&>(DEF_FREF(IPhysCloth::Attach), pos)
	);
	return 0;
}

// присоединить ткань заданным вертексом к мировой точке
void ClothProxy::Attach(unsigned int vertexID, const Vector& pos)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, unsigned int, const Vector&>(DEF_FREF(IPhysCloth::Attach), vertexID, pos)
	);
}

// присоединить ткань к физ. телу
void ClothProxy::Attach(IPhysRigidBody& body, bool twoWayInteraction, bool tearable)
{
	Assert(m_dependences);
	body.AddRef();
	m_dependences->Add(&body);

	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, IPhysRigidBody&, bool, bool>(DEF_FREF(IPhysCloth::Attach), body, twoWayInteraction, tearable)
	);
}

// присоединить ткань к заданным элементам физ. тела
void ClothProxy::Attach(IPhysRigidBody& body, const array<unsigned int>& shapes, bool twoWayInteraction, bool tearable)
{
	Assert(m_dependences);
	body.AddRef();
	m_dependences->Add(&body);

	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, IPhysRigidBody&, const array<unsigned int>&, bool, bool>(DEF_FREF(IPhysCloth::Attach), body, shapes, twoWayInteraction, tearable)
	);
}

// присоединить ткань к заданным элементам физ. тела
void ClothProxy::Attach(IPhysRigidBody& body, int shapeIndex, const Vector & vPos, int vertexID, bool twoWayInteraction, bool tearable)
{
	Assert(m_dependences);
	body.AddRef();
	m_dependences->Add(&body);

	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, IPhysRigidBody&, int, const Vector&, int, bool, bool>(DEF_FREF(IPhysCloth::Attach), body, shapeIndex, vPos, vertexID, twoWayInteraction, tearable)
	);
}

// отсоединить все закрепленные точки ткани
void ClothProxy::Detach()
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void>(DEF_FREF(IPhysCloth::Detach))
	);
}

// отсоединить закрепленную точку ткани
void ClothProxy::Detach(unsigned int vertexID)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void>(DEF_FREF(IPhysCloth::Detach), vertexID)
	);
}

// отсоединить все точки закрепленные на физическом теле
void ClothProxy::Detach(IPhysRigidBody& body)
{
	Assert(m_dependences);
	body.AddRef();
	m_dependences->Add(&body);

	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, IPhysRigidBody&>(DEF_FREF(IPhysCloth::Detach), body)
	);
}

// отсоединить все точки закрепленные на элементах физ. тела
void ClothProxy::Detach(IPhysRigidBody& body, const array<unsigned int>& shapes)
{
	Assert(m_dependences);
	body.AddRef();
	m_dependences->Add(&body);

	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, IPhysRigidBody&, const array<unsigned int>&>(DEF_FREF(IPhysCloth::Detach), body, shapes)
	);
}

// получить ABB ткани
Box ClothProxy::GetABB() const
{
	if (!m_realCloth)
		return Box();

	return m_realCloth->GetABB();
}

// получить кол-во вершин в ткани
unsigned int ClothProxy::GetVertexCount() const
{
	if (!m_realCloth)
		return 0;

	return m_realCloth->GetVertexCount();
}

// приложить силу/импульс в точку ткани
void ClothProxy::AddForceAtPos(const Vector & pos, float magnitude, float radius, PhysForceMode forceMode)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, const Vector&, float, float, PhysForceMode>(DEF_FREF(IPhysCloth::AddForceAtPos), pos, magnitude, radius, forceMode)
	);
}

// разорвать ткань в заданной точке
void ClothProxy::TearVertex(unsigned int vertexID, const Vector & normal)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, unsigned int, const Vector&>(DEF_FREF(IPhysCloth::TearVertex), vertexID, normal)
	);
}

// разорвать ткань в заданной точке
void ClothProxy::Tear(unsigned int vertexID, bool tearWholeTriangle)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, unsigned int, bool>(DEF_FREF(IPhysCloth::Tear), vertexID, tearWholeTriangle)
	);
}

// проверить пересечение ткани и луча
bool ClothProxy::Raycast(const Vector& from, const Vector& to, Vector * hitPos, unsigned int * vertexID) const
{
	if (!m_realCloth)
		return false;

	return m_realCloth->Raycast(from, to, hitPos, vertexID);
}

// установить коэффициент разрыва ткани
void ClothProxy::SetTearCoef(float coef)
{
	m_tearcoef = coef;
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, float>(DEF_FREF(IPhysCloth::SetTearCoef), coef)
	);
}

// получить коэффициент разрыва ткани
float ClothProxy::GetTearCoef() const
{
	if (!m_realCloth)
		return m_tearcoef;

	return m_realCloth->GetTearCoef();
}

// установить коэффициент разрыва в точке крепления ткани
void ClothProxy::SetAttachmentTearCoef(float coef)
{
	m_attachmenttearcoef = coef;
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, float>(DEF_FREF(IPhysCloth::SetAttachmentTearCoef), coef)
	);
}

// получить коэффициент разрыва в точке крепления ткани
float ClothProxy::GetAttachmentTearCoef() const
{
	if (!m_realCloth)
		return m_attachmenttearcoef;

	return m_realCloth->GetAttachmentTearCoef();
}

// установить ускорение
void ClothProxy::SetAcceleration(const Vector& accel )
{
	m_acceleration = accel;
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, const Vector&>(DEF_FREF(IPhysCloth::SetAcceleration), accel)
	);
}

// получить ускорение
Vector ClothProxy::GetAcceleration() const
{
	if (!m_realCloth)
		return m_acceleration;
	
	return m_realCloth->GetAcceleration();
}

// установить кол-во итераций солвера
void ClothProxy::SetSolverIterations(unsigned int count)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, unsigned int>(DEF_FREF(IPhysCloth::SetSolverIterations), count)
	);
}

// сохранить внутреннее состояние и прекратить симуляцию
void ClothProxy::Sleep()
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void>(DEF_FREF(IPhysCloth::Sleep))
	);
}

// восстановить внутреннее состояние и продолжить симуляцию
void ClothProxy::WakeUp(const Matrix& mtx)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, const Matrix&>(DEF_FREF(IPhysCloth::WakeUp), mtx)
	);
}

// применить трансформацию к каждой вершине ткани
void ClothProxy::Transform(const Matrix& mtx)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, const Matrix&>(DEF_FREF(IPhysCloth::Transform), mtx)
	);
}

// задать бокс где будет жить ткань, при выходе за этот бокс вершины ткани удаляются
void ClothProxy::SetValidBox(const Vector& min, const Vector& max)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, const Vector&, const Vector&>(DEF_FREF(IPhysCloth::SetValidBox), min, max)
	);
}

// включить/выключить использование ValidBox'а
void ClothProxy::EnableValidBox(bool enable)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void, bool>(DEF_FREF(IPhysCloth::EnableValidBox), enable)
	);
}

// получить внутренности - !только для использования внутри сервиса
void ClothProxy::GetInternals(PhysInternal & internals) const
{
	if (!m_realCloth)
		throw "Invalid request";

	m_realCloth->GetInternals(internals);
}


//////////////////////////////////////////////////////////////////////////
// IPhysBase
//////////////////////////////////////////////////////////////////////////
bool ClothProxy::Release()
{
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true;
	//return false;
}

void ClothProxy::RealRecreate()
{
	RELEASE(m_realCloth);

	OnSyncCreate();
}

void ClothProxy::Recreate()
{
	dword idx = m_calls.Add
	(
		MakeDeferrer<IPhysCloth, void>(DEF_FREF(IPhysCloth::RealRecreate))
	);

	m_recreateIndexes.Add(idx);
}

//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void ClothProxy::OnSyncCreate()
{
	if (m_realCloth)
		return;
	
	m_realCloth = NEW PhysCloth(GetFileName(), GetFileLine(), m_scene, m_renderInfo, m_clothMeshBuilder, m_clothSimData, m_density);

	Assert(m_realCloth);

	SetRealPhysObject(m_realCloth);

	/*PhysInternal internals;
	m_realCloth->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;*/
}

void ClothProxy::OnSyncCalls()
{
	Assert(m_realCloth);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		if (m_recreateIndexes.IsExist(i))
			m_calls[i]->Call(this);
		else
			m_calls[i]->Call(m_realCloth);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();

	IProxy::OnSyncCalls();
}

bool ClothProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	/*if (m_bReleaseCall)
		return IPhysBase::Release();
	return false;*/
}

void ClothProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
