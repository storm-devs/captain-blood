/*
#include "SelectFromGMX.h"

#include "..\..\NodesPool.h"


extern TreeNodesPool* globalNodesPool;
	
//	GUIButton* btnOK; 
//	GUIButton* btnCANCEL; 
//	GUITreeView* TreeView1;

extern string my_gmx_val;

#define WINDOW_WIDTH  320
#define WINDOW_HEIGHT 270

	
TSSFGeometry::TSSFGeometry (int posX, int posY, IGMXScene* scene) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;
	pScene = scene;
	Caption = "Select geometry";
  TreeView1 = NEW GUITreeView (this, 5, 5, 300, 230);
	TreeView1->FontColor = 0xFF000000;
	TreeView1->pFont->SetName ("arialcyrsmall");
	TreeView1->SetImagesArray ("gtree");
	TreeView1->Items->Clear ();
	TreeView1->MakeFlat (true);


	//btnCANCEL; 
	btnOK = NEW GUIButton (this, 5, 240, 100, 24);
	btnOK->Glyph->Load ("ok");
	btnOK->Caption = "Select";
	btnOK->FontColor = 0xFF000000;
	btnOK->pFont->SetName ("arialcyrsmall");
  btnOK->OnMousePressed = (CONTROL_EVENT)&TSSFGeometry::OKPressed;
	btnOK->FlatButton = true;

	btnCANCEL = NEW GUIButton (this, 116, 240, 100, 24);
	btnCANCEL->Glyph->Load ("cancel");
	btnCANCEL->Caption = "cancel";
	btnCANCEL->FontColor = 0xFF000000;
	btnCANCEL->pFont->SetName ("arialcyrsmall");
  btnCANCEL->OnMousePressed = (CONTROL_EVENT)&TSSFGeometry::CANCELPressed;
	btnCANCEL->FlatButton = true;


	if (scene)
	{

		IGMXIterator* pIter = scene->CreateIterator (_FL_);

		for (pIter->Begin (); !pIter->IsDone(); pIter->Next ())
		{
			IGMXEntity* pEnt = pIter->Get ();

			AddNode (pEnt);
		}
		

		pIter->Release ();
	}

}

TSSFGeometry::~TSSFGeometry ()
{
	delete btnOK;
	delete btnCANCEL;
	delete TreeView1;
}




GUITreeNode* TSSFGeometry::FindObject (IGMXEntity* obj, GUITreeNodes* items)
{
	for (int n =0; n < items->GetCount(); n++)
	{
		IGMXEntity* tn = (IGMXEntity*)items->Get(n)->Data;
		if (tn == obj) return items->Get(n);

		if (items->Get(n)->Childs.GetCount () > 0)
		{
			GUITreeNode* result = FindObject (obj, &items->Get(n)->Childs);
			if (result) return result;
		}
	}
	return NULL;
}

void TSSFGeometry::AddNode (IGMXEntity* pEnt)
{
	GUITreeNode* treenode = globalNodesPool->CreateNode();
	treenode->SetText(pEnt->GetName ());
	if (pEnt->GetType () == GMXET_LOCATOR)
	{
		treenode->Image->Load ("locator");
	} else
		{
			if (pEnt->GetType () == GMXET_GROUP)
			{
				treenode->Image->Load ("folder");
			} else
				{
					treenode->Image->Load ("mesh");
				}
		}

	

	treenode->Data = pEnt;

	// Если нужно добавляем к корню...
	if (pEnt->GetParent() == pScene->GetRoot ())
	{
		TreeView1->Items->Add (treenode);
		return;
	} else
		{
		 GUITreeNode* parent = FindObject (pEnt->GetParent(), TreeView1->Items);
		 if (parent)
		 {
			 parent->Childs.Add (treenode);
			 return;
		 } else
			 {
				 AddNode (pEnt->GetParent());
				 AddNode (pEnt);
				 return;
			 }
		}

}


void _cdecl TSSFGeometry::OKPressed (GUIControl* sender)
{
	GUITreeNode* s_node = TreeView1->GetSelectedNode();
	if (s_node)
	{
		//my_gmx_val = pScene->getf;
		my_gmx_val = ((IGMXEntity*)s_node->Data)->GetFullName().c_str();
	} else
		{
		 my_gmx_val = "";
		}

	Close (this);
}

void _cdecl TSSFGeometry::CANCELPressed (GUIControl* sender)
{
	Close (this);
}

*/