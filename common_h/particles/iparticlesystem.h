#ifndef PARTICLE_SYSTEM_INTERFACE
#define PARTICLE_SYSTEM_INTERFACE

#include "..\math3d.h"
#include "iparticlemanager.h"

#include "..\render\ShaderID.h"
#include "..\gmx.h"


class IParticleSystem : public Object
{

protected:

	virtual ~IParticleSystem() {};

public:

//Создание/удаление 
	IParticleSystem() {};
	virtual bool Release () = 0;

//Отработать всем партиклам
  //virtual DWORD Execute (float DeltaTime) = 0;


//Перезапустить партикловую систему  
  virtual void Restart (DWORD RandomSeed) = 0;
  
//Запаузить испускание партиклов  
  virtual void PauseEmission (bool bPause) = 0;
  
//Узнать на паузе эмиссия или нет  
  //virtual bool IsEmissionPaused () = 0;

	virtual bool IsActive() = 0;
//Установить автоудаляемая система или обычная...  
  virtual void AutoDelete (bool Enabled) = 0;

//автоматически спрячется на следующий кадр, если не вызвать CancelHide каждый кадр
	virtual void AutoHide (bool Enabled) = 0;

	virtual void CancelHide() = 0;

//Прикрепить систему к геометрии  
	virtual void AttachTo (IGMXScene* scene, GMXHANDLE node, bool bDestroyWithGeometry) = 0;
//Узнаять автоудаляемая система или нет  
	virtual bool IsAutoDeleted () = 0;
//Установить матрицу трансформации для системы 
	virtual void SetTransform (const Matrix& transform) = 0;
	//virtual void GetTransform (Matrix& _matWorld) = 0;

	virtual void Teleport (const Matrix &transform) = 0;

	virtual bool IsAlive () = 0;

	virtual const char* GetName () =0;

	//Скорость времени для партикловой системы
	virtual void SetTimeScale (float _fTimeScale) = 0;

	//Размеры партикловой системы
	virtual void SetScale (float _fScale) = 0;

	//Установить дополнительну начальную скорость
	virtual void AdditionalStartVelocity (const Vector& additionalVelocity) = 0;


	virtual bool IsLooped () = 0;


	virtual void SetLocalMode (bool bLocalModeEnable) = 0;


};

#endif