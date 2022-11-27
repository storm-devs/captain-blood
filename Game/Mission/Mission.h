//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Mission	
//============================================================================================

#ifndef _Mission_h_
#define _Mission_h_

#include "..\..\Common_h\Mission.h"
#include "ObjectsFindManager.h"
#include "MissionObjectsList.h"

//#define MISSSION_GROUP_TIMESCALE	"_sys_Mis_Time_Scale_"

class MissionLoader;
class MissionIterator;
class MissionsManager;
class ISoundService;

class Mission : public IMission  
{
	friend MissionLoader;
	friend MissionIterator;
	friend MissionsManager;

	struct GroupElement
	{
		MissionObject * object;
		MOF_EVENT func;
	};

	struct Group
	{
		GroupId id;
		Group * next;
		MissionObjectsList list;
		ObjectsFindManager findManager;
	};

	struct Element
	{
		MissionObject * object;
		MOF_UPDATE funcUpdate;
		long level;
		long isDelete;
#ifndef STOP_PROFILES
		__int64 count;
		unsigned __int64 max;
		double sum;
#endif
	};

	struct ProfileObject
	{
		MissionObject * object;
		double max;
		double sum;
	};

	struct ProfileType
	{
		const char * type;
		double max;
		double sum;
		dword counter;				
	};

	struct ProfileCreation
	{
		unsigned __int64 time;
		const char * type;
		const char * id;
	};

	class ParticlesUpdater : public MissionObject
	{
	public:
		void _cdecl UpdateParticles(float dltTime, long level);
		void _cdecl DrawParticles(float dltTime, long level);
	};

	class PhysicsUpdater : public MissionObject
	{
	public:
		void _cdecl StartFrame(float dltTime, long level);
		void _cdecl EndFrame(float dltTime, long level);
	};

	class AnimationUpdater : public MissionObject
	{
	public:
		void _cdecl UpdateAnimation(float dltTime, long level);
	public:
		IAnimationScene * scene;
		IAnimationService * srv;
	};

	class PostEffectsUpdater : public MissionObject
	{
	public:
		void _cdecl PreparePostEffects(float dltTime, long level);
		void _cdecl DrawPostEffects(float dltTime, long level);
	};

	struct ValidateCache
	{
		MissionObject * obj;
		dword useCounter;
	};

	class FakeConsole : public IConsole
	{
		virtual void  RunStart(){};
		virtual void  RunEnd(){};

		virtual bool Init(){ return false; };

		virtual void _cdecl Trace(ConsoleOutputLevel Level, const char* FormatString,...){};

		virtual void ExecuteCommand(const char* szCommand){};

		virtual void RegisterCommand(const char* CommandName, const char* HelpDesc, Object* _class, CONSOLE_COMMAND _method){};
		virtual void Register_PureC_Command(const char* CommandName, const char* HelpDesc, PURE_C_CONSOLE_COMMAND _method){};
		virtual void UnregisterCommand(const char* CommandName){};


		virtual dword GetTextColor (){ return 0; };
		virtual dword SetTextColor (dword dwNewColor){ return 0; };

		virtual bool IsHided() { return true; };
	};

	struct TypeElement
	{
		char * type;
		dword hash;
		long next;
	};

	struct FastFinder
	{
		FastFinder() : mo(_FL_)
		{
		};

		array<MissionObject *> mo;
	};

	typedef void (Mission::* PrecacheCreator)(const char * name);

//--------------------------------------------------------------------------------------------
public:
	Mission();
	virtual ~Mission();

	//Получить текущую миссию
	//static inline IMission * GetCurrent(){ return current; };


	virtual bool EditMode_IsAdditionalDraw();
	virtual void EditMode_AdditionalDraw(bool value);


//---------------------------------------------------------------------------------------
//Управление миссией
//---------------------------------------------------------------------------------------
private:
	//Заргузить миссию
	virtual bool CreateMission(const char * missionName, float percentsPerMission, float & percentsCounter);
	void PrecachePack(const char * packPath, PrecacheCreator creator, float filePerc, float dataPerc, dword & loadTime, dword & creationTime);
	void PrecacheCreator_Textures(const char * name);
	void PrecacheCreator_Animation(const char * name);
	void PrecacheCreator_Models(const char * name);
	void LoadingProgress(float delta);
	//Удалить миссию
	virtual void DeleteMission();
	//Получить имя загруженой миссии
	virtual const char * GetMissionName();
	//Закончен ли процесс загрузки
	bool IsLoadDone();
public:
	//Добавить в список модификатор времени
	void AddTimeModifier(float * modifier);
	//Добавить из списока модификатор времени
	void RemoveTimeModifier(float * modifier);
	
	//Пауза миссии
	void MissionPause(bool isPause);

//---------------------------------------------------------------------------------------
//Функции для редактора миссий
//---------------------------------------------------------------------------------------
#ifndef NO_TOOLS

public:	
	//Подключить пак для редактора если нет, то создать
	virtual void EditorSetPack(const char * missionName);
	//Получить путь до пак-файла миссии
	virtual void EditorGetPackPath(string & path);
	//Получить путь до файла миссии .mis, чтобы сохранить его
	virtual void EditorGetMisPath(string & path);
	//Усыпить/разбудить миссию
	virtual void EditorSetSleep(bool isSleep);
	//Обновить объект
	virtual void EditorUpdateObject(MissionObject * mo, MOPWriter & writer);
	//Нарисовать миссию
	virtual void EditorDraw(float dltTime);

#endif

//---------------------------------------------------------------------------------------
//Управление объектами миссии
//---------------------------------------------------------------------------------------
public:	
	//Перезапуск всех объектов миссии
	void RestartAllObjects();

private:
	//Найти объект по идентификатору
	virtual MissionObject * FindObject(const ConstString & id);
	//Изменить состояние видимости объекта
//	virtual MissionObject * ObjectShow(const char * id, bool isShow);
	//Изменить состояние активности объекта
//	virtual MissionObject * ObjectActivate(const char * id, bool isActive);
	//Получить итератор по группе
	virtual MGIterator & GroupIterator(GroupId group, const char* cppFileName, long cppFileLine);
	//Изменить состояние видимости группы
	virtual void GroupShow(GroupId group, bool isShow);
	//Изменить состояние активности группы
	virtual void GroupActivate(GroupId group, bool isActive);
	//Получить игрока
	virtual MissionObject * Player();
	//Получить матрицу качки
	virtual const Matrix & GetSwingMatrix();
	//Получить инверсную матрицу качки
	virtual const Matrix & GetInverseSwingMatrix();
	//Получить параметры качки
	virtual const Vector & GetSwingParams();
	//Установить параметры качки
	virtual void SetSwingParams(const Vector & angs);

//--------------------------------------------------------------------------------------------
private:
	//Инициализация
	bool Init();
	//Создать миссионный объект в режиме игры
	bool CreateObjectEx(const char * name, MOPReader & rd, const void * initData, long initDataSize);
	//Отрисовка
	void __fastcall FrameUpdate(float dltTime);
	//Запустить встроенный профайлинг
	void StartProfile();
	//Остановить встроенный профайлинг
	void StopProfile();
	static bool CompareProfileElements(Element * const & grtElm, Element * const & lesElm);	
	static bool CompareProfileObjects(const ProfileObject & grtElm, const ProfileObject & lesElm);
	static bool CompareProfileTypes(const ProfileType & grtElm, const ProfileType & lesElm);	
	static bool CompareProfileCreations(const ProfileCreation & grtElm, const ProfileCreation & lesElm);
	//Подвести итоги профайла загрузки
	void CreationProfile();
	//Найти группу по имени
	Group * FindGroup(GroupId group);
	//Добавить миссионный объект в таблицу быстрого поиска
	void AddMissionObjectToFFTable(MissionObject * mo);
	//Удалить миссионный объект из таблицы быстрого поиска
	void RemoveMissionObjectFromFFTable(MissionObject * mo);
	//Получить индекс в таблице поизка из хэша
	long GetIndexInFFTableFromHash(long hash);
	//Установить объекту уникальный номер
	void SetObjectUId(MissionObject * mo);

//--------------------------------------------------------------------------------------------
//Консольные команды
//--------------------------------------------------------------------------------------------
private:
#ifndef NO_CONSOLE
	static void _cdecl Console_ActivateObject(const ConsoleStack & params);
	static void _cdecl Console_DeactivateObject(const ConsoleStack & params);
	static void _cdecl Console_ShowObject(const ConsoleStack & params);
	static void _cdecl Console_HideObject(const ConsoleStack & params);
	static void _cdecl Console_CommandToObject(const ConsoleStack & params);
	static void _cdecl Console_ViewStateObject(const ConsoleStack & params);
	static void _cdecl Console_OnLogicDebug(const ConsoleStack & params);
	static void _cdecl Console_OffLogicDebug(const ConsoleStack & params);
	static void _cdecl Console_TraceLogicDebug(const ConsoleStack & params);
	static void _cdecl Console_AddDebugObject(const ConsoleStack & params);
	static void _cdecl Console_DelDebugObject(const ConsoleStack & params);
	static void _cdecl Console_ViewDebugObjects(const ConsoleStack & params);
	static void _cdecl Console_ClearDebugObjects(const ConsoleStack & params);
	static void _cdecl Console_ShowSoundsDebug(const ConsoleStack & params);
	static void _cdecl Console_Storage(const ConsoleStack & params);
	static void _cdecl Console_StorageView(const ConsoleStack & params);
	static void _cdecl Console_StorageSet(const ConsoleStack & params);
	static void _cdecl Console_StorageDel(const ConsoleStack & params);
	static void _cdecl Console_StorageCopy(const ConsoleStack & params);
	static void _cdecl Console_FreeCamera(const ConsoleStack & params);
	static void _cdecl Console_TraceUnuse(const ConsoleStack & params);
	static bool Console_NoExecute();
	static MissionObject * Console_GetMissionObject(const ConsoleStack & params);
#endif

//--------------------------------------------------------------------------------------------
private:
	virtual MissionObject * sysCreateObject(const char * type, const ConstString & id, bool alone);
	virtual MissionObject * sysCreateObject(const char * type, MOPReader & rd);
	const char * CreateObject_AddType(const char * type);
	virtual void sysRegistry(GroupId group, MissionObject * object, MOF_EVENT func, long level);
	virtual void sysUnregistry(GroupId group, MissionObject * object);
	virtual void sysUnregistryAll(MissionObject * object);	
	virtual void sysEvent(GroupId group, MissionObject * object);
	virtual void sysSetUpdate(long level, MissionObject * obj, MOF_UPDATE func);
	virtual void sysDelUpdate(long level, MissionObject * obj, MOF_UPDATE func);
	virtual void sysDelUpdate(MissionObject * obj, MOF_UPDATE func);	
	virtual void sysDeleteObject(MissionObject * object);
	virtual void sysUpdateObjectID(MissionObject * obj, const char * oldId, const char * newId);
	virtual void sysLogicDebug(MissionObject * mo, const char * format, void * data, bool isError);
	virtual void sysLogicDebugLevel(bool increase);
	virtual IMissionQTObject * sysQTCreateObject(GroupId group, MissionObject * mo, const char* cppFileName, long cppFileLine);
	virtual dword sysQTFindObjects(GroupId group, Vector minVrt, Vector maxVrt);
	virtual IMissionQTObject * sysQTGetObject(dword index);
	virtual void sysQTDraw(GroupId group, float levelScale);
	virtual void sysQTDump(GroupId group);
	virtual bool sysFindObject(const ConstString * id, MissionObject * & object, dword & uid);
	virtual bool sysValidatePointer(MissionObject * object, dword uid, long & hashIndex);

private:
	//Игрок
	MissionObject * player;
	//Загрузчик миссии
	MissionLoader * loader;
	//Путь до миссии
	IMirrorPath * mirrorPath;
	//Данные, описывающие данную миссию
	ILoadBuffer * dataBuffer;
	//Класифицированные списки объектов
	array<Group *> group;
	//Группа созданных объектов
	MissionObjectsList objectsList;
	//Группа исполнения
	MissionObjectsList executeList;
	//Итератор исполнения
	MGIterator * executeIterator;
	//Входная таблица
	Group * entryGroups[1024];
	//Модификаторы времени
	array<float *> timeModifiers;
	//Имя текущей миссии
	string name;
	//Функции обновления
	ParticlesUpdater * particlesUpdater;
	PhysicsUpdater * physicsUpdater;
	AnimationUpdater * animationUpdater;
	PostEffectsUpdater * postEffectsUpdater;
	//Кешь проверки на валидейтность
	ValidateCache validateCache[8];
	dword validateCacheCounter;
	//Профайлинг
	long profileLevel;
	//Уровень смещения строк вывода
	string logicDebugLevelString;
	//Режим засыпания
	bool isSleep;
	//Режим паузы
	bool isMissionPause;
	bool isProcessMissionPause;
	//Остановлен ли звук
	long missionPauseCount;
	//Отладочная информация
	bool enableDebug;
	//Текущий режим
	bool isEditMode;
	
	
	//Чёртов Серёга пустил свои сосиски в мой код  %D
	// да-да-да и еще раз сюда же сосиски свои пущу я
	bool bEditModeAdditionalDraw;
	dword framesCounter;
	

	//Список для поиска
	array<IMissionQTObject *> objectsFindList;

	//Массив зарегистрированных типов
	array<TypeElement> regObjectTypes;
	long regObjectTypesEntry[32];
	//Качка
	Matrix swingMatrix;
	Matrix swingInverseMatrix;
	Vector swingAngs;
	//Фейковая консоль
	FakeConsole fakeConsole;

	//Таблица для быстрого поиска
	FastFinder fastFindTable[1024];

	//Таблицы предзагруженных ресурсов
	array<IBaseTexture *> textures;
	array<IGMXScene *> models;
	array<IAnimation *> animations;
	array<IPhysPlane *> boundPlanes;

	//Переменные ядра
	ICoreStorageLong * largeshot;
	ICoreStorageFloat * largeshotPrjScale;
	ICoreStorageFloat * largeshotPrjX;
	ICoreStorageFloat * largeshotPrjY;

	//Счётчик уникальных объектов в миссии
	dword uIdCodeCounter;

	//Звуковой сервис
	ISoundService * soundService;

#ifndef STOP_PROFILES
	array<ProfileCreation> creationTable;
#endif

	static const char * loadingHint;
	static float loadingProgress;
	static float loadingProgressScale;

#ifndef NO_CONSOLE
	//Миссия, для которой исполняются команды консоли
	static Mission * controlMission;
	static IConsole * console;
	static bool enableConsoleLogicDebug;
	static char debugObjects[64][128];
	static long debugObjectsCounts;
	static char debugObjectCurrent[256];
#endif
	static const char * missionsPath;
	static const char * missionPath;
	static const char * cutLine;
#ifndef STOP_LOGICTRACE
	static Mission * traceFirstInList;
	Mission * traceNextInList;
	float traceDltTime;
	float traceTimeScale;
	float traceFPS;
	bool traceIsOutHeader;
#endif

};

#endif

