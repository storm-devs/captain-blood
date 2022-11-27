
//****************************************************************

#include "newobject.h"
#include "mainwindow.h"
#include "..\attributes\AttributeList.h"



#define WINDOW_POSX   100
#define WINDOW_POSY   100
#define WINDOW_WIDTH  512
#define WINDOW_HEIGHT 384


#define DEFAULT_GROUP_NAME "Default"

extern IMission* miss;
extern MissionEditor* sMission;
extern TMainWindow* MainWindow;

extern TreeNodesPool* globalNodesPool;

bool bCanCreate = false;

MissionEditor::tAvailableMO* cSelectedMissionTemplate = NULL;

char Temp8K[8192];
char CutName[8192];

bool ExistInMission (const char* name)
{
	return miss->FindObject(ConstString(name), MOSafePointer());
	/*
	MGIterator& iterator = miss->GroupIterator (MG_OBJECTS, _FL_);
	for (;!iterator.IsDone();iterator.Next())
	{
		MOSafePointer mo = iterator.Get ();
		const char* mName = mo->GetObjectID ();
		if (crt_stricmp (mName, name) == 0) return true;
	}
	iterator.Release ();

	return false;
	*/
}

char* MakeUniqueName (const char* Name)
{
	// Сохраняем во временный буффер
	crt_snprintf (Temp8K, ARRSIZE(Temp8K), "%s", Name);
	if (!ExistInMission(Temp8K)) return Temp8K;

	int slen = strlen (Temp8K);
	for (int n = 0; n < slen; n++)
	{
		char cur_char = Temp8K[slen-n-1];
		if ((cur_char < '0') || (cur_char > '9')) break;
	}
	// Выбираем номер :)
	int unique_num = 0;

	if (n > 0)
	{
		static char Temp8K_Digit[8192];
		crt_strcpy (Temp8K_Digit, 8191, Temp8K + (slen-n));
		Temp8K [slen-n] = 0;
		unique_num = atoi (Temp8K_Digit);
		unique_num++;
	}

	crt_strncpy (CutName, 8191, Temp8K, 8192);
	if (n > 0)	crt_snprintf (Temp8K, ARRSIZE(Temp8K), "%s%d", CutName, unique_num);


	for (;;)
	{
		bool NeedMore = false;

		if (ExistInMission(Temp8K))
		{
			crt_snprintf (Temp8K, ARRSIZE(Temp8K), "%s%d", CutName, unique_num);
			unique_num++;
			NeedMore = true;
		}

		if (!NeedMore) break;
	}


	return Temp8K;
}


bool CompareFunc (const string& s1, const string& s2)
{
	if (s1 < s2)	return true;
	return false;
}

TNewObjectWindow::TNewObjectWindow () : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT), Groups(_FL_)
{
	dwWindowBackColor = 0xFFCECEE8;

	bCanCreate = false;
	bPopupStyle = true;

	bAlwaysOnTop = true;
	icon->Load ("newobject");
	Caption = "Create mission object";
	pFont->SetSize (16);


  txtObjectName = NEW GUIEdit (this, 10, 5, 365, 20);
	txtObjectName->pFont->SetName("arialcyrsmall");
	txtObjectName->OnAccept = (CONTROL_EVENT)&TNewObjectWindow::NameChanged;
	txtObjectName->Hint = "Object name";
	txtObjectName->Flat = true;

	

/*
  TreeView1 = NEW GUITreeView (this, 340, 36, WINDOW_WIDTH-320-30, WINDOW_HEIGHT-80);
	TreeView1->FontColor = 0xFF000000;
	TreeView1->pFont->SetName("arialcyrsmall");
	TreeView1->SetImagesArray ("gtree");
	TreeView1->OnChange = (CONTROL_EVENT)SelChange;
	TreeView1->OnDblClick = (CONTROL_EVENT)SelectionChange;
	TreeView1->Hint = "Object parameters";
	TreeView1->MakeFlat(true);
	TreeView1->OnBeforeHintShow = (CONTROL_EVENT)TreeShowHint;
*/	


	btnOK = NEW GUIButton (this, 140, WINDOW_HEIGHT-38, 110, 30);
	btnOK->Caption = "Create";
	btnOK->Glyph->Load ("lamp");
	btnOK->FontColor = 0xFF000000;
	btnOK->pFont->SetSize (16);
  btnOK->OnMousePressed = (CONTROL_EVENT)&TNewObjectWindow::btnCreatePressed;
	btnOK->FlatButton = true;

	btnCancel = NEW GUIButton (this, 20, WINDOW_HEIGHT-38,  110, 30);
	btnCancel->Caption = "< Back";
	//btnCancel->Glyph->Load ("exit");
	btnCancel->FontColor = 0xFF000000;
	btnCancel->pFont->SetSize (16);
  btnCancel->OnMousePressed = (CONTROL_EVENT)&TNewObjectWindow::btnCancelPressed;
	btnCancel->FlatButton = true;

	//AvailObj = NEW GUIComboBox (this, 5, 5, 200, 20);;
	//AvailObj->OnChange = (CONTROL_EVENT)MissionObjectSelected;

	//GUIListBox* 
	ObjectsList = NEW GUIListBox (this, 230, 36, 260, WINDOW_HEIGHT-80);
	ObjectsList->pFont->SetName("arialcyrsmall");
	ObjectsList->bCaptureKeyboard = false;
	ObjectsList->OnChange = (CONTROL_EVENT)&TNewObjectWindow::MissionObjectSelected;
	ObjectsList->Hint = "Available mission objects";
	ObjectsList->OnBeforeHintShow= (CONTROL_EVENT)&TNewObjectWindow::ObjectShowHint;
	ObjectsList->MakeFlat(true);
	ObjectsList->OnMouseDBLClick = (CONTROL_EVENT)&TNewObjectWindow::btnCreatePressed;


	GroupList = NEW GUIListBox (this, 10, 36, 200, WINDOW_HEIGHT-80); 
	GroupList->pFont->SetName("arialcyrsmall");
	GroupList->bCaptureKeyboard = false;
	GroupList->OnChange = (CONTROL_EVENT)&TNewObjectWindow::GroupChanged;
	GroupList->Hint = "Object groups...";
	GroupList->MakeFlat(true);
	//GroupList->OnMouseDBLClick = (CONTROL_EVENT)btnCreatePressed;


	Groups.DelAll();

	for (DWORD n = 0; n < sMission->AvailableMO.Size(); n++)
	{
		MissionEditor::tAvailableMO* availMO = &sMission->AvailableMO[n];

		const char* ObjectGroup = availMO->Params->GetGroup();
		AddGroup(ObjectGroup);

		//availMO->
		//ObjectsList->Items.Add (availMO->Name);
	}

	//ObjectsList->Items.Sort(CompareFunc);

	ObjectsList->Items.Clear();


	for (n = 0; n < Groups.Size(); n++)
	{
		GroupList->Items.Add (Groups[n].c_str());
	}

	GroupList->Items.Sort(CompareFunc);


	GroupChanged(NULL);


	SetScreenCenter ();



}


void _cdecl TNewObjectWindow::GroupChanged (GUIControl* sender)
{
	int index = GroupList->SelectedLine;
	if (index == -1) return;

	string MasterGroup = GroupList->Items[index];


	ObjectsList->Items.Clear();

	for (DWORD n = 0; n < sMission->AvailableMO.Size(); n++)
	{
		MissionEditor::tAvailableMO* availMO = &sMission->AvailableMO[n];

		string ObjectGroup = DEFAULT_GROUP_NAME;
		const char* szGName = availMO->Params->GetGroup();
		if (szGName) ObjectGroup = szGName;
		if (ObjectGroup == MasterGroup)
		{
			ObjectsList->Items.Add (availMO->Name);
		}
	}

	ObjectsList->Items.Sort(CompareFunc);

}


void TNewObjectWindow::AddGroup (const char* szGroup)
{
	string GroupName = DEFAULT_GROUP_NAME;
	if (szGroup != NULL) GroupName = szGroup;

	for (dword i = 0; i < Groups.Size(); i++)
	{
		if (Groups[i] == GroupName) return;
	}

	Groups.Add(GroupName);

	
}

void TNewObjectWindow::OnCreate ()
{
	//txtObjectName->SetFocus ();
	
}

TNewObjectWindow::~TNewObjectWindow ()
{
}



void _cdecl TNewObjectWindow::OnAcceptNewObjectName (GUIControl* sender)
{
}


void _cdecl TNewObjectWindow::SelectionChange (GUIControl* sender)
{
}

GUITreeNode* TNewObjectWindow::FindFirstFolder (GUITreeNode* m_node)
{
	if (m_node->Tag == TAG_FOLDER) return m_node;
	if (m_node->Parent == NULL) return NULL;
	return FindFirstFolder (m_node->Parent);
}



void _cdecl TNewObjectWindow::btnCreatePressed (GUIControl* sender)
{
		//if (!pMissionObject) return;
	if (!bCanCreate) return;

	if (txtObjectName->Text.IsEmpty())
	{

		return;
	}

	MissionEditor::tAvailableMO* pMissionObject = NULL;

	int index = ObjectsList->SelectedLine;
	if (index == -1) return;

	for (DWORD n = 0; n < sMission->AvailableMO.Size(); n++)
	{
		pMissionObject = &sMission->AvailableMO[n];

		if (crt_stricmp (pMissionObject->Name, ObjectsList->Items[index].GetBuffer()) == 0)
		{
			break;
		}
	}

		//api->Trace("!!!!!!!!!!!!!!! Close window - 0x%08X", this);
	
		MOPWriter wrt(pMissionObject->Params->GetVersion (), MakeUniqueName (txtObjectName->Text.GetBuffer()));
		pMissionObject->AttrList->AddToWriter (wrt);

		
		MOSafePointer mo;
		miss->CreateObject (mo, pMissionObject->ClassName, wrt.Reader ());

		//mo = NULL;
		if (!mo.Validate())
		{
			Application->MessageBox (" Can't create object !!! ", "Error", GUIMB_OK);
			//_asm int 3;
			return;
		}


		//OnAcceptNewObjectName (sender);


		MissionEditor::tCreatedMO* nEntry = &sMission->AddCreatedMissionObjectStruct();


		GUITreeNode* tn = MainWindow->TreeView1->GetSelectedNode ();
		
		if (tn)
		{
			GUITreeNode* upperfolder = FindFirstFolder (tn);

			if (upperfolder)
			{
				const char* fullname = upperfolder->GetFullPath();
				nEntry->PathInTree = fullname;
				//strncpy (nEntry->PathInTree, fullname, MAXOBJECTPATH);
			} else
			{
				nEntry->PathInTree = "";
			}
		} else
		{
			nEntry->PathInTree = "";
		}

		GUITreeNode* treenode = globalNodesPool->CreateNode();
		treenode->SetText(mo.Ptr()->GetObjectID ().c_str());
		treenode->Image->Load ("meditor\\mobject");
		treenode->Tag = TAG_OBJECT;
		treenode->Data = nEntry;
		treenode->CanDrop = false;

    GUITreeNode* folder = MainWindow->TreeView1->FindItem (nEntry->PathInTree);
		if (folder)
		{
			folder->Childs.Add (treenode);
		} else
			{
				MainWindow->TreeView1->Items->Add (treenode);
			}

		nEntry->Version = pMissionObject->Params->GetVersion ();
		nEntry->pObject = mo;
		nEntry->AttrList = NEW AttributeList;
		nEntry->Level = pMissionObject->Params->GetLevel ();
		nEntry->ClassName = pMissionObject->ClassName;
		//strncpy (nEntry->ClassName, pMissionObject->ClassName, MAX_ENTITY_NAME);
		nEntry->AttrList->CreateFromList (pMissionObject->AttrList);

		nEntry->AttrList->AddToTree (treenode, globalNodesPool);


		//Записываем в каждый аттрибут указатель
    // на объект...
    
    nEntry->AttrList->SetMasterData (nEntry);

		pMissionObject->AttrList->BeforeDelete ();
		pMissionObject = NULL;

		
		Close (this);

}

void _cdecl TNewObjectWindow::btnCancelPressed (GUIControl* sender)
{
	Close (this);
}



void _cdecl TNewObjectWindow::NameChanged (GUIControl* sender)
{
	txtObjectName->Text = MakeUniqueName (txtObjectName->Text);
}

void _cdecl TNewObjectWindow::MissionObjectSelected (GUIControl* sender)
{
	bCanCreate = false;
	int index = ObjectsList->SelectedLine;
	if (index == -1) return;

	bCanCreate = true;


	txtObjectName->Text = MakeUniqueName (ObjectsList->Items[index].GetBuffer());
	NameChanged(NULL);
	
	txtObjectName->SetFocus();
	txtObjectName->SelectText(0, txtObjectName->Text.Len());
}



void TNewObjectWindow::KeyPressed(int Key, bool bSysKey)
{
  if (!bSysKey) return;

//	if (!IsActive()) return;
	if (!IsActive) return;

	if (Key == VK_ESCAPE)
	{
	 btnCancelPressed (this);
	}

	if (Key == 13)
	{
	 btnCreatePressed (this);
	}

}



void _cdecl TNewObjectWindow::ObjectShowHint (GUIControl* sender)
{
	// Получить узел находящийся под курсором
	int Line = ObjectsList->GetElementNumAtPos(Application->GetCursor()->GetPosition());

	

	if (Line < 0)
	{
		ObjectsList->Hint = " ";
		ObjectsList->OnHintShow_RetValue = false;
	} else
	{
		
		string TextHint = sMission->GetCommentForName(ObjectsList->Items[Line].c_str());

		//Если подсказка не пустая...
		if (ObjectsList->Hint != " ")
		{
			// Сравнить, то, что мы хотим показать
			// с тем, что мы уже показываем
			if (TextHint != ObjectsList->Hint)
			{
				//Если подсказки разные погасить, на время :)
				ObjectsList->OnHintShow_RetValue = false;
			}
		}

		// Присвоить нужный текст...
		ObjectsList->Hint = TextHint;

	}



}