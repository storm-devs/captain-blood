

#include "mainwindow.h"
#include "..\missioneditor.h"
#include "..\..\common_h\mission.h"
#include "..\attributes\pathutils.h"
#include "..\attributes\AttributeList.h"
#include "panel.h"
#include "fastpreview.h"
#include "..\load.h"
#include "globalParams.h"



#define WINDOW_POSX   0
#define WINDOW_POSY   0
#define WINDOW_WIDTH  240
#define WINDOW_HEIGHT 768

#define REACTION_TIME 0.2f



//IMission* playing_miss = NULL;
extern IRender * pRS;
extern int EditorMode;
extern IGUIManager* igui;
extern IMission* miss;
extern MissionEditor* sMission;
extern char* MakeUniqueName (const char* Name);
extern TPanelWindow* PanelWindow;

extern TreeNodesPool* globalNodesPool;


extern TPanelWindow* panelWnd;

float fAdditionalWidth = 0.0f;
int AdditionalWidthNeed = 100;
int AdditionalWidth = 100;
bool bBigPanel = true;

GUITreeNode* SpecialNode = NULL;

bool bSysSelection = false;
BaseAttribute* pEditableNode = NULL;

MOSafePointer pCurrentSelected;


BaseAttribute* pRTEditedAttribute = NULL;





char TempBuffer[2048];
char* FtoA (float f)
{
  crt_snprintf (TempBuffer, ARRSIZE(TempBuffer), "%3.2f", f);
  return TempBuffer;
}


bool TMainWindow::FuncCompare (GUITreeNode* const &a1, GUITreeNode* const &a2)
{
  if (a1->Tag < a2->Tag) return true;
  if (a1->Tag == a2->Tag)
  {
    if (crt_stricmp(a1->GetText(), a2->GetText()) < 0) return true;
  }
  return false;
}

int TMainWindow::NeedToSortChilds(GUITreeNode* const &a1)
{
	if (a1->Tag == TAG_FOLDER) return 1;
	return 0;
}


TMainWindow::TMainWindow (GUIControl* parent, int posX, int posY, int width, int height) : GUIWindow (parent, posX, posY, width, height)
{
  //WINDOW_WIDTH
  //AdditionalWidth


  int tttt = (pRS->GetScreenInfo3D().dwWidth - WINDOW_WIDTH);
  AdditionalWidthNeed = tttt / 4;
  AdditionalWidth = 0;
  fAdditionalWidth = 0.0f;
  


  if (width <= 10) SetWidth(WINDOW_WIDTH);
  if (height <= 10) SetWidth(WINDOW_HEIGHT);
  bPopupStyle = true;

  WDontPressedTime = 0.0f;
  EDontPressedTime = 0.0f;

  bSystemButton = false;
  Caption = "Mission editor";
  pFont->SetSize (16);

  
  TreeView1 = NEW GUITreeView (this, 1, 1, DrawRect.w-12, DrawRect.h - 10);
  TreeView1->FontColor = 0xFF000000;
  TreeView1->pFont->SetName("arialcyrsmall");
  TreeView1->SetImagesArray ("gtree");
  TreeView1->OnChange = (CONTROL_EVENT)&TMainWindow::SelectEvent;
  TreeView1->OnBeforeChange = (CONTROL_EVENT)&TMainWindow::BeforeSelectEvent;
  TreeView1->OnDblClick = (CONTROL_EVENT)&TMainWindow::SelectionChange;
  TreeView1->Items->Clear ();
  TreeView1->bDragAndDrop = true;
  TreeView1->AfterDrop = (CONTROL_EVENT)&TMainWindow::WhenDrop;
  TreeView1->BeforeDrop = (CONTROL_EVENT)&TMainWindow::BeforeDropItem;
  TreeView1->OnCopyNode = (CONTROL_EVENT)&TMainWindow::WhenCopy;
  TreeView1->OnBeforeHintShow = (CONTROL_EVENT)&TMainWindow::TreeShowHint;
  TreeView1->OnRightSelect = (CONTROL_EVENT)&TMainWindow::FastObjectPropView;

	TreeView1->SetFinishingChar('|');



  
  TreeView1->MakeFlat (true);

  CreateDefaultFolders(this);


  SortTree();

  
}

void TMainWindow::SortTree()
{
  TreeView1->SortEx(FuncCompare, NeedToSortChilds);
}


void _cdecl TMainWindow::CreateDefaultFolders (GUIControl* sender)
{
// Создаем стандартные папки...
  GUITreeNode* folder = globalNodesPool->CreateNode();
  folder->SetText("Objects");
  folder->Image->Load ("folder");
  folder->CanCopy = false;
  folder->Tag = TAG_FOLDER;
  TreeView1->Items->Add (folder);

  GUITreeNode* folder2 = globalNodesPool->CreateNode();
  folder2->SetText("Events");
  folder2->Image->Load ("folder");
  folder2->CanCopy = false;
  folder2->Tag = TAG_FOLDER;
  TreeView1->Items->Add (folder2);
  
  GUITreeNode* folder3 = globalNodesPool->CreateNode();
  folder3->SetText("Personages");
  folder3->Image->Load ("folder");
  folder3->CanCopy = false;
  folder3->Tag = TAG_FOLDER;
  TreeView1->Items->Add (folder3);

  GUITreeNode* folder4 = globalNodesPool->CreateNode();
  folder4->SetText("Triggers");
  folder4->Image->Load ("folder");
  folder4->CanCopy = false;
  folder4->Tag = TAG_FOLDER;
  TreeView1->Items->Add (folder4);

  GUITreeNode* folder5 = globalNodesPool->CreateNode();
  folder5->SetText("Temp");
  folder5->Image->Load ("folder");
  folder5->CanCopy = false;
  folder5->Tag = TAG_FOLDER;
  TreeView1->Items->Add (folder5);

  bSysSelection = true;
  TreeView1->SetSelectedNode (folder);
  bSysSelection = false;

}

void TMainWindow::OnCreate()
{
  SetFocus();
}

void TMainWindow::Draw()
{





	// Max переделал на контрол, поскольку дибильный пунтосвичер меняет раскладку и регистр на scroll
	if ((miss->Controls().GetControlStateType("ChangeWidePanelMode") == CST_ACTIVATED))
	{
		bBigPanel = !bBigPanel;
	}
/*
  if (GetKeyState(VK_SCROLL) & 0x0001)
  {
    bBigPanel = true;
  } else
  {
    bBigPanel = false;
  }
*/
  if (bBigPanel )
  {
    fAdditionalWidth += 300.0f * api->GetDeltaTime();
    if (fAdditionalWidth >= (float)AdditionalWidthNeed) fAdditionalWidth = (float)AdditionalWidthNeed;
    AdditionalWidth = (int)fAdditionalWidth;
    
  } else
  {
    fAdditionalWidth -= 300.0f * api->GetDeltaTime();
    if (fAdditionalWidth < 0) fAdditionalWidth = 0;
    AdditionalWidth = (int)fAdditionalWidth;


    

  }


  Resize(WINDOW_WIDTH+AdditionalWidth);


  if (IsActive)
  {
   WDontPressedTime += api->GetDeltaTime();
   EDontPressedTime += api->GetDeltaTime();
  }

  GUIControl::Draw();


	if (panelWnd->SaveOKTime > 0)
	{
		float Alpha = (panelWnd->SaveOKTime / (SAVEDOK_SHOWTIME*2.0f));
		Color BackColor(1.0f, 1.0f, 1.0f, 0.5f + Alpha);
		Color BlackColor(0.0f, 0.0f, 0.0f, Alpha);
		int sLeft = 0;
		int sTop = 0;
		int sWidth = 300;
		int sHeight = 80;

		//pEditor->Render()->SetTexture(0, NULL);
		//pEditor->Render()->SetTexture(1, NULL);



		float sLeftAdd = (pRS->GetScreenInfo3D().dwWidth - sWidth) / 2.0f;
		float sTopAdd = (pRS->GetScreenInfo3D().dwHeight - sHeight) / 2.0f;


		RENDERVIEWPORT ViewPort;
		ViewPort.X = 0;
		ViewPort.Y = 0;
		ViewPort.Width = pRS->GetScreenInfo3D().dwWidth;
		ViewPort.Height = pRS->GetScreenInfo3D().dwHeight;
		ViewPort.MinZ = 0.0f;
		ViewPort.MaxZ = 1.0f;
		pRS->SetViewport(ViewPort);


		sLeft += (int)sLeftAdd;
		sTop += (int)sTopAdd;
		GUIHelper::Draw2DRectAlpha( sLeft, sTop, sWidth, sHeight, BackColor.GetDword());
		GUIHelper::DrawWireRect( sLeft, sTop, sWidth, sHeight, BlackColor.GetDword());
		int fntHeight = panelWnd->pFont->GetHeight();
		int fntWidth = panelWnd->pFont->GetWidth(gp->SaveBoxText.c_str());
		int fntAddWidth = (sWidth - fntWidth) / 2;
		int fntAddHeight = (sHeight - fntHeight) / 2;
		panelWnd->pFont->Print(sLeft+fntAddWidth, sTop+fntAddHeight, 0xFF000000, gp->SaveBoxText.c_str());
		panelWnd->SaveOKTime -= api->GetDeltaTime();
	}


}

TMainWindow::~TMainWindow ()
{
	TreeView1->Items->Clear();

	delete TreeView1;
	TreeView1 = NULL;


//  delete btnCreate;
  //delete btnCreatePath;
}




// Энумерация всех объектов и занесение их в три
//void _cdecl TMainWindow::btnEnumPressed (GUIControl* sender)
//{
/*
  // Получить кол-во созданных объектов  миссии
  int count = sMission->CreatedMO.GetCount ();
  for (int n =0; n < count; n++)
  {
    MissionEditor::tCreatedMO* pMo = sMission->CreatedMO(n);
    const char* object_name = pMo->pObject->GetObjectID ();
    string real_path = pMo->PathInTree;
    real_path += object_name;
    GUITreeNode* foundednode = TreeView1->FindItem (real_path.GetBuffer());


    // Пытаемся найти этот объект в дереве


    // Если нашли используем существующий узел
    GUITreeNode* treenode = NULL;
    if (foundednode != NULL)
    {
      treenode = foundednode;
    } else
    {
      GUITreeNode* folder = TreeView1->FindItem (pMo->PathInTree);
      //assert (folder != NULL);

      // Иначе создаем новый
      treenode = NEW GUITreeNode;
      MOSafePointer pMo = sMission->CreatedMO(n)->pObject;
      treenode->Text = pMo->GetObjectID ();
      //treenode->Text += sMission->CreatedMO(n)->PathInTree;
      treenode->Image->Load ("mobject");
      
      if (folder == NULL)
        TreeView1->Items->Add (treenode);
      else
        folder->Childs.Add (treenode);
      
        
      treenode->Data = pMo;
    }

    treenode->Tag = TAG_OBJECT;
    // Убиваем все аттрибуты...
    treenode->Childs.Clear ();
    // Создаем новые аттрибуты
    sMission->CreatedMO(n)->AttrList->AddToTree (treenode);

    // Во все объекты пишем указатель на структуру-мастер
    // чтобы можно было лего по аттрибуту мастера найти...
    int c = sMission->CreatedMO(n)->AttrList->GetCount ();
    for (int i = 0; i < c; i++)
    {
      sMission->CreatedMO(n)->AttrList->Get(i)->Data = sMission->CreatedMO(n);
    }



  }

*/
//}





void _cdecl TMainWindow::SelectionChange (GUIControl* sender)
{
  GUITreeNode* sNode = TreeView1->GetSelectedNode ();
  if (sNode == NULL) return;

  if (sNode->Tag == TAG_FOLDER)
  {
    ShowOrHideFolder(sNode);
    return;
  }

	if (sNode->bReadOnly)
	{
		return;
	}


  // Это объект миссии
  if (sNode->Tag == TAG_OBJECT)
  {
    MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)sNode->Data;
    if (pMo->pObject.Validate())
    {
      pMo->pObject.Ptr()->EditMode_Select (true);
      if (pCurrentSelected.Validate()) pCurrentSelected.Ptr()->EditMode_Select (false);
      pCurrentSelected = pMo->pObject;
    }

		PanelWindow->RenameObjectPressed(NULL);

    return;
  }

  // Это аттрибут объекта...
  if (sNode->Tag == TAG_ATTRIBUTE)
  {
    int cX, cY;
    cX = cY = 0;
    Application->GetCursor (cX, cY);
    cX -= 20;
    cY -= 10;

  
    BaseAttribute* pBaseNode = (BaseAttribute*)sNode->Data;
    if (pBaseNode->IsSupportRTEdit())
    {
      // Bool'ы по DBL клику переключаем 
      if (pBaseNode->GetType() == IMOParams::t_bool)
      {
        BoolAttribute* battr = (BoolAttribute*)pBaseNode;
        battr->SetValue(!battr->GetValue());
        pEditableNode = pBaseNode;
        UpdateTree(this);
      }
      return;
    }

    pEditableNode = pBaseNode;
    if (pBaseNode == NULL) return;
    if (pBaseNode->GetMasterData() == NULL) return;

    pBaseNode->PopupEdit ((int)cX, (int)cY);
    pBaseNode->pForm->OnClose = (CONTROL_EVENT)&TMainWindow::UpdateTree;

    //GUITreeNode* sNode1 = TreeView1->GetSelectedNode ();
    //sNode1->getp
    //TreeView1->SetSelectedNode(sNode->Parent, false);
    
    return;
  }
}

GUITreeNode* TMainWindow::FindObjectNode (GUITreeNode* c_node)
{
  if ((c_node) && (c_node->Tag == TAG_OBJECT)) return c_node;

  if (c_node->Parent)
  {
    return FindObjectNode (c_node->Parent);
  }

  return NULL;
}

void _cdecl TMainWindow::UpdateTree (GUIControl* sender)
{
  static char TempSelectedNode[MAXOBJECTPATH];
  TempSelectedNode[0] = 0;

  //Если нету аттрибута который мы редактировали на выход...
  if (pEditableNode == NULL) return;

  // Получаем объект которому принадлежит аттрибут
  MissionEditor::tCreatedMO* curNode = (MissionEditor::tCreatedMO*)pEditableNode->GetMasterData();

  // Обновляем объект миссии...
  MOPWriter wrt(curNode->Level, curNode->pObject.Ptr()->GetObjectID().c_str());
  curNode->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
  miss->EditorUpdateObject(curNode->pObject.Ptr(), wrt);
#endif
  //curNode->pObject->EditMode_Update (wrt.Reader ());

  // получаем выделенный узел дерева
  GUITreeNode* sNode1 = TreeView1->GetSelectedNode ();
  

  // NodeToAdd - это узел куда мы будем добавлять...
  GUITreeNode* NodeToAdd = sNode1;

  // Если нужно будет добавлять в узел отличный от выделенного учитываем это...
  if (SpecialNode) NodeToAdd = SpecialNode;

  /* Выясняем какой узел был выделен */
  int AttrIndex = -1;
  if (sNode1)
  {
    if (sNode1->Tag == TAG_ATTRIBUTE)
    {
      // Для аттрибутов используем индексы
      // т.к. у них меняеться название...
      crt_strcpy (TempSelectedNode, MAXOBJECTPATH-1, sNode1->Parent->GetFullPath());
      AttrIndex = -1;
      for (int n = 0; n < sNode1->Parent->Childs.GetCount(); n++)
      {
        if (sNode1->Parent->Childs[n] == sNode1)
        {
          AttrIndex = n;
          break;
        }
      }
    } else
    {
      // Для не АТТРИБУТОВ применяем обычный алгоритм поиска по имени...
      crt_strcpy (TempSelectedNode, MAXOBJECTPATH-1, sNode1->GetFullPath());
      AttrIndex = -1;     
    }
  }
  

  // Поднимаемся по дереву, пока не найдем узел ОБЪЕКТ...
  GUITreeNode* sNode = NULL;
  if (NodeToAdd) sNode = FindObjectNode (NodeToAdd);
  // Если нашли...
  if (sNode)
  {
    //Стираем у него всех детей...
    TreeView1->ResetSelection(sNode->Childs);
    curNode->AttrList->BeforeDelete ();
    sNode->Childs.Clear ();

    // И добавляем заново...
    curNode->AttrList->AddToTree (sNode, globalNodesPool);
  }

  /* Восстанавливаем выделение*/
  if (sNode1)
  {
    //Если не по индексу...
    if (AttrIndex == -1)
    {
      GUITreeNode* f_node = TreeView1->FindItem(TempSelectedNode);
      if (f_node) TreeView1->SetSelectedNode(f_node, false);
    } else
      {
        //Если по индексу...
        GUITreeNode* f_node = TreeView1->FindItem(TempSelectedNode);
		if (f_node)
		{
			GUITreeNode* sel_node = f_node->Childs[AttrIndex];
			bSysSelection = true;
			//if ((sel_node) && (SpecialNode != NULL))
			if (sel_node)
			{
			  TreeView1->SetSelectedNode(sel_node, false);
			}
		}
        bSysSelection = false;
      }
  }


  //Сбрасываем специальный узел...
  SpecialNode = NULL;
}






void _cdecl TMainWindow::BeforeDropItem (GUIControl* sender)
{
  if (miss->Controls().GetControlStateType("DragCopyButton") == CST_ACTIVE) return;

  GUITreeNode* droped_node = (GUITreeNode*)sender;
  GUITreeNode* draged_node = TreeView1->GetDragedItem();

  GUITreeNodes* nodes = NULL;

  if (droped_node)
    nodes = &droped_node->Childs;
  else
    nodes = TreeView1->Items;


	if (droped_node && droped_node->bReadOnly)
	{
		TreeView1->BeforeDrop_CanDrop = false;
		return;
	}

  for (int n = 0; n < nodes->GetCount(); n++)
  {
    GUITreeNode* cur_node = nodes->Get(n);
    if (crt_stricmp(cur_node->GetText(), draged_node->GetText())== 0)
    {
      TreeView1->BeforeDrop_CanDrop = false;
      return;
    }
  }



}

void _cdecl TMainWindow::WhenDrop (GUIControl* sender)
{
  GUITreeNode* dropedobject = (GUITreeNode*)sender;

  // Если папка у всех детей пересчитываем путь
  if (dropedobject->Tag == TAG_FOLDER) 
  {
    for (int n = 0 ; n < dropedobject->Childs.GetCount (); n++)
    {
      WhenDrop (dropedobject->Childs.Get(n));
    }
  }

  if (dropedobject->Tag != TAG_OBJECT) return;
  MissionEditor::tCreatedMO* pObj = (MissionEditor::tCreatedMO*)dropedobject->Data;

  
  if (dropedobject->Parent)
  {
    const char* fullname = dropedobject->Parent->GetFullPath();
		pObj->PathInTree = fullname;
    //strncpy (pObj->PathInTree, fullname, MAXOBJECTPATH);
  } else
    {
      pObj->PathInTree = "";
    }
}


void _cdecl TMainWindow::WhenCopy (GUIControl* sender)
{
  GUITreeNode* DestNode = (GUITreeNode*)sender;
  GUITreeNode* SrcNode = TreeView1->GetDragedItem ();

  if (SrcNode->Tag == TAG_FOLDER) return;
  if (SrcNode->Tag == 0) return;

  const char* newName = MakeUniqueName (DestNode->GetText());
  DestNode->SetText(newName);

  if (DestNode->Tag == TAG_OBJECT)
  {
    MissionEditor::tCreatedMO* pMissionObject = (MissionEditor::tCreatedMO*)SrcNode->Data;


    MOPWriter wrt(pMissionObject->Version, newName);
    pMissionObject->AttrList->AddToWriter (wrt);

    MOSafePointer mo;
	miss->CreateObject (mo, pMissionObject->ClassName, wrt.Reader ());

    Assert (mo.Validate());

    MissionEditor::tCreatedMO* nEntry = &sMission->AddCreatedMissionObjectStruct();
    DestNode->SetText(mo.Ptr()->GetObjectID ().c_str());
    DestNode->Image->Load ("meditor\\mobject");
    DestNode->Tag = TAG_OBJECT;
    DestNode->Data = nEntry;
    DestNode->CanDrop = false;
    DestNode->Childs.Clear ();
    WhenDrop (DestNode);

    nEntry->Version = pMissionObject->Version;
    nEntry->pObject = mo;
    nEntry->AttrList = NEW AttributeList;
    nEntry->Level = pMissionObject->Level;
		nEntry->ClassName = pMissionObject->ClassName;
    //strncpy (nEntry->ClassName, pMissionObject->ClassName, MAX_ENTITY_NAME);
    nEntry->AttrList->CreateFromList (pMissionObject->AttrList);
    nEntry->AttrList->AddToTree (DestNode, globalNodesPool);

    nEntry->AttrList->SetMasterData (nEntry);
    /*
    int c = nEntry->AttrList->GetCount ();
    for (int i = 0; i < c; i++)
    {
      nEntry->AttrList->Get(i)->MasterData = nEntry;
    }
        */



  }
//  DestNode->Image->Load ("folder");
//  SrcNode->Image->Load ("folder");
}


bool TMainWindow::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{


  GUITreeNode* t = TreeView1->GetSelectedNode();
  if (message == GUIMSG_KEYPRESSED)
  {
    int Key = (int)lparam;
/*
    if (Key == VK_F1)
    {
      RunGamePressed (NULL);
      return true;
    }
*/
    if ((t) && (t->Tag == TAG_OBJECT))
    {
      MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)t->Data;
      if ((Key == 'W') && (WDontPressedTime > REACTION_TIME))
      {
        for (int n = 0; n < pMo->AttrList->GetCount(); n++)
        {
          if (pMo->AttrList->Get(n)->GetType() == IMOParams::t_position)
          {
            WDontPressedTime = 0.0f;
            TreeView1->SetSelectedNode(t->Childs.Get(n), false);
            return true;
          }
        }
      }

      if ((Key == 'E') && (EDontPressedTime > REACTION_TIME))
      {
        for (int n = 0; n < pMo->AttrList->GetCount(); n++)
        {
          if (pMo->AttrList->Get(n)->GetType() == IMOParams::t_angles)
          {
            EDontPressedTime = 0.0f;
            TreeView1->SetSelectedNode(t->Childs.Get(n), false);
            return true;
          }
        }
      }

    }
  }

  
  return GUIWindow::ProcessMessages(message, lparam, hparam);
}


void _cdecl TMainWindow::TreeShowHint (GUIControl* sender)
{
  // Получить узел находящийся под курсором
  GUITreeNode* tn = TreeView1->GetNodeAtPos(Application->GetCursor()->GetPosition());

  // Если под курсором пусто
  if (!tn)
  {
    //Сбросить подсказку...
    TreeView1->Hint = " ";
    TreeView1->OnHintShow_RetValue = false;
  } else
    {
      if (tn->Tag == TAG_ATTRIBUTE)
      {
        BaseAttribute* attr = (BaseAttribute*)tn->Data;
        const char* mHint = attr->GetHint();
        if (mHint != NULL && strlen(mHint) > 0)
        {
          TreeView1->Hint = mHint;
          return;
        }
      }

      string TextHint = "#b";

      switch (tn->Tag)
      {
        case TAG_FOLDER :
          TextHint += "[ Folder ] ";
          break;
        case TAG_OBJECT :
          {
            MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)tn->Data;
            if (pMo)
            {

              TextHint = sMission->GetCommentForClassName (pMo->ClassName);
            }
            break;
          }
          //TextHint += "[ Object ] ";
        case TAG_ATTRIBUTE :
          {
            BaseAttribute* attr = (BaseAttribute*)tn->Data;
            IMOParams::Type attr_type = attr->GetType();
            switch (attr_type)
            {
              case IMOParams::t_string :
                TextHint += "[ String ] ";
                break;
              case IMOParams::t_locstring :
                TextHint += "[ Localized String ] ";
                break;
              case IMOParams::t_bool :
                TextHint += "[ Bool ] ";
                break;
              case IMOParams::t_long :
                TextHint += "[ Long ] ";
                break;
              case IMOParams::t_float :
                TextHint += "[ Float ] ";
                break;
              case IMOParams::t_position :
                TextHint += "[ Position ] ";
                break;
              case IMOParams::t_angles :
                TextHint += "[ Rotation ] ";
                break;
              case IMOParams::t_color :
                TextHint += "[ Color ] ";
                break;
              case IMOParams::t_array :
                TextHint += "[ Array ] ";
                break;
              case IMOParams::t_enum :
                TextHint += "[ Enumerate ] ";
                break;
            }
            break;
          }
        case TAG_EVENT :
          TextHint += "[ Event ] ";
          break;
        case TAG_ARRAYITEM:
          TextHint += "[ Array item ] ";
          break;
      }

      if (tn->Tag != TAG_OBJECT)
      {
        TextHint += "@b";
        TextHint += tn->GetText();
      }

      //Если подсказка не пустая...
      if (TreeView1->Hint != " ")
      {
        // Сравнить, то, что мы хотим показать
        // с тем, что мы уже показываем
        if (TextHint != TreeView1->Hint)
        {
          //Если подсказки разные погасить, на время :)
          TreeView1->OnHintShow_RetValue = false;
        }
      }

      // Присвоить нужный текст...
      TreeView1->Hint = TextHint;
    }
}


void _cdecl TMainWindow::SelectEvent (GUIControl* sender)
{
  if (bSysSelection)  return;
  
  
  if (SpecialNode)
  {
    //UpdateTree(NULL);

	  if (SpecialNode->Tag == TAG_ATTRIBUTE)
	  {
		  BaseAttribute* pBaseNode = (BaseAttribute*)SpecialNode->Data;
		  pBaseNode->UpdateTree(SpecialNode);
	  }

    SpecialNode = NULL;
  }
  // Старый аттрибут поддтверждаем
  if (pRTEditedAttribute)
  {
    pRTEditedAttribute->ApplyRTEdit();
    pRTEditedAttribute = NULL;
    //
  }

  GUITreeNode* sNode = TreeView1->GetSelectedNode();

  if (!sNode) return;



	

	if (sNode->bReadOnly)
	{
		if (sNode->Tag == TAG_ATTRIBUTE)
		{
			PanelWindow->PanelMessage->Caption = "This attribute is read only\nAnd exist in imported mission\nCan't edit";
			return;
		} 

		PanelWindow->PanelMessage->Caption = "";
		return;
	}



  GUITreeNode* pMySel = FindObjectNode(sNode);
  if (pMySel)
  {
    MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)pMySel->Data;
    if (pMo)
    {
      if (pMo->pObject != pCurrentSelected)
      {
        pMo->pObject.Ptr()->EditMode_Select (true);
        if (pCurrentSelected.Validate()) pCurrentSelected.Ptr()->EditMode_Select (false);
        pCurrentSelected = pMo->pObject;
      }
    }
  } else
  {
    if (pCurrentSelected.Validate()) pCurrentSelected.Ptr()->EditMode_Select (false);
    pCurrentSelected.Reset();
  }



  if ((sNode) && (sNode->Tag == TAG_ATTRIBUTE))
  {
    BaseAttribute* pBaseNode = (BaseAttribute*)sNode->Data;

    
    if (pBaseNode->GetType() == IMOParams::t_position)
    {
      PanelWindow->KeyPressed('W', false);
    }

    if (pBaseNode->GetType() == IMOParams::t_angles)
    {
      PanelWindow->KeyPressed('E', false);
    }

    if (!pBaseNode->IsSupportRTEdit())
    {
      PanelWindow->PanelMessage->Caption = "This attribute not support #breal time edit\nDbl click@b to attribte for edit dialog";
      return;
    }

    PanelWindow->PanelMessage->Caption = "";
    GUITreeNode* ObjectNode = FindObjectNode (sNode);
    if (ObjectNode)
    {
      //Включаем новый реалтаймовые редактор аттрибута...
      MissionEditor::tCreatedMO* pMo = (MissionEditor::tCreatedMO*)ObjectNode->Data;
      pBaseNode->BeginRTEdit(pMo, PanelWindow->ToolsPanel);
      pRTEditedAttribute = pBaseNode;
    }
  }

  
}

void _cdecl TMainWindow::BeforeSelectEvent (GUIControl* sender)
{
  if (bSysSelection) return;
  if (!pRTEditedAttribute) return;
  SpecialNode = TreeView1->GetSelectedNode();
  pEditableNode = pRTEditedAttribute;
}

void TMainWindow::ShowOrHideFolder(GUITreeNode* node)
{
  if (crt_stricmp (node->Image->GetName(), "folder") == 0)
  {
    ShowOrHideFolderProcess (node, true);
    return;
  }

  if (crt_stricmp (node->Image->GetName(), "meditor\\hiddenfolder") == 0)
  {
    ShowOrHideFolderProcess (node, false);
    return;
  }
}

void TMainWindow::ShowOrHideFolderProcess(GUITreeNode* node, bool Hide)
{
  if (node->Tag == TAG_FOLDER)
  {
    if (Hide == true)
      node->Image->Load("meditor\\hiddenfolder");
    else
      node->Image->Load("folder");

    for (int n = 0; n < node->Childs.GetCount(); n++)
    {
      ShowOrHideFolderProcess (node->Childs[n], Hide);
    }
  }

  if (node->Tag == TAG_OBJECT)
  {
    if (Hide == true)
      node->Image->Load("meditor\\hiddenobject");
    else
      node->Image->Load("meditor\\mobject");

    MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)node->Data;
    if (pMo->pObject.Validate())
    {
      pMo->pObject.Ptr()->EditMode_Visible(!Hide);
    }
  }


  
    
}


void _cdecl TMainWindow::FastObjectPropView (GUIControl* sender)
{
  GUITreeNode* sNode = TreeView1->GetSelectedNode ();
  if (sNode == NULL) return;
  if (sNode->Tag != TAG_OBJECT) return;

  MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)sNode->Data;

  if (pMo == NULL) return;
  if (!pMo->pObject.Validate()) return;


  TFastPreviewWindow* pWind = NEW TFastPreviewWindow(pMo);
  Application->ShowModal (pWind);

  
}


void TMainWindow::Resize (dword dwNewWidth)
{
  TreeView1->ChangeWidth (dwNewWidth);
  SetWidth(dwNewWidth);
}


void TMainWindow::DeleteAllReadOnlyObjectsRecursive (GUITreeNodes* items)
{
	for (int q = 0; q < items->GetCount(); q++)
	{
		GUITreeNode* node = items->Get(q);


		if (!node)
		{
			continue;
		}

		if (node->bReadOnly)
		{
			TreeView1->SetSelectedNode(node, false);
			PanelWindow->RealDeleteSelected(this);


/*
			MissionEditor::tCreatedMO* pMo = (MissionEditor::tCreatedMO*)node->Data;
			if (pMo)
			{
				MOSafePointer objToDelete = pMo->pObject;

				// Удаляем из списка созданных объектов



				DWORD total_nodes = sMission->GetCreatedMissionObjectsCount();
				for (DWORD n = 0; n < total_nodes; n++)
				{
					MissionEditor::tCreatedMO* cObj = &sMission->GetCreatedMissionObjectStructByIndex(n);
					if (cObj->pObject == objToDelete)
					{
						// Убиваем аттрибуты объекта...
						delete cObj->AttrList;
						//sMission->CreatedMO.DelIndex (n);
						sMission->DeleteCreatedMissionObject(n);
						break;
					}
				}

				delete objToDelete;
			}



			// Удаляем из дерева...

			node->Release ();
*/

			// Убиваем текущий выбранный объект...
			pCurrentSelected.Reset();

			//MainWindow->TreeView1->SetSelectedNode (sParent, false);

			q--;

			continue;
		}

		DeleteAllReadOnlyObjectsRecursive(&node->Childs);
	}


}

void TMainWindow::DeleteAllReadOnlyObjects ()
{
	DeleteAllReadOnlyObjectsRecursive(TreeView1->Items);

	TreeView1->ResetSelection(*TreeView1->Items);

}


void TMainWindow::RefreshImportedMissions ()
{
	//Удаляем все объекты текущие которые импортированные
	DeleteAllReadOnlyObjects();

	//Тут надо проимпортировать все миссии по списку...

	array<string> & list = sMission->GetImportList();

	string a;
	for (dword i = 0; i < list.Size(); i++)
	{
		MissionLoad* mloader = NEW MissionLoad(sMission);

		a = gp->StartDirectory;
		a += list[i].c_str();

		mloader->LoadXML (a.c_str(), false, true);
		delete mloader;
	}

	SortTree ();

}