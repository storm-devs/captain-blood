

#ifndef _CacheService_h_
#define _CacheService_h_


#include "..\..\..\Common_h\mission.h"


class CacheService : public Service
{
	struct Environment
	{
		Environment();
		~Environment();

		void Init();

		IFileService * fs;
		IGMXService * gs;
		IRender * rs;
		IAnimationService * as;
		IAnimationScene * aniScene;		
		string buffer;
		string root;
		string path;
		string file;		
		array<string> elements;
		array<IPackFile *> packs;
		const char * loadHint;
		float loadProgress;
		float progressRange;
		long waitCount;
	};

	typedef void (CacheService::* Loader)(const char * name);

public:
	CacheService();
	virtual ~CacheService();
	//Инициализация
	virtual bool Init();
	//Вызываеться перед удалением сервисов
	virtual void PreRelease();

private:
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime);
	//Прекэширование временных пак-файлов
	void PrecacheTmpPacks(const char * records, const char * files_path);
	//Прекэширование определённого типа данных
	void Precache(const char * records, const char * files_path, Loader loader);
	//Прокэшировать список элементов
	void Process(Loader loader, bool isRecursive, float step);
	//Вывести текущий прогресс прекэширования
	void ShowLoadingScreen(float delta);



private:
	//Загрузка пака
	void Loader_Pkx(const char * name);
	//Загрузка текстуры
	void Loader_Txx(const char * name);
	//Загрузка анимации
	void Loader_Ani(const char * name);
	//Загрузка геометрии
	void Loader_Gmx(const char * name);

private:
	IAnimationScene * aniScene;
	Environment * env;

private:	
	array<IPackFile *> packs;
	array<IBaseTexture *> textures;
	array<IAnimation *> animations;
	array<IGMXScene *> models;

	static const char * section;
};

#endif

