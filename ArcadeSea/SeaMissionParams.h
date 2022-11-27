#pragma once
#include "..\common_h\mission.h"

class Sail;
class Ship;

class SeaMissionParams : public MissionObject
{
public:
	SeaMissionParams();
	~SeaMissionParams();

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader) {ReadMOPs(reader); return true;}
	MO_IS_FUNCTION(SeaMissionParams, MissionObject);

	void _cdecl Work(float fDeltaTime, long level);

	// интерфейсные методы
	//-----------------------------------------------
	// установить парус физическим или фейковым
	bool SailLimit_SetFake( bool bFake, Sail* pSail );
	// удалить физический парус из ограничений
	void SailLimit_DelSail( Sail* pSail );
	// добавить физический парус в ограничения
	void SailLimit_AddSail( Sail* pSail );
	// текстура тени от ядра
	const char* GetCannonBallShadowTexture() const {return m_pcCannonBallShadowTexture;}
	// итератор кораблей
	MGIterator * GetShipIterator() {return m_pShipIterator;}
	// добавить парус в список для удаления
	void ClothToDelete(IPhysCloth* pCloth) {m_aClothForDelete.Add(pCloth);}
	// добавить актера в список для удаления
	void ActorToDelete(IPhysRigidBody* pActor) {m_aActorForDelete.Add(pActor);}
	// добавить коннектор в список для удаления
	void JointToDelete(IPhysRigidBodyConnector* pConnector) {m_aConnectorForDelete.Add(pConnector);}

	// для дебага
	void _cdecl Console_ShipCommand(const ConsoleStack &params);

	static const char* GetMaterialName(dword dwMat);

protected:
	void ReadMOPs(MOPReader & reader);

	//-----------------------------------------------
	// секция ограничения физических парусов
	//-----------------------------------------------
	long m_nPhysicsSailsQuantityMax;
	long m_nPhysicsSailsQuantityForDestroy;
	array<Sail*> m_aPhysicSails;

	array<IPhysCloth*> m_aClothForDelete;
	array<IPhysRigidBody*> m_aActorForDelete;
	array<IPhysRigidBodyConnector*> m_aConnectorForDelete;

	bool m_bShowDebugInfo;
	long m_nDestroingSailQuantity;
	bool m_bKeyPressed;
	Sail* m_pSailForSailInfo;

	void SailLimit_Frame();
	void SailLimit_LaunchSailDestroy(Sail* pSail);
	//-----------------------------------------------

	const char* m_pcCannonBallShadowTexture;

	MGIterator * m_pShipIterator;

	enum CommandAction
	{
		cmdact_set,
		cmdact_reset,
		cmdact_switch,

		cmdact_none
	};
	bool SendInfoCommandToShip(Ship* pship, dword mask, dword action);
};
