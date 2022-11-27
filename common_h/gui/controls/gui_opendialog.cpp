#ifndef _XBOX

#include "gui_opendialog.h"


//  GUIButton* btnOK; 
//	GUIButton* btnCancel; 
#define WINDOW_POSX   100
#define WINDOW_POSY   100
#define WINDOW_WIDTH  580
#define WINDOW_HEIGHT 395

bool UserChangeDrive = true;

GUIFileOpen::GUIFileOpen () : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bShowReadOnly = false;
	bSystemButton = false;
	t_OnOK = NEW GUIEventHandler;
	t_OnCancel = NEW GUIEventHandler;
	t_OnChangeExtension = NEW GUIEventHandler;


 bAlwaysOnTop = true;
 StartFromDrive = "c:\\";
 StartFromDrive.Upper ();
 CurrentDrive = StartFromDrive;
 Caption = "Open file";


	SetScreenCenter ();
//	Caption = "Open file";

	Filters.Add (".*");
	FiltersDesc.Add ("All files");


	this->pFont->SetName("arialcyrsmall");

	btnOK = NEW GUIButton (this, 395+70, 270+38, 95, 22); 
	btnOK->Caption = " Open ";
	btnOK->pFont->SetName("arialcyrsmall");
	btnOK->OnMousePressed = (CONTROL_EVENT)&GUIFileOpen::btnOKPressed;

	


	btnCancel = NEW GUIButton (this, 395+70, 310+25, 95, 22); 
	btnCancel->Caption = " Cancel ";
	btnCancel->pFont->SetName("arialcyrsmall");
	btnCancel->OnMousePressed = (CONTROL_EVENT)&GUIFileOpen::btnCancelPressed;


	edtFileName = NEW GUIEdit (this, 10, 278+32, 410, 18);
	edtFileName->Text = "";
	edtFileName->pFont->SetName("arialcyrsmall");
	edtFileName->OnChange = (CONTROL_EVENT)&GUIFileOpen::filenameChanged;

	Filter = NEW GUIComboBox (this, 10, 318+20, 410, 18);

	for (int n = 0; n < Filters.Size (); n++)
	{
		string tmp = Filters.Get(n);
		if ((n >= 0) && (n < FiltersDesc.Size()))
		{
			string t2 = string (" (") + FiltersDesc[n] + string (")");
			tmp = string ("*") + tmp + t2;
		}

		if (n == 0) Filter->Edit->Text = tmp;
		Filter->ListBox->Items.Add (tmp);
	}

	
	Filter->ListBox->pFont->SetName("arialcyrsmall");
	Filter->Edit->pFont->SetName("arialcyrsmall");
	Filter->OnChange = (CONTROL_EVENT)&GUIFileOpen::FilterChanged;

	DirList = NEW GUITreeView (this, 10, 10, 270, 260);
	DirList->pFont->SetName("arialcyrsmall");
	DirList->SetImagesArray ("gtree");
	DirList->OnExpand = (CONTROL_EVENT)&GUIFileOpen::TreeExpand;
	DirList->OnColapse = (CONTROL_EVENT)&GUIFileOpen::TreeColapse;
	DirList->OnChange = (CONTROL_EVENT)&GUIFileOpen::TreeSelectionChange;
	DirList->OnBeforeHintShow = (CONTROL_EVENT)&GUIFileOpen::onDirBeforeHintShow;
	DirList->Hint = "XXX";

	


	Files = NEW GUIListBox (this, 290, 10, 276, 290);
	Files->pFont->SetName("arialcyrsmall");
	Files->OnChange = (CONTROL_EVENT)&GUIFileOpen::FileSelectionChange;
	Files->OnMouseDBLClick = (CONTROL_EVENT)&GUIFileOpen::btnOKPressed;
	Files->OnBeforeHintShow = (CONTROL_EVENT)&GUIFileOpen::onBeforeHintShow;
	Files->Hint = "XXX";

	

	Drives = NEW GUIComboBox (this, 10, 280, 270, 18);

	Drives->Edit->pFont->SetName("arialcyrsmall");
	Drives->ListBox->pFont->SetName("arialcyrsmall");

	CurrentDrivesPaths.Clear ();
	//Собираем диски
	char FinalStr[8192];
	char buf[4096];
	char volname[4096];
	char systype[4096];
	GetLogicalDriveStrings(4096, buf);
	buf[4095] = 0;
	const char * cr = buf;
	
	while(*cr)
	{
		DWORD maxComLen;
		DWORD SysFlag;
		memset (volname, 0, 4096);
		memset (systype, 0, 4096);

		if ((cr[0] != 'A') && (cr[0] != 'a') &&
			  (cr[0] != 'B') && (cr[0] != 'b'))
		{
			GetVolumeInformation (cr, volname, 4096, NULL, &maxComLen, &SysFlag, systype, 4096);
		}
		

		
	
		//CurrentDrivesPaths.Add ();

		crt_snprintf (FinalStr, 8191, "%s [%s] [%s]", cr, volname, systype);
		

		if (string::IsEqual (cr, StartFromDrive.GetBuffer()))
		{
			
			Drives->Edit->Text = FinalStr;
			switch(GetDriveType(cr))
			{
			case DRIVE_REMOVABLE:
				Drives->Edit->Image.Load ("fdd");
				break;
			case DRIVE_CDROM:
				Drives->Edit->Image.Load ("cd");
				break;
			case DRIVE_REMOTE:
				Drives->Edit->Image.Load ("net");
				break;
			default:
				Drives->Edit->Image.Load ("hdd");
			}

		}


		CurrentDrivesPaths.Add ("");
		Drives->ListBox->Items.Add (FinalStr);

		GUIImage* img1 = &Drives->ListBox->Images[Drives->ListBox->Images.Add ()];
		switch(GetDriveType(cr))
		{
		case DRIVE_REMOVABLE:
			img1->Load ("fdd");
			break;
		case DRIVE_CDROM:
			img1->Load ("cd");
			break;
		case DRIVE_REMOTE:
			img1->Load ("net");
			break;
		default:
			img1->Load ("hdd");
		}
		cr += strlen(cr) + 1;
	 }


	Drives->OnChange = (CONTROL_EVENT)&GUIFileOpen::DriveIsChange;

	//SelectItem
	UserChangeDrive = false;
	DriveIsChange ();

//	OnCreate = (CONTROL_EVENT);


//	SetCurrentdirectory ();



}

void GUIFileOpen::OnCreate ()
{
	if (StartDirectory.IsEmpty())
		SetCurrentdirectory ();
			else
		SetCurrentdirectory (StartDirectory.GetBuffer());
	RefreshFilters();
	TreeSelectionChange ();
}

GUIFileOpen::~GUIFileOpen ()
{
	delete t_OnChangeExtension;
	delete t_OnOK;
	delete t_OnCancel;


	delete Drives;
	delete Files;
	delete DirList;
	delete Filter;
	delete edtFileName;
	delete btnOK;
	delete btnCancel;
}
 

void _cdecl GUIFileOpen::DriveIsChange ()
{
	if (UserChangeDrive)
	{
		GUITreeNode* gtnode = DirList->GetSelectedNode ();
		if (gtnode)
		{
			const char* dir_path = GetFullNodeName (gtnode);
			string f_path = CurrentDrive + string (dir_path);

			int d_num = GetDriveNum (CurrentDrive);

			if (d_num != -1)
			{
				CurrentDrivesPaths[d_num] = f_path;
			}
		}


	}
	
	DirList->Items->Clear ();
	DirList->sbVert->Position = 0;
	DirList->sbHoriz->Position = 0;
	DirList->sbVertChange (this);
	DirList->sbHorizChange (this);

	string temp = Drives->Edit->Text;
	temp.Delete (3, 10000);

	CurrentDrive = temp;

	if (UserChangeDrive)
	{
		int drive_num = GetDriveNum (CurrentDrive);
		if (drive_num != -1)
		{
			if (!CurrentDrivesPaths[drive_num].IsEmpty())
			{
				SetDirectory (CurrentDrivesPaths[drive_num]);
			}
		}
	}

	//CurrentDrivesPaths.Add ("");
	UserChangeDrive = true;
	RefreshDirs (CurrentDrive);
}

void GUIFileOpen::RefreshDirs (const string& str)
{
	//Ставим текущий путь
	//SetCurrentDirectory(str.GetBuffer());

	GUIStringList lstFiles;
	GUIStringList lstDirs;

	string tstr = str;
	GetFilesAndDirs (tstr, lstFiles, lstDirs);


	for (int n = 0; n < lstDirs.Size (); n++)
	{
		GUITreeNode* treenode = NEW GUITreeNode;
		treenode->SetText(lstDirs[n]);
 		treenode->Image->Load ("folder");
		treenode->ExpandedImage->Load ("ofolder");
		treenode->SelectedImage->Load ("ofolder");
		DirList->Items->Add (treenode);

		GUIStringList sublstFiles;
		GUIStringList sublstDirs;
		GetFilesAndDirs ((tstr+lstDirs[n]+string("\\")), sublstFiles, sublstDirs, "*.*", true);

		if (treenode->Childs.GetCount() <= 0)
		{
			for (int m = 0; m < sublstDirs.Size (); m++)
			{
				GUITreeNode* subtreenode = NEW GUITreeNode;
				subtreenode->SetText(sublstDirs[m]);
 				subtreenode->Image->Load ("folder");
				subtreenode->ExpandedImage->Load ("ofolder");
				subtreenode->SelectedImage->Load ("ofolder");
				
				treenode->Childs.Add (subtreenode);
			}
		}
	}

}

void GUIFileOpen::GetFilesAndDirs (string& path, GUIStringList& files, GUIStringList& dirs, const char* Mask, bool ExitOnFolder)
{
	files.Clear ();
	dirs.Clear ();

	string tmpMask = path + Mask;
	WIN32_FIND_DATA findData;
	memset(&findData, 0, sizeof(findData));
	HANDLE hdl = FindFirstFile(tmpMask.GetBuffer(), &findData);
	if(hdl == INVALID_HANDLE_VALUE) return;

	string fullpath;
	do
	{
		if (strcmp(findData.cFileName, ".") == 0) continue;
		if (strcmp(findData.cFileName, "..") == 0) continue;
		if(!string::IsFileMask(findData.cFileName, Mask))
		{
			continue;
		}
		
		fullpath = path;
		fullpath += findData.cFileName;

		

		//
		DWORD attr = GetFileAttributes(fullpath.GetBuffer ());
		if(attr == 0xffffffff) continue;
		if(attr & FILE_ATTRIBUTE_SYSTEM) continue;
		if(attr & FILE_ATTRIBUTE_HIDDEN) continue;

		if (!bShowReadOnly)
		{
			if(attr & FILE_ATTRIBUTE_READONLY) continue;
		}
		


		if(attr & FILE_ATTRIBUTE_DIRECTORY)
		{
			dirs.Add (findData.cFileName);
			if (ExitOnFolder) return;
		} else
			{
				files.Add (findData.cFileName);
			}

	} while(FindNextFile(hdl, &findData));

	FindClose(hdl);



		 

}



void _cdecl GUIFileOpen::TreeColapse ()
{
	GUITreeNode* gtnode = DirList->GetExpandedOrCollapsedNode ();
}

void _cdecl GUIFileOpen::TreeExpand ()
{
	GUITreeNode* gtnode = DirList->GetExpandedOrCollapsedNode ();

	GUIStringList lstFiles;
	GUIStringList lstDirs;

	//TODO : Надо из узла получить полный путь...
	const char* fullpath = GetFullNodeName (gtnode);
	string ScanPath = CurrentDrive + string (fullpath);


	GetFilesAndDirs (ScanPath, lstFiles, lstDirs);

	for (int n = 0; n < lstDirs.Size(); n++)
	{
		GUITreeNode* treenode = NULL;
		for (int z = 0; z < gtnode->Childs.GetCount(); z++)
		{
			GUITreeNode* child = gtnode->Childs.Get (z);
			if (string::IsEqual(child->GetText(), lstDirs[n].GetBuffer())) 
			{
				treenode = child;
				break;
			}
		}

		// Такой папки нет, надо добавить...
		if (z >= gtnode->Childs.GetCount())
		{
			treenode = NEW GUITreeNode;
			treenode->SetText(lstDirs[n]);
// 			treenode->Text = ;
 			treenode->Image->Load ("folder");
			treenode->ExpandedImage->Load ("ofolder");
			treenode->SelectedImage->Load ("ofolder");

			
			gtnode->Childs.Add (treenode);
		}

		//TODO :-------- Надо просканить на предмет хотя бы одной подпапки
		GUIStringList sublstFiles;
		GUIStringList sublstDirs;
		GetFilesAndDirs ((ScanPath + lstDirs[n]+string("\\")), sublstFiles, sublstDirs, "*.*", true);

		if (treenode->Childs.GetCount() <= 0)
		{
			for (int m = 0; m < sublstDirs.Size (); m++)
			{
				GUITreeNode* subtreenode = NEW GUITreeNode;
				subtreenode->SetText(sublstDirs[m]);
 				subtreenode->Image->Load ("folder");
				subtreenode->ExpandedImage->Load ("ofolder");
				subtreenode->SelectedImage->Load ("ofolder");
				
				treenode->Childs.Add (subtreenode);
			}
		}
//-----------------------

	}
}


char TempArray[MAX_PATH];
char TempArray2[MAX_PATH];

void GUIFileOpen::GetFullNodeProcess (GUITreeNode* node)
{
	crt_strncpy (TempArray2, MAX_PATH, TempArray, MAX_PATH - 1);
	const char* lpName = node->GetText();
	crt_snprintf (TempArray, MAX_PATH, "%s\\%s", lpName, TempArray2);

	GUITreeNode* Parent = node->Parent;
	if (Parent) GetFullNodeProcess (Parent);
}


const char* GUIFileOpen::GetFullNodeName (GUITreeNode* node)
{
	memset (TempArray, 0, MAX_PATH);
	if (!node) return TempArray;
	GetFullNodeProcess (node);
	int l = strlen (TempArray);
	//TempArray[l-1] = 0;
	return TempArray;
}



void _cdecl GUIFileOpen::TreeSelectionChange ()
{
	GUITreeNode* gtnode = DirList->GetSelectedNode ();
	const char* fullpath = GetFullNodeName (gtnode);
	string ScanPath = CurrentDrive + string (fullpath);

	GUIStringList lstFiles;
	GUIStringList lstDirs;

	string mask = string ("*") + string (GetCurrentMask ());
	GetFilesAndDirs (ScanPath, lstFiles, lstDirs, mask.GetBuffer());

	Files->Items.Clear ();

	for (int n = 0; n < lstFiles.Size(); n++)
	{
		Files->Items.Add (lstFiles[n]);
	}


}

const char* GUIFileOpen::GetCurrentMask ()
{
	if (Filter->ListBox->SelectedLine < 0) return "*.*";
	return Filters[Filter->ListBox->SelectedLine].GetBuffer();
}

void _cdecl GUIFileOpen::FileSelectionChange ()
{
	GUITreeNode* gtnode = DirList->GetSelectedNode ();
	const char* fullpath = GetFullNodeName (gtnode);
	FileName = CurrentDrive + string (fullpath);
	if (Files->SelectedLine == -1) return;

	FileName = FileName + Files->Items[Files->SelectedLine];
	//FileName->Text = FilePath;
	edtFileName->Text = Files->Items[Files->SelectedLine];
}


void GUIFileOpen::SetDirectory (const char* FullPath)
{
	char drive[10];
	char path [MAX_PATH];

	memset (drive, 0, 10);
	crt_strncpy (drive, 10, FullPath, 3);
	crt_strncpy (path, MAX_PATH, FullPath+2, MAX_PATH - 1);
	

	int n = GetDriveNum(drive);
	if (n == -1) return;

//	Drives->Edit->Text = Drives->ListBox->Items[n]

	UserChangeDrive = false;
	Drives->SelectItem (n);

	DirList->OnFind = (CONTROL_EVENT)&GUIFileOpen::WhenFindItem;

	char TempPath[MAX_PATH];
	crt_snprintf (TempPath, MAX_PATH, "%s\\", path);
	char* lpName = TempPath;
	if (lpName[0] == '\\') lpName++;

	char* lpNameLong = lpName;


	int l = strlen (lpName);
		
	for (int q=0; q < l; q++)
	{
		if (lpName[q] == '\\')
		{
			BYTE saved = lpName[q];
			lpName[q] = 0;
			DirList->FindItem (lpNameLong);
			lpName[q] = saved;
		}
	}
}

void _cdecl GUIFileOpen::SetCurrentdirectory (const char* dir)
{
	//char cur_dir[MAX_PATH];
	//GetCurrentDirectory (MAX_PATH, cur_dir);

	if (dir)
		SetDirectory (dir);
	else
 	  SetDirectory (Application->GetCurrentDir ());
}

void _cdecl GUIFileOpen::WhenFindItem (GUIControl* item)
{
	GUITreeNode* node = (GUITreeNode*)item;
	node->Expanded = true;
	DirList->SetExpandedOrCollapsedNode (node);
	DirList->SetSelectedNode (node);
}

int GUIFileOpen::GetDriveNum (const char* Drive)
{
	for (int n = 0; n < Drives->ListBox->Items.Size (); n++)
	{
		string temp = Drives->ListBox->Items[n];
		temp.Delete (3, 10000);
		if (string::IsEqual(Drive, temp.GetBuffer()))
		{
			return n;
		}
	}

	return -1;
}


//

void _cdecl GUIFileOpen::btnOKPressed ()
{
	if (edtFileName->Text.IsEmpty()) return;
	//if (FileName)
// Меняем текущую директорию....
	char drive[MAX_PATH];
	char dir[MAX_PATH];
	char file[MAX_PATH];
	char ext[MAX_PATH];
	crt_splitpath (FileName.GetBuffer(), drive, MAX_PATH, dir, MAX_PATH, file, MAX_PATH, ext, MAX_PATH);
	char OnlyPath[MAX_PATH];
	crt_snprintf (OnlyPath, MAX_PATH, "%s%s", drive, dir);
  Application->SetCurrentDir (OnlyPath);
// Меняем текущую директорию....

	t_OnOK->Execute (this);
	Close (this);
}

void _cdecl GUIFileOpen::btnCancelPressed ()
{
	FileName = "";
	t_OnCancel->Execute (this);
	Close (this);
}



void GUIFileOpen::RefreshFilters ()
{
	if (GetMainControl()->Application == NULL) return;
	Filter->ListBox->Items.Clear ();

	for (int n = 0; n < Filters.Size (); n++)
	{
		string tmp = Filters.Get(n);
		if ((n >= 0) && (n < FiltersDesc.Size()))
		{
			string t2 = string (" (") + FiltersDesc[n] + string (")");
			tmp = string ("*") + tmp + t2;
		}

		if (n == 0) Filter->Edit->Text = tmp;
		Filter->ListBox->Items.Add (tmp);
	}

	if(Filter->ListBox->Items.Size() > 0)
	{
		Filter->ListBox->SelectedLine = 0;
	}

}


void _cdecl GUIFileOpen::filenameChanged (GUIControl* item)
{
GUITreeNode* gtnode = DirList->GetSelectedNode ();
const char* fullpath = GetFullNodeName (gtnode);
FileName = CurrentDrive + string (fullpath);

FileName = FileName + edtFileName->Text;
}

void _cdecl GUIFileOpen::FilterChanged (GUIControl* item)
{
	TreeSelectionChange ();
	t_OnChangeExtension->Execute(this);
}

void GUIFileOpen::MakeFlat (bool Flat)
{
	btnOK->FlatButton = Flat;
	btnCancel->FlatButton = Flat;
	DirList->MakeFlat(Flat);
	Filter->MakeFlat(Flat);
	Files->MakeFlat(Flat);
	Drives->MakeFlat(Flat);
	edtFileName->Flat = Flat;
}

bool GUIFileOpen::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	/*
	if (message == GUIMSG_LMB_DBLCLICK)
	{
		btnOKPressed ();
	}
*/
	return GUIWindow::ProcessMessages( message,  lparam,  hparam);

}

void GUIFileOpen::KeyPressed(int Key, bool bSysKey)
{
  if (!IsActive) return;
  if (!bSysKey) return;

	if (Key == VK_ESCAPE)
	{
		btnCancelPressed ();
	}
	if (Key == 13)
	{
		btnOKPressed ();
	}

}


void _cdecl GUIFileOpen::onBeforeHintShow (GUIControl* item)
{
	int num = Files->GetElementNumAtPos(Application->GetCursor()->GetPosition());
	if (num == -1)
	{
		Files->Hint = " ";
		Files->OnHintShow_RetValue = false;
	} else
		{
			if (Files->Hint != " ")
			{
				if (Files->Items[num] != Files->Hint)
				{
					Files->OnHintShow_RetValue = false;
				}
			}
			Files->Hint = Files->Items[num];
		}
}

void _cdecl GUIFileOpen::onDirBeforeHintShow (GUIControl* item)
{
	GUITreeNode* tn = DirList->GetNodeAtPos(Application->GetCursor()->GetPosition());
	if (!tn)
	{
		DirList->Hint = " ";
		DirList->OnHintShow_RetValue = false;
	} else
		{
			if (DirList->Hint != " ")
			{
				if (DirList->Hint != tn->GetText())
				{
					DirList->OnHintShow_RetValue = false;
				}
			}
			DirList->Hint = tn->GetText();
		}
}

#endif