
#include "VSSResourceBrowser.h"
#include "mainwindow.h"
#include "panel.h"



#define WINDOW_POSX   400
#define WINDOW_POSY   10
#define WINDOW_WIDTH  440
#define WINDOW_HEIGHT 300

extern TMainWindow* MainWindow;

extern string VSSUserName;

extern IGUIManager* igui;

//extern SourceSafeItem VSSRoot;

extern string MissionsSrcLocalFolder;
extern TPanelWindow* PanelWindow; 
extern bool LoadCameraPosition;

TVSSResourceBrowser::TVSSResourceBrowser (const char* szUserName) : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	strVSSUserName = szUserName;

	bAlwaysOnTop = true;
	bSystemButton = false;
  Caption = "VSS Browser";

	TreeView1 = NEW GUITreeView(this, 5, 5, 320, 250);
	TreeView1->FontColor = 0xFF000000;
	TreeView1->pFont->SetName("arialcyrsmall");
	TreeView1->SetImagesArray ("gtree");
	//TreeView1->OnChange = (CONTROL_EVENT)SelectEvent;
	//TreeView1->OnBeforeChange = (CONTROL_EVENT)BeforeSelectEvent;
	//TreeView1->OnDblClick = (CONTROL_EVENT)SelectionChange;
	TreeView1->Items->Clear ();
	TreeView1->bDragAndDrop = false;
	//TreeView1->AfterDrop = (CONTROL_EVENT)WhenDrop;
	//TreeView1->BeforeDrop = (CONTROL_EVENT)BeforeDropItem;
	//TreeView1->OnCopyNode = (CONTROL_EVENT)WhenCopy;
	//TreeView1->OnBeforeHintShow = (CONTROL_EVENT)TreeShowHint;
	//TreeView1->OnRightSelect = (CONTROL_EVENT)FastObjectPropView;
	TreeView1->MakeFlat (true);



	btnOK = NEW GUIButton (this, 340,9, 78, 24);
	btnOK->Glyph->Load ("ok");
	btnOK->Caption = "Load";
	btnOK->pFont->SetSize (12);
	btnOK->OnMousePressed = (CONTROL_EVENT)&TVSSResourceBrowser::onOKPressed;
	btnOK->FlatButton = true;
	
	
	btnCancel = NEW GUIButton (this, 340,39, 78, 24);
	btnCancel->Glyph->Load ("cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->pFont->SetSize (12);
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TVSSResourceBrowser::onCancelPressed;
	btnCancel->FlatButton = true;


	SetScreenCenter ();
}

void TVSSResourceBrowser::OnCreate ()
{
	//Max
	return;
/*
	bool bDatabase = VSSAcess::VSSEnumDatabase(VSSRoot, strVSSUserName.c_str());

	if (bDatabase == false)
	{
		igui->MessageBox("Can't open VSS database", "Error", GUIMB_OK, true);
		Close (this);
		return;
	}

	
	SourceSafeItem &new_root = VSSRoot.Childs[0];
	//показываем не с корня а от нужного нам узла VSS

	GUITreeNode* pRoot = NEW GUITreeNode;
	pRoot->Text = new_root.Name;
	//pRoot->Text = root.Name;
	pRoot->Image->Load ("folder");
	pRoot->CanCopy = false;
	pRoot->Data = &VSSRoot.Childs[0];
	TreeView1->Items->Add (pRoot);

	AddRecursive(&pRoot->Childs, new_root);
	//AddRecursive(&pRoot->Childs, root);
*/
	

}


TVSSResourceBrowser::~TVSSResourceBrowser ()
{
}

/*
void TVSSResourceBrowser::AddRecursive (GUITreeNodes* pNodes, SourceSafeItem &root)
{
	for (dword i = 0; i < root.Childs.Size(); i++)
	{
		GUITreeNode* pRoot = NEW GUITreeNode;
		pRoot->Text = root.Childs[i].Name;
		pRoot->Data = &root.Childs[i];

		SourceSafeItem* pSSItem = (SourceSafeItem*)pRoot->Data;

		if (root.Childs[i].Type == SourceSafeItem::VSS_PROJECT)
		{
			pRoot->Image->Load ("folder");
		} else
		{
			if (!root.Childs[i].bCheckedOut)
			{
				pRoot->Image->Load ("meditor\\vss_normal");
			} else
			{
				pRoot->Image->Load ("meditor\\vss_checkedout");
			}
		}

		//
		pRoot->CanCopy = false;
		pNodes->Add (pRoot);

		AddRecursive(&pRoot->Childs, root.Childs[i]);
	}
	
}
*/

void _cdecl TVSSResourceBrowser::onOKPressed (GUIControl* sender)
{
	/*
	GUITreeNode* pSelectedNode = TreeView1->GetSelectedNode();

	if (pSelectedNode)
	{
		SourceSafeItem* pSSItem = (SourceSafeItem*)pSelectedNode->Data;
		//pSSItem->Checkout("d:\\projects");
		bool bResult = pSSItem->GetLatestVersion(MissionsSrcLocalFolder.c_str());

		if (bResult)
		{
			string msr_name;
			msr_name = MissionsSrcLocalFolder;
			msr_name += "\\";
			msr_name += pSSItem->Name;
			PanelWindow->StartLoad(msr_name);
		}
	}

	Close (this);
	*/
}

void _cdecl TVSSResourceBrowser::onCancelPressed (GUIControl* sender)
{
	Close (this);
}

void TVSSResourceBrowser::KeyPressed(int Key, bool bSysKey)
{
  if (!bSysKey) return;

	if (Key == 13) onOKPressed (NULL);
	if (Key == VK_ESCAPE) onCancelPressed (NULL);

}


