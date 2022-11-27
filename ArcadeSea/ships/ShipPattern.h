#pragma once
#include "..\..\Common_h\Mission\Mission.h"
#include "..\..\Common_h\gmx.h"

#include "HullPart.h"
#include "Mast.h"
#include "Yard.h"
#include "Rope.h"
#include "ShipSide.h"

// параметры корабля
struct ShipContent : public ShipPart::PatternParams
{
	// параметры человечка на палубе
	struct LittleMan
	{
		LittleMan() : groups(_FL_) {}

		Vector			pos;				// позиция на корабле
		Vector			angles;				// ориентация
		float			explFlyDir;
		float			explFlyLength;
		float			explFlySpeed;
		float			ballHitDist;
		ConstString		patternName;		//
		array<const char*> groups;
		const char*		defaultBehaviour;
		bool			show;
	};

	// параметры для лода
	struct ShipLod
	{
		const char* mainModelName;	// имя модели корабля
		const char* reflModelName;	// имя модели корабля для отражения
		const char* refrModelName;	// имя модели корабля для переломления

		//IGMXScene*	pMainModel;		// модель корабля
		//IGMXScene*	pReflModel;		// модель корабля для отражения
		//IGMXScene*	pRefrModel;		// модель корабля для переломления

		float fDist2Pow;			// расстояние с которого данный лод актуален (в квадрате, для облегчения сравнения по дальности)
	};

	// массив лодов (отсортирован по дальности. сортировка не проводится, должна в правильном порядке задаваться дизайнерами)
	array<ShipLod> m_Lods;
	float fShipLodFadeDist;
	float fShipLodFadeLength;

	array<HullPart::Params>		hull_;						// корпус
	ShipPart::DescrSFX			shipExplosionParticles_;
	const char*					shipExplosionSound_;
	//const char*					shipExplosionParticles_;	// партиклы взрыва корабля
	array<Rope::Params>			ropes_;						// веревки
	ShipSide					sides_[4];					// борта
	array<Mast::Params>			masts_;						// мачты
	array<IClothMeshBuilder*>	cookedSails_;				// сетки парусов
	array<LittleMan>			littleMen_;					// человечки на палубе
	const char*					m_pcShipPartWaterSplash;	// партикл падения в воду части корабля
	float						m_fShipPartWaterSplashStep;	// шаг партиклов (когда возникают много в линию на одной длинной палке)
	const char*					m_pcShipBoneWaterSplash;	// партикл попадания в воду косточки из анимации разрушения

	float						m_fDamageFromMastBroke;		// дамаг который получает корабль при поломке мачт

	long						nDamageHullRecurseLevel;	// глубина рекурсии для передачи повреждения при уничтожении части корпуса

	float			m_fSFXLimitationDistance;

	//////////////////////////////////////////////////////////////////////////
	// Характеристики корабля
	//////////////////////////////////////////////////////////////////////////
	string modelName_;				// имя модели корабля (специально в стринге - чтоб можно было сравнить при перечитывании параметров)
	//const char* m_reflModelName;	// имя модели корабля для отражения
	//const char* m_refrModelName;	// имя модели корабля для переломления
	Vector vCenterMass_;			// положение центра масс (локальная СК)
	Color m_cModelColor;			// цвет дополнительной окраски модельки

	Vector massBoxCenter_;			// центр бокса для распределения массы
	Vector massBoxBounds_;			// размеры бокса для распределения массы

	Vector boundsCenter_;			// центр коллижн-бокса
	Vector boundsSize_;				// размеры коллижн бокса

	float fMass_;					// масса
	float fWaterLineHeight_;		// уровень ватерлинии
	float fForwardSlopeForce_;		// скорость крена
	float fBackwardSlopeForce_;		// скорость возврата из крена
	float fBackAlongSlopeScale_;	// скорость возврата из крена
	float fLinearFrictionForce_;	// линейное трение 
	float fAngularFrictionForce_;	// угловое трение 
	float fFloatForce_;				// сила возврата на глубину плавания
	float fWaveness_;				// коэфициент волнения

	const char* pcBreakAnimation;	// анимация взрыва

	bool bUsedByStatistics;			// смерть корабля учитывается в статистике
	bool bIsBoat;					// смерть корабля учитывается в ачивменте как лодка

	// sound params
	enum SoundEnum
	{
		se_hull_destroy,

		se_ship_not_moved,
		se_ship_moved,
		se_ship_fire,
		se_ship_touched,
		se_ship_knock,

		se_sail_damage,
		se_sail_fire,

		se_ship_hit,

		se_quantity
	};
	const char* pSound_[se_quantity];
	float fShipMoveFadeTime_; // время фейда для звука движения

	MissionTrigger cannonFireBase;
	MissionTrigger cannonFireMight;

	struct WeaponLight
	{
		ConstString pcLightObjID;
		float fLifeTime;
	};
	array<WeaponLight> aWeaponLights;
	float fExcludeLightRadius;
	float fLightOffset;

	ShipContent() : PatternParams(),
		ropes_(_FL_),
		masts_(_FL_),
		hull_(_FL_),
		cookedSails_(_FL_),
		littleMen_(_FL_),
		aWeaponLights(_FL_),
		m_Lods(_FL_)
	{
		//shipExplosionParticles_ = "";
		m_pcShipPartWaterSplash = "";
		m_fShipPartWaterSplashStep = 0.f;
//		m_reflModelName = "";
//		m_refrModelName = "";

		fShipLodFadeDist = 500.f;
		fShipLodFadeLength = 100.f;

		m_fSFXLimitationDistance = 100.f;

		fMass_ = 0.f;
		fWaterLineHeight_ = 0.f;
		fForwardSlopeForce_ = 0.f;
		fBackwardSlopeForce_ = 0.f;
		fBackAlongSlopeScale_ = 0.f;
		fLinearFrictionForce_ = 0.f;
		fAngularFrictionForce_ = 0.f;
		fFloatForce_ = 0.f;
		fWaveness_ = 0.f;

		// sound params
		for( int i=0; i<se_quantity; i++ )
			pSound_[i] = "";
		fShipMoveFadeTime_ = 0.f;

		fExcludeLightRadius = 0.f;
		fLightOffset = 0.f;
	}
};


// шаблон миссионного объекта "Корабль"
class Ship;
class ShipPattern :	public MissionObject
{
	ShipContent				pattern_;		//
	Vector					vPos_;			// положение шаблона

	IGMXScene*				mainModel_;

	//////////////////////////////////////////////////////////////////////////
	// Флаги визуализации в редакторе
	//////////////////////////////////////////////////////////////////////////
	bool bDrawHull_;
	bool bDrawMasts_;
	bool bDrawRopes_;
	bool bDrawSides_;
	bool bDrawShootSmokeSFX_;
	bool m_bDrawSailors;
	bool m_bDrawBurnPoints;

	long m_nEditLittleManIndex;

	void ReadMOPs(MOPReader & reader);
	void VisualizePhysForm(const PhysForm& form);
	bool GetRopePointPosition(const char* name, Vector & pos);
	void BuildSail(const Sail::Params& params, unsigned int index);
	bool CheckSailChange(array<Mast::Params> & oldMasts, array<Mast::Params> & newMasts);

	// Реестр кораблей, использующих шаблон
	array<Ship*> registry_;

	bool ReadMOPString(MOPReader & reader,string & pcStr);

	inline void ChangeFlag_Float(float fNewVal, float & fOldVal, bool & bChangeFlag)
	{
		if( !bChangeFlag )
			bChangeFlag = fabs(fNewVal - fOldVal) > 0.01f;
		fOldVal = fNewVal;
	}

	void ReadLodsParams(MOPReader & reader);

public:
	ShipPattern(void);
	virtual ~ShipPattern(void);

	//////////////////////////////////////////////////////////////////////////
	// Взаимодействие с реальными кораблями
	//////////////////////////////////////////////////////////////////////////
	enum Notification { ParamChanged, PatternRemoved };
	// копирует параметры из структуры параметров шаблона-корабля в 
	// структуру парамеров реального корабля
	void CopyContent(ShipContent & other) const { other = pattern_; }
	// регистрирует корабль в шаблоне
	void RegisterShipObject(Ship*);
	// отменяет регистрацию корабля в шаблоне
	void UnregisterShipObject(Ship*);
	// оповещает все зарегистрированные корабли об изменениях шаблона
	void NotifyShipObjects(Notification) const;
    

	///////////////////////////////////////////////
	// Реализация поведения миссионного объекта
	///////////////////////////////////////////////

	// создание объекта в игровом режиме
	virtual bool Create(MOPReader & reader);
	// инициализация 
	virtual bool Init();

	// создание в режиме редактора
	virtual bool EditMode_Create(MOPReader & reader);
	// обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	// бокс выделение
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

	virtual Matrix & GetMatrix(Matrix & mtx);
	
	void _cdecl Draw(float fDeltaTime, long level);

	MO_IS_FUNCTION(ShipPattern, MissionObject);

};
