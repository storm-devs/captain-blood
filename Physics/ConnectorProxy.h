#pragma once
#include "RigidBodyProxyHelper.h"
#include "IProxy.h"

class PhysicsScene;
class ConnectorProxy : public IPhysRigidBodyConnector, public IProxy
{
	IPhysRigidBodyConnector*						m_realConn;
	//PhysicsScene&									m_scene;

	BrokeEvent *		m_eventHandler;
	IPhysRigidBody*		m_left;
	IPhysRigidBody*		m_right;
	float				m_brokeForce;
public:
	ConnectorProxy(PhysicsScene& scene, IPhysRigidBody * left, IPhysRigidBody * right, float brokeForce);
	virtual ~ConnectorProxy(void);

	//////////////////////////////////////////////////////////////////////////
	// IPhysRigidBodyConnector
	//////////////////////////////////////////////////////////////////////////
	//Сломано соединение или нет
	virtual bool IsBroke() { if (!m_realConn) return false; return m_realConn->IsBroke(); }
	//Поставить обработчик
	virtual void SetEventHandler(BrokeEvent * event);
	//Получить первый объект к которому присоеденены
	virtual IPhysRigidBody * GetLeft() { return m_left; }
	//Получить второй объект к которому присоеденены
	virtual IPhysRigidBody * GetRight() { return m_right; }

	//////////////////////////////////////////////////////////////////////////
	// IPhysBase
	//////////////////////////////////////////////////////////////////////////
	virtual bool Release();

	//////////////////////////////////////////////////////////////////////////
	// IProxy
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSyncCreate();
	virtual void OnSyncCalls();
	virtual bool OnSyncRelease();
	virtual void OnSyncTrace();
};
