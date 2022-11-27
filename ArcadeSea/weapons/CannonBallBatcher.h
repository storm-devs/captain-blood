#pragma once
#include "..\..\common_h\math3d.h"
#include "..\..\common_h\gmx.h"
#include "..\..\common_h\render.h"
#include "cannonball.h"
#include "weapon.h"

class CannonBallBatcher
{
	struct ShadowVertex
	{
		Vector pos;
		float u,v;
		float alpha;
	};

	static const unsigned int	m_nMaxInstance = 32;
	array<Vector4>				m_Positions;

	IBaseTexture*	m_shadowTexture;
	IVariable*		m_shadowTextureVariable;
	IVBuffer*		m_vbShadow;
	IIBuffer*		m_ibShadow;

	IGMXScene*		m_pAniModel;
	long			m_nBoneQuantity;
	IAnimationProcedural* m_pAni;


	ShaderId BallShadow_id;

	void InitShadowBuffers();
	void Update();

public:
	CannonBallBatcher();
	~CannonBallBatcher();

	void ReleaseModel();

	//////////////////////////////////////////////////////////////////////////
	// IBatch
	//////////////////////////////////////////////////////////////////////////
	unsigned int GetInstanceCount() const { return m_Positions.Size(); }
	void AddInstance(const Vector& pos);
	void Draw();
	void Flush() { m_Positions.Empty(); }
	void SetBallModel(IGMXScene* pModel);
	void SetShadowTexture(const char* pcTextureName);
};

const unsigned int CannonBallBatcherMaxQuantity = 4;
class CannonBallContainer : public MissionObject
{
	struct BatchInfo
	{
		CannonBallBatcher batcher;
		string modelname;
	};
	BatchInfo m_Batch[CannonBallBatcherMaxQuantity];

	array<CannonBall> m_aBalls;
	//ITrailManager*	m_trailmanager;
	MOSafePointer	m_trailmanager;

	void Simulate(float deltaTime);

public:
	// конструктор/деструктор
	CannonBallContainer();
	~CannonBallContainer();

	// наследование от MissionObject
	bool Create(MOPReader &reader);
	void Show(bool isShow);
	MO_IS_FUNCTION(CannonBallContainer, MissionObject);

	// отрисовка
	void _cdecl Draw(float dltTime, long level);

	// интерфейсные методы
	bool AddBall(Weapon * parent, const char* waterHitSFX, long nModelBatcher, IGMXScene* pModel, const Vector& vFrom, const Vector& vTo, float fSpeed, float fMinAngle, float fMaxAngle, const WeaponTrailParams& trail, float fDamageMultiply, const char* pcFlySound);
	void AddBallWithoutCheck(Weapon * parent, const Vector& src, const Vector& dst, float fSpeed, TargetPoints* pTargPoints, long pointIdx, long nModelBatcher, IGMXScene* pModel, const WeaponTrailParams& trail, const char* waterHitSFX, const char* pcFlySound);
	void DeleteByWeapon(Weapon * parent);
	void SetBallParams(CannonBall& ball, Weapon * parent, TargetPoints* pTargPoints, long pointIdx, long nModelBatcher, IGMXScene* pModel, const WeaponTrailParams& trail, const char* waterHitSFX, const char* pcFlySound);
	long RegistryCannonballBatcher(const char* pcModelName, IGMXScene* pModel);
	void SetShadowTexture(const char* pcShadowTexture);

private:
	CannonBall & GetFreeBall();
};
