

#include "..\Utils\InterfaceUtils.h"


class TaskViewer : public MissionObject
{
	struct TextLine
	{
		string text;
		float width;
		float height;
	};


	ShaderId Interface_TaskviewerPictureMask_id;
	ShaderId Interface_TaskviewerPicture_id;
	

public:
	TaskViewer();
	virtual ~TaskViewer();

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
	//Установить текст
	void SetText(const char * rawText);

private:
	IBaseTexture * back;
	IFont * font;
	IBaseTexture * mask;
	IVariable * texVar;
	array<TextLine> lines;
	float totalTextHeight;
	float showTime;
	float currentTime;
	float w, h;					//Относительные размеры
	float showPosition;			//Позиция для показа
	static Vector4 vertices[4];	//Вершины для рендера картинок
	long drawPriority;			//порядок отрисовки ML_GUI# + drawPriority
public:
	static const char * comment;
};



