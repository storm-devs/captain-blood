/*#ifndef Milagrosa_Minigame
#define Milagrosa_Minigame

#include "..\..\Common_h\Mission.h"
#include "..\Character\Components\ButtonMG.h"



class MilagrosaMinigame : public MissionObject
{
public:

	enum charstate
	{
		chr_idle = 0,
		chr_walk = 1,
		chr_upattack = 2,
		chr_downattack = 3,
		chr_hit = 4,
		chr_jump = 5,
		chr_duck = 6,
		chr_parish = 7,
		chr_fall = 8,
		chr_fight = 9,
		chr_weak = 10
	};

	CharacterButtonMiniGame buttonMG;

	class AnimListener : public IAnimationListener
	{
	public:
		MilagrosaMinigame* pMilagrosaMinigame;

		AnimListener()
		{
			pMilagrosaMinigame = NULL;
		};

		//начинаем мини игру
		void _cdecl BeginMiniGame(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (!pMilagrosaMinigame) return;
			
			if (numParams<1) return;

			MiniGame_Desc desc;

			desc.id = "id";
							
			desc.type = mg_disabled;
			desc.button = params[0];

			desc.difficulty = null;
			desc.autowin_difficulty = null;

			desc.iTimes = 10;
			desc.fDuration = 0.5f;

			desc.wait_end_mg = false;
			desc.end_node = null;

			pMilagrosaMinigame->buttonMG.Start(desc,false);
		}

		//Выбираем линк анимации	
		void _cdecl ChooseAnimLink(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (!pMilagrosaMinigame) return;
			
			IAnimation* anim[1];
			anim[0] = pMilagrosaMinigame->player.animation;
			pMilagrosaMinigame->buttonMG.ChoseLink(anim,1);

			if (!pMilagrosaMinigame->buttonMG.IsButtonPressed())			
			{
				pMilagrosaMinigame->player.HP = 0;

				pMilagrosaMinigame->player.animation->ActivateLink("Death");

				pMilagrosaMinigame->boss.win_triger.Activate(pMilagrosaMinigame->Mission(),true);				
			}
		};


		void _cdecl AnimEvent(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (numParams<1)
			{
				return;
			}

			if(pMilagrosaMinigame && string::IsEqual(params[0],"Player Hit"))
			{
				pMilagrosaMinigame->PlayerHit();
			}
			else
			if(pMilagrosaMinigame && string::IsEqual(params[0],"Boss Hit"))
			{
				pMilagrosaMinigame->BossHit();
			}
			else
			if(pMilagrosaMinigame && string::IsEqual(params[0],"Show Help"))
			{
				pMilagrosaMinigame->ShowHelpWidgets(true);
			}
		};
	};

	int fight_phase;
	int num_pressed;

	float button_duration;
	float button_time;

	int   button_times;

	AnimListener animListener;

	bool IsTargetAttack;
	float timetoattack;

	int NumPlayerHits;	
	int NumBossHits;	

	bool bFightMiniGame;

	Vector left_border,right_border;

	struct TCharacter
	{
		IGMXScene*  model;
		IAnimation* animation;
		IGMXScene*  item;
		
		const char* progress_bar;
		MissionTrigger win_triger;		

		float delta;		

		Vector      pos;
		float       ay;
		int         HP;
		int         MaxHP;

		TCharacter()
		{
			model = NULL;
			animation = NULL;
			item = NULL;

			pos = 0.0f;
			ay  = 0.0f;

			delta = 0.0f;
		}
	};
	
	TCharacter player;
	TCharacter boss;		
	
	float deltatime;

	const char* widgetFightID;
	MOSafePointer widgetFight;
	
	const char* widgetUpID;
	MOSafePointer widgetUp;

	const char* widgetDownID;
	MOSafePointer widgetDown;	

	float timeto_show_Wgt;
	float maxtimeto_show_Wgt;

	//Конструктор - деструктор
	MilagrosaMinigame();
	virtual ~MilagrosaMinigame();

	virtual void Activate(bool isActive);

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	MO_IS_FUNCTION(Milagrosa_Minigame, MissionObject);

	void _cdecl Realize(float fDeltaTime, long level);
	void _cdecl EditMode_Draw(float fDeltaTime, long level);

	void DrawItem(TCharacter* chr, const char* locator);
	void DrawCharacter(TCharacter* chr);

	static const char * comment;

	virtual void PlayerHit();
	virtual void BossHit();

	void BossThinking(float fDeltaTime);
	void MovePlayer();
	void MoveCharacter(TCharacter* chr, TCharacter* enemy_chr);
	void CheckBorder(TCharacter* chr,TCharacter* enemy_chr);

	void LoadModel(TCharacter* chr,char* model,char* animation,char* weapon);
	void StartFightMiniGame();
	void FightMiniGame(float fDeltaTime);

	void UpdateHelpWidgets(float fDeltaTime);
	void ShowHelpWidgets(bool show);

	void HitCharater(TCharacter* chr, TCharacter* enemy_chr);

	//Пересоздать объект
	virtual void Restart();

	float CalcDistance();
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);

	const char * GetConstString(TCharacter* chr,const char * constName);
	charstate GetState(TCharacter* chr);

	bool IfCanStartFight();
	void UpdateBar(TCharacter* chr);
	//Нарисовать модельку персонажа для тени
	void _cdecl ShadowInfo(const char * group, MissionObject * sender);
	//Нарисовать модельку персонажа для тени
	void _cdecl ShadowDraw(const char * group, MissionObject * sender);	
};

#endif*/