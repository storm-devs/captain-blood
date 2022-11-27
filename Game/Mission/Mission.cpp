//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Mission	
//============================================================================================

#include "Mission.h"
#include "MissionLoader.h"


#include "Objects\MissionTime.h"
#include "..\..\common_h\corecmds.h"
#include "..\..\common_h\InputSrvCmds.h"
#include "..\..\common_h\LocStrings.h"

#ifdef LOAD_DEBUG
	#include "CAPROFAPI.h"
	#pragma comment(lib, "CAProfAPI32.lib")
#endif


//============================================================================================

CREATE_CLASS(Mission)

//============================================================================================

#ifndef _XBOX
#define MISSION_DEBUGFILENAME	"logic_debug.txt"
#else
#define MISSION_DEBUGFILENAME	"DEVKIT:\\logic_debug.txt"
#endif
#define MISSION_PARTICLES		"missions"
#define LOGICTRACE_SPACE	"    "

//Текущая миссия при исполнении
//Mission * Mission::current = null;

//Миссия, для которой исполняются команды консоли
#ifndef NO_CONSOLE
Mission * Mission::controlMission = null;
IConsole * Mission::console = null;
bool Mission::enableConsoleLogicDebug = false;
char Mission::debugObjects[64][128];
long Mission::debugObjectsCounts = 0;
char Mission::debugObjectCurrent[256];
#endif

const char * Mission::loadingHint = "";
float Mission::loadingProgress = 0.0f;
float Mission::loadingProgressScale = 1.0f;

//Пути
const char * Mission::missionsPath = "Resource\\Missions\\";
const char * Mission::missionPath = "data\\";

const char * Mission::cutLine = "---------------------------------------------------------------------------------\n";

#ifndef STOP_LOGICTRACE
Mission * Mission::traceFirstInList = null;
#endif

//============================================================================================

void _cdecl Mission::ParticlesUpdater::UpdateParticles(float dltTime, long level)
{
	Particles().Execute(dltTime);
}

void _cdecl Mission::ParticlesUpdater::DrawParticles(float dltTime, long level)
{
	Particles().DrawAllParticles();
}


void _cdecl Mission::PhysicsUpdater::StartFrame(float dltTime, long level)
{
	Physics().UpdateBeginFrame(dltTime);
}

void _cdecl Mission::PhysicsUpdater::EndFrame(float dltTime, long level)
{
	Physics().UpdateEndFrame(dltTime);
}

void _cdecl Mission::AnimationUpdater::UpdateAnimation(float dltTime, long level)
{
	srv->Update(scene, dltTime);
}

void _cdecl Mission::PostEffectsUpdater::PreparePostEffects(float dltTime, long level)
{
}

void _cdecl Mission::PostEffectsUpdater::DrawPostEffects(float dltTime, long level)
{
	Render().PostProcess();
}


//============================================================================================

Mission::Mission() :	regObjectTypes(_FL_),
						group(_FL_),
						timeModifiers(_FL_),
						objectsFindList(_FL_),
						textures(_FL_, 128),
						models(_FL_, 128),
						animations(_FL_, 128),
						boundPlanes(_FL_, 32)
#ifndef STOP_PROFILES
						,creationTable(_FL_)
#endif
{
#ifndef STOP_LOGICTRACE
	traceNextInList = null;
	traceDltTime = 0.0f;
	traceTimeScale = 0.0f;
	traceFPS = 0.0f;
#endif
	bEditModeAdditionalDraw = true;
	loader = null;
	for(long i = 0; i < ARRSIZE(entryGroups); i++) entryGroups[i] = null;
	player = null;
	particlesUpdater = null;
	physicsUpdater = null;
	animationUpdater = null;
	postEffectsUpdater = null;
	dataBuffer = null;
	validateCacheCounter = 0;
	uIdCodeCounter = 1;
	//static const char * objectsListName = "Mission.objectsList";
	objectsList.SetId(GroupId(0, 'O','b','j'));
	//static const char * executeListName = "Mission.executeList";
	executeList.SetId(GroupId(0, 'E','x','e'));
	executeIterator = executeList.CreateIterator(_FL_);
	profileLevel = 0;
	isSleep = false;
	isMissionPause = false;
	isProcessMissionPause = true;
	missionPauseCount = 0;
	swingAngs = 0.0f;
	enableDebug = false;
	largeshot = null;
	largeshotPrjScale = null;
	largeshotPrjX = null;
	largeshotPrjY = null;
	framesCounter = 0;

#ifndef MIS_STOP_EDIT_FUNCS
	isEditMode = true;
#else
	isEditMode = false;
#endif
	for(long i = 0; i < ARRSIZE(regObjectTypesEntry); i++) regObjectTypesEntry[i] = -1;
	mirrorPath = null;


	//TestFrames();



}






Mission::~Mission()
{
#ifndef NO_CONSOLE
	if(controlMission == this)
	{
		controlMission = null;
		console = null;
	}
#endif
	//Разлочиваем доступ к файлам
	if(&Files()) Files().LockFileAccess(false);
	//Удаляем кэшированые ресурсы
	for(long i = 0; i < textures; i++)
	{
		textures[i]->Release();
	}
	textures.Empty();
	for(long i = 0; i < animations; i++)
	{
		animations[i]->Release();
	}
	animations.Empty();
	for(long i = 0; i < models; i++)
	{
		models[i]->Release();
	}
	models.Empty();
	//Удалим подземную ограничивающую плоскость
	for(long i = 0; i < boundPlanes; i++)
	{
		if(boundPlanes[i])
		{
			boundPlanes[i]->Release();
			boundPlanes[i] = null;
		}
	}
	if(particlesUpdater) delete particlesUpdater;
	particlesUpdater = null;
	if(physicsUpdater) delete physicsUpdater;
	physicsUpdater = null;
	if(animationUpdater) delete animationUpdater;
	animationUpdater = null;
	if(postEffectsUpdater) delete postEffectsUpdater;
	postEffectsUpdater = null;
	player = null;
	//Если загрузчик ещё остался, удалим его
	if(loader) delete loader; loader = null;
	//Удалим все объекты
	objectsList.DeleteList();
	//Удалим списки
	for(long i = 0; i < group; i++) delete group[i];
	//Удалим системные итераторы
	RELEASE(executeIterator);
	//Удалим звуковые банки
	ISoundService * ssrv = (ISoundService *)api->GetService("SoundService");
	Assert(ssrv);
	ssrv->ReleaseSoundBank(name.c_str());
	ILocStrings * locStrings = (ILocStrings *)api->GetService("LocStrings");
	Assert(locStrings);
//	locStrings->ReleaseLocSoundBank(name.c_str());
	//Отключаем пак миссии
	if(mirrorPath)
	{
		mirrorPath->Release();
		mirrorPath = null;
	}
	//Удаляем звуковую сцену
	if(&Sound()) Sound().Release();
	//Удаляем партикловый менеджер
	if(&Particles()) Particles().Release();
	//Удаляем физическую сцену	
	if(&Physics()) Physics().Release();
	//Удаляем сцену ввода
	if(&Controls()) Controls().Release();
	//Удалим имена зарегестрённых объектов
	for(long i = 0; i < regObjectTypes; i++)
	{
		delete regObjectTypes[i].type;
	}
	regObjectTypes.Empty();
	//Удаляем анимационную сцену
	if(&Animation()) Animation().Release();
	//Мисионные данные
	if(dataBuffer)
	{
		dataBuffer->Release();
		dataBuffer = null;
	}
	RELEASE(largeshot);
	RELEASE(largeshotPrjScale);
	RELEASE(largeshotPrjX);
	RELEASE(largeshotPrjY);
}

//============================================================================================


bool Mission::EditMode_IsAdditionalDraw ()
{
	return bEditModeAdditionalDraw;
}

void Mission::EditMode_AdditionalDraw (bool value)
{
	bEditModeAdditionalDraw = value;
}

//============================================================================================
//Управление миссией
//============================================================================================
//Заргузить миссию
bool Mission::CreateMission(const char * missionName, float percentsPerMission, float & percentsCounter)
{
	AssertCoreThread
#ifdef LOAD_DEBUG
	CAProfResume();
#endif
	loadingProgressScale = percentsPerMission*0.01f;
	LogicDebug("--------------------------------------------\nLoad mission: %s\n--------------------------------------------\n", missionName);
	name = missionName;
	api->SetWatchDog();
	Sound().ModifyOvnerName(missionName);
	string path = missionsPath;
	path += name;
	path += "\\";
	path += name;
	api->Trace("\n===========================================================================\nCreate mission: %s\n===========================================================================\n", missionName);
	loadingHint = "Load level description...";
	loadingProgress = percentsCounter;	
	LoadingProgress(0.0f);	
	isEditMode = false;
	if(!missionName || !missionName[0])
	{
		api->Trace("CreateMission -> invalidate mission name");
		percentsCounter = loadingProgress;
		return false;
	}	
	if(enableDebug)
	{
		LogicDebug("===========================================================================\nInit mission: \"%s\"\n===========================================================================\n", missionName);
	}
#ifndef _XBOX
	const char * basePacksPath = "resource\\";	
#else
	const char * basePacksPath = "";
#endif
	//Подключаем путь миссии
	path = missionsPath;
	path += name;
	mirrorPath = Files().CreateMirrorPath(path, "Resource\\", _FL_);
	//Загружаем пак с данными объектов
	string packName = basePacksPath;
	packName += "d_";
	packName += missionName;
	packName += ".pkx";
	IPackFile * misPack = Files().LoadPack(packName.c_str(), _FL_);
	//Путь до файла описывающего миссию
	path = "Resource\\";
	path += missionPath;
	path += name;	
	path.AddExtention(".mis");
	//Загружаем файл описания миссии
	dataBuffer = Files().LoadData(path, _FL_);
	if(!dataBuffer)
	{
		api->Trace("CreateMission -> file %s not loaded", path.GetBuffer());		
		percentsCounter = loadingProgress;
		RELEASE(misPack);
		return false;
	}
	//Загружаем паки
	dword loadTime = 0;
	dword creationTime = 0;
	//Текстуры
	api->SetWatchDog();
	dword startTime = GetTickCount();
	LoadingProgress(1.0f);
	loadingHint = "Load textures...";
	packName = basePacksPath;
	packName += "t_";
	packName += missionName;
	packName += ".pkx";
	dword loadTimeForTextures = loadTime;
	dword creationTimeForTextures = creationTime;
	PrecachePack(packName.c_str(), &Mission::PrecacheCreator_Textures, 14.0f, 25.0f, loadTime, creationTime);
	loadTimeForTextures = loadTime - loadTimeForTextures;
	creationTimeForTextures = creationTime - creationTimeForTextures;
	//Анимации
	api->SetWatchDog();
	loadingHint = "Load animations...";
	packName = basePacksPath;
	packName += "a_";
	packName += missionName;
	packName += ".pkx";
	dword loadTimeForAnimations = loadTime;
	dword creationTimeForAnimations = creationTime;
	PrecachePack(packName.c_str(), &Mission::PrecacheCreator_Animation, 5.0f, 5.0f, loadTime, creationTime);
	loadTimeForAnimations = loadTime - loadTimeForAnimations;
	creationTimeForAnimations = creationTime - creationTimeForAnimations;
	//Модельки
	api->SetWatchDog();
	loadingHint = "Load models...";
	packName = basePacksPath;
	packName += "m_";
	packName += missionName;
	packName += ".pkx";
	dword loadTimeForModels = loadTime;
	dword creationTimeForModels = creationTime;
	PrecachePack(packName.c_str(), &Mission::PrecacheCreator_Models, 10.0f, 20.0f, loadTime, creationTime);
	loadTimeForModels = loadTime - loadTimeForModels;
	creationTimeForModels = creationTime - creationTimeForModels;
	dword precacheTime = GetTickCount() - startTime;	
	//Звуковые банки
	api->SetWatchDog();
	dword loadTimeForSounds = GetTickCount();	
	ISoundService * ssrv = (ISoundService *)api->GetService("SoundService");
	Assert(ssrv);
	ssrv->LoadSoundBank(name.c_str());
	api->SetWatchDog();
	ILocStrings * locStrings = (ILocStrings *)api->GetService("LocStrings");
	Assert(locStrings);
//	locStrings->LoadLocSoundBank(name.c_str());
	loadTimeForSounds = GetTickCount() - loadTimeForSounds;
	loadTime += loadTimeForSounds;
	//Пишем отчёт о загрузке
#ifndef STOP_PROFILES
	api->Trace("Packs load time = %.2f seconds, resource creation time = %.2f seconds, total precache time = %.2f seconds", loadTime*0.001f, creationTime*0.001f, precacheTime*0.001f);
	if(profileLevel > 0)
	{
		api->Trace("    Textures load time = %.2f seconds, textures creation time = %.2f seconds", loadTimeForTextures*0.001f, creationTimeForTextures*0.001f);	
		api->Trace("    Animations load time = %.2f seconds, animations creation time = %.2f seconds", loadTimeForAnimations*0.001f, creationTimeForAnimations*0.001f);	
		api->Trace("    Models load time = %.2f seconds, models creation time = %.2f seconds", loadTimeForModels*0.001f, creationTimeForModels*0.001f);	
		api->Trace("    Load time for mission sound banks %.2f seconds", loadTimeForSounds*0.001f);	
	}
#endif
	//Создаём миссионные объекты
	api->SetWatchDog();
	startTime = GetTickCount();
	Assert(loader == null);
	loader = NEW MissionLoader(*this, dataBuffer->Buffer(), dataBuffer->Size());
	loadingHint = "Creating game objects...";
	LoadingProgress(2.0f);
	if(!loader->StartProcess(18.0f))
	{
		api->Trace("CreateMission -> loader create process if failed");
		percentsCounter = loadingProgress;
		RELEASE(misPack);
		return false;
	}
	//Доинициализация объектов
	api->SetWatchDog();
	MGIterator * it = objectsList.CreateIterator(_FL_);
	Assert(it);
	for(; !it->IsDone(); it->Next())
	{
		it->Get()->PostCreate();
	}
	it->Release();
	//	
	LoadingProgress(100.0f);
	api->Trace("Objects creation time = %.2f seconds", (GetTickCount() - startTime)*0.001f);
#ifndef STOP_PROFILES
	if(profileLevel >= 1)
	{
		CreationProfile();
	}
#endif
	api->Trace("Mission \"%s\" loaded successful...", missionName);
	api->ExecuteCoreCommand(CoreCommand_MemStat(cmemstat_onlytotal));
	//Пока грузим синхронно
	api->SetWatchDog();
	if(enableDebug)
	{
		LogicDebug("===========================================================================\nStart mission...\n===========================================================================\n", null);
	}
	loadingHint = "";
#ifdef LOAD_DEBUG
	CAProfPause();
#endif
	percentsCounter = loadingProgress;
	RELEASE(misPack);
	return true;
}

void Mission::PrecachePack(const char * packPath, PrecacheCreator creator, float filePerc, float dataPerc, dword & loadTime, dword & creationTime)
{
	dword startTime = GetTickCount();
	IPackFile * pack = Files().LoadPack(packPath, _FL_);
	loadTime += GetTickCount() - startTime;
	LoadingProgress(filePerc);
	float step = dataPerc;
	if(pack)
	{
		if(pack->Count())
		{
			step = step/pack->Count();
		}else{
			LoadingProgress(step);
		}
		startTime = GetTickCount();
		textures.Reserve(pack->Count());
		for(dword i = 0; i < pack->Count(); i++)
		{
			const char * name = pack->LocalPath(i);
			(this->*creator)(name);
			LoadingProgress(step);
		}
		creationTime += GetTickCount() - startTime;
		pack->Release();
		pack = null;
	}else{
		LoadingProgress(step);
	}

}

void Mission::PrecacheCreator_Textures(const char * name)
{
	IBaseTexture * texture = Render().CreateTexture(_FL_, name);
	if(texture)
	{
		textures.Add(texture);
	}
}

void Mission::PrecacheCreator_Animation(const char * name)
{
	IAnimation * animation = Animation().Create(name, _FL_);
	if(animation)
	{
		animations.Add(animation);
	}
}

void Mission::PrecacheCreator_Models(const char * name)
{
	IGMXScene * model = Geometry().CreateScene(name, &Animation(), &Particles(), &Sound(), _FL_);
	if(model)
	{
		models.Add(model);
	}
}

void Mission::LoadingProgress(float delta)
{
	loadingProgress += delta;
	if(loadingProgress > 100.0f)
	{
		loadingProgress = 100.0f;
	}
	Render().SetLoadingScreenPercents(loadingProgress, 100.0f, loadingHint);
}

//Удалить миссию
void Mission::DeleteMission()
{
	delete this;
}

//Получить имя загруженой миссии
const char * Mission::GetMissionName()
{
	return name;
}

//Закончен ли процесс загрузки
bool Mission::IsLoadDone()
{
	if(loader)
	{
		if(loader->IsDone())
		{
			delete loader;
			loader = null;
			return true;
		}
		return false;
	}

	return true;
}

//Добавить в список модификатор времени
void Mission::AddTimeModifier(float * modifier)
{
	for(long i = 0; i < timeModifiers; i++)
	{
		if(timeModifiers[i] == modifier)
		{
			return;
		}
	}
	timeModifiers.Add(modifier);
}

//Добавить из списока модификатор времени
void Mission::RemoveTimeModifier(float * modifier)
{
	for(long i = 0; i < timeModifiers; i++)
	{
		if(timeModifiers[i] == modifier)
		{
			timeModifiers.DelIndex(i);
			return;
		}
	}
}

//Пауза миссии
void Mission::MissionPause(bool isPause)
{
	if(isPause)
	{
		if(missionPauseCount == 0)
		{
			//Встать на паузу
			LogicDebug("%s\nPause mission: %s\n%s", cutLine, name.c_str(), cutLine);
			MGIterator & it = GroupIterator(DEACTIVATE_EVENT_GROUP, _FL_);
			for(; !it.IsDone(); it.Next())
			{
				it.ExecuteEvent();
			}
			it.Release();
			if(services.sound)
			{
				services.sound->Pause();
			}
			if(services.ctrl)
			{
				services.ctrl->Activate(false);
			}
		}
		missionPauseCount++;
	}else{
		if(missionPauseCount > 0)
		{
			missionPauseCount--;			
			Assert(missionPauseCount >= 0);
			if(missionPauseCount == 0)
			{
				//Возобновить работу
				LogicDebug("%s\nResume mission: %s\n%s", cutLine, name.c_str(), cutLine);
				MGIterator & it = GroupIterator(ACTIVATE_EVENT_GROUP, _FL_);
				for(; !it.IsDone(); it.Next())
				{
					it.ExecuteEvent();
				}
				it.Release();
				if(services.sound)
				{
					services.sound->Resume();
				}
				if(services.ctrl)
				{
					services.ctrl->Activate(true);
				}
			}
		}
	}
}

//============================================================================================
//Функции для редактора миссий
//============================================================================================

#ifndef NO_TOOLS

//Подключить пак для редактора если нет, то создать
void Mission::EditorSetPack(const char * missionName)
{
	Assert(isEditMode);
	if(!missionName || !missionName[0]) return;
	if(name.Len() > 0) return;
	//Сохраним имя миссии
	name = missionName;
	Sound().ModifyOvnerName(missionName);
	//Путь до пака миссии
	string path = missionsPath;
	path += name;
	path += "\\";
	//Создадим путь
	Files().CreateFolder(path);
	//Подключим заркальный путь
	path = missionsPath;
	path += name;
	if(mirrorPath)
	{
		mirrorPath->Release();
		mirrorPath = null;
	}
	mirrorPath = Files().CreateMirrorPath(path, "Resource\\", _FL_);
}

//Получить путь до пак-файла миссии
void Mission::EditorGetPackPath(string & path)
{
	Assert(isEditMode);
	path.Empty();
	if(name.Len() == 0) return;
	path = missionsPath;
	path += name;
	if(name.Len() > 0) path += "\\";
	path.CheckPath();
}

//Получить путь до файла миссии .mis, чтобы сохранить его
void Mission::EditorGetMisPath(string & path)
{
	Assert(isEditMode);
	path.Empty();
	if(name.Len() == 0) return;
	path = missionsPath;
	path += name;
	if(name.Len() > 0) path += "\\";
	path += missionPath;
	path += "\\";
	path += name;
	path += ".mis";
	path.CheckPath();
}

//Усыпить/разбудить миссию
void Mission::EditorSetSleep(bool isSleep)
{
	Assert(isEditMode);
	if(this->isSleep == isSleep) return;
	this->isSleep = isSleep;
	MGIterator * it = objectsList.CreateIterator(_FL_);
	Assert(it);
	for(; !it->IsDone(); it->Next())
	{
		it->Get()->EditMode_Sleep(isSleep);
	}
	it->Release();
	MissionPause(isSleep);
}

//Обновить объект
void Mission::EditorUpdateObject(MissionObject * mo, MOPWriter & writer)
{
	Assert(isEditMode);
	if(!mo) return;
	void * data = null;
	dword readerDataSize = writer.GetData(data);
	void * readerData = NEW byte[readerDataSize];
	memcpy(readerData, data, readerDataSize);	
	MOPReader reader(readerData, readerDataSize);
	bool isDifferentName = (string::NotEqual(reader.GetObjectID(), mo->GetObjectID().c_str()));
	if(isDifferentName)
	{
		RemoveMissionObjectFromFFTable(mo);
	}
	sysSetObjectReaderDataBeforeCreate(mo, readerData, readerDataSize, readerData);
	sysSetObjectIDBeforeCreate(mo, reader.GetObjectID());
	if(isDifferentName)
	{		
		AddMissionObjectToFFTable(mo);
	}
	sysEditMode_Update(mo, reader);
}

//Нарисовать миссию
void Mission::EditorDraw(float dltTime)
{
	isProcessMissionPause = false;
	FrameUpdate(dltTime);
}


#endif

//============================================================================================
//Управление объектами миссии
//============================================================================================


//Динамическое создание миссионного объекта
MissionObject * Mission::sysCreateObject(const char * type, const ConstString & id, bool alone)
{
	if(id.IsEmpty()) return null;
	if(alone)
	{
		MissionObject * mo = FindObject(id);
		if(mo)
		{
			return mo;
		}
	}
	MOPWriter writer('0.00', id.c_str());
	MOPReader reader = writer.Reader();
	dword readerDataSize = 0;
	void * readerData = reader.ReplaseData(readerDataSize);
	const char * rid = reader.GetObjectID();
	MissionObject * mo = sysCreateObject(type, reader);
	//Регистрации
	RemoveMissionObjectFromFFTable(mo);
	sysSetObjectReaderDataBeforeCreate(mo, readerData, readerDataSize, readerData);
	sysSetObjectIDBeforeCreate(mo, rid);
	AddMissionObjectToFFTable(mo);
	return mo;
}

//Динамическое создание миссионного объекта
MissionObject * Mission::sysCreateObject(const char * type, MOPReader & rd)
{
	if(!type || !type[0]) return null;
	//Создаём объект
	MissionObject * mo = (MissionObject *)api->CreateObject(type);
	if(!mo) return null;
	SetObjectUId(mo);
	type = CreateObject_AddType(type);
	//В режиме редактора дублируем имя
#ifndef MIS_STOP_EDIT_FUNCS
	if(isEditMode)
	{	
		dword readerDataSize;
		void * readerData = rd.ReplaseData(readerDataSize);
		sysSetObjectReaderDataBeforeCreate(mo, readerData, readerDataSize, readerData);
	}
#endif
	const char * objectID = rd.GetObjectID();
	//Проставляем указатель на себя
	sysSetThisPointerToObject(mo);
	//Устанавливаем идентификатор	
	sysSetObjectIDBeforeCreate(mo, objectID);
	sysSetObjectTypeBeforeCreate(mo, type);
	//Ставим текущий режим
	sysSetObjectEMBeforeCreate(mo, isEditMode);
	//Заносим в таблицу поиска
	AddMissionObjectToFFTable(mo);
	//Инициализируем
	bool isOk = true;
	if(!isEditMode)
	{
#ifndef STOP_PROFILES
		if(profileLevel < 2)
		{
			if(!mo->Create(rd)) isOk = false;
		}else{
			ProfileTimer timer;
			if(!mo->Create(rd)) isOk = false;
			timer.Stop();
			ProfileCreation & cr = creationTable[creationTable.Add()];
			cr.time = timer.GetTime64();
			cr.type = type;
			cr.id = objectID;
		}
#else
		if(!mo->Create(rd)) isOk = false;
#endif
	}else{
#ifndef MIS_STOP_EDIT_FUNCS
		if(!sysEditMode_Create(mo, rd)) isOk = false;
#endif
	}
	if(!isOk)
	{
		sysDelUpdate(mo, null);
		sysUnregistryAll(mo);
		delete mo;
		return null;
	}
	MissionObjectsList::Func func;
	func.funcEvent = null;
	objectsList.Add(mo, 0, func);
#ifndef MIS_STOP_EDIT_FUNCS
	if(isEditMode)
	{
		mo->EditMode_Sleep(isSleep);
	}
#endif
	return mo;
}

const char * Mission::CreateObject_AddType(const char * type)
{
	dword typeHash = string::Hash(type);	
	long index = typeHash & (ARRSIZE(regObjectTypesEntry) - 1);
	for(long i = regObjectTypesEntry[index]; i >= 0; i = regObjectTypes[i].next)
	{
		TypeElement & rte = regObjectTypes[i];
		if(rte.hash == typeHash)
		{
			if(strcmp(rte.type, type) == 0)
			{
				return rte.type;
			}
		}
	}
	TypeElement te;
	te.hash = typeHash;
	te.next = regObjectTypesEntry[index];
	long len = strlen(type) + 1;
	te.type = NEW char[len];
	memcpy(te.type, type, len);
	regObjectTypesEntry[index] = (long)regObjectTypes.Add(te);
	return te.type;
}

//Перезапуск всех объектов миссии
void Mission::RestartAllObjects()
{
	LogicDebug("\n%sGlobal mission restart (mission:\"%s\")\n%s", cutLine, name.c_str(), cutLine);
	//Рестартим все объекты
	MGIterator * it = objectsList.CreateIterator(_FL_);
	Assert(it);
	for(; !it->IsDone(); it->Next())
	{
		MissionObject * mo = it->Get();
		Assert(mo);
		mo->Restart();
	}
	it->Release();
	//Проматываем итератор на конец списка
	while(!executeIterator->IsDone())
	{
		executeIterator->Next();
	}
	LogicDebug("\n%s\n\n", cutLine);
}

//Найти объект по идентификатору
MissionObject * Mission::FindObject(const ConstString & id)
{
	//Поиск через таблицу
	if(id.IsEmpty()) return null;
	long hash = id.Hash();
	long index = GetIndexInFFTableFromHash(hash);
	long count = fastFindTable[index].mo.Size();
	MissionObject ** mos = fastFindTable[index].mo.GetBuffer();
	for(long i = 0; i < count; i++)
	{
		MissionObject * mo = mos[i];
		if(mo->GetObjectID() == id)
		{
			return mo;
		}
	}
	return null;
}
/*
//Изменить состояние видимости объекта
MissionObject * Mission::ObjectShow(const char * id, bool isShow)
{
	MissionObject * mo = FindObject(id);
	if(!mo) return null;
	mo->Show(isShow);
	return mo;
}

//Изменить состояние активности объекта
MissionObject * Mission::ObjectActivate(const char * id, bool isActive)
{
	MissionObject * mo = FindObject(id);
	if(!mo) return null;
	mo->Activate(isActive);
	return mo;
}*/

//Получить итератор по группе
MGIterator & Mission::GroupIterator(GroupId group, const char* cppFileName, long cppFileLine)
{
	//Получаем группу
	Group * grp = FindGroup(group);
	if(!grp)
	{
		sysRegistry(group, null, null, 0);
		grp = FindGroup(group);
	}
	//Создаём итератор
	MGIterator * it = grp->list.CreateIterator(cppFileName, cppFileLine);
	Assert(it);
	return *it;
}

//Изменить состояние видимости группы
void Mission::GroupShow(GroupId group, bool isShow)
{
	for(MGIterator & it = GroupIterator(group, _FL_); !it.IsDone(); it.Next())
	{
		it.Get()->Show(isShow);
	}
	it.Release();
}

//Изменить состояние активности группы
void Mission::GroupActivate(GroupId group, bool isActive)
{
	for(MGIterator & it = GroupIterator(group, _FL_); !it.IsDone(); it.Next())
	{
		it.Get()->Activate(isActive);
	}
	it.Release();
}

//Получить игрока
MissionObject * Mission::Player()
{
	if(player) return player;
	static const ConstString name("Player");
	MissionObject * mo = FindObject(name);
	if(!mo) return null;
/*(	if(!mo->Is("AIObject"))
	{
		api->Trace("Player not AIObject!");
		return null;
	}*/
	player = mo;
	return player;
}

//Получить матрицу качки
const Matrix & Mission::GetSwingMatrix()
{
	return swingMatrix;
}

//Получить инверсную матрицу качки
const Matrix & Mission::GetInverseSwingMatrix()
{
	return swingInverseMatrix;
}

//Получить параметры качки
const Vector & Mission::GetSwingParams()
{
	return swingAngs;
}

//Установить параметры качки
void Mission::SetSwingParams(const Vector & angs)
{
	swingAngs = angs;
	swingMatrix.Build(swingAngs);
	swingInverseMatrix.Inverse(swingMatrix);
}

//============================================================================================

//Инициализация
bool Mission::Init()
{
	//Получаем переменные
	largeshot = api->Storage().GetItemLong("system.screenshot.Largeshot", _FL_);
	largeshotPrjScale = api->Storage().GetItemFloat("system.screenshot.Projection scale", _FL_);
	largeshotPrjX = api->Storage().GetItemFloat("system.screenshot.Projection x", _FL_);
	largeshotPrjY = api->Storage().GetItemFloat("system.screenshot.Projection y", _FL_);
	//Получаем указатели на сервисы
	services.file = (IFileService *)api->GetService("FileService");
	if(!services.file)
	{
		api->Trace("Mission::Init -> FileService is not created!");
		return false;
	}
	services.render = (IRender *)api->GetService("DX9Render");
	if(!services.render)
	{
		api->Trace("Mission::Init -> DX9Render is not created!");
		return false;
	}
	services.render->SetBackgroundColor(Color((dword)0xff000000));
	IAnimationService * ass = (IAnimationService *)api->GetService("AnimationService");
	if(!ass)
	{
		api->Trace("Mission::Init -> AnimationService is not created!");
		return false;
	}
	services.animation = ass->CreateScene(_FL_);
	//
	services.geometry = (IGMXService *)api->GetService("GMXService");
	if(!services.geometry)
	{
		api->Trace("Mission::Init -> GMXService is not created!");
		return false;
	}
	soundService = (ISoundService *)api->GetService("SoundService");
	if(!soundService)
	{
		api->Trace("Mission::Init -> SoundService is not created!");
		return false;
	}
	services.sound = soundService->CreateScene("Mission", _FL_);
	if(!services.sound)
	{
		api->Trace("Mission::Init -> SoundService is not created sound context!");
		return false;
	}
	IParticleService * psrv = (IParticleService *)api->GetService("ParticleService");
	if(!psrv)
	{
		api->Trace("Mission::Init -> ParticleService is not created!");
		return false;
	}
	services.particles = psrv->CreateManager(null);
	if(!services.particles)
	{
		api->Trace("Mission::Init -> ParticleManager is not created!");
		return false;
	}
	IControlsService * ictrl = (IControlsService *)api->GetService("ControlsService");
	services.ctrl = ictrl->CreateInstance(_FL_);
	if(!services.ctrl)
	{
		api->Trace("Mission::Init -> Controls not created!");
		return false;
	}
	services.ctrl->ExecuteCommand(InputSrvLockMouse(true));
	services.colsole = (IConsole *)api->GetService("Console");
	if(!services.colsole)
	{
		services.colsole = &fakeConsole;
	}
	IPhysics * phs = (IPhysics *)api->GetService("PhysicsService");
	if(!phs)
	{
		api->Trace("Mission::Init -> PhysicsService is not created!");
		return false;
	}
	services.physics = phs->CreateScene();
	if(!services.physics)
	{
		api->Trace("Mission::Init -> PhysycsScene is not created!");
		return false;
	}
	services.liveService = (ILiveService *)api->GetService("LiveService");
	if(!services.liveService)
	{
		api->Trace("Mission::Init -> LiveSrvice is not created!");
		return false;
	}

	/*
	IPhysicsScene * scene = services.physics;
	IPhysEditableRagdoll * ragdoll = scene->CreateEditableRagdoll();
	IPhysEditableRagdoll::IBone & bone = ragdoll->GetRootBone();
	bone.SetBoxShape(Matrix(), Vector(1.0f), 1.0f);
	IPhysEditableRagdoll::IBone & chld = bone.AddChild();
	chld.SetCapsuleShape(Matrix(0.0f, Vector(0.0f, 2.5f, 0.0f)), 2.0f, 0.3f, 3.0f);
	IPhysEditableRagdoll::SphericalJointParams params;
	params.worldJointPosition = Vector(0.0f, 1.0f, 0.0f);
	bone.CreateSphericalJoint(chld, params);
	*/


	//Ограничивающий объём
	Plane planes[6];
	planes[0] = Plane(Vector(0.0f, 1.0f, 0.0f), Vector(0.0f, -1000.0f, 0.0f));
	planes[1] = Plane(Vector(0.0f, -1.0f, 0.0f), Vector(0.0f, 2000.0f, 0.0f));
	const float planeMaxDist = 50000.0f;
	planes[2] = Plane(Vector(0.0f, 0.0f, 1.0f), Vector(0.0f, 0.0f, -planeMaxDist));
	planes[3] = Plane(Vector(0.0f, 0.0f, -1.0f), Vector(0.0f, 0.0f, planeMaxDist));
	planes[4] = Plane(Vector(1.0f, 0.0f, 0.0f), Vector(-planeMaxDist, 0.0f, 0.0f));
	planes[5] = Plane(Vector(-1.0f, 0.0f, 0.0f), Vector(planeMaxDist, 0.0f, 0.0f));
	PhysicsCollisionGroup boundGroups[] = {
		phys_world, phys_ragdoll, phys_character, 
		phys_player, phys_ship, phys_physobjects, 
		phys_charitems};

	boundPlanes.Reserve((ARRSIZE(planes))*(ARRSIZE(boundGroups)));
	for(long i = 0; i < ARRSIZE(planes); i++)
	{
		Plane & p = planes[i];
		for(long j = 0; j < ARRSIZE(boundGroups); j++)
		{
			IPhysPlane * pl = services.physics->CreatePlane(_FL_, p.N, p.D);
			if(pl)
			{
				pl->SetGroup(boundGroups[j]);
				boundPlanes.Add(pl);
			}			
		}
	}
	//Объект орбновляющий партиклы
	particlesUpdater = NEW ParticlesUpdater();
	sysSetThisPointerToObject(particlesUpdater);
	sysSetObjectIDBeforeCreate(particlesUpdater, "ParticlesUpdater");	
	sysSetObjectTypeBeforeCreate(particlesUpdater, "ParticlesUpdater");
	sysSetUpdate(ML_TRIGGERS-1, particlesUpdater, (MOF_UPDATE)&ParticlesUpdater::UpdateParticles);
	sysSetUpdate(ML_PARTICLES5, particlesUpdater, (MOF_UPDATE)&ParticlesUpdater::DrawParticles);
	//Объект орбновляющий физику
	physicsUpdater = NEW PhysicsUpdater();
	sysSetThisPointerToObject(physicsUpdater);
	sysSetObjectIDBeforeCreate(physicsUpdater, "PhysicsUpdater");
	sysSetObjectTypeBeforeCreate(physicsUpdater, "PhysicsUpdater");
	sysSetUpdate(ML_FIRST, physicsUpdater, (MOF_UPDATE)&PhysicsUpdater::StartFrame);
	sysSetUpdate(ML_LAST, physicsUpdater, (MOF_UPDATE)&PhysicsUpdater::EndFrame);
	//Объект орбновляющий анимацию
	animationUpdater = NEW AnimationUpdater();
	sysSetThisPointerToObject(animationUpdater);
	sysSetObjectIDBeforeCreate(animationUpdater, "AnimationUpdater");
	sysSetObjectTypeBeforeCreate(animationUpdater, "AnimationUpdater");
	animationUpdater->scene = &Animation();
	animationUpdater->srv = ass;
	sysSetUpdate(ML_LAST, animationUpdater, (MOF_UPDATE)&AnimationUpdater::UpdateAnimation);
	//Объект инициирующий рисование постэффектов
	postEffectsUpdater = NEW PostEffectsUpdater();
	sysSetThisPointerToObject(postEffectsUpdater);
	sysSetObjectIDBeforeCreate(postEffectsUpdater, "PostEffectsUpdater");
	sysSetObjectTypeBeforeCreate(postEffectsUpdater, "PostEffectsUpdater");
	sysSetUpdate(ML_FIRST + 1, postEffectsUpdater, (MOF_UPDATE)&PostEffectsUpdater::PreparePostEffects);
	sysSetUpdate(ML_POSTEFFECTS, postEffectsUpdater, (MOF_UPDATE)&PostEffectsUpdater::DrawPostEffects);
	//Флажёк дебага
	IIniFile * ini = services.file->SystemIni();
	if(ini)
	{
		enableDebug = ini->GetLong("Mission", "Debug", 0) != 0;
#ifndef _XBOX
		static bool isFirstStart = true;
		if(isFirstStart && ini->GetLong("Mission", "Debug clear", 0) != 0)
		{
			isFirstStart = false;
			services.file->Delete(MISSION_DEBUGFILENAME);
		}		
#endif
		profileLevel = ini->GetLong("Mission", "Profile", 0);
	}
	const char * errorDB = "! error access to database !";
	LogicDebug("\n\n\n%s", cutLine);	
	LogicDebug("Platform: %s", api->Storage().GetString("runtime.platform", errorDB));
	LogicDebug("Core: %s", api->Storage().GetString("system.core.id", errorDB));
	LogicDebug("Build time: %s", api->Storage().GetString("system.core.build", errorDB));
	LogicDebug("Game start time: %s", api->Storage().GetString("system.core.starttime", errorDB));	
	char tmpbuf[128];
	__time64_t ltime;
	_time64(&ltime);
	struct tm today;
	crt_localtime64(&today, &ltime);
	strftime(tmpbuf, sizeof(tmpbuf), "%d %B %Y, %H:%M:%S", &today);	
	LogicDebug("Mission load time: %s", tmpbuf);
	LogicDebug("%s\n\n\n", cutLine);
	//Будем исполняться
	api->SetObjectExecution(this, "mission", 0x1000, &Mission::FrameUpdate);
#ifndef NO_CONSOLE
	debugObjectCurrent[0] = 0;
	Console().Register_PureC_Command("activate", "activate <mission object name> : activate some mission object", Console_ActivateObject);
	Console().Register_PureC_Command("deactivate", "deactivate <mission object name> : Deactivate some mission object", Console_DeactivateObject);
	Console().Register_PureC_Command("show", "show <mission object name> : Show some mission object", Console_ShowObject);
	Console().Register_PureC_Command("hide", "hide <mission object name> : Hide some mission object", Console_HideObject);
	Console().Register_PureC_Command("command", "command <mission object name> command_name command params: Send command to some mission object", Console_CommandToObject);
	Console().Register_PureC_Command("state", "state <mission object name> : View state of some mission object", Console_ViewStateObject);
	Console().Register_PureC_Command("starttrace", "Start trace debug messages from set objects", Console_OnLogicDebug);
	Console().Register_PureC_Command("stt", "Start trace debug messages from set objects", Console_OnLogicDebug);
	Console().Register_PureC_Command("stoptrace", "Stop trace debug messages from set objects", Console_OffLogicDebug);
	Console().Register_PureC_Command("stp", "Stop trace debug messages from set objects", Console_OffLogicDebug);
	Console().Register_PureC_Command("trace", "add <mission object name> : Add object to debug trace list", Console_AddDebugObject);
	//Console().Register_PureC_Command("tc", "add <mission object name> : Add object to debug trace list", Console_AddDebugObject);
	Console().Register_PureC_Command("notrace", "del <mission object name> : Remove object from debug trace list", Console_DelDebugObject);
	//Console().Register_PureC_Command("ntc", "del <mission object name> : Remove object from debug trace list", Console_DelDebugObject);
	Console().Register_PureC_Command("tracelist", "Show trace debug list", Console_ViewDebugObjects);
	//Console().Register_PureC_Command("tl", "Show trace debug list", Console_ViewDebugObjects);
	Console().Register_PureC_Command("clearlist", "Clear trace debug list", Console_ClearDebugObjects);
	//Console().Register_PureC_Command("cll", "Clear trace debug list", Console_ClearDebugObjects);
	Console().Register_PureC_Command("sounds", "sounds [none|2d|3d|lis|lis3d|all] [1|2]; default parameters: all 2", Console_ShowSoundsDebug);
	Console().Register_PureC_Command("storage", "Show storage access commands", Console_Storage);
	Console().Register_PureC_Command("fc", "Activate/deactivatre \"Free camere\"", Console_FreeCamera);
	Console().Register_PureC_Command("locstrings", "Output to system log unuse loc strings", Console_TraceUnuse);
	/*
	Console().Register_PureC_Command("storageview", "storageview [field_name1 field_name2 ... field_nameX]", Console_StorageView);
	Console().Register_PureC_Command("storageset", "storageset type(string,long,float) field_name value", Console_StorageSet);
	Console().Register_PureC_Command("storagedel", "storagedel field_name", Console_StorageDel);
	Console().Register_PureC_Command("storagecopy", "storagecopy field_name_from field_name_to", Console_StorageCopy);
	*/
#endif
	return true;
}

//Создать миссионный объект в режиме игры
bool Mission::CreateObjectEx(const char * name, MOPReader & rd, const void * initData, long initDataSize)
{
	MissionObject * obj = sysCreateObject(name, rd);
	if(!obj)
	{
		return false;
	}
	sysSetObjectReaderDataBeforeCreate(obj, initData, initDataSize, null);
	return true;
}

//Отрисовка
void __fastcall Mission::FrameUpdate(float dltTime)
{
	if(isSleep) return;
#ifndef NO_CONSOLE	
	controlMission = this;
#endif

	//Получим текущий коэфициент масштаба времени
	float timeScale = 1.0f;	
	long timescaleGroupCount = timeModifiers;
	float ** tms = timeModifiers.GetBuffer();
	for(long i = 0; i < timescaleGroupCount; i++)
	{
		timeScale *= *tms[i];
	}
	dltTime *= timeScale;
	if(loader)
	{
		if(loader->IsDone())
		{
			delete loader;
			loader = null;
		}else{
			return;
		}
	}

#ifndef STOP_LOGICTRACE	
	/*
	for(Mission ** missStart = &traceFirstInList; *missStart; missStart = &((*missStart)->traceNextInList))
	{
		Assert(*missStart != this);
	}
	*missStart = this;
	traceNextInList = null;
	*/	
	//По рекурсии список выведеться задом наперёд
	traceNextInList = traceFirstInList;	
	traceFirstInList = this;
	traceDltTime = dltTime;
	traceTimeScale = timeScale;
	traceFPS = api->EngineFps();
	traceIsOutHeader = false;
#endif	

#ifndef _XBOX
	if(api->DebugKeyState(VK_F7))
	{
		if(api->DebugKeyState(VK_SHIFT))
		{
			api->Trace("\nMemory statistics:");
			api->Trace("      SHIFT+F7 - sort by size");
			api->Trace("      SHIFT+1+F7 - sort by blocks count");
			api->Trace("      SHIFT+2+F7 - sort by alloc frequency");
			api->Trace("      SHIFT+3+F7 - sort by files\n");
			if(api->DebugKeyState('1'))
			{
				CoreCommand_MemStat memStats(cmemstat_byblocks);
				api->ExecuteCoreCommand(memStats);
			}else
			if(api->DebugKeyState('2'))
			{
				CoreCommand_MemStat memStats(cmemstat_byfreq);
				api->ExecuteCoreCommand(memStats);
			}else
			if(api->DebugKeyState('3'))
			{
				CoreCommand_MemStat memStats(cmemstat_byfile);
				api->ExecuteCoreCommand(memStats);
			}else{
				CoreCommand_MemStat memStats(cmemstat_bysize);
				api->ExecuteCoreCommand(memStats);
			}
		}else{
			StartProfile();
		}
	}else{
		StopProfile();
	}
	if(api->DebugKeyState('R'))
	{
		if(api->DebugKeyState(VK_SHIFT))
		{
			api->SetTimeScale(6.0f);
		}else{
			api->SetTimeScale(3.0f);
		}
	}else
	if(api->DebugKeyState('Y'))
	{
		if(api->DebugKeyState(VK_SHIFT))
		{
			api->SetTimeScale(0.05f);
		}else{
			api->SetTimeScale(0.3f);
		}
	}else{
		api->SetTimeScale(1.0f);
	}
	if(isProcessMissionPause && api->DebugKeyState(VK_PAUSE))
	{
		isMissionPause = !isMissionPause;
		MissionPause(isMissionPause);
		Sleep(100);
	}
#endif



	/*

	Вернуть когда будет переписан звуковой сервис

	if(!isEditMode)
	{
		Files().LockFileAccess(true);
	}
	*/


	if(isMissionPause) dltTime = 0.0f;
	Sound().SetListenerMatrix(Matrix(Render().GetView()).Inverse());
	bool isCorrectPrj = largeshot ? largeshot->Get(0) != 0 : false;
	for(executeIterator->Reset(); !executeIterator->IsDone(); executeIterator->Next())
	{
		if(isCorrectPrj && executeIterator->Level() > ML_CAMERAMOVE_FREE)
		{
			isCorrectPrj = false;
			Matrix prj = Render().GetProjection();
			float scale = largeshotPrjScale ? largeshotPrjScale->Get(8.0f) : 8.0f;
			float x = largeshotPrjX ? largeshotPrjX->Get(0.0f) : 0.0f;
			float y = largeshotPrjY ? largeshotPrjY->Get(0.0f) : 0.0f;
			prj.vx.x *= scale;
			prj.vy.y *= scale;
			prj.vz.x =	scale - 1.0f - x*2.0f;
			prj.vz.y =	scale - 1.0f - (scale - 1.0f - y)*2.0f;
			Render().SetProjection(prj);
		}
#ifdef _XBOX
#ifndef STOP_PROFILES
		MissionObject * mo = executeIterator->Get();
		if(mo)
		{
//			PIXBeginNamedEvent(0, mo->GetObjectType());
		}		
		ProfileTimer timer;
#endif
#endif
		executeList.ExecuteUpdate(dltTime, executeIterator);
#ifdef _XBOX
#ifndef STOP_PROFILES
		if(mo)
		{
			timer.AddToCounter(mo->GetObjectType(), 1000.0f/50000000.0f);
//			PIXEndNamedEvent();
		}
#endif
#endif
	}
	soundService->DebugDraw();
	for(dword i = 0; i < validateCacheCounter; i++)
	{
		validateCache[i].useCounter = 1;
	}
	Physics().DebugDraw(Render());
	if(!isEditMode)
	{
		Files().LockFileAccess(false);
	}
#ifndef STOP_LOGICTRACE	
	//Выводим сообщение об окончании кадра
	if(traceIsOutHeader)
	{
		LogicDebug("---------<Frame end: frame = %d, mission name = %s>---------", framesCounter, name.c_str());
	}	
	//Удаляем себя из списка активных
	for(Mission ** missEnd = &traceFirstInList; *missEnd; )
	{
		if(*missEnd == this)
		{
			*missEnd = (*missEnd)->traceNextInList;
		}else{
			missEnd = &((*missEnd)->traceNextInList);
		}
	}

#endif
	framesCounter++;
}

//Запустить встроенный профайлинг
void Mission::StartProfile()
{
#ifndef STOP_PROFILES
	MissionProfiler::Start(profileLevel);
#endif
}

//Остановить встроенный профайлинг
void Mission::StopProfile()
{
#ifndef STOP_PROFILES
	MissionProfiler::Stop();
#endif
}

bool Mission::CompareProfileElements(Element * const & grtElm, Element * const & lesElm)
{
#ifndef STOP_PROFILES
	return lesElm->sum < grtElm->sum;
#else
	return false;
#endif
}

bool Mission::CompareProfileObjects(const ProfileObject & grtElm, const ProfileObject & lesElm)
{
#ifndef STOP_PROFILES
	return lesElm.sum < grtElm.sum;
#else
	return false;
#endif
}

bool Mission::CompareProfileTypes(const ProfileType & grtElm, const ProfileType & lesElm)
{
#ifndef STOP_PROFILES
	return lesElm.sum < grtElm.sum;
#else
	return false;
#endif
}

bool Mission::CompareProfileCreations(const ProfileCreation & grtElm, const ProfileCreation & lesElm)
{
#ifndef STOP_PROFILES
	return lesElm.time < grtElm.time;
#else
	return false;
#endif
}

//Подвести итоги профайла загрузки
void Mission::CreationProfile()
{
#ifndef STOP_PROFILES
	//Таблица по типам
	array<ProfileType> profileTypeTable(_FL_);
	double full = 0.0f;
	for(long i = 0; i < creationTable; i++)
	{
		ProfileCreation & pc = creationTable[i];
		full += (double)pc.time;
		for(long j = 0; j < profileTypeTable; j++)
		{
			ProfileType & pt = profileTypeTable[j];
			if(pc.type == pt.type || string::IsEqual(pc.type, pt.type))
			{				
				pt.sum += (double)pc.time;
				pt.counter++;
				if(pt.max < (double)pc.time) pt.max = (double)pc.time;
				break;
			}
		}
		if(j >= profileTypeTable)
		{
			ProfileType & pt = profileTypeTable[profileTypeTable.Add()];
			pt.type = pc.type;
			pt.sum = pt.max = (double)pc.time;
			pt.counter = 1;
		}
	}
	profileTypeTable.QSort(&Mission::CompareProfileTypes);
	//Выводим накопленные данные
	static const char * stopLine = "------------------------------------------------------------------";
	api->Trace(stopLine);
	api->Trace("Mission creation profile info start");
	api->Trace(stopLine);
	api->Trace("");
	api->Trace("Brake per type");
	api->Trace("");
	api->Trace("Brake\t\tpercents              objectType      objects        average         max             total");
	api->Trace("");
	for(i = 0; i < profileTypeTable; i++)
	{		
		ProfileType & t = profileTypeTable[i];
		double average = t.sum/double(t.counter ? t.counter : 1);
		api->Trace("%5d\t\t%4.4f\t    %20s     %8d     %10.0f    %10.0f    %10.0f",
			i + 1, 
			t.sum/full*100.0,
			t.type,
			t.counter ? t.counter : 1,
			average + 0.5,
			t.max + 0.5,
			t.sum);
	}
	api->Trace("");
	if(profileLevel > 1)
	{
		creationTable.QSort(&Mission::CompareProfileCreations);
		api->Trace("");
		api->Trace("Brake per object");
		api->Trace("");
		api->Trace("Brake\t\tpercents              objectType                                    object          time");
		api->Trace("");
		for(long i = 0; i < creationTable; i++)
		{		
			ProfileCreation & pc = creationTable[i];
			api->Trace("%5d\t\t%4.4f\t    %20s          %32s     %10.0f",
				i + 1, 
				(double)pc.time/full*100.0,
				pc.type,
				pc.id,
				(float)pc.time);
		}		
	}
	api->Trace(stopLine);
	api->Trace("Total: %f", full);
	api->Trace("Create objects: %i", creationTable.Size());
	api->Trace(stopLine);
	api->Trace("Mission creation profile info end");
	api->Trace(stopLine);
	api->Trace("\n\n");	
	creationTable.DelAll();
#endif
}


//Найти группу по имени
Mission::Group * Mission::FindGroup(GroupId group)
{
	Group * g = entryGroups[group.id & (ARRSIZE(entryGroups) - 1)];
	for(; g; g = g->next)
	{		
		if(g->id.id == group.id) return g;
	}
	return null;
}

//Добавить миссионный объект в таблицу быстрого поиска
__forceinline void Mission::AddMissionObjectToFFTable(MissionObject * mo)
{
	if(!mo) return;
	long hash = mo->GetObjectID().Hash();
	long index = GetIndexInFFTableFromHash(hash);
	fastFindTable[index].mo.Add(mo);
}

//Удалить миссионный объект из таблицы быстрого поиска
__forceinline void Mission::RemoveMissionObjectFromFFTable(MissionObject * mo)
{
	if(!mo) return;
	long hash = mo->GetObjectID().Hash();
	long index = GetIndexInFFTableFromHash(hash);
	fastFindTable[index].mo.Del(mo);
}

//Получить индекс в таблице поизка из хэша
__forceinline long Mission::GetIndexInFFTableFromHash(long hash)
{
	return hash & (ARRSIZE(fastFindTable) - 1);
}

//Установить объекту уникальный номер
__forceinline void Mission::SetObjectUId(MissionObject * mo)
{
	//До сюда не должен счётчик дойти и через 100 лет работы в редакторе и никогда в игре
	Assert(uIdCodeCounter < 0xfffffff0);
	sysSetObjectUID(mo, uIdCodeCounter);
	uIdCodeCounter++;
}

//============================================================================================

void Mission::sysRegistry(GroupId group, MissionObject * object, MOF_EVENT func, long level)
{
	//Получаем группу
	Group * grp = FindGroup(group);
	if(!grp)
	{
		//Регистрируем группу		
		grp = NEW Group();
		grp->id = group;
		grp->next = entryGroups[group.id & (ARRSIZE(entryGroups) - 1)];
		grp->list.SetId(grp->id);
		entryGroups[grp->id.id & (ARRSIZE(entryGroups) - 1)] = grp;
		this->group.Add(grp);
	}
	if(object)
	{
		MissionObjectsList::Func lfunc;
		lfunc.funcEvent = func;
		grp->list.Add(object, level, lfunc);
	}
}

void Mission::sysUnregistry(GroupId group, MissionObject * object)
{
	Group * grp = FindGroup(group);
	if(grp)
	{
		grp->list.Del(object);
	}
}

void Mission::sysUnregistryAll(MissionObject * object)
{
	if(player == object) player = null;
	for(dword i = 0; i < validateCacheCounter; i++)
	{
		if(validateCache[i].obj == object)
		{
			validateCache[i] = validateCache[--validateCacheCounter];
		}
	}
	for(long i = 0; i < group; i++)
	{
		group[i]->list.Del(object);
	}
}

void Mission::sysEvent(GroupId group, MissionObject * object)
{
	for(MGIterator & it = GroupIterator(group, _FL_); !it.IsDone(); it.Next())
	{
		it.ExecuteEvent(object);
	}
	it.Release();
}

void Mission::sysSetUpdate(long level, MissionObject * obj, MOF_UPDATE func)
{
	if(!func) return;
	MissionObjectsList::Func lfunc;
	lfunc.funcUpdate = func;
	executeList.Add(obj, level, lfunc);
}

void Mission::sysDelUpdate(long level, MissionObject * obj, MOF_UPDATE func)
{
	MissionObjectsList::Func lfunc;
	lfunc.funcUpdate = func;
	executeList.Del(obj, level, lfunc);
}

void Mission::sysDelUpdate(MissionObject * obj, MOF_UPDATE func)
{
	MissionObjectsList::Func lfunc;
	if(func)
	{
		lfunc.funcUpdate = func;
		executeList.Del(obj, lfunc);
	}else{
		executeList.Del(obj);
	}
}

void Mission::sysDeleteObject(MissionObject * object)
{
	executeList.Del(object);
	sysUnregistryAll(object);
	RemoveMissionObjectFromFFTable(object);
	objectsList.Del(object);
}

void Mission::sysUpdateObjectID(MissionObject * obj, const char * oldId, const char * newId)
{
	RemoveMissionObjectFromFFTable(obj);
	sysSetObjectIDBeforeCreate(obj, newId);
	AddMissionObjectToFFTable(obj);
	player = null;
}

void Mission::sysLogicDebug(MissionObject * mo, const char * format, void * data, bool isError)
{
#ifndef STOP_LOGICTRACE
	if(isEditMode)
	{
		return;
	}
 	if(enableDebug)
	{
		for(Mission ** mis = &traceFirstInList; *mis; mis = &((*mis)->traceNextInList))
		{
			if(!(*mis)->traceIsOutHeader)
			{
				(*mis)->traceIsOutHeader = true;
				LogicDebug("---------<Frame start: frame = %d, dltTime = %f (timeScale = %f), fps = %f, mission name = %s>---------", (*mis)->framesCounter, (*mis)->traceDltTime, (*mis)->traceTimeScale, (*mis)->traceFPS, (*mis)->name.c_str());
			}
		}

		FILE * f = crt_fopen(MISSION_DEBUGFILENAME, "a+t");
		if(f)
		{
#ifndef NO_CONSOLE
			fseek(f, 0, SEEK_END);
			long pos = ftell(f);
#endif
			const char * space = logicDebugLevelString.GetBuffer();
			if(mo) fprintf(f, "%sObject: \"%s\",  type: %s\n", space, mo->GetObjectID().c_str(), mo->GetObjectType());
			if(isError)
			{
				fprintf(f, "!  <error> %s -> ", space);
			}else{
				fprintf(f, "%s%s", space, mo ? ">" : "");
			}
			vfprintf(f, format, (va_list)data);
			fprintf(f, "\n");
#ifndef NO_CONSOLE
			if(enableConsoleLogicDebug)
			{
				//Сохраняем имя объекта нулевого уровня
				if(mo && logicDebugLevelString.IsEmpty())
				{
					const char * id = mo->GetObjectID().c_str();
					for(long i = 0; i < ARRSIZE(debugObjectCurrent) && id[i]; i++)
					{
						debugObjectCurrent[i] = id[i];
					}
					debugObjectCurrent[ARRSIZE(debugObjectCurrent) - 1] = 0;
				}
				//Проверяем по фильтру
				for(long i = 0; i < debugObjectsCounts; i++)
				{
					if(string::IsEqual(debugObjectCurrent, debugObjects[i]))
					{
						break;
					}
				}
				bool isOut = (i < debugObjectsCounts);
				if(mo)
				{
					const char * id = mo->GetObjectID().c_str();
					for(long i = 0; i < debugObjectsCounts; i++)
					{
						if(string::IsEqual(id, debugObjects[i]))
						{
							isOut = true;
							break;
						}
					}
				}
				if(isOut)
				{				
					long len = ftell(f) - pos;
					if(len > 0)
					{
						if(len > 4095) len = 4095;
						char buf[4096];
						fseek(f, pos, SEEK_SET);
						pos = fread(buf, 1, len, f);
						if(pos > len) pos = len;
						buf[pos] = 0;
						Console().Trace(COL_DESIGNERS, buf);
					}
				}
			}
#endif
			fclose(f);
		}
	}
#endif
}

void Mission::sysLogicDebugLevel(bool increase)
{
#ifndef STOP_LOGICTRACE
	if(increase)
	{
		LogicDebug("{");
		logicDebugLevelString += LOGICTRACE_SPACE;		
	}else{
		dword size = sizeof(LOGICTRACE_SPACE) - 1;
		if(logicDebugLevelString.Len() <= size)
		{
			logicDebugLevelString.Empty();
		}else{
			logicDebugLevelString.Delete(logicDebugLevelString.Len() - size, size);
		}
		LogicDebug("}");
		if(logicDebugLevelString.IsEmpty())
		{
			debugObjectCurrent[0] = 0;
		}
	}
#endif
}

IMissionQTObject * Mission::sysQTCreateObject(GroupId group, MissionObject * mo, const char* cppFileName, long cppFileLine)
{
	Group * grp = FindGroup(group);
	if(!grp)
	{
		sysRegistry(group, null, null, null);
		sysUnregistry(group, null);
		grp = FindGroup(group);
		if(!grp) return false;
	}
	return grp->findManager.CreateObject(mo, cppFileName, cppFileLine);
}

dword Mission::sysQTFindObjects(GroupId group, Vector minVrt, Vector maxVrt)
{
	objectsFindList.Empty();
	Group * grp = FindGroup(group);
	if(grp)
	{
		grp->findManager.FindObjects(minVrt, maxVrt, objectsFindList);
	}
	return objectsFindList.Size();
}

IMissionQTObject * Mission::sysQTGetObject(dword index)
{	
	return objectsFindList[index];
}

void Mission::sysQTDraw(GroupId group, float levelScale)
{
	Group * grp = FindGroup(group);
	if(grp)
	{
		grp->findManager.Draw(services.render, levelScale);
	}
}

void Mission::sysQTDump(GroupId group)
{
	Group * grp = FindGroup(group);
	char name[5];
	name[0] = char(group.id >> 0);
	name[1] = char(group.id >> 8);
	name[2] = char(group.id >> 16);
	name[3] = char(group.id >> 24);
	name[4] = 0;
	if(grp)
	{
		api->Trace("************************************************\nDump mission quad tree from group \"%s\"\n\n\n", name);
		grp->findManager.Dump();
		api->Trace("************************************************\n", name);
	}else{
		api->Trace("Can't dump mission quad tree from group \"%s\" - group not found", name);
	}
}

bool Mission::sysFindObject(const ConstString * id, MissionObject * & object, dword & uid)
{
	if(id)
	{
		object = FindObject(*id);
		if(!object)
		{		
			uid = 0;			
			return false;
		}
	}
	Assert(object);
	uid = object->GetObjectUID();
	return true;
}

bool Mission::sysValidatePointer(MissionObject * object, dword uid, long & hashIndex)
{
	return objectsList.InList(object, uid, hashIndex);
}

//============================================================================================
//Console functions
//============================================================================================

#ifndef NO_CONSOLE

void _cdecl Mission::Console_ActivateObject(const ConsoleStack & params)
{
	MissionObject * mo = Console_GetMissionObject(params);
	if(!mo) return;
	mo->Activate(true);
	if(mo->IsActive())
	{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" is active.", params.GetParam(0));
	}else{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" is not active.", params.GetParam(0));
	}
}

void _cdecl Mission::Console_DeactivateObject(const ConsoleStack & params)
{
	MissionObject * mo = Console_GetMissionObject(params);
	if(!mo) return;
	mo->Activate(false);
	if(mo->IsActive())
	{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" active.", params.GetParam(0));
	}else{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" is not active.", params.GetParam(0));
	}
}

void _cdecl Mission::Console_ShowObject(const ConsoleStack & params)
{
	MissionObject * mo = Console_GetMissionObject(params);
	if(!mo) return;
	mo->Show(true);
	if(mo->IsShow())
	{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" is show.", params.GetParam(0));
	}else{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" can't be show.", params.GetParam(0));
	}
}

void _cdecl Mission::Console_HideObject(const ConsoleStack & params)
{
	MissionObject * mo = Console_GetMissionObject(params);
	if(!mo) return;
	mo->Show(false);
	if(mo->IsShow())
	{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" can't be hide.", params.GetParam(0));
	}else{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" is hide.", params.GetParam(0));
	}
}

void _cdecl Mission::Console_CommandToObject(const ConsoleStack & params)
{
	MissionObject * mo = Console_GetMissionObject(params);
	if(!mo) return;
	if(params.GetSize() < 2)
	{
		console->Trace(COL_CMD_OUTPUT, "Not define command name for mission object.");
		return;
	}
	const char * cmdParams[64];
	long paramsCount = params.GetSize() - 2;
	if(paramsCount > ARRSIZE(cmdParams)) paramsCount = ARRSIZE(cmdParams);
	if(paramsCount < 0) paramsCount = 0;
	for(long i = 0; i < paramsCount; i++)
	{
		cmdParams[i] = params.GetParam(i + 2);
	}
	mo->Command(params.GetParam(1), paramsCount, cmdParams);
	console->Trace(COL_CMD_OUTPUT, "Execute command for mission object \"%s\".", params.GetParam(0));
}

void _cdecl Mission::Console_ViewStateObject(const ConsoleStack & params)
{
	MissionObject * mo = Console_GetMissionObject(params);
	if(!mo) return;
	console->Trace(COL_CMD_OUTPUT, "Mission object: \"%s\"", params.GetParam(0));
	console->Trace(COL_CMD_OUTPUT, "    Visible state: \"%s\"", mo->IsShow() ? "show" : "hide");
	console->Trace(COL_CMD_OUTPUT, "    Active state: \"%s\"", mo->IsActive() ? "active" : "not active");
	MO_IS_IF(id_DamageReceiverObject, "DamageReceiver", mo)
	{
		DamageReceiver * ai = (DamageReceiver *)mo;
		console->Trace(COL_CMD_OUTPUT, "    HP: \"%f\"", ai->GetHP());
		console->Trace(COL_CMD_OUTPUT, "    Max HP: \"%f\"", ai->GetMaxHP());
		console->Trace(COL_CMD_OUTPUT, "    It is \"%s\"", ai->IsDead() ? "dead" : (ai->IsDie() ? "die" : "alive"));
	}
	console->Trace(COL_CMD_OUTPUT, "     state: \"%s\"", mo->IsActive() ? "active" : "not active");
	Matrix mtx; mtx.SetZero();
	mo->GetMatrix(mtx);
	for(long i = 0; i < 16; i++)
	{
		if(fabsf(mtx.matrix[i]) > 1e-30f) break;
	}
	if(i < 16)
	{
		console->Trace(COL_CMD_OUTPUT, "    Position: (%f, %f, %f)", mtx.pos.x, mtx.pos.y, mtx.pos.z);
	}
}

void _cdecl Mission::Console_OnLogicDebug(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	enableConsoleLogicDebug = true;
	console->Trace(COL_CMD_OUTPUT, "Start trace logic debug messages");
}

void _cdecl Mission::Console_OffLogicDebug(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	enableConsoleLogicDebug = false;
	console->Trace(COL_CMD_OUTPUT, "Stop trace logic debug messages");
}

void _cdecl Mission::Console_TraceLogicDebug(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	const char * param = params.GetParam(0);
	if(!param) param = "";
	controlMission->LogicDebug(param);
}

void _cdecl Mission::Console_AddDebugObject(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	if(params.GetSize() == 0)
	{
		console->Trace(COL_CMD_OUTPUT, "Invalidate mission object name.");
		return;
	}
	if(strlen(params.GetParam(0)) > 127)
	{
		console->Trace(COL_CMD_OUTPUT, "Mission object name is very long.");
		return;
	}
	for(long i = 0; i < debugObjectsCounts; i++)
	{
		if(string::IsEqual(debugObjects[i], params.GetParam(0)))
		{
			console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" already in debug list.", params.GetParam(0));
			return;
		}
	}
	if(debugObjectsCounts >= 64)
	{
		console->Trace(COL_CMD_OUTPUT, "Can't addition object to list - debug list is full.");
		return;
	}
	crt_strcpy(debugObjects[debugObjectsCounts++], 127, params.GetParam(0));
	console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" addition to debug list.", params.GetParam(0));
}

void _cdecl Mission::Console_DelDebugObject(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	if(params.GetSize() == 0)
	{
		console->Trace(COL_CMD_OUTPUT, "Invalidate mission object name.");
		return;
	}
	for(long i = 0; i < debugObjectsCounts; i++)
	{
		if(string::IsEqual(debugObjects[i], params.GetParam(0)))
		{
			for(debugObjectsCounts--; i < debugObjectsCounts; i++)
			{
				console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" remove from debug list.", params.GetParam(0));
				crt_strcpy(debugObjects[i], 127, debugObjects[i + 1]);
			}
			return;
		}
	}
	console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" not found in debug list.", params.GetParam(0));
}

void _cdecl Mission::Console_ViewDebugObjects(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	console->Trace(COL_CMD_OUTPUT, "Debug list:");
	for(long i = 0; i < debugObjectsCounts; i++)
	{
		console->Trace(COL_CMD_OUTPUT, "    [%i] \"%s\"", i, debugObjects[i]);
	}
	console->Trace(COL_CMD_OUTPUT, "Total objects in debug list: %i", debugObjectsCounts);
}

void _cdecl Mission::Console_ClearDebugObjects(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	debugObjectsCounts = 0;
}

void _cdecl Mission::Console_ShowSoundsDebug(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	ISoundService * srv = (ISoundService *)api->GetService("SoundService");
	ISoundService::DebugLevel level = ISoundService::dl_maximum; 
	ISoundService::DebugView view = srv->GetDebugView() != ISoundService::dv_none ?  ISoundService::dv_none : ISoundService::dv_all;
	for(dword i = 0; i < params.GetSize(); i++)
	{
		const char * p = params.GetParam(i);
		if(string::IsEqual(p, "viewmin"))
		{
			level = ISoundService::dl_minimal;
		}else
		if(string::IsEqual(p, "viewmax"))
		{
			level = ISoundService::dl_maximum;
		}else
		if(string::IsEqual(p, "none"))
		{
			view = ISoundService::dv_none;
		}else
		if(string::IsEqual(p, "2d"))
		{
			view = ISoundService::dv_2d;
		}else
		if(string::IsEqual(p, "3d"))
		{
			view = ISoundService::dv_3d;
		}else
		if(string::IsEqual(p, "lis"))
		{
			view = ISoundService::dv_listener;
		}else
		if(string::IsEqual(p, "lis3d"))
		{
			view = ISoundService::dv_listener3d;
		}else
		if(string::IsEqual(p, "all"))
		{
			view = ISoundService::dv_all;
		}else{
			console->Trace(COL_CMD_OUTPUT, "Unknown parameter %i: \"%s\"", p);
		}
	}
	srv->SetDebugLevel(level);
	const char * slevel = "unknown";
	switch(srv->GetDebugLevel())
	{
	case ISoundService::dl_minimal:
		slevel = "1 (minimal)";
		break;
	case ISoundService::dl_maximum:
		slevel = "2 (maximum)";
		break;
	};
	srv->SetDebugView(view);
	const char * sview = "unknown";
	switch(srv->GetDebugView())
	{
	case ISoundService::dv_none:
		sview = "none";
		break;
	case ISoundService::dv_2d:
		sview = "2d";
		break;
	case ISoundService::dv_3d:
		sview = "3d";
		break;
	case ISoundService::dv_listener:
		sview = "lis";
		break;
	case ISoundService::dv_listener3d:
		sview = "lis3d";
		break;
	case ISoundService::dv_all:
		sview = "all";
		break;
	}
	console->Trace(COL_CMD_OUTPUT, "Sounds: debug level is %s, show is %s sounds", slevel, sview);
}


void _cdecl Mission::Console_Storage(const ConsoleStack & params)
{
	if(!console)
	{
		console = (IConsole *)api->GetService("Console");
		if(!console) return;
	}
	if(params.GetSize() == 0)
	{
		console->Trace(COL_CMD_OUTPUT, "View storage fields:\nstorage view [field_name1 field_name2 ... field_nameX]\n");
		console->Trace(COL_CMD_OUTPUT, "Set storage field:\nstorage set type(string,long,float|number) field_name value\n");
		console->Trace(COL_CMD_OUTPUT, "Delete storage fields:\nstorage delete [field_name1 field_name2 ... field_nameX]\n");
		console->Trace(COL_CMD_OUTPUT, "Copy storage field:\nstorage copy field_name_from field_name_to\n");
		return;
	}
	if(string::IsEqual(params.GetParam(0), "view"))
	{
		Console_StorageView(params);
	}else
	if(string::IsEqual(params.GetParam(0), "set"))
	{
		Console_StorageSet(params);
	}else
	if(string::IsEqual(params.GetParam(0), "delete") || string::IsEqual(params.GetParam(0), "del"))
	{
		Console_StorageDel(params);
	}else
	if(string::IsEqual(params.GetParam(0), "copy"))
	{
		Console_StorageCopy(params);
	}else{
		console->Trace(COL_CMD_OUTPUT, "Unknown storage command: %s", params.GetParam(0));
	}
}

void _cdecl Mission::Console_StorageView(const ConsoleStack & params)
{
	if(!console)
	{
		console = (IConsole *)api->GetService("Console");
		if(!console) return;
	}
	console->Trace(COL_CMD_OUTPUT, "Storage log:");
	string buffer;
	if(params.GetSize() > 1)
	{
		for(dword i = 1; i < params.GetSize(); i++)
		{
			const char * name = params.GetParam(i);
			if(!string::IsEmpty(name))
			{
				buffer += "for ";
				buffer += name;
				buffer += ":\n";
				api->Storage().Print(buffer, name);
				buffer += "\n";
			}
		}
	}else{		
		api->Storage().Print(buffer);
	}
	buffer += "\n";
	console->Trace(COL_CMD_OUTPUT, buffer.c_str());
}

void _cdecl Mission::Console_StorageSet(const ConsoleStack & params)
{
	if(!console)
	{
		console = (IConsole *)api->GetService("Console");
		if(!console) return;
	}
	if(params.GetSize() < 4)
	{
		console->Trace(COL_CMD_OUTPUT, "Not enougth parameters...");
		return;
	}
	const char * type = params.GetParam(1);
	const char * name = params.GetParam(2);
	const char * value = params.GetParam(3);
	if(!value) value = "";
	if(string::IsEqual(type, "string"))
	{
		if(api->Storage().SetString(name, value))
		{
			console->Trace(COL_CMD_OUTPUT, "Storage: string %s = \"%s\"", name, value);
		}else{
			console->Trace(COL_CMD_OUTPUT, "Storage error: cant set %s as string", name);
		}
	}else
	if(string::IsEqual(type, "long"))
	{
		long v = atol(value);
		if(api->Storage().SetLong(name, v))
		{
			console->Trace(COL_CMD_OUTPUT, "Storage: long %s = %i", name, v);
		}else{
			console->Trace(COL_CMD_OUTPUT, "Storage error: cant set %s as long", name);
		}
	}else	
	if(string::IsEqual(type, "float") || string::IsEqual(type, "number"))
	{
		float v = (float)atof(value);
		if(api->Storage().SetFloat(name, v))
		{
			console->Trace(COL_CMD_OUTPUT, "Storage: float %s = %f", name, v);
		}else{
			console->Trace(COL_CMD_OUTPUT, "Storage error: cant set %s as float", name);
		}
	}else{
		console->Trace(COL_CMD_OUTPUT, "Storage error: unknown storage type %s", type);
	}
}

void _cdecl Mission::Console_StorageDel(const ConsoleStack & params)
{
	if(!console)
	{
		console = (IConsole *)api->GetService("Console");
		if(!console) return;
	}
	if(params.GetSize() <= 1)
	{
		console->Trace(COL_CMD_OUTPUT, "Not enougth parameters...");
		return;
	}
	for(dword i = 1; i < params.GetSize(); i++)
	{
		const char * name = params.GetParam(i);
		if(!string::IsEmpty(name))
		{
			api->Storage().Delete(name);
			console->Trace(COL_CMD_OUTPUT, "Storage field \"%s\" is delete", name);
		}
	}
}

void _cdecl Mission::Console_StorageCopy(const ConsoleStack & params)
{
	if(!console)
	{
		console = (IConsole *)api->GetService("Console");
		if(!console) return;
	}
	if(params.GetSize() < 2)
	{
		console->Trace(COL_CMD_OUTPUT, "Not enougth parameters...");
		return;
	}	
	const char * from = params.GetParam(1);
	const char * to = params.GetParam(2);
	array<byte> buffer(_FL_, 1024);
	api->Storage().Save(from, buffer);
	dword readPointer = 0;
	if(!api->Storage().Load(to, buffer.GetBuffer(), buffer.Size(), readPointer))
	{
		console->Trace(COL_CMD_OUTPUT, "Storage error: can't copy. Difference types? Try before copy delete destination folder.");
		return;
	}	
	console->Trace(COL_CMD_OUTPUT, "Copy from fields %s to %s", from, to);
}

void _cdecl Mission::Console_FreeCamera(const ConsoleStack & params)
{
	if(Console_NoExecute()) return;
	static const ConstString cameraName("Free camera");
	MissionObject * mo = controlMission->FindObject(cameraName);
	if(!mo)
	{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" not found.", cameraName.c_str());
		return;
	}
	mo->Activate(!mo->IsActive());	
	console->Trace(COL_CMD_OUTPUT, "%s was %s.", cameraName.c_str(), mo->IsActive() ? "activated" : "deactivated");
}

void _cdecl Mission::Console_TraceUnuse(const ConsoleStack & params)
{
	ILocStrings * ls = (ILocStrings *)api->GetService("LocStrings");
	if(ls)
	{
		ls->TraceUnuse();
		IConsole * console = (IConsole *)api->GetService("Console");
		if(console)
		{
			console->Trace(COL_CMD_OUTPUT, "Look system_log.txt for results");
		}
	}
}

bool Mission::Console_NoExecute()
{
	if(!controlMission)
	{
		console = null;
		return true;
	}
	console = &controlMission->Console();
	Assert(console);
	return false;
}

MissionObject * Mission::Console_GetMissionObject(const ConsoleStack & params)
{
	if(Console_NoExecute()) return null;
	const char * name = params.GetParam(0);
	if(!name) name = "";
	MissionObject * mo = controlMission->FindObject(ConstString(name));
	if(!mo)
	{
		console->Trace(COL_CMD_OUTPUT, "Mission object \"%s\" not found.", name);
	}
	return mo;
}


#endif

