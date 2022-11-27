//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// SndEditor
//============================================================================================


#include "..\..\Common_h\FileService.h"
#include "SndEditor.h"
#include "Forms\SndEditorMainWindow.h"

#ifndef NO_TOOLS

CREATE_CLASS(SndEditor)


//============================================================================================

SndEditor::SndEditor()
{
	hMutex = null;
}

SndEditor::~SndEditor()
{
	if(options.gui_manager) delete options.gui_manager;
	if(hMutex)
	{
		::CloseHandle(hMutex);
		hMutex = null;
	}
}

//============================================================================================

bool SndEditor::Init()
{
	//Недопустим запуск второй копии редактора
	hMutex = ::CreateMutex(null, TRUE, "Storm engine sound editor mutex for diasable clones.");
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		api->Storage().SetString("system.error", "Cant start more sound editors then one instance...");
		api->Trace("Sound editor error! Cant start more then one instance...");
		return false;
	}
	//GUI менеджер
	options.gui_manager = (IGUIManager *)api->CreateObject("GUIManager");
	if(!options.gui_manager) return false;
	options.gui_manager->PrecacheImages("");
	options.gui_manager->PrecacheImages("SndEditor");
	//
	options.gui_manager->Show(NEW SndEditorMainWindow());
	options.gui_manager->ShowCursor(true);
	//
	options.render->EnableLoadingScreen(false);
	//
	if(options.screenWidth < 1000 || options.screenHeight < 680)
	{
		api->Trace("Sound editor error! Window resolution is too low. Minimum request is 1024x768...");
		return false;
	}
	//
	options.SetInputOptions();	
	return true;
}

void SndEditor::Realize(dword Delta_Time)
{
}

#endif

