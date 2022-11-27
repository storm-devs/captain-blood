

#include "..\common_h\coreloader.h"
#include <Xbdm.h>


namespace Game
{
	//-------------------------------------------------------------------------------------------
	//Log
	//-------------------------------------------------------------------------------------------

	class Log : public CoreSystemLogReciver
	{
	public:
		Log();
		virtual ~Log();

		//Принять сообщение
		virtual void Out(const char * msg);

	public:
		static const char * fileName;
	};


	const char * Log::fileName = "DEVKIT:\\system_log.txt";

	Log::Log()
	{
		DmMapDevkitDrive();
		DeleteFile(fileName);		
	}

	Log::~Log()
	{	
	}

	//Принять сообщение
	void Log::Out(const char * msg)
	{	
		if(!msg || !msg[0]) return;		
		FILE * file = crt_fopen(fileName, "a+b");
		if(file)
		{
			fseek(file, 0, SEEK_END);
			long len = strlen(msg);
			fwrite(msg, len, 1, file);
			fclose(file);
		}
		OutputDebugString(msg);
	}


	//-------------------------------------------------------------------------------------------
	//Global data
	//-------------------------------------------------------------------------------------------
		const char * bootIni = "game:\\xbox.ini";
		const char * coreDll = "game:\\systems.xdl";
		Log log;
		CoreLoader loader;
};


void __cdecl main()
{






/*	WIN32_FIND_DATA findData;
	HANDLE hFind = ::FindFirstFile("d:\\*", &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			int i = 0;
		}while(::FindNextFile(hFind, &findData));
	}*/
/*	HANDLE handle = 0;
	DWORD err = 0;

	handle = ::LoadLibrary("d:\\FileService.xdl");
	err = GetLastError();
	handle = ::LoadLibrary("d:\\AnimationService.xdl");
	err = GetLastError();
	handle = ::LoadLibrary("d:\\core.xdl");
	err = GetLastError();
*/

	//Запрашиваем у загрузчика параметры загрузки
	Game::loader.GetBootParams(Game::bootIni);
	//Создаём ядро
	if(!Game::loader.LoadCore(&Game::log, Game::coreDll) || !Game::loader.Manager())
	{
		DWORD err = GetLastError();
		Game::log.Out("Error: Core not loaded!");
		//DebugBreak();
		return;
	}
	//Устанавливаем глобальные переменные ядра
	if(!Game::loader.Manager()) return;
	Game::loader.Manager()->SetVarInt("system.hInstance", (long)0);
	Game::loader.Manager()->SetVarInt("system.hWnd", (long)0);
	//Заводим заданные сервисы и объекты
	if(!Game::loader.DefaultGameInit())
	{
		Game::log.Out("Error: Game initialization fault!");
	//	DebugBreak();
		return;
	}
	//Очищаем более ненужные ресурсы
	Game::loader.ClearBootData();
	Game::log.Out("Storm Engine 3.0 started...");
	//Наблюдаем за активностью игры
	HANDLE notificationListener = XNotifyCreateListener(XNOTIFY_SYSTEM);	
	//Основной цикл	
	while(true)
	{
		//Опрашиваем джойстики

		//JOKER: что бы на 4 -шифта не выходило, т.к. падает в звуке....
		/*
		for(DWORD i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			XINPUT_STATE inputState;
			if(XInputGetState(i, &inputState) != ERROR_SUCCESS) continue;
			if(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
			{
				if(inputState.Gamepad.bLeftTrigger > 128 && inputState.Gamepad.bRightTrigger > 128)
				{
					break;
				}	
			}
		}
		if(i < XUSER_MAX_COUNT)
		{
			return;
		}
		*/

		//	try{
		//Проверяем текущие состояние системных интерфейсов
		DWORD notificationId = 0;
		ULONG param = 0;
		if(XNotifyGetNext(notificationListener, XN_SYS_UI, &notificationId, &param))
		{
			if(notificationId == XN_SYS_UI)
			{
				bool isActiveGame = ((param == 0) ? true : false);
				Game::loader.Manager()->SetActive(isActiveGame);
			}
		}

		Game::loader.Manager()->FrameUpdate();
		if(Game::loader.Manager()->IsExit())
		{
			break;
			//loader.UnloadCore();
		}
		//	}catch(...){
		//	}
	}	

	if(notificationListener)
	{
		CloseHandle(notificationListener);
		notificationListener = null;
	}

}






