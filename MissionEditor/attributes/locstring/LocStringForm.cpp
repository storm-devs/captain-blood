

#include "locstringform.h"
#include "locstringattr.h"
#include "..\strutil.h"
#include "..\pathutils.h"

#include "..\..\forms\mainwindow.h"
#include "..\..\forms\resourceselect.h"
#include "..\..\..\common_h\mission.h"


extern TMainWindow* MainWindow;
extern IMission* miss;

#define WINDOW_WIDTH  280
#define WINDOW_HEIGHT 65

extern IRender* pRS;


string currentLocStr_value;

extern ILocStrings* pLocStr;



TLocStringEdit::TLocStringEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	currentValue = -1;
	bPopupStyle = true;
	bAlwaysOnTop = true;

	bSystemButton = false;
  Caption = STRINGEDIT_WINDOWNAME;

  TextValue = NEW GUIEdit (this, 6, 6, 217-12, 19);
	TextValue->pFont->SetName("arialcyrsmall");
	TextValue->Flat = true;
	TextValue->Enabled = false;
  
	btnOK = NEW GUIButton (this, 30+9+15,30, 78, 24);
	btnOK->Glyph->Load ("meditor\\ok");
	btnOK->Caption = "Accept";
	btnOK->pFont->SetName("arialcyrsmall");
	btnOK->FlatButton = true;

	btnCancel = NEW GUIButton (this, 30+95+15,30, 78, 24);
	btnCancel->Glyph->Load ("meditor\\cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->pFont->SetName("arialcyrsmall");
	btnCancel->FlatButton = true;

//------------------


	btnTakeFromTranslatedString = NEW GUIButton (this, 212+5,6, 22, 22);
	btnTakeFromTranslatedString->Glyph->Load ("meditor\\local_text");
	btnTakeFromTranslatedString->OnMousePressed = (CONTROL_EVENT)&TLocStringEdit::OnSelectFromLocalText;
	btnTakeFromTranslatedString->FlatButton = true;
	btnTakeFromTranslatedString->Hint = "Take string from file";


	btnOK->OnMousePressed = (CONTROL_EVENT)&TLocStringEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TLocStringEdit::OnButtonCancel;

}

TLocStringEdit::~TLocStringEdit ()
{

}



void _cdecl TLocStringEdit::OnButtonOK (GUIControl* sender)
{
	MasterAttrib->SetValue (currentValue);
	
	Close (this);
}

void _cdecl TLocStringEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}






void TLocStringEdit::KeyPressed(int key, bool bSysKey)
{
  if (!bSysKey) return;

	if (bLocked) return;


	if (key == 13) OnButtonOK (this);
}


void _cdecl TLocStringEdit::OnSelectFromLocalText (GUIControl* sender)
{
	TLocTextEditor* locEditor = NULL;
	locEditor = NEW TLocTextEditor (0, 0);
	locEditor->OnClose = (CONTROL_EVENT)&TLocStringEdit::OnSelectFromLocalTextClose;
	bLocked = true;
	Application->ShowModal (locEditor);
}

void _cdecl TLocStringEdit::OnSelectFromLocalTextClose (GUIControl* sender)
{
	TLocTextEditor* sndr = (TLocTextEditor*)sender;
	currentValue = sndr->listBox->SelectedLine;

	if (currentValue >= 0)
	{
		currentLocStr_value = sndr->listBox->Items.Get(currentValue).c_str();
	} else
	{
		currentLocStr_value = "";
	}

	currentValue = pLocStr->GetIdByIndex(currentValue);

	TextValue->Text = currentLocStr_value;

	bLocked = false;

}
