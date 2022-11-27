#include "CharacterProxy.h"
#include "PhysicsScene.h"
#include "PhysCharacter.h"

CharacterProxy::CharacterProxy(float radius, float height, PhysicsScene& scene) :
IPhysCharacter(&scene),
IProxy(scene),
CONSTRUCT_MCALLS,//m_calls(__FILE__, __LINE__),
m_realChar(NULL),
m_radius(radius),
m_height(height),
m_position(0.0f),
m_changedPosition(false)
{
}

CharacterProxy::~CharacterProxy(void)
{
	if (m_realChar)
		m_realChar->Release(), m_realChar = NULL;

	DELETE_MCALLS
	//for (unsigned int i = 0; i < m_calls.Size(); ++i) def_delete(m_calls[i]);
	//m_calls.DelAll();
}

//////////////////////////////////////////////////////////////////////////
// IPhysCharacter
//////////////////////////////////////////////////////////////////////////
//Установить радиус персонажу
void CharacterProxy::SetRadius(float v)
{
	m_radius = v;
	m_calls.Add
	(
		MakeDeferrer<IPhysCharacter, void, float>(DEF_FREF(IPhysCharacter::SetRadius), v)
	);
}

//Получить радиус персонажа
float CharacterProxy::GetRadius()
{
	if (!m_realChar)
		return m_radius;
	return m_realChar->GetRadius();
}

//Установить высоту персонажу
void CharacterProxy::SetHeight(float v)
{
	m_height = v;
	m_calls.Add
	(
		MakeDeferrer<IPhysCharacter, void, float>(DEF_FREF(IPhysCharacter::SetHeight), v)
	);
}

//Получить высоту персонажа
float CharacterProxy::GetHeight()
{
	if (!m_realChar)
		return m_height;
	return m_realChar->GetHeight();
}

//Установить позицию
void CharacterProxy::SetPosition(const Vector & pos)
{
	m_changedPosition = true;
	m_position = pos;
	m_calls.Add
	(
		MakeDeferrer<IPhysCharacter, void, const Vector&>(DEF_FREF(IPhysCharacter::SetPosition), pos)
	);
}

//Получить позицию
Vector CharacterProxy::GetPosition()
{
	if (!m_realChar || m_changedPosition)
		return m_position;

	return m_realChar->GetPosition();
}

//Передвинуть персонажа
dword CharacterProxy::Move(const Vector & move, dword collisionGroups)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCharacter, dword, const Vector&, dword>(DEF_FREF(IPhysCharacter::Move), move, collisionGroups)
	);
	//Assert("how to return collision flags ?")
	//Возвращаем коллижн флаги с предыдущего кадра, если есть m_realChar
	if (m_realChar)
		return ((PhysCharacter*)m_realChar)->GetMoveCollisionFlags();

	return 0;
}

//Активировать/деактивировать персонажа
void CharacterProxy::Activate(bool isActive)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCharacter, void, bool>(DEF_FREF(IPhysCharacter::Activate), isActive)
	);
}

//Если не 0.0f, то на все объекты при столкновении будет приложена сила v
void CharacterProxy::ApplyForceToObjects(float v)
{
	m_calls.Add
	(
		MakeDeferrer<IPhysCharacter, void, float>(DEF_FREF(IPhysCharacter::ApplyForceToObjects), v)
	);
}

bool CharacterProxy::Release() 
{ 
	return IProxy::AddReleaseCount();
	//m_bReleaseCall = true; 
	//return false; 
}

//////////////////////////////////////////////////////////////////////////
// IProxy
//////////////////////////////////////////////////////////////////////////
void CharacterProxy::OnSyncCreate()
{
	if (m_realChar)
		return;

	m_realChar = NEW PhysCharacter(GetFileName(), GetFileLine(), m_radius, m_height, &m_scene);
	Assert(m_realChar);
	SetRealPhysObject(m_realChar);
}

void CharacterProxy::OnSyncCalls()
{
	Assert(m_realChar);

	for (unsigned int i = 0; i < m_calls.Size(); ++i)
	{
		m_calls[i]->Call(m_realChar);
		def_delete(m_calls[i]);
	}

	m_calls.Empty();
	m_changedPosition = false;
}

bool CharacterProxy::OnSyncRelease()
{
	return IProxy::ReleaseCounts(this);
	//if (m_bReleaseCall)
	//	return IPhysBase::Release();
	//return false;
}

void CharacterProxy::OnSyncTrace()
{
	TRACE_MCALLS
}
