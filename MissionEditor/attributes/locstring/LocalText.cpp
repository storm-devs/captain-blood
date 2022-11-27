#include "localtext.h"
#include "newString.h"


extern IRender * pRS; 
extern ILocStrings* pLocStr;


TLocTextEditor::TLocTextEditor(int posX, int posY) : GUIWindow (NULL, posX, posY, 400, 300)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;

	dword dwWinWidth = pRS->GetScreenInfo3D().dwWidth - 10;
	dword dwWinHeight = pRS->GetScreenInfo3D().dwHeight - 10;

	SetWidth(dwWinWidth);
	SetHeight(dwWinHeight);

	SetScreenCenter();


	listBox = NEW GUIListBox (this, 5, 35, pRS->GetScreenInfo3D().dwWidth-10-15, pRS->GetScreenInfo3D().dwHeight - 10-65);
	listBox->FontColor = 0xFF000000;
	listBox->pFont->SetName ("arialcyrsmall");
	listBox->Items.Clear ();
	listBox->MakeFlat (true);
	listBox->OnMouseDBLClick = (CONTROL_EVENT)&TLocTextEditor::OKPressed;


	btnOK = NEW GUIButton (this, 10, pRS->GetScreenInfo3D().dwHeight - 36, 100, 20);
	btnOK->Glyph->Load ("ok");
	btnOK->Caption = "Select";
	btnOK->FontColor = 0xFF000000;
	btnOK->pFont->SetName ("arialcyrsmall");
	btnOK->OnMousePressed = (CONTROL_EVENT)&TLocTextEditor::OKPressed;
	btnOK->FlatButton = true;

	btnCancel = NEW GUIButton (this, 115, pRS->GetScreenInfo3D().dwHeight - 36, 100, 20);
	btnCancel->Glyph->Load ("cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->FontColor = 0xFF000000;
	btnCancel->pFont->SetName ("arialcyrsmall");
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TLocTextEditor::CancelPressed;
	btnCancel->FlatButton = true;


/*
	addString = NEW GUIButton (this, 10, 5, 100, 20);
	addString->Glyph->Load ("ok");
	addString->Caption = "Add string";
	addString->FontColor = 0xFF000000;
	addString->pFont->SetName ("arialcyrsmall");
	addString->OnMousePressed = (CONTROL_EVENT)&TLocTextEditor::strAdd;
	addString->FlatButton = true;

	delString = NEW GUIButton (this, 120, 5, 100, 20);
	delString->Glyph->Load ("cancel");
	delString->Caption = "Del string";
	delString->FontColor = 0xFF000000;
	delString->pFont->SetName ("arialcyrsmall");
	delString->OnMousePressed = (CONTROL_EVENT)&TLocTextEditor::strDel;
	delString->FlatButton = true;
*/	


	BuildLocString();

}

TLocTextEditor::~TLocTextEditor()
{
}

void TLocTextEditor::BuildLocString()
{
	listBox->Items.Clear();
	dword dwCount = pLocStr->GetStringsCount();

	string itemValue;
	for (dword n = 0; n < dwCount; n++)
	{
		const char* strValue = pLocStr->GetStringByIndex(n);
		long strId = pLocStr->GetIdByIndex(n);

		itemValue = strValue;

		listBox->Items.Add(itemValue);
	}
}



void _cdecl TLocTextEditor::OKPressed (GUIControl* sender)
{
	Close(NULL);
}

void _cdecl TLocTextEditor::CancelPressed (GUIControl* sender)
{
	Close(NULL);
}

void _cdecl TLocTextEditor::strAdd (GUIControl* sender)
{
	TNewLocStringWindow* cWind = NEW TNewLocStringWindow ("Type new string here !");
	cWind->OnClose = (CONTROL_EVENT)&TLocTextEditor::strAddOk;
	Application->ShowModal (cWind);
}

void _cdecl TLocTextEditor::strDel (GUIControl* sender)
{
	GUIMessageBox* mb = Application->MessageBox("You want to delete selected string ?", "Delete warning", GUIMB_YESNO, true);
	mb->OnOK = (CONTROL_EVENT)&TLocTextEditor::strDelConfirm;
}

void _cdecl TLocTextEditor::strDelConfirm (GUIControl* sender)
{
/*
	int n = listBox->SelectedLine;
	if (n < 0) return;
	listBox->Items.Delete(n);

	long id = pLocStr->GetIdByIndex(n);
	pLocStr->DelString(id);
	pLocStr->Save();

	BuildLocString();
*/
}

void _cdecl TLocTextEditor::strAddOk (GUIControl* sender)
{
/*
	TNewLocStringWindow* cWind = (TNewLocStringWindow*)sender;
	if (!cWind->ExitByOK) return;

	string s = cWind->pText->Text;

	pLocStr->AddString(s);
	pLocStr->Save();

	BuildLocString();
*/
}