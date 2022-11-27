//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "..\..\common_h\mission\mission.h"
#include "FlameThrower.h"

class MissionFlameThrower : public MissionObject
{
	FlameThrower ft_;

	void CommonUpdate(MOPReader & reader);

	void _cdecl DrawEditorFeatures(float fDeltaTime, long level);
public:
	MissionFlameThrower(void);
	virtual ~MissionFlameThrower(void);

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

	virtual void Command(const char * id, dword, const char **);
};
