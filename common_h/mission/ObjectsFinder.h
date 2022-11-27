//============================================================================================
// Spirenkov Maxim, 2004, 2008
//============================================================================================
// Mission fast find objects system
//============================================================================================

#ifndef _Mission_ObjectsFinder_h_
#define _Mission_ObjectsFinder_h_


//Объект поиска
class IMissionQTObject
{
protected:
	virtual ~IMissionQTObject(){};

//------------------------------------------------------------------------
//Управление объектов
//------------------------------------------------------------------------
public:
	//Удалить
	virtual void Release() = null;
	//Изменить активность
	virtual void Activate(bool isActive) = null;
	//Активен ли объект
	virtual bool IsActivate() = null;	
	//Получить указатель на объект миссии которому принадлежит запись
	virtual MissionObject & GetMissionObject() = null;

//------------------------------------------------------------------------
//Позииция в пространстве
//------------------------------------------------------------------------
public:
	//Установить матрицу
	virtual void SetMatrix(const Matrix & mtx) = null;
	//Получить матрицу
	virtual const Matrix & GetMatrix() const = null;

//------------------------------------------------------------------------
//Описывающий занимаемый объем ящик или сфера
//------------------------------------------------------------------------
public:
	//Установить локальную позицию описывающего бокса
	virtual void SetBoxCenter(float centerX, float centerY, float centerZ) = null;
	//Установить локальную позицию описывающего бокса
	void SetBoxCenter(const Vector & center);
	//Установить размер описывающего бокса
	virtual void SetBoxSize(float sizeX, float sizeY, float sizeZ) = null;
	//Установить размер описывающего бокса
	void SetBoxSize(const Vector & size);
	//Установить описывающий бокс
	void SetBox(const Vector & min, const Vector & max);
	//Установить описывающий бокс по сфере
	void SetShpere(float centerX, float centerY, float centerZ, float R);
	//Установить описывающий бокс по сфере
	void SetShpere(const Vector & center, float R);
	//Установить описывающий бокс по сфере
	void SetShpere(const Sphere & sphere);
	//Получить размеры коробки OBB
	virtual const Vector & GetBoxSize() = null;
	//Получить центр OBB
	virtual const Vector & GetBoxCenter() = null;
	//Получить квадрад, занимаемый на дереве
	virtual bool GetQTAbb(Vector & minAbb, Vector & maxAbb) = null;

//------------------------------------------------------------------------
//Пользовательские данные
//------------------------------------------------------------------------
public:
	//Установить пользовательские данные
	virtual void SetUserData(void * data) = null;
	//Получить пользовательские данные
	virtual void * GetUserData() = null;


};

//============================================================================================
//ObjectFinder inline's
//============================================================================================


//Установить локальную позицию описывающего бокса
__forceinline void IMissionQTObject::SetBoxCenter(const Vector & center)
{
	SetBoxCenter(center.x, center.y, center.z);
};

//Установить размер описывающего бокса
__forceinline void IMissionQTObject::SetBoxSize(const Vector & size)
{
	SetBoxSize(size.x, size.y, size.z);
};

//Установить описывающий бокс
__forceinline void IMissionQTObject::SetBox(const Vector & min, const Vector & max)
{
	Vector center = (min + max)*0.5f;
	Vector size = max - min;
	SetBoxCenter(center.x, center.y, center.z);
	SetBoxSize(size.x, size.y, size.z);
}

//Установить описывающий бокс по сфере
__forceinline void IMissionQTObject::SetShpere(float centerX, float centerY, float centerZ, float R)
{
	SetBoxCenter(centerX, centerY, centerZ);
	SetBoxSize(R*2.0f, R*2.0f, R*2.0f);
};

//Установить описывающий бокс по сфере
__forceinline void IMissionQTObject::SetShpere(const Vector & center, float R)
{
	SetShpere(center.x, center.y, center.z, R);
};

//Установить описывающий бокс по сфере
__forceinline void IMissionQTObject::SetShpere(const Sphere & sphere)
{
	SetShpere(sphere.pos.x, sphere.pos.y, sphere.pos.z, sphere.r);
};


#endif

