

#include "..\Common_h\Physics.h"
#include <PxCharacter.h>
#include <PxCapsuleController.h>

using namespace physx;

class PhysCharacter : public IPhysCharacter
{
	friend class PhysicsScene;
	friend class UserControllerHitReport;

public:
	PhysCharacter(const char * filename, long fileline, float r, float h, IPhysicsScene * _scene);
	virtual ~PhysCharacter();

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
	virtual dword Move(const Vector & move, dword collisionGroups);
	//Активировать/деактивировать персонажа
	virtual void Activate(bool isActive);
	//Если не 0.0f, то на все объекты при столкновении будет приложена сила v
	virtual void ApplyForceToObjects(float v);
		
	//Обновить состояние активности
	void UpdateActiveState();

	//Возвращает коллижн флаги с последнего выполненого Move
	dword GetMoveCollisionFlags();

private:
	PxCapsuleController* controller;
	PxCapsuleControllerDesc desc;
	dword bublegum;
	float force;
	PxU32 move_collisionflags;
	dword sceneChangedIndex;
	bool isApplyForce;
};



















