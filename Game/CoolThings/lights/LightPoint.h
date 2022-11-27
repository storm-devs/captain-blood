//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// LightPoint	
//============================================================================================

#ifndef _LightPoint_h_
#define _LightPoint_h_

#include "Light.h"

class LightPoint : public Light  
{
	struct Oscilator
	{
		float step;		//Шаг во времени
		float amp;		//Амплитуда размаха
		float kamp;		//Обратная величина амплитуды
		float newv;		//Новое значение
		float oldv;		//Старое значение
		float k;		//Текущий коэфициент
	};

	bool isFlicker;

	//Время разгорания или погасания
	float flameUpDownTime;


	//Время разгорания или погасания
	float lenUpDownTimeSquared;


//--------------------------------------------------------------------------------------------
public:
	LightPoint();
	virtual ~LightPoint();

	MO_IS_FUNCTION(LightPoint, Light);


	void SetPosition (const Vector& vec);
	
	//Обработчик команд для объекта
	void Command(const char * id, dword numParams, const char ** params);	

//--------------------------------------------------------------------------------------------
protected:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Перевести объект в спящий режим
	virtual void EditMode_Sleep(bool isSleep);

	//Выполнить событие активации
	void _cdecl ActivateEvent(const char * group, MissionObject * sender);
	//Выполнить событие деактивации
	void _cdecl DeactivateEvent(const char * group, MissionObject * sender);

//--------------------------------------------------------------------
protected:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);

//--------------------------------------------------------------------------------------------
protected:
	//Мерцание
	void _cdecl Flicker(float dltTime, long level);
	//Корона
	void _cdecl Corona(float dltTime, long level);


	void _cdecl PointLightDraw(float dltTime, long level);


	//void DrawRotateGizmo (float fRadius, DWORD dwCOLOR);
	//bool NeedDrawRotateGizmoLine (const Vector &v1, const Vector &v2, const Vector& vCamDir);

	//Плавное разгорание источника
	void _cdecl FlameUp(float dltTime, long level);
	//Плавное погосание источника
	void _cdecl FlameDown(float dltTime, long level);

	//Обновить указатель на объект к которому прицепленны
	void UpdateConnectPointer();
	//Обновить позицию источника
	void _cdecl UpdateConnection(float dltTime, long level);

	void SetColorLight(Color color);

	//Установка источника в основном цикле
	void _cdecl SetLight(float dltTime, long level);	
	//Установка источника в основном цикле
	void _cdecl RemoveLight(float dltTime, long level);

#ifndef NO_CONSOLE
	virtual void _cdecl RuntimeDebug(float dltTime, long level);

	static void _cdecl Console_ShowRuntimeDebug(const ConsoleStack & params);
#endif
	

//--------------------------------------------------------------------------------------------
protected:
	//Позиция источника
	Vector position;
	Vector basePosition;
	//Радиус источника света
	float radius;
	//Направленность источника
	float directivity;
	//Источник света
	ILight * light;
	//Мерцание
	Oscilator osc[2];
	//Текстура короны
	IBaseTexture * texture;
	//Интенсивность короны в зависимости от источника
	float crnIntenc;
	//Интенсивность короны в зависимости от видимости
	float crnIntencVis;
	//Дистанция видимости короны
	float crnRange;
	//Размер короны
	float crnSize;
	//Мерцание короны
	float crnFlicker;
	//Цвет короны
	Color crnColor;
	//Текущая интенсивность источника
	float intensity;
	//Объект к которому прицепляемся
	MOSafePointer connectObjectPtr;	
	ConstString connectToObject;

	float intensity_multiply;	


	bool bShadowAffected;



};

#endif

