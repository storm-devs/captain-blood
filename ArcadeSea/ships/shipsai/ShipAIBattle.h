
#ifndef _ShipsAIBattle_h_
#define _ShipsAIBattle_h_

#include "ShipAIBattleParams.h"

class ShipAIManeurChecker
{
public:
	enum ManeurType
	{
		ManeurType_Turning,
		ManeurType_Interception,
		ManeurType_None
	};

protected:
	struct StatisticData
	{
		float fAfterShootTime;
		float fRecievedDamage;
	} m_Statistic;

	MissionObject* m_pSelf;
	MissionObject* m_pEnemy;
	Vector m_vManeurTargetPos;
	float m_fManeurShipSpeed;
	bool m_bManeurProcessing;
	float m_fManeurTime;

public:

	ShipAIManeurChecker();
	~ShipAIManeurChecker();

	void SetShips(MissionObject* pSelf, MissionObject* pEnemy);
	void Update(float fDeltaTime);

	bool IsManeurNeed();
	bool IsManeurProcessing() {return m_bManeurProcessing;}
	bool StartManeur();
	bool ManeurIsDone();
	void StopManeur();

	Vector GetSwimTarget() {return m_vManeurTargetPos;}
	float GetSwimSpeed() {return m_fManeurShipSpeed;}

protected:
	void ClearStatistic();
	void CalculateTargPos(ManeurType mt);
	ManeurType ChooseType();
};

class Battle_ShipAI : public ShipAI
{
	enum AttackStage
	{
		AttackStage_none,

		AttackStage_SwimToTarget,
		AttackStage_FireToTarget,
		AttackStage_BoardTarget,
		AttackStage_Maneur
	};

public:
	Battle_ShipAI ( IShip & rShip, IMission & rMission );
	~Battle_ShipAI ();

	// управление кораблем
	void Work( float dltTime );
	// установить объект параметров
	void SetControllerParams( IShipControllerParams * pControllerParams );
	// получить текущее АИ-состояние
	ShipControlType GetType() const;
	// получить  объект преследования
	MissionObject *	GetEnemy() const;
	// получить  тип поведения в бою
	BattleBehaviorType GetBehaviorType() const;

private:
	// АИ стрельбы
	void ProcessAIFire ( float fDTime );
	// прекращение атаки выбранной цели
	void StopAttacking ();
	// отрисовка дебажной инфы
	void DrawDebugInfo ();
	// проверка необходимости совершения маневра

	ShipBattleAIParams *	m_pShipBattleAIParams;		// объект параметров
	MissionObject *			m_pEnemy;					// объект противника
	float					m_fOptimumFireDistance;     // оптимальная дальность до противника при стрельбе
	BattleBehaviorType		m_eBehaviorType;			// вид поведения в бою ( огонь по врагу, абордаж, бегство ) 
	float					m_fMaxFireRadius;			// максимальная дистанция до корабля, при которой по нему можно вести огонь
	float					m_fEnemyDiscoverDistance;	// дистанция обнаружения противника
	Vector					m_vTargetPos;				// текущая целевая точка
	AttackStage				m_iAttackStage;				// стадия атаки: 1 - подплытие к цели, 2 - собственно атака
	ShipAIManeurChecker		m_Maneur;					// объект работы с маневрами
	float					m_fFireLevel;				// уровень стрельбы из пушек (0.f-по корпусу, 1.f-по парусам, между ними - вероятность)
};

#endif
