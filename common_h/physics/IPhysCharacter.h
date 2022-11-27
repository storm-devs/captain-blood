



class IPhysCharacter : public IPhysBase
{
public:
	enum CollisionFlags
	{
		cf_sides = (1 << 0),
		cf_up = (1 << 1),
		cf_down = (1 << 2),
	};

public:
	IPhysCharacter(IPhysicsScene * _scene) : IPhysBase(_scene){};
	//Установить радиус персонажу
	virtual void SetRadius(float v) = null;
	//Получить радиус персонажа
	virtual float GetRadius() = null;
	//Установить высоту персонажу
	virtual void SetHeight(float v) = null;
	//Получить высоту персонажа
	virtual float GetHeight() = null;
	//Установить позицию
	virtual void SetPosition(const Vector & pos) = null;
	//Получить позицию
	virtual Vector GetPosition() = null;
	//Передвинуть персонажа
	virtual dword Move(const Vector & move, dword collisionGroups = 0xffffffff) = null;
	//Активировать/деактивировать персонажа
	virtual void Activate(bool isActive) = null;
	//Если не 0.0f, то на все объекты при столкновении будет приложена сила v
	virtual void ApplyForceToObjects(float v) = null;
};



















