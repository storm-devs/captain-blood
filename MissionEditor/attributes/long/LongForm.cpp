
#include "longform.h"
#include "longattr.h"
#include "..\strutil.h"

#define WINDOW_WIDTH  180
#define WINDOW_HEIGHT 65




TLongEdit::TLongEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;
	bSystemButton = false;
  Caption = LONGEDIT_WINDOWNAME;

  eValue = NEW GUIEdit (this, 8, 5, 165, 19);
	eValue->Flat = true;
	eValue->OnAccept = (CONTROL_EVENT)&TLongEdit::OnDataChange;
	eValue->pFont->SetName("arialcyrsmall");



	btnOK = NEW GUIButton (this, 9,30, 78, 24);
	btnOK->Glyph->Load ("meditor\\ok");
	btnOK->Caption = "Accept";
	btnOK->pFont->SetName("arialcyrsmall");
	btnOK->FlatButton = true;

	btnCancel = NEW GUIButton (this, 95,30, 78, 24);
	btnCancel->Glyph->Load ("meditor\\cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->pFont->SetName("arialcyrsmall");
	btnCancel->FlatButton = true;


	btnOK->OnMousePressed = (CONTROL_EVENT)&TLongEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TLongEdit::OnButtonCancel;
}

void TLongEdit::OnCreate()
{
	eValue->SelectText(0, eValue->Text.Size());
	eValue->SetFocus();
}

void TLongEdit::KeyPressed(int key, bool bSysKey)
{
  if (!bSysKey) return;

	if (key == 13) OnButtonOK (this);
	if (key == VK_ESCAPE) OnButtonCancel (this);
}

TLongEdit::~TLongEdit ()
{
	delete btnOK;
	delete btnCancel;


  delete eValue;




}


void _cdecl TLongEdit::OnDataChange (GUIControl* sender)
{
	if (MasterAttrib->GetIsLimit ())
	{
		int needValue = atoi (eValue->Text.GetBuffer ());

		if (needValue < MasterAttrib->GetMin ())  needValue = MasterAttrib->GetMin ();
		if (needValue > MasterAttrib->GetMax ())  needValue = MasterAttrib->GetMax ();

		eValue->Text = IntToStr (needValue);
	}
 
}


void _cdecl TLongEdit::OnButtonOK (GUIControl* sender)
{
	int needValue = atoi (eValue->Text.GetBuffer ());
	MasterAttrib->SetValue (needValue);
	Close (this);
}

void _cdecl TLongEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}

bool TLongEdit::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	return GUIWindow::ProcessMessages (message, lparam, hparam);
}

