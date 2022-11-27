
#include "NewString.h"


#define WINDOW_POSX   400
#define WINDOW_POSY   10
#define WINDOW_WIDTH  440
#define WINDOW_HEIGHT 100


TNewLocStringWindow::TNewLocStringWindow (const string &curname) : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	ExitByOK = false;
	bAlwaysOnTop = true;
	bSystemButton = false;
	Caption = "New localization string:";

	pDesc = NEW GUILabel (this, 20, 3, 20, 18);
	pDesc->Layout = GUILABELLAYOUT_Left;
	pDesc->Caption = "New localization string:";
	pDesc->FontColor = 0xFF000000;
	pDesc->pFont->SetSize (16);


	pText = NEW GUIEdit (this, 23, 27, 300, 22);
  pText->Text = curname;
	pText->FontColor = 0xFF000000;
	pText->pFont->SetSize (16);
	pText->OnAccept = (CONTROL_EVENT)&TNewLocStringWindow::onOKPressed;
	pText->Flat = true;
		//onOKPressed;

	btnOK = NEW GUIButton (this, 340,9, 78, 24);
	btnOK->Glyph->Load ("ok");
	btnOK->Caption = "Accept";
	btnOK->pFont->SetSize (12);
	btnOK->OnMousePressed = (CONTROL_EVENT)&TNewLocStringWindow::onOKPressed;
	btnOK->FlatButton = true;
	
	
	btnCancel = NEW GUIButton (this, 340,39, 78, 24);
	btnCancel->Glyph->Load ("cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->pFont->SetSize (12);
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TNewLocStringWindow::onCancelPressed;
	btnCancel->FlatButton = true;


	SetScreenCenter ();
}

void TNewLocStringWindow::OnCreate ()
{
	pText->SetFocus ();
	pText->SelectText (0, pText->Text.Len ());
}


TNewLocStringWindow::~TNewLocStringWindow ()
{
 delete pText;
 delete pDesc;
}


void _cdecl TNewLocStringWindow::onOKPressed (GUIControl* sender)
{
	ExitByOK = true;
	Close (this);
}

void _cdecl TNewLocStringWindow::onCancelPressed (GUIControl* sender)
{
	Close (this);
}

void TNewLocStringWindow::KeyPressed(int Key, bool bSysKey)
{
  if (!bSysKey) return;

	if (Key == 13) onOKPressed (NULL);
	if (Key == VK_ESCAPE) onCancelPressed (NULL);
}


