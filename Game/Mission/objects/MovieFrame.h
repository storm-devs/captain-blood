//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// Mission objects
//============================================================================================
// MovieFrame
//============================================================================================

#ifndef _MovieFrame_h_
#define _MovieFrame_h_

#include "..\..\..\Common_h\mission\Mission.h"

class MovieFrame : public MissionObject
{

	enum Action
	{
		act_activate,		//Выезжает рамка
		act_show,			//Рисуеться рамка
		act_deactivate,		//Уезжает рамка
		act_hide,			//Скрыта рамка
	};

	struct QuadVertex
	{
		Vector4 pos;
		float u, v;
	};


//--------------------------------------------------------------------------------------------
public:
	MovieFrame();
	virtual ~MovieFrame();


//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Пересоздать объект
	virtual void Restart();
	//Показать/скрыть объект
	virtual void Show(bool isShow);
	//Активировать
	virtual void Activate(bool isActive);


//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Работа
	void _cdecl Work(float dltTime, long level);
	//Рисование
	void _cdecl Draw(float dltTime, long level);

private:
	void SetUpdate();
	void DelUpdate();

//--------------------------------------------------------------------------------------------
private:


	ShaderId PostProcessFillScreen_Color_id;

	float time;				//Текущее время	
	float speed;			//Скорость изменения	
	float height;			//Высота поля	
	Action action;			//Текущие действие	
	Color color;			//Цвет полей
	IVariable * varColor;
	ICoreStorageLong * largeshot;
};

#endif

