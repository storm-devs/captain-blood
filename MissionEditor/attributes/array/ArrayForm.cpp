
#include "arrayform.h"
#include "arrayattr.h"

#define WINDOW_WIDTH  180
#define WINDOW_HEIGHT 65




TArrayEdit::TArrayEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;
	bSystemButton = false;
  Caption  = ARRAYEDIT_WINDOWNAME;


  eValue = NEW GUIEdit (this, 8, 5, 165, 19);
	eValue->Flat = true;
	//eValue->OnAccept = (CONTROL_EVENT)OnDataChange;
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

	btnOK->OnMousePressed = (CONTROL_EVENT)&TArrayEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TArrayEdit::OnButtonCancel;
}


void TArrayEdit::OnCreate()
{
	eValue->SelectText(0, eValue->Text.Size());
	eValue->SetFocus();
}

TArrayEdit::~TArrayEdit ()
{
 delete eValue;
 delete btnOK;
 delete btnCancel;
}


void _cdecl TArrayEdit::OnDataChange (GUIControl* sender)
{

}


void _cdecl TArrayEdit::OnButtonOK (GUIControl* sender)
{
//	MasterAttrib->SetValue (cbValue->Checked);

	int newSize = atoi (eValue->Text.GetBuffer ());
	MasterAttrib->Resize(newSize);
	Close (this);
}

void _cdecl TArrayEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}


void TArrayEdit::KeyPressed (int key, bool bSysKey)
{
  if (!bSysKey) return;
	if (!IsActive) return;
	if (key == 13) OnButtonOK (this);
}
