
#include "VSSLogin.h"
#include "mainwindow.h"
#include "globalParams.h"


#define WINDOW_POSX   400
#define WINDOW_POSY   10
#define WINDOW_WIDTH  440
#define WINDOW_HEIGHT 100

extern TMainWindow* MainWindow;


TVSSLogin::TVSSLogin () : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT)
{

	ExitByOK = false;
	bAlwaysOnTop = true;
	bSystemButton = false;
  Caption = "Login to VSS";

	pDesc = NEW GUILabel (this, 20, 3, 20, 18);
	pDesc->Layout = GUILABELLAYOUT_Left;
	pDesc->Caption = string ("User");
	pDesc->FontColor = 0xFF000000;
	pDesc->pFont->SetSize (16);


	pText = NEW GUIEdit (this, 23, 27, 300, 22);
    pText->Text = gp->VSSUserName;
	pText->FontColor = 0xFF000000;
	pText->pFont->SetSize (16);
	pText->OnAccept = (CONTROL_EVENT)&TVSSLogin::onOKPressed;
	pText->Flat = true;
		//onOKPressed;

	btnOK = NEW GUIButton (this, 340,9, 78, 24);
	btnOK->Glyph->Load ("ok");
	btnOK->Caption = "Accept";
	btnOK->pFont->SetSize (12);
	btnOK->OnMousePressed = (CONTROL_EVENT)&TVSSLogin::onOKPressed;
	btnOK->FlatButton = true;
	
	
	btnCancel = NEW GUIButton (this, 340,39, 78, 24);
	btnCancel->Glyph->Load ("cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->pFont->SetSize (12);
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TVSSLogin::onCancelPressed;
	btnCancel->FlatButton = true;


	SetScreenCenter ();
}

void TVSSLogin::OnCreate ()
{
	pText->SetFocus ();
	pText->SelectText (0, pText->Text.Len ());
}


TVSSLogin::~TVSSLogin ()
{
 delete pText;
 delete pDesc;
}


void _cdecl TVSSLogin::onOKPressed (GUIControl* sender)
{
	gp->VSSUserName = pText->Text;
	ExitByOK = true;
	Close (this);
}

void _cdecl TVSSLogin::onCancelPressed (GUIControl* sender)
{
	//folder_to_create = "";
	Close (this);
}

void TVSSLogin::KeyPressed(int Key, bool bSysKey)
{
  if (!bSysKey) return;

	if (Key == 13) onOKPressed (NULL);
	if (Key == VK_ESCAPE) onCancelPressed (NULL);
}


