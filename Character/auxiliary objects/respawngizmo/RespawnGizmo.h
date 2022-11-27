

#include "..\..\..\Common_h\mission.h"

class RespawnGizmo : public MissionObject
{
	struct WatchElement
	{
		MOSafePointer mo;		
	};

public:
	struct EnumElement
	{
		ConstString desc;
		GroupId id;
	};


public:
	RespawnGizmo();
	virtual ~RespawnGizmo();

public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	//Инициализировать объект в режиме редактирования
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры в режиме редактирования
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);

	//Пересоздать объект
	virtual void Restart();


private:
	//Работа логики
	void _cdecl Work(float dltTime, long level);
	//Наблюдение за списком персонажей
	void _cdecl Watch(float dltTime, long level);
	//Рисование точек
	void _cdecl Draw(float dltTime, long level);

private:
	struct BestPoint
	{
		int index;
		float distance;
		Matrix mtx;
	
		static bool QSort(const BestPoint & b1, const BestPoint & b2)
		{
			return b1.distance < b2.distance;
		}
	};

	array<Vector> points;
	array<BestPoint> bestPoints;
	array<MissionObject *> list;
	array<WatchElement> watchList;
	Matrix tmpMtx;	
	bool skipIfNoActive;
	bool watchEnable;
	MGIterator * group;
public:
	static EnumElement enumElements[];
	static const char * comment;
};

