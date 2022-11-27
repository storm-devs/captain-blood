#ifndef PPL_MISSION_OBJECT
#define PPL_MISSION_OBJECT

#include "..\..\..\Common_h\Mission.h"


class DirectionLight : public MissionObject
{	
	Color cAmbient;



	bool bShowCubeMap;


	struct PPLLight
	{
		Vector vPos;
		Vector vDir;
		Color cColor;
		Color cBackColor;
		float fSize;
		bool bSpecularAttachedToCamera;
	};

	PPLLight Diffuse[1];
	//array<PPLLight> Specular;

	bool bAttachSpecularToCamera;

	bool bSetupVertex;


	//PPLService* pLS;

public:

	//Конструктор - деструктор
	DirectionLight();
	virtual ~DirectionLight();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	//Выполнить событие активации
	void _cdecl ActivateEvent(const char * group, MissionObject * sender);

	MO_IS_FUNCTION(DirectionLight, MissionObject);

	void _cdecl Realize(float fDeltaTime, long level);

	virtual void Activate(bool isActive);

	//Установка источника в основном цикле
	void _cdecl SetLight(float dltTime, long level);


};

#endif