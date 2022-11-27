

#ifndef _CharacterItems_h_
#define _CharacterItems_h_


#include "..\Character.h"
#include "SwordTrail.h"
#include "..\..\utils\SafePointerEx.h"

//#define CharacterSwordDebug



struct BladeLine
{
	Vector s;
	Vector e;
};

struct droped_item_data
{
	Matrix transform;
	float alpha;
	Vector bMin;
	Vector bMax;
};

struct ItemData
{
	ItemData() : collision(_FL_, 32) {}
	~ItemData() {}

	ConstString id;			      //Идентификатор итема
	long locator;				  //Индекс локатора		
	TWeaponType weapon_type;      //тип оружия
	const char* model_name;       //Имя Модельки
	IGMXScene * model;			  //Моделька итема
	IGMXScene * model_part1;	  //Первая часть сломанной модельки
	const char* model_part1_name; //Имя Модельки
	IGMXScene * model_part2;	  //Вторая часть сломанной модельки
	const char* model_part2_name; //Имя Модельки

	SwordTrail * trail;			//След за оружием
	bool isActive;				//Клинок активен и наносит повреждения
	bool isSetBladeLocators;	//Установленны локаторы лезвия
	Vector bladeStart;			//Локатор начала лезвия
	Vector bladeEnd;			//Локатор окончания лезвия
	array<BladeLine> collision;	//Накопленный след за оружием
	//Vector pos[4];			//Четырёхугольник клинка (0 - начало, 1 конец, 2 предыдущий конец, 3 предыдущее начало)

	ConstString tip_id;		    //Идентификатор подсказки
	bool showFlare;
	ConstString attachedobject;
	const char* locatortoattache;
	MOSafePointer mobj;	
	Vector objectoffset;
	bool  visible;
	float itemLife;
	float fProbality;
	float time_lie;
	float armor_damage_absorb;
	int   drop_priority;

	MOSafePointerTypeEx<EffectTable> effectTable;
	bool  armor_mtl;

	droped_item_data drp_data;

	Color  color;
};

class CharacterItems
{
public:	

	struct Locator
	{
		ConstString name;		// Имя локатора
		long logicLocator;		// Ссылка на локатор на модельке
		IGMXScene * scene;		// FIX-ME возможно надо убрать эту сцену отсюда
		GMXHANDLE locator;		// Локатор на модельке
	};

	struct SwordDebug
	{
		Vector quadrangle[4];
		bool isBase;
	};

	struct TFlare
	{
		bool Show;
		float fTime;

		TFlare()		
		{
			Show = false;
			fTime = 0.0f;
		};
	};

	float fMaxFlareTime;
	TFlare Flare;

	char params[3][32];
	
	bool isAlwaysArmor;
	
	int armor_hp_drop;
	float armor_drop_prop;	

public:	

	CharacterItems(Character * character);
	~CharacterItems();

	float max_armor_damage_absorb;

public:	

	EffectTable* lastactive_effectTable;

	//Установить флаг динамического освещения
	void SetDynamicLightingFlag(bool isDynamicLighting);
	//Добавить итем
	bool AddItem(CharacterPattern::TItemDescr* descr);
	//Удалить итем
	long DelItems(const ConstString & id);
	//Добавить логический локатор
	void SetLogicLocator(const ConstString & logicName, const ConstString & locatorName);
	//Обновить состояние
	void Update(float dltTime);

	void Reset();

	//Нарисовать итемы
	void Draw(const Matrix & toWorld, const Color & userColor);
	//Нарнисовать шлейфы за итемами
	void DrawTrails(float dltTime, const Matrix & toWorld);
	//Нарнисовать фларики от предметов
	void DrawFlares(float dltTime, const Matrix & toWorld);
	//Получить индекс локатора итема
	long GetLocatorID(const ConstString & locatorName);
	//Запустить партикловую систему за локаторе итемов
	void StartParticlesOnItems(const ConstString & id, const char * itemLocatorName, const char * particlasName, bool isAttach, const Matrix & toWorld, float scale, bool normalize);
	//Изменить видимость шлейфа за итемом
	void SetTrailVisible(const ConstString & id, bool isVisible);
	//Изменить активность оружия
	void ActivateItem(const ConstString & id, bool isActive);
	//Расчёт повреждений от атак
	void AttackProcess(array<Character *> & characters);

	//Выкидование предметов
	void DropItems();
	bool DropItem(const ConstString & item_id, bool isBroken);
	bool HideItem(const ConstString & item_id);
	void DropArmor(bool drop_all);

	void DropArmor(float dmg, float dmg2);
	void ShowRandomArmor(const ConstString & id);

	//Дополнительное обновление следа от оружия (коллижен и визуализация)
	void AnimationFrameUpdate();

	bool FindLocatorinItems(const char* locatorName,Matrix & mtx);
	bool FindLocatorinItem(int index, const char* locatorName,Matrix & mtx);

	int FindItemIndex(const ConstString & item_id);

	float GetLastWeaponLive();
	void DamageWeapon(bool kill, float damage = 1.0f);
	void ShowWeapon(bool show,const ConstString & id);

	void GetWeaponTrail(Vector& start,Vector& end,const ConstString & wp_id);
	bool CheckHit(Character* c, Vector quadrangle[4]);

	float CalcDamage(float dmg);
	
	void SetAlwaysArmor(bool always_armor);
	bool IfArmorMtrlPresent();

	bool IfAttackWeapon();

	//Получить матрицу локатора
	bool GetLocatorMatrix(long index, Matrix & mtx, bool updateHierarchy = false);

	void ReserveElements(dword reserveCount);

protected:
	//Рекурсивное деление дистанции трейла
	void RecursiveSplitAttackQuadrangle(array<Character *> & characters, Vector quadrangle[4], dword dwLevel);	
	//Добавить или найти для использования геометрический локатор
	long AddLocator(const ConstString & locatorName);
	//Расчёт повреждений от атак
	bool AttackProcessForBlade(array<Character *> & characters, Vector pos[4]);

	

protected:

	Matrix lastDrawToWorld;

public:
	static bool isCharacterSwordDebug;
	array<ItemData> items;			//Предметы игрока

protected:

	array<Locator> locators;		//Используемые локаторы
	Character & chr;				//Персонаж которому принадлежим
	bool dynamicLighting;			//Флаг динамического освещения
#ifndef STOP_DEBUG
	array<SwordDebug> swDebug;		//Временные ректанглы
	array<BladeLine> swlDebug;		//Временные линии, пришеднии из анимации
#endif
};


#endif

