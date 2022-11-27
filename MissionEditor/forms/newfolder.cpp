
#include "newfolder.h"
#include "mainwindow.h"
#include "globalParams.h"


#define WINDOW_POSX   400
#define WINDOW_POSY   10
#define WINDOW_WIDTH  440
#define WINDOW_HEIGHT 100

extern TMainWindow* MainWindow;


TNewFolder::TNewFolder () : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bAlwaysOnTop = true;
	bSystemButton = false;
  Caption = "Create folder";

	pDesc = NEW GUILabel (this, 20, 3, 20, 18);
	pDesc->Layout = GUILABELLAYOUT_Left;
	pDesc->Caption = "Folder name";
	pDesc->FontColor = 0xFF000000;
	pDesc->pFont->SetSize (16);


	pDirName = NEW GUIEdit (this, 23, 27, 300, 22);
  pDirName->Text = "NewFolder";
	pDirName->FontColor = 0xFF000000;
	pDirName->pFont->SetSize (16);
	pDirName->OnAccept = (CONTROL_EVENT)&TNewFolder::onOKPressed;
	pDirName->Flat = true;
		//onOKPressed;

	btnOK = NEW GUIButton (this, 340,9, 78, 24);
	btnOK->Glyph->Load ("ok");
	btnOK->Caption = "Accept";
	btnOK->pFont->SetSize (12);
	btnOK->OnMousePressed = (CONTROL_EVENT)&TNewFolder::onOKPressed;
	btnOK->FlatButton = true;
	
	
	btnCancel = NEW GUIButton (this, 340,39, 78, 24);
	btnCancel->Glyph->Load ("cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->pFont->SetSize (12);
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TNewFolder::onCancelPressed;
	btnCancel->FlatButton = true;


	SetScreenCenter ();
}

void TNewFolder::OnCreate ()
{
	pDirName->SetFocus ();
	pDirName->SelectText (0, pDirName->Text.Len ());
}


TNewFolder::~TNewFolder ()
{
 delete pDirName;
 delete pDesc;
}


void _cdecl TNewFolder::onOKPressed (GUIControl* sender)
{
	gp->folder_to_create = pDirName->Text;
	Close (this);
}

void _cdecl TNewFolder::onCancelPressed (GUIControl* sender)
{
	gp->folder_to_create = "";
	Close (this);
}


void TNewFolder::KeyPressed(int Key, bool bSysKey)
{
  if (!bSysKey) return;

	if (Key == 13) onOKPressed (NULL);
	if (Key == VK_ESCAPE) onCancelPressed (NULL);
}