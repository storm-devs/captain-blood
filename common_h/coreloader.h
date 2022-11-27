//==================================================================================================
//Загрузчик и держатель dll библиотеки с ядром для Win32
//==================================================================================================

#ifndef _CoreLoaderWin32_h_
#define _CoreLoaderWin32_h_

#ifndef _XBOX
#define NOMINMAX
#include <windows.h>
#else
#include <xtl.h>
#endif


#include <stdio.h>
#include "Storm3Crt.h"
#include "coremanager.h"







//Загрузчик и держатель dll библиотеки с ядром
class CoreLoader
{
public:
	enum MTMode
	{
		mt_off,
		mt_on,
		mt_auto,
	};


	CoreLoader()
	{
		manager = 0;
		handle = 0;
		iniName = 0;
		pictureName = 0;
		quickModules = 0;
		objects = 0;
		memorycheck = false;
		panicmemorycheck = false;
		memorypools = false;
		trace = false;
		debugkeys = false;
		noidle = false;
		loResTimer = false;
		mtmode = mt_auto;
		fixedFPS = 0;
		winPosX = 10;
		winPosY = 10;
	};

	~CoreLoader()
	{
		ClearBootData();
		UnloadCore();
		if(iniName) delete iniName; iniName = 0;
	};

	//Загрузить библиотеку с ядром и инициализировать его
	bool LoadCore(CoreSystemLogReciver * reciver, const char * coreLibrary = "Modules\\systems.dll")
	{
		if(manager) return true;
		UnloadCore();
		//Загружаем библиотеку
		handle = ::LoadLibrary(coreLibrary);
		if(!handle) return false;
		//Получаем указатель на функцию инициализации
		GetCoreManager getManager = (GetCoreManager)GetProcAddress(handle, GetCoreManagerFunction);
		if(!getManager)
		{
			UnloadCore();
			return false;
		}
		//Подключаем библиотеку
		getManager(manager);
		if(!manager)
		{
			UnloadCore();
			return false;
		}
		//Создаём ядро
		manager->CreateCore(reciver, coreLibrary);
		if(manager->GetCore() == 0)
		{
			UnloadCore();
			return false;
		}
		return true;
	};

	//Выгрузить библиотеку с ядром
	void UnloadCore()
	{
		if(handle)
		{
			if(manager)
			{
				manager->DeleteCore();
				manager = 0;
			}
			FreeLibrary(handle);
			handle = 0;
		}
	};

	//Получить параметры для старта игры из ini
	__forceinline void GetBootParams(const char * bootIni)
	{
		if(iniName) delete iniName; iniName = 0;
		//Зачитываем ini файл целиком
		if(!bootIni || !bootIni[0]) return;
		long iniNameLen = StrLen(bootIni) + 1;
		iniName = new char[iniNameLen];
		MemCpy(iniName, bootIni, iniNameLen);
		FILE * file = crt_fopen(bootIni, "rt");
		if(!file)
		{
			//Пробуем открыть с коротким именем
			long start = 0;
			for(dword i = 0; bootIni[i]; i++)
			{
				if(bootIni[i] == '\\' || bootIni[i] == '/')
				{
					start = i + 1;
				}
			}
			iniNameLen = StrLen(bootIni + start) + 1;
			MemCpy(iniName, bootIni + start, iniNameLen);
			if(bootIni[start] && start > 0)
			{
				file = crt_fopen(bootIni + start, "rt");
			}
			if(!file)
			{
				//ini не найден
				return;
			}
		}
		fseek(file, 0, SEEK_END);
		long fileSize = ftell(file);
		if(fileSize > 65535) fileSize = 65535;	//Что то слишком большой ini, не будем целиком разбирать его
		fseek(file, 0, SEEK_SET);
		unsigned char * ini = new unsigned char[fileSize + 1];
		fileSize = fread(ini, 1, fileSize, file);
		fclose(file);
		file = 0;
		if(fileSize == 0)
		{
			//Непрочитали
			delete ini;
			return;
		}
		ini[fileSize] = 0;
		//Теперь в ini лежит строка представляющая ini файл и заканчавающаяся нулём
		for(unsigned char * c = ini; *c;)
		{
			switch(*c)
			{
			case ';':
				//Пропустим коментарий
				for(c++; *c && *c != '\n'; c++);
				break;
			case '[':
				//Дошли до секции, заканчиваем поиски
				*c = 0;
				break;
			case '\r':
			case '\n':
			case ' ':
			case '\t':
				//Пропускаем пустые символы
				c++;
				break;
			default:
				//Начало ключа - разберём его
				{		
					unsigned char * key = c;
					for(c++; *c && *c != '\n' && *c != '='; c++);
					if(*c == '=')
					{
						unsigned char * t;
						//Ищем окончание ключа
						for(t = c - 1; *t == ' '  || *t == '\t'; t--);
						t[1] = 0;
						//Пропускаем пробелы после '='
						for(c++; *c == ' ' || *c == '\t'; c++);
						unsigned char * value = c;
						//Ищем окончание значения
						for(c++; *c && *c != '\n' && *c != '\r'; c++);
						for(t = c - 1; *t == ' '  || *t == '\t'; t--);
						t[1] = 0;
						c++;
						//Преобразуем значение ключа в нижний регистр
						for(t = key; *t; t++)
						{
							if(*t >= 'A' && *t <= 'Z')
							{
								*t += 'a' - 'A';
							}
						}
						//Добавляем ключ на анализ
						AddKey((const char *)key, (const char *)value);
					}
				}
			}
		}
		delete ini;
	}

	//Получить имя картинки	инициализации
	__forceinline const char * GetPictureName()
	{
		return pictureName ? pictureName : "";
	}

	//Типовая инициализация игры
	__forceinline bool DefaultGameInit()
	{
		if(!manager) return false;
		//Ставим переменные
		manager->SetState(cmcorestate_trace, trace);
		manager->SetState(cmcorestate_debugkeys, debugkeys);
		manager->SetState(cmcorestate_memcheck, memorycheck);
		manager->SetState(cmcorestate_panicmemcheck, panicmemorycheck);
		manager->SetState(cmcorestate_memorypools, memorypools);
		if(mtmode != mt_auto)
		{
			manager->SetState(cmcorestate_mtmanual, mtmode != mt_off);
		}
		manager->SetState(cmcorestate_lorestimer, loResTimer);
		manager->SetState(cmcorestate_fixedfps, fixedFPS);
		manager->SetVarString("system.ini", iniName);
		//Выводим в лог стартовое сообщение
		const char * ver = manager->GetVarString("system.core.id");
		const char * build = manager->GetVarString("system.core.build");
		const char * starttime = manager->GetVarString("system.core.starttime");
		manager->Trace(GetDivLine());
		manager->Trace("%s", ver ? ver : "??");
		manager->Trace("Build: %s", build ? build : "??");
		manager->Trace("Started: %s", starttime ? starttime : "??");
		manager->Trace(GetDivLine());
		if(iniName)
		{
			manager->Trace("Ini file: %s", iniName);
			manager->Trace(GetDivLine());
		}
		manager->Trace("Core states:");
		manager->Trace("    trace: %s", manager->GetState(cmcorestate_trace) ? "1" : "0");
		manager->Trace("    debugkeys: %s", manager->GetState(cmcorestate_debugkeys) ? "1" : "0");
		manager->Trace("    memorycheck: %s", manager->GetState(cmcorestate_memcheck) ? "1" : "0");
		manager->Trace("    panicmemorycheck: %s", manager->GetState(cmcorestate_panicmemcheck) ? "1" : "0");
		manager->Trace("    memorypools: %s", manager->GetState(cmcorestate_memorypools) ? "1" : "0");
		manager->Trace("    lorestimer: %s", manager->GetState(cmcorestate_lorestimer) ? "1" : "0");
		const char * mtmode_str = "auto";
		switch(mtmode)
		{
		case mt_off:
			mtmode_str = "0 (off)";
			break;
		case mt_on:
			mtmode_str = "1 (always)";
			break;
		case mt_auto:
			mtmode_str = "-1 (autodetect)";
			break;
		}
		manager->Trace("    multithreading: ini set = %s; current: %s", mtmode_str, manager->GetState(cmcorestate_mtmanual) ? "multi" : "single");
		manager->Trace("    no idle: %s", noidle ? "1" : "0");
		manager->Trace("    startimage: \"%s\"", pictureName ? pictureName : "");
		long fFPS = manager->GetState(cmcorestate_fixedfps);
		manager->Trace(fFPS ? "    fixedfps: %i (for activate press key 'Num Lock')" : "    fixedfps: off (set FPS count for enable this feature)", fFPS);
		if(quickModules)
		{
			for(const char * name = quickModules; name[0]; name++)
			{
				manager->Trace("    quick module: %s", name);
				while(name[0]) name++;
			}
		}
		if(objects)
		{
			for(const char * name = objects; name[0]; name++)
			{
				manager->Trace("    load class: %s", name);
				while(name[0]) name++;
			}
		}
		if(!QuickStart())
		{
			return false;
		}
		if(!Start())
		{
			return false;
		}
		return true;
	}

	//Запуск первичных сервисов, которые нужно запустить до основной загрузки
	__forceinline bool QuickStart(const char * quickStartModules = null)
	{
		if(!manager) return false;
		//Список модулей
		if(!quickStartModules) quickStartModules = quickModules;
		if(!quickStartModules || !quickStartModules[0]) return true;
		manager->Trace("%s\nStart quick modules loading\n%s", GetDivLine(), GetDivLine());
		const char * error = "\n!\n!-----= CoreLoader error =-----\n!\n!   ";
		//Загружаем быстрые модули
		if(!manager->LoadQuickStartModules(quickStartModules))
		{
			manager->Trace("%sCan't load some quick module!!!%s", error, error);
			manager->SetVarString("system.error", "Can't load some module, try reinstall game.");
			return false;
		}
		manager->Trace("%s\nStart quick modules load sucsesful.\nStart quick service creation.\n%s", GetDivLine(), GetDivLine());
		//Создаём сервисы с загруженых модулей
		if(!manager->CreateAllServices())
		{			
			manager->Trace("%sCan't create some service from quick modules%s", error, error);
			manager->SetVarString("system.error", "Can't create some service, try reinstall game.");
			return false;
		}
		manager->Trace("%s\nQuick modules services initialize sucsesful.\n%s", GetDivLine(), GetDivLine());
		return true;
	}

	//Запуск всего что небыло запущено
	__forceinline bool Start()
	{
		if(!manager) return false;
		manager->Trace("%s\nLoad modules\n%s", GetDivLine(), GetDivLine());
		const char * error = "\n!\n!-----= CoreLoader error =-----\n!\n!   ";
		//Загружаем модули
		manager->LoadModules();
		manager->Trace("%s\nLoad modules complette sucsesful.\nCreate services & objects\n%s", GetDivLine(), GetDivLine());
		//Создаём несозданные сервисы
		if(!manager->CreateAllServices())
		{			
			manager->Trace("%sCan't create some service%s", error, error);
			manager->SetVarString("system.error", "Can't create some service, try reinstall game.");
			return false;
		}		
		//Создаём объекты
		if(objects)
		{
			for(const char * name = objects; *name; name++)
			{
				if(!manager->CreateObject(name))
				{
					manager->Trace("%sCan't create object \"%s\"%s", error, name, error);
					manager->SetVarString("system.error", "Can't create some game object, try reinstall game.");
					return false;
				}
				while(*name) name++;
			}
		}
		manager->Trace("%s\nServices & objects creating sucsesful.\n%s", GetDivLine(), GetDivLine());
		return true;
	}

	__forceinline void ClearBootData()
	{
		if(pictureName) delete pictureName; pictureName = 0;
		if(quickModules) delete quickModules; quickModules = 0;
		if(objects) delete objects; objects = 0;
	}

	__forceinline const char * GetIniName()
	{
		return iniName;
	}

	//Получить доступ к менеджеру
	__forceinline CoreManager * Manager()
	{
		return manager;
	};

	__forceinline ICore * Core()
	{
		if(!manager) return 0;
		return manager->GetCore();
	};

	__forceinline bool IsNoIdle()
	{
		return noidle;
	}

	__forceinline long getWindowPositionX()
	{
		return winPosX;
	}

	__forceinline long getWindowPositionY()
	{
		return winPosY;
	}


protected:
	//Добавить ключ на анализ
	void AddKey(const char * key, const char * value)
	{
		if(StrCmp(key, "load class"))
		{
			AddObjectName(value);
		}else
		if(StrCmp(key, "quick module"))
		{
			AddQuickStartName(value);
		}else
		if(StrCmp(key, "memorycheck"))
		{
			if(value[0] == '0' && value[1] == 0)
			{
				memorycheck = false;
			}else
			if(value[0] == '1' && value[1] == 0)
			{
				memorycheck = true;
			}
		}else
		if(StrCmp(key, "panicmemorycheck"))
		{
			if(value[0] == '0' && value[1] == 0)
			{				
				panicmemorycheck = false;
			}else
			if(value[0] == '1' && value[1] == 0)
			{
				panicmemorycheck = true;
			}
		}else
		if(StrCmp(key, "memorypools"))
		{
			if(value[0] == '0' && value[1] == 0)
			{				
				memorypools = false;
			}else
				if(value[0] == '1' && value[1] == 0)
				{
					memorypools = true;
				}
		}else
		if(StrCmp(key, "trace"))
		{
			if(value[0] == '0' && value[1] == 0)
			{				
				trace = false;
			}else
			if(value[0] == '1' && value[1] == 0)
			{
				trace = true;
			}
		}else
		if(StrCmp(key, "debugkeys"))
		{
			if(value[0] == '0' && value[1] == 0)
			{				
				debugkeys = false;
			}else
				if(value[0] == '1' && value[1] == 0)
				{
					debugkeys = true;
				}
		}else
		if(StrCmp(key, "startimage"))
		{
			SetPictureName(value);
		}else
		if(StrCmp(key, "no idle"))
		{
			if(value[0] == '0' && value[1] == 0)
			{				
				noidle = false;
			}else
			if(value[0] == '1' && value[1] == 0)
			{
				noidle = true;
			}
		}else
		if(StrCmp(key, "multithreading"))
		{
			if(value[0] == '0' && value[1] == 0)
			{				
				mtmode = mt_off;
			}else
			if(value[0] == '1' && value[1] == 0)
			{
				mtmode = mt_on;
			}else{
				mtmode = mt_auto;
			}
		}else
		if(StrCmp(key, "lorestimer"))
		{
			if(value[0] == '0' && value[1] == 0)
			{				
				loResTimer = false;
			}else
			if(value[0] == '1' && value[1] == 0)
			{
				loResTimer = true;
			}else{
				loResTimer = false;
			}
		}else
		if(StrCmp(key, "fixedfps"))
		{			
			for(fixedFPS = 0; *value >= '0' && *value <= '9'; value++)
			{
				fixedFPS = fixedFPS*10 + (*value - '0');
			}
		}else		
		if(StrCmp(key, "windowposx"))
		{
			for(winPosX = 0; *value >= '0' && *value <= '9'; value++)
			{
				winPosX = winPosX*10 + (*value - '0');
			}
		} else			
		if(StrCmp(key, "windowposy"))
		{
			for(winPosY = 0; *value >= '0' && *value <= '9'; value++)
			{
				winPosY = winPosX*10 + (*value - '0');
			}
		}
	}

	//Добавить имя картинки
	void SetPictureName(const char * value)
	{
		if(pictureName) delete pictureName; pictureName = 0;
		long len = StrLen(value);
		if(!len) return;		
		pictureName = new char[len + 1];
		for(long i = 0; i <= len; i++)
		{
			pictureName[i] = value[i];
		}		
	}

	//Добавить объект для автоматического создания
	void AddObjectName(const char * value)
	{
		AddToArray(objects, value);
	}

	//Добавить модуль с сервисами для быстрого старта
	void AddQuickStartName(const char * value)
	{
		AddToArray(quickModules, value);
	}

private:
	__forceinline long StrLen(const char * s)
	{
		if(!s) return 0;
		long i;
		for(i = 0; s[i]; i++);
		return i;
	}
	__forceinline bool StrCmp(const char * s1, const char * s2)
	{
		for(; *s1 == *s2; s1++, s2++)
		{
			if(*s1 == 0) return true;
		}
		return false;
	}
	__forceinline void MemCpy(char * d, const char * s, long size)
	{
		for(; size; size--) *d++ = *s++;
	}
	__forceinline void AddToArray(char * & arr, const char * value)
	{
		if(!value || !value[0]) return;
		//Размер добавляемой строки
		long len = StrLen(value);
		if(!len) return;
		len++;
		//Определим размер массива
		long size;
		if(arr)
		{
			const char * e;
			for(e = arr; *e; e++)
			{
				while(*e) e++;
			}
			size = e - arr;
		}else{
			size = 0;
		}
		//Добавляем в массив
		char * tmp = arr;
		arr = new char[size + len + 1];
		MemCpy(arr, tmp, size);
		MemCpy(arr + size, value, len);
		arr[size + len] = 0;
		delete tmp;
	}

	__forceinline const char * GetDivLine()
	{
		static const char * str = "---------------------------------------------------------------------------------";
		return str;
	}

protected:
	CoreManager * manager;		//Мэнеджер ядра
	HMODULE handle;			//Хэндл dll
	char * iniName;			//Имя ini файла
	char * pictureName;		//Имя загрузочной картинки
	char * quickModules;		//Массив модулей с первичными сервисами для быстрого старта: name0name0...name00
	char * objects;			//Массив объектов: name0name0...name00
	bool memorycheck;		//Разрешить переодическую проверку памяти
	bool panicmemorycheck;		//Включить непрерывную полную проверку памяти
	bool memorypools;		//Пулы памяти
	bool trace;			//Вывод в лог
	bool debugkeys;			//Отладочные клавиши	
	bool noidle;			//При потере фокуса не переходить в idle режим
	bool loResTimer;		//Текущий таймер
	MTMode mtmode;			//Режим многопоточной работы
	long fixedFPS;			//Фиксированные FPS
	long winPosX;			//позиция окна по X
	long winPosY;			//позиция окна по Y
};


#endif


