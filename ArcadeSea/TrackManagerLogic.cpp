
#include "TrackManagerLogic.h"
#include "Ships\ship.h"

//============================================================================================

TrackManagerLogic::TrackManagerLogic() :
	m_TrackList(_FL_),
	m_ShipsList(_FL_)
{
}

TrackManagerLogic::~TrackManagerLogic()
{
	m_TrackList.DelAll();
	m_ShipsList.DelAll();
}


//============================================================================================


//Инициализировать объект
bool TrackManagerLogic::Create(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

//Активировать
void TrackManagerLogic::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if(!EditMode_IsOn())
	{
		if( IsActive() )
		{
			LogicDebug("Activate");
			SetUpdate(&TrackManagerLogic::Work, ML_TRIGGERS);
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&TrackManagerLogic::Work);
		}
	}else{
		DelUpdate(&TrackManagerLogic::Work);
	}
}

//============================================================================================

//Работа детектора
void _cdecl TrackManagerLogic::Work(float dltTime, long level)
{
	// проверяем наличие свободных кораблей
	if( m_ShipsList.Size()==0 )
	{
		// нет свободных кораблей нечего делать, закончим работу
		Activate(false);
		return;
	}

	// проверяем треки на наличие свободных
	for( dword n=0; n<m_TrackList.Size(); n++ )
	{
		bool bIsLaunchNextShip = false;
		if( m_TrackList[n].spShip.Validate() )
		{
			if( m_TrackList[n].spShip.Ptr()->IsDead() )
				bIsLaunchNextShip = true;
		}
		else
			bIsLaunchNextShip = true;

		// пробуем запустить корабль на трек
		while( bIsLaunchNextShip && m_ShipsList.Size()>0 && m_TrackList[n].AIParams.Size()>0 )
		{
			// берем очередной корабль и удаляем его из списка очередников
			const ConstString pcShipID = m_ShipsList[0];
			m_ShipsList.DelIndex(0);
			// ищем объект представляющий этот корабль
			if( FindObject(pcShipID, m_TrackList[n].spShip) )
			{
				// выбираем рандомный трек из текущей группы треков
				const char* pcNewAI = NULL;
				dword idxTrack = rand() % m_TrackList[n].AIParams.Size();

				// ставим новый АИ (трек) для выбранного корабля
				const char* aParams[1];
				aParams[0] = m_TrackList[n].AIParams[idxTrack];
				m_TrackList[n].spShip.Ptr()->Command("changeAI",1,aParams);
				break;
			}
		}
	}
}

void TrackManagerLogic::ReadMOPs(MOPReader & reader)
{
	// тут нечего показывать - это логический объект
	Show(false);

	// список кораблей
	dword shipsQ = reader.Array();
	m_ShipsList.DelAll();
	if( shipsQ > 0 )
	{
		m_ShipsList.AddElements(shipsQ);
		for( dword n=0; n<shipsQ; n++ )
			m_ShipsList[n] = reader.String();
	}

	// список треков
	dword trackGroupQ = reader.Array();
	m_TrackList.DelAll();
	if( trackGroupQ > 0 )
	{
		m_TrackList.AddElements(trackGroupQ);
		for( dword n=0; n<trackGroupQ; n++ )
		{
			m_TrackList[n].spShip.Reset();
			dword trackQ = reader.Array();
			if( trackQ > 0 )
			{
				m_TrackList[n].AIParams.AddElements(trackQ);
				for( dword i=0; i<trackQ; i++ )
					m_TrackList[n].AIParams[i] = reader.String().c_str();
			}
		}
	}

	// активация
	Activate(reader.Bool());
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(TrackManagerLogic, "Ship track manager", '1.00', 0x0fffffff, "Track manager looking for ship objects order and track order.\nSend ship from common order into free track and triggered when no more ship", "Logic")
	MOP_ARRAYBEG("Ships list", 0, 100)
		MOP_STRING("Ship object id", "")
	MOP_ARRAYEND
	MOP_ARRAYBEG("Track groups", 1, 100)
		MOP_ARRAYBEG("Track AI parameters list", 1, 100)
			MOP_STRING("AI parameters", "")
		MOP_ARRAYEND
	MOP_ARRAYEND
	MOP_BOOL("Active", true)
MOP_ENDLIST(TrackManagerLogic)
