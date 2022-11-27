#include "MissionTimeCalculator.h"

CREATE_SERVICE(TimeCalcService, 500)


TimeCalcService::TimeCalcService()
{
	m_mode = CM_UI;
	fGameTime = 0.0f;

	fTime = 0.0f;

	timeInSec = NULL;
	sec = NULL;
	minutes = NULL;
	hours = NULL;
	detail = NULL;

	bActive = false;

	pRS = NULL;
	pConsole = NULL;

	bShowTime = false;
}

TimeCalcService::~TimeCalcService()
{
	ReleaseStorage();
}

void TimeCalcService::ReleaseStorage()
{
	if (timeInSec)
	{
		timeInSec->Release();
		timeInSec = NULL;
	}

	if (sec)
	{
		sec->Release();
		sec = NULL;
	}

	if (minutes)
	{
		minutes->Release();
		minutes = NULL;
	}

	if (hours)
	{
		hours->Release();
		hours = NULL;
	}

	if (detail)
	{
		detail->Release();
		detail = NULL;
	}
}

//Инициализация
bool TimeCalcService::Init()
{
	api->SetEndFrameLevel(this, 100);	
	return true;
}

//Вызываеться перед удалением сервисов
void TimeCalcService::PreRelease()
{
}

void TimeCalcService::CalculateHMS(float fSec, long & h, long & m, long & s)
{
	h = (long)(fSec / 3600.0f);
	m = ((long)(fSec / 60)) - (h * 60);
	s = (long)(fSec) - (h * 3600) - (m * 60);
}

void _cdecl TimeCalcService::EnableDisableDebug(const ConsoleStack& stack)
{
	bShowTime = !bShowTime;

	if (!pConsole) return;
	
	if (bShowTime)
	{
		if (pConsole)
			pConsole->Trace(COL_ALL, "Debug time info - enabled");
	} else
	{
		if (pConsole)
			pConsole->Trace(COL_ALL, "Debug time info - disabled");
	}


}

//Исполнение в конце кадра
void TimeCalcService::EndFrame(float dltTime)
{
	if (!bActive) return;
	bActive = false;

	dltTime = api->GetNoScaleDeltaTime();


	if (pRS == NULL)
	{
		pRS = (IRender*)api->GetService("DX9Render");
	}

	if (pConsole == NULL)
	{
		pConsole = (IConsole *)api->GetService("Console");
		if (pConsole)
			pConsole->RegisterCommand("st", "show time statistic info", this, (CONSOLE_COMMAND)&TimeCalcService::EnableDisableDebug);
	}

	if (pRS && bShowTime)
	{
		long _h;
		long _m;
		long _s;
		CalculateHMS(fGameTime, _h, _m, _s);

		pRS->Print(32, 32, 0xFFFFFFFF, "game time - %02d:%02d:%02d", _h, _m, _s);
	}


	fGameTime += dltTime;
	fTime += dltTime;

	float fSecValue = 0.0f;
	bool bisOK = false;

	if (timeInSec)
	{
		fSecValue = timeInSec->Get(0.0f, &bisOK);
	}

	if (fTime > 1.0f)
	{
		if (timeInSec)
		{
			//fSecValue = timeInSec->Get(0.0f, &bisOK);
			fSecValue += fTime;
			timeInSec->Set(fSecValue);

			if (sec && minutes && hours && detail)
			{
				float fDetailSecValue = detail->Get();
				fDetailSecValue += fTime;
				detail->Set(fDetailSecValue);

				long h;
				long m;
				long s;
				CalculateHMS(fSecValue, h, m, s);

				sec->Set(s);
				minutes->Set(m);
				hours->Set(h);
			}
		} 
		fTime = 0.0f;
	}



	if (pRS && bShowTime)
	{
		if (timeInSec)
		{
			long _h;
			long _m;
			long _s;
			CalculateHMS(fSecValue, _h, _m, _s);

			const char * m = "none";
			switch (m_mode)
			{
			case TimeCalcService::CM_UI:
				m = "ui";
				break;
			case TimeCalcService::CM_SHOPING:
				m = "shoping";
				break;
			case TimeCalcService::CM_CUTSCENE:
				m = "cutscene";
				break;
			case TimeCalcService::CM_SHOOTER:
				m = "shooter";
				break;
			case TimeCalcService::CM_LAND:
				m = "land";
				break;
			case TimeCalcService::CM_NAVAL:
				m = "naval";
				break;


			}

			pRS->Print(32, 32+16, 0xFFFFFFFF, "Profile time [%s] - %02d:%02d:%02d, storage query: %d, mode : '%s'", dif.c_str(), _h, _m, _s, bisOK, m);
		} else
		{
			pRS->Print(32, 32+16, 0xFFFF0000, "No difficulty set! profile time not calculated");
		}
	}


}


void TimeCalcService::Update(bool bEnabled, TimeCalcService::CalcMode mode)
{
	bActive = bEnabled;
	m_mode = mode;

	const char* difficulty = api->Storage().GetString("profile.global.difficulty", NULL);
	if (difficulty)
	{
		//Что бы несколько раз не делать...
		if (_stricmp(difficulty, dif.c_str()) == 0)
		{
			return;
		}


		dif = difficulty;

		ReleaseStorage();

		static char tmpBuff[128];
		switch(mode)
		{
		case TimeCalcService::CM_LAND:
			crt_snprintf(tmpBuff, 120, "Profile.%s.time.land.totalSec", difficulty);
			detail = api->Storage().GetItemFloat(tmpBuff, _FL_);
			break;
		case TimeCalcService::CM_NAVAL:
			crt_snprintf(tmpBuff, 120, "Profile.%s.time.navy.totalSec", difficulty);
			detail = api->Storage().GetItemFloat(tmpBuff, _FL_);
			break;
		case TimeCalcService::CM_SHOOTER:
			crt_snprintf(tmpBuff, 120, "Profile.%s.time.shooter.totalSec", difficulty);
			detail = api->Storage().GetItemFloat(tmpBuff, _FL_);
			break;
		case TimeCalcService::CM_SHOPING:
			crt_snprintf(tmpBuff, 120, "Profile.%s.time.shoping.totalSec", difficulty);
			detail = api->Storage().GetItemFloat(tmpBuff, _FL_);
			break;
		case TimeCalcService::CM_UI:
			crt_snprintf(tmpBuff, 120, "Profile.%s.time.ui.totalSec", difficulty);
			detail = api->Storage().GetItemFloat(tmpBuff, _FL_);
			break;
		case TimeCalcService::CM_CUTSCENE:
			crt_snprintf(tmpBuff, 120, "Profile.%s.time.cutscene.totalSec", difficulty);
			detail = api->Storage().GetItemFloat(tmpBuff, _FL_);
			break;
		}


		crt_snprintf(tmpBuff, 120, "Profile.%s.time.totalSec", difficulty);
		timeInSec = api->Storage().GetItemFloat(tmpBuff, _FL_);

		crt_snprintf(tmpBuff, 120, "Profile.%s.time.h", difficulty);
		hours = api->Storage().GetItemLong(tmpBuff, _FL_);

		crt_snprintf(tmpBuff, 120, "Profile.%s.time.m", difficulty);
		minutes = api->Storage().GetItemLong(tmpBuff, _FL_);

		crt_snprintf(tmpBuff, 120, "Profile.%s.time.s", difficulty);
		sec = api->Storage().GetItemLong(tmpBuff, _FL_);
	} else
	{
		ReleaseStorage();
		api->Trace("Difficulty is not set, time value can't be saved");
	}




}

//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================

MissionTimeCalculator::MissionTimeCalculator()
{
	mode = TimeCalcService::CM_UI;
	srv = NULL;
}

MissionTimeCalculator::~MissionTimeCalculator()
{

}


//Создание объекта
bool MissionTimeCalculator::Create(MOPReader & reader)
{
	EditMode_Update (reader);
	return true;
}


//Обновление параметров
bool MissionTimeCalculator::EditMode_Update(MOPReader & reader)
{
	srv = (TimeCalcService*)api->GetService("TimeCalcService");

	char enumMode = reader.Enum().c_str()[0];
	switch(enumMode)
	{
	case 'L':
		mode = TimeCalcService::CM_LAND;
		break;
	case 'N':
		mode = TimeCalcService::CM_NAVAL;
		break;
	case 'M':
		mode = TimeCalcService::CM_SHOOTER;
		break;
	case 'S':
		mode = TimeCalcService::CM_SHOPING;
		break;
	case 'U':
		mode = TimeCalcService::CM_UI;
		break;
	case 'C':
		mode = TimeCalcService::CM_CUTSCENE;
		break;
	}

	Activate(reader.Bool());
	return true;
}


void _cdecl MissionTimeCalculator::TimeCalculator(float fDeltaTime, long level)
{
	if (fDeltaTime < 0.0001)
	{
		return;
	}

	if (!srv)
	{
		return;
	}
	
	srv->Update(true, mode);
}


void MissionTimeCalculator::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (isActive)
	{
		SetUpdate((MOF_UPDATE)&MissionTimeCalculator::TimeCalculator, ML_SOUND_LISTENER);

		
	} else
	{
		DelUpdate((MOF_UPDATE)&MissionTimeCalculator::TimeCalculator);
	}
}



MOP_BEGINLISTG(MissionTimeCalculator, "MissionTimeCalculator", '1.00', 100, "Managment")
	MOP_ENUMBEG("Details Modes")
		MOP_ENUMELEMENT("Land fight")
		MOP_ENUMELEMENT("Naval fight")
		MOP_ENUMELEMENT("Minigame shooter")
		MOP_ENUMELEMENT("Shoping")
		MOP_ENUMELEMENT("UI")
		MOP_ENUMELEMENT("CutScenes")
	MOP_ENUMEND

	MOP_ENUM("Details Modes", "Details")

	MOP_BOOL("Active", true)
MOP_ENDLIST(MissionTimeCalculator)
