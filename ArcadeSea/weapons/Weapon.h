//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\..\Common_h\Mission\Mission.h"
#include "WeaponInspector.h"

class TargetPoints;

struct IWeaponEvents
{
	virtual void OnShoot() = 0;
	virtual void OnHit() = 0;
	virtual void OnDraw() = 0;
};

struct WeaponTargetZone
{
	// центр стрельбы
	Vector target;
	// использование зоны
	bool bUseZone;
	// стрельбы книпелями
	bool bKnippels;

	struct IZone
	{
		float fPrioritet;
		Vector c;
		virtual bool GetCrossConeXZ(const Vector& vSrc, const Vector& vRay, float fAng, Vector* pVOut) const {return false;}
	};
	struct SphereZone : public IZone
	{
		float r;
		virtual bool GetCrossConeXZ(const Vector& vSrc, const Vector& vRay, float fAng, Vector* pVOut) const;
	};

	array<SphereZone> z;

	WeaponTargetZone(const Vector& t) : z(_FL_) {target=t; bUseZone=false; bKnippels=false;}
	WeaponTargetZone(const Vector& src, MissionObject* pMO, bool upperTarget);

	long GetTargetZoneByCone(const Vector& vSrc, const Vector& vRay, float fAng, Vector* pVOut) const;
};

#define MOP_TRAILPARAMS(s) \
	MOP_FLOATEX(s##" init size", 0.05f, 0.01f, 20.0f) \
	MOP_FLOATEX(s##" final size", 0.2f, 0.01f, 20.0f) \
	MOP_FLOATEX(s##" min fadeout time", 0.1f, 0.01f, 20.0f) \
	MOP_FLOATEX(s##" max fadeout time", 0.1f, 0.01f, 20.0f) \
	MOP_FLOATEX(s##" offset strenght", 1.0f, 0.1f, 5.0f) \
	MOP_COLORC(s##" color", 0x88ffffff, "Trail color")

struct WeaponTrailParams
{
	float fTraceInitSize;
	float fTraceFinalSize;
	float fTraceMinFadeoutTime;
	float fTraceMaxFadeoutTime;
	float fTraceOffsetStrength;
	Color cTraceColor;
};

class Weapon;
// базовый класс для шаблонов орудий
class WeaponPattern : public MissionObject
{
public:
	virtual Weapon* CloneWeapon() = 0;

	MO_IS_FUNCTION(WeaponPattern, MissionObject);

	static void ReadTrailParams(WeaponTrailParams& param, MOPReader & reader)
	{
		param.fTraceInitSize = reader.Float();
		param.fTraceFinalSize = reader.Float();
		param.fTraceMinFadeoutTime = reader.Float();
		param.fTraceMaxFadeoutTime = reader.Float();
		param.fTraceOffsetStrength = reader.Float();
		param.cTraceColor = reader.Colors();
	}
};

//////////////////////////////////////////////////////////////////////////
// Базовый класс орудий
//////////////////////////////////////////////////////////////////////////
class Weapon
{
public:
	enum WeaponType
	{
		WeaponType_cannon,
		WeaponType_flamethrower,
		WeaponType_mine,

		WeaponType_undefined
	};

protected:
	enum FireState { Ready, Reloading, Firing };

	struct Params
	{
		Params() :
		shootSound_(NULL),
		shootSoundDelay_(0.f),
		explosionSound_(NULL),
		sfx_(NULL),
		model_(NULL),
		pos_(0.f),
		dirAngle_(0.f),
		damage_(0.f),
		fireState_(Ready),
		bActive_(true),
		animBoneName_(NULL),
		owner_(NULL) {}

		const char*			shootSound_;
		float				shootSoundDelay_;
		const char*			explosionSound_;
		Vector				pos_;				// местоположение орудия
		float				dirAngle_;			// направление стрельбы (угол вокруг Y)
		float				damage_;			// наносимый урон
		const char*			sfx_;				// спецэффект выстрела (взрыва)
		IGMXScene			*model_;			// модель орудия
		Matrix				parentTransform_;	// трансформация объекта-владельца (корабля и т.д.)
		Matrix				transform_;			// конечная трансформация объекта (уже с учетом родительской)
		FireState			fireState_;
		bool				bActive_;
		const char*			animBoneName_;		// имя кости для анимации выстрела
		MissionObject*		owner_;
	} params_;
	
	
	// Конструктор
	Weapon();
	Weapon(const Weapon&);
	const Weapon& operator=(const Weapon&);
	
	void SetFireState(FireState state) { params_.fireState_ = state; }
	FireState GetFireState() const { return params_.fireState_; }
	
	void SetTransform(const Matrix& m) { params_.transform_ = m; }

private:

	// принять инспектора
	virtual void Accept(IWeaponInspector & visitor) = 0;

public:

	// Деструктор
	virtual ~Weapon(void);

	// можем ли стрелять
	bool CanFire() const { return params_.fireState_ == Ready; }

	virtual float GetReadyFactor() = 0;

	// выстрелить
	virtual bool Fire(const WeaponTargetZone& wtz, float fDamageMultiply) { return false; }
	virtual bool FireByPoint(TargetPoints* pTargPoints,long pointIdx) { return false; }
	// производит симуляцию стрельбы (расчет физики и т.д.)
	virtual void Simulate(float deltaTime) = 0;
	// расчет трансформации
	virtual const Matrix& GetTransform() const { return params_.transform_; }
	// отрисовка
	virtual void Draw(float deltaTime) {};
	// создание копии
	virtual Weapon* Clone() const = 0;

	virtual WeaponType GetType() {return WeaponType_undefined;}

	virtual void DrawEditorFeatures() = 0;

	virtual void DrawDebugInfo() = 0;

	// принять инспектора
	void AcceptVisitor(IWeaponInspector & visitor) { Accept(visitor); }

	// Get/Set методы для базовых параметров
	void SetPosition(const Vector& pos) { params_.pos_ = pos; }
	const Vector& GetPosition() const { return params_.pos_; }

	void SetDirectionAngle(float angle) { params_.dirAngle_ = angle; }
	float GetDirectionAngle() const { return params_.dirAngle_; }

	Vector GetDirection2D() const { return Vector(sin(params_.dirAngle_), 0, cos(params_.dirAngle_)); }

	virtual void SetModel(IGMXScene* model) { if (params_.model_) params_.model_->Release(); params_.model_ = model; if (params_.model_) params_.model_->AddRef(); }
	IGMXScene* GetModel() const { return params_.model_; }

	void SetDamage(float damage) { params_.damage_ = damage; }
	float GetDamage() const { return params_.damage_; }

	virtual void SetSFX(const char* sfx) { params_.sfx_ = sfx;}
	const char* GetSFX() const { return params_.sfx_; }

	void SetParentTransform(const Matrix& mtx) { params_.parentTransform_ = mtx; }
	const Matrix& GetParentTransform() const { return params_.parentTransform_; }

	void SetAnimBoneName(const char* name) { params_.animBoneName_ = name; }
	const char* GetAnimBoneName() const { return params_.animBoneName_; }

	void SetShootSound(const char* name,float fDelay) { params_.shootSound_ = name; params_.shootSoundDelay_ = fDelay; }
	const char* GetShootSound() const { return params_.shootSound_; }

	void SetExplosionSound(const char* name) { params_.explosionSound_ = name; }
	const char* GetExplosionSound() const { return params_.explosionSound_; }

	virtual void Activate(bool active) { params_.bActive_ = active; }
	bool IsActive() const { return params_.bActive_; }

	virtual void SetOwner(MissionObject* owner) { params_.owner_ = owner; }
	MissionObject* GetOwner() const { return params_.owner_; }

	virtual float GetFlySoundTime() { return 2.f; }
};

// орудие-заглушка
class DummyWeapon : public Weapon
{
	// принять инспектора
	virtual void Accept(IWeaponInspector&) {}

public:
	// производит симуляцию стрельбы (расчет физики и т.д.)
	virtual void Simulate(float) {}
	// создание копии
	virtual Weapon* Clone() const { return NEW DummyWeapon(*this); }
	virtual void ApplyPattern(WeaponPattern*) {}
	virtual float GetReadyFactor() {return 0.f;}

	// отрисовка информации в редакторе
	virtual void DrawEditorFeatures() {}
	virtual void DrawDebugInfo() {};
};