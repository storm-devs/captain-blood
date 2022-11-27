#ifndef _XBOX

#include "gui_messagebox.h"
#include "gui_helper.h"
#include "..\gui_imanager.h"



GUIMessageBox::GUIMessageBox (int X, int Y, const char* caption, const char* message, DWORD flags, bool bFlat) : GUIWindow (NULL, X, Y, 100, 100)
{
	bHotKeys = true;
	pFont->SetName("arialcyrsmall");
	this->flags = flags;
	bAlwaysOnTop = true;
	t_OnOK     = NEW GUIEventHandler;
	t_OnCancel = NEW GUIEventHandler;


	
	
	
	int Height = 100;
	//GUIButton* 
	//GUIButton* CancelButton;
	//GUIURLLabel* MessageLabel;
	this->Caption = caption;
	
	//Application->modal_result = false;
	
	MessageLabel = NEW GUILabel (this, 8, 10, 500, 20);
	MessageLabel->Caption = message;
	MessageLabel->pFont->SetName("arialcyrsmall");
	MessageLabel->FontColor = 0xFF000000;
	MessageLabel->Layout = GUILABELLAYOUT_Left;

	
	int need_width = MessageLabel->pFont->GetWidth(MessageLabel->Caption.GetBuffer ());
	int width2 = pFont->GetWidth(Caption.GetBuffer()) + 20;
	if (width2 > need_width) need_width = width2;
	
	int razn = need_width;
	if (need_width < 150) need_width = 150;
	
	razn = (need_width - razn) / 2;
	
	GUIRectangle gr = MessageLabel->GetDrawRect ();
	gr.Left += razn;
	MessageLabel->SetDrawRect (gr);
	
	
	GUIRectangle bound_rec;
	bound_rec.Left = 0;
	bound_rec.Top = 0;
	bound_rec.Width = 60+60+14;
	bound_rec.Height = 24;
	
	GUIRectangle wnd_rec;
	wnd_rec.Left = 0;
	wnd_rec.Top = 0;
	wnd_rec.Width = (need_width+16);
	wnd_rec.Height = Height;
	
	GUIPoint n_pt;
	GUIHelper::Center (n_pt, bound_rec, wnd_rec);
	

	GUIRectangle r = GetDrawRect();
	int AddHeight = pFont->GetHeight(message) - pFont->GetHeight ();
	SetHeight (r.h + AddHeight);



  if (flags != GUIMB_OK)
	{
		OKButton = NEW GUIButton (this, n_pt.X, 40+AddHeight, 60, 24);
		
		OKButton->Caption = "OK";
		OKButton->FontColor = 0xFF000000;
//		OKButton->pFont->SetSize (16);
		OKButton->OnMousePressed = (CONTROL_EVENT)&GUIMessageBox::OKClick;
		
		CancelButton = NEW GUIButton (this, n_pt.X+60+14, 40+AddHeight, 60, 24);
		CancelButton->Caption = "Cancel";
		CancelButton->FontColor = 0xFF000000;
//		CancelButton->pFont->SetSize (16);
		CancelButton->OnMousePressed = (CONTROL_EVENT)&GUIMessageBox::CloseClick ;

		
		if (flags == GUIMB_YESNO)
		{
			OKButton->Caption = "Yes";
			CancelButton->Caption = "No";
		}

		if (flags == GUIMB_ABORTIGNORE)
		{
			OKButton->Caption = "Abort";
			CancelButton->Caption = "Ignore";
		}
	} else
		{
			GUIRectangle bound_rec2;
			bound_rec2.Left = 0;
			bound_rec2.Top = 0;
			bound_rec2.Width = 60;
			bound_rec2.Height = 24;

			wnd_rec.Left = 0;
			wnd_rec.Top = 0;
			wnd_rec.Width = (need_width+16);
			wnd_rec.Height = Height;


			GUIPoint n_pt2;
			GUIHelper::Center (n_pt2, bound_rec2, wnd_rec);


			OKButton = NEW GUIButton (this, n_pt2.X, 40+AddHeight, 60, 24);
			OKButton->Caption = "OK";
			OKButton->FontColor = 0xFF000000;
//			OKButton->pFont->SetSize (16);
			OKButton->OnMousePressed = (CONTROL_EVENT)&GUIMessageBox::OKClick;

			CancelButton = NULL;
		}
	
	
	if (OKButton) OKButton->pFont->SetName("arialcyrsmall");
	if (CancelButton) CancelButton->pFont->SetName("arialcyrsmall");
	if (bFlat)
	{
		if (OKButton) OKButton->FlatButton = true;
		if (CancelButton) CancelButton->FlatButton = true;
	}
	
	SetWidth (need_width+16);



	bSystemButton = false;
	
}

GUIMessageBox::~GUIMessageBox ()
{
	delete t_OnOK;
	delete t_OnCancel;
	delete OKButton;
	delete CancelButton;
	delete MessageLabel;
}


void _cdecl GUIMessageBox::OKClick (GUIControl* sender)
{
	//  Application->modal_result = true;
	Close (sender);
	t_OnOK->Execute (this);
}

void _cdecl GUIMessageBox::CloseClick (GUIControl* sender)
{
	//  Application->modal_result = false;
	Close (sender);
	t_OnCancel->Execute (this);
}

void GUIMessageBox::Draw ()
{
	GUIWindow::Draw ();
		 
}


bool GUIMessageBox::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	return GUIWindow::ProcessMessages(message, lparam, hparam);
}
//

void GUIMessageBox::KeyPressed( int key, bool bSysKey)
{
	if (!IsActive) return;
  if (!bHotKeys) return;
  if (!bSysKey) return;

	
	
	if (key == 13)
		if (OKButton) OKButton->OnPressed();
		
	if (key == VK_ESCAPE)
		if (CancelButton) CancelButton->OnPressed();


}
#endif