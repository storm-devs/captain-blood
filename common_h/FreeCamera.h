#ifndef _IFreeCamera_h_
#define _IFreeCamera_h_

#include "core.h"
#include "math3d.h"

class IFreeCamera : public RegObject
{
public:
	virtual ~IFreeCamera(){};

	//Установить новую позицию камеры
	virtual void SetPosition(const Vector & pos) = null;
	//Получить позицию камеры
	virtual Vector GetPosition() = null;
	//Установить углы
	virtual void SetAngles(const Vector & ang) = null;
	//Получить углы
	virtual Vector GetAngles() = null;
	//Соориентировать камеру на точку
	virtual void SetTarget(const Vector & pos) = null;
	//Установить угол обзора
	virtual void SetFOV(float fov) = null;
	//Получить угол обзора
	virtual float GetFOV() = null;
	//Приостановить работу камеры
	virtual void Pause(bool isPause) = null;
};


#endif
