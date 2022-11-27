
#ifndef _CharacterPattern_h_
#define _CharacterPattern_h_


#include "..\..\Common_h\mission.h"
#include "..\auxiliary objects\BasePattern\BasePattern.h"

#define CHARACTER_PATTERN_GROUP "CHARACTER_PATTERN"

#include "Components\BodyParts\CharacterBlender.h"

enum HitType
{
	htds_item = 100000,
	htds_force_dword = 0x7FFFFFFF
};

enum TWeaponType
{
	wp_undropable = 0,
	wp_sword1 = 1,
	wp_sword2 = 2,
	wp_sword3 = 3,
	wp_axe1 = 4,
	wp_axe2 = 5,
	wp_axe3 = 6,
	wp_twohanded1 = 7,
	wp_twohanded2 = 8,
	wp_twohanded3 = 9,
	wp_rifle = 10,
	wp_rapire = 11,
	wp_gun = 12,
	wp_armor = 13,
	wp_bomb = 14,
	wp_box = 15,
	wp_bombardier_body = 16,
};

class CharacterPattern : public BasePattern
{	
public:
	
	//Модельки
	const char* modelName;
	const char* armorName;
	const char* ragdollName;
	const char* shadowName;

	ILoadBuffer * rdData;

	//Анимацмя
	const char* animationName;

	//Физические парметры	
	float fRad;
	float fHeight;

	//парметры AI		
	ConstString aiType;
	ConstString side;	

	float hp;
	ConstString hpBar;

	bool bShotImmuneWhenRoll;

	float fBloodDropDamage;

	float fHitDmgMultiplayer;
	float fBombDmgMultiplayer;
	float fShootDmgMultiplayer;
	float fRageDmgMultiplayer;

	float attackSpeedModifier;

	float block_cooldown;

	float fBlockProbabilityPerHit, fBlockProbabilityPerShoot;

	float fTotalDmgMultiplier;

	float fCointAltetude;	

	float fHeadRotateLimit;
	float fHeadRotateOffset;

	ConstString step_effectTable;
	ConstString shoot_effectTable;

	ConstString sword_effectTable;

	bool bShowCoinED;

	ConstString bonusTable;	

	float max_armor_damage_absorb;

	ConstString bombExplosionPattern;
	MOSafePointer bombExplosionMO;

	struct TLogicLoactor
	{
		ConstString id;		
		ConstString locator;
	};

	array<TLogicLoactor> logic_loactors;
	

	struct TItemDescr
	{
		ConstString id;
		const char* modelName;
		const char* boneName;
		const char* broken_part1;
		const char* broken_part2;		
		ConstString locatorName;		
		ConstString tip_id;		
		ConstString effectTable;
		bool  armor_mtl;
		TWeaponType weapon_type;		
		float armor_damage_absorb;
		int   drop_priority;
		float fProbality;
		float time_lie;
		float itemLife;	
		bool  useTrail;
		const char * uniqTexture;
		const char * trailTechnique;
		float trailPower;
		bool showFlare;
		ConstString attachedobject;
		const char* locatortoattache;
		Vector objectoffset;		
	};

	array<TItemDescr> item_descr;
	
	struct armorset
	{
		array<ConstString> id_armors;

		armorset(): id_armors(_FL_)
		{
		};
	};

	array<armorset> armorsets;	

	int armor_hp_drop;
	int armor_drop_prop;	

	//Параметры рендера
	float fLODDist;
	bool  dynamicLighting;
	bool  shadowCast;
	bool  shadowReceive;
	float fDistShadow;
	bool  seaReflection;
	float fDistReflect;
	bool  seaRefraction;
	float fDistRefract;	

	ConstString FatalityParams;
	ConstString AttackParams;

	struct TPartOfMe
	{		
		ConstString id;
		const char* modelName;
		ConstString locator;
		bool  isBox;
	};

	array<TPartOfMe> body_parts;
	
	ConstString cHairParams;	

	CharacterPattern();
	virtual ~CharacterPattern();
	
	virtual bool EditMode_Update(MOPReader & reader);

	MO_IS_FUNCTION(CharacterPattern, MissionObject);

	void PostCreate();

	// Загрузить данные регдолла, если они еще не загружены
	void LoadRagdoll();

	static const char * comment;
};

TWeaponType GetWeaponType(const char* type);


#endif


