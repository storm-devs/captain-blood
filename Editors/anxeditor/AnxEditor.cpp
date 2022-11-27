//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================
//
//===========================================================================================================================
// AnxEditor
//===========================================================================================================================

#include "..\..\Common_h\FileService.h"
#include "AnxEditor.h"
#include "Forms\AnxEditorMainWindow.h"
#include "..\..\Common_h\corecmds.h"
#include "..\..\Common_h\InputSrvCmds.h"

#ifndef NO_TOOLS
CREATE_CLASS(AnxEditor)

//============================================================================================

AnxEditor::AnxEditor()
{
	gui = null;
}

AnxEditor::~AnxEditor()
{
	if(gui) delete gui;
}

//============================================================================================

bool AnxEditor::Init()
{
	//Блокируем в редакторе асинхронную загрузку
	IFileService * fs = (IFileService *)api->GetService("FileService");
	//GUI менеджер	
	gui = (IGUIManager *)api->CreateObject("GUIManager");
	if(!gui) return false;
	gui->PrecacheImages("");
	gui->PrecacheImages("AnxEditor");	
	//
	api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(true));
	IControlsService * ctrl = (IControlsService *)api->GetService("ControlsService");
	long num = ctrl->GetControlGroupsQuantity();
	for(long i = 0; i < num; i++)
	{
		ctrl->EnableControlGroup(i, false);
	}	
	ctrl->EnableControlGroup("GUI", true);
	ctrl->EnableControlGroup("AnxEditor", true);	
	ctrl->ExecuteCommand(InputSrvLockMouse(false));
	//
	gui->Show(wnd = NEW AnxEditorMainWindow());
	gui->ShowCursor(true);

	IRender * render = (IRender *)api->GetService("DX9Render");
	Assert(render);
	render->EnableLoadingScreen(false);

	return true;
}

void AnxEditor::Realize(dword Delta_Time)
{
}

#endif