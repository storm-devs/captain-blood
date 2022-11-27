#ifndef _ButtonMG_h_
#define _ButtonMG_h_


#include "..\..\..\common_h\Mission.h"
#include "..\Character.h"

enum MiniGame_type
{
	mg_disabled = 0,
	mg_button_normal = 1,
	mg_button_cycled = 2
};	

struct rnd_memory_params
{
	const char* difficulty;
	bool  remember_button;
};

struct MiniGame_Desc
{		
	const char* id;

	MiniGame_type type;
	const char*   button;
	
	array<rnd_memory_params> memory_params;

	const char* difficulty;
	const char* autowin_difficulty;

	int   iTimes;
	float fDuration;

	bool  wait_end_mg;
	const char* end_node;

	ConstString wintriger;
	ConstString losetriger;
	ConstString buttontriger;

	ConstString widgets[16];	

	MiniGame_Desc(): memory_params(_FL_)
	{
		static const ConstString wgs[16] =
		{
			ConstString("Wgt_MngA"),
			ConstString("Wgt_MngB"),
			ConstString("Wgt_MngC"),
			ConstString("Wgt_MngD"),
			ConstString("Wgt_MngLeft"),
			ConstString("Wgt_MngRight"),
			ConstString("Wgt_MngUp"),
			ConstString("Wgt_MngDown"),
			ConstString("Wgt_MngA_anim"),
			ConstString("Wgt_MngB_anim"),
			ConstString("Wgt_MngC_anim"),
			ConstString("Wgt_MngD_anim"),
			ConstString("Wgt_MngLeft_anim"),
			ConstString("Wgt_MngRight_anim"),
			ConstString("Wgt_MngUp_anim"),
			ConstString("Wgt_MngDown_anim")
		};

		for(dword i = 0; i < 16; i++)
		{
			widgets[i] = wgs[i];
		}
	}
};

class CharacterButtonMiniGame
{	
public:	

	struct TNodeButton
	{		
		unsigned long  hash;
		int  button;
	};

	static array<TNodeButton> nodeButtons;

	int  iMG_Button;	

	struct PressedButton
	{
		char ButtonName[64];		
		long ButtonCode;
		bool bBeenPressed;		
	};
	
	ConstString widgets[16];
	array<PressedButton> buttonsStatus;

	MiniGame_type mode;

	int num_pressed;

	float button_duration;
	float button_time;

	int   button_times;

	bool  right_pressed;
	bool  lose_triggered;

	bool  wait_end_mg;

	MissionObject* win_triger;
	MissionObject* lose_triger;
	MissionObject* button_triger;

	const char* end_node;
	const char* played_node;

	const char* id;

	bool autoSkip;

	bool nodeChanged;

	int last_show_wg;	

	int debug_count;

	bool auto_win_game;

public:
		
	CharacterButtonMiniGame(Character * character);

	void Init();
	void AddButton(const char* BtnName);
	void ChoseRandomButton(bool remember_button, float border, int offset);
	void Start(MiniGame_Desc& desc, bool auto_win);
	void ShowWidgetImpl(bool show,int bt_index, bool is_anim);
	void ShowWidget(bool show);
	bool IsButtonPressed();
	void Update(float fDeltaTime,IAnimation** anim, int num_anim);
	void ChoseLink(IAnimation** anim, int num_anim);
	void Finish();

	void ActivateTriger();	

	bool FilterDifficulty(const char* difficulty);	

	inline bool IsEnabled() { return mode != mg_disabled; };

	void ActivateLink(const char* link,IAnimation** anim, int num_anim);

	void WinNormalMG(IAnimation** anim, int num_anim);
	void WinCycledMG(IAnimation** anim, int num_anim);

	bool IfMissionTime();
	void SetMissionTime(MissionObject* tm);

	bool NeedToRememeber(MiniGame_Desc& desc);

private:

	Character* chr;

	MissionObject* m_time;
	bool           bGameBroken;
};

#endif

