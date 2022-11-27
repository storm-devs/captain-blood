#ifndef SAFTYPOINTS_H
#define SAFTYPOINTS_H

#include "..\..\..\Common_h\Mission.h"

#define SAFTY_POINTS_GROUP GroupId('S','A','F','P')

//#include "..\..\..\Character\Character\Character.h"

class SaftyPoints : public MissionObject
{
public:

	struct SaftyPoint
	{
		Vector position;		
		float radius;
	};

	array<SaftyPoint> Points;
	bool bShow;
	
	//Конструктор - деструктор
	SaftyPoints();
	virtual ~SaftyPoints();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	MO_IS_FUNCTION(SaftyPoints, MissionObject);

	void _cdecl Realize(float fDeltaTime, long level);

	static const char * comment;

};

#endif