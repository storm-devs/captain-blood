#include "importmis.h"

#include "..\forms\mainwindow.h"
#include "..\forms\globalParams.h"


extern IRender * pRS; 
extern MissionEditor* sMission;
extern TMainWindow* MainWindow;


TImportedMistEditor::TImportedMistEditor(int posX, int posY) : GUIWindow (NULL, posX, posY, 400, 300)
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
	listBox->OnMouseDBLClick = (CONTROL_EVENT)&TImportedMistEditor::OKPressed;


	btnOK = NEW GUIButton (this, 10, pRS->GetScreenInfo3D().dwHeight - 36, 100, 20);
	btnOK->Glyph->Load ("ok");
	btnOK->Caption = "Add";
	btnOK->FontColor = 0xFF000000;
	btnOK->pFont->SetName ("arialcyrsmall");
	btnOK->OnMousePressed = (CONTROL_EVENT)&TImportedMistEditor::OKPressed;
	btnOK->FlatButton = true;

	btnCancel = NEW GUIButton (this, 115, pRS->GetScreenInfo3D().dwHeight - 36, 100, 20);
	btnCancel->Glyph->Load ("cancel");
	btnCancel->Caption = "Remove";
	btnCancel->FontColor = 0xFF000000;
	btnCancel->pFont->SetName ("arialcyrsmall");
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TImportedMistEditor::CancelPressed;
	btnCancel->FlatButton = true;

	btnCancel = NEW GUIButton (this, 115 + 105, pRS->GetScreenInfo3D().dwHeight - 36, 200, 20);
	//btnCancel->Glyph->Load ("cancel");
	btnCancel->Caption = "Close & Update mission";
	btnCancel->FontColor = 0xFF000000;
	btnCancel->pFont->SetName ("arialcyrsmall");
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TImportedMistEditor::ClosePressed;
	btnCancel->FlatButton = true;

	array<string> & list = sMission->GetImportList();

	for (dword i = 0; i < list.Size(); i++)
	{
		const char* mName = list[i].c_str();
		listBox->Items.Add(mName);
	}


}

TImportedMistEditor::~TImportedMistEditor()
{
}



void _cdecl TImportedMistEditor::OKPressed (GUIControl* sender)
{
	//Close(NULL);
	//тут добавить новую миссию в список...

	GUIFileOpen* fo = NEW GUIFileOpen ();

	fo->Filters.Add (".xmlz");
	fo->FiltersDesc.Add ("Mission zip (Source)");
	fo->Filters.Add (".xml");
	fo->FiltersDesc.Add ("Mission (Source)");

	fo->OnOK = (CONTROL_EVENT)&TImportedMistEditor::NewMissionSelected;
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);

}

void _cdecl TImportedMistEditor::CancelPressed (GUIControl* sender)
{
	//Close(NULL);
	//тут удалить миссию из списка...

	array<string> & list = sMission->GetImportList();

	int n = listBox->SelectedLine;
	if (n < 0) return;
	list.Extract(n);


	listBox->Items.Clear();

	for (dword i = 0; i < list.Size(); i++)
	{
		const char* mName = list[i].c_str();
		listBox->Items.Add(mName);
	}
}

void _cdecl TImportedMistEditor::ClosePressed (GUIControl* sender)
{
	MainWindow->RefreshImportedMissions();

	Close(NULL);
}



void _cdecl TImportedMistEditor::NewMissionSelected (GUIControl* sender)
{
	GUIFileOpen* dialog = (GUIFileOpen*)sender;



	

	string misName = dialog->FileName.GetRelativePath(gp->StartDirectory);

	listBox->Items.Add(misName);

	array<string> & list = sMission->GetImportList();
	list.Add(misName);
}