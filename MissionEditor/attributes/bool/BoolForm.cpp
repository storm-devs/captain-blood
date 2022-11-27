
#include "boolform.h"
#include "boolattr.h"

#define WINDOW_WIDTH  180
#define WINDOW_HEIGHT 65




TBoolEdit::TBoolEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;
	bSystemButton = false;
  Caption  = BOOLEDIT_WINDOWNAME;


  cbValue = NEW GUICheckBox (this, 5, 5, 100, 32);
	cbValue->FontColor = 0xFF000000;
	cbValue->pFont->SetName("arialcyrsmall");
	cbValue->ImageChecked->Load ("checked");
	cbValue->ImageNormal->Load ("normal");
	
//  cbValue->OnChange = (CONTROL_EVENT)OnDataChange;

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

	btnOK->OnMousePressed = (CONTROL_EVENT)&TBoolEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TBoolEdit::OnButtonCancel;



	 

}

TBoolEdit::~TBoolEdit ()
{
 delete cbValue;
 delete btnOK;
 delete btnCancel;
}


void _cdecl TBoolEdit::OnDataChange (GUIControl* sender)
{

}


void _cdecl TBoolEdit::OnButtonOK (GUIControl* sender)
{
	MasterAttrib->SetValue (cbValue->Checked);
	Close (this);
}

void _cdecl TBoolEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}


bool TBoolEdit::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
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
