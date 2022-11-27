

#include "..\Utils\InterfaceUtils.h"


class TaskPointer : public MissionObject
{
	ShaderId Interface_TaskpointerPicture_id;
	ShaderId Interface_TaskpointerPictureMask_id;

public:
	TaskPointer();
	virtual ~TaskPointer();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	//Показать/скрыть объект
	virtual void Show(bool isShow);
	//Активировать
	virtual void Activate(bool isActive);
#ifndef MIS_STOP_EDIT_FUNCS
	//Выделить объект
	virtual void EditMode_Select(bool isSelect);
#endif
	//Нарисовать модельку
	virtual void _cdecl Draw(float dltTime, long level);	

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

private:
	//Обновить указатель на объект
	bool UpdatePointer(const string &str, MOSafePointer &ptr);

private:
	IBaseTexture * back;
	IBaseTexture * mask;
	IVariable * texVar;
	IGMXScene * scene;
	float w, h;					//Относительные размеры
	float fov;					//Прараметры камеры
	Matrix camera;				//Положение камеры
	Matrix modelRot;			//Ориентация модельки
	Vector modelPos;			//Комбинированная позиция модельки
	string observerId;			//Исполнитель
	MOSafePointer observer;		//Указатель
	string targetId;			//Место назначения
	MOSafePointer target;		//Указатель
	Quaternion current;			//Текущее направление
	float showPosition;			//Позиция для показа
	static Vector4 vertices[4];	//Вершины для рендера картинок
	long drawPriority;			//порядок отрисовки ML_GUI# + drawPriority

public:
	static const char * comment;
};



