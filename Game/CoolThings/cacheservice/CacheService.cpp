/*


[cache]
параметр = путь. для описания путей поддерживаются маски.
...

постфиксы к описанию параметра:
*_rec - путь до файла(ов) перебираемых рекурсивно начиная с текущей папки
*_pack - перед загрузкой данных файлов будет временно загружен данный пак-файл (указание файла без маски)

параметры в порядке обработки:
pack - пак файлы
texture - текстуры
animation - анимация
geometry - модельки


пример:

[cache]
pack = resource\packs\*.add_paks
pack = resource\models\ragdolls\ragdolls.pkx
...
texture_pack = fonts\fonts.pkx	временная загрузка при кэшировании файлов
texture = fonts\*.*				поиск в текущей папке
texture_rec = fonts\*.*			рекурсивный поиск начиная с текущей папки
...
animation_pack = global_ani.pkx
animation = blood*.anx
...
geometry_pack = global_gmx.pkx
geometry =	weapons\*.gmx
...

*/




#include "CacheService.h"


CREATE_SERVICE(CacheService, 1)

const char * CacheService::section = "cache";

CacheService::Environment::Environment() : elements(_FL_),
											packs(_FL_)
{
	fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	gs = null;
	rs = null;
	as = null;
	aniScene = null;
	loadHint = "Precaching global data...";
	loadProgress = 0.0f;
	progressRange = 0.0f;
	waitCount = 0;
}

CacheService::Environment::~Environment()
{
}

void CacheService::Environment::Init()
{
	Assert(fs);
	gs = (IGMXService *)api->GetService("GMXService");
	Assert(gs);
	rs = (IRender *)api->GetService("DX9Render");
	Assert(rs);
	as = (IAnimationService *)api->GetService("AnimationService");
	Assert(as);
	aniScene = as->CreateScene(_FL_);
	Assert(aniScene);
}


CacheService::CacheService() : packs(_FL_, 256),
						       textures(_FL_, 256),
							   animations(_FL_, 256),
							   models(_FL_, 256)
{
	aniScene = null;
	env = null;
}

CacheService::~CacheService()
{
	for(long i = 0; i < packs; i++)
	{
		packs[i]->Release();
	}
	packs.Empty();
}

//Инициализация
bool CacheService::Init()
{
	//Среда с которой работаем на момент создания
	env = NEW Environment;
	api->Storage().SetLong("system.WaitPrecache", 1);
	api->SetEndFrameLevel(this, 100);	
	//Загружаем временные пак-файлы глобальных ресурсов (20%)
	Assert(packs.Size() == 0);
	//Паки
	PrecacheTmpPacks("pack", ".\\");
	//Текстурные файлы
	PrecacheTmpPacks("texture", "resource\\textures\\");
	//Анимация
	PrecacheTmpPacks("animation", "resource\\animation\\");
	//Геометрия
	PrecacheTmpPacks("geometry", "resource\\models\\");	
	return true;
}

//Вызываеться перед удалением сервисов
void CacheService::PreRelease()
{
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
	if(aniScene)
	{
		aniScene->Release();
		aniScene = null;
	}
}

//Исполнение в конце кадра
void CacheService::EndFrame(float dltTime)
{
	if(!env)
	{
		return;
	}
	if(env->waitCount < 3)
	{
		env->waitCount++;
		return;
	}	
	//Время кэширования
	dword time = GetTickCount();
	//Ини файл
	IIniFile * ini = env->fs->SystemIni();
	if(ini && ini->IsSectionCreated(section))
	{
		env->Init();
		ShowLoadingScreen(0.0f);
		//Паки
		env->progressRange = 10.0f;
		Precache("pack", ".\\", &CacheService::Loader_Pkx);
		//Текстурные файлы
		env->progressRange = 40.0f;
		Precache("texture", "resource\\textures\\", &CacheService::Loader_Txx);
		//Анимация
		env->progressRange = 15.0f;
		Precache("animation", "resource\\animation\\", &CacheService::Loader_Ani);
		//Геометрия
		env->progressRange = 25.0f;
		Precache("geometry", "resource\\models\\", &CacheService::Loader_Gmx);
		env->loadHint = "";
		ShowLoadingScreen(100.0f);
		//Отгружаем временные пак-файлы
		for(long i = 0; i < env->packs; i++)
		{
			if(env->packs[i]) env->packs[i]->Release();
		}
		env->packs.Empty();
		
		//Закончили
		time = GetTickCount() - time;
		api->Trace("Global precache time = %.2f seconds", time*0.001f);
	}
	delete env;
	api->Storage().SetLong("system.WaitPrecache", 0);
	env = null;
}

//Прекэширование временных пак-файлов
void CacheService::PrecacheTmpPacks(const char * records, const char * files_path)
{
	IIniFile * ini = env->fs->SystemIni();
	if(!ini) return;
	env->buffer = records;
	env->buffer += "_pack";
	ini->GetStrings(section, env->buffer, env->elements);
	for(long i = 0; i < env->elements; i++)
	{
		IPackFile * pf = env->fs->LoadPack(env->elements[i], _FL_);
		if(pf)
		{
			env->packs.Add(pf);
		}		
	}
}

//Прекэширование определённого типа данных
void CacheService::Precache(const char * records, const char * files_path, Loader loader)
{
	IIniFile * ini = env->fs->SystemIni();
	if(!ini) return;
	//Перебераем нерекурсивные параметры
	ini->GetStrings(section, records, env->elements);
	env->fs->BuildPath(files_path, env->root);
	Process(loader, false, env->progressRange*0.5f);
	//Перебераем рекурсивные параметры
	env->buffer = records;
	env->buffer += "_rec";
	ini->GetStrings(section, env->buffer, env->elements);
	env->fs->BuildPath(files_path, env->root);
	Process(loader, true, env->progressRange*0.5f);
}

//Прокэшировать список элементов
void CacheService::Process(Loader loader, bool isRecursive, float step)
{
	if(env->elements > 0)
	{
		step /= (float)env->elements.Size();
	}else{
		ShowLoadingScreen(step);
	}
	for(long i = 0; i < env->elements; i++)
	{
		//Путь до места файлов
		env->file.GetFilePath(env->elements[i]);
		env->path = env->root;
		env->path += env->file;
		//Маска для поиска
		env->file.GetFileName(env->elements[i]);
		//Загружаем найденые файлы
		dword flags = find_all_files;
		if(!isRecursive)
		{
			flags |= find_no_recursive;
		}
		IFinder * finder = env->fs->CreateFinder(env->path, env->file, flags, _FL_);
		for(dword j = 0; j < finder->Count(); j++)
		{
			env->buffer = finder->FilePath(j);
			env->buffer.GetRelativePath(env->root);
			(this->*loader)(env->buffer.c_str());
		}
		finder->Release();
		ShowLoadingScreen(step);
	}

}

//Вывести текущий прогресс прекэширования
void CacheService::ShowLoadingScreen(float delta)
{
	if(env->rs)
	{
		env->loadProgress += delta*0.5f;
		if(env->loadProgress > 50.0f)
		{
			env->loadProgress = 50.0f;
		}
		env->rs->SetLoadingScreenPercents(env->loadProgress, 50.0f, env->loadHint);
	}
}


//Загрузка пака
void CacheService::Loader_Pkx(const char * name)
{
	IPackFile * pack = env->fs->LoadPack(name, _FL_);
	if(pack)
	{
		packs.Add(pack);
	}
}

//Загрузка текстуры
void CacheService::Loader_Txx(const char * name)
{
	IBaseTexture * tex = env->rs->CreateTexture(_FL_, name);
	if(tex)
	{
		textures.Add(tex);
	}
}

//Загрузка анимации
void CacheService::Loader_Ani(const char * name)
{
	IAnimation * ani = env->aniScene->Create(name, _FL_);
	if(ani)
	{
		animations.Add(ani);
	}
}

//Загрузка геометрии
void CacheService::Loader_Gmx(const char * name)
{
	IGMXScene * mdl = env->gs->CreateScene(name, aniScene, null, null, _FL_);
	if(mdl)
	{
		models.Add(mdl);
	}
}

