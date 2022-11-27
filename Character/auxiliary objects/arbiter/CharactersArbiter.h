//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// CharactersArbiter
//===========================================================================================================================

#ifndef _CharactersArbiter_h_
#define _CharactersArbiter_h_


#include "..\..\..\Common_h\mission.h"
#include "..\..\..\Common_h\ICharactersArbiter.h"
#include "..\..\..\Common_h\ICharacter.h"
#include "..\..\..\Common_h\ITips.h"
#include "..\..\..\Common_h\IFly.h"
#include "..\..\..\Common_h\IStainManager.h"
#include "..\..\..\Common_h\IOceans.h"
#include "CharactersTime.h"

class Character;
class IExplosionPatch;
struct ItemData;
struct droped_item_data;
enum TWeaponType;
enum HitType;


struct CharacterFind
{
	Character * chr;				//Указатель на персонажа
	Vector      dv;				    //Направление на персонажа
	//Vector      dir;				//Нормализированное направление на персонажа
	float       dist_pow;				//дистанции
};


class CharactersArbiter : public ICharactersArbiter
{
private:
	static bool charStatsShow;

	ICoreStorageFloat * statKilledTotal;
	ICoreStorageFloat * statKilledPlayer;
	ICoreStorageFloat * statKilledDismember;
	ICoreStorageFloat * statKilledFatalities;
	ICoreStorageFloat * statKilledShot;
	ICoreStorageFloat * statKilledBombed;
	ICoreStorageFloat * statKilledByItems;
	ICoreStorageFloat * statKilledShootered;

	ICoreStorageFloat * statChestsTotal;
	ICoreStorageFloat * statChestsOpened;

	ICoreStorageFloat * statMoney;
	ICoreStorageFloat * statShipsSunk;

	// скорость атаки врагов для разных режимов сложности
	ICoreStorageFloat * enemyAttackSpeedModifier;

	ICoreStorageFloat * playerIdleTime;

	IExplosionPatch * explosionPatch;

	Character * m_player;

	array<Character*> deadBodies;

	int curFrame;

	bool SetStainManager();

public:
	struct TDropedItemInitData
	{		
		const char* model_name;
		const char* broken_part1;
		const char* broken_part2;
		const char* broken_taken_player;

		float item_life;
		ConstString tip_id;	

		TWeaponType weapon_type_broken;
	};

	struct Vertex
	{
		Vector p;
		dword color;
		float u, v;		
	};

	struct TAddGmRData
	{		
		IVBuffer* flareBuffer;
		IBaseTexture* flareTex1;
		IBaseTexture* flareTex2;
		IVariable* vPower;
		IVariable* vTex;

		IVBuffer * bulletBuffer;
		IBaseTexture * bulletTex;

		TAddGmRData()		
		{
			flareBuffer = null;
			flareTex1 = null;
			flareTex2 = null;			

			bulletBuffer = null;
			bulletTex = null;

			vPower = null;
			vTex = null;
		};
	};	
	

	struct TDropedItem
	{
		float fTimeLife;
		IGMXScene* model;

		Matrix physicPivot;
		IPhysBox* pPhysicBox;

		TWeaponType type;

		bool is_bomb;

		float flash;		

		TDropedItemInitData init_data;

		ITip* tip;		

		droped_item_data* drp_data;

		TDropedItem()
		{
			drp_data = null;
		}
	};

	CharactersTime time;

	IGMXScene* coin_model;
	IGMXScene* bomb_model;

	IStainManager * stainManager;

	#define TOTAL_DROPPED_ITEMS 128
	#define NUM_DROPPED_ITEMS 64
	TDropedItem droped_items[TOTAL_DROPPED_ITEMS];

	struct DroppedWeapon
	{
		int idx;
		float timeLife;
	};

	static bool DroppedWeaponsSortTimeLife(const DroppedWeapon & i1, const DroppedWeapon & i2)
	{
		return i1.timeLife > i2.timeLife;
	}
	bool IsItemDroppableWeapon(TWeaponType weaponType);
	float AdjustTimeLie(ItemData * item, int index, int max_weapons);

	array<DroppedWeapon>	droppedWeapons;

	int item_index;

	struct TCharDist
	{
		Character* chr;
		float      dist_pow;
		Vector     dv;

		TCharDist()
		{
			chr = NULL;
			dist_pow = 0.0f;
			dv = 0.0f;
		};
	};

	/*struct TCharDistTable
	{
		Character* owner;

		array<TCharDist> table;

		TCharDistTable(): table(_FL_,64)
		{
			owner = NULL;
			table.DelAll();
		};		
	};*/

	bool showMap;
	bool showWayPointsOnMap;
	bool showCPtOnMap;
	bool showAIZnOnMap;
	char cTmpBuffer[256];
	
	bool showWayPoints;
	bool showCPt;
	bool showAIZn;

	bool show_beacon;

	float map_scale;
	float map_angle;
	void  DrawSprite(Vector p, float sz, dword color);
	void  DrawLine(Vector v1, Vector v2, dword color);	

	bool  bHairDebug;

	ICoreStorageFloat* npc_hp;
	ICoreStorageFloat* npc_damage;
	ICoreStorageFloat* player_live;
	ICoreStorageFloat* gsGodMode;

	ConstString level_difficulty_name;
	ICoreStorageFloat* gsFatalityA;
	ICoreStorageFloat* gsFatalityB;
	ICoreStorageFloat* gsFatalityC;
	ICoreStorageFloat* gsFatalityD;

	ICoreStorageFloat * GetEnemyAttackSpeedModifier() { return enemyAttackSpeedModifier; }

	ITipsManager * tip_manager;

	bool IsFatalityAllowed(int num);
	void SetFatalityAllowe(int num,float allow);

	char* CorrectStateId(const char* id);

	//void StatsMoneySet(float gold);
	void StatsMoneyAdd(float add);
	void StatsFatalitiesAdd();
	void StatsDismemberAdd();
	void StatsOpenedChestAdd(float add);
	void StatsTotalChestAdd();

	int   index2d;

	struct dmg_reciver
	{
		array<DamageReceiver*> recivers;

		dmg_reciver():recivers(_FL_,32)
		{
		};
	};

	array<dmg_reciver> d_recivers;

	bool AnimDebugOn;

	bool isMultiplayer;

public:
	TAddGmRData gmRData;

	MOSafePointerType<IFlyManager> fly_manager;

	CharactersArbiter();
	~CharactersArbiter();	

	const CharactersTime & GetTime() { return time; }

	bool IsMultiplayer() const { return isMultiplayer; }

	bool IsSphereInFrustrum(Vector pos, float radius);
	bool IsPointInFrustum(const Vector & pnt);

	bool IsSlowmoReady();

	void ExplosionPatch(const Vector & pos, const ConstString & type);

	__inline int GetCurFrame() { return curFrame; }

//--------------------------------------------------------------------------------------------
//Работа арбитра
//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

	virtual void PostCreate();

	// Вызывается на конце экзекутов, для того чтобы поставить флажок обновления опроса "state" у анимации персов
	void _cdecl EndWork(float dltTime, long level);
	// Работа персонажа
	void _cdecl Work(float dltTime, long level);
	// Обработка мертвых тел персонажей
	void _cdecl DeadBodiesWork(float dltTime, long level);
	//Отладочная информация
	void _cdecl DrawMap(float dltTime, long level);

	void _cdecl AnimDebug(float dltTime, long level);

	void _cdecl RealizeDebug(float dltTime, long level);

#ifdef CHARACTER_STARFORCE
	dword dwProtectValue2;
	ICoreStorageFloat * timeInSecFromStart;

	template<class _T> inline _T GuardingValueDamage(int ar)
	{
		return _T(guardingValueDamage[ar]);
	}

	struct SpikeStar
	{
		IParticleSystem * ps;
		Vector pos;
		float time;
		float scale;
		float speed;
	};

	array<SpikeStar> spikeStars;
	dword dwProtectValue;
	__forceinline IRender & GetRender() 
	{ 
#ifdef CHARACTER_STARFORCE
		if ((time.GetTime() > 250.0 || curFrame > 10000) && rand()%1000 == 501)
			return *pOceans->GetVRender();
#endif

		return Render(); 
	}
#else
	__forceinline IRender & GetRender() { return Render(); }
#endif

//--------------------------------------------------------------------------------------------
//Управление персонажами
//--------------------------------------------------------------------------------------------
public:
	// эвент смерти персонажа
	void Death(dword source, Character & victim, Character * killer);

	// добавляет значение в текущую миссионную статистику
	void StatAdd(ICoreStorageFloat & var, float value);

	//Активировался-деактивировался персонаж
	void Activate(Character * chr, bool isActive);

	//Найти персонажей в заданном секторе и дальности	
	void Find(Character & chr, float rmin, float rmax, bool CheckHeight = false, float amin = 0.0f, float amax = 4*PI, float backMove = 0.0f, float maxHeightDiff = 1000.0f);

	//Найти персонажей в заданном радиусе
	void FindCircle(Character & chr, float fRadius);

	//Найти персонажей в заданном радиусе
	void FindCircle(const Vector& pos, float fRadius);

	//Найденные персонажи
	array<CharacterFind> find;		

	//Нарисовать сектор поиска
	void FindDraw(Character & chr, float rmin, float rmax, float amin, float amax, float backMove);

	//Получить список активных персонажей
	const array<Character*> & GetActiveCharacters();

	void AttackPhyObj(MissionObject* from, const Vector& pos, float radius, float damage, MissionObject* filter = null);
	bool AttackPhyObj(MissionObject* attacker, dword source, float damage,Vector from, Vector to);

	//Взорвать бомбу
	void Boom(MissionObject * offender, dword source, const Vector & pos, float radius, float damage, float boom_power, MissionObject * boomCreator = null);

	//Взорвать бомбу
	void BlowSelf(MissionObject * from, const Vector & pos, float radius, float damage, float boom_power);

	//Нанести SplashDamage
	void SplashDamage(dword source, MissionObject* from, const Vector & pos, float radius, float MAXDamage, bool orient_to_attacker,
					  const char* particlename = null, bool DamageAlly = true,float pow = 1.0f,bool allowkick = true,
					  const char* reaction = null, const char* deathLink = null, MissionObject* filter = null);

	void AddDeadBody(Character & chr);
	void DelDeadBody(Character & chr);
	static bool SortDeadBodies(Character * const & c1, Character * const & c2);

	#ifndef NO_CONSOLE

	void Action_ShowDebugInfo(int type);
	void Action_SetDebugInfo(int type);
	void Action_ShowControllerInfo(bool show);
	void Action_ShowMap(bool show);
	bool Action_SetCharFilter(const ConsoleStack & params);
	void Action_DebugInfoChanged(const ConsoleStack & params,int type);

	void _cdecl Console_CharStats(const ConsoleStack & params);
	void _cdecl Console_CharID(const ConsoleStack & params);
	void _cdecl Console_CharAnim(const ConsoleStack & params);	
	void _cdecl Console_CharRenderParams(const ConsoleStack & params);
	void _cdecl Console_CharHP(const ConsoleStack & params);
	void _cdecl Console_CharAI(const ConsoleStack & params);
	void _cdecl Console_CharItems(const ConsoleStack & params);
	void _cdecl Console_CharActor(const ConsoleStack & params);
	void _cdecl Console_CharHideInfo(const ConsoleStack & params);
	
	void _cdecl Console_CharSetFilter(const ConsoleStack & params);	

	void _cdecl Console_SwordDebug(const ConsoleStack & params);
	void _cdecl Console_AnimDebug(const ConsoleStack & params);

	void _cdecl Console_ShowMap(const ConsoleStack & params);
	void _cdecl Console_ShowWaysOnMap(const ConsoleStack & params);
	void _cdecl Console_ShowCPtOnMap(const ConsoleStack & params);
	void _cdecl Console_ShowAIZnOnMap(const ConsoleStack & params);
	
	void _cdecl Console_ShowWays(const ConsoleStack & params);
	void _cdecl Console_ShowCPt(const ConsoleStack & params);	
	void _cdecl Console_ShowAIZn(const ConsoleStack & params);

	void _cdecl Console_ShowBeacon(const ConsoleStack & params);

	void _cdecl Console_GodMode(const ConsoleStack & params);	
	void _cdecl Console_GiveMoney(const ConsoleStack & params);
	void _cdecl Console_GiveAmmo(const ConsoleStack & params);
	void _cdecl Console_GiveAll(const ConsoleStack & params);

	void _cdecl Console_ShowPairMiniGame(const ConsoleStack & params);

	void _cdecl Console_CharDebug(const ConsoleStack & params);

	void _cdecl Console_CharDraw(const ConsoleStack & params);

	void _cdecl Console_CharHairDebug(const ConsoleStack & params);
	
	void _cdecl Console_ShowAchievements(const ConsoleStack & params);
	void _cdecl Console_WatchAchieve(const ConsoleStack & params);	
	void _cdecl Console_FreezeRagdolls(const ConsoleStack & params);

	#endif

	bool IsCharacterDebug() { return bCharacterDebug; };
	bool CheckNumAttckers(Character* target, int MaxAlowed, char* Type);

	//array<TCharDistTable> distTable;
	float GetPowDistance(Character* from, Character* to);
	__forceinline Vector GetVector(Character * from, Character * to);

	bool NeedCalcDistances;
	void CalcDistances();	

#ifdef CHARACTER_STARFORCE
	ICoreStorageFloat * gameTicks;
#endif

	void DropeItem(ItemData* item,Matrix& world_mat,bool isBroken);
	void DropeItemImpl(ItemData* item,Matrix& world_mat,int item_part);
	void TakeItem(int item_index);
	int  FindClosesetItem(Character* from, float dist = 1.35f);
	bool AllowDropItem(bool is_item_armor);

	void _cdecl ReciveShadowDropedItems(const char * group, MissionObject * sender);

	void _cdecl DrawDropedItems(float dltTime, long level);
	
	void _cdecl DrawBeacon(float dltTime, long level);

	TWeaponType GetDropedWpType(int index);
	TDropedItemInitData* GetDropedWpItemData(int index);	

	bool  IsActiveCharacter(MissionObject* mo);

	bool  IfAnyCharacer(Character* chr,const Vector& pos,float radius,Vector &char_pos);
	bool  IfEnemyCharacer(Character* chr,float radius,bool ignoreNpc = false);
	bool  IsEnemyStatistAllive(Character* chr);

	void  DestroyAllWeapon();
	
	void  DrawFlare(int tex_index,Matrix& mat, float power);
	void  DrawBulletTrace(Matrix& mat, float power);
	void  DrawCoin(int index, Matrix& mat,float alpha);
	void  DrawBomb(Matrix& mat);

	void  PrintTextShadowed(const Vector& pos, int line, DWORD color, const char * pFormatString, ...);
	void  PrintTextShadowed(DWORD color, const char * pFormatString, ...);

	void  AttackChrItems(Character* chr);
	char  charFilter[128];
	int   debugInfoType;

	Character* GetPlayer();
	const ConstString & GetDifficulty();

	bool  IsHairDebug() { return bHairDebug; };

	void  SwitchGodMode();
//--------------------------------------------------------------------------------------------
protected:

#ifdef CHARACTER_STARFORCE
	int nSpikeValues;
	float guardCloud;
	array<Vector> spikeBoomz;

	__forceinline void SetSpikeData(dword protectValue);
#endif

	Vector spikeDamagePos;

	dword dwCurrentCharIndex;
	array<Character*> characters;				//Активные персонажи
	float tickTime;								//Время с последнего тика для контроллеров

	bool bCharacterDebug;	

	float guardingValueDamage[128];

	array<Character*> all_characters;

	IOcean3Service * pOceans;

	void _cdecl SpikeBoom();
};

#endif
