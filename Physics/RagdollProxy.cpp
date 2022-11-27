#include "RagdollProxy.h"
#include "PhysicsScene.h"
#include "PhysRagdoll.h"

RagdollProxy::RagdollProxy(const void * data, dword dataSize, PhysicsScene& scene) :
IPhysEditableRagdoll(&scene),
IProxy(scene),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_realRagdoll(NULL),
m_data(data),
m_dataSize(dataSize),
m_animations(_FL_, 4)
{
}

RagdollProxy::RagdollProxy(PhysicsScene& scene) :
IPhysEditableRagdoll(&scene),
IProxy(scene),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_realRagdoll(NULL),
m_data(NULL),
m_dataSize(0),
m_animations(_FL_, 4)
{
}

RagdollProxy::~RagdollProxy(void)
{
	if (m_realRagdoll)
		m_realRagdoll->Release(), m_realRagdoll = NULL;

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();

	for (unsigned int i = 0; i < m_animations.Size(); ++i)
		if (m_animations[i]) m_animations[i]->Release();

	m_animations.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysRagdoll
//////////////////////////////////////////////////////////////////////////
//Установить блендер для анимацию
void RagdollProxy::SetBlendStage(IAnimation * ani, dword level)
{
	IAnimation * ani_clone = (ani) ? (IAnimation *)ani->Clone() : null;
	m_animations.Add(ani_clone);

	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, IAnimation*, dword>(DEF_FREF(IPhysEditableRagdoll::SetBlendStage), ani_clone, level)
	);
}

//Удалить блендер для анимации
void RagdollProxy::RemoveBlendStage(IAnimation * ani)
{
	IAnimation * ani_clone = (ani) ? (IAnimation *)ani->Clone() : null;
	m_animations.Add(ani_clone);

	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, IAnimation*>(DEF_FREF(IPhysEditableRagdoll::RemoveBlendStage), ani_clone)
	);
}

//Включить блэндер
void RagdollProxy::Activate(float blendTime, const Matrix & transform)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, float, const Matrix&>(DEF_FREF(IPhysEditableRagdoll::Activate), blendTime, transform)
	);
}

//Выключить блэндер
void RagdollProxy::Deactivate(float blendTime, Matrix & transform)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, float, Matrix&>(DEF_FREF(IPhysEditableRagdoll::Deactivate), blendTime, transform)
	);
}

//Установить линейную скорость
void RagdollProxy::SetVelocity(const Vector & vel, bool isRecursive)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, const Vector&, bool>(DEF_FREF(IPhysEditableRagdoll::SetVelocity), vel, isRecursive)
	);
}

//Установить угловую скорость
void RagdollProxy::SetRotation(float ay)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, float>(DEF_FREF(IPhysEditableRagdoll::SetRotation), ay)
	);
}

//Применить силу к рутовой кости
void RagdollProxy::ApplyForce(const Vector & force)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, const Vector&>(DEF_FREF(IPhysEditableRagdoll::ApplyForce), force)
	);
}

//Применить импульс к рутовой кости
void RagdollProxy::ApplyImpulse(const Vector & imp)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, const Vector&>(DEF_FREF(IPhysEditableRagdoll::ApplyImpulse), imp)
	);
}

//Приложить вращающий момент к рутовой кости в глобальном системе
void RagdollProxy::ApplyTorque(const Vector & imp)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, const Vector&>(DEF_FREF(IPhysEditableRagdoll::ApplyTorque), imp)
	);
}

//Приложить вращающий момент к рутовой кости в локальной системе
void RagdollProxy::ApplyLocalTorque(const Vector & imp)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, const Vector&>(DEF_FREF(IPhysEditableRagdoll::ApplyLocalTorque), imp)
	);
}

//Убивает физику, оставляя последние параметры костей
void RagdollProxy::Freeze()
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void>(DEF_FREF(IPhysEditableRagdoll::Freeze))
	);
}

//Текущие состояние регдолла - заморожен ли
bool RagdollProxy::IsFreezed()
{
	if (m_realRagdoll)
		return m_realRagdoll->IsFreezed();

	return false;
}

//Изменить групппу
void RagdollProxy::SetGroup(PhysicsCollisionGroup group)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void, PhysicsCollisionGroup>(DEF_FREF(IPhysEditableRagdoll::SetGroup), group)
	);
}

//////////////////////////////////////////////////////////////////////////
// IPhysEditableRagdoll
//////////////////////////////////////////////////////////////////////////
//Очистить для создания нового
void RagdollProxy::Clear()
{
	m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, void>(DEF_FREF(IPhysEditableRagdoll::Clear))
	);
}

//Получить рутовую кость
IPhysEditableRagdoll::IBone & RagdollProxy::GetRootBone()
{
	if (m_realRagdoll)
		return m_realRagdoll->GetRootBone();

	Assert("i don't know what to do here");
	return *(IPhysEditableRagdoll::IBone*)NULL;
}

//Получить кость по идентификатору рэйкаста
IPhysEditableRagdoll::IBone * RagdollProxy::GetBone(PhysRaycastId id)
{
	if (m_realRagdoll)
		return m_realRagdoll->GetBone(id);

	Assert("i don't know what to do here");
	return NULL;
}

//Сохранить параметры рэгдола в массив
bool RagdollProxy::BuildSaveData(array<byte> & buffer)
{
	if (m_realRagdoll)
	{
		m_realRagdoll->BuildSaveData(buffer);

	}

	/*m_calls.Add
	(
		MakeDeferrer<IPhysEditableRagdoll, bool, array<byte>&>(DEF_FREF(IPhysEditableRagdoll::BuildSaveData), buffer)
	);*/

	return true;
}


bool RagdollProxy::Release() 
{ 
	// релизим все анимационные блендеры
	if (m_realRagdoll)
		m_realRagdoll->RemoveBlendStages();
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true; 
	//return false; 
}

//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void RagdollProxy::OnSyncCreate()
{
	if (m_realRagdoll)
		return;

	if (m_dataSize == 0)
		m_realRagdoll = (PhysRagdoll*)m_scene.CreateRealEditableRagdoll();
	else
	{
		if (!m_data || !m_dataSize) 
		{
			api->Trace("Physics: Can't create ragdoll, data : %x, size: %d", m_data, m_dataSize);
			api->Trace("file: %s, line: %d", GetFileName(), GetFileLine());
			Assert(false);
		}
	
		m_realRagdoll = NEW PhysRagdoll(this, GetFileName(), GetFileLine(), m_data, m_dataSize, &m_scene);

		if (!m_realRagdoll || !m_realRagdoll->IsLoaded())
		{
			api->Trace("Physics: Can't load ragdoll");
			api->Trace("file: %s, line: %d", GetFileName(), GetFileLine());
			delete m_realRagdoll;
			Assert(false);
		}
	}

	Assert(m_realRagdoll);

	SetRealPhysObject(m_realRagdoll);

	/*PhysInternal internals;
	m_realRagdoll->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;*/
}

void RagdollProxy::OnSyncCalls()
{
	Assert(m_realRagdoll);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realRagdoll);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();

	for (unsigned int i = 0; i < m_animations.Size(); ++i)
		if (m_animations[i]) m_animations[i]->Release();

	m_animations.Empty();
}

bool RagdollProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void RagdollProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
