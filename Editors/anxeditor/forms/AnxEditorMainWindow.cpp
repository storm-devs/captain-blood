//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxEditorMainWindow	
//============================================================================================
			

#include "AnxEditorMainWindow.h"
#include "AnxEditorToolBar.h"
#include "..\AntFile.h"
#include "..\Commands\AnxCmd_Undo.h"
#include "..\Commands\AnxCmd_Redo.h"
#include "..\Commands\AnxCmd_Copy.h"
#include "..\Commands\AnxCmd_Paste.h"
#include "..\Commands\AnxCmd_DelSelectedNodes.h"

//============================================================================================



AnxEditorMainWindow::AnxEditorMainWindow() : GUIWindow(null, 0, 0, 100, 100)
{
	//Инициализируем опции
	options.Init(GetRender());
	//Настраиваем окно
	GetRender()->SetBackgroundColor(options.screenBkg);
	SetWidth(GetRender()->GetScreenInfo3D().dwWidth);
	SetHeight(GetRender()->GetScreenInfo3D().dwHeight);
	bPopupStyle = true;
	controlState = false;

	/*
	options.project = NEW AnxProject(options);
	AntFile * ant = NEW AntFile(options);
	if(ant->Load("idle_0.ant"))
	{
		options.project->skeleton = ant->CreateMasterSkeleton();
	}else{
		delete options.project;
		options.project = null;
	}
	delete ant;
	*/
}

AnxEditorMainWindow::~AnxEditorMainWindow()
{
	if(options.project) delete options.project;
	if(tools) Application->DestroyGUIWindow(tools);
}

void AnxEditorMainWindow::OnCreate()
{
	options.gui_manager = Application;
	Application->Show(tools = NEW AnxEditorToolBar(options));
}


//============================================================================================



void AnxEditorMainWindow::Draw()
{
	dword bkgColor = options.screenBkg;
	if(options.IsShowInfo())
	{
		bkgColor = 0xffffffff;		
	}
	GetRender()->Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, bkgColor, 1.0f, 0);
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем данные проекта
	cliper.Push();
	if(options.project) options.project->Draw(GetRender(), GetDrawRect(), cliper);
	cliper.Pop();
	if(tools && options.project)
	{
		GUIControl * list[2] = {this, tools};
		for(long i = 0; i < Application->GetNumRegWindows(); i++)
		{
			GUIWindow * win = Application->GetRegWindow(i);
			for(long j = 0; j < ARRSIZE(list); j++)
			{
				if(win == list[j])
				{
					break;
				}
			}
			if(j == ARRSIZE(list))
			{
				break;
			}
		}
		if(i == Application->GetNumRegWindows())
		{
			bool isCtrl = (GetAsyncKeyState(VK_CONTROL) < 0);
			//Обрабатываем хоткеи
			if(isCtrl && GetAsyncKeyState('S') < 0)
			{
				if(!controlState)
				{
					tools->SaveProject(this);
					controlState = true;
				}
			}else
			if(isCtrl && GetAsyncKeyState('Z') < 0)
			{
				if(!controlState)
				{
					AnxCmd_Undo * cmd = NEW AnxCmd_Undo(options);
					cmd->Process();
					delete cmd;
					controlState = true;
				}
			}else
			if(isCtrl && GetAsyncKeyState('Y') < 0)
			{
				if(!controlState)
				{
					AnxCmd_Redo * cmd = NEW AnxCmd_Redo(options);
					cmd->Process();
					delete cmd;
					controlState = true;
				}
			}else
			if(isCtrl && GetAsyncKeyState('C') < 0)
			{
				if(!controlState)
				{
					AnxCmd_Copy * cmd = NEW AnxCmd_Copy(options);
					cmd->Process();
					delete cmd;
					controlState = true;
				}
			}else
			if(isCtrl && GetAsyncKeyState('V') < 0)
			{
				if(!controlState)
				{
					AnxCmd_Paste * cmd = NEW AnxCmd_Paste(options);
					cmd->Process();
					delete cmd;
					controlState = true;
				}
			}else
			if(GetAsyncKeyState(VK_DELETE) < 0)
			{
				if(!controlState)
				{
					AnxCmd_DelSelectedNodes * cmd = NEW AnxCmd_DelSelectedNodes(options);
					cmd->Process();
					delete cmd;
					controlState = true;
				}
			}else{
				controlState = false;
			}
		}
	}
}

//Сообщения
bool AnxEditorMainWindow::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	cursor_pos -= rect.pos;
	dword flags = 0;
	if(GetAsyncKeyState(VK_SHIFT) < 0) flags |= agks_shift;
	if(GetAsyncKeyState(VK_CONTROL) < 0) flags |= agks_ctrl;
	if(GetAsyncKeyState(VK_MENU) < 0) flags |= agks_alt;
	if(GetAsyncKeyState(VK_SPACE) < 0) flags |= agks_space;
	switch(message)
	{
	case GUIMSG_LMB_DOWN:
		cursorPos = cursor_pos;
		MouseLeftDown(cursor_pos, flags);
		return true;
	case GUIMSG_LMB_UP:
		cursorPos = cursor_pos;
		MouseLeftUp(cursor_pos, flags);		
		return true;
	case GUIMSG_RMB_DOWN:
		cursorPos = cursor_pos;
		MouseRightDown(cursor_pos, flags);		
		return true;
	case GUIMSG_RMB_UP:
		cursorPos = cursor_pos;
		MouseRightUp(cursor_pos, flags);		
		return true;
	case GUIMSG_MOUSEMOVE:		
		MouseMove(cursor_pos, cursor_pos - cursorPos, flags);		
		cursorPos = cursor_pos;
		return true;
	case GUIMSG_LMB_DBLCLICK:
		MouseDblClick(cursor_pos, flags);		
		cursorPos = cursor_pos;
		return true;
	}
	return false;
}


//Обрабатываем нажание левой кнопки мыши
void AnxEditorMainWindow::MouseLeftDown(GUIPoint & pnt, dword flags)
{
	if(options.project) options.project->graph.MouseLeftDown(pnt, flags);
}

//Обрабатываем отпускание левой кнопки мыши
void AnxEditorMainWindow::MouseLeftUp(GUIPoint & pnt, dword flags)
{
	if(options.project) options.project->graph.MouseLeftUp(pnt, flags);
}

//Обрабатываем нажание правой кнопки мыши
void AnxEditorMainWindow::MouseRightDown(GUIPoint & pnt, dword flags)
{
	if(options.project) options.project->graph.MouseRightDown(pnt, flags);
}

//Обрабатываем отпускание правой кнопки мыши
void AnxEditorMainWindow::MouseRightUp(GUIPoint & pnt, dword flags)
{
	if(options.project) options.project->graph.MouseRightUp(pnt, flags);
}

//Обрабатываем двойной щелчёк мыши
void AnxEditorMainWindow::MouseDblClick(GUIPoint & pnt, dword flags)
{
	if(options.project) options.project->graph.MouseDblClick(pnt, flags);
}

//Обрабатываем перемещение мышки
void AnxEditorMainWindow::MouseMove(GUIPoint & pnt, GUIPoint & dpnt, dword flags)
{
	if(options.project) options.project->graph.MouseMove(pnt, dpnt, flags);
}

/*
virtual void DrawHint (int pX, int pY, const string& Hint)
{
	;
}*/
