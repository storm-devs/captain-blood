

#include "game.h"
#include "..\..\common_h\winworkpath.h"
#include "MutexProtector.h"

const char * windowName = "Game 'Captain Blood'";
const char * bootIni = "engine.ini";

/*
#ifndef STOP_DEBUG
#ifndef _XBOX

#define INTERNAL_DEBUGGER (1)

#endif
#endif
*/

#ifdef INTERNAL_DEBUGGER

#pragma message ("----> Storm3 Debugger Activate")

#include "dbg\Debugger.h"

appDebuger * globalApplicationDebuger = NULL;

#endif


int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR szCmdLine,int iCmdShow)
{
	SystemWorkPath workPath(SystemWorkPath::t_ini);	
	//Определяем имя загрузочного ini	
	if(szCmdLine && szCmdLine[0])
	{
		bootIni = szCmdLine;
	}
	workPath.AppendFileName(bootIni);
	//Проверка на копию
	MutexProtector mutex(bootIni);
	if(!mutex.Check())
	{
		::MessageBox(null, "Cant start more then one instance. Please close other, before start new.", windowName, MB_OK | MB_ICONWARNING);
		return -1;
	}
	//Создаём игру
	Game game(hInstance, windowName, workPath.GetPath());
	if(!game.InitWindow())
	{
		::MessageBox(null, "Can't create application window", "System error (" GAME_TITLE ")", MB_OK | MB_ICONERROR);
		return -1;
	}	




#ifdef INTERNAL_DEBUGGER
	globalApplicationDebuger = new appDebuger(Game::logFileName);
	globalApplicationDebuger->start(VK_CONTROL, 'X');
#endif

	game.CreateCore();


	//g_WheatyExceptionReport.SetLogFileName(game.GetLogFileName());
	//Основной цикл
	MSG msg;
	memset(&msg, 0, sizeof(msg));
	long idleCount = 0;
	while(true)
	{
		if(game.IsExit()) break;
		if(PeekMessage(&msg,NULL,0U,0U,PM_REMOVE))
		{
			if(msg.message != WM_QUIT)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}else{
				break;
			}
		}else{
			if(game.IsActive())
			{
				game.Update();
				idleCount = 0;
			}else{
				//idleCount--;
				if(idleCount > 0)
				{
					Sleep(100);
				}else{
					idleCount = 10;
					game.Update();
				}					
			}
		}
	}


	game.Release();


#ifdef INTERNAL_DEBUGGER
	if (globalApplicationDebuger)
	{
		delete globalApplicationDebuger;
		globalApplicationDebuger = NULL;
	}
#endif

	return msg.wParam;
}



