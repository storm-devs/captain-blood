#pragma once
#include "ShipPart.h"
#include "..\ASSpecial\PhysSailRender.h"
#include "..\ASSpecial\FakeSailRender.h"

class Ship;
class IPhysCloth;
class IClothMeshBuilder;
class IClothBurns;
class Sail;

// хозяин паруса
class SailOwner
{
public:
	virtual IPhysCombined* GetPhysView() = 0;
	virtual IPhysCloth* CreateCloth(const char* file, long line, IClothRenderInfo & renderInfo, IClothMeshBuilder & builder, IPhysCloth::SimulationData & simData) = 0;
	virtual bool EditMode_IsOn() = 0;
	virtual Matrix& GetMatrix(Matrix& mtx) = 0;
	virtual float GetAlpha() = 0;
	virtual float GetLastDeltaTime() = 0;
	virtual IClothBurns* GetClothBurns() = 0;
	virtual void PartDestroyFlagSet(DWORD dwFlag) = 0;
	virtual const char* GetSoundNameForEvent(dword dwIdx) = 0;
	virtual void AcceptVisitor(IPartVisitor& clsf) = 0;
	virtual bool TieToPhysicBody(Sail* pSail,bool bTie,bool bTop) = 0;
};

class Sail : public ShipPart
{
	// режим учета ветра
	enum WindMode
	{
		WindMode_Enabled,
		WindMode_Disabled,
		WindMode_FallBegin,
		WindMode_FallEnding
	};

	// отложенное действие
	struct DeferredAction
	{
		// идентификатор действия
		enum Action
		{
			da_hCutSail,	// разрезать парус по горизонтальной линии
			da_hDetachSail,	// оторвать парус от всего к чему был привязан
			da_unknown = 0xFFFFFFFF
		};
		// параметры для действия
		struct Param
		{
			union
			{
				dword dwVal;
				float fVal;
			};
		};

		Action act;
		float fTime;
		Param param;
	};

	// описание привязки паруса к реям
	struct TieData
	{
		ShipPart* pTopYard;
		ShipPart* pBottomYard;
		Vector points[4];
		bool bTied;
		Matrix mtxYardTransf;
		Matrix mtxToSailTransf;
		bool bBottomTied;

		TieData() {pTopYard=pBottomYard=NULL;}
	};
 
public:
	struct Params : public ShipPart::PatternParams
	{
		struct Rope
		{
			Vector	begin;
			Vector	end;
			float	thickness;
		};

		Params() : PatternParams(),
			ropes(_FL_)
		{
		}
		Params(const Params& other) : PatternParams(),
			ropes(_FL_)
		{
			pointsCount = other.pointsCount;
			widthPointsCount = other.widthPointsCount;
			heightPointsCount = other.heightPointsCount;
			orient = other.orient;
			pos = other.pos;
			tearFactor = other.tearFactor;
			texture = other.texture;
			glimpseTexture = other.glimpseTexture;
			color = other.color;
			glimpseColor = other.glimpseColor;
			colorScale = other.colorScale;
			fakeAnimAmplitude = other.fakeAnimAmplitude;
			xFreq = other.xFreq;
			yFreq = other.yFreq;
			windCurvature = other.windCurvature;
			bottomCurvature = other.bottomCurvature;
			ropes = other.ropes;
			for (unsigned int i = 0; i < other.pointsCount; ++i)
				points[i] = other.points[i];
		}

		unsigned int	pointsCount;
		Vector			points[4];
		unsigned int	widthPointsCount;
		unsigned int	heightPointsCount;

		Vector			orient;					// локальные углы
		Vector			pos;					// относительная позиция на корабле

		const char*		texture;
		const char*		glimpseTexture;
		Color			color;
		Color			glimpseColor;
		float			colorScale;

		float			fakeAnimAmplitude;
		float			xFreq;
		float			yFreq;

		float			tearFactor;
		float			windCurvature;
		float			bottomCurvature;

		array<Rope>		ropes;
	};

	struct SailDebugData
	{
		long nVertexQ;
		long nIndexQ;
	};

protected:
	TieData m_TieData;

	Params						params_;
	string						name_;

	SailOwner*					m_pSailOwner;
	unsigned int				hits_;

	IPhysCloth*					cloth_;
	SailRender					sailRender_;
	FakeRender					fakeRender_;
	IClothMeshBuilder*			builder_;
	IPhysCloth::SimulationData*	simData_;
	bool						m_bUseHighTearFactor;
	float						m_fHighTearTime;
	float						m_fClothPushTime;

	bool						m_bSleep;
	Matrix						m_mtxSleepTransform;

	bool						isFake_;
	bool						m_bNeverPhysics;
	Matrix						shipMtx_;
	Box							fakeABB_;
	unsigned int				doNotRenderPhysicalCounter_;
	float						m_fFadeInTime;
	float						m_fFadeInTimeMax;

	bool						m_bIsDestroyed;
	bool						m_bNoShow;
	float						m_fDestroyTime;
	bool						m_bUpdateBuffers;

	array<DeferredAction>		m_aDeferredActions;

	WindMode					m_eWindMode;
	float						m_fFallTime;

	Sail& operator=(const Sail& other);
	Sail(const Sail&);

	void InitSimData();
	bool InFrustum(IRender& render) const;

public:
	
	Sail(const Sail::Params&, const char* name, SailOwner* owner);
	virtual ~Sail(void);

	// привязать парус к физактеру
	void Attach(IPhysRigidBody& actor);
	// отцепить весь парус от физшейпов (рея)
	void Detach();
	// отцепить нижний край паруса от физшейпов (рея)
	void DetachBottom();
	// получить повреждение паруса
	float GetHealth();
	// начать процесс разрушения паруса (в результате разрушения привязанной к нему реи)
	void BeginDestruction(float fDetachTime);
	// атаковать парус отрезком
	bool Attack(const Vector&, const Vector&, bool isTrace);
	// атаковать парус сферой
	bool Attack(const Vector& vCenter, float fRadius);

	//////////////////////////////////////////////////////////////////////////

	// установить параметры паруса для разрушения
	void ActivateBreakParams();
	bool EventPartBreak(ShipPart* pPart);

	// инициализировать парус
	void Make(bool isFake, IClothMeshBuilder* builder, bool neverPhysics);
	// переключить режим паруса
	bool SetFake(bool fake);
	// дать текущий режим
	bool IsFake() const { return isFake_; }
	// починить парус
	void Rebuild();

	// отключить показ паруса
	void FadeIn(float fFadeTime);

	//////////////////////////////////////////////////////////////////////////
	// ShipPart
	//////////////////////////////////////////////////////////////////////////
	virtual ShipPart* Clone() { return NEW Sail(*this); } // клонирование
	virtual float HandleDamage(const Vector&, float) { return 0.0f; } // возвращает переданный дэмэдж
	virtual void DrawPartRefl(const Matrix& parent);
	virtual void StartFrame() {m_bUpdateBuffers=true; ShipPart::StartFrame();}

	virtual const PatternParams& GetPatternParams() {return params_;}

	SailOwner* GetSailOwner() {return m_pSailOwner;}

	void ShipPart_EnablePhysicActor(bool bEnable);

	float fDestroyTime;
	long m_nPrevVrtxID;
	void LaunchDestroy();
	bool IsDestroying() {return m_bIsDestroyed;}
	void DestroyFrame();
	// получить степень порванности паруса: 0 = целый / 1 = максимально поврежден
	float GetTearedState();

	// показать дебажную информацию (строки начинают выводится с указанных координат)
	void ShowSailDebugInfo(float x,float y);
	void GetDebugData(SailDebugData& sdd);

	void TieSailToPhysShape(bool bTop, IPhysRigidBody & body, long nShape, const Vector & v1, const Vector & v2);

protected:
	virtual void DoWork(const Matrix&);
	virtual void Draw(const Matrix&);

	virtual void Accept(IPartVisitor& visitor) { visitor.Visit(*this); }

	bool MissionSailsSetFake(bool bFake);
	void ExcludeSailFromLimits();
	void ExcludeSailFromBurns();

	void InitSailFall();
	void FindTiedYard();
	void TieSailToYards();

	void SleepCloth(const Matrix& mtxSleepTransform);
	void WakeupCloth(const Matrix& mtxWakeupTransform);
};