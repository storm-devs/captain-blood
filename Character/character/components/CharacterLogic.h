//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// CharacterLogic
//===========================================================================================================================

#ifndef _CharacterLogic_h_
#define _CharacterLogic_h_


#include "..\Character.h"

#include "..\..\auxiliary objects\Fatality Params\FatalityParams.h"
#include "..\..\auxiliary objects\Attack Params\AttackParams.h"
#include "..\..\auxiliary objects\NoFatality\NoFatality.h"
#include "..\..\auxiliary objects\EffectTable\EffectTable.h"
#include "..\..\Auxiliary objects\AutoZoom\AutoZoom.h"
#include "..\..\utils\SafePointerEx.h"
#include "..\..\..\Common_h\ICharacter.h"


class CharacterAnimationEvents;
class CharacterItems;


class CharacterLogic
{
	friend class CharacterAnimationEvents;
	friend class CharacterItems;


public:
	//Логичесткое состояние персонажа, задаваемое в анимации
	enum State
	{
		state_unknown,
		state_idle,
		state_walk,
		state_run,
		state_strafe,
		state_attack,
		state_block,
		state_guard,
		state_shoot,
		state_findpair,
		state_topair,
		state_pair,
		state_dmg_pair,
		state_knockdown,
		state_kick,
		state_hit,
		state_die,
		state_dead,
		state_aimbomb,
		state_dropbomb,
		state_roll,
		state_imbo
	};

	//Сторона в бою
	enum Side
	{
		s_enemy = 0,
		s_ally,
		s_ally_frfire,		
		s_boss,
		s_npc		
	};
	
//--------------------------------------------------------------------------------------------
//Инициализация
//--------------------------------------------------------------------------------------------
public:
	CharacterLogic(Character * character);
	~CharacterLogic();

	//Дополнительная инициализация, когда есть моделька и анимация
	void Init();
	//Сбросить состояние
	void Reset();

//--------------------------------------------------------------------------------------------
//Логические состояния персонажа
//--------------------------------------------------------------------------------------------
public:
	//Установить количество жизней и дать максимальное количество
	void InitHP(float hp);
	//Установить новое значение HP
	void SetHP(float hp,bool drop_armor = true);
	//Получить текущее значение HP
	float GetHP();
	//Установить максимальное количество жизней 
	void InitMaxHP(float hp);
	//Получить текущее значение maxHP
	float GetMaxHP();

	float GetAbsoluteHP();

	//Сделать персонажа актёкром
	void SetActor(bool isEnable);
	//Актёр ли персонаж
	bool IsActor() const;
	//В режиме парной анимации
	bool IsPairMode();
	//Врежиме атаки персонаж или нет
	bool IsAttack();

	//Работает ли в данный момент минигейм
	bool IsMinigameStarted();

	//Умереть
	void Death(dword source, const char* deathLink = NULL);
	//Жив или нет персонаж
	bool IsDead();

	//Установить сторну персонажа
	void SetSide(Side sd);
	//Получить сторну персонажа
	Side GetSide();

	State GetState();
	State GetCurrentState();
	const char* GetStateString();

	// Возвращает вероятность блокирования(<0.0 то стандартная логика, иначе срабатывает % вероятности)
	float GetBlockState();

	//Получить текстовое описание стейта
	static const char * GetStateText(State state);

//--------------------------------------------------------------------------------------------
//Функции боя
//--------------------------------------------------------------------------------------------
public:
	//Враг персонаж или нет
	bool IsRealEnemy(Character* opponent, bool includeNPC = true);
	//Враг персонаж или нет, для плеера, даже друг возвращается как враг если рядом нет других врагов
	bool IsEnemy(Character* opponent, bool includeNPC = true);

	bool IsSlave();

	//Пнуть персонажа в заданном направление
	void Kick(const Vector & dir);
	
	//Бросить бомбу
	void DropBomb(const Vector & boomPosition);
	//Попадание выстрела
	void ShootHit(float dmg);

	//Перейти в режим парной анимации
	void SetPairMode(bool isPair);	

	//Получить имя линка на парную анимацию
	const char* GetToPairLink(FatalityParams::TFatalityType &type);

	//Проверка на возмжность выполнения фаталити персу
	bool PairModeAllowed(bool ignoreFatalityTime, bool ignore_ai, bool* is_instant = null);	

	Character * FindCharacterForPair(Character & forWho, bool skip_ally, char * pairLink, FatalityParams::TFatalityType & type, int filter, bool * charExist = null);
	void PairProcess(float dltTime, int filter);
	void EndPairProcess();

	Character* FindTarget(float distance,float secktor);
	void AimToTarget(float distance,float secktor);

	//Попадание холодным оружием
	void Hit(dword source, float dmg, const char * reaction, float hpLimit, float block_dmg, const char * blockReaction, const char * deathLink, Character* _offender, bool checkImmune = true, bool play_hit = true);
	//Применить попадание
	void HitMoment(const char * reaction);


	void UpdateFatalityParams ();


	inline void SetMinHPLimitter(float MinHPLimiter)
	{
		fMinHPLimiter = MinHPLimiter;
		if (fMinHPLimiter<0.01f) fMinHPLimiter = -1.0f;
		if (fMinHPLimiter>1.0f) fMinHPLimiter = 1.0f;
	};

	inline float GetMinHPLimitter() { return fMinHPLimiter;};
	inline void CorrectHP()
	{ 		
		if (fMinHPLimiter>-0.5f)
		{
			if(HP < GetMaxHP() * fMinHPLimiter) HP = GetMaxHP() * fMinHPLimiter;
		}
		else
		{
			if(HP < 0.0f) HP = 0.0f;
		}
	};

private:


	//Переход в режим атаки и обратно
	void SetAttackActive(bool isAttackNow);
	//Атаковать демедж детектор
	void Attack(DamageObject * obj, const Vector quadrangle[4]);
	//Атаковать заданного персонажа
	bool Attack(Character * chr, dword source);	
	//Невозможность нанесения повреждения
	bool IsHitImmune();
	//Невозможность попадания выстрела
	bool IsShotImmune();


//--------------------------------------------------------------------------------------------
//Разное
//--------------------------------------------------------------------------------------------
public:
	//Обновить логическое состояние персонажа
	void Update(float dltTime);	

	//Получить имя линка из текущего графа в графе главного персонажа для парной анимации
	const char* GetPairLink(char* NodeName = NULL);

	bool AllowOrient();
	bool AllowOrientHead();

	float GetCrossLinkTime();
	
	//Получить имя перса
	const char* GetCharName();

	// Ставит флажок что в следующий раз когда спросят GetState() надо его взять из анимации, а не прекешированный
	void NeedStateUpdate();

private:

	Character & chr;			//Персонаж которому принадлежим
	float HP, maxHP;			//Текущее и максимальное значение здоровия
	float fMinHPLimiter;         //Нижний предел, ниже которого ХП не будет опускаться
	Side side;					//Сторона персонажа
	float attackDamage;			//Демедж, который нанесётся в случае события атаки
	bool isDead;				//Игрок умер
	bool isActor;				//Актёр ли персонаж
	bool isAttack;				//В режиме атаки персонаж или нет
	bool isPairMode;			//Режим парной анимации
	float turnSpd;				//Скорость поворота заданная в физике	
	array<void *> attacks;		//Уже атакованые объекты, которых больше пока не трогать
	long stopPairCounter;		//Счётчик остановки парной анимации
	long stopShootCounter;		//Счётчик сброса цели
	long stopKickCounter;		//Счётчик сброса состояния пинка

#ifdef CHARACTER_STARFORCE
	float * time2StartDamageSpikes;
#endif

	bool needUpdateState;
	State cachedState;

	bool animStateWasOrIsDieOrDead;

	NoFatality*     pNoFatality;

	Character* offender;	
	EffectTable* offenderEffectTable;
	
	int    pair_count;
	Vector dir_pair_v;
	Vector last_pair_v;

	bool       auto_aim;
	Character* targeted_char;

	bool find_pair_failed;

public:

	MOSafePointerTypeEx<FatalityParams> FParams;
	bool  check_fatality_state;	

	MOSafePointerTypeEx<AttackParams> attackParams;
	AttackParams::attacks_params_state attacks_params_st;

	MOSafePointerType<AutoZoom> autoZoom;

	char	last_pair_link[32];

	Character* pair;
	Character* pair_mg;

	const char*  attack_react;
	void SetAttackReaction(const char*  react) { attack_react = react; };
	
	float attack_damage;
	void SetAttackDamage(float dmg) { attack_damage = dmg; };

	float time_to_return;

	bool deathPass;

	struct TShootTarget
	{
		Character* target;
		bool bFlyAway;
		float fDamageKoef;
		char  react[32];

		TShootTarget()
		{
			target = NULL;
			bFlyAway = true;
			fDamageKoef = 1.0f;
			react[0] = 0;
		}
	};

	array<TShootTarget> shootTargets;	//Цель, в которую стреляем	

	void SetShootTarget(Character* target,const char* hit_react,float fDamageKoef = 1.0f, bool bFlyAway = true, bool bAppend = false);
	bool IsMultipleShootTargets();

	bool InBombSafe(Vector pos);

	void SetOffender(Character* _offender) { offender = _offender; }
	Character* GetOffender() { return offender;}	

	bool    CheckPairLinks(Character* target, const char* action, char* pairLink, FatalityParams::TFatalityType& type, bool isTracePair = false);
	void    PrepareForPairMode (Character* pair_chr,const char* pairLink,FatalityParams::TFatalityType pair_type);

	virtual Character* GetPairCharacter () {return pair;};
	virtual void SetPairCharacter (Character* pair_chr) { pair = pair_chr; };	

	int is_attached;
	Character* master;
	Character* slave;

	float aniSpeed;	

	bool bonusWasDroped;

	bool allow_orient;

	void SetAllowOrient(bool b_allow_orient) { allow_orient = b_allow_orient; };

	void SetBonusWasDroped( bool wasdroped) { bonusWasDroped = wasdroped;};	
	bool WasAttacked(Character* ch);

	void OrientBodyPart(bool isHead);

	static bool allow_animslowdown;

	void SetAnimSlowDown(bool animslowdown);
	
	float time_scale;
	void  SetTimeScale(float timescale) {time_scale = timescale;};
	float GetTimeScale() { return time_scale; };

	Vector spawn_point;

	void   SetSpawnPoint(Vector pt);
	Vector GetSpawnPoint();

	bool   splahdamage_enabled;
	float  next_splahdamage;
	bool   IsSplahdamage() { return splahdamage_enabled; };
	void   EnableSplahdamage(bool enable) { next_splahdamage = -1.0f; splahdamage_enabled = enable; };

	float  fRageAttackPow;
	float  fRageHitPow;

	bool   bRageMode;
	bool   IsRageMode() { return bRageMode; };
	void   EnableRageMode(bool enable) { bRageMode = enable; };
	
	void   EnableAutoZoom(bool enable);	

	void   SplashDamage(const char* locator, const char* particale,
						float radius,float damage, bool allowkick,const char* reaction,const char* deathLink = "Blow");

	float hp_mult;
	void  SetHPMultipler(float mult);

	float attack_mult;
	void  SetAttackMultipler(float mult) { attack_mult = mult; };	

	bool  attack_imune;
	void  SetAttackImune(bool imune) { attack_imune = imune; };	
	bool  IsAttackImune() { return attack_imune; };	

	void  EnableAutoAim(bool enable);
	bool  IsAutoAimEnabled();

#ifdef CHARACTER_STARFORCE
	template<class _T> inline _T UpdateDamageSpikes()
	{
		return _T(powf(chr.arbiter->GetTime().GetTimeFloat() / time2StartDamageSpikes[rand()%3], 4.0f));
	}
#endif

	void  UpdateAttacks(aiActionStack* actionStack);

	int   GetFightAct();
};

//Получить текущее значение HP
inline float CharacterLogic::GetHP()
{
	return HP;
}

//Получить текущее значение maxHP
inline float CharacterLogic::GetMaxHP()
{
	return maxHP * hp_mult;
}

//Получить текущее значение HP
inline float CharacterLogic::GetAbsoluteHP()
{
	return GetHP()/GetMaxHP();
}

//Актёр ли персонаж
inline bool CharacterLogic::IsActor() const
{
	return isActor;
}

//Жив или нет
inline bool CharacterLogic::IsDead()
{
	//if (GetState() == state_die || GetState() == state_dead) return true;

	return HP < 1.0f;
}

//Установить сторну персонажа
inline void CharacterLogic::SetSide(Side sd)
{
	side = sd;
};

//Получить сторну персонажа
inline CharacterLogic::Side CharacterLogic::GetSide()
{
	return side;
}

//Врежиме атаки персонаж или нет
inline bool CharacterLogic::IsAttack()
{
	return isAttack;
}

//В режиме парной анимации
inline bool CharacterLogic::IsPairMode()
{
	if (GetState() == state_pair) return true;

	return isPairMode;
}

#endif

