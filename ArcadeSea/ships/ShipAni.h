#pragma once

#include "..\..\Common_h\Mission.h"
#include "..\..\common_h\AnimationStdEvents.h"

class ShipAniBlendStage;
class ShipPart;
class Ship;

class ShipAniBlendStage : public IAniBlendStage
{
	// режим получения правильного положения кости
	enum PartTransformMode
	{
		ptm_physics,	// управление положением через физ объект
		ptm_animation,	// управление положением из анимации
		ptm_child,		// управляющая часть другая. этой надо только брать текущее положение оттуда
		ptm_collapse,	// часть не показывается
		ptm_default = 0xFFFFFFFF
	};

	// описание кости
	struct BoneDescr
	{
		long partIndex;			// индекс части корабля которая связана с данной костью
		bool isChild;			// флаг того, что кость является ребенком указанной части (управление также через нее)
		bool wasWaterSplash;	// флаг того, что часть уже упала в воду
		Matrix mtxPhys2Bone;	// матрица перевода положения физ объекта в положение кости
	};

	// описание части
	struct PartDescr
	{
		PartTransformMode ptm;	// режим трансформации (задана из анимации, физ тела или вообще не показывается)
		ShipPart* part;			// указатель на часть корабля
		long bone;				// индекс кости в скелете
		Matrix mtxTransform;	// текущая трансформация (для апдейтов физики или анимации)
	};

public:
	ShipAniBlendStage();
	virtual ~ShipAniBlendStage();

	void Init(ShipPart* pRootPart, IAnimationTransform* ani);
	bool IsFinished() {return m_bFinish;}

	// установить партикл для водного всплеска
	void CreateWaterSplashParticles(const char* pcSFX, MissionObject* pMO);

	// подключить к системе часть корабля. Повесить ее на заданную кость.
	void AttachPart(ShipPart* pPart,const char* pcBoneName);

	// переключить все управляемые физикой косточки в режим управления из анимации
	void TurnOnAnimation();

	// проверить и устаноить все связи дети-родитель в костях для блендера
	void SetupChilds();

	//Получить необходимость в данных предыдущих стадий
	virtual bool NeedPrevStageData(long boneIndex) {return true;}
	//Получить коэфициент блендинга для интересующей кости
	virtual float GetBoneBlend(long boneIndex) {return 1.f;}
	//Получить трансформацию кости
	virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale);
	//Обновить состояние
	virtual void Update(float dltTime);

private:
	bool m_bFinish;
	array<BoneDescr> m_aBoneDescr;
	array<PartDescr> m_aParts;
	IAnimation* m_pAni;
	array<bool> m_aWaterSplash;
	array<Vector> m_aCreateParticlesPos;

#ifndef _XBOX
	void TraceDebugInfo();
	bool bTraceDebug;
#endif
};

class ShipAniEvents : public AnimationStdEvents
{
	Ship* m_pShip;

public:
	ShipAniEvents(Ship* pShip) {m_pShip=pShip;}
	virtual ~ShipAniEvents() {}

protected:
	virtual void AddHandlers(IAnimation * anx)
	{
		AnimationStdEvents::AddHandlers(anx);
		//Оторвать паруса
		anx->SetEventHandler(this, (AniEvent)&ShipAniEvents::DetachSail, "SailDetach");
	}

	virtual void DelHandlers(IAnimation * anx)
	{
		AnimationStdEvents::DelHandlers(anx);
		//Отрыв парусов
		anx->DelEventHandler(this, (AniEvent)&ShipAniEvents::DetachSail, "SailDetach");
	}

public:
	//Отрыв парусов
	void _cdecl DetachSail(IAnimation * ani, const char * name, const char ** params, dword numParams);
};

class IShipAnimationMaker
{
public:
	// выскрести скелет из анимационной можели
	//static void GetMasterSceleton(IGMXScene* pModel, array<const GMXBone*>& aSkeleton);
	// создать процедурную анимацию для модельки
	static dword CreateProcedureAnimation(IGMXScene* pModel, IAnimationProcedural* & pAni, IAnimationProcedural** ppAniRefl);
	// создать анимационный блендер (для анимации взрыва корабля)
	//static ShipAniBlendStage* CreateAniBlender(IGMXScene* pModel, const char* pcNewAniName);
};
