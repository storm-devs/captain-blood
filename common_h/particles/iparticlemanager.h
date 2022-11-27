//*
//****************************************************************

#ifndef PARTICLES_MANAGER_INTERFACE
#define PARTICLES_MANAGER_INTERFACE

#include "..\core.h"
#include "..\math3d.h"

class ParticleService;
class IFileService;
class IRender;
class IBaseTexture;
class IParticleSystem;
class Matrix;


#ifndef NULL
#define NULL 0L
#endif


#define CreateParticleSystem(name) CreateParticleSystemEx(name, __FILE__, __LINE__)
#define CreateParticleSystemE(name, mWorld, bAutoDelete) CreateParticleSystemEx2(name, mWorld, bAutoDelete, __FILE__, __LINE__)


typedef bool (_cdecl Object::*FORCEFIELD_PROCESS)(dword dwGUID, Vector& old_pos, Vector& pos, Vector& externalforce);


//Менеджер партикловых систем
class IParticleManager
{


protected:

	virtual ~IParticleManager () {};

public:

//Создание/удаление 
	IParticleManager (ParticleService* service) {};
	virtual bool Release () = 0;

	virtual IFileService* Files () = 0;

//Удалить из списка ресурсов (системная)
	virtual void RemoveResource (IParticleSystem* pResource) = 0;

//Начать обновлять партиклы (для многопоточной системы) и обновить их для однопоточной
	virtual void Execute (float DeltaTime) = 0;

//Узнать доступна система или нет 
	virtual bool IsSystemAvailable (const char* FileName) = 0;

	//Нарисовать только простые партиклы (для отражений и прочей ерунды использовать)
	virtual bool Draw () = 0;

	//Нарисовать все партиклы (с дисторшеном) вызывать из мисии надо...
	virtual bool DrawAllParticles () = 0;

 //Создать пустую партикловую систему, для редактора...
 virtual IParticleSystem* CreateEmptyParticleSystemEx  (const char* FileName, int Line) = 0;


//Создать партикловую систему из файла (файл должен быть в проекте!!!!!)
 virtual IParticleSystem* CreateParticleSystemEx (const char* FileName, const char* File, int Line) = 0;

 //Создать партикловую систему из файла (файл должен быть в проекте!!!!!)
 virtual IParticleSystem* CreateParticleSystemEx2 (const char* FileName, const Matrix& mWorld, bool bAutoDelete, const char* File, int Line) = 0;


 //Проверить "валиден" ли указатель на систему партиклов, вдруг она уже удалилась
 virtual bool ValidateSystem (IParticleSystem* pSystem) = 0;


 virtual void Editor_UpdateCachedData () = 0;

 virtual void WriteSystemCache (const char* FileName) = 0;

	
};


#endif