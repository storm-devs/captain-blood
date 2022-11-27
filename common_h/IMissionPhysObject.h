#ifndef _IMissionPhysObject_h_
#define _IMissionPhysObject_h_

#include "Mission.h"

class IMissionPhysObject : public DamageReceiver
{
public:
	enum DamageSourceEx
	{
		ds_pickup = DamageReceiver::ds_unknown + 1,
	};

public:
	//Переместить целую часть в новую позицию
	virtual bool SetPartMatrix(long partIndex, const Matrix & mtx) = null;
	//Получить текущую матрицу целой части объекта
	virtual bool GetPartMatrix(long partIndex, Matrix & mtx) = null;
	//Получить пивот целой части
	virtual bool GetPartPivot(long partIndex, Matrix & mtx) = null;
	//Применить импульс в мировых координатах
	virtual void Impulse(const Vector & dir) = null;
	//Сломать объект
	virtual void BrokeObject() = null;
	//Показать-скрыть подсказки
	virtual void ShowTips(bool isShow) = null;
	//Найти индекс локатора по имени, -1 если нет такого
	virtual long GetLocatorIndexByName(const ConstString & name) = null;
	//Получить позицию локатора по индексу
	virtual bool GetLocator(long index, Matrix & mtx) = null;

	//Проверить на пересечение отрезка и ящиков описывающих объекты
	virtual bool OverlapLine(const Vector & v1, const Vector & v2, float skin) = null;

	MO_IS_FUNCTION(IMissionPhysObject, DamageReceiver)

};

#endif

