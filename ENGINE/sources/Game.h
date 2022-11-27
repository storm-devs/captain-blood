
#ifndef _Game_h_
#define _Game_h_

#include <windows.h>
#include "..\..\common_h\coreloader.h"


#define null 0

#define GameAssert(cond)		{ if((cond) == 0){ Game::AssertFailed(__FILE__, __LINE__); } }

class Game
{
	class Log : public CoreSystemLogReciver
	{
	public:
		Log();
		virtual ~Log();
		
		//Принять сообщение
		virtual void Out(const char * msg);

	protected:
	};

	struct Tga
	{
#pragma pack(push, 1)
		struct TGAHeader
		{
			unsigned char IDLength;
			unsigned char ColorMapType;
			unsigned char ImageType;
			unsigned short CMapStart;
			unsigned short CMapLength;
			unsigned char CMapDepth;
			unsigned short XOffset;
			unsigned short YOffset;
			unsigned short Width;
			unsigned short Height;
			unsigned char PixelDepth;
			unsigned char ImageDescriptor;
		};

		struct Color
		{
			unsigned char b;
			unsigned char g;
			unsigned char r;
			unsigned char a;
		};

#pragma pack(pop)

		Tga();
		~Tga();

		bool Load(const char * name);
		HBITMAP MakeBitmap(HDC hdc);
		HRGN MakeRegion();

		Color * image;
		long w, h;
	};


public:
	Game(HINSTANCE _hInstance, const char * _windowName, const char * _bootIni);
	~Game();
	void Release();

	//Инициализировать окно
	bool InitWindow();
	//Обновить игру
	void Update();
	//Активность приложения
	bool IsActive();
	//Надо выходить
	bool IsExit();
	//Получить имя лог-файла
	const char * GetLogFileName();

	//Сообщить об ошибке
	static void AssertFailed(const char * file, long line);

	//Создание ядра
	void CreateCore();


protected:
	//Установить картинку с маской
	void SetPicture(const char * pictName);
	//Убрать картинку
	void RemovePicture();
	//Нарисовать всё необходимое
	void Draw(HDC hdc); 
	//Сделать скриншот
	void MakeScreenshot();

protected:
	//Обработчик сообщений
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	//Начало отрисовки
	static void WndProcPaint();
	//Отсылка клавишных сообщений
	static void WndProcCtrl(UINT iMsg, WPARAM wParam, LPARAM lParam);
	
	
protected:
	HINSTANCE hInstance;
	HWND hWnd;
	HBITMAP image;
	HCURSOR cursor;	
	HCURSOR arrow_cursor;
	HCURSOR app_cursor;

	const char * windowName;
	const char * bootIni;
	Log log;
	CoreLoader loader;
	bool isActive;
	bool isInitCore;
	bool isNoIdle;
	bool isStartExitLookup;
	long screenshotsCount;
	static Game * game;

public:
	static const char * logFileName;
};



#endif
