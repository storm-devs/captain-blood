#ifndef _WaterTrack_h_
#define _WaterTrack_h_

#include "..\..\..\Common_h\Mission.h"

class WaterTrack : public MissionObject
{
private:
	bool bPrevPosInited;
	Vector vPrevPos;

	Matrix objectMatrix;

	ConstString MissionObjectName;
	MOSafePointer pMaster;

	IParticleSystem* BurunParticle;
	IParticleSystem* BurunBackParticle;

	float m_fParticleScale;

	Vector offset;
	Vector back_offset;

	Vector curPos;
	Vector backPos;

	float fTime;

	bool bSmoothSpeedInited;
	float fSmoothSpeed;

	bool bWorkOnlyWithActive;
	bool bWorkOnlyWithVisible;
	bool bWorkOnlyWithLive;

	bool noSwing;

public:
	 WaterTrack();
	~WaterTrack();

	//Создание объекта
	virtual bool Create			(MOPReader &reader);

	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader &reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader &reader);

	void _cdecl Work(float fDeltaTime, long level);

	virtual void Restart();

private:
	void ClearTrack();
	void PauseParticles(bool bPause);
};

#endif
