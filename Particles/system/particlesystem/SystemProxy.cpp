#include "SystemProxy.h"
#include "..\emitters\point.h"
#include "..\..\manager\particlemanager.h"





//Создание/удаление 
ParticleSystemProxy::ParticleSystemProxy(ParticleManager* serv, ParticleSystem* async_system)
{
	m_async_system = async_system;

	AutoDeleted = false;

	pMaster = serv;

	DeleteWithGeomEntity = true;
	AttachedToGeom.reset();

	pAttachedScene = NULL;
	AutoDeleted = false;
	pMaster = serv;
	DeleteWithGeomEntity = true;

	packet.out.mTransformType = MOVE_NONE;
	packet.out.bAutoHide = false;
	packet.out.bCancelHide = false;
	packet.out.EmissionPause = false;
	packet.out.fTimeScale = 1.0f;
	packet.out.fScale = 1.0f;
	packet.out.addVelocity = 0.0f;
	packet.out.bLocalMode = false;

	packet.in.bIsAlive = true;

	UseTheTeleport = true;

}

ParticleSystemProxy::~ParticleSystemProxy()
{
	if (pAttachedScene)	pAttachedScene->UnSubscribeDeletionEvent(this);
	pAttachedScene = NULL;
	AttachedToGeom.reset();
}

bool ParticleSystemProxy::DeleteIfNeed ()
{
	if (!IsAlive())
	{
		pMaster->DefferedDelete(this);
		return true;
	}
	return false;
}


void _cdecl ParticleSystemProxy::GeomEntityDeleted ()
{
	AttachedToGeom.reset();
	if (DeleteWithGeomEntity)
	{
		Release();
	}
	pAttachedScene = NULL;
}


bool ParticleSystemProxy::Release ()
{
	if (pAttachedScene)	pAttachedScene->UnSubscribeDeletionEvent(this);
	pAttachedScene = NULL;
	AttachedToGeom.reset();

	pMaster->RemoveResource(this);

	return false;
}

void ParticleSystemProxy::Restart (DWORD RandomSeed)
{
	UseTheTeleport = true;

	packet.in.bIsAlive = true;
	packet.out.bNeedRestart = true;
	packet.out.dwRestartRandomSeed = RandomSeed;
}

void ParticleSystemProxy::PauseEmission (bool bPause)
{
	packet.out.EmissionPause = bPause;
}


//Установить автоудаляемая система или обычная...  
void ParticleSystemProxy::AutoDelete (bool Enabled)
{
	AutoDeleted = Enabled;
}

//Установить матрицу трансформации для системы 
void ParticleSystemProxy::SetTransform (const Matrix& transform)
{
	packet.out.mTransformType = MOVE_SETTRANSFORM;
	packet.out.mTransform = transform;
}

void ParticleSystemProxy::Teleport (const Matrix &transform)
{
	packet.out.mTransformType = MOVE_TELEPORT;
	packet.out.mTransform = transform;
}


void  ParticleSystemProxy::SetName (const char* Name)
{
	systemName = Name;
}


const char* ParticleSystemProxy::GetName ()
{
	return systemName.c_str();
}


//Прикрепить систему к геометрии  
void ParticleSystemProxy::AttachTo (IGMXScene* scene, GMXHANDLE node, bool bDestroyWithGeometry)
{
	if (pAttachedScene)	pAttachedScene->UnSubscribeDeletionEvent(this);
	pAttachedScene = NULL;
	AttachedToGeom.reset();

	if (scene && node.isValid())
	{
		AttachedToGeom = node;
		pAttachedScene = scene;
		scene->SubscribeDeletionEvent(this, (GMX_EVENT)&ParticleSystemProxy::GeomEntityDeleted);
		DeleteWithGeomEntity = bDestroyWithGeometry;
	} else
	{
		DeleteWithGeomEntity = false;
	}
}


//автоматически спрячется на следующий кадр, если не вызвать CancelHide каждый кадр
void ParticleSystemProxy::AutoHide (bool Enabled)
{
	packet.out.bAutoHide = Enabled;
	packet.out.bCancelHide = false;
}

void ParticleSystemProxy::CancelHide()
{
	packet.out.bCancelHide = true;
}


//Скорость времени для партикловой системы
void ParticleSystemProxy::SetTimeScale (float _fTimeScale)
{
	packet.out.fTimeScale = _fTimeScale;
}

//Размеры партикловой системы
void ParticleSystemProxy::SetScale (float _fScale)
{
	packet.out.fScale = _fScale;
}


void ParticleSystemProxy::AdditionalStartVelocity (const Vector& additionalVelocity)
{
	packet.out.addVelocity = additionalVelocity;
}

bool ParticleSystemProxy::IsActive()
{
	if (packet.out.EmissionPause) return false;

	if (packet.out.bAutoHide == false || packet.out.bCancelHide == true) return true;

	return false;
}

void ParticleSystemProxy::SendReceivePackets ()
{
	if (AttachedToGeom.isValid() && pAttachedScene)
	{
		Matrix m;
		pAttachedScene->GetNodeWorldTransform(AttachedToGeom, m);
		SetTransform(m);
	}


	if (packet.out.bNeedRestart)
	{
		m_async_system->Restart(packet.out.dwRestartRandomSeed);
		packet.out.bNeedRestart = false;
	}


	if (AutoDeleted)
	{
		if (DeleteIfNeed ())
		{
			return;
		}
	}

	//receive...
	packet.in.bIsAlive = m_async_system->IsAlive();

	//send...
	m_async_system->AutoHide(packet.out.bAutoHide);

	if (packet.out.bCancelHide)
	{
		m_async_system->CancelHide();
	} 
	packet.out.bCancelHide = false;

	m_async_system->PauseEmission(packet.out.EmissionPause);
	m_async_system->SetTimeScale(packet.out.fTimeScale);
	m_async_system->SetScale(packet.out.fScale);
	m_async_system->AdditionalStartVelocity(packet.out.addVelocity);

	

	m_async_system->SetLocalMode(packet.out.bLocalMode);


	if (packet.out.mTransformType == MOVE_TELEPORT)
	{
		m_async_system->Teleport(packet.out.mTransform);
	}

	if (packet.out.mTransformType == MOVE_SETTRANSFORM)
	{
		if (UseTheTeleport)
		{
			m_async_system->Teleport(packet.out.mTransform);
			UseTheTeleport = false;
		} else
		{
			m_async_system->SetTransform(packet.out.mTransform);
		}
	}

	packet.out.mTransformType = MOVE_NONE;


}


ParticleSystem* ParticleSystemProxy::system_GetGUID_dontUSE_for_WORK_use_LOCK ()
{
	return m_async_system;
}

ParticleSystem* ParticleSystemProxy::Lock ()
{
#ifdef ENABLE_PARTICLE_THREADS
	pMaster->Lock();
#endif
	return m_async_system;
}

void ParticleSystemProxy::Unlock ()
{
#ifdef ENABLE_PARTICLE_THREADS
	pMaster->Unlock();
#endif
}

IGMXScene * ParticleSystemProxy::GetAttachedSceneToGeom()
{
	return pAttachedScene;
}

GMXHANDLE ParticleSystemProxy::GetAttachedToGeom ()
{
	return AttachedToGeom;
}

bool ParticleSystemProxy::IsAutoDeleted ()
{
	return AutoDeleted;
}

bool ParticleSystemProxy::IsAlive ()
{
	return packet.in.bIsAlive;
}


bool ParticleSystemProxy::IsLooped ()
{
	return false;
}


void ParticleSystemProxy::SetLocalMode (bool bLocalModeEnable)
{
	packet.out.bLocalMode = bLocalModeEnable;

}