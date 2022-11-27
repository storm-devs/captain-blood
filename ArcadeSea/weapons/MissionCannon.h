//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "..\..\common_h\mission\mission.h"
#include "cannon.h"

class MissionCannon : public MissionObject
{
	Cannon	cannon_;

	void ReadMOPs(MOPReader & reader);

	void _cdecl DrawEditorFeatures(float fDeltaTime, long level);
public:
	MissionCannon(void);
	virtual ~MissionCannon(void);

	void _cdecl Draw(float fDeltaTime, long level);

	// Создание объекта
	virtual bool Create(MOPReader & reader);
	// Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	// Инициализация 
	virtual bool Init();

	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

	virtual Matrix & GetMatrix(Matrix & mtx);

	MO_IS_FUNCTION(Weapon, MissionObject);

};
