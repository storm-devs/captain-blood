
#ifndef _Flares_h_
#define _Flares_h_

#include "..\..\..\Common_h/Mission.h"

class FlaresManager;

class Flares : public MissionObject
{		
	FlaresManager* manager;

public:
		
	struct Oscilator
	{
		float step;		//Шаг во времени
		float amp;		//Амплитуда размаха
		float kamp;		//Обратная величина амплитуды
		float newv;		//Новое значение
		float oldv;		//Старое значение
		float k;		//Текущий коэфициент
	};
	
	struct Flare
	{
		ConstString connectedObjectName;
		MOSafePointer connectedObject;

		const char * locatorName;
		GMXHANDLE connectedLocator;
		IGMXScene * scene;

		bool alreadyTryToFind;


		Vector pos;
		Vector vFlarePos;
		
		//Мерцание
		Oscilator osc[2];
		float flicker;
		bool isFlicker;
				
		float intenc;		
		float intencVis;		
		float range;		
		
		//Цвет короны
		Color color;

		float dist;
		float u,v;
	};


	array<Flare> flares;

	Flares();	
	~Flares();

	MO_IS_FUNCTION(Flares, MissionObject);
	
	void SetManager();	

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
				
	
	Oscilator osc[2];
	float flicker;
	bool isFlicker;
				
	float intenc;		
	float intencVis;		
	float range;		
	float size;	
	bool  autosize;
	float size_angle;
		
	float zOffset;

	bool  noSwing;

	bool  showDebug;

	float fade_speed;

	//Цвет короны
	Color color;

};

#endif

