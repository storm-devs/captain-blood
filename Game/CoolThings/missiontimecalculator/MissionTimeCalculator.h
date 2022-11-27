#ifndef MISSION_TIME_CALCULATOR
#define MISSION_TIME_CALCULATOR

#include "..\..\..\Common_h\Mission.h"


class TimeCalcService : public Service
{
public:

	enum CalcMode
	{
		CM_UI = 0,
		CM_SHOPING = 1,
		CM_CUTSCENE = 2,
		CM_SHOOTER = 3,

		CM_LAND = 4,
		CM_NAVAL = 5,

		CM_FORCE_DWORD = 0x7fffffff
	};


private:

	bool bShowTime;

	IRender* pRS;
	IConsole* pConsole;

	bool bActive;

	float fGameTime;
	float fTime;

	ICoreStorageFloat * detail;
	ICoreStorageFloat * timeInSec;
	ICoreStorageLong * sec;
	ICoreStorageLong * minutes;
	ICoreStorageLong * hours;

	void ReleaseStorage();

	string dif;
	CalcMode m_mode;

	void CalculateHMS(float fSec, long & h, long & m, long & s);

	void _cdecl EnableDisableDebug(const ConsoleStack& stack);


public:


	TimeCalcService();
	virtual ~TimeCalcService();

	//Инициализация
	virtual bool Init();
	//Вызываеться перед удалением сервисов
	virtual void PreRelease();

	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime);


	void Update(bool bEnabled, TimeCalcService::CalcMode mode);

};


class MissionTimeCalculator : public MissionObject
{
	TimeCalcService::CalcMode mode;

	TimeCalcService* srv;

public:

	MissionTimeCalculator();
	~MissionTimeCalculator();

	void _cdecl TimeCalculator(float fDeltaTime, long level);

	void Activate(bool isActive);

	bool Create(MOPReader & reader);
	bool EditMode_Update(MOPReader & reader);
};


#endif