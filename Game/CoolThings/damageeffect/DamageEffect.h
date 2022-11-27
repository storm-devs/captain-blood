#pragma once

#include "..\..\..\Common_h\Mission.h"

class IAfterImage;
class IBlur;

class DamageEffect : public MissionObject
{
	struct Vertex
	{
		float x,y; Color c;
	};


	ShaderId DamageEffect_id;

public:

	 DamageEffect();
	~DamageEffect();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Activate(bool isActive);
	void Show	 (bool isShow);

	void Command(const char *id, dword numParams, const char **params);

	void Restart();

private:

	void _cdecl Draw(float dltTime, long level);

	void CreateBuffer();
	void UpdateBuffer();

	void InitParams		(MOPReader &reader);

private:

	//// editor params ////

	ConstString CharacterName;

	MissionTrigger TriggerOnHit;

	const char *OnHitTexture;
	Color OnHitColor;

	float OnHitShowTime;
	float OnHitHideTime;

	float OnHitBorderWidth;
	float OnHitBorderHeight;

	float OnDeadStartHP;

	MissionTrigger OnDeadTriggerON;
	MissionTrigger OnDeadTriggerOFF;

	float OnDeadBlendTime;

	Color OnDeadColorStart;
	Color OnDeadColorEnd;

	const char *OnDeadTexture;

/*	const char *OnDeadTimeScaleObject;

	float OnDeadTimeScaleStart;
	float OnDeadTimeScaleEnd;*/

	ConstString OnDeadSoundName;

	float OnDeadSoundVolumeStart;
	float OnDeadSoundVolumeEnd;

	long OnDeadBlurPasses;

	float OnDeadBlurStart;
	float OnDeadBlurEnd;

	bool OnDeadBlurRadial;

	float OnDeadAfterApplyForceStart;
	float OnDeadAfterApplyForceEnd;

	float OnDeadAfterPostFactorStart;
	float OnDeadAfterPostFactorEnd;

	float OnDeadBeatMin;
	float OnDeadBeatMax;

	float OnDeadBeatAmp;
	float OnDeadBeatBlurMul;

	string OnDeadForce;

	MissionTrigger OnDeathTrigger;

	///////////////////////

	bool bActive;

	MOSafePointer character;

	IBaseTexture *pFace;
	IBaseTexture *pBack;

	IVariable *Texture;
	IVariable *Alpha;

	float fore;
	float back;

	IVBuffer *pVBuffer;

	float prevHP;
	float nextHP;

	float time;

	float blen;
	float blenTime;

	float beat;

	float backHP;

	bool OnDead;
	bool IsDead;

//	IAfterImage *pAfterImage;
	//IBlur		*pBlur;

	ISound *beatSound;

	IForce *force;
	long	forceDevice;

	bool firstTime;

	float rageBeg;
	float rageEnd; float rageTime; float rageTo;

	bool prevDead;

	bool bEnabled;

	bool m_show;
	bool m_active;

};
