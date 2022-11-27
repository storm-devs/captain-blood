
#include "floatform.h"
#include "floatattr.h"
#include "..\strutil.h"
#include "..\..\fast_atof.h"

#define WINDOW_WIDTH  180
#define WINDOW_HEIGHT 65




TFloatEdit::TFloatEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;
	bSystemButton = false;
  Caption = FLOATEDIT_WINDOWNAME;

  eValue = NEW GUIEdit (this, 6, 5, 165, 19);
	eValue->OnAccept = (CONTROL_EVENT)&TFloatEdit::OnDataChange;
	eValue->Flat = true;
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

	btnOK->OnMousePressed = (CONTROL_EVENT)&TFloatEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TFloatEdit::OnButtonCancel;

//  cbValue->OnChange = 
}

TFloatEdit::~TFloatEdit ()
{
	delete btnOK;
	delete btnCancel;


  delete eValue;


}


void _cdecl TFloatEdit::OnDataChange (GUIControl* sender)
{
	if (MasterAttrib->GetIsLimit ())
	{
		float needValue = fast_atof (eValue->Text.GetBuffer ());

		if (needValue < MasterAttrib->GetMin ())  needValue = MasterAttrib->GetMin ();
		if (needValue > MasterAttrib->GetMax ())  needValue = MasterAttrib->GetMax ();

		eValue->Text = FloatToStr (needValue);
	}
}


void _cdecl TFloatEdit::OnButtonOK (GUIControl* sender)
{
	float needValue = fast_atof (eValue->Text.GetBuffer ());
	MasterAttrib->SetValue (needValue);

	Close (this);
}

void _cdecl TFloatEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}

bool TFloatEdit::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (message == GUIMSG_KEYPRESSED)
	{
		int key = (int)lparam;
		if ((key == ' ') || (key == 13))
		{
			OnButtonOK (this);
			return true;
		}
	}
	return GUIWindow::ProcessMessages (message, lparam, hparam);
}


void TFloatEdit::OnCreate()
{
	eValue->SelectText(0, eValue->Text.Size());
	eValue->SetFocus();
}

void TFloatEdit::KeyPressed(int key, bool bSysKey)
{
  if (!bSysKey) return;
	if (key == 13) OnButtonOK (this);
	if (key == VK_ESCAPE) OnButtonCancel (this);
}
