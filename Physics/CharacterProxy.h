#pragma once
#include "common.h"
#include "Deferrer.h"
#include "IProxy.h"

class PhysicsScene;
class CharacterProxy : public IPhysCharacter, public IProxy
{
	DECLARE_MCALLS(IPhysCharacter)
	//array<IDeferrer<IPhysCharacter>*>	m_calls;
	IPhysCharacter*						m_realChar;
	//PhysicsScene&						m_scene;

	Vector	m_position;
	float	m_radius;
	float	m_height;
	bool	m_changedPosition;
public:
	CharacterProxy(float radius, float height, PhysicsScene& scene);
	virtual ~CharacterProxy(void);

	//////////////////////////////////////////////////////////////////////////
	// IPhysCharacter
	//////////////////////////////////////////////////////////////////////////
	//Установить радиус персонажу
	virtual void SetRadius(float v);
	//Получить радиус персонажа
	virtual float GetRadius();
	//Установить высоту персонажу
	virtual void SetHeight(float v);
	//Получить высоту персонажа
	virtual float GetHeight();
	//Установить позицию
	virtual void SetPosition(const Vector & pos);
	//Получить позицию
	virtual Vector GetPosition();
	//Передвинуть персонажа
	virtual dword Move(const Vector & move, dword collisionGroups = 0xffffffff);
	//Активировать/деактивировать персонажа
	virtual void Activate(bool isActive);
	//Если не 0.0f, то на все объекты при столкновении будет приложена сила v
	virtual void ApplyForceToObjects(float v);

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
