#include "particlesystem.h"
#include "..\emitters\point.h"
#include "..\..\manager\particlemanager.h"




//Глобальный GUID для присваивания эмиттерам
DWORD EmitterID = 0;





//Создание/удаление 
ParticleSystem::ParticleSystem(BillBoardProcessor* processor) : Emitters (_FL_, 16)
{
	m_processor = processor;
	vAdditionalVelocity = 0.0f;
	delayedStart = 2;
	passed_frames_from_start = 0;


	bAutoHide = false;
	bCancelHide = false;
	EmissionPause = false;

	fTimeScale = 1.0f;
	fScale = 1.0f;

	fSystemTime = 0.0f;

}

ParticleSystem::~ParticleSystem()
{
	DeleteAllEmitters ();
}


float ParticleSystem::GetSystemTime ()
{
	return fSystemTime;
}

bool ParticleSystem::Release ()
{
	delete this;
	return false;
}

bool ParticleSystem::IsActive()
{
	if (EmissionPause) return false;

	if (bAutoHide == false || bCancelHide == true) return true;

	return false;

}

//Отработать всем партиклам
DWORD ParticleSystem::Execute (float DeltaTime)
{
	if (passed_frames_from_start < delayedStart)
	{
		passed_frames_from_start++;
		return 0;
	}
	



	if (EmissionPause) return Emitters.Size();


	//Испускать партиклы только если не прячется система сама
	//или если прячется но отменено на этом кадре...
	bool bAllStoped = true;
	if (bAutoHide == false || bCancelHide == true)
	{
		for (DWORD n = 0; n < Emitters.Size(); n++)
		{
			Emitters[n].pEmitter->Execute(DeltaTime, fTimeScale, fScale, vAdditionalVelocity);

			if (!Emitters[n].pEmitter->IsStoped()) bAllStoped = false;
		}

		bCancelHide = false;
	}

	if (!bAllStoped) fSystemTime += (DeltaTime * fTimeScale);

	
	//pMaster->Render()->DrawMatrix(matWorld, 2.0f);

	
	return Emitters.Size();
}



IEmitter* ParticleSystem::CreatePointEmitter (DataSource::EmitterDesc* pEmitter)
{

	
//	api->Trace("Create 'Point' emitter\n");
	PointEmitter* pNewEmitter = NEW PointEmitter(this, m_processor);
	pNewEmitter->AttachToDataSource(pEmitter);
	pNewEmitter->SetGUID (EmitterID);

	EmitterDesc NewEmitter;
	NewEmitter.Type = POINT_EMITTER;
	NewEmitter.pEmitter = pNewEmitter;
	Emitters.Add(NewEmitter);

	EmitterID+=GUIDSTEP;

	return pNewEmitter;
}


void ParticleSystem::DeleteAllEmitters ()
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		if (Emitters[n].pEmitter)
		{
			delete Emitters[n].pEmitter;
		}
	}

	Emitters.DelAll();
}



//Перезапустить партикловую систему  
void ParticleSystem::Restart (DWORD RandomSeed)
{
	fSystemTime = 0.0f;
	srand (RandomSeed);
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		Emitters[n].pEmitter->Restart();
	}

	PauseEmission (false);
}

//Запаузить испускание партиклов  
void ParticleSystem::PauseEmission (bool bPause)
{
	EmissionPause = bPause;
}

//Узнать на паузе эмиссия или нет  
bool ParticleSystem::IsEmissionPaused ()
{
	return EmissionPause;
}





//Установить матрицу трансформации для системы 
void ParticleSystem::SetTransform (const Matrix& transform)
{
	//api->Trace("particles: set transform");

	matWorld = transform;
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		Emitters[n].pEmitter->SetTransform (transform);
	}
}

void ParticleSystem::Teleport (const Matrix &transform)
{
	//api->Trace("particles: set transform");

	matWorld = transform;
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		Emitters[n].pEmitter->Teleport (transform);
	}
}

void ParticleSystem::GetTransform (Matrix& _matWorld)
{
	_matWorld = matWorld;
}


IEmitter* ParticleSystem::FindEmitterByData (FieldList* Data)
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		if (Emitters[n].pEmitter->GetData() == Data) return Emitters[n].pEmitter;
	}

	return NULL;
}

IEmitter* ParticleSystem::FindEmitter (const char* name)
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		if (crt_stricmp (Emitters[n].pEmitter->GetName(), name) == 0) return Emitters[n].pEmitter;
	}

	return NULL;
}


DWORD ParticleSystem::GetEmittersCount ()
{
	return Emitters.Size();
}

IEmitter* ParticleSystem::GetEmitterByIndex (DWORD Index)
{
	return Emitters[Index].pEmitter;
}

EmitterType ParticleSystem::GetEmitterTypeByIndex  (DWORD Index)
{
	return Emitters[Index].Type;
}

bool ParticleSystem::IsAlive ()
{
	DWORD pCount = 0;
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		pCount += Emitters[n].pEmitter->GetParticleCount();
		if (!Emitters[n].pEmitter->IsStoped())	return true;
	}

	if (pCount > 0) return true;

	return false;
}



void ParticleSystem::Editor_UpdateCachedData ()
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		Emitters[n].pEmitter->Editor_UpdateCachedData ();
	}
}


//автоматически спрячется на следующий кадр, если не вызвать CancelHide каждый кадр
void ParticleSystem::AutoHide (bool Enabled)
{
	bAutoHide = Enabled;
	bCancelHide = false;
}

void ParticleSystem::CancelHide()
{
	bCancelHide = true;
}


//Скорость времени для партикловой системы
void ParticleSystem::SetTimeScale (float _fTimeScale)
{
	fTimeScale = _fTimeScale;
}

//Размеры партикловой системы
void ParticleSystem::SetScale (float _fScale)
{
	fScale = _fScale;
}

void ParticleSystem::ReatachToDataSource (DataSource* pDataSource)
{
	DeleteAllEmitters ();

	if (!pDataSource) return;

	CreateFromDataSource(pDataSource);
}


void ParticleSystem::AdditionalStartVelocity (const Vector& additionalVelocity)
{
	vAdditionalVelocity = additionalVelocity;
}


void ParticleSystem::SetLocalMode (bool bForce)
{
	DWORD dwEmitersCount = Emitters.Size();
	for (DWORD n = 0; n < dwEmitersCount; n++)
	{
		Emitters[n].pEmitter->SetLocalMode(bForce);
	}
}
