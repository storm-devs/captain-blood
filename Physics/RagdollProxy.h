#pragma once
#include "common.h"
#include "Deferrer.h"
#include "IProxy.h"

class PhysicsScene;
class PhysRagdoll;
class RagdollProxy : public IPhysEditableRagdoll, public IProxy
{
	DECLARE_MCALLS(IPhysEditableRagdoll)
	//array<IDeferrer<IPhysEditableRagdoll>*>	m_calls;
	//PhysicsScene&							m_scene;
	array<IAnimation*>						m_animations;
	PhysRagdoll*							m_realRagdoll;

	const void*		m_data;
	unsigned int	m_dataSize;
public:
	RagdollProxy(const void * data, dword dataSize, PhysicsScene& scene);
	RagdollProxy(PhysicsScene& scene);
	virtual ~RagdollProxy(void);

	//////////////////////////////////////////////////////////////////////////
	// IPhysRagdoll
	//////////////////////////////////////////////////////////////////////////
	//Установить блендер для анимацию
	virtual void SetBlendStage(IAnimation * ani, dword level);
	//Удалить блендер для анимации
	virtual void RemoveBlendStage(IAnimation * ani);
	//Включить блэндер
	virtual void Activate(float blendTime, const Matrix & transform);
	//Выключить блэндер
	virtual void Deactivate(float blendTime, Matrix & transform);
	//Установить линейную скорость
	virtual void SetVelocity(const Vector & vel, bool isRecursive = true);
	//Установить угловую скорость
	virtual void SetRotation(float ay);
	//Применить силу к рутовой кости
	virtual void ApplyForce(const Vector & force);
	//Применить импульс к рутовой кости
	virtual void ApplyImpulse(const Vector & imp);
	//Приложить вращающий момент к рутовой кости в глобальном системе
	virtual void ApplyTorque(const Vector & imp);
	//Приложить вращающий момент к рутовой кости в локальной системе
	virtual void ApplyLocalTorque(const Vector & imp);

	//Изменить групппу
	virtual void SetGroup(PhysicsCollisionGroup group);

	//Убивает физику, оставляя последние параметры костей
	virtual void Freeze();
	//Текущие состояние регдолла - заморожен ли
	virtual bool IsFreezed();

	//////////////////////////////////////////////////////////////////////////
	// IPhysEditableRagdoll
	//////////////////////////////////////////////////////////////////////////
	//Очистить для создания нового
	virtual void Clear();
	//Получить рутовую кость
	virtual IBone & GetRootBone();
	//Получить кость по идентификатору рэйкаста
	virtual IBone * GetBone(PhysRaycastId id);

	//Сохранить параметры рэгдола в массив
	virtual bool BuildSaveData(array<byte> & buffer);

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
