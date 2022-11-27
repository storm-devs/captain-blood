//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxEditorMainWindow	
//============================================================================================

#ifndef _AnxEditorMainWindow_h_
#define _AnxEditorMainWindow_h_

#include "..\AnxBase.h"
#include "..\AnxProject.h"

class AnxEditorToolBar;

class AnxEditorMainWindow : public GUIWindow  
{
//--------------------------------------------------------------------------------------------
public:
	AnxEditorMainWindow();
	virtual ~AnxEditorMainWindow();
private:
	virtual void OnCreate();

//--------------------------------------------------------------------------------------------
public:
	//Рисование
	virtual void Draw();
	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);

	//Получить доступ к опциям
	AnxOptions & Options();

//--------------------------------------------------------------------------------------------
private:
	//Обрабатываем нажание левой кнопки мыши
	void MouseLeftDown(GUIPoint & pnt, dword flags);
	//Обрабатываем отпускание левой кнопки мыши
	void MouseLeftUp(GUIPoint & pnt, dword flags);
	//Обрабатываем нажание правой кнопки мыши
	void MouseRightDown(GUIPoint & pnt, dword flags);
	//Обрабатываем отпускание правой кнопки мыши
	void MouseRightUp(GUIPoint & pnt, dword flags);
	//Обрабатываем двойной щелчёк мыши
	void MouseDblClick(GUIPoint & pnt, dword flags);
	//Обрабатываем перемещение мышки
	void MouseMove(GUIPoint & pnt, GUIPoint & dpnt, dword flags);

//--------------------------------------------------------------------------------------------
private:
	AnxOptions options;
	GUIPoint cursorPos;
	AnxEditorToolBar * tools;
	bool controlState;
};

//Получить доступ к опциям
inline AnxOptions & AnxEditorMainWindow::Options()
{
	return options;
}

#endif

