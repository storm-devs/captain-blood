
#ifndef _ProjectBuilder_h_
#define _ProjectBuilder_h_


#include "..\..\Game\Mission\IMissionsManagerService.h"
#include "..\..\common_h\Mission.h"

#ifndef NO_TOOLS
#ifndef _XBOX

class Mission;

class ProjectBuilder
{
	typedef void (_fastcall ProjectBuilder::*ExecuteFunc)(float dltTime);

	enum PackType
	{
		global_pc,
		mission_pc,
		global_xbox,
		mission_xbox
	};

	struct PackOptions
	{
		PackType type;
		int compressRatio;
		bool isDelete;
		string folderPath;
		string fileName;
		string mask;
	};

	struct OpFiles
	{
		string dst;
		string src;
		bool isRecursive;
		bool isPC;
		bool inBuildPath;
	};

	struct CheckFileName
	{
		dword hash;
		dword len;
		dword nameIndex;
		long next;
	};

public:
	ProjectBuilder();
	~ProjectBuilder();


	//Экспорт xml миссий
	bool StartExportXMLMissions();
	//Процесс сбора билда из подготовленного проекта
	bool StartBuildProcess();

private:
	//Добавить процессируемые файлы
	void AddOpFiles(array<OpFiles> & arr, IIniFile * ini, const char * key, bool isRecursive, bool isPC, bool inBuildPath);

private:
	//Экспорт xml миссий
	void ExportXMLMissions();
	//Процесс сбора билда из подготовленного проекта
	void BuildProcess();

	//Загрузка мисии
	void Load();
	//Ожидание работы миссии
	void WorkCounter();

private:
	//Собираем билд
	void Build();


private:
	//Проходимся по полученным миссиям и глобальным ресурсам, собираем паки
	void BuildPacks(const char * workPath, bool isPC);
	//Создать пак-файл из папки и удалить файлы из папки
	void BuildFolderPack(PackOptions & opt, const char * packPath, const char * replaceName);
	//Копировать файлы
	void CopyFiles(const string & destBasePath, const string & destPath, const string & srcBasePath, const string & srcMaskPath, bool isRecursive, bool noRelative = false);
	//Удалить файлы
	void DeleteFiles(const string & basePath, const string & maskPath, bool isRecursive);
	//Получить полный путь имея базовый путь и заданный
	void MakeFullPath(const string & basePath, const string & path, string & result);
	//Удалить пустые папки из билда
	bool DeleteEmptyFolders(const char * path);
	//Проверить файлы в ресурсах
	void CheckFilesForUnique(const char * path);
	//Сконвертировать файлы из PC формата в Xbox формат
	void ConvertXBoxFiles();
	//Сконвертировать файлы из PC формата в Xbox формат заданного типа
	void ConvertXBoxFiles(const char * mask, void (* convert)(const char * from, const char * to));



public:
	//Вывести сообщение об ошибке
	void _cdecl Error(const char * error, ...);

private:
	IFileService * fs;

private:
	IFinder * finder;
	long index;
	
private:
	string workPath_PC;
	string workPath_Xbox;
	string missionsPath;
	string resourcesPath;
	string backupPath_PC;
	string backupPath_Xbox;
	array<PackOptions> packs;
	array<OpFiles> copyFiles;
	array<OpFiles> deleteFiles;
	array<string> compareFilters;
	array<string> skipMissionsTable;
	IMission * currentMission;
	long workCountFrames;

private:
	ExecuteFunc executeFunc;
	long waitTimer;

};


class ProjectBuilderService : public Service
{
public:
	ProjectBuilderService();
	virtual ~ProjectBuilderService();

	//Исполнить следующую функцию выждив несколько кадров
	void Execute(void (ProjectBuilder:: * _exeFunction)());

private:
	//Инициализация
	virtual bool Init();
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime);

	//Вывести в лог форматированый список миссионых объектов и их параметров
	void LogMissionObjects();
	//Вывести параметр
	void OutputParameter(const char * shift, const IMOParams::Param * p);

private:
	void (ProjectBuilder:: * exeFunction)();
	long waitTimer;

public:
	static ProjectBuilderService * service;
	static ProjectBuilder * builder;
};


#endif
#endif
#endif