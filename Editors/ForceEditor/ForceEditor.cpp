#include "ForceEditor.h"
#include "ForceEditorMainWindow.h"

#include "..\..\common_h\InputSrvCmds.h"

#include "..\..\common_h\gui.h"
#include "..\..\common_h\controls.h"

#include "..\..\common_h\FileService.h"

#ifndef NO_TOOLS
CREATE_CLASS(ForceEditor)
#endif

IControlsService *controls	  = null;
IGUIManager		 *application = null;
IFileService	 *storage	  = null;
IRender			 *render	  = null;

ForceEditor:: ForceEditor()
{
}

ForceEditor::~ForceEditor()
{
}

bool ForceEditor::Init()
{
	controls = (IControlsService *)api->GetService("ControlsService");
	controls->ExecuteCommand(InputSrvLockMouse(false));

	render = (IRender *)api->GetService("DX9Render");

	application = (IGUIManager *)api->CreateObject("GUIManager");
	Assert(application)

	storage = (IFileService *)api->GetService("FileService");

	main = NEW ForceEditorMainWindow;
	application->Show(main);

	return true;
}
