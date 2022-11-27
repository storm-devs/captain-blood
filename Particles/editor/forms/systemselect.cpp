#ifndef _XBOX

#include "systemselect.h"
#include "..\CustomControls\StrUtil.h"
#include "..\..\..\Common_h\FileService.h"

#define SYSTEMS_PLACE "resource\\particles"

extern bool AlphabetSortFuncCompare (const string &a1, const string &a2);

	
TSystemSelector::TSystemSelector () : GUIWindow (NULL, 0, 0, 250, 400)
{
	OkPressed = false;
	SystemName = "";

	pFS = (IFileService*)api->GetService("FileService");
	pFont->SetName("arialcyrsmall");
	Caption = "Select system";
	SetScreenCenter();

	pSystemsList = NEW GUIListBox(this, 10, 10, 230, 330);
	pSystemsList->MakeFlat(true);
	//pSystemsList->OnChange = (CONTROL_EVENT)OnSelectProject;

	pOKButton = NEW GUIButton (this, 25+10, 345, 80, 22);
	pOKButton->FlatButton = true;
	pOKButton->Caption = "Add";
	pOKButton->OnMouseClick = (CONTROL_EVENT)&TSystemSelector::OnAddSystem;

	pCancelButton = NEW GUIButton (this, 35+100, 345, 80, 22);
	pCancelButton->FlatButton = true;
	pCancelButton->Caption = "Cancel";
	pCancelButton->OnMouseClick = (CONTROL_EVENT)&TSystemSelector::OnCancelSelect;


	BuildSystemsList ();
}


TSystemSelector::~TSystemSelector ()
{
}


void TSystemSelector::Draw ()
{
	GUIWindow::Draw();
}


void TSystemSelector::BuildSystemsList ()
{
	pSystemsList->Items.Clear();

	IFinder* finder = pFS->CreateFinder(SYSTEMS_PLACE, "*.xps", find_all_files_no_mirrors | find_no_files_from_packs, _FL_);

	for (dword i = 0; i < finder->Count(); i++)
	{
		string filename = string(finder->FilePath(i)).GetRelativePath (SYSTEMS_PLACE);
		string name;
		name.GetFileName(filename);

		if (finder->IsMirror(i))
			name = string ("#c0000FF") + name;
		else
			name = string ("#c000000") + name;

		pSystemsList->Items.Add(name);
	}

	finder->Release();


}


void _cdecl TSystemSelector::OnAddSystem ()
{
	OkPressed = true;

	if (pSystemsList->SelectedLine < 0)
	{
		Application->MessageBox("You must select system", "Error", GUIMB_OK);
		return;
	}
	SystemName = pSystemsList->Items[pSystemsList->SelectedLine].GetBuffer()+8;

	Close (this);
}

void _cdecl TSystemSelector::OnCancelSelect ()
{
	OkPressed = false;
	Close (this);
}


#endif