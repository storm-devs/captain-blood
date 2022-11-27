//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// Mission objects
//============================================================================================
// Fader
//============================================================================================

#ifndef _Fader_h_
#define _Fader_h_

#include "..\..\..\Common_h\mission\Mission.h"

class Fader : public MissionObject
{

	enum Action
	{
		act_stop,
		act_start,
		act_fadeout,
		act_fadein,
	};

//--------------------------------------------------------------------------------------------
public:
	Fader();
	virtual ~Fader();


//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Пересоздать объект
	virtual void Restart();
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


//--------------------------------------------------------------------------------------------
private:
	void SetTexture(const char * fileName);
	void SetColor(const Color & c);
	void SetAlpha(float alpha);


	ShaderId PostProcessFillScreen_Texture_id;
	ShaderId PostProcessFillScreen_Color_id;

//--------------------------------------------------------------------------------------------
private:
	//Времена фэйда
	float fadeOutTime;
	float fadeInTime;
	//Текущее время
	float time;
	Action action;
	//Триггер на 
	MissionTrigger start;
	MissionTrigger hide;
	MissionTrigger stop;
	Color color;
	IBaseTexture * texture;
	IVariable * varColor;
	IVariable * varTexture;
};

#endif

