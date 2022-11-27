
#include "enumform.h"
#include "enumattr.h"

#define WINDOW_WIDTH  240
#define WINDOW_HEIGHT 65




TEnumEdit::TEnumEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;
	bSystemButton = false;
  Caption  = ENUMEDIT_WINDOWNAME;


//	cbValue->FontColor = 0xFF000000;
//	cbValue->pFont->SetSize (14);
//	cbValue->ImageChecked->Load ("checked");
//	cbValue->ImageNormal->Load ("normal");
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

	btnOK->OnMousePressed = (CONTROL_EVENT)&TEnumEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TEnumEdit::OnButtonCancel;


  cbValue = NEW GUIComboBox (this, 5, 5, 230, 18);
	cbValue->Edit->pFont->SetName("arialcyrsmall");
	cbValue->ListBox->pFont->SetName("arialcyrsmall");
	cbValue->MakeFlat (true);

	 

}

TEnumEdit::~TEnumEdit ()
{
 delete cbValue;
 delete btnOK;
 delete btnCancel;
}


void _cdecl TEnumEdit::OnDataChange (GUIControl* sender)
{

}


void _cdecl TEnumEdit::OnButtonOK (GUIControl* sender)
{
//	MasterAttrib->SetValue (cbValue->Checked);

	MasterAttrib->SetValue (cbValue->ListBox->SelectedLine);
	Close (this);
}

void _cdecl TEnumEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}

bool TEnumEdit::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
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




