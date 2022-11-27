
#include "stringform.h"
#include "stringattr.h"
#include "..\strutil.h"
#include "..\pathutils.h"
#include "SelectFromMission.h"
#include "SelectFromGMX.h"

#include "..\..\forms\mainwindow.h"
#include "..\..\forms\resourceselect.h"
#include "..\..\..\common_h\mission.h"

TSSFMission* newMisWnd = NULL;
//TSSFGeometry* newGMXWnd = NULL;

extern TMainWindow* MainWindow;
extern IMission* miss;
string my_val;
string my_gmx_val;

#define WINDOW_WIDTH  302
#define WINDOW_HEIGHT 65

extern IRender* pRS;





TStringEdit::TStringEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;

	bSystemButton = false;
  Caption = STRINGEDIT_WINDOWNAME;

  eValue = NEW GUIEdit (this, 6, 6, 217-12, 19);
	eValue->OnAccept = (CONTROL_EVENT)&TStringEdit::OnDataChange;
	eValue->pFont->SetName("arialcyrsmall");
	eValue->Flat = true;
  
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
/*
  btnTakeFromStatic = NEW GUIButton (this, 212+59,6, 22, 22);
  btnTakeFromStatic->Glyph->Load ("meditor\\take_from_static");
	btnTakeFromStatic->OnMousePressed = (CONTROL_EVENT)OnSelectFromGMX;
	btnTakeFromStatic->FlatButton = true;
	btnTakeFromStatic->Hint = "Take string from static geometry";
*/

	btnTakeFromMission = NEW GUIButton (this, 212+32,6, 22, 22);
	btnTakeFromMission->Glyph->Load ("meditor\\take_from_mission");
	btnTakeFromMission->OnMousePressed = (CONTROL_EVENT)&TStringEdit::OnSelectFromMission;
	btnTakeFromMission->FlatButton = true;
	btnTakeFromMission->Hint = "Take string from mission";

	btnTakeFromFile = NEW GUIButton (this, 212+5,6, 22, 22);
	btnTakeFromFile->Glyph->Load ("folder");
	btnTakeFromFile->OnMousePressed = (CONTROL_EVENT)&TStringEdit::OnSelectFromFile;
	btnTakeFromFile->FlatButton = true;
	btnTakeFromFile->Hint = "Take string from file";

	btnOK->OnMousePressed = (CONTROL_EVENT)&TStringEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TStringEdit::OnButtonCancel;


}

TStringEdit::~TStringEdit ()
{
	delete btnTakeFromFile;
//	delete btnTakeFromStatic;
	delete btnTakeFromMission;

	delete btnOK;
	delete btnCancel;


  delete eValue;


}


void _cdecl TStringEdit::OnDataChange (GUIControl* sender)
{
	if (MasterAttrib->GetIsLimit ())
	{
		if (MasterAttrib->onlyHi) eValue->Text.Upper();
		if (MasterAttrib->onlyLo) eValue->Text.Lower ();
	}

   
}


void _cdecl TStringEdit::OnButtonOK (GUIControl* sender)
{
	if (MasterAttrib->GetIsLimit ())
	{
		if (eValue->Text.Len() < (DWORD)MasterAttrib->GetMinChars()) return;
		if (eValue->Text.Len() > (DWORD)MasterAttrib->GetMaxChars()) return;
	}

	MasterAttrib->SetValue (eValue->Text);

	
	Close (this);
}

void _cdecl TStringEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}

void _cdecl TStringEdit::OnSelectFromGMX (GUIControl* sender)
{
/*
	StaticGeometry* sg = miss->GetStatic ();
	IGMXScene* pSc = NULL;
	if (sg)	pSc = sg->GetScene ();


	int cX;
	int cY;
	Application->GetCursor (cX, cY);
	newGMXWnd = NEW TSSFGeometry (cX, cY, pSc);
	newGMXWnd->OnClose = (CONTROL_EVENT)OnCloseSFG;
	Application->ShowModal (newGMXWnd);
 */
}

void _cdecl TStringEdit::OnSelectFromMission (GUIControl* sender)
{
	int cX;
	int cY;
	Application->GetCursor (cX, cY);
	newMisWnd = NEW TSSFMission (cX, cY);
	newMisWnd->OnClose = (CONTROL_EVENT)&TStringEdit::OnCloseSFM;
	Application->ShowModal (newMisWnd);
}

void _cdecl TStringEdit::OnCloseSFM (GUIControl* sender)
{
	if (newMisWnd)
	{
		eValue->Text = my_val;
	}
}

void _cdecl TStringEdit::OnCloseSFG (GUIControl* sender)
{
/*
	if (newGMXWnd)
	{
		eValue->Text = my_gmx_val;
	}
*/
}

void _cdecl TStringEdit::OnSelectFromFile (GUIControl* sender)
{
	TResourceSelectorWindow* Selector = NEW TResourceSelectorWindow();
	Selector->OnSelect = (CONTROL_EVENT)&TStringEdit::OnSelectFileAndClose;
	Application->ShowModal (Selector);
}


void _cdecl TStringEdit::OnSelectFileAndClose (GUIControl* sender)
{
	TResourceSelectorWindow* sndr = (TResourceSelectorWindow*)sender;

	eValue->Text = sndr->ObjectName;
	
}



void TStringEdit::KeyPressed(int key, bool bSysKey)
{
  if (!bSysKey) return;

	if (key == 13) OnButtonOK (this);
}


