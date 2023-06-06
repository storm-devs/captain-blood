#pragma once

#include "common.h"
#include "../Common_h/IConsole.h"
//#include "NxDebugRenderable.h"
//#include "NxStream.h"
#include <PxControllerManager.h>

class IProxy;
class PhysicsScene;

//Физический сервис
class PhysicsService : public IPhysics
{
	friend class PhysicsScene;

	// FIX_PX3 Need PxAllocatorCallback?
	/*
	class Allocator : public PxAllocatorCallback
	{
	public:
		virtual void * malloc(PxU32 size);
		virtual void * mallocDEBUG(PxU32 size,const char * fileName, int line);
		virtual void * realloc(void * memory, PxU32 size);
		virtual void free(void * memory);
	};
	*/

	class ErrorStream : public PxErrorCallback
	{
	public:
		ErrorStream();

		void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);
		void print(const char* message);
		void EnableWarnings(bool enable);

	private:
		bool enabledWarnings;// FIX_PX3 remove?
	};

	// вывод локализованной ошибки в лог
	void Error(long id, const char * errorEnglish);

public:

	// FIX_PX3 Don't need NxStream
	/*
	class MemoryReadStream : public NxStream
	{
	public:
		MemoryReadStream(const void * _source, dword _size);
		virtual ~MemoryReadStream();

		virtual PxU8 readByte() const;
		virtual PxU16 readWord() const;
		virtual PxU32 readDword() const;
		virtual float readFloat() const;
		virtual double readDouble() const;
		virtual void readBuffer(void * buffer, NxU32 size) const;

		virtual NxStream & storeByte(NxU8 b);
		virtual NxStream & storeWord(NxU16 w);
		virtual NxStream & storeDword(NxU32 d);
		virtual NxStream & storeFloat(PxReal f);
		virtual NxStream & storeDouble(NxF64 f);
		virtual NxStream & storeBuffer(const void * buffer, NxU32 size);

	private:
		template<class T> T & readData(T & v) const;

	private:
		const byte * source;
		dword sourceSize;
		mutable dword current;
	};

	class MemoryWriteStream : public NxStream
	{
	public:
		MemoryWriteStream();
		virtual ~MemoryWriteStream();

		virtual NxU8 readByte() const;
		virtual NxU16 readWord() const;
		virtual NxU32 readDword() const;
		virtual float readFloat() const;
		virtual double readDouble() const;
		virtual void readBuffer(void * buffer, NxU32 size) const;

		virtual NxStream & storeByte(NxU8 b);
		virtual NxStream & storeWord(NxU16 w);
		virtual NxStream & storeDword(NxU32 d);
		virtual NxStream & storeFloat(PxReal f);
		virtual NxStream & storeDouble(NxF64 f);
		virtual NxStream & storeBuffer(const void * buffer, NxU32 size);

		const void * Pointer();
		dword Size();

	private:
		template<class T> NxStream & writeData(const T & v);

	private:
		array<byte> data;
	};
	*/

public:
	PhysicsService();
	virtual ~PhysicsService();

public:
	//Создать сцену
	virtual IPhysicsScene * CreateScene();
	//Текущий режим работы
	virtual bool IsHardware();
	//Использование мультипоточности
	virtual bool IsMultiThreading();
	//Текущий режим дебаг инфы
	virtual bool IsEnableDebug();
	//Создать сетку, основываясь на бинарных данных
	virtual IPhysTriangleMesh * CreateTriangleMesh(const void * meshData, dword meshDataSize, const void * pMapData, dword pMapDataSize);
	//Создать построитель сеток ткани
	virtual IClothMeshBuilder* CreateClothMeshBuilder();
	//Удаление сетки из списка
	virtual void UnregistryPhysTriangleMesh(IPhysTriangleMesh * obj);
	virtual void UnregistryClothMeshBuilder(IClothMeshBuilder * obj);

	//Ищем прокси объект во всех сценах с данным актером
	IProxy * FindProxyObject(NxActor * actor);

	//Получить мэнеджер контролеров для персонажей
	PxControllerManager & CtrManager() { return *m_ctrManager; }

	void AddScene2Execute(PhysicsScene * scene);

private:
	//Инициализация
	virtual bool Init();
	//Исполнение в начале кадра
	virtual void StartFrame(float dltTime);
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime);

#ifndef STOP_DEBUG
	static bool m_gShowColliders;
	static bool m_gShowPhysBoxes;
	static bool m_gShowWorldPoint;
	static Vector m_gWorldPoint;
	static long m_gMaterialIndex;

	static void _cdecl Console_ShowNull(const ConsoleStack & params);
	static void _cdecl Console_ShowColliders(const ConsoleStack & params);
	static void _cdecl Console_ShowPhysBoxes(const ConsoleStack & params);

	//Отрисовка статистики по сцене
	void DrawSceneStat(unsigned int index);
#endif

	static dword __stdcall SimulationThread(void * ptr);

private:
	HANDLE m_hThread;
	HANDLE m_hSimulateStartEvent;
	HANDLE m_hExitEvent;

	array<PhysicsScene*> m_scenes2Execute;
	CritSection	csArrayUpdate;
	CritSection	csSimulate;
	PxPhysics* physicsSDK;
	PxFoundation* m_Foundation;
	PxCooking* m_Cooking;
	PxControllerManager* m_ctrManager;
	PxAllocatorCallback* allocator;
	ErrorStream errorStream;
	array<PhysicsScene *> scenes;
	array<IPhysTriangleMesh*> meshes;
	array<IClothMeshBuilder*> builders;
	bool isEnableDebug;
	bool isHardware;
	bool isStop;
	bool isMultiThreading;
	IConsole* console;
};

