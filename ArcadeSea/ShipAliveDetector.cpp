
#include "ShipAliveDetector.h"
#include "Ships\ship.h"

//============================================================================================

ShipAliveDetector::ShipAliveDetector() :
	m_events(_FL_)
{
	m_time = -1.0f;
	m_delayTime = 0.0f;
	m_bWaitTrigger = false;
}

ShipAliveDetector::~ShipAliveDetector()
{
	m_events.DelAll();
}


//============================================================================================


//Инициализировать объект
bool ShipAliveDetector::Create(MOPReader & reader)
{
	m_time = -1.0f;
	m_delayTime = 0.0f;

	// тут нечего показывать - это логический объект
	Show(false);

	// Выбираем объект для слежения
	ConstString pcWatchedObjID = reader.String();
	FindObject( pcWatchedObjID, m_Object );
	if( !m_Object.Ptr() )
	{
		Mission().LogicDebugError("ShipAliveDetector \"%s\" not created -> AIObject not found", GetObjectID().c_str());
		return false;
	}
	MO_IS_IF_NOT(tid, "Ship", m_Object.Ptr())
	{
		Mission().LogicDebugError("ShipAliveDetector \"%s\" not created -> AIObject \"%s\" is not \"Ship\"", GetObjectID().c_str(), pcWatchedObjID.c_str());
		return false;
	}

	// Задержка на включение тригера
	m_delayTime = reader.Float();

	// Инициализируем масссив событий
	m_events.DelAll();
	long nSize = reader.Array();
	m_events.AddElements( nSize );
	for( long n=0; n<nSize; n++ )
	{
		m_events[n].Init(reader);
	}

	// Сопоставляем тригеры по типам смерти с событиями из массива
	if( !ReadDieEventIndexWithCheck(reader, "default", m_nDefaultDie) )
		return false;
	if( !ReadDieEventIndexWithCheck(reader, "flamethrower", m_nFireDie) )
		return false;
	if( !ReadDieEventIndexWithCheck(reader, "cannon", m_nCannonDie) )
		return false;
	if( !ReadDieEventIndexWithCheck(reader, "mine", m_nMineDie) )
		return false;
	if( !ReadDieEventIndexWithCheck(reader, "ram", m_nRamDie) )
		return false;

	m_bWaitTrigger = true;

	Activate(reader.Bool());
	return true;
}

bool ShipAliveDetector::ReadDieEventIndexWithCheck( MOPReader & reader, const char* pcDieTypeName, long& nVal )
{
	nVal = reader.Long();
	if( nVal < 0 || nVal >= (long)m_events.Size() )
	{
		Mission().LogicDebugError("ShipAliveDetector \"%s\" not created -> Wrong index for %s die", GetObjectID().c_str(), pcDieTypeName);
		return false;
	}
	return true;
}

//Активировать
void ShipAliveDetector::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if(!EditMode_IsOn())
	{
		if(IsActive() )
		{
			if( m_bWaitTrigger )
			{
				LogicDebug("Activate");
				SetUpdate(&ShipAliveDetector::Work, ML_TRIGGERS);
			}
			else
			{
				LogicDebug("Activate but object already is dead and triggered");
				DelUpdate(&ShipAliveDetector::Work);
			}
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&ShipAliveDetector::Work);
		}
	}else{
		DelUpdate(&ShipAliveDetector::Work);
	}
}

//============================================================================================

//Работа детектора
void _cdecl ShipAliveDetector::Work(float dltTime, long level)
{
	if( !m_bWaitTrigger )
		return;

	//Проверяем правильность объекта
	if( !m_Object.Ptr() ) return;
	if( !m_Object.Validate() )
		return;

	//Если жив
	bool bLive = !(m_Object.Ptr()->IsDie() | m_Object.Ptr()->IsDead());

	// Только умерли - ставим задержку на тригер смерти
	if(!bLive && m_time < -0.5f)
	{
		m_time = m_delayTime;
		LogicDebug("Start timer in time: %f", m_time);
	}

	// отсчет задержки для тригера смерти
	if(m_time > -0.5f)
	{
		m_time -= dltTime;
		if(m_time <= 0.0f)
		{
			LogicDebug("Triggering");
			m_time = -1.0f;
			m_bWaitTrigger = false;
			Activate(false);

			// выбираем евент в зависимости от того как умер корабль
			MissionTrigger* pEv = 0;
			Ship::DieType dt = ((Ship*)m_Object.Ptr())->GetDieType();
			switch( dt )
			{
			case Ship::DieType_flamethrower: pEv = &m_events[m_nFireDie]; break;
			case Ship::DieType_cannon: pEv = &m_events[m_nCannonDie]; break;
			case Ship::DieType_mine: pEv = &m_events[m_nMineDie]; break;
			case Ship::DieType_ram: pEv = &m_events[m_nRamDie]; break;
			default: pEv = &m_events[m_nDefaultDie];
			}

			if( pEv ) pEv->Activate(Mission(), false);
		}
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(ShipAliveDetector, "Ship alive detector", '1.00', 0x0fffffff, "Alive detector look for ship object,\nand triggering if object is dead.", "Logic")
	MOP_STRING("Object id", "Player")
	MOP_FLOATEXC("Delay time", 0.0f, 0.0f, 100000.0f, "Delay time in seconds before activate trigger")
	MOP_ARRAYBEG("Die events", 1, 10)
		MOP_MISSIONTRIGGER("")
	MOP_ARRAYEND
	MOP_LONGC("Die type default", 0, "Index into events array for default die");
	MOP_LONGC("Die type flame", 0, "Index into events array for die from falmethrower fire");
	MOP_LONGC("Die type cannon", 0, "Index into events array for die from cannon fire");
	MOP_LONGC("Die type mine", 0, "Index into events array for die from mine explosion");
	MOP_LONGC("Die type ram", 0, "Index into events array for die from ram collision");
	MOP_BOOL("Active", true)
MOP_ENDLIST(ShipAliveDetector)





