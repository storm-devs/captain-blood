#ifndef Door_H
#define Door_H

#include "QuickEvent.h"
#include "DoorBlender.h"
#include "..\..\..\Common_h\ICharacter.h"

class Door : public QuickEvent
{
private:
	bool brokeStarted;

public:
			
	DoorBlendStage* doorBlendStage;	
	
	struct diff_timing
	{
		const char* difficulty;		
		int   num_presses;

#ifndef CHARACTER_STARFORCE
		int __inline GetPresses(Door * door) { return num_presses; }
#else
		int __inline GetPresses(Door * door) {
			AssertStarForce(int(*door->SpikeValue) == 0);
			return num_presses + int(*door->SpikeValue);
		}
#endif
	};	

	int         cur_timing;
	int         num_timing;
	diff_timing timings[7];
	
	float anim_speed;

	float button_time;
	int   num_pressed;
	
	float anim_addspeed;
	float anim_speedlimit;
	float anim_downspeed;

	ConstString progressBar;

#ifdef CHARACTER_STARFORCE
	float checkSpike;
	float * SpikeValue;
#endif

	MissionTrigger triger_Broke;
	MissionTrigger triger_Kick;
	
	MOSafePointer bar;

	float fPushTime;

	bool  bIstance;

	int      num_coliders_open;
	TColider coliders_open[5];

	bool  isOpened;
	ConstString kick_Widget;

	bool  post_finish;

	virtual ~Door();	

	virtual void InitData();		
	virtual bool EditMode_Update(MOPReader & reader);	
	virtual void _cdecl EditMode_Draw(float dltTime, long level);

	virtual void Release();

	virtual void BeginQuickEvent();	
	virtual void QuickEventUpdate(float dltTime);
	virtual bool AllowInterruptByButton();
	virtual void Interupt(bool win_game);
	virtual void AnimEvent(const char * param);	

	virtual bool InteruptOnHit();

	virtual void ActivateColision(bool isAct);

	virtual void Command(const char * id, dword numParams, const char ** params);

	virtual const char* GetName() { return "Door";};

	virtual void HideHUD();

	void DetermineDifficulty();
	void BarSetHP(float hp);

	static const char * comment;
};

#endif