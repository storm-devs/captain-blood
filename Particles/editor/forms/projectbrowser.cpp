#ifndef _XBOX

#include "projectbrowser.h"
#include "mainwindow.h"
#include "systemselect.h"
#include "..\CustomControls\StrUtil.h"
#include "..\EditorMain.h"
#include "..\..\manager\particlemanager.h"
#include "..\..\..\Common_h\FileService.h"

#define PACK_PATH "Resource\\Missions\\"


extern string PakName;
extern ParticleEditor* pEditor;


bool AlphabetSortFuncCompare (const string &a1, const string &a2)
{
	if (a1 > a2) return false;
	return true;
}

	
TProjectBrowser::TProjectBrowser () : GUIWindow (NULL, 0, 0, 550, 600)
{
	//Надо отрелизить все паки !!!! при старте!!!!
	//==========================
	UserSelectSystemToOpen = false;
	bSystemButton = false;
	pFS = (IFileService*)api->GetService("FileService");
	pFont->SetName("arialcyrsmall");
	Caption = "Projects Editor v1.10";
	SetScreenCenter();

	pProjectList = NEW GUIListBox(this, 10, 10, 220, 550);
	pProjectList->MakeFlat(true);
	pProjectList->OnChange = (CONTROL_EVENT)&TProjectBrowser::OnSelectProject;

	pAvailableSystemsList = NEW GUIListBox(this, 245, 70, 290, 490);
	pAvailableSystemsList->MakeFlat(true);
	pAvailableSystemsList->OnMouseDBLClick = (CONTROL_EVENT)&TProjectBrowser::OnSystemDblClick;



	pTextureNameField = NEW GUIEdit(this, 245, 10, 180, 22);
	pTextureNameField->Flat = true;
	pTextureNameField->Hint = "Project texture name";

	pPackName = NEW GUIComboBox(this, 245, 40, 180, 22);
	pPackName->MakeFlat(true);
	pPackName->Edit->Hint = "Pack file name";
	pPackName->ListBox->Items.Add("mission17");
	pPackName->ListBox->Items.Add("mis18");
	pPackName->ListBox->Items.Add("mis19");
	pPackName->bUserCanTypeText = true;
	pPackName->Edit->OnAccept = (CONTROL_EVENT)&TProjectBrowser::OnCreateNewPackInList;
	pPackName->OnSelect = (CONTROL_EVENT)&TProjectBrowser::OnPackIsChange;

	pOpenTexture = NEW GUIButton (this, 427, 10, 22, 22);
	pOpenTexture->FlatButton = true;
	pOpenTexture->Glyph->Load("peditor\\open");
	pOpenTexture->Hint = "Select texture";
	pOpenTexture->OnMouseClick = (CONTROL_EVENT)&TProjectBrowser::OnSelectTexture;


	pNewSystem = NEW GUIButton (this, 457-26, 40, 22, 22);
	pNewSystem->FlatButton = true;
	pNewSystem->Hint = "Create new system in project";
	pNewSystem->Glyph->Load("peditor\\create");

	
	pAddSystem = NEW GUIButton (this, 457+26*0, 40, 22, 22);
	pAddSystem->FlatButton = true;
	pAddSystem->Hint = "Add system to project";
	pAddSystem->Glyph->Load("peditor\\addsystem");
	pAddSystem->OnMouseClick = (CONTROL_EVENT)&TProjectBrowser::OnAddSystem;

	
	pRemoveSystem = NEW GUIButton (this, 457+26*1, 40, 22, 22);
	pRemoveSystem->FlatButton = true;
	pRemoveSystem->Hint = "Remove system from project";
	pRemoveSystem->Glyph->Load("peditor\\delsystem");
	pRemoveSystem->OnMouseClick = (CONTROL_EVENT)&TProjectBrowser::OnRemoveSystem;


	pCloseBrowser = NEW GUIButton (this, 457+26*2, 40, 22, 22);
	pCloseBrowser->FlatButton = true;
	pCloseBrowser->Hint = "Close browser";
	pCloseBrowser->Glyph->Load("peditor\\exit");
	pCloseBrowser->OnMouseClick = (CONTROL_EVENT)&TProjectBrowser::ExitFromBrowser;

	

/*
	pUpdateGeomCache = NEW GUIButton (this, 457+26*2, 40, 22, 22);
	pUpdateGeomCache->FlatButton = true;
	pUpdateGeomCache->Hint = "Refresh geometry cache info";
	pUpdateGeomCache->Glyph->Load("peditor\\refresh");
	pUpdateGeomCache->OnMouseClick = (CONTROL_EVENT)UpdateGeomCacheInfo;
	*/
	

	BuildProjectList ();
}


TProjectBrowser::~TProjectBrowser ()
{
//	pFS->ReleasePak(PakName);
}


void TProjectBrowser::Draw ()
{
	GUIWindow::Draw();
}

void TProjectBrowser::BuildProjectList ()
{
	pProjectList->Items.Clear();

	IFinder* finder = pFS->CreateFinder(PARTICLES_PLACE, "*.prj", find_all_files_no_mirrors | find_no_files_from_packs, _FL_);

	for (dword i = 0; i < finder->Count(); i++)
	{
		string filename = string(finder->FilePath(i)).GetRelativePath (PARTICLES_PLACE);
		string name;
		name.GetFileName(filename);

		if (finder->IsMirror(i))
			name = string ("#c0000FF") + name;
		else
			name = string ("#c000000") + name;

		pProjectList->Items.Add(name);
	}


	finder->Release();


	string cProjectName = "FIXME!";//pEditor->Manager()->GetProjectFileName();
	string ShortProjectName;
	ShortProjectName.GetFileName(cProjectName);

	for (int n = 0; n < pProjectList->Items.Size(); n++)
	{
		const char* pItemName = pProjectList->Items[n].GetBuffer() + 8;
		if (crt_stricmp (pItemName, ShortProjectName) == 0)
		{
			pProjectList->SelectedLine = n;
			//pProjectList->SelectedLine
			break;
		}
	}



	//pProjectList->SelectedLine


	OnSelectProject (NULL);

}


void _cdecl TProjectBrowser::OnSelectProject (GUIControl* sender)
{
	pAvailableSystemsList->Items.Clear();
	if (pProjectList->SelectedLine < 0) return;
	string ProjectName = pProjectList->Items[pProjectList->SelectedLine];

	string LongFileName = "resource\\particles\\";
	LongFileName += (const char*)(ProjectName.GetBuffer()+8);
	LongFileName.AddExtention(".prj");

	CurrentProjectName = LongFileName;
	CurrentProjectNameShort = (ProjectName.GetBuffer()+8);

	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IIniFile* IniFile = pFS->OpenIniFile(LongFileName.GetBuffer(), _FL_);
	Assert (IniFile);

	pTextureNameField->Text = IniFile->GetString("Textures", "MainTexture", "none");

	//Загружаем данные 
	for (int n = 0; n < 9999; n++)
	{
		string Section;
		Section.Format("System_%04d", n);


		const char* pSysName = IniFile->GetString("Manager", Section.GetBuffer(), NULL);
		if (!pSysName) break;
		string SystemName = pSysName;

		string LongFileName = "resource\\particles\\";
		LongFileName+=SystemName;
		LongFileName.AddExtention(".xps");

		if (pFS->IsExist(LongFileName))
			SystemName = string ("#c000000") + SystemName;
		else
			SystemName = string ("#cFF0000") + SystemName;

		pAvailableSystemsList->Items.Add(SystemName);
	}


	pAvailableSystemsList->Items.Sort(AlphabetSortFuncCompare);


	IniFile->Release();
}

void _cdecl TProjectBrowser::OnCreateNewPackInList (GUIControl* sender)
{
	string PackName = pPackName->Edit->Text;

	for (int n = 0; n < pPackName->ListBox->Items.Size(); n++)
	{
		if (pPackName->ListBox->Items[n] == PackName)
		{
			return;
		}
	}

	int NewIndex = pPackName->ListBox->Items.Add(PackName);

	pPackName->SelectItem(NewIndex);
}

void _cdecl TProjectBrowser::OnPackIsChange (GUIControl* sender)
{
//	pFS->ReleasePak(PakName);

	string PakFileName = PACK_PATH;
	PakFileName += pPackName->ListBox->Items[pPackName->ListBox->SelectedLine];

//	pFS->AddPak(PakFileName);

	PakName = PakFileName;

	BuildProjectList ();
}



void _cdecl TProjectBrowser::OnAddSystem (GUIControl* sender)
{
	TSystemSelector* pSystemSelector = NEW TSystemSelector;
	Application->ShowModal(pSystemSelector);
	pSystemSelector->OnClose = (CONTROL_EVENT)&TProjectBrowser::AddSystemClosed;
}

void _cdecl TProjectBrowser::OnSelectTexture (GUIControl* sender)
{
	GUIFileOpen* TextureSelector = NEW GUIFileOpen;
	TextureSelector->Caption = "Select project texture";
	TextureSelector->StartDirectory = string(pEditor->GetStartDir()) + "\\resource\\textures\\";
	TextureSelector->Filters.Add(".txx");
	TextureSelector->FiltersDesc.Add("Texture file");
	TextureSelector->OnOK = (CONTROL_EVENT)&TProjectBrowser::TextureSelected;
	Application->ShowModal(TextureSelector);
	TextureSelector->Filter->SelectItem(1);
}

void _cdecl TProjectBrowser::TextureSelected (GUIControl* sender)
{
	if (CurrentProjectName.IsEmpty()) return;

	GUIFileOpen* TextureFileSelector = (GUIFileOpen*)sender;
	string CropPath = string(pEditor->GetStartDir()) + "\\resource\\textures\\";
	string TextureName = TextureFileSelector->FileName;
	TextureName.GetRelativePath(CropPath);

	//pEditor->Manager()->SetProjectTexture(TextureName);
	//pPS->SetProjectTextureName(tName);


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IEditableIniFile * IniFile = pFS->OpenEditableIniFile(CurrentProjectName.GetBuffer(), file_open_always, _FL_);
	Assert(IniFile);

	IniFile->SetString("Textures", "MainTexture", TextureName.GetBuffer());
	IniFile->Release();

	OnSelectProject (NULL);
}

void _cdecl TProjectBrowser::AddSystemClosed (GUIControl* sender)
{
	if (CurrentProjectName.IsEmpty()) return;
	TSystemSelector* pSystemSelector = (TSystemSelector*)sender;
	if (!pSystemSelector->OkPressed) return;

	//pSystemSelector->SystemName


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IEditableIniFile* IniFile = pFS->OpenEditableIniFile(CurrentProjectName.GetBuffer(), file_open_always, _FL_);
	Assert (IniFile);

	string Section;
	//Загружаем данные 
	for (int n = 0; n < 9999; n++)
	{
		Section.Format("System_%04d", n);
		const char* szIniStr = IniFile->GetString("Manager", Section.GetBuffer(), NULL);

		if (!szIniStr) break;

		//system already exist in project...
		if (crt_stricmp (szIniStr, pSystemSelector->SystemName.GetBuffer()) == 0)
		{
			IniFile->Release();
			return;
		}

	}

	IniFile->SetString("Manager", (char*)Section.GetBuffer(), pSystemSelector->SystemName.GetBuffer());

	IniFile->Release();

	OnSelectProject (NULL);
}

void _cdecl TProjectBrowser::OnRemoveSystem (GUIControl* sender)
{
	if (CurrentProjectName.IsEmpty()) return;
	if (pAvailableSystemsList->SelectedLine < 0) return;
	pAvailableSystemsList->Items.Delete(pAvailableSystemsList->SelectedLine);


	IEditableIniFile* IniFile = pFS->OpenEditableIniFile(CurrentProjectName.GetBuffer(), file_open_always, _FL_);
	Assert (IniFile);

	int n = 0;
	string Section;
	for (n = 0; n < pAvailableSystemsList->Items.Size(); n++)
	{
		char* pSystemName = (char*)pAvailableSystemsList->Items[n].GetBuffer() + 8; 
		Section.Format("System_%04d", n);
		IniFile->SetString("Manager", (char*)Section.GetBuffer(), pSystemName);
	}

	for (DWORD m = n; m < 9999; m++)
	{
		Section.Format("System_%04d", m);
		if (!IniFile->IsKeyCreated("Manager", (char*)Section.GetBuffer())) break;
		IniFile->DelKey("Manager", (char*)Section.GetBuffer());
	}

	IniFile->Release();

	OnSelectProject (NULL);
}

/*
void _cdecl TProjectBrowser::UpdateGeomCacheInfo (GUIControl* sender)
{
	if (CurrentProjectName.IsEmpty()) return;
	GUIMessageBox* pMB = Application->MessageBox("This action close all edited system\nYou sure ?", "Warning", GUIMB_YESNO);
	pMB->OnOK = (CONTROL_EVENT)UpdateGeomCacheInfoReal;
}

void _cdecl TProjectBrowser::UpdateGeomCacheInfoReal (GUIControl* sender)
{
	if (CurrentProjectName.IsEmpty()) return;

	UsedGeomNames.DelAll();

	string OldProjectName = pEditor->Manager()->GetProjectFileName();
	pEditor->Manager()->OpenProject(CurrentProjectNameShort.GetBuffer());

	INIFILE* IniFile = pFS->OpenIniFile(CurrentProjectName.GetBuffer(), _FL_);
	Assert (IniFile);


	DWORD GeomIndex = 0;
	string Section;
	for (int n = 0; n < pAvailableSystemsList->Items.GetCount(); n++)
	{
		char* pSystemName = (char*)pAvailableSystemsList->Items[n].GetBuffer() + 8; 

		const char* GeomName = NULL;
    GeomName = pEditor->Manager()->GetFirstGeomName(pSystemName);

		while (GeomName)
		{
			bool DuplicateFound = false;
			for (DWORD x = 0; x < UsedGeomNames.Size(); x++)
			{
				if (crt_stricmp(UsedGeomNames[x].GetBuffer(), GeomName) == 0)
				{
					DuplicateFound = true;
					break;
				}
			}

			if (!DuplicateFound)
			{
				UsedGeomNames.Add(GeomName);
				string Section;
				Section.Format("Model_%04d", GeomIndex);
				IniFile->WriteString("ModelsCache", (char*)Section.GetBuffer(), (char*)GeomName);
				GeomIndex++;
			}

			GeomName = pEditor->Manager()->GetNextGeomName();
		}
	}


	for (DWORD m = GeomIndex; m < 9999; m++)
	{
		Section.Format("Model_%04d", m);
		if (!IniFile->TestKey("ModelsCache", (char*)Section.GetBuffer(), NULL)) break;
		IniFile->DeleteKey("ModelsCache", (char*)Section.GetBuffer());
	}


	delete IniFile;
	pEditor->Manager()->OpenProject(OldProjectName);
}
*/

void _cdecl TProjectBrowser::OnSystemDblClick (GUIControl* sender)
{
	if (pAvailableSystemsList->SelectedLine < 0) return;
	CurrentSystemName = (pAvailableSystemsList->Items[pAvailableSystemsList->SelectedLine].GetBuffer() + 8);
	UserSelectSystemToOpen = true;
	Close (this);
}

void _cdecl TProjectBrowser::ExitFromBrowser (GUIControl* sender)
{
	UserSelectSystemToOpen = false;
	Close (this);
}

#endif