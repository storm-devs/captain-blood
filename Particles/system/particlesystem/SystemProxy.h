#ifndef _PARTICLE_SYSTEM_PROXY_H_
#define _PARTICLE_SYSTEM_PROXY_H_

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
class ParticleSystem;

class ParticleSystemProxy : public IParticleSystem
{
	bool UseTheTeleport;

	//Автоудаляемая система или обычная
	bool AutoDeleted;  

	ParticleManager* pMaster;


	bool DeleteWithGeomEntity;

	IGMXScene * pAttachedScene;
	GMXHANDLE AttachedToGeom;

	ParticleSystem* m_async_system;

	bool DeleteIfNeed ();

	string systemName;

	enum MoveType
	{
		MOVE_NONE = 0,
		MOVE_TELEPORT = 1,
		MOVE_SETTRANSFORM = 2,

		MOVE_FORCE_DWORD = 0x7fffffff
	};

	struct SyncSend
	{
		bool bNeedRestart;
		dword dwRestartRandomSeed;

		bool bAutoHide;
		bool bCancelHide;
		bool EmissionPause;
		float fTimeScale;
		float fScale;
		
		Vector addVelocity;

		MoveType mTransformType;
		Matrix mTransform;

		bool bLocalMode;
	};

	struct SendReceive
	{
		bool bIsAlive;
	};

	struct SyncData
	{
		SyncSend out;
		SendReceive in;
	};


	SyncData packet;


public:

	ParticleSystemProxy(ParticleManager* serv, ParticleSystem* async_system);
	virtual ~ParticleSystemProxy();

	void _cdecl GeomEntityDeleted ();


	virtual bool Release ();

	//Отработать всем партиклам
	//virtual DWORD Execute (float DeltaTime) = 0;


	//Перезапустить партикловую систему  
	virtual void Restart (DWORD RandomSeed);

	//Запаузить испускание партиклов  
	virtual void PauseEmission (bool bPause);

	//Узнать на паузе эмиссия или нет  
	//virtual bool IsEmissionPaused () = 0;
	//Установить автоудаляемая система или обычная...  
	virtual void AutoDelete (bool Enabled);

	//автоматически спрячется на следующий кадр, если не вызвать CancelHide каждый кадр
	virtual void AutoHide (bool Enabled);

	virtual void CancelHide();

	//Прикрепить систему к геометрии  
	virtual void AttachTo (IGMXScene* scene, GMXHANDLE node, bool bDestroyWithGeometry);
	//Узнаять автоудаляемая система или нет  
	virtual bool IsAutoDeleted ();
	//Установить матрицу трансформации для системы 
	virtual void SetTransform (const Matrix& transform);
	//virtual void GetTransform (Matrix& _matWorld) = 0;

	virtual void Teleport (const Matrix &transform);

	virtual bool IsAlive ();

	virtual const char* GetName ();
	virtual void SetName (const char* Name);

	//Скорость времени для партикловой системы
	virtual void SetTimeScale (float _fTimeScale);

	//Размеры партикловой системы
	virtual void SetScale (float _fScale);

	//Установить дополнительну начальную скорость
	virtual void AdditionalStartVelocity (const Vector& additionalVelocity);

	virtual void SendReceivePackets ();

	virtual bool IsActive();


	IGMXScene * GetAttachedSceneToGeom();
	GMXHANDLE GetAttachedToGeom ();

	ParticleSystem* Lock ();
	void Unlock ();



	ParticleSystem* system_GetGUID_dontUSE_for_WORK_use_LOCK ();


	virtual bool IsLooped ();


	virtual void SetLocalMode (bool bLocalModeEnable);


};

#endif