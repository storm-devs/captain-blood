#ifndef _PARTICLE_SYSTEM_H_
#define _PARTICLE_SYSTEM_H_

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../../common_h/core.h"
#include "..\..\..\common_h\templates.h"
#include "..\..\..\common_h\gmx.h"
#include "..\..\..\common_h\particles\iparticlesystem.h"
#include "..\..\\icommon\types.h"
#include "..\datasource\datasource.h"



class IEmitter;
class ParticleManager;
class BillBoardProcessor;

class ParticleSystem
{
	struct EmitterDesc
	{
		EmitterType Type;
		IEmitter* pEmitter;
	};

	array<EmitterDesc> Emitters;

	
	// Не рождать новые партиклы !!!
	bool EmissionPause;		


	string SystemName;

	long delayedStart;
	long passed_frames_from_start;

	Vector vAdditionalVelocity;
	

protected:

	virtual ~ParticleSystem();


	IEmitter* CreatePointEmitter (DataSource::EmitterDesc* pEmitter);

	void DeleteAllEmitters ();


	Matrix matWorld;

	bool bAutoHide;
	bool bCancelHide;

	float fTimeScale;
	float fScale;

	float fSystemTime;

	BillBoardProcessor* m_processor;

public:

//Создание/удаление 
	ParticleSystem(BillBoardProcessor* processor);
	virtual bool Release ();

//Отработать всем партиклам
  virtual DWORD Execute (float DeltaTime);


//Перезапустить партикловую систему  
  virtual void Restart (DWORD RandomSeed);

	virtual float GetSystemTime ();
  
//Запаузить испускание партиклов  
  virtual void PauseEmission (bool bPause);
//Узнать на паузе эмиссия или нет  
  virtual bool IsEmissionPaused ();


//Прикрепить систему к геометрии  
	//virtual void AttachTo (IGMXEntity* ent, bool bDestroyWithGeometry);
//Установить матрицу трансформации для системы 
	virtual void SetTransform (const Matrix& transform);
	virtual void GetTransform (Matrix& _matWorld);

	//автоматически спрячется на следующий кадр, если не вызвать CancelHide каждый кадр
	virtual void AutoHide (bool Enabled);

	virtual void CancelHide();


	//Установить дополнительну начальную скорость
	virtual void AdditionalStartVelocity (const Vector& additionalVelocity);




	__forceinline void CreateFromDataSource (DataSource* pDataSource)
	{
		int EmitterCount = pDataSource->GetEmitterCount();
		if (EmitterCount <= 0) return;

		for (int n = 0; n < EmitterCount; n++)
		{
			DataSource::EmitterDesc* pEmitterDecription = pDataSource->GetEmitterDesc(n);
			IEmitter* pEmitter = NULL;
			switch (pEmitterDecription->Type)
			{
			case POINT_EMITTER:
				pEmitter = CreatePointEmitter (pEmitterDecription);
				break;
			default:
				throw ("Particles: Unknown emitter type !!!!");
			}

			//Assert (pEmitter);
			//CreateParticles (pEmitter, pEmitterDecription->Particles);
		}
	}



	IEmitter* FindEmitter (const char* name);
	IEmitter* FindEmitterByData (FieldList* Data);
	

	void Teleport (const Matrix &transform);

	bool IsAlive ();


public:
	DWORD GetEmittersCount ();
	IEmitter* GetEmitterByIndex (DWORD Index);
	EmitterType GetEmitterTypeByIndex  (DWORD Index);

	void Editor_UpdateCachedData ();


	//IGMXEntity* GetAttachedToGeom ();

	bool IsActive();

	//Скорость времени для партикловой системы
	virtual void SetTimeScale (float _fTimeScale);

	//Размеры партикловой системы
	virtual void SetScale (float _fScale);


	virtual void ReatachToDataSource (DataSource* pDataSource);


	void SetLocalMode (bool bForce);

	



};

#endif