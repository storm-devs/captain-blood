#ifndef _ShipDamageDetector_h_
#define _ShipDamageDetector_h_

#include "..\Common_h\Mission\Mission.h"

class ShipDamageDetector : public DamageReceiver
{
	struct DamageEvent
	{
		float hp;
		MissionTrigger trigger;
		bool bIsDoing;
	};
//--------------------------------------------------------------------------------------------
public:
	ShipDamageDetector();
	virtual ~ShipDamageDetector();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	// Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);

	virtual void Restart();

	//
	MO_IS_FUNCTION(ShipDamageDetector, DamageReceiver);

	virtual Matrix & GetMatrix(Matrix & mtx) {return (mtx = m_transform);}
	virtual void GetBox(Vector& min, Vector &max) {min=m_minBound; max=m_maxBound;}
	virtual float GetHP() { return m_fHP; }
	virtual float GetMaxHP() { return m_fMaxHP; }
	virtual bool IsDead() { return m_fHP<=0.f; }

	//Воздействовать на объект сферой
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius);
	//Воздействовать на объект линией
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to);
	//Воздействовать на объект выпуклым чехырёхугольником
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4]);

//--------------------------------------------------------------------------------------------
private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);
	// дебажная отрисовка
	void _cdecl DebugDraw(float dltTime, long level);

	void EnableCollision(bool bEnable);

	// наносим повреждения
	void DoDamage(MissionObject* srcobj, dword source, float hp);
	// рожаем эффект попадания
	void CreateEffect( const Vector& pos, const Vector& normal, MissionObject* srcobj, dword source);

//--------------------------------------------------------------------------------------------
protected:
	array<DamageEvent> m_events;

	float	m_fHP;
	float	m_fMaxHP;

	Matrix	m_transform;
	Vector	m_minBound;
	Vector	m_maxBound;
	IMissionQTObject* m_pFinder;

	const char*	m_pcColliderGeoName;
	IGMXScene*	m_pColliderGeo;
	IPhysRigidBody* m_pCollider;
	bool		m_bShowCollider;

	const char* m_pcHitSFX;
	float m_fHitSFXScale;
	const char* m_pcHitSFXlod;
	float m_fHitSFXlodScale;
	float m_fHitSFXlodDistance;
	const char* m_pcHitSound;

	MOSafePointer m_arbiter;
	bool m_bExplodeHit;
	float m_fExplodeRadius;
	float m_fExplodeDamage;
	float m_fExplodePower;
};

#endif

