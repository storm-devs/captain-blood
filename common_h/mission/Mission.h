
#ifndef _MissionClasses_h_
#define _MissionClasses_h_

#include "..\core.h"
#include "..\physics.h"
#include "..\render.h"
#include "..\Sound.h"
#include "..\particles.h"
#include "..\FileService.h"
#include "..\gmx.h"
#include "..\Animation.h"
#include "..\IConsole.h"
#include "..\ILiveService.h"
#include "..\controls.h"
#include "MOParams.h"
#include "Groups.h"
#include "Levels.h"
#include "ObjectsFinder.h"

#ifdef _XBOX
#define MIS_STOP_EDIT_FUNCS
#endif

#define MO_IS_FUNCTION(myName, parent) 	virtual bool Is(const ConstString & objName){ static const ConstString thisName(#myName); if(objName == thisName) return true; return parent::Is(objName); };
#define MO_IS_IF(var, type, mo) static const ConstString var(type); if((mo)->Is(var))
#define MO_IS_IF_NOT(var, type, mo) static const ConstString var(type); if(!((mo)->Is(var)))

//============================================================================================
//Общая иерархия классов миссии
//============================================================================================

class IMission;						//Базовый класс миссии, заведующий миссией
class MGIterator;					//Класс для перебора элементов группы
class MGIObject;					//Класс-обёртка для итератора, автоманически удаляющая итератор, после использования

class MissionObject;				//Базовый объект миссии

	class GeometryObject;			//Объекты геометрии окружающего мира и коллижена

	class AIObject;				//Интелектуальные объекты
		class MissionCharacter;	//Персонаж
		class MissionShip;		//Корабль
		

	class LogicObject;				//Логические объекты
		class AcceptorObject;		//Объект для воздействия со стороны игрока
		class DetectorObject;		//Объект отслеживающий игрока
		class DamageObject;			//Объект которому игрок может наносить повреждения

	class WaterObject;				//Вода
		class SeaWater;				//Море
		class LandWater;			//Вода в локациях


class IMOParams;					//Описание параметров миссионого объекта, для редактора
class MOPReader;					//Класс для чтения параметров из области памяти
class MOPWriter;					//Класс для накопления параметров в области памяти

class MissionEvent;					//Класс активирующий событие в миссии


//============================================================================================
//Миссия
//============================================================================================

//Тип функии для обновления
typedef void (_cdecl MissionObject::* MOF_UPDATE)(float dltTime, long level);
//Тип функии для события
typedef void (_cdecl MissionObject::* MOF_EVENT)(GroupId group, MissionObject * sender);
//Тип функии пре и пост обновления
typedef void (_cdecl Object::* MF_UPDATE)(float dltTime);


#define MO_IS(obj, name) ((name *)(obj->Is(#name) ? obj : null))

//Класс для перебора элементов группы
class MGIterator
{
protected:
	virtual ~MGIterator(){};
public:
	//Закончить ли цикл
	virtual bool IsDone() = null;
	//Взять следующий элемент
	virtual void Next() = null;
	//Получить элемент
	virtual MissionObject * Get() = null;
	//Узнать уровень на котором находиться запись
	virtual long Level() = null;
	//Освободить итератор
	virtual void Release() = null;
	//Установить итератор на начало
	virtual void Reset() = null;
	//Исполнить эвент, если это возможно
	virtual void ExecuteEvent(MissionObject * sender = null) = null;
private:
	//Копирование запрещено
	inline MGIterator & operator = (const MGIterator & op){ Assert(false); return *this; };
};

class MGIObject
{
	MGIterator & it;
public:
	MGIObject(MGIterator & _it) : it(_it){};
	~MGIObject(){ it.Release(); };

	//Закончить ли цикл
	inline bool _fastcall IsDone(){ return it.IsDone(); };
	//Взять следующий элемент
	inline void _fastcall Next(){ it.Next(); };
	//Получить элемент
	inline MissionObject * _fastcall Get(){ return it.Get(); };
	//Узнать уровень на котором находиться запись
	inline long Level(){ return it.Level(); };
	//Установить итератор на начало
	inline void _fastcall Reset(){ it.Reset(); };
	//Исполнить эвент, если это возможно
	inline void ExecuteEvent(MissionObject * sender = null){ it.ExecuteEvent(sender); };
	//Копирование ссылки на тот же итератор
	inline MGIObject operator = (MGIObject & op){ return MGIObject(it); };
};

class IMission : public RegObject
{
	friend class MissionObject;
	friend class MOSafePointer;
public:

	struct MissionServices
	{
#ifndef GAME_RUSSIAN
		MissionServices()
		{
			render = null;
			geometry = null;
			sound = null;
			animation = null;
			file = null;
			particles = null;
			ctrl = null;
			colsole = null;
			physics = null;
			liveService = null;
		};

		IRender	* render;
		IGMXService * geometry;
		ISoundScene * sound;
		IParticleManager * particles;
		IAnimationScene * animation;
		IFileService * file;
		IControls * ctrl;
		IConsole * colsole;
		IPhysicsScene * physics;
		ILiveService * liveService;
#else
#ifndef GAME_DEMO
		MissionServices()
		{
			file = null;
			render = null;
			geometry = null;
			particles = null;
			sound = null;		
			animation = null;		
			ctrl = null;
			colsole = null;
			physics = null;
			liveService = null;
		};
		IFileService * file;
		IRender	* render;
		IGMXService * geometry;
		IParticleManager * particles;
		ISoundScene * sound;		
		IAnimationScene * animation;		
		IControls * ctrl;
		IConsole * colsole;
		IPhysicsScene * physics;
		ILiveService * liveService;
#else
		MissionServices()
		{
			geometry = null;
			liveService = null;
			render = null;				
			particles = null;
			animation = null;
			file = null;
			ctrl = null;
			colsole = null;
			sound = null;
			physics = null;
		};
		IGMXService * geometry;
		ILiveService * liveService;
		IRender	* render;				
		IParticleManager * particles;
		IAnimationScene * animation;
		IFileService * file;
		IControls * ctrl;
		IConsole * colsole;
		ISoundScene * sound;
		IPhysicsScene * physics;
#endif
#endif
	};


	//---------------------------------------------------------------------------------------
	//Управление отладочной отрисовкой
	//---------------------------------------------------------------------------------------
public:
	//Нужно ли рисовать тригеры и прочую отладочную инфу
	virtual bool EditMode_IsAdditionalDraw () = null;
	virtual void EditMode_AdditionalDraw (bool value) = null;



//---------------------------------------------------------------------------------------
//Управление миссией
//---------------------------------------------------------------------------------------
public:
	//Заргузить миссию
	virtual bool CreateMission(const char * missionName, float percentsPerMission, float & percentsCounter) = null;
	//Удалить миссию
	virtual void DeleteMission() = null;
	//Получить имя загруженой миссии
	virtual const char * GetMissionName() = null;

//---------------------------------------------------------------------------------------
//Функции для редактора миссий
//---------------------------------------------------------------------------------------
#ifndef NO_TOOLS

public:	
	//Подключить пак для редактора если нет, то создать
	virtual void EditorSetPack(const char * missionName) = null;
	//Получить путь до пак-файла миссии
	virtual void EditorGetPackPath(string & path) = null;
	//Получить путь до файла миссии .mis, чтобы сохранить его
	virtual void EditorGetMisPath(string & path) = null;
	//Усыпить/разбудить миссию
	virtual void EditorSetSleep(bool isSleep) = null;
	//Обновить объект
	virtual void EditorUpdateObject(MissionObject * mo, MOPWriter & writer) = null;
	//Нарисовать миссию
	virtual void EditorDraw(float dltTime) = null;

#endif

//---------------------------------------------------------------------------------------
//Управление объектами миссии
//---------------------------------------------------------------------------------------
public:
	//Динамическое создание миссионного объекта
	bool CreateObject(MOSafePointer & ptr, const char * type, const ConstString & id, bool alone = true);
	//Динамическое создание миссионного объекта
	bool CreateObject(MOSafePointer & ptr, const char * type, MOPReader & rd);
	//Найти объект по идентификатору
	bool FindObject(const ConstString & id, MOSafePointer & ptr);
	//Получить защищёный указатель на миссионый объект
	bool BuildSafePointer(MissionObject * mo, MOSafePointer & ptr);
	//Получить итератор по группе
	virtual MGIterator & GroupIterator(GroupId group, const char* cppFileName, long cppFileLine) = null;

	//Получить игрока
	virtual MissionObject * Player() = null;

	//Получить матрицу качки
	virtual const Matrix & GetSwingMatrix() = null;
	//Получить инверсную матрицу качки
	virtual const Matrix & GetInverseSwingMatrix() = null;
	//Получить параметры качки
	virtual const Vector & GetSwingParams() = null;
	//Установить параметры качки
	virtual void SetSwingParams(const Vector & angs) = null;
	
//---------------------------------------------------------------------------------------
//Утилитные
//---------------------------------------------------------------------------------------
public:
	//Получить ссылку на файловый сервис
	inline IFileService & Files(){ return *services.file; };
	//Получить ссылку на сервис отрисовки
	inline IRender & Render(){ return *services.render; };
	//Получить ссылку на сервис геометрии
	inline IGMXService & Geometry(){ return *services.geometry; };
	//Получить ссылку на сервис анимации
	inline IAnimationScene & Animation(){ return *services.animation; };
	//Получить ссылку на сервис звука
	inline ISoundScene & Sound(){ return *services.sound; };
	//Получить ссылку на сервис ввода
	inline IControls & Controls(){ return *services.ctrl; };
	//Получить менеджер партиклов
	inline IParticleManager & Particles(){ return *services.particles; };
	//Получить ссылку на консоль
	inline IConsole & Console(){ return *services.colsole; };
	//Получить ссылку на сервис физики
	inline IPhysicsScene & Physics(){ return *services.physics; };
	//Получить сервис для взаимодействия с Xbox Live
	inline ILiveService & LiveService(){ return *services.liveService; };
	//Вывод отладочной информации работы логики
	inline void _cdecl LogicDebug(const char * format, ...)
	{
#ifndef STOP_LOGICTRACE
		sysLogicDebug(null, format, &format + 1, false);
#endif
	}
	//Вывод отладочной информации ошибки работы логики
	inline void _cdecl LogicDebugError(const char * format, ...)
	{
#ifndef STOP_LOGICTRACE
		sysLogicDebug(null, format, &format + 1, true);
#endif
	}
	//Изменить текущий уровень вывода
	inline void LogicDebugLevel(bool increase)
	{
#ifndef STOP_LOGICTRACE
		sysLogicDebugLevel(increase);
#endif
	}


//---------------------------------------------------------------------------------------
//Системные
//---------------------------------------------------------------------------------------
protected:
	virtual MissionObject * sysCreateObject(const char * type, const ConstString & id, bool alone) = null;
	virtual MissionObject * sysCreateObject(const char * type, MOPReader & rd) = null;
	virtual void sysRegistry(GroupId group, MissionObject * object, MOF_EVENT func, long level) = null;	
	virtual void sysUnregistry(GroupId group, MissionObject * object) = null;	
	virtual void sysUnregistryAll(MissionObject * object) = null;	
	virtual void sysEvent(GroupId group, MissionObject * object) = null;
	virtual void sysSetUpdate(long level, MissionObject * obj, MOF_UPDATE func) = null;
	virtual void sysDelUpdate(long level, MissionObject * obj, MOF_UPDATE func) = null;
	virtual void sysDelUpdate(MissionObject * obj, MOF_UPDATE func) = null;
	virtual void sysDeleteObject(MissionObject * object) = null;
	virtual void sysUpdateObjectID(MissionObject * obj, const char * oldId, const char * newId) = null;
	virtual void sysLogicDebug(MissionObject * mo, const char * format, void * data, bool isError) = null;
	virtual void sysLogicDebugLevel(bool increase) = null;
	virtual IMissionQTObject * sysQTCreateObject(GroupId group, MissionObject * mo, const char* cppFileName, long cppFileLine) = null;
	virtual dword sysQTFindObjects(GroupId group, Vector minVrt, Vector maxVrt) = null;	
	virtual IMissionQTObject * sysQTGetObject(dword index) = null;
	virtual void sysQTDraw(GroupId group, float levelScale) = null;
	virtual void sysQTDump(GroupId group) = null;	
	void sysSetThisPointerToObject(MissionObject * obj);
	void sysSetObjectIDBeforeCreate(MissionObject * obj, const char * id);
	void sysSetObjectTypeBeforeCreate(MissionObject * obj, const char * type);
	void sysSetObjectEMBeforeCreate(MissionObject * obj, bool editMode);
	void sysSetObjectReaderDataBeforeCreate(MissionObject * obj, const void * data, long size, void * deleteDataWithObject);
	bool sysEditMode_Create(MissionObject * obj, MOPReader & reader);	
	bool sysEditMode_Update(MissionObject * obj, MOPReader & reader);
	void sysSetObjectUID(MissionObject * obj, dword uid);
	virtual bool sysFindObject(const ConstString * id, MissionObject * & object, dword & uid) = null;
	virtual bool sysValidatePointer(MissionObject * object, dword uid, long & hashIndex) = null;

	
	//Сервисы используемые в миссии
	MissionServices services;
};


//============================================================================================
//Защищёный указатель на миссионый объект
//============================================================================================

class MOSafePointer
{
	friend class IMission;
public:
	__forceinline MOSafePointer()
	{
		Reset();
	};

	~MOSafePointer()
	{
		object = (MissionObject *)((char  *)0 + 0xbadbad01);
		index = 0;
		uid = 0;
		mission = (IMission *)((char  *)0 + 0xbadbad02);
	}

	//Сбросить указатель
	__forceinline void Reset()
	{
		object = null;
		index = 0;
		uid = 0;
		mission = null;		
	}

	//Проверить указатель на корректность
	__forceinline bool Validate()
	{
		if(!object) return false;
		if(mission)
		{
			if(mission->sysValidatePointer(object, uid, index))
			{
				return true;
			}else{
				Reset();
				return false;
			}
		}
		return false;
	}

	//Получить указатель на объект
	__forceinline MissionObject * Ptr() const { return object; };
	//Получить указатель на объект с проверкой
	__forceinline MissionObject * SPtr(){ Assert(Validate()); return object; };

	//Сравнить указатели
	__forceinline bool operator == (const MOSafePointer & sp) const
	{
		return (object == sp.object) && (uid == sp.uid);
	}

	//Сравнить указатели
	__forceinline bool operator != (const MOSafePointer & sp) const
	{
		return (object != sp.object) || (uid != sp.uid);
	}

private:
	MissionObject * object;
	dword uid;
	long index;		
	IMission * mission;
};

template<class T> class MOSafePointerType
{
public:
	//Проверить указатель на корректность
	__forceinline bool Validate()
	{
		return ptr.Validate();
	}

	//Сбросить указатель
	__forceinline void Reset()
	{
		ptr.Reset();
	}

	//Получить указатель на объект
	__forceinline T * Ptr() const { return (T *)ptr.Ptr(); };

	//Получить указатель на объект с проверкой
	__forceinline T * SPtr(){ return (T *)ptr.SPtr(); };

	//Получить сэйфпоинтер
	__forceinline MOSafePointer & GetSPObject()
	{
		return ptr;
	}

private:
	MOSafePointer ptr;
};


//============================================================================================
//Базовый объект миссии
//============================================================================================

class MissionObject : public RegObject
{
	friend IMission;

protected:
	inline MissionObject()
	{
		mission = null;
		flags = mof_initial;
		type = "Just create and not set";
		initDataSize = 0;
		initData = null;
		initDataDelete = null;
		uIdCode = 0;
	};

	enum MisObjectFlags
	{
		mof_isShow = 0x0001,			//Виден ли объект
		mof_isActive = 0x0002,			//Активен ли объект
		mof_isSelect = 0x0004,			//Выделен в режиме редактирования объект или нет
		mof_isEditMode = 0x10000,		//Объект создан в режиме редактирования
		mof_isEditVisible = 0x20000,	//Видимость в редакторе
		mof_isEditSleep = 0x40000,		//Объект в редакторе находится в спящем режиме
		
		//Начальное значение флагов при создании
		mof_initial = mof_isEditVisible,
	};

public:
	~MissionObject()
	{
		if(initDataDelete) delete initDataDelete;
		initDataDelete = null;
		if(mission) mission->sysDeleteObject(this);
	};

	virtual bool Is(const ConstString & objName)
	{
		static const ConstString thisName("MissionObject");
		return (objName == thisName);
	};

//--------------------------------------------------------------------
//Создание, обновление
//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader){ return true; };
	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate(){};
	//Пересоздать объект
	virtual void Restart(){ LogicDebugError("This object can't support Restart option"); };
	//Пересоздать объект
	void ReCreate()
	{
#ifndef STOP_ASSERTS
		//*****************
		//прививка для дизайнеров, чтобы не использовали бездумно пересоздание
		
		
		
		
		//Серёга потребывал убрать прививку. За тормаза на рестартах не ручаюсь... Sleep(5);
		//*****************
#endif
		if(initData && initDataSize > 0)
		{
			MOPReader reader(initData, initDataSize);
			LogicDebug("Restart object \"%s\", type \"%s\"", id.c_str(), GetObjectType());
			Create(reader);
		}else{
			api->Trace("Mission object \"%s\" cant do ReCreate. Not assign initData for this object (runtime object or something).", id.c_str());
			LogicDebugError("This object can't process Restart, for more information look system.log");
		}
	}

//--------------------------------------------------------------------
//Общие свойства
//--------------------------------------------------------------------
public:
	//Получить идентификатор объекта
	__forceinline const ConstString & GetObjectID() const { return id; };
	//Получить тип объекта
	__forceinline const char * GetObjectType() const { return type; };
	//Получить уникальный номер объекта
	__forceinline dword GetObjectUID(){ return uIdCode; };
	//Показать/скрыть объект
	virtual void Show(bool isShow){ if(isShow){ flags |= mof_isShow; }else{ flags &= ~mof_isShow; } };
	//Виден ли объект
	virtual bool IsShow() const { return (flags & mof_isShow) != 0; };

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive){ if(isActive){ flags |= mof_isActive; }else{ flags &= ~mof_isActive; } };
	//Активен объект или нет
	virtual bool IsActive() const { return (flags & mof_isActive) != 0; };

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params){ LogicDebugError("Skip command \"%s\"", id); };

	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx){ mtx.SetIdentity(); return mtx; };
	//Получить бокс, описывающий объект в локальных координатах
	virtual void GetBox(Vector & min, Vector & max){ min = 0.0f; max = 0.0f; };

	//Умирает
	virtual bool IsDie(){ return false; };
	//Мёртв
	virtual bool IsDead(){ return false; };
	//Получить текущее значение жизни
	virtual float GetHP(){ return 1.0f; };
	//Получить максимальное значение жизни
	virtual float GetMaxHP(){ return 1.0f; };

//--------------------------------------------------------------------
//Работа с классом миссии
//--------------------------------------------------------------------
public:
	//Получить миссию
	inline IMission & Mission() const { Assert(mission); return *mission; };

	//Зарегистрировать объект в группе
	template<class T> inline void Registry(GroupId group, T func, long level)
	{
		if(mission)
		{
			mission->sysRegistry(group, this, (MOF_EVENT)func, level);
		}
	};
	//Зарегистрировать объект в группе
	inline void Registry(GroupId group)
	{
		if(mission)
		{
			mission->sysRegistry(group, this, (MOF_EVENT)null, 0);
		}
	};

	//Удалить из списка регистрации данной группы
	inline void Unregistry(GroupId group){ if(mission) mission->sysUnregistry(group, this); };
	//Удалить из списка регистрации всех групп
	inline void Unregistry(){ if(mission) mission->sysUnregistryAll(this); };

	//Отослать событие группе
	inline void Event(GroupId group){ if(mission) mission->sysEvent(group, this); };

	//Получить итератор для получения объектов группы
	inline MGIterator & GroupIterator(GroupId group, const char* szFile, long Line){ Assert(mission) return mission->GroupIterator(group, szFile, Line); };
	//Найти объект по идентификатору
	inline bool FindObject(const ConstString & id, MOSafePointer & safePointer){ if(mission) return mission->FindObject(id, safePointer); return false; };
	//Получить защищёный указатель на объект
	inline bool BuildSafePointer(MissionObject * mo, MOSafePointer & safePointer){ if(mission) return mission->BuildSafePointer(mo, safePointer); return false; };

	//Создать объект поиска в группе через quad-tree
	inline IMissionQTObject * QTCreateObject(GroupId group, const char* cppFileName, long cppFileLine){ Assert(mission); return mission->sysQTCreateObject(group, this, cppFileName, cppFileLine); };
	//Найти объекты в пространстве
	inline dword QTFindObjects(GroupId group, Vector minVrt, Vector maxVrt){ Assert(mission); return mission->sysQTFindObjects(group, minVrt, maxVrt); };
	//Получить объект из только что найденного списка
	inline IMissionQTObject * QTGetObject(dword index){ Assert(mission); return mission->sysQTGetObject(index); };
	//Нарисовать дерево с объектами
	inline void QTDraw(GroupId group, float levelScale = 2.0f){ Assert(mission); return mission->sysQTDraw(group, levelScale); };
	//Вывести в лог узлы дерева поиска
	inline void QTDump(GroupId group){ Assert(mission); return mission->sysQTDump(group); };

	//Добавить в список исполнения
	template<class T> inline void SetUpdate(T func, long level = 0)
	{ 
		if(mission)
		{
			mission->sysSetUpdate(level, this, (MOF_UPDATE)func);
		}
	};
	//Удалить из списка исполнения на заданном уровне
	template<class T> inline void DelUpdate(long level, T func = null){ if(mission) mission->sysDelUpdate(level, this, (MOF_UPDATE)func); };
	//Удалить из списка исполнения
	template<class T> inline void DelUpdate(T func){ if(mission) mission->sysDelUpdate(this, (MOF_UPDATE)func); };
	//Удалить всё из списка исполнения
	inline void DelUpdate(){ if(mission) mission->sysDelUpdate(this, null); };


//--------------------------------------------------------------------
//Утилитные
//--------------------------------------------------------------------
public:
	//Получить ссылку на файловый сервис
	inline IFileService & Files() const { return *services.file; };
	//Получить ссылку на сервис отрисовки
	inline IRender & Render() const { return *services.render; };
	//Получить ссылку на сервис геометрии
	inline IGMXService & Geometry() const { return *services.geometry; };
	//Получить ссылку на сервис анимации
	inline IAnimationScene & Animation() const { return *services.animation; };
	//Получить ссылку на сервис звука
	inline ISoundScene & Sound() const { return *services.sound; };
	//Получить ссылку на сервис ввода
	inline IControls & Controls() const { return *services.ctrl; };
	//Получить менеджер партиклов
	inline IParticleManager & Particles() const { return *services.particles; };
	//Получить ссылку на игровое сотояние
	//inline IGameState & GameState() const { return *services.gameState; };
	//Получить ссылку на консоль
	inline IConsole & Console() const { return *services.colsole; };
	//Получить ссылку на сервис физики
	inline IPhysicsScene & Physics() const { return *services.physics; };
	//Получить сервис для взаимодействия с Xbox Live
	inline ILiveService & LiveService(){ return *services.liveService; };
	//Вывод отладочной информации работы логики
	inline void _cdecl LogicDebug(const char * format, ...)
	{
#ifndef STOP_LOGICTRACE
		if(mission) mission->sysLogicDebug(this, format, &format + 1, false); 
#endif
	}
	//Вывод отладочной информации ошибки работы логики
	inline void _cdecl LogicDebugError(const char * format, ...)
	{
#ifndef STOP_LOGICTRACE
		if(mission) mission->sysLogicDebug(this, format, &format + 1, true); 
#endif
	}
	//Изменить текущий уровень вывода
	inline void LogicDebugLevel(bool increase)
	{
#ifndef STOP_LOGICTRACE
		if(mission) mission->sysLogicDebugLevel(increase);
#endif
	}

//--------------------------------------------------------------------
//Опции для редактора
//--------------------------------------------------------------------
#ifndef MIS_STOP_EDIT_FUNCS

protected:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader){ Create(reader); return true; };
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader){ return true; };

public:
	//Находимся в режиме редактирования
	inline bool EditMode_IsOn() const { return (flags & mof_isEditMode) != 0; };
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max){ Assert(flags & mof_isEditMode); min = 0.0f; max = 0.0f; };
	//Выделить объект
	virtual void EditMode_Select(bool isSelect){ Assert(flags & mof_isEditMode); if(isSelect){ flags |= mof_isSelect; }else{ flags &= ~mof_isSelect; } };
	//Выделен ли объект
	inline bool EditMode_IsSelect() const { return (flags & mof_isSelect) != 0; };
	//Показать или скрыть объект в редакторе
	virtual void EditMode_Visible(bool isVisible){ Assert(flags & mof_isEditMode); if(isVisible){ flags |= mof_isEditVisible; }else{ flags &= ~mof_isEditVisible; } };
	//Виден ли объект в редакторе
	inline bool EditMode_IsVisible() const { return (flags & mof_isEditVisible) != 0; };
	//Перевести объект в спящий режим
	virtual void EditMode_Sleep(bool isSleep){ Assert(flags & mof_isEditMode); if(isSleep){ flags |= mof_isEditSleep; }else{ flags &= ~mof_isEditSleep; } };
	//В спящем режиме находимся или нет
	inline bool EditMode_IsSleep() const { return (flags & mof_isEditSleep) != 0; };
	//Момент выгрузки. Можно расчитать данные препроцесса, Путь на пак можно получить функцией EditorGetPackPath
	//Чтобы функция вызывалась необходимо в EditMode_Create зарегистрироваться в группе MG_EXPORT
	virtual bool EditMode_Export(){ return true; };

#else
	//Находимся в режиме редактирования
	inline bool EditMode_IsOn() const { return false; };
	//Выделен ли объект
	inline bool EditMode_IsSelect() const { return false; };
	//Виден ли объект в редакторе
	inline bool EditMode_IsVisible() const { return true; };
	//В спящем режиме находимся или нет
	inline bool EditMode_IsSleep() const { return false; };
#endif

//--------------------------------------------------------------------
private:
	IMission * mission;		//Указатель на миссию	
	dword flags;			//Флаги объекта
	dword uIdCode;			//Уникальный для миссии числовой идентификатор объекта
	ConstString id;			//Идентификатор объекта
	const char * type;		//Тип объекта
	long initDataSize;		//Размер данных инициализации
	const void * initData;	//Данные инициализации
	void * initDataDelete;	//Данные которые надо удалить при удалении объекта

	//Сервисы используемые в миссии
	IMission::MissionServices services;	
};




//Динамическое создание миссионного объекта
inline bool IMission::CreateObject(MOSafePointer & ptr, const char * type, const ConstString & id, bool alone)
{
	ptr.object = sysCreateObject(type, id, alone);
	return BuildSafePointer(ptr.object, ptr);
}

//Динамическое создание миссионного объекта
inline bool IMission::CreateObject(MOSafePointer & ptr, const char * type, MOPReader & rd)
{
	ptr.object = sysCreateObject(type, rd);
	return BuildSafePointer(ptr.object, ptr);
}

inline void IMission::sysSetThisPointerToObject(MissionObject * obj)
{
	obj->services = services;
	obj->mission = this;
}

inline void IMission::sysSetObjectIDBeforeCreate(MissionObject * obj, const char * id)
{
	obj->id.Set(id);
}

inline void IMission::sysSetObjectTypeBeforeCreate(MissionObject * obj, const char * type)
{
	obj->type = type;
}


inline void IMission::sysSetObjectEMBeforeCreate(MissionObject * obj, bool editMode)
{
#ifndef MIS_STOP_EDIT_FUNCS
	if(editMode)
	{
		obj->flags |= MissionObject::mof_isEditMode;
	}	
#endif
}

inline void IMission::sysSetObjectReaderDataBeforeCreate(MissionObject * obj, const void * data, long size, void * deleteDataWithObject)
{
	if(obj->initDataDelete && obj->initDataDelete != deleteDataWithObject)
	{
		delete obj->initDataDelete;
	}
	obj->initDataDelete = deleteDataWithObject;
	obj->initData = data;
	obj->initDataSize = size;
}

inline bool IMission::sysEditMode_Create(MissionObject * obj, MOPReader & reader)
{
#ifndef MIS_STOP_EDIT_FUNCS
	return obj->EditMode_Create(reader);
#else
	return false;
#endif
}

inline bool IMission::sysEditMode_Update(MissionObject * obj, MOPReader & reader)
{
#ifndef MIS_STOP_EDIT_FUNCS
	return obj->EditMode_Update(reader);
#else
	return false;
#endif
}

inline void IMission::sysSetObjectUID(MissionObject * obj, dword uid)
{
	obj->uIdCode = uid;
}

//Найти объект по идентификатору
inline bool IMission::FindObject(const ConstString & id, MOSafePointer & ptr)
{
	if(id.IsEmpty())
	{
		ptr.Reset();
		return false;
	}
	ptr.object = null;
	ptr.index = -1;
	ptr.mission = this;
	return sysFindObject(&id, ptr.object, ptr.uid);
}

//Получить указатель на миссионый объект
inline bool IMission::BuildSafePointer(MissionObject * mo, MOSafePointer & ptr)
{
	if(!mo)
	{
		ptr.Reset();
		return false;
	}
	ptr.object = mo;
	ptr.index = -1;
	ptr.mission = this;
	return sysFindObject(null, ptr.object, ptr.uid);
}

//============================================================================================
//Базовые объекты
//============================================================================================

//Логические объекты
class LogicObject : public MissionObject
{
public:
	MO_IS_FUNCTION(LogicObject, MissionObject);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader){ return MissionObject::Create(reader); };

	//Сбросить состояние объекта
	virtual void Reset(){};
};

//Уровень воды
class IWaterLevel : public MissionObject
{
public:
	IWaterLevel()
	{
		level = 0.0f;
	};

	MO_IS_FUNCTION(WaterLevel, MissionObject);

	__forceinline static float GetWaterLevel(IMission & mis, MOSafePointer & ptr)
	{
		if(!ptr.Validate())
		{
			static const ConstString id("__WaterLevel__");
			mis.CreateObject(ptr, "WaterLevel", id);
			Assert(ptr.Validate());
		}
		
		return ((IWaterLevel *)ptr.Ptr())->level;
	}

protected:
	float level;
};


//Объекты геометрии окружающего мира и коллижена
class GeometryObject : public MissionObject
{
public:
	//Получить сцену
	virtual IGMXScene * GetScene() = null;


	//Активировать линк
	virtual bool ActivateAnimationLink(const char * command)
	{
		IGMXScene * model = GetScene();
		if(!model) return false;
		IAnimationTransform * ani = model->GetAnimation();
		if(ani && ani->Is(anitype_animation))
		{
			bool res = ((IAnimation *)ani)->IsCanActivateLink(command);
			((IAnimation *)ani)->ActivateLink(command, true);
			ani->Release();
			return res;
		}
		return false;
	}

	MO_IS_FUNCTION(GeometryObject, MissionObject);

};

//Тип функии для события
class MissionShadowCaster : public MissionObject
{
public:
	template<class T> inline void AddObject(MissionObject * obj, T func, const Vector & min, const Vector & max)
	{
		AddShadowCastObject(obj, (MOF_EVENT)func, min, max);
	}

	MO_IS_FUNCTION(MissionShadowCaster, MissionObject);

protected:
	virtual void AddShadowCastObject(MissionObject * obj, MOF_EVENT func, const Vector & min, const Vector & max) = null;


protected:
	inline void GetShadowCastObjects()
	{
		Event(MG_SHADOWCAST);
	};

	inline void ExecuteEvent(MissionObject * obj, MOF_EVENT func)
	{
		(obj->*func)(MG_SHADOWCAST, this);
	};
};



//============================================================================================
//Физические объекты
//============================================================================================

//Интелектуальные объекты
class AIObject : public MissionObject
{
public:
	MO_IS_FUNCTION(AIObject, MissionObject);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader){ return MissionObject::Create(reader); };

public:
};

//Персонаж
class MissionCharacter : public AIObject
{
public:
	MO_IS_FUNCTION(MissionCharacter, AIObject);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader){ Registry(MG_CHARACTER); return AIObject::Create(reader); };

};

//Корабль
class MissionShip : public AIObject
{
public:
	MO_IS_FUNCTION(MissionShip, AIObject);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader){ Registry(MG_SHIP); return AIObject::Create(reader); };
};

//--------------------------------------------------------------------------------------------
//Класс активирующий событие в миссии
//--------------------------------------------------------------------------------------------

//Параметры для инициатора события
#define MOP_MISSIONTRIGGER_IN(s)	MOP_ARRAYBEG(s##"Complex", 0, 100)\
										MOP_STRING("Object name", "")\
										MOP_BOOL("Restart object", false)\
										MOP_ENUMTL("Active object")\
										MOP_ENUMTL("Visible object")\
										MOP_STRING("Animation command", "")\
										MOP_STRING("Command", "")\
										MOP_ARRAYBEG("Command params", 0, 8)\
										MOP_STRING("Param", "")\
										MOP_ARRAYEND\
									MOP_ARRAYEND\
									MOP_ARRAYBEG(s##"Activate objects", 0, 100)\
										MOP_STRING("Active object name", "")\
										MOP_BOOL("Is active object", false)\
									MOP_ARRAYEND\
									MOP_ARRAYBEG(s##"Visible objects", 0, 100)\
										MOP_STRING("Visible object name", "")\
										MOP_BOOL("Is visible object", true)\
									MOP_ARRAYEND\
									MOP_ARRAYBEG(s##"Animate objects", 0, 100)\
										MOP_STRING("Animate object name", "")\
										MOP_STRING("Animation command", "")\
									MOP_ARRAYEND\
									MOP_ARRAYBEG(s##"Commands", 0, 100)\
										MOP_STRING("Object name", "")\
										MOP_STRING("Command", "")\
										MOP_ARRAYBEG("Command params", 0, 8)\
											MOP_STRING("Param", "")\
										MOP_ARRAYEND\
									MOP_ARRAYEND\

								

//Параметры для инициатора события
#define MOP_MISSIONTRIGGER(s)		MOP_MISSIONTRIGGER_IN(s)

//Параметры для инициатора события
#define MOP_MISSIONTRIGGERG(s, p)	MOP_GROUPBEG(s)\
										MOP_MISSIONTRIGGER_IN(p)\
									MOP_GROUPEND()

//Параметры для инициатора события
#define MOP_MISSIONTRIGGERC(s, c)	MOP_GROUPBEGC(s, c)\
										MOP_MISSIONTRIGGER_IN(s)\
									MOP_GROUPEND()


//Класс реализующий событие
class MissionTrigger
{
#pragma pack(push, 1)

	enum Flags
	{
		f_numParamsMask = 0xff,
		f_isRestart = 0x100,
		f_isActivate = 0x200,
		f_isChangeActive = 0x400,
		f_isShow = 0x800,
		f_isChangeShow = 0x1000,
		f_isSetLink = 0x2000,
		f_isSetCommand = 0x4000,
	};

	struct Element
	{
		dword flags;		
		ConstString objectID;
		const char * link;
		const char * command;
		const char * params[8];

	};

#pragma pack(pop)



public:
	MissionTrigger() : elements(_FL_, 1)
	{
		isTriggerred = false;
	}

	MissionTrigger(const MissionTrigger & copyFrom) : elements(_FL_, 1)
	{
		*this = copyFrom;
	}

public:
	//Инициализация параметров
	void Init(MOPReader & reader)
	{
		Empty();
		//Комбинированые записи
		long size = reader.Array();
		long index = 0;
		elements.AddElements(size);
		for(long i = 0; i < size; i++)
		{
			Element & e = elements[index++];
			e.flags = 0;
			e.objectID = reader.String();
			bool state = false;
			e.flags |= reader.Bool() ? f_isRestart : 0;
			e.flags |= reader.TripleLogic(state) ? f_isChangeActive : 0;
			e.flags |= state ? f_isActivate : 0;
			e.flags |= reader.TripleLogic(state) ? f_isChangeShow : 0;
			e.flags |= state ? f_isShow : 0;
			const char * str = reader.String().c_str();
			if(str && str[0])
			{
				e.flags |= f_isSetLink;
				e.link = str;
			}
			str = reader.String().c_str();
			long cnt = reader.Array();
			if(str && str[0])
			{
				e.flags |= f_isSetCommand | cnt;
				e.command = str;
			}			
			for(long j = 0; j < cnt; j++)
			{
				e.params[j] = reader.String().c_str();
			}
		}
		//Объекты для активации и деактивации
		size = reader.Array();
		elements.AddElements(size);
		for(long i = 0; i < size; i++)
		{
			Element & e = elements[index++];
			e.objectID = reader.String();
			e.flags = f_isChangeActive | (reader.Bool() ? f_isActivate : 0);
		}
		//Объекты для показа или скрытия
		size = reader.Array();
		elements.AddElements(size);
		for(long i = 0; i < size; i++)
		{
			Element & e = elements[index++];
			e.objectID = reader.String();
			e.flags = f_isChangeShow | (reader.Bool() ? f_isShow : 0);
		}
		//Включение анимации
		size = reader.Array();
		elements.AddElements(size);
		for(long i = 0; i < size; i++)
		{
			Element & e = elements[index++];
			e.flags = f_isSetLink;
			e.objectID = reader.String();
			e.link = reader.String().c_str();
		}
		//Команды
		size = reader.Array();
		elements.AddElements(size);
		for(long i = 0; i < size; i++)
		{
			Element & e = elements[index++];
			e.objectID = reader.String();
			e.command = reader.String().c_str();
			long cnt = reader.Array();
			e.flags = f_isSetCommand | cnt;
			for(long j = 0; j < cnt; j++)
			{
				e.params[j] = reader.String().c_str();
			}
		}		
		//Скорректируем указатели
		size = elements;
	};

	//Активация события
	void Activate(IMission & mission, bool isOnece = true, MissionObject * thisObject = null)
	{
		if(isTriggerred) return;
		if(elements.Size() == 0) return;
		mission.LogicDebug(">>Activate trigger (id = 0x%.8x)", this);
		mission.LogicDebugLevel(true);
		if(isOnece) isTriggerred = true;
		for(long i = 0; i < elements; i++)
		{
			Element & e = elements[i];
			MissionObject * mo = GetMissionObject(e.objectID, mission, thisObject);
			if(mo)
			{
				if(e.flags & f_isRestart)
				{
					mo->Restart();
				}
				if(e.flags & f_isChangeActive)
				{					
					if(e.flags & f_isActivate)
					{
						mission.LogicDebug("Activate object \"%s\"", e.objectID.c_str());
						mission.LogicDebugLevel(true);
						mo->Activate(true);
						mission.LogicDebugLevel(false);
					}else{
						mission.LogicDebug("Deactivate object \"%s\"", e.objectID.c_str());
						mission.LogicDebugLevel(true);
						mo->Activate(false);
						mission.LogicDebugLevel(false);
					}
				}
				if(e.flags & f_isChangeShow)
				{
					if(e.flags & f_isShow)
					{
						mission.LogicDebug("Show object \"%s\"", e.objectID.c_str());
						mission.LogicDebugLevel(true);
						mo->Show(true);
						mission.LogicDebugLevel(false);
					}else{
						mission.LogicDebug("Hide object \"%s\"", e.objectID.c_str());
						mission.LogicDebugLevel(true);
						mo->Show(false);
						mission.LogicDebugLevel(false);
					}
				}
				if(e.flags & f_isSetLink)
				{
					static const ConstString geomObjId("GeometryObject");
					if(mo->Is(geomObjId))
					{
						if(((GeometryObject *)mo)->ActivateAnimationLink(e.link))
						{
							mission.LogicDebug("Animation link \"%s\" in object \"%s\" activated", e.link, e.objectID.c_str());
						}else{
							mission.LogicDebugError("Can't start animation link \"%s\" in object \"%s\"", e.link, e.objectID.c_str());
						}
					}else{
						mission.LogicDebugError("Can't start animation link from object \"%s\" - invalidate object's type (\"%s\")", e.objectID.c_str(), mo->GetObjectType());
					}
				}
				if(e.flags & f_isSetCommand)
				{
					mission.LogicDebug("Execute command \"%s\" for object \"%s\"", e.command, e.objectID.c_str());
					long numParams = e.flags & f_numParamsMask;
					for(long n = 0; n < numParams; n++)
					{
						mission.LogicDebug("|-param %i: \"%s\"", n, e.params[n]);
					}
					if(e.command[0] == 'A' || e.command[0] == 'a')
					{
						if(string::IsEqual(e.command, "activate"))
						{
							mission.LogicDebug("Special command - activate");
							mission.LogicDebugLevel(true);
							mo->Activate(true);
							mission.LogicDebugLevel(false);
							continue;
						}
					}else
						if(e.command[0] == 'D' || e.command[0] == 'd')
						{
							if(string::IsEqual(e.command, "deactivate"))
							{
								mission.LogicDebug("Special command - deactivate");
								mission.LogicDebugLevel(true);
								mo->Activate(false);
								mission.LogicDebugLevel(false);
								continue;
							}
						}else
							if(e.command[0] == 'S' || e.command[0] == 's')
							{
								if(string::IsEqual(e.command, "show"))
								{
									mission.LogicDebug("Special command - show");
									mission.LogicDebugLevel(true);
									mo->Show(true);
									mission.LogicDebugLevel(false);
									continue;
								}
							}else
								if(e.command[0] == 'H' || e.command[0] == 'h')
								{
									if(string::IsEqual(e.command, "hide"))
									{
										mission.LogicDebug("Special command - hide");
										mission.LogicDebugLevel(true);
										mo->Show(false);
										mission.LogicDebugLevel(false);
										continue;
									}
								}
								mission.LogicDebugLevel(true);
								mo->Command(e.command, numParams, &e.params[0]);
								mission.LogicDebugLevel(false);
				}
			}else{
				mission.LogicDebugError("Mission object \"%s\" not found, can't triggered it", e.objectID.c_str());
			}
		}
		mission.LogicDebugLevel(false);
	};

	//Сбросить состояние активации
	void Reset()
	{
		isTriggerred = false;
	}

	//Очистить событие
	void Empty()
	{
		elements.Empty();
		isTriggerred = false;
	}

	//Пустой ли триггер
	bool IsEmpty()
	{
		return elements.IsEmpty();
	}

	//Сработал триггер или нет
	bool IsTriggerred()
	{
		return isTriggerred;
	}

	
	MissionTrigger & operator = (const MissionTrigger & copyFrom)
	{
		elements = copyFrom.elements;
		isTriggerred = copyFrom.isTriggerred;
		return *this;
	}

private:
	__forceinline MissionObject * GetMissionObject(const ConstString & objectName, IMission & mission, MissionObject * thisObject)
	{
		static const ConstString thisName("*this*");
		MissionObject * mo;
		if(objectName != thisName)
		{
			MOSafePointer sp;
			if(mission.FindObject(objectName, sp))
			{
				mo = sp.Ptr();
			}else{
				mo = null;
			}
		}else{
			mo = thisObject;
		}
		return mo;
	}

private:
	array<Element> elements;		//Список объектов, которые надо активировать или деактивировать
	bool isTriggerred;				//Защёлкнут триггер или нет
};



//============================================================================================
//Логические объекты
//============================================================================================

//Объект для воздействия со стороны игрока
class AcceptorObject : public LogicObject
{
public:
	MO_IS_FUNCTION(AcceptorObject, LogicObject);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader){ Registry(MG_ACCEPTOR); return LogicObject::Create(reader); };
	//Воздействовать на объект
	virtual bool Accept(MissionObject * obj) = null;
};

//Объект отслеживающий игрока
class DetectorObject : public LogicObject
{
public:
	DetectorObject()
	{
		autoReset = false;
		isDown = false;
	}

	MO_IS_FUNCTION(DetectorObject, LogicObject);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader){  return LogicObject::Create(reader); };

	//Сбросить состояние детектора
	virtual void Reset(){ isDown = false; };
	//Режим автосброса
	virtual void AutoReset(bool isEnable){ autoReset = isEnable;  };
	//Активация детектора
	virtual void ActivateDetector(const char * initiatorID){};

protected:
	bool isDown;
	bool autoReset;
};









#define MOP_DR_MULTIPLIERS		MOP_FLOAT("Sword", 1.0f)	\
								MOP_FLOAT("Bomb", 1.0f)		\
								MOP_FLOAT("Bullet", 1.0f)	\
								MOP_FLOAT("Cannon", 1.0f)	\
								MOP_FLOAT("Flame", 1.0f)	\
								MOP_FLOAT("Shooter", 1.0f)

#define MOP_DR_MULTIPLIERSG		MOP_GROUPBEG("Damage multipliers")	\
								MOP_DR_MULTIPLIERS					\
								MOP_GROUPEND()

class DamageReceiver : public LogicObject
{
public:
	enum DamageSource
	{		
		ds_sword = 0,				//Sword
		ds_bomb = 1,				//Bomb
		ds_bullet = 2,				//Bullet
		ds_cannon = 3,				//Cannon
		ds_flame = 4,				//Flame
		ds_shooter = 5,				//Shooter		
		ds_check,					//Проверить на попадание
		ds_unknown,					//Неизвестный источник
		ds_trap_flame,				//Огненая ловушка
		ds_trap_stakes,				//Ловушка-колья		
		ds_max = ds_shooter + 1,	//Количество задаваемых дизайнерами мультипликаторов
	};

	struct DamageMultiplier
	{
		DamageMultiplier()
		{
			for(long i = 0; i < ds_max; i++)
			{
				dm[i] = 1.0f;
			}
		};

		void Init(MOPReader & reader)
		{
			for(long i = 0; i < ds_max; i++)
			{
				dm[i] = reader.Float();
			}
		}

		float Multiplier(dword type, float invMul = 0.0f)
		{
			if(type < ds_max)
			{
				return dm[type];
			}
			return invMul;
		}

		float dm[ds_max];
	};

public:

	DamageReceiver()
	{
		finder = null;
	}
	

	virtual ~DamageReceiver()
	{
		RELEASE(finder);
	}


	MO_IS_FUNCTION(DamageReceiver, LogicObject);

	//Воздействовать на объект сферой
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius) = null;
	//Воздействовать на объект линией
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to) = null;
	//Воздействовать на объект выпуклым чехырёхугольником
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4]) = null;

protected:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader)
	{
		if(!LogicObject::Create(reader)) return false;
		finder = QTCreateObject(MG_DAMAGEACCEPTOR, _FL_);
		return true;
	};

public:
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx)
	{	
		if(finder)
		{
			mtx = finder->GetMatrix();
		}else{
			mtx.SetIdentity();
		}
		return mtx;
	}

protected:
	IMissionQTObject * finder;				//Регистрационная запись поиска
};


class DamageObject : public DamageReceiver
{
public:
	MO_IS_FUNCTION(DamageObject, DamageReceiver);

	DamageObject()
	{
		HP = initHP = 1.0f;
		applyDamage = 0.0f;
		autoReset = false;
	}

	//Сбросить состояние детектора
	virtual void Reset(){ HP = initHP; applyDamage = 0.0f; };

	//Применить демедж
	inline void ApplyDamage(float damage)
	{
		if(!IsActive()) return;
		if(damage < 0.0f) damage = 0.0f;
		applyDamage += damage;
		SetUpdate(&DamageObject::StepWork, ML_TRIGGERS);
		LogicDebug("Apply damage, hp = %f, damage = %f. Wait next frame now...", HP, damage);
	}

protected:

	//Активировать
	virtual void Activate(bool isActive)
	{
		LogicObject::Activate(isActive);
		if(!EditMode_IsOn())
		{
			if(finder)
			{
				finder->Activate(IsActive());
			}
			if(IsActive())
			{
				LogicDebug("Activated");
				if(applyDamage > 0.0f)
				{
					LogicDebug("Apply damage waiting is resuming...");
					SetUpdate(&DamageObject::StepWork, ML_TRIGGERS);
				}
			}else{
				LogicDebug("Deactivated");
				if(applyDamage > 0.0f)
				{
					LogicDebug("Waiting is paused...");
				}
				DelUpdate(&DamageObject::StepWork);
			}
		}
	}

	//Установить цель
	inline void SetTarget(const ConstString & trgID)
	{
		targetID = trgID;
		FindObject(targetID, target);
		UpdateTarget();
	}

	//Обновить состояние цели
	inline void UpdateTarget()
	{
		target.Validate();
	}

	inline void InitHP(float hp)
	{
		HP = initHP = hp;
		applyDamage = 0.0f;
	}

	//Задержка на кадр
	void _cdecl StepWork(float dltTime, long level)
	{
		DelUpdate(&DamageObject::StepWork);
		Assert(applyDamage >= 0.0f);
		HP -= applyDamage;
		applyDamage = 0.0f;
		if(HP > 0.0f || registerEventEverytime)
		{
			LogicDebug("Register damage, current hp = %f", HP);
			eventDamage.Activate(Mission(), false);
			if(HP > 0.0f) return;
		}
		HP = 0.0f;
		LogicDebug("Triggering, hp is done.");
		eventActivate.Activate(Mission(), false);
		if(autoReset)
		{
			LogicDebug("Autoreset, hp = %f", HP);
		}else{
			Activate(false);
		}
	}

protected:
	float initHP, HP, applyDamage;		//Жизни
	ConstString targetID;				//Идентификатор объекта на который реагировать
	MOSafePointer target;				//Указатель на объект на который реагировать	
	MissionTrigger eventDamage;			//Обработчик попадания
	MissionTrigger eventActivate;		//Обработчик смерти
	bool registerEventEverytime;		//Активировать демедж всегда
	bool autoReset;						//Флажёк режима автосброса после срабатывания	
};



#endif





