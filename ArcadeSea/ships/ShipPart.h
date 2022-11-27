#pragma once
#include "..\..\Common_h\core.h"
#include "..\..\Common_h\templates\array.h"
#include "PartVisitor.h"
#include "ShipAni.h"

class Ship;
class IParticleSystem;
class MissionObject;
class IPhysRigidBody;
class ShipsDrawer;
class SeaMissionParams;

//--------------------------------------------------------------
// определение набора партиклов с параметрами
//--------------------------------------------------------------
#define MOP_PARTICLE_DESCR(name) \
	MOP_ARRAYBEG(name, 1, 100) \
		MOP_STRING("SFX name", "") \
		MOP_FLOATEX("Owner velocity factor", 0.75f, 0, 1.5f) \
	MOP_ARRAYEND

struct OwnerDependedParticles
{
	struct OneDescr
	{
		const char* sfx;
		float fOwnerVelocityFactor;
	};
	array<OneDescr> e;

	OwnerDependedParticles() : e(_FL_) {}
	void ReadMOPs(MOPReader& reader)
	{
		long q = reader.Array();
		e.DelAll();
		if( q > 0 )
		{
			e.AddElements( q );
			for( long n=0; n<q; n++ )
			{
				e[n].sfx = reader.String().c_str();
				e[n].fOwnerVelocityFactor = reader.Float();
			}
		}
	}
};
//--------------------------------------------------------------
// конец определению набора партиклов
//--------------------------------------------------------------

// часть корабля
class ShipPart
{
public:
	struct SystemItem
	{
		IPhysRigidBody*				actor;
		bool						active;
		long						boneIndex;
		IPhysRigidBodyConnector*	joint;
		Vector						vWakeVelocity;
	};

	struct PatternParams
	{
		float fExplodeImpulsePower;

		PatternParams() {fExplodeImpulsePower=0.5f;}
	};

	struct DescrSFX
	{
		const char* name;
		const char* lodname;

		DescrSFX() {name=lodname="";}

		void ReadMOPs(MOPReader & reader)
		{
			name = reader.String().c_str();
			lodname = reader.String().c_str();
		}

		const char* GetSFXName(bool isLod) {return isLod ? lodname : name;}
		const char* GetSFXName(const Vector& pos, IRender & render, float dist)
		{
			return GetSFXName( ~render.GetView().MulVertex(pos) < dist * dist );
		}
	};

	enum HitGroup
	{
		hitgrp_common,
		hitgrp_burn
	};

private:
	struct Hit
	{
		Vector pos;
		IParticleSystem* ps;
		float fOwnerVelocityFactor;

		unsigned short dwGroupID;
		long nID;
		float fLiveTime;
		float fMaxLiveTime;

		void Release()
		{
			RELEASE(ps);
		}

		void Create(IParticleManager& pm, const char* pcName)
		{
			ps = pm.CreateParticleSystemEx(pcName, _FL_);
			if( ps )
			{
				ps->AutoHide(true);
			}
		}
	};

	struct BrokenPartPhysData
	{
		IPhysRigidBody*		actor;
		bool				active;
		bool				bWaitFallToSea;

		float maxDamping;
		float curDamping;

		BrokenPartPhysData() {curDamping=0.f; maxDamping=0.f; actor=0; bWaitFallToSea=false;}
	};

	// хозяин части (как корабль и как миссионный объект)
	Ship*			m_pOwnerShip;
	MissionObject*	m_pOwnerMO;

	array<Matrix>		poses_;				// начальные положения шейпов
	array<unsigned int>	shapes_;			// физ. шейпы
	array<ShipPart*>	children_;			// потомки
	ShipPart*			parent_;			// родительская часть
	array<Hit>			hits_;				// партиклы попаданий в часть корабля
	bool				isBroken_;			// разрушена/не разрушена
	long				m_nAnimationBone;	// часть управляется анимацией а не физикой (индекс кости от которой берется положение)

	void DrawHits(const Matrix& parent);
	virtual void DrawBroken();
	
	virtual void UpdateDamagedModelStatus() {}
	virtual void Draw(const Matrix& parent) {}
	virtual void Accept(IPartVisitor& visitor) { visitor.Visit(*this); }
	virtual void DoWorkChildren(const Matrix&);

	array<ShipPart::SystemItem> brokenSystem_;

protected:
	BrokenPartPhysData  physdata_;

	bool AddHitAnimation(const char* sfxName, float fOwnerVelocityFactor, const Vector& worldPosition, unsigned short grpID=hitgrp_common, long nID=-1, float fLiveTime=0.f);
	bool AddHitAnimation(OwnerDependedParticles & particles, const Vector& worldPosition, unsigned short grpID=hitgrp_common, long nID=-1, float fLiveTime=0.f);
	void AddClothBurnSphere(const Vector& worldPosition);
	void RestoreLiveTimeForHitGroup(unsigned short grpID);
	void ReleaseHitParticles();
	void DoWaitFallToSea();
	bool ReleasePhysData(IPhysRigidBody* actor);
	
public:

	ShipPart() :	poses_(__FILE__, __LINE__),
					shapes_(__FILE__, __LINE__),
					children_(__FILE__, __LINE__),
					hits_(__FILE__, __LINE__),
					brokenSystem_(__FILE__, __LINE__)
	{
		parent_ = NULL;
		isBroken_ = false;
		m_nAnimationBone = -1; // первоначально, положение части не зависит от анимации
	};
	virtual ~ShipPart(void);

	// установить получить хозяина части
    void SetOwner(Ship* ship,MissionObject* obj) { m_pOwnerShip = ship; m_pOwnerMO = obj; }
	Ship* GetShipOwner() { return m_pOwnerShip; }
	MissionObject* GetMOOwner() { return m_pOwnerMO; }

	virtual float HandleDamage(const Vector& pos, float damage) = 0; // возвращает переданный дэмэдж
	virtual float GetTouchDamageFactor() {return 1.f;} // получить коеффициент использования дамага от столкновения

	void DrawPart(const Matrix& parent);
	virtual void DrawPartRefl(const Matrix& parent);
	void Work(const Matrix& parent);
	virtual void DoWork(const Matrix&);
	virtual void StartFrame() {for(long n=0; n<children_; n++) if(children_[n]) children_[n]->StartFrame();}

	void SetParent(ShipPart* parent) { parent_ = parent; }
	ShipPart* GetParent() { return parent_; }

	SeaMissionParams* GetSeaMParams();

	virtual const PatternParams& GetPatternParams() = 0;

	virtual void Rebuild();
	void TreeRebuild() {Rebuild(); for(long n=0; n<children_; n++) if(children_[n]) children_[n]->TreeRebuild();}

	array<Matrix>&			GetInitialPoses() { return poses_; }
	array<unsigned int>&	GetShapes() { return shapes_; }
	array<ShipPart*>&		GetChildren() { return children_; }

	float	GetDestroyImpulsePower() { return GetPatternParams().fExplodeImpulsePower; }

	void SetBroken(bool value);
	bool IsBroken() const { return isBroken_; }
	void SetBrokenSystem(const array<ShipPart::SystemItem> & system);
	const array<ShipPart::SystemItem>& GetBrokenSystem() const { return brokenSystem_; };
	void CleanBrokenSystem(bool doRelease = false);
	void DelFromBrokenSystem(long nItemIdx);
	void CleanAllBrokenSystems() {CleanBrokenSystem(true); for(long n=0; n<children_; n++) if(children_[n]) children_[n]->CleanBrokenSystem(true);}

	void AcceptVisitor(IPartVisitor& visitor) { Accept(visitor); }

	virtual void SetPhysActor(IPhysRigidBody* actor);
	IPhysRigidBody* GetPhysActor() {return physdata_.actor;}
	void KillJoint(IPhysRigidBody* actor);
	Matrix& GetInitMatrix(Matrix& mtx);

	void UpdateBoneTransform();
	void SetAlpha(float fAlpha);

	virtual bool IsNotRotable() {return false;}

	Matrix& GetWorldTransform(Matrix& mtx);

	virtual void ShipPart_EnablePhysicActor(bool bEnable);

	void SetMovementByAnimation(long nBone) {m_nAnimationBone=nBone;}

	void SetPhysTransform( const Matrix& mtx );
	void GetPhysTransform( Matrix& mtx );

	string m_strDebugPartName;
	virtual const char* GetDebugName() {return NULL;}

	void DebugShowPhysForm();
};
