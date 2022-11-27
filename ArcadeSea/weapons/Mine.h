#pragma once
#include "weapon.h"

class IPhysRigidBody;

class MineDamageDetector : public DamageReceiver
{
	struct MineDescr
	{
		float r;
		Vector pos;
		bool bAttacked;
		bool bActive;
	};

public:
	MineDamageDetector();
	virtual ~MineDamageDetector();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);

	//
	MO_IS_FUNCTION(MineDamageDetector, DamageReceiver);

	virtual Matrix & GetMatrix(Matrix & mtx) {return (mtx = m_transform);}

	virtual bool IsDead() { return !IsActive(); }
	virtual bool IsShow() { return IsActive(); }

	//Воздействовать на объект сферой
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius);
	//Воздействовать на объект линией
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to);
	//Воздействовать на объект выпуклым чехырёхугольником
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4]);

	long AddMine(float fMineRadius, const Vector& pos);
	void DelMine(long nMine);
	void ChangeMinePos(long nMine, const Vector& pos);
	bool IsMineAttacked(long nMine);
//--------------------------------------------------------------------------------------------
private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);
	// дебажная отрисовка
	void _cdecl DebugDraw(float dltTime, long level);

	Matrix	m_transform;

	IMissionQTObject* m_pFinder;
	bool m_bUpdateFinder;

	array<MineDescr> m_mines;
};


class Mine : public Weapon
{
	// эмиттер
	long				minesCount_;
	float				dropInterval_;
	float				killRadius_;			// радиус поражения
	float				triggerOffRadius_;		// радиус срабатывания

	bool				isOnboard_;				// набортная/обычная
	float				timer_;
	bool				useTimer_;
	bool				hostDamage_;

	float				reloadTime_;

	float				AttractMaxRadius_;
	float				AttractMinRadius_;
	float				AttractSpeed_;

	MOSafePointerType<MineDamageDetector> m_DamageDetector;

	// экземпляр
	struct MineItem
	{
		MineItem()
		{
			active_ = false;
			mineActor_ = NULL;
			curTimer_ = 0.0f;
			nMineDamageDetector = -1;
		}
		float				curTimer_;
		IPhysRigidBody*		mineActor_;				// актер для мины
		bool				active_;
		long				nMineDamageDetector;
	};
	array<MineItem>			items_;
	
	// инициализация
	void Init(MineItem& item);
	// проверка условия срабатывания
	bool CheckTrigger(MineItem& item);
	// нанести дамаг
	void ApplyDamage(MineItem& item);
	// взорватся
	void Explode(MineItem& item);
	// притянуться к ближайшему врагу
	void AttractToEnemy(MineItem& item);

	// принять инспектора
	virtual void Accept(IWeaponInspector & visitor) { visitor.Visit(*this); }

	// найти стартовую позицию для мины ( чтобы не было коллизии на старте)
	Matrix FindStartMatrix( const Matrix& mtx, const Vector& offset );

	bool CheckAcceptedObjType(MissionObject* mo);

public:
	Mine();
	Mine(const Mine& other);
	Mine& operator=(const Mine& other);
	virtual ~Mine(void);

	virtual float GetReadyFactor()
	{
		if( reloadTime_ <= 0.f ) return 1.f;
		return reloadTime_<dropInterval_ ? (1.f - reloadTime_/dropInterval_) : 0.f;
	}

	// выстрелить - скинуть мину в указанную точку
	virtual bool Fire(const WeaponTargetZone& wtz, float fDamageMultiply);
	// логика работы мины
	virtual void Simulate(float deltaTime);
	// отрисовка
	virtual void Draw(float deltaTime);
	// создание копии
	virtual Weapon* Clone() const { return NEW Mine(*this); }

	virtual WeaponType GetType() {return Weapon::WeaponType_mine;}

	// отрисовка информации в редакторе
	virtual void DrawEditorFeatures();
	virtual void DrawDebugInfo() {}

	// Get/Set
	void SetTriggerRadius(float radius);
	float GetTriggerRadius() const { return triggerOffRadius_; }

	void SetKillRadius(float radius) { killRadius_ = radius; }
	float GetKillRadius() const { return killRadius_; }

	void SetOnboard(bool isOnboard) { isOnboard_ = isOnboard; }
	bool GetOnboard() const { return isOnboard_; }

	void SetMinesCount(long count) { minesCount_ = count; }
	void SetDropInterval(float interval) { dropInterval_ = interval; }

	void SetTimer(float timer) { timer_ = timer; }
	void EnableTimer(bool enable) { useTimer_ = enable; }
	void EnableHostDamage(bool enable) { hostDamage_ = enable; }

	void SetAttractMaxRadius(float radius) {AttractMaxRadius_ = radius;}
	float GetAttractMaxRadius() {return AttractMaxRadius_;}

	void SetAttractMinRadius(float radius) {AttractMinRadius_ = radius;}
	float GetAttractMinRadius() {return AttractMinRadius_;}

	void SetAttractSpeed(float fSpeed) {AttractSpeed_ = fSpeed;}
	float GetAttractSpeed() {return AttractSpeed_;}
};
