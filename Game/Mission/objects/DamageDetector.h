//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// DamageDetector	
//============================================================================================

#ifndef _DamageDetector_h_
#define _DamageDetector_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class DamageDetector : public DamageObject
{
	struct Immune
	{
		MissionObject * obj;
		float time;
	};


//--------------------------------------------------------------------------------------------
public:
	DamageDetector();
	virtual ~DamageDetector();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);


	void Command(const char * id, dword numParams, const char ** params);


//--------------------------------------------------------------------------------------------
private:
	//Воздействовать на объект сферой
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius);
	//Воздействовать на объект линией
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to);
	//Воздействовать на объект выпуклым чехырёхугольником
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4]);
	//Логическая проверка на возможность аттаки
	bool IsCanAttack(MissionObject * obj, dword source, float & hp);
	//Отработка попадания
	bool HitProcess(dword source, MissionObject * obj, float hp);
	//Сделать объект имунным
	void SetImmune(MissionObject * obj);
	//Таймер невосприимчевости
	void _cdecl ImmuneTimer(float dltTime, long level);

//--------------------------------------------------------------------------------------------
private:
	//Работа детектора в режиме редактирования
	void _cdecl EditModeWork(float dltTime, long level);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

//--------------------------------------------------------------------------------------------
private:
	Sphere detector;					//Позиция и размер детектора
	DamageMultiplier multipliers;		//Множители на демедж
	Immune immune[4];					//Массив иммунных объектов
	dword immuneCount;					//Количество иммуных объектов
	float immuneTime;					//Время именности до следующей атаки
	float immuneTimerWorkTime;			//Часы работы таймера иммуности
	bool immuneTimerIsActive;			//Активен ли таймер
	bool collide;						//Флаг колизиться или нет	
};

#endif

