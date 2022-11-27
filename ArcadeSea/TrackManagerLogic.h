#ifndef _TrackManagerLogic_h_
#define _TrackManagerLogic_h_

#include "..\Common_h\Mission\Mission.h"

class TrackManagerLogic : public MissionObject
{
	struct TrackGroup
	{
		array<const char*> AIParams;
		MOSafePointer spShip;

		TrackGroup() : AIParams(_FL_) {spShip.Reset();}
	};

//--------------------------------------------------------------------------------------------
public:
	TrackManagerLogic();
	virtual ~TrackManagerLogic();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);

//--------------------------------------------------------------------------------------------
private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

	void ReadMOPs(MOPReader & reader);

//--------------------------------------------------------------------------------------------
protected:
	array<ConstString> m_ShipsList;
	array<TrackGroup> m_TrackList;
};

#endif

