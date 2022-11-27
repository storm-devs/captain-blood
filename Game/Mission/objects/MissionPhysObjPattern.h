//============================================================================================
// Spirenkov Maxim, 2005
//============================================================================================
// Mission objects
//============================================================================================
// MissionPhysObjPattern
//============================================================================================

#ifndef _MissionPhysObjPattern_h_
#define _MissionPhysObjPattern_h_

#include "MissionGeometry.h"

class MissionPhysObject;

class MissionPhysObjPattern : public MissionObject
{
	friend class MissionPhysObject;

	struct CollisionBox
	{
		Matrix mtx;
		Vector size05;
	};

	struct CollisionCapsule
	{
		Matrix mtx;
		float height;
		float radius;
		Vector size05;
	};

	struct Effect
	{
		Effect();

		ConstString name;
		Matrix mtx;
	};

	struct Sfx
	{
		ConstString particles;
		ConstString sound;
	};

	enum FadeoutEffect
	{
		fe_none,
		fe_hide,
		fe_static,
	};

	struct FadeoutElement
	{
		ConstString name;
		FadeoutEffect id;
	};

	struct MaterialDescription
	{
		ConstString name;
		PhysTriangleMaterialID id;
	};

	struct PatternObject
	{
		friend class MissionPhysObjPattern;

		PatternObject();


		Matrix mtx;
		ConstString modelName;
		array<CollisionBox> boxes;
		array<CollisionCapsule> capsules;
		array<Effect> sounds;
		array<Effect> particles;
		ConstString receiveBoom;
		ConstString waterParticle;
		ConstString waterSound;
		//const char * id;
		float dencity;
		Vector centerOfMass;
		bool isStatic;
		bool isShow;
		bool isSoloDraw;
		FadeoutEffect fe;
		Vector impulsePosition;
		Vector impulse;
		bool isAddImpulse;
		//long sourceIndex;
		Vector min, max;
		ConstString tipId;
		Vector tipPosition;
		ConstString bonusesTable;
	private:
		IGMXScene * model;
	};

	struct CacheScene
	{
		IGMXScene * model;
		dword use;
	};

	struct Locator
	{
		Matrix mtx;
		long objectIndex;
		ConstString name;		
	};

public:
	MissionPhysObjPattern();
	~MissionPhysObjPattern();

	MO_IS_FUNCTION(MissionPhysObjPattern, MissionObject);

private:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	void ReadModelParams(long index, MOPReader & reader);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);

	//Нарисовать модельку
	void _cdecl Draw(float dltTime, long level);
	//Прекешировать модельки заданные в патерне
	void CacheModels();

	//Проиграть звук столкновения с данным материалом
	void PlayCollisionEffect(long mtlId, const Vector & pos, float impulse2);

private:
	array<PatternObject> objects;
	array<Locator> locators;
	float hp;
	float activeTime;
	PhysTriangleMaterialID materialId;
	DamageReceiver::DamageMultiplier damageModifier;
	long soldObjectsCount;
	Vector buildPosition;	
	bool isPickup;
	bool showOnlySelected;
	bool showSolid;
	bool showBroken;
	bool haveSoloDraw;
	bool showObjectCenter;
	bool showSoundsPoints;
	bool showParticlesPoints;
	bool showBrokeImpulses;
	bool showCenterOfMass;
	bool showTipsPosition;
	bool showLocators;
	bool dynamicLighting;						//Динамическое освещение
	bool shadowCast;							//Отбрасывает тень
	bool shadowReceive;							//Принимает тень
	bool seaReflection;							//Отражается в море	
	long level;									//Уровень отрисовки
	float hideDistanceMin2;						//Дистанция скрывания физического объекта
	float hideDistanceK2;						//Коэфициент дистанции скрывания
	array<MissionPhysObject *> regObjects;
	array<CacheScene> scenes;
	Sfx hitSword;
	Sfx hitBullet;
	Sfx hitBomb;
	Sfx hitShooter;
	Sfx hitCannon;
	Sfx hitFlame;
	Sfx hitToMaterial[8];
	Vector viewMinBox, viewMaxBox;	
	

public:
	static FadeoutElement fadeoutElements[];
	static MaterialDescription materialDescs[];
};

#endif

