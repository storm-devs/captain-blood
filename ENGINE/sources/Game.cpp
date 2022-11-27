	

#include "game.h"
#include "..\..\common_h\systemmsg.h"
#include "..\..\common_h\Storm3Crt.h"



Game * Game::game = null;
const char * Game::logFileName = "system_log.txt";


Game::Log::Log()
{
	DeleteFile(logFileName);
}

Game::Log::~Log()
{	
}

//Принять сообщение
void Game::Log::Out(const char * msg)
{	
	if(!msg || !msg[0]) return;
	FILE * file = crt_fopen(logFileName, "a+b");
	if(file)
	{
		fseek(file, 0, SEEK_END);
		long len = strlen(msg);
		fwrite(msg, len, 1, file);
		fclose(file);
	}
}

Game::Tga::Tga()
{
	image = null;
	w = h = 0;
}

Game::Tga::~Tga()
{
	if(image) delete image;
	image = null;
	w = h = 0;
}

bool Game::Tga::Load(const char * name)
{
	FILE * file = crt_fopen(name, "rb");
	if(!file) return false;
	TGAHeader header;
	if(!fread(&header, sizeof(TGAHeader), 1, file)) goto error;
	if(header.ColorMapType > 1) goto error;
	switch(header.ImageType)
	{
	case 1:
	case 2:
	case 3:
		break;
	default:
		goto error;
	}
	if(header.PixelDepth != 32) goto error;
	w = header.Width;
	h = header.Height;
	{
		bool rightToLeft = (header.ImageDescriptor & 0x40) != 0;
		bool upToDown = (header.ImageDescriptor & 0x20) != 0;
		long palPnt = header.IDLength + sizeof(TGAHeader);
		long imagePnt = palPnt + header.CMapLength*((header.CMapDepth + 7)/8);
		fseek(file, imagePnt, SEEK_SET);
		image = new Color[w*h];
		if(!fread(image, w*h*sizeof(Color), 1, file)) goto error;
		if(upToDown)
		{
			long to = h/2;
			for(long y = 0; y < to; y++)
			{
				Color * line1 = image + y*w;
				Color * line2 = image + (h - 1 - y)*w;
				for(long x = 0; x < w; x++, line1++, line2++)
				{
					Color c = *line1;
					*line1 = *line2;
					*line2 = c;
				}
			}
		}
		if(rightToLeft)
		{
			Color * line = image;
			long to = w/2;
			for(long y = 0; y < h; y++, line += w)
			{
				for(long x = 0; x < to; x++)
				{
					Color c = line[x];
					line[x] = line[w - 1 - x];
					line[w - 1 - x] = c;
				}
			}
		}
	}
	return true;
error:
	fclose(file);
	if(image) delete image;
	image = null;
	w = h = 0;
	return false;	
}

HBITMAP Game::Tga::MakeBitmap(HDC hdc)
{
	if(!image) return null;
	BITMAPINFO btInfo = {sizeof(BITMAPINFO), w, h, 1, 32, BI_RGB, DWORD(w*h*32), 0, 0, 0, 0, 0, 0, 0, 0};
	return CreateDIBitmap(hdc, &btInfo.bmiHeader, CBM_INIT, image, &btInfo, DIB_RGB_COLORS);
	//return CreateBitmap(w, h, 1, 32, image);
}

HRGN Game::Tga::MakeRegion()
{
	if(!image) return null;
	const unsigned char threshold = 128;
	//Проверяем картинку на наличие прозрачных пикcелей
	long i, count;
	for(i = 0, count = w*h; i < count; i++)
	{
		if(image[i].a < threshold)
		{
			break;
		}
	}
	if(i >= count) return null;
	//Перебираем все сканлайны
	HRGN rgn = null;
	for(long y = 0; y < h; y++)
	{
		Color * line = image + (h - 1 - y)*w;
		for(long x = 0, xs = -1; x <= w; x++)
		{
			if(x == w || line[x].a < threshold)
			{
				if(xs >= 0)
				{
					HRGN tmp = CreateRectRgn(xs, y, x, y + 1);
					if(rgn)
					{
						CombineRgn(rgn, rgn, tmp, RGN_OR);
						DeleteObject(tmp);
					}else{
						rgn = tmp;
					}
					xs = -1;
				}
			}else{
				if(xs < 0)
				{
					xs = x;
				}
			}
		}
	}
	return rgn;
	
}


Game::Game(HINSTANCE _hInstance, const char * _windowName, const char * _bootIni)
{
	GameAssert(!game)
	game = this;
	GameAssert(_windowName && _windowName[0]);
	hInstance = _hInstance;
	hWnd = null;
	image = null;
	windowName = _windowName;
	bootIni = _bootIni;
	isInitCore = false;
	isActive = true;
	isNoIdle = false;
	isStartExitLookup = false;
	screenshotsCount = 0;
	srand(GetTickCount());
}

Game::~Game()
{
	Release();
}

void Game::Release()
{
	if(hWnd)
	{
		WINDOWINFO info;
		info.cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(hWnd, &info);
		if(loader.Manager())
		{
			loader.Manager()->SetVarInt("system.winPosX", info.rcClient.left);
			loader.Manager()->SetVarInt("system.winPosY", info.rcClient.top);
		}
	}	
	loader.UnloadCore();
	RemovePicture();
	game = null;
	if(hWnd)
	{
		HWND hwnd = hWnd;
		hWnd = null;
		DestroyWindow(hwnd);
	}
	PostQuitMessage(0);	
}

//Начальная инициализация
bool Game::InitWindow()
{
	app_cursor = ::LoadCursor(hInstance, "NULL_CURSOR");
	//app_cursor = ::LoadCursor(hInstance, IDC_WAIT);
	arrow_cursor = ::LoadCursor(hInstance, IDC_ARROW);

	//Регистрируем класс окна
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW |CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(WORD);
	wndclass.hInstance = hInstance;
	wndclass.hIcon = ::LoadIcon(hInstance,"IDI_ICON1");
	wndclass.hCursor = app_cursor;
	wndclass.hbrBackground = (HBRUSH)::GetStockObject(NULL_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = windowName;
	wndclass.hIconSm = ::LoadIcon(NULL,IDI_APPLICATION);
	::RegisterClassEx(&wndclass);

	//Запрашиваем у загрузчика параметры загрузки
	loader.GetBootParams(bootIni);
	bootIni = loader.GetIniName();

	//Создаём окно
	hWnd = ::CreateWindow(windowName,					//идентификатор класса
							windowName,					//название окна
							DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_VISIBLE,		//стиль окна
							loader.getWindowPositionX(),        //Позиция
							loader.getWindowPositionY(),
							1, 1,						//Размер
							null,						//Родитель
							null,						//Меню
							hInstance,					//Принодлежность к приложению
							null);
	if(!hWnd) return false;
	isNoIdle = loader.IsNoIdle();
	//Устанавливаем картинку на загрузку
	SetPicture(loader.GetPictureName());
	//Сообщим об необходимости перерисовать окно
	isInitCore = true;
	PostMessage(hWnd, WM_USER, 0, 0);
	::UpdateWindow(hWnd);
	RemovePicture();
	return true;
}

//Обновить игру
void Game::Update()
{
	static bool isProcess = false;	
//	try{		
		if(!loader.Manager()) return;
		if(!isProcess)
		{
			isProcess = true;
			MakeScreenshot();
			loader.Manager()->FrameUpdate();
			if(loader.Manager()->IsExit())
			{
				Release();
				PostQuitMessage(0);
			}
			isProcess = false;
		}		
//	}catch(...){
//	}	
}

//Активность приложения
bool Game::IsActive()
{
	return isActive | isNoIdle;
}

//Надо выходить
bool Game::IsExit()
{
	if(isStartExitLookup)
	{
		if(!loader.Manager()) return true;
		if(loader.Manager()->IsExit()) return true;
	}
	return false;
}

//Получить имя ини-файла
const char * Game::GetLogFileName()
{
	return logFileName;
}

//Создание ядра
void Game::CreateCore()
{
	isInitCore = false;
	//Создаём ядро
	if(!loader.LoadCore(&log))
	{
		::MessageBox(hWnd, "Can't create game core. Try reinstal game.", "System error", MB_OK | MB_ICONERROR);
		PostQuitMessage(-1);
		return;
	}
	//Устанавливаем глобальные переменные ядра
	GameAssert(loader.Manager());
	loader.Manager()->SetVarInt("system.hInstance", (long)hInstance);
	loader.Manager()->SetVarInt("system.hWnd", (long)hWnd);
	//Заводим заданные сервисы и объекты
	if(!loader.DefaultGameInit())
	{
		const char * error = loader.Manager()->GetVarString("system.error", "Game error");
		::MessageBox(hWnd, error, "Can't init game", MB_OK | MB_ICONERROR);
		PostQuitMessage(-1);
	}
	//Очищаем более ненужные ресурсы
	loader.ClearBootData();
	//Начинаем анализировать выход
	isStartExitLookup = true;

	// данные в базе используемые для различных защит
	// запомним в базе дату запуска
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	loader.Manager()->SetVarInt("system.date.month",(long)systime.wMonth);
	loader.Manager()->SetVarInt("system.date.day",(long)systime.wDay);
	loader.Manager()->SetVarInt("system.date.weekday",(long)systime.wDayOfWeek);
}

//Сообщить об ошибке
void Game::AssertFailed(const char * file, long line)
{
	_asm int 3;
}

//Обработчик сообщений
LRESULT CALLBACK Game::WndProc(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
	bool isSendMessage = false;
	switch(iMsg)
	{
	case WM_USER:
		if(game && game->isInitCore)
		{
			game->CreateCore();
		}
		break;
	case WM_PAINT:
		if(game && hwnd == game->hWnd)
		{
			WndProcPaint();
		}
		break;
	case WM_ACTIVATE:
		isSendMessage = true;
		if(game)
		{
			bool curActive = (LOWORD(wParam) == WA_CLICKACTIVE || LOWORD(wParam) == WA_ACTIVE);
			if(game->isActive != curActive)
			{
				if (curActive == false)
				{
					//Восстанавливаем виндовый курсор
					//::SetCursor(game->arrow_cursor);
					SetClassLong(game->hWnd, GCL_HCURSOR,(LONG)game->arrow_cursor);
					//OutputDebugString("arrow cursor\n");
				} else
				{
					//ПЕребиваем своим
					//::SetCursor(game->app_cursor);
					SetClassLong(game->hWnd, GCL_HCURSOR,(LONG)game->app_cursor);
					//OutputDebugString("app cursor\n");
				}
				game->isActive = curActive;
				if(game->loader.Manager())
				{
					game->loader.Manager()->SetActive(game->isActive);
				}
			}			
		}
		break;
/*
	case WM_SETCURSOR:
		isSendMessage = true;
		if(game && game->cursor) 
		{ 
			::SetCursor(game->cursor);
		}
		break;
*/
	case WM_SIZE:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case 0x20A:
		isSendMessage = true;
		break;
	case WM_SYSCOMMAND:
		if(wParam == SC_MONITORPOWER || wParam == SC_SCREENSAVE)
		{
			return 0;
		}
		break;
	case WM_CLOSE:
		{
			if(game && hwnd == game->hWnd)
			{
				game->Release();
			}			
			DestroyWindow(hwnd);
			return 0;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	if(isSendMessage)
	{
		if(game && hwnd == game->hWnd)
		{
			WndProcCtrl(iMsg, wParam, lParam);
		}
	}
	return DefWindowProc(hwnd,iMsg,wParam,lParam);
}

//Начало отрисовки
void Game::WndProcPaint()
{
	if(!game) return;
	PAINTSTRUCT paint;
	HDC dc = ::BeginPaint(game->hWnd, &paint);
	if(dc) game->Draw(dc);
	::EndPaint(game->hWnd, &paint);
}

//Отсылка клавишных сообщений
void Game::WndProcCtrl(UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	if(!game) return;
	if(game->loader.Manager())
	{
		if(game->loader.Manager())
		{
			ControlsMessage msg;
			msg.dwMsgID = iMsg;
			msg.wParam = wParam;
			msg.lParam = lParam;
			game->loader.Manager()->AddSystemMessage(CONTROLS_MESSAGE, &msg, sizeof(msg));
		}
	}
}

//Установить картинку с маской
void Game::SetPicture(const char * pictName)
{
	RemovePicture();
	if(!pictName || !pictName[0] || !hWnd) return;
	//Загружаем картинку
	Tga tga;
	if(!tga.Load(pictName)) return;



	MONITORINFO miMonInfo = { 0 };  
	miMonInfo.cbSize = sizeof( miMonInfo );   

	// Get associated monitor for our window  
	HMONITOR hMonitor = MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );   

	// Get monitor information  
	GetMonitorInfo( hMonitor, &miMonInfo );  

	long sw = miMonInfo.rcMonitor.right - miMonInfo.rcMonitor.left;
	long sh = miMonInfo.rcMonitor.bottom - miMonInfo.rcMonitor.top;



	//Форматируем окно под размер картинки
	//long sw = GetSystemMetrics(SM_CXSCREEN);
	//long sh = GetSystemMetrics(SM_CYSCREEN);

	long px = ((sw - tga.w)/2)+miMonInfo.rcMonitor.left;
	long py = ((sh - tga.h)/2)+miMonInfo.rcMonitor.top;

	MoveWindow(hWnd, px, py, tga.w, tga.h, false);
	//Получаем битмап
	HDC hdc = GetDC(hWnd);
	image = tga.MakeBitmap(hdc);
	ReleaseDC(hWnd, hdc);
	if(!image) return;
	//Создаём регион
	HRGN rgn = tga.MakeRegion();
	if(rgn)
	{
		::SetWindowRgn(hWnd, rgn, false);
	}

	cursor = ::LoadCursor(NULL,IDC_ARROW);
	::SetCursor(cursor);
	//Пререрисовываем окно
	UpdateWindow(hWnd);
}

//Убрать картинку
void Game::RemovePicture()
{
	::SetWindowRgn(hWnd, null, false);
	cursor = null;
	if(image)
	{
		::DeleteObject(image);
		image = null;		
	}	
}

//Нарисовать всё необходимое
void Game::Draw(HDC hdc)
{
	if(!hWnd) return;
	RECT rect;
	if(image)
	{
		GetWindowRect(hWnd, &rect);
		HDC memdc = CreateCompatibleDC(hdc);
		HGDIOBJ old = SelectObject(memdc, image);
		BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, memdc, 0, 0, SRCCOPY);
		SelectObject(memdc, old);
		DeleteDC(memdc);
	}
	Update();
}

#define _CORE_NO_NEW_
#include "..\..\common_h\core.h"
#include "..\..\common_h\render.h"

ICore * api = null;

//Сделать скриншот
void Game::MakeScreenshot()
{
	if(!loader.Core())
	{
		return;
	}
	//if(!loader.Core()->DebugKeyState(VK_SNAPSHOT))
	if(GetAsyncKeyState(VK_SNAPSHOT) >= 0)
	{
		return;
	}
	//Получим указатель на рендер
	IRender * render = (IRender *)loader.Core()->GetService("DX9Render");
	if(!render)
	{
		return;
	}
	//if(loader.Core()->DebugKeyState(VK_MENU))
	if(GetAsyncKeyState(VK_MENU) < 0)
	{
		//Сгенерим имя
		char name[64];
		crt_snprintf(name, sizeof(name) - 1, "_big%.4u%.4u_", screenshotsCount, rand());
		name[sizeof(name) - 1] = 0;
		screenshotsCount++;
		//Настроем временной промежуток
		bool isFixed = loader.Core()->IsFixedDeltaTime();
		float fdt = loader.Core()->GetFixedDeltaTime();
		loader.Core()->SetFixedDeltaTime(0.0f);
		loader.Core()->EnableFixedDeltaTime(true);
		//Установим режим корректирования матрицы проекции
		loader.Core()->Storage().SetLong("system.screenshot.Largeshot", 1);
		//Сохраняем картинку по частям		
		long c = 8;
		for(long y = 0; y < c; y++)
		{
			for(long x = 0; x < c; x++)
			{
				if(loader.Core()->DebugKeyState(VK_ESCAPE))
				{
					y = 1000000;
					break;
				}
				loader.Core()->Storage().SetFloat("system.screenshot.Projection scale", (float)c);
				loader.Core()->Storage().SetFloat("system.screenshot.Projection x", (float)x);
				loader.Core()->Storage().SetFloat("system.screenshot.Projection y", (float)y);
				loader.Manager()->FrameUpdate();
				//Сгенерим суфикс
				char sufix[256];
				crt_snprintf(sufix, sizeof(sufix) - 1, "%s_c%.2u_x%.2u_y%.2u_", name, c, x, y);
				sufix[sizeof(sufix) - 1] = 0;
				render->MakeScreenshot(sufix);
			}
		}
		//Возвратим настройки
		loader.Core()->SetFixedDeltaTime(fdt);
		loader.Core()->EnableFixedDeltaTime(isFixed);
		loader.Core()->Storage().SetLong("system.screenshot.Largeshot", 0);
	}else{
		render->MakeScreenshot();
	}
}


