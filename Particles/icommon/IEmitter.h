#ifndef EMITTER_INTERFACE
#define EMITTER_INTERFACE


#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../common_h/core.h"
#include "../../common_h/templates.h"
#include "../../common_h/math3d.h"
#include "../system/DataSource/FieldList.h"



class IParticleSystem;

class IEmitter
{

protected:

	bool bForceLocalMode;


public:
 
	// Конструктор / деструктор
  IEmitter()
  {
	  bForceLocalMode = false;
  };
  virtual ~IEmitter() {};
  
  
	//Родить партиклы, используеться при движении привязанного эмиттера
	virtual void BornParticles (float DeltaTime, float fTimeScale, float fScale, const Vector& additionalVelocity) = 0;

	//Исполнить
	virtual void Execute (float DeltaTime, float fTimeScale, float fScale, const Vector& additionalVelocity) = 0;

	virtual void Restart () = 0;

	virtual DWORD GetParticleCount () = 0;

	virtual bool IsStoped () = 0;

	virtual void SetTransform (const Matrix &matWorld) = 0;
	virtual void Teleport (const Matrix &matWorld) = 0;


	virtual const char* GetName () = 0;

	//Если флаг в true емиттер не будет самостоятельно испускать партиклы
	//так, как он привязан
	virtual void SetAttachedFlag (bool Flag) = 0;
	virtual bool IsAttached () = 0;

	//Запомнить/восстановить тек.время эмиттера
	virtual float GetTime () = 0;
	virtual void SetTime (float Time) = 0;



	virtual DWORD GetParticleTypesCount () = 0;
	virtual FieldList* GetParticleTypeDataByIndex (DWORD Index) = 0;
	virtual ParticleType GetParticleTypeByIndex  (DWORD Index) = 0;

	virtual FieldList* GetData () = 0;


	virtual bool SetEnable (bool bVisible) = 0;
	virtual bool GetEnable () = 0;

	//-1 если не нашли, иначе индекс
	virtual int GetParticleTypeIndex (FieldList* pFields) = 0;
	virtual bool SetParticleTypeEnable (bool bVisible, DWORD Index) = 0;
	virtual bool GetParticleTypeEnable (DWORD Index) = 0;


	virtual void Editor_UpdateCachedData () = 0;


	virtual void SetName (const char* Name) = 0;

	virtual Matrix& GetTransform () = 0;


	virtual void GetSystemTransform (Matrix& trans) = 0;


	//virtual void SetLocalMode (bool bForce) = 0;

	__forceinline void SetLocalMode (bool bForce)
	{
		bForceLocalMode = bForce;
	}




};

#endif