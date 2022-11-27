#ifndef COMBATPOINTS_H
#define COMBATPOINTS_H

#include "..\..\..\Common_h\Mission.h"

#define COMBAT_POINTS_GROUP GroupId('C','B','T','P')

//#include "..\..\..\Character\Character\Character.h"

class CombatPoints : public MissionObject
{
public:

	struct CombatPoint
	{
		Vector position;
		int NumAllyAttackers;
		int NumEnemyAttackers;
		bool bIsStatistPoint;

		int MaxAttackers;
		float radius;

		int itmp;
		int itmpGr;

		array<MissionObject*> Chars;

		CombatPoint():Chars(_FL_)
		{
		}
	};

	array<CombatPoint> Points;
	bool bShow;

	bool bDebugShow;


	
	//Конструктор - деструктор
	CombatPoints();
	virtual ~CombatPoints();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);


	void ShowDebug(bool show);

	MO_IS_FUNCTION(CombatPoints, MissionObject);

	void _cdecl Realize(float fDeltaTime, long level);

	void Command(const char * id, dword numParams, const char ** params);

	static const char * comment;

};

#endif