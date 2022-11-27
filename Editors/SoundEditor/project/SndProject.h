//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// SndProject
//============================================================================================

#ifndef _SndProject_h_
#define _SndProject_h_


#include "ProjectObject.h"
#include "ProjectWave.h"
#include "ProjectSound.h"
#include "ProjectAnimation.h"


//Звуковой банк
class ProjectSoundBank : public ProjectObject
{
	//friend class SndProject;
public:
	enum GlobalConsts
	{
		c_namelen = 32,
	};
public:
	ProjectSoundBank(UniqId & folderId);

public:
	//Получить путь по которому экспортировать банк
	const char * GetExportPath();
	//Получить папку звукового банка
	const UniqId & GetSBFolderId();
	//Сбровить поля в начальное значение
	void Reset(bool rebuildId);

private:
	//Сохранить объект
	virtual ErrorId OnSaveObject(const char * defaultFilePath);
	//Первоначальная загрузка при старте редактора
	virtual ErrorId OnLoadObject(const char * defaultFilePath);

public:
	string exportPath;				//Относительный путь с именем файла, куда экспортировать банк
	static const char * section;	//Секция в которой сохраняються параметры банка
};

//Глобальные параметры звука
class ProjectSoundBaseParams : public ProjectObject
{
public:
	enum GlobalConsts
	{
		c_namelen = 48,
	};
public:
	ProjectSoundBaseParams(UniqId & folderId);
	//Сохранить объект
	virtual ErrorId OnSaveObject(const char * defaultFilePath);
	//Первоначальная загрузка при старте редактора
	virtual ErrorId OnLoadObject(const char * defaultFilePath);

public:
	SoundBaseParams params;	//Параметры
};

//Глобальные графики
class ProjectSoundAttGraph : public ProjectObject
{
public:
	enum GlobalConsts
	{
		c_namelen = 48,
	};
public:
	ProjectSoundAttGraph(UniqId & folderId);
	//Сохранить объект
	virtual ErrorId OnSaveObject(const char * defaultFilePath);
	//Первоначальная загрузка при старте редактора
	virtual ErrorId OnLoadObject(const char * defaultFilePath);

public:
	SoundAttGraph attGraph;	//График
};


class SndProject
{
public:
	enum Consts
	{
		c_waitTimeForCloseFileInMs = 5000,		//Время удержания файла открытым
		c_MaxOpenFiles = 10,					//Максимальное количество одновременно открытых файлов

		c_updateTimeInMs = 500,					//Квант времени для обновления

	};

	/*
	enum CheckForImport
	{
		CheckForImport_Ok = 0,			//Уникальное имя волны
		CheckForImport_Wave = 1,		//Имя пересекаеться с другой волной
		CheckForImport_Folder = 2,		//Имя пересекаеться с папкой
	};*/

	enum ObjectTypes
	{
		pot_any_object = 0,				//Любой объект
		pot_folder = 'pfld',			//Папка
		pot_wave = 'pwav',				//Волна
		pot_sound = 'psnd',				//Звук
		pot_baseparams = 'pprm',		//Глобальные параметры звука
		pot_attenuation = 'patt',		//Глобальные графики
		pot_animation = 'panm',			//Анимации
		pot_soundbank = 'psbk',			//Звуковой банк
	};

	enum PrjOpenFileFlags
	{
		poff_noflags = 0,
		poff_isCreateNewFile = 0x10000,	//Надо создать новый файл
		poff_isWaitWhenOpen = 0x20000,	//Дождаться, пока файл освободит другой пользователь		
		poff_isOpenForRead = 0x40000,	//Надо создать новый файл
	};


public:

	//Класс, представляющий агентов отложенного сохранения
	class ProjectUpdater
	{
	public:
		virtual ~ProjectUpdater();
		virtual void Execute() = null;
	};

public:
	//Подписчик на удаление из папки объекта
	typedef ErrorId (SndProject::* FolderObjectDelete)(const UniqId & id);

private:

	enum PrjOpenFileFlagsEx
	{
		ffex_isOpen = 0x1000000,
	};

	//Папка, представляющая папку в проекте реально размещённую на диске
	struct PrjFolder
	{
		PrjFolder();
		UniqId id;					//Идентификатор папки
		ExtNameStr name;			//Имя папки
		PrjFolder * parent;			//Указатель на родительскую папку
		array<PrjFolder *> child;	//Указатели на дочернии папки
		array<UniqId> objects;		//Указатели на объекты, находящиеся в данной папке
		string fullPath;			//Полный путь на диске этой папки оканчивающийся слэшом
		FolderObjectDelete del;		//Функция удаляющая объект из папки (при удалении папки)
		bool noEditable;			//Запрещено изменять имя папки и удалять её
	};
	

	//Запись об открытом файле
	struct PrjOpenFile
	{
		IFileBase * file;
		dword flags;
		float waitTime;
		const char * cppFile;
		long cppLine;
		long debugId;
		string path;
	};

	//Регистрационная запись для поиска объектов
	struct RegRecord
	{
		dword hash;
		dword type;
		UniqId id;
		void * objectPtr;
		long next;
	};

public:
	SndProject();
	~SndProject();


//--------------------------------------------------------------------------------------
//Глобальные объекты поиска
//--------------------------------------------------------------------------------------
public:
	//Зарегистрировать объект
	bool ObjectRegister(const UniqId & id, ObjectTypes type, void * ptr);
	//Удалить объект
	bool ObjectRemove(const UniqId & id, ObjectTypes type);
	//Найти объект по идентификатору
	void * ObjectFind(const UniqId & id, ObjectTypes type);


//--------------------------------------------------------------------------------------
//Работа с папками
//--------------------------------------------------------------------------------------
public:
	//Добавить папку
	bool FolderAdd(const UniqId & parentFolderId, const char * name, UniqId * id = null);
	//Удалить папку
	ErrorId FolderDelete(const UniqId & folderId);
	//Переименовать папку
	ErrorId FolderRename(const UniqId & folderId, const char * name);
	//Получить родительскую папку
	bool FolderGetParent(const UniqId & folderId, UniqId & parentFolderId);
	//Получить дочернии папки
	const array<UniqId> & FolderGetChild(const UniqId & folderId);
	//Получить название папки
	const ExtName * FolderGetName(const UniqId & folderId);
	//Являеться ли папка дочерней для заданной (или текущей)
	bool FolderIsChild(const UniqId & parentFolderId, const UniqId & folderId);
	//Получить путь папки
	const char * FolderGetPath(const UniqId & folderId, const UniqId * stopFolderId = null);
	//Проверить на уникальность имени среди детей данной папки
	bool FolderCheckUniqeName(const UniqId & folderId, const char * name, const UniqId * skipFolderId = null);

	//Добавить объект в папку
	bool FolderObjectAdd(const UniqId & folderId, const UniqId & id);
	//Удалить объект из папку
	bool FolderObjectDel(const UniqId & folderId, const UniqId & id);
	//Список объектов, которые лежат в папке
	const array<UniqId> & FolderObjects(const UniqId & folderId);

	//Папка с волнами
	const UniqId & FolderWaves();
	//Папка со звуками
	const UniqId & FolderSounds();
	//Папка с роликами
	const UniqId & FolderMovies();
	//Звуковые установки
	const UniqId & FolderSoundParams();
	//Папка с графиками затухания
	const UniqId & FolderAttenuations();
	//Получить идентификатор виртуальной папки - подняться вверх по иерархии
	const UniqId & FolderUp();

//--------------------------------------------------------------------------------------
//Работа с волнами
//--------------------------------------------------------------------------------------
public:
	//Получить волну
	ProjectWave * WaveGet(const UniqId & id);
	//Получить список волн (пользоваться им можно только сразу после получения)
	const array<ProjectWave *> & WaveArray();
	//Импортировать волну в проект (без проверки совпадения имён)
	ErrorId WaveImport(const UniqId & folderId, const char * path, const char * name, const UniqId & replaceId, UniqId * id = null);
	//Удалить волну
	ErrorId WaveDelete(const UniqId & waveId);
	//Переименовать волну
	bool WaveRename(const UniqId & waveId, const char * name);
	//Проверить имя на уникальность
	bool WaveCheckUniqeName(const UniqId & folderId, const char * name, const UniqId * skipWaveId = null);

//--------------------------------------------------------------------------------------
//Работа со звуком
//--------------------------------------------------------------------------------------
public:
	//Получить звук
	ProjectSound * SoundGet(const UniqId & id);
	//Найти звук по имени
	ProjectSound * SoundFind(const char * name);
	//Получить список звуков (пользоваться им можно только сразу после получения)
	const array<ProjectSound *> & SoundArray();
	//Добавить звук
	bool SoundAdd(const UniqId & folderId, const char * name, UniqId * id = null);
	//Удалить файл
	ErrorId SoundDelete(const UniqId & soundId);
	//Переименовать звук
	bool SoundRename(const UniqId & soundId, const char * name);
	//Проверить имя на уникальность
	bool SoundCheckUniqeName(const UniqId & folderId, const char * name, const UniqId * skipSoundId = null);

	
	//Получить доступ к базовым параметрам звука
	ProjectSoundBaseParams * SndBaseParamsGet(const UniqId & id);
	//Получить массив глобальных параметров (пользоваться им можно только сразу после получения)
	const array<ProjectSoundBaseParams *> & SndBaseParamsArray();
	//Добавить глобальные параметры звука
	bool SndBaseParamsAdd(const char * name, UniqId * id = null);
	//Удалить глобальные параметры звука
	bool SndBaseParamsDelete(const UniqId & id);
	//Переименовать глобальные параметры звука
	bool SndBaseParamsRename(const UniqId & id, const char * name);
	//Проверить имя на уникальность
	bool SndBaseParamsCheckUniqeName(const char * name, const UniqId * skipParamsId = null);


	//Получить доступ к графику затухания 
	ProjectSoundAttGraph * AttGraphGet(const UniqId & id);
	//Получить массив глобальных графиков
	const array<ProjectSoundAttGraph *> & AttGraphArray();
	//Добавить график затухания
	bool AttGraphAdd(const char * name, UniqId * id = null);
	//Удалить график затухания
	bool AttGraphDelete(const UniqId & id);
	//Переименовать график затухания
	bool AttGraphRename(const UniqId & id, const char * name);
	//Проверить имя на уникальность
	bool AttGraphCheckUniqeName(const char * name, const UniqId * skipAttId = null);

//--------------------------------------------------------------------------------------
//Работа с банками
//--------------------------------------------------------------------------------------
public:
	
	//Получить звуковой банк
	ProjectSoundBank * SoundBankGet(const UniqId & id);
	//Получить звуковой банк для папки
	ProjectSoundBank * SoundBankGetByFolder(const UniqId & folderId);
	//Получить список звуковых банков (пользоваться им можно только сразу после получения)
	const array<ProjectSoundBank *> & SoundBankArray();	
	//Добавить звуковой банк
	ErrorId SoundBankAdd(const char * name, UniqId * id = null);
	//Удалить звуковой банк
	bool SoundBankDelete(const UniqId & bankId);
	//Переименовать звук
	bool SoundBankRename(const UniqId & soundId, const char * name);
	//Проверить имя на уникальность
	bool SoundBankCheckUniqeName(const char * name, const UniqId * skipSoundBankId = null);


//--------------------------------------------------------------------------------------
//Работа с анимацией
//--------------------------------------------------------------------------------------
public:
	//Получить анимацию
	ProjectAnimation * AnimationGet(const UniqId & id);
	//Получить список роликов
	const array<ProjectAnimation *> & AnimationArray();
	//Добавить анимацию в проект
	ErrorId AnimationAdd(const char * anxName, UniqId * id = null);
	//Удалить анимацию из проекта
	bool AnimationDelete(const UniqId & id);
	//Подготовить анимацию к поигрыванию звуков
	void AnimationPrepareForSounds(const UniqId & id);
	//Проверить имя на уникальность
	bool AnimationCheckUniqeName(const char * name, const UniqId * skipAniId = null);

//--------------------------------------------------------------------------------------
//Состояние проекта
//--------------------------------------------------------------------------------------
public:
	//Добавить обновление в очередь
	void AddToSave(ProjectObject * po);
	//Удалить обновляющий объект из списка
	void RemoveFromSave(ProjectObject * po);
	//Ожидание записи в файл
	bool IsWaitToSave();
	//Сохранить все изменения
	void FlushData();
	//Получить текущее время проекта
	double GetProjectTime();


//--------------------------------------------------------------------------------------
//Файлы
//--------------------------------------------------------------------------------------
public:
	//Открыть бинарный файл
	IFile * FileOpenBin(const char * path, dword flags, bool * isLock, const char * cppFile, long cppLine, long debugId);
	//Закрыть бинарный файл
	void FileCloseBin(IFile * file, long debugId);
	//Принудительно закрыть бинарный файл с данным именем
	bool FileForceReleaseBin(const char * path);

public:
	//Открыть ini файл на полный доступ
	IEditableIniFile * FileOpenIni(const char * path, bool isNewFile, bool isReadMode, const char * cppFile, long cppLine);
	//Закрыть ini файл
	void FileCloseIni(IEditableIniFile * ini);
	//Принудительно закрыть ini файл с данным именем
	bool FileForceReleaseIni(const char * path);

private:
	//Найти открытый файл по имени
	static long FileFindFile(array<PrjOpenFile> & files, string & fullPath);
	//Принудительно закрыть файл с данным именем
	static bool FileForceRelease(array<PrjOpenFile> & files, const char * path, string & fullPathBuffer);
	//Принудительно закрыть файл с данным индексом
	static bool FileForceRelease(array<PrjOpenFile> & files, long index);
	//Проверка на закрытие неиспольуемых файлов
	static void FilesCloseProcess(array<PrjOpenFile> & files, float dltTime);
	//Удалить все файлы игнорируя состояние
	static void FilesRelease(array<PrjOpenFile> & files);



//--------------------------------------------------------------------------------------
//Обновление
//--------------------------------------------------------------------------------------
public:
	//Обносить состояние проекта
	void Update(float dltTime);


//--------------------------------------------------------------------------------------
//Загрузка проекта
//--------------------------------------------------------------------------------------
private:
	//Загрузить проект
	void LoadProject();
	//Зачитать линейные данные
	template<class T> void LoadLinearData(const char * resourceName, ObjectTypes type, UniqId & folderId, array<T *> & arr, FolderObjectDelete delFunc);
	//Зачитать рекурсивные данные
	template<class T> void LoadRecursiveData(const char * resourceName, const char * extention, ObjectTypes type, UniqId & folderId, array<T *> & arr, bool skipParentFolderObjects, FolderObjectDelete delFunc);
	//Создать группу папок соответствующих относительному пути родителя
	void LoadMakePath(const char * path, const UniqId & parentFolderId, UniqId & folderId);
	//Зачитать параметры звуковых банков
	void LoadSoundBanksParams();
	//Проверить имя на уникальность, если folderId указывает на папку, то проверить имя и у дочерних папок
	template<class ProjectObjectBranch> bool NameCheckForUnique(array<ProjectObjectBranch *> & objects, dword maxNameLen, const UniqId & folderId, const char * name, const UniqId * skipId, const UniqId * parentFolderForCheck = null);
	

private:
	bool waitToSave;
	array<ProjectWave *> waves;
	array<ProjectSound *> sounds;
	array<ProjectSoundBaseParams *> soundBaseParams;
	array<ProjectSoundAttGraph *> attGraphs;
	array<PrjFolder *> folders;
	array<ProjectSoundBank *> soundBanks;
	array<ProjectAnimation *> animations;
	array<ProjectObject *> waitForSave;
	array<UniqId> folderChilds;
	string folderPathBuffer;
	string openIniFileBuffer;
	CritSection binFilesSyncro;
	array<PrjOpenFile> openBinFiles;
	array<PrjOpenFile> openIniFiles;
	float updateTimer;
	double projectTime;
	dword waveUpdater;
	dword waveExportPointer;
	dword soundUpdater;
	float soundUpdaterPauseTimer;
	array<RegRecord> regTable;
	long firstRegFree;
	long entryRegTable[0x4000];
	dword regChainLengMax;
	UniqId folderRoot;
	UniqId folderWaves;	
	UniqId folderSounds;	
	UniqId folderMovies;
	UniqId folderSoundParams;
	UniqId folderAttenuations;
	UniqId folderUp;
};

//Ожидание записи в файл
__forceinline bool SndProject::IsWaitToSave()
{
	return waitToSave;
}

//Папка с волнами
__forceinline const UniqId & SndProject::FolderWaves()
{
	return folderWaves;
}

//Папка со звуками
__forceinline const UniqId & SndProject::FolderSounds()
{
	return folderSounds;
}

//Папка с роликами
__forceinline const UniqId & SndProject::FolderMovies()
{
	return folderMovies;
}

//Звуковые установки
__forceinline const UniqId & SndProject::FolderSoundParams()
{
	return folderSoundParams;
}

//Папка с графиками затухания
__forceinline const UniqId & SndProject::FolderAttenuations()
{
	return folderAttenuations;
}

//Получить идентификатор виртуальной папки - подняться вверх по иерархии
__forceinline const UniqId & SndProject::FolderUp()
{
	return folderUp;
}

#endif
