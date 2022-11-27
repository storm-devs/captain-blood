//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// SndEditorMainWindow
//============================================================================================

#ifndef _SndEditorMainWindow_h_
#define _SndEditorMainWindow_h_

#include "..\SndBase.h"
#include "..\SndOptions.h"

class SndEditorMainWindow : public GUIWindow
{

	enum Consts
	{
		TabHeight = 22,
		TabSpaceEnd = 10,
		TabSpaceStart = TabHeight,
		CloseButtonSize = 18,
		SaveImageRightPos = 64,
		SaveImageSize = 16,
	};




	struct Tab
	{
		const char * name;
		float textX, textY;
		long xs, xe;
		long width;
		GUIControl * tab;
		GUIControl * (SndEditorMainWindow::* createTab)();
		GUIPoint points[6];
	};

//--------------------------------------------------------------------------------------------
public:
	SndEditorMainWindow();
	virtual ~SndEditorMainWindow();
private:
	virtual void OnCreate();

//--------------------------------------------------------------------------------------------
private:
	//Рисование
	virtual void Draw();
	//Нарисовать закладку
	void DrawTab(Tab & tab, bool isSelect);
	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	//Нажатие кнопок
	virtual void KeyPressed (int key, bool bSysKey);
	//Проверить на попадание в закладку
	bool IsInsideTab(Tab & tab, GUIPoint & p);
	//Выделить закладку
	void SelectTab(long tabIndex);
	//Создание закладак по необходимости
	GUIControl * CreateTabSounds();
	GUIControl * CreateTabWaves();
	GUIControl * CreateTabAnimation();
	GUIControl * CreateTabPolygon();
	GUIControl * CreateTabGame();
	//Эвент закрытия редактора
	void _cdecl OnCloseApp(GUIControl* sender);


//--------------------------------------------------------------------------------------------
private:
	GUIPoint cursorPos;			//Позиция курсора
	bool setCloseState;			//Как только проект запишет надо закрыть приложение
	long infoStartX;			//Отступ по горизонтали информационной строки

private:
	long selTab;				//Текущая закладка
	Tab tabs[5];				//Закладки	







	enum test
	{
		t_bufSize = 128000,
		t_spectSize = 128,
		t_spectMin = 20,
		t_spectMax = 20000,
		t_spectLen = coremin(t_bufSize, 44100)/2,
	};
	float src[t_bufSize];
	float prc[t_bufSize];	
	short prev[t_bufSize];
	float spect[t_spectSize];
	UniqId prevId;
};

#endif

