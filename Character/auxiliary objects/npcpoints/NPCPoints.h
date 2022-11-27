#ifndef FEARPOINTS_H
#define FEARPOINTS_H

#include "..\..\..\Common_h\Mission.h"

#define NPC_POINTS_GROUP GroupId('N','P','C','P')

class NPCPoints : public MissionObject
{
public:

	struct NPCPoint
	{
		Vector position;
		int    num_chars;
		MissionObject* chr1;
		MissionObject* chr2;

		NPCPoint()
		{
			num_chars = 0;
			chr1 = chr2 = null;
		}
	};

	array<NPCPoint> Points;
	bool bShow;

	bool bDebugShow;


	
	//Конструктор - деструктор
	NPCPoints();
	virtual ~NPCPoints();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);


	void ShowDebug(bool show);

	MO_IS_FUNCTION(FearPoints, MissionObject);

	int  GivePoint(int num_chars);

	MissionObject* GetFrined(int index, MissionObject* initiator);

	void TakePoint(int index, bool take, MissionObject* invader);

	void _cdecl Realize(float fDeltaTime, long level);	

	bool IsPointReached(int index, Vector pos);

	static const char * comment;

};

#endif