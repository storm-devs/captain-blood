#pragma once

#include "..\..\common_h\mission\mission.h"

class IPhysCloth;

class IBurnZone
{
public:
	virtual ~IBurnZone() {}
};

class IClothBurns : public MissionObject
{
public:
	// группа пламени
	enum BurnGroupID
	{
		group_cloth,
		group_shipfire,
		group_flamethrower,
		group_explose
	};

	// добавить пламя в указанной позиции (позиция либо локальная и привязывается к объекту, либо глобальная pMO=NULL), при этом уничтожение пламени ложится на манагер
	virtual void AddBurnNotHosted(BurnGroupID grpID, const Vector& vPos, MissionObject* pMO, float fLiveTime=0.f, float fRad=0.f) = 0;
	// добавить пламя в указанной позиции (позиция либо локальная и привязывается к объекту, либо глобальная pMO=NULL)
	virtual IBurnZone* AddBurn(BurnGroupID grpID, const Vector& vPos, MissionObject* pMO, float fLiveTime=0.f, float fRad=0.f) = 0;
	// добавить сплошной ряд пламени от заданной позиции до другой позиции (позиция либо локальная и привязывается к объекту, либо глобальная pMO=NULL)
	virtual IBurnZone* AddBurnLine(BurnGroupID grpID, const Vector& vPosSrc, const Vector& vPosDst, MissionObject* pMO, float fLiveTime=0.f, float fRad=0.f) = 0;
	// обработать ткань пламенем
	virtual bool ProcessCloth(MissionObject* pTiedMO, IPhysCloth* pCloth, IPhysCloth::SimulationData* pSimData) = 0;
	// проверить попадает ли какое нить пламя в бокс
	virtual bool CheckBurnIntoBox(IPhysCloth* pCloth, const Matrix& mtx, const Vector& size) = 0;

	// объект уничтожен - удалим все связанное с ним, во избежание недействительных ссылок
	virtual void ExcludeObjects(MissionObject* pMO) = 0;
	// парус уничтожен - удалим все связанное с ним, во избежание недействительных ссылок
	virtual void ExcludeObjects(IPhysCloth* pCloth) = 0;

	// получить вероятность рождения очага огня в месте попадания ядра
	virtual float GetExplosionBurnProbability() = 0;
};

class BurnZone;
// рендер физической ткани
class ClothBurns : public IClothBurns
{
	friend BurnZone;
	// параметры
	struct BurnParams
	{
		struct BurnGroup
		{
			// время жизни нового очага возгарания
			float fTimeMin;
			float fTimeMax;
			float fTimeInactive;

			// радиус нового очага возгарания
			float fMinRadius;
			float fMaxRadius;
			float fGrowingFactor;
		};

		// радиус группирования очагов горения в один
		float m_fGroupingRadius;

		// имя партиклов для горящего куска паруса
		const char* m_pcBurnSFX;
		// имя партикловой системы для уничтожаемого куска паруса
		const char* m_pcBurnDestroySFX;

		// время которое точка паруса живет находясь в огне
		float m_fMaxBurningTime;
		float m_fReburnProbability;

		BurnGroup m_ClothGroup;
		BurnGroup m_ShipFireGroup;
		BurnGroup m_FlameThrowerGroup;
		BurnGroup m_ExploseGroup;

		// вероятность рождения очага огня от взрыва ядра
		float m_fExplosionBurnProbability;

		// рисовать дебажную инфу или нет
		bool m_bShowDebug;
	};

	// сферы пламени
	struct BurnSphere
	{
		Vector c;
		float r;

		// объект и позиция центра в его координатах - для привязки к объекту
		Vector vLoc;
		MissionObject * pTieObj;

		// привязка к тряпке (если нуно)
		IPhysCloth* pCloth;
		IPhysCloth::SimulationData* pSimData;
		long nVertexID;

		// время жизни: текущее и максимальное - для расчета изменяемых параметров
		float fLiveTime;
		float fLiveTimeMax;
		float fInactiveTime;

		// стартовый радиус и максимальный радиус в конце жизни
		float fRad;
		float fRadAdd;

		// группа пламени
		BurnGroupID grpID;
	};

	struct BurnedClothPoints
	{
		IPhysCloth* pCloth;
		long nVertexID;

		IPhysCloth::SimulationData* pSimData;
		MissionObject* pMO;

		IParticleSystem* pParticle;

		float fBurningTime;
		float fFreezeTime;
		Vector vOldPos;

		Vector GetPosition() {return *(Vector*)((BYTE*)pSimData->posBuffer + pSimData->posStride*nVertexID);}
	};

public:
	ClothBurns();
	virtual ~ClothBurns();

	void _cdecl Update(float fDeltaTime, long level);

	// Создание объекта
	virtual bool Create(MOPReader & reader);
	// Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	// Инициализация 
	virtual bool Init();
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	virtual void AddBurnNotHosted(BurnGroupID grpID, const Vector& vPos, MissionObject* pMO, float fLiveTime=0.f, float fRad=0.f);
	virtual IBurnZone* AddBurn(IClothBurns::BurnGroupID grpID, const Vector& vPos, MissionObject* pMO, float fLiveTime=0.f, float fRad=0.f);
	virtual IBurnZone* AddBurnLine(IClothBurns::BurnGroupID grpID, const Vector& vPosSrc, const Vector& vPosDst, MissionObject* pMO, float fLiveTime=0.f, float fRad=0.f);
	virtual bool ProcessCloth(MissionObject* pTiedMO, IPhysCloth* pCloth, IPhysCloth::SimulationData* pSimData);
	virtual bool CheckBurnIntoBox(IPhysCloth* pCloth, const Matrix& mtx, const Vector& size);

	virtual void ExcludeObjects(MissionObject* pMO);
	virtual void ExcludeObjects(IPhysCloth* pCloth);

	virtual float GetExplosionBurnProbability() {return m_params.m_fExplosionBurnProbability;}

	void DeleteBurn(BurnSphere* pBurnSphere) {if(pBurnSphere && m_burns.Find(pBurnSphere)!=INVALID_ARRAY_INDEX) {m_burns.Del(pBurnSphere); delete pBurnSphere;}}
	void DeleteZone(IBurnZone* pBurnZone) {m_burnzone.Del(pBurnZone);}

protected:
	BurnParams m_params;
	array<IBurnZone*> m_burnzone;
	array<BurnSphere*> m_burns;
	array<BurnedClothPoints> m_burnedpoints;

	void ReadMOPs(MOPReader & reader);

	BurnSphere* AddOneBurn(IClothBurns::BurnGroupID grpID, const Vector& vPos, MissionObject* pMO, float fLiveTime=0.f, float fRad=0.f, IPhysCloth* pCloth=0, IPhysCloth::SimulationData* pSimData=0, long nVertexID=-1);
	BurnSphere* GroupBurns(BurnSphere& bs);

	bool DestroyClothPoint(MissionObject* pMO, IPhysCloth* pCloth, IPhysCloth::SimulationData* pSimData, long nVertexID);
	bool AddBurnedPoint(IPhysCloth* pCloth, IPhysCloth::SimulationData* pSimData, long nVertexID, MissionObject* pMO);
};
