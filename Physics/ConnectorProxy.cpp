#include "ConnectorProxy.h"
#include "PhysicsScene.h"

ConnectorProxy::ConnectorProxy(PhysicsScene& scene, IPhysRigidBody * left, IPhysRigidBody * right, float brokeForce) :
IPhysRigidBodyConnector(&scene),
IProxy(scene),
m_realConn(NULL),
m_left(left),
m_right(right),
m_brokeForce(brokeForce),
m_eventHandler(NULL)
{
	if (left) left->AddRef();
	if (right) right->AddRef();
}

ConnectorProxy::~ConnectorProxy(void)
{
	if (m_realConn)
		m_realConn->Release(), m_realConn = NULL;

	if (m_left) m_left->Release();
	if (m_right) m_right->Release();
}

void ConnectorProxy::SetEventHandler(BrokeEvent * event)
{
	m_eventHandler = event;
}

bool ConnectorProxy::Release() 
{ 
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true; 
	//return false; 
}

void ConnectorProxy::OnSyncCreate()
{
	if (m_realConn)
		return;

	m_realConn = NEW PhysRigidBodyConnector(GetFileName(), GetFileLine(), &m_scene, m_left, m_right, m_brokeForce);

	Assert(m_realConn);

	SetRealPhysObject(m_realConn);

	/*PhysInternal internals;
	m_realConn->GetInternals(internals);
	internals.actor->userData = (IPhysBase*)this;*/
}

void ConnectorProxy::OnSyncCalls()
{
	if (m_eventHandler)
		m_realConn->SetEventHandler(m_eventHandler);

}

bool ConnectorProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void ConnectorProxy::OnSyncTrace()
{
}
