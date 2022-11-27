#ifndef _XBOX
#include "gui_treeview.h"

#define MAXOBJECTPATH 16384
//----------------------------------------------------------------

GUITreeNode::GUITreeNode () : GUIControl (NULL)
{
	bReadOnly = false;

	text_data[0] = 0;

	bUseCustomColor = false;;
	Color = 0xFFFFFFFF;


	CanCopy = true;
	CanDrag = true;
	CanDrop = true;
	OwnerList = NULL;
	Tag = 0;
  Data = NULL;
  Expanded = false;
	Selected = false;

/*
	Image = NEW GUIImage ();
	ExpandedImage = NEW GUIImage ();
	SelectedImage = NEW GUIImage ();
*/
	Image = &_Image;
	ExpandedImage = &_ExpandedImage;
	SelectedImage = &_SelectedImage;


  Parent = NULL;
  Childs.Clear ();
//  Text = "";

  Childs.OnAdd = (CONTROL_EVENT)&GUITreeNode::OnStrAdd;


}

char GTN_StrTempArray[MAXOBJECTPATH];
char GTN_StrTempArray2[MAXOBJECTPATH];


const char* GUITreeNode::GetFullPath()
{
	memset (GTN_StrTempArray, 0, MAXOBJECTPATH);
	GetFullNodeProcess (this);
	int l = strlen (GTN_StrTempArray);
	return GTN_StrTempArray;
}


const char* GUITreeNode::GetText()
{
	return text_data;
}

void GUITreeNode::SetText(const char* value)
{
	crt_strncpy(text_data, (TREE_VIEW_ITEM_LEN-1), value, (TREE_VIEW_ITEM_LEN-1));
}


void GUITreeNode::GetFullNodeProcess (GUITreeNode* node)
{
	crt_strncpy (GTN_StrTempArray2, MAXOBJECTPATH, GTN_StrTempArray, MAXOBJECTPATH - 1);
	const char* lpName = node->GetText();
	crt_snprintf (GTN_StrTempArray, MAXOBJECTPATH-1,  "%s\\%s", lpName, GTN_StrTempArray2);
	
	GUITreeNode* Parent = node->Parent;
	if (Parent) GetFullNodeProcess (Parent);
}




void GUITreeNode::Release ()
{
	// Если есть список-владелец
	if (OwnerList)
	{
		// Ищем себя в этом списке
		for (int n =0; n < OwnerList->GetCount (); n++)
		{
			// Нашли
			if (OwnerList->Get (n) == this)
			{
				// Удалили себя из списка...
				OwnerList->Delete (n);
				delete this;
				break;
			}
		}
	} else
		{
			delete this;
		}

}


GUITreeNode::~GUITreeNode ()
{
  Childs.Clear ();

/*
	delete SelectedImage;
	delete ExpandedImage;
  delete Image;
*/
}
  
bool GUITreeNode::HasChildren ()
{
 if (Childs.GetCount () > 0 ) return true;
 return false; 
}
  
int GUITreeNode::GetChildsCount ()
{
 return Childs.GetCount ();
}
  

void _cdecl GUITreeNode::OnStrAdd (GUIControl* sender)
{
	int node_num = GetChildsCount ();
	if (node_num > 0)
	{
		// Добавленному узлу, дописываем указатель на парента...
		node_num -= 1;
		Childs[node_num]->Parent = this;
	}
	
}

//----------------------------------------------------------------


GUITreeNodes::GUITreeNodes () : Items (_FL_, 128)
{
 bSystemClear = false;
/*
 t_OnAdd = NEW GUIEventHandler;
 t_OnClear = NEW GUIEventHandler;
*/

 t_OnAdd = &_add;
 t_OnClear = &_clear;

 Clear ();
}

void GUITreeNodes::ReleaseChilds ()
{
 int dwChildsToDelete = GetCount();

 for (int n = 0; n < dwChildsToDelete; n++)
 {
	 Items[0]->Release();
 }
}

void GUITreeNodes::Sort (bool (*FuncCompare)(GUITreeNode* const &a1, GUITreeNode* const &a2))
{
	Items.QSort(FuncCompare);

	for (DWORD n = 0; n < Items.Size(); n++)
	{
		Items[n]->Childs.Sort(FuncCompare);
	}
}

void GUITreeNodes::SortEx (bool (*FuncCompare)(GUITreeNode* const &a1, GUITreeNode* const &a2), int (*NeedToSortChilds)(GUITreeNode* const &a1))
{
	Items.QSort(FuncCompare);

	for (DWORD n = 0; n < Items.Size(); n++)
	{
		if (NeedToSortChilds(Items[n]) <= 0) continue;

		Items[n]->Childs.SortEx(FuncCompare, NeedToSortChilds);
	}

}



GUITreeNodes::~GUITreeNodes ()
{
 bSystemClear = true;
 //delete t_OnAdd;
 //delete t_OnClear;
 Clear ();
// delete t_OnAdd;
}
	
int GUITreeNodes::Add (GUITreeNode* n)
{
	Items.Add (n);
	t_OnAdd->Execute (NULL);

	n->OwnerList = this;
	return (Items.Size ()-1);
}
	

GUITreeNode* GUITreeNodes::FindItemByData (void* Data)
{
	for (DWORD n = 0; n < Items.Size (); n++)
	{
		if (Items[n]->Data == Data) return Items[n];
	}

	return NULL;
}



GUITreeNode* GUITreeNodes::FindItemByName (const char* name)
{
	for (DWORD n = 0; n < Items.Size (); n++)
	{
		if (string::IsEqual(Items[n]->GetText(), name)) return Items[n];
	}

	return NULL;
}

void GUITreeNodes::Clear ()
{
 if (!bSystemClear) t_OnClear->Execute (NULL);
 ReleaseChilds ();
 Items.DelAll ();
}
	
int GUITreeNodes::GetCount ()
{
	return Items.Size ();
}
	
GUITreeNode* GUITreeNodes::Get(int index)
{
	if (Items.Size() > 0)
	{
		if (index >= (int)Items.Size())
		{
			return Items[Items.Size()-1];
		}

		if (index < 0)
		{
			return null;
		}

		return Items[index];
	}

	return null;	
}
	
void GUITreeNodes::Insert (int index, GUITreeNode* n)
{
	Items.Insert (n,index);
}
	
void GUITreeNodes::Delete (int index)
{
	Items.DelIndex (index);
}
	
GUITreeNode* GUITreeNodes::operator[] (int index)
{
	return Get (index);
}


//----------------------------------------------------------------


GUITreeView::GUITreeView (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	_finishChar = 0;

	inDragInDrop = false;
	
	NeedScrolToSelection = true;
	SelectionDrawY = 0;
	TimeFromLastPressed = 0.0f;
	Flat = false;
	ImageForCopyIcon = NEW GUIImage ();
	ImageForCopyIcon->Load ("copyicon");
	SystemMDown = false;
	DragedNode = NULL;
	bDragAndDrop = false;
	item_to_center = NULL;
	exp_node = NULL;
	NeedHorizScroll = false;
	Items = NEW GUITreeNodes ();
  ClickTime = 0;
	OffsetY = 0;
	OffsetX = 0;

	printed_width = 0;
	sel_node = NULL;

	for (int n =0; n < MAX_IMAGES; n++)
	{
		ImagesArray[n] = NEW GUIImage;
	}


	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;
	ClientRect.Height -= 19;
	ClientRect.Width -= 19;


	sbHoriz = NEW GUIScrollBar (this, GUISBKIND_Horizontal, 0, Height-19, ((Width-3)-16), 19);
	sbHoriz->OnChange = (CONTROL_EVENT)&GUITreeView::sbHorizChange;
	sbHoriz->Min = 0;
	sbHoriz->Max = 10;
	
	sbVert = NEW GUIScrollBar (this, GUISBKIND_Vertical, Width-20, 0, 20, ((Height-2)-16));
	sbVert->OnChange = (CONTROL_EVENT)&GUITreeView::sbVertChange;
	sbVert->Min = 0;
	sbVert->Max = 10;

	

	Items->Clear ();
	Items->OnClear = (CONTROL_EVENT)&GUITreeView::ItemsBeforeClear;
	
	
	
	FontColor = 0xFF000000;
	pFont = NEW GUIFont ("Courier");

	t_OnRightSelect = NEW GUIEventHandler;  
	t_OnBeforeChange = NEW GUIEventHandler;  
	t_OnDblClick = NEW GUIEventHandler;  
	t_OnExpand = NEW GUIEventHandler;  
	t_OnColapse = NEW GUIEventHandler;  

	t_OnChange = NEW GUIEventHandler;

	t_OnFind = NEW GUIEventHandler;

	t_AfterDrop = NEW GUIEventHandler;
	t_BeforeDrop = NEW GUIEventHandler;

	t_OnCopyNode = NEW GUIEventHandler;

	IControlsService * ctrlsrv = (IControlsService *)api->GetService("ControlsService");
	Assert(ctrlsrv);
	ctrl = ctrlsrv->CreateInstance(_FL_);
	Assert(ctrl);

}

GUITreeView::~GUITreeView ()
{
  delete ImageForCopyIcon; 
	delete Items;
  delete sbHoriz;
  delete sbVert;
	delete t_OnDblClick;
	delete t_OnCopyNode;
	delete t_BeforeDrop;
	delete t_AfterDrop;
  delete t_OnChange;
  delete t_OnFind;
  delete t_OnColapse;
  delete t_OnExpand;
	delete t_OnBeforeChange;
	delete t_OnRightSelect;

	for (int n =0; n < MAX_IMAGES; n++)
	{
		delete ImagesArray[n];
	}


  

	delete pFont;

	RELEASE(ctrl)

}


void GUITreeView::Sort (bool (*FuncCompare)(GUITreeNode* const &a1, GUITreeNode* const &a2))
{
	Items->Sort(FuncCompare);
}

void GUITreeView::SortEx (bool (*FuncCompare)(GUITreeNode* const &a1, GUITreeNode* const &a2), int (*NeedToSortChilds)(GUITreeNode* const &a1))
{
	Items->SortEx(FuncCompare, NeedToSortChilds);
}


void GUITreeView::SetImagesArray (const char* filename)
{
	static char Temp4K[4096];
	for (int n =0; n < MAX_IMAGES; n++)
	{
		crt_snprintf (Temp4K, 4095, "%s%02d", filename, n);
		ImagesArray[n]->Load (Temp4K);
	}

}


bool GUITreeView::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
 	if (Visible == false) return false;

	bool Handled = false;
	
	Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	if (Handled) return Handled;
	
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);

	if (message == GUIMSG_WHEEL_UP)
	{
		ProcessKeyDown (38);
	}

	if (message == GUIMSG_WHEEL_DOWN)
	{
		ProcessKeyDown (40);
	}

	

	if (message == GUIMSG_KEYPRESSED && hparam == 1) 
	{
		ProcessKeyDown ((int)lparam);
	}

	if (message == GUIMSG_LMB_DOWN) 
	{
		Handled = true;
		OnMDown (message, cursor_pos);
	}

	if (message == GUIMSG_RMB_DOWN) 
	{
		Handled = true;
		OnMDown (message, cursor_pos);

		t_OnRightSelect->Execute(this);
	}



	if (message == GUIMSG_LMB_DBLCLICK) 
	{
		GUITreeNode* node = this->GetSelectedNode();
		if (node)
		{
			if (!t_OnDblClick->Execute(this))
			{
				node->Expanded = !node->Expanded;
				exp_node = node;
				if (node->Expanded)	t_OnExpand->Execute (this);  
				if (!node->Expanded) t_OnColapse->Execute (this);  
			}
		}
	}


	return Handled;
	
}


void GUITreeView::Draw ()
{
	if (Visible == false) return;

	TimeFromLastPressed += api->GetDeltaTime();
	
	ClickTime += api->GetDeltaTime();

	cliper.Push ();
	
	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	
	GUIRectangle rect = GetClientRect ();
	this->ClientToScreen (rect);
	
	
	
	GUIPoint np;
	np.X = DrawRect.Left;
	np.Y = DrawRect.Top;
	ClientToScreen (np);
	
	int nLeft = np.X;
	nTop = np.Y;
	nHeight = DrawRect.Height;


	if (OffsetY < 0)
	{
		OffsetY = 0;
	}
	int PrintY = nTop - OffsetY;


	
	if (NeedHorizScroll)
	{

		sbHoriz->Visible = true;
		ClientRect = DrawRect;
		ClientRect.Height -= 19;
		ClientRect.Width -= 19;
		nHeight = DrawRect.Height;

		sbVert->ChangeSize (sbVert->GetDrawRect().Width, ((Height-2)-16));

	} else
		{
			OffsetX = 0;
			sbHoriz->Visible = false;
			ClientRect = DrawRect;
			ClientRect.Width -= 19;
			nHeight = DrawRect.Height+19;

			sbVert->ChangeSize (sbVert->GetDrawRect().Width, Height+1);
		}

	

	

	rect.Width--;
	GUIRectangle clip_rect = rect;
	clip_rect.x += 1;
	clip_rect.y += 1;
	clip_rect.w -= 2;
	clip_rect.h -= 2;
	cliper.SetRectangle (rect);

	
//if (NeedHorizScroll)
//{
	GUIHelper::Draw2DRect (nLeft, nTop, DrawRect.Width-19, ClientRect.Height, 0xFFFFFFFF);
//} else
//{
//	GUIHelper::Draw2DRect (nLeft, nTop, DrawRect.Width-19, ClientRect.Height, 0xFFFF0000);
//}

  if (!Flat)
		GUIHelper::DrawDownBorder (nLeft, nTop, DrawRect.Width, DrawRect.Height);
	else
		GUIHelper::DrawWireRect (nLeft, nTop, DrawRect.Width, DrawRect.Height, 0x00000000);

	printed_lines = 0;
	printed_width = 0;

	int LastDrawn = 1;
	if (Items->GetCount () <= 1) LastDrawn = 0; 
	NeedHorizScroll = NeedHorizScrollBar (*(Items), PrintY, nLeft+3-OffsetX, 0);

	cliper.SetRectangle (clip_rect);
	int NewPrintY = DrawText (*(Items), PrintY, nLeft+3-OffsetX, 0, LastDrawn);
	int DeltaY = NewPrintY-PrintY;
	int TotalRows = DeltaY / pFont->GetHeight ();

	if (TotalRows > 1)	sbVert->Max = TotalRows;
	if (printed_width != 0)	sbHoriz->Max = printed_width;


	if (NeedScrolToSelection)
	{
		if (SelectionDrawY < nTop)
		{
			OffsetY -= (nTop-SelectionDrawY);
		}

		if (SelectionDrawY > (nTop+nHeight-20-pFont->GetHeight()))
		{
			OffsetY += (SelectionDrawY-(nTop+nHeight-20-pFont->GetHeight()));
		}

		NeedScrolToSelection = false;

	}
	
	UpdateScrollBars ();
	
	cliper.Pop ();
	GUIControl::Draw ();
	
}

bool GUITreeView::NeedHorizScrollBar (GUITreeNodes& items, int fromY, int formX, int level)
{
	for (int n = 0; n < items.GetCount (); n++)
	{
		const char* txt = items[n]->GetText();
		if (txt != NULL && txt[0] != 0) 
		{
			int addX = ((level+2) * 16);
			int RightLen = pFont->GetWidth (txt);
			RightLen+=addX;
			RightLen+=OffsetX;
			if (RightLen > DrawRect.Width-19)
			{
				return true;
			}
		}

		if ((items[n]->HasChildren () == true) && (items[n]->Expanded))
		{
			bool result = NeedHorizScrollBar (items[n]->Childs, fromY, formX, (level+1));
			if (result) return true;
		}
	}

	return false;
}

int GUITreeView::DrawText (GUITreeNodes& items, int fromY, int formX, int level, DWORD BitMask)
{
	for (int n = 0; n < items.GetCount (); n++)
	{
		if (GetSelectedNode() == items[n])
		{
			SelectionDrawY = fromY;
		}



		int theLastDrawn = 0;

    if ((n+1) == items.GetCount ()) theLastDrawn = 1;
		const char* txt = items[n]->GetText();
		
		int addX = 0;
		for (int z =0; z < level; z++)
		{
			int a = (BitMask >> z) & 0x1;
			GUIImage* i = ImagesArray[2];
			if (a)
			{
				// -19
				if ((fromY >= nTop-pFont->GetHeight()) && ((fromY) <= (nTop+nHeight-19)))
				{
					GUIHelper::DrawSprite (formX+addX, fromY, i->GetWidth(), i->GetHeight(), i);
				}
			}
			addX+= i->GetWidth ();
		}




		if (txt != NULL && txt[0] != 0) 
		{
			static char realPrintBuffer[8192];
			const char* buf = txt;

			if (buf)
			{
				int ctr = 0;
				bool bInsideFinishChar = false;
				for (int ct = 0;;ct++)
				{
					if (buf[ct] == _finishChar)
					{
						bInsideFinishChar = !bInsideFinishChar;
					} else
					{
						if (!bInsideFinishChar)
						{
							realPrintBuffer[ctr] = buf[ct];
							ctr++;
						}
					}

					if (buf[ct] == 0) break;
				}

				realPrintBuffer[ctr] = 0;
			} else
			{
				realPrintBuffer[0] = 0;
			}

			
			int image_index = 1;
			GUIImage* i = ImagesArray[image_index];
			
			// Имеет детей, ставим картинку [-]
			if (items[n]->HasChildren ())
			{
				image_index = 4;
				i = ImagesArray[image_index];
			}

			// Имеет детей и первый в списке, ставим картинку [-] (без верха)
			if ((n == 0) && (items[n]->HasChildren ()) && (level == 0))
			{
				image_index = 3;
				i = ImagesArray[image_index];
			}

			// Имеет детей и последний в списке, ставим картинку [-] (без низа)
			if (((n+1) == items.GetCount ()) && (items[n]->HasChildren ()))
			{
				image_index = 5;
				i = ImagesArray[image_index];
			}

			// посреди или сначала списка и нет детей
			if ((n >= 0) && ((n+1) < items.GetCount ()) && (!items[n]->HasChildren ()))
			{
				image_index = 1;
				i = ImagesArray[image_index];
			}

			// последний в списке и нет детей
			if (((n+1) == items.GetCount ()) && (!items[n]->HasChildren ()))
			{
				image_index = 0;
				i = ImagesArray[image_index];
			}

			if ((items[n]->HasChildren ()) && (!items[n]->Expanded))
			{
				image_index += 3;
				i = ImagesArray[image_index];
			}

			if ((fromY >= nTop-pFont->GetHeight()) && ((fromY) < (nTop+nHeight-19)))
			{
				GUIHelper::DrawSprite (formX+addX, fromY, i->GetWidth(), i->GetHeight(), i);
			}

			int subAddX = 0;
			
			GUIImage* img = items[n]->Image;

			if ((items[n]->Expanded) && (items[n]->ExpandedImage->GetTexture())) img = items[n]->ExpandedImage;
			if ((items[n]->Selected) && (items[n]->ExpandedImage->GetTexture()))	img = items[n]->SelectedImage;
			

			
			if ((fromY >= nTop-pFont->GetHeight()) && ((fromY) <= (nTop+nHeight-19)))
			{
				GUIHelper::DrawSprite (formX+addX+i->GetWidth(), fromY, img->GetWidth(),img->GetHeight(), img);
			}
			subAddX = img->GetWidth ();




			//-19
			if ((fromY >= nTop-pFont->GetHeight()) && ((fromY) <= (nTop+nHeight-19)))
			{
				DWORD savedcolor = FontColor;
				if (items[n]->bUseCustomColor) FontColor = items[n]->Color;
				if (items[n]->bReadOnly)
				{
					FontColor = ((FontColor & 0xFFFFFF) | (0x90 << 24));
				}

				pFont->Print (formX+i->GetWidth()+addX+subAddX, fromY, FontColor, "%s", realPrintBuffer);
				FontColor = savedcolor;
			}

			//_finishChar
			

			int curent_width = strlen (realPrintBuffer);
			curent_width+=level + 4;
			if (printed_width < curent_width)	printed_width = curent_width;
   		printed_lines++;

//---------------------
			if (items[n]->Selected == true)
			{
					GUIHelper::Draw2DRect2 ( formX+addX+i->GetWidth()+subAddX, 
																	 fromY, 
																	 pFont->GetWidth (realPrintBuffer), 
																	 pFont->GetHeight (), 
			                             0xFF404040);


			}
//---------------------

			fromY+=pFont->GetHeight();
		}

		if ((items[n]->HasChildren () == true) && (items[n]->Expanded))
		{
			if (!theLastDrawn)
			{
				BitMask = BitMask | (0x1 << level);
			} else
			{
				DWORD mask = 0xFFFFFFFF ^ (0x1 << level);
				BitMask = BitMask & mask;
			}

			int newY = DrawText (items[n]->Childs, fromY, formX, (level+1), BitMask);
			fromY = newY;
		}
	}

	return fromY;
}


bool GUITreeView::OnMDown (int MouseKey, const GUIPoint& pt)
{
//	if (ClickTime < 0.2f) return;
	GUIPoint g_point = pt;
	this->ScreenToClient (g_point);

	CursorUnderNodeButton = false;
	NodeUnderCursor = NULL;
	ProcessMDown (g_point, *Items,  -OffsetY, -OffsetX, 0);


	if ((NodeUnderCursor) && (CursorUnderNodeButton) && (!inDragInDrop))
	{
//********************** Обрабатываем схлопывание/захлопывание
		GetMainControl ()->Application->SetKeyboardFocus(this);
		NodeUnderCursor->Expanded = !NodeUnderCursor->Expanded;

		exp_node = NodeUnderCursor;
		if (!SystemMDown)
		{
			if (NodeUnderCursor->Expanded)	t_OnExpand->Execute (this);  
			if (!NodeUnderCursor->Expanded) t_OnColapse->Execute (this);  
		}

		// Выбрать схлопнутый узел, если схлопнули...
		if (NodeUnderCursor->Expanded == false)
		{
			t_OnBeforeChange->Execute(this);
			GUITreeNode* m_item = NodeUnderCursor;
			ResetSelection (*Items);
			m_item->Selected = true;
			sel_node = m_item;
			if (!SystemMDown) t_OnChange->Execute (this);
		}
		ClickTime = 0;


//********************** Обрабатываем схлопывание/захлопывание
	}

	if ((NodeUnderCursor) && (!CursorUnderNodeButton))
	{
//********************** Обрабатываем выделение узла
		t_OnBeforeChange->Execute(this);
		
		GetMainControl ()->Application->SetKeyboardFocus(this);
		ResetSelection (*Items);
		NodeUnderCursor->Selected = true;
		sel_node = NodeUnderCursor;
		ClickTime = 0;
		if (!SystemMDown) t_OnChange->Execute (this);
//********************** Обрабатываем выделение узла
	}

	return true;
}

int GUITreeView::ProcessMDown (const GUIPoint& pt, GUITreeNodes& items, int fromY, int formX, int level)
{
	for (int n = 0; n < items.GetCount (); n++)
	{
		int addX = 0;
		for (int z =0; z < level; z++)
		{
			GUIImage* i = ImagesArray[2];
			//GUIHelper::DrawSprite (formX+addX, fromY, i->GetWidth(), i->GetHeight(), i);
			addX+= i->GetWidth ();
		}


		const char* txt = items[n]->GetText();
		if (txt != NULL && txt[0] != 0) 
		{
			int image_index = 1;
			GUIImage* i = ImagesArray[image_index];
			
			// Имеет детей, ставим картинку [-]
			if (items[n]->HasChildren ())
			{
				image_index = 4;
				i = ImagesArray[image_index];
			}

			// Имеет детей и первый в списке, ставим картинку [-] (без верха)
			if ((n == 0) && (items[n]->HasChildren ()) && (level == 0))
			{
				image_index = 3;
				i = ImagesArray[image_index];
			}

			// Имеет детей и последний в списке, ставим картинку [-] (без низа)
			if (((n+1) == items.GetCount ()) && (items[n]->HasChildren ()))
			{
				image_index = 5;
				i = ImagesArray[image_index];
			}

			// посреди или сначала списка и нет детей
			if ((n >= 0) && ((n+1) < items.GetCount ()) && (!items[n]->HasChildren ()))
			{
				image_index = 1;
				i = ImagesArray[image_index];
			}

			// последний в списке и нет детей
			if (((n+1) == items.GetCount ()) && (!items[n]->HasChildren ()))
			{
				image_index = 0;
				i = ImagesArray[image_index];
			}

			if ((items[n]->HasChildren ()) && (!items[n]->Expanded))
			{
				image_index += 3;
				i = ImagesArray[image_index];
			}

//-------------------------------------- add-----------------
			// Если щелкаем в [+] или в [-]
			if (image_index >= 3)
			{
				GUIRectangle spr_rect;
				spr_rect.Left = formX+addX;
				spr_rect.Top = fromY;
				spr_rect.Width = i->GetWidth();
				spr_rect.Height = i->GetHeight();

				if (GUIHelper::PointInRect (pt, spr_rect))
				{
					CursorUnderNodeButton = true;
					NodeUnderCursor = items[n];
					

				}
			}




			int subAddX = 0;
			if (items[n]->Image)
			{
				GUIImage* img = items[n]->Image;
				subAddX = img->GetWidth ();
			}

			GUIRectangle fnt_rect;
			fnt_rect.Left = formX+addX+i->GetWidth();
			fnt_rect.Top = fromY;
			fnt_rect.Width = pFont->GetWidth (txt)+subAddX;
			fnt_rect.Height = pFont->GetHeight ();

			if (GUIHelper::PointInRect (pt, fnt_rect))
			{
				CursorUnderNodeButton = false;
				NodeUnderCursor = items[n];

			}
		
			fromY+=pFont->GetHeight();
		}

		if ((items[n]->HasChildren () == true) && (items[n]->Expanded))
		{
			int newY = ProcessMDown (pt, items[n]->Childs, fromY, formX, (level+1));
			fromY = newY;
		}
	}

	return fromY;

}


void GUITreeView::ResetSelection (GUITreeNodes& items)
{
	sel_node = NULL;
	for (int n = 0; n < items.GetCount (); n++)
	{
		items[n]->Selected = false;
	  if (items[n]->HasChildren () == true)
		{
		 ResetSelection (items[n]->Childs);
		}
	}
}


GUITreeNode* GUITreeView::GetSelectedNode ()
{
	return sel_node;
}

void GUITreeView::SetSelectedNode (GUITreeNode* node, int NeedCenter)
{
	t_OnBeforeChange->Execute(this);
	if (sel_node) sel_node->Selected = false;
	sel_node = node;
	if (node)
	{
		node->Selected = true;
		if (NeedCenter) CenterItem (node);
	}
	t_OnChange->Execute (this);
}


void GUITreeView::UpdateScrollBars ()
{
	GUIRectangle rect = GetClientRect ();
	this->ClientToScreen (rect);
	rect.Left += 2;
	rect.Top += 2;
	rect.Width -=4;
	rect.Height -=4;

	int symbY = rect.Height / pFont->GetHeight ();
	sbVert->PageSize = symbY;

	//sbVert->Min = 0;
	//sbVert->Max = printed_lines;
}

void _cdecl GUITreeView::sbVertChange (GUIControl* sender)
{
	
	int ofs_pos = (sbVert->Position - sbVert->PageSize)+1;
	if (ofs_pos < 0) ofs_pos = 0;


	if (pFont)
	{
	OffsetY = ofs_pos * pFont->GetHeight ();
	} else
		{
			OffsetY = 0;
		}

//	OffsetY++;
}

void _cdecl GUITreeView::sbHorizChange (GUIControl* sender)
{
	
	int ofs_pos = (sbHoriz->Position - sbHoriz->PageSize)+1;
	if (ofs_pos < 0) ofs_pos = 0;


	if (pFont)
	{
	OffsetX = ofs_pos * pFont->GetHeight ();
	} else
		{
		OffsetX = 0;
		}

}


GUITreeNode* GUITreeView::GetExpandedOrCollapsedNode ()
{
	return exp_node;
}

void GUITreeView::SetExpandedOrCollapsedNode (GUITreeNode* node)
{
	exp_node = node;
	t_OnExpand->Execute (this);
}

void _cdecl GUITreeView::ItemsBeforeClear (GUIControl* sender)
{
	sel_node = NULL;
	exp_node = NULL;
	sbHoriz->Min = -1;
	sbHoriz->Max = 0;
	sbHoriz->Position = 0;
	sbHorizChange (NULL);
	
	sbVert->Min = -1;
	sbVert->Max = 0;
	sbVert->Position = 0;
	sbVertChange (NULL);
}

GUITreeNode* GUITreeView::FindItemProcess (GUITreeNode* root, char* path)
{
	char* lpName = path;
	lpName++;
	int l = strlen (lpName);
	for (int q=0; q < l; q++)
	{
		if (lpName[q] == '\\')
		{
			lpName[q] = 0;
			// lpName - это имя "директории"
			// (lpName + q) - продолжается имя;
			for (int n = 0; n < root->Childs.GetCount (); n ++)
			{
				// Есть такое имя у детей, идем дальше...
				if (string::IsEqual (root->Childs[n]->GetText(), lpName))
			 {
				 return FindItemProcess (root->Childs[n], (lpName + q));
			 }
			}
		}
	}


	for (int n = 0; n < root->Childs.GetCount (); n ++)
	{
		// Есть такое имя у детей, идем дальше...
	 const char* Name = root->Childs[n]->GetText();
	 if (string::IsEqual (Name, lpName))
	 {
		 // Нашли конечный узел = root->Childs[n]
		 t_OnFind->Execute (root->Childs[n]);
		 return root->Childs[n];
	 }
	}


	return NULL;
}

GUITreeNode* GUITreeView::FindItem (const char* path)
{
	char TempPath[4096];
	crt_strncpy (TempPath, 4096, path, 4095);

	int y = strlen (TempPath) - 1;
	if (TempPath[y] == '\\') TempPath[y] = 0;



	char* lpName = TempPath;
	if (lpName[0] == '\\') lpName++;
	int l = strlen (lpName);
	for (int q=0; q < l; q++)
	{
		if (lpName[q] == '\\')
		{
			lpName[q] = 0;
			for (int n = 0; n < Items->GetCount(); n ++)
			{
				// Есть такое имя у детей, идем дальше...
				if (string::IsEqual (Items->Get(n)->GetText(), lpName))
			 {
				 return FindItemProcess (Items->Get(n), (lpName + q));
			 }
			}

		}
	}


	for (int n = 0; n < Items->GetCount(); n ++)
	{
		// Есть такое имя у детей, идем дальше...
	 if (string::IsEqual (Items->Get(n)->GetText(), lpName))
	 {
		 // Нашли, этот узел оказываеться в корне...
     t_OnFind->Execute (Items->Get(n));
     return Items->Get(n);
	 }
	}

return NULL;

}

void GUITreeView::CenterItem (GUITreeNode* item)
{
	item_to_center = item;

	GUIPoint np;
	np.X = DrawRect.Left;
	np.Y = DrawRect.Top;
	ClientToScreen (np);
	
	int nLeft = np.X;
	nTop = np.Y;

	CenterItemProcess (*(Items), 0);
}

bool GUITreeView::CenterItemProcess (GUITreeNodes& items, int PrintedLines)
{
	for (int n = 0; n < items.GetCount (); n++)
	{

		if (items[n] == item_to_center) 
		{
			sbVert->Position = PrintedLines + sbVert->PageSize-1;
			//if (sbVert->Position < sbVert->Min) sbVert->Position = sbVert->Min;
			//if (sbVert->Position > sbVert->Max) sbVert->Position = sbVert->Max;
			//api->Trace ("Position - %d", sbVert->Position);
			//api->Trace ("Min - %d", sbVert->Min);
			//api->Trace ("Max - %d", sbVert->Max);
			sbVertChange (this);
			//api->Trace ("Position - %d\n", sbVert->Position);
			return true;
		}

		PrintedLines++;
		if ((items[n]->HasChildren () == true) && (items[n]->Expanded))
		{
			bool Result = CenterItemProcess (items[n]->Childs, PrintedLines);
			if (Result) return true;
		}
	}

	return false;
}

bool GUITreeView::IsParentIs (GUITreeNode* NeedParent, GUITreeNode* Node)
{
	if (Node->Parent == NeedParent) return true;
	if (Node->Parent) return IsParentIs (NeedParent, Node->Parent);
	return false;
}

bool GUITreeView::DragOver (int MouseX, int MouseY, GUIControl* dragfrom)
{
//CanCopy
	if (ctrl->GetControlStateType("DragCopyButton") == CST_ACTIVE)
	{
		if (DragedNode->CanCopy == false) return false;
	}


	ResetSelection (*Items);
	GUIPoint mpos;
	mpos.x = MouseX;
	mpos.y = MouseY;
	SystemMDown = true;
	OnMDown (GUIMSG_LMB_DOWN, mpos);
	SystemMDown = false;

	GUITreeNode* tmpNode = GetSelectedNode ();
	if (!tmpNode)
	{
		
		return true;
	}
	if (IsParentIs(DragedNode, tmpNode)) return false;
	if (tmpNode->CanDrop == false) return false;
	
	return true;
}


bool GUITreeView::DragBegin ()
{
	if (!api->DebugKeyState(VK_CONTROL) && !api->DebugKeyState(VK_SHIFT)) return false;

	DragedNode = GetSelectedNode ();
	if (!DragedNode) return false;
	if (DragedNode->CanDrag == false) return false;
	DragedNode->Selected = false;
	sel_node = NULL;
	GetMainControl ()->Application->SetDragAndDropCursorCustomDraw (this, (CUSTOM_CURSORDRAW)&GUITreeView::CustomDraw);
	inDragInDrop = true;
	return true;
}


bool _cdecl GUITreeView::CustomDraw (int mouseX, int mouseY)
{
	mouseX+= 15;
	mouseY+= 10;
	int SizeX = DragedNode->Image->GetWidth();
	int SizeY = DragedNode->Image->GetHeight();

	if (ctrl->GetControlStateType("DragCopyButton") == CST_ACTIVE)
	{
		int ifc_width = ImageForCopyIcon->GetWidth ();
		int ifc_height = ImageForCopyIcon->GetHeight ();
		GUIHelper::DrawSprite (mouseX+SizeX+4, mouseY+pFont->GetHeight ()+2, ifc_width, ifc_height, ImageForCopyIcon);
	}


	GUIHelper::DrawSprite (mouseX, mouseY, SizeX, SizeY, DragedNode->Image);
	pFont->Print (mouseX+SizeX+4, mouseY, 0xFF000000, "%s", DragedNode->GetText());
	
	return true;
}


//-----------------------------------------------------------------------
void GUITreeView::DragDrop (int MouseX, int MouseY, GUIControl* dragfrom)
{
	if (dragfrom == NULL)
	{
		inDragInDrop = false;
		return;
	}

	if (ctrl->GetControlStateType("DragCopyButton") == CST_ACTIVE)
	{
		if (DragedNode->CanCopy == false)
		{
			inDragInDrop = false;
			return;
		}
	}


	GUITreeNode* tmpNode = GetSelectedNode ();

	BeforeDrop_CanDrop = true;
	t_BeforeDrop->Execute (tmpNode);
	if (!BeforeDrop_CanDrop)
	{
		inDragInDrop = false;
		return;
	}

	GUITreeView* tView = (GUITreeView*)dragfrom;
	if (tView->DragedNode == NULL)
	{
		inDragInDrop = false;
		return; 
	}
	if (tView->DragedNode != DragedNode)
	{
		inDragInDrop = false;
		return; // Между деревьями нельзя таскать
	}
	if (tmpNode)
	{
		if (tmpNode->CanDrop == false)
		{
			inDragInDrop = false;
			return;
		}
	}
	

	if (DragedNode == tmpNode)
	{
		inDragInDrop = false;
		return;
	}

	GUITreeNode* newBranch = NEW GUITreeNode;

	CopyBranch (newBranch, DragedNode);

	if (tmpNode != NULL) 
	{
		tmpNode->Childs.Add (newBranch);
	} else
		{
			Items->Add  (newBranch);
		}

	//!!!
	//if (!api->Controls->GetControlStateType("DragCopyButton") == CST_ACTIVE)
	if (ctrl->GetControlStateType("DragCopyButton") != CST_ACTIVE)
	{
		// Переносим !!!
		DragedNode->Release ();
	} else
		{
		// Копируем !!!
		// В параметрах узел который cкопировали? т.е. новый узел
			t_OnCopyNode->Execute (newBranch);
		}


	

	t_AfterDrop->Execute (newBranch);

	inDragInDrop = false;

	
}


GUITreeNode* GUITreeView::GetDragedItem ()
{
	return DragedNode;
}

GUITreeNode* GUITreeView::CopyBranch (GUITreeNode* destination, GUITreeNode* from)
{
	// Копируем...
	destination->SetText(from->GetText());
	destination->Image->Load (from->Image->GetName ());
	destination->Tag = from->Tag;
	destination->Data = from->Data;
	destination->Expanded = from->Expanded;
	destination->CanDrag = from->CanDrag;
	destination->CanDrop = from->CanDrop;
	destination->CanCopy = from->CanCopy;
	
	int ChildsCount = from->Childs.GetCount ();
	for (int n = 0; n < ChildsCount; n++)
	{
		GUITreeNode* newChild = NEW GUITreeNode;
		CopyBranch (newChild, from->Childs.Get (n));
		destination->Childs.Add (newChild);
	}

	return destination;
}

/*	if (!Flat)
	{
		GUIHelper::DrawEditBox (nLeft, nTop, Width, Height);
		return;
	}

	GUIHelper::Draw2DRect(nLeft, nTop, Width, Height, 0xFFFFFFFF);
	GUIHelper::DrawWireRect(nLeft, nTop, Width, Height, 0xFF000000);
*/

void GUITreeView::MakeFlat (bool enable)
{
	sbVert->MakeFlat (enable);
	sbHoriz->MakeFlat (enable);
	Flat = enable;
}

GUITreeNode* GUITreeView::GetNodeAtPos (const GUIPoint& pt)
{
	GUIPoint g_point = pt;
	this->ScreenToClient (g_point);
	CursorUnderNodeButton = false;
	NodeUnderCursor = NULL;
	ProcessMDown (g_point, *Items,  -OffsetY, -OffsetX, 0);
	return NodeUnderCursor;
}

void GUITreeView::ProcessKeyDown (int key)
{
	if (TimeFromLastPressed < 0.07f) return;
	/* DOWN */
	if (key == 40)
	{
		MoveSelectionToDown ();      
	}


	/* UP */
	if (key == 38)
	{
		MoveSelectionToUp ();
	}

	/*plus*/
	if (false)
	{
		NodeUnderCursor = GetSelectedNode();
		NodeUnderCursor->Expanded = true;

		exp_node = NodeUnderCursor;
		if (!SystemMDown)
		{
			if (NodeUnderCursor->Expanded)	t_OnExpand->Execute (this);  
			if (!NodeUnderCursor->Expanded) t_OnColapse->Execute (this);  
		}

		// Выбрать схлопнутый узел, если схлопнули...
		if (NodeUnderCursor->Expanded == false)
		{
			t_OnBeforeChange->Execute(this);
			GUITreeNode* m_item = NodeUnderCursor;
			ResetSelection (*Items);
			m_item->Selected = true;
			sel_node = m_item;
			if (!SystemMDown) t_OnChange->Execute (this);
		}
		ClickTime = 0;

	}

	if (false)
	{
		NodeUnderCursor = GetSelectedNode();
		NodeUnderCursor->Expanded = false;

		exp_node = NodeUnderCursor;
		if (!SystemMDown)
		{
			if (NodeUnderCursor->Expanded)	t_OnExpand->Execute (this);  
			if (!NodeUnderCursor->Expanded) t_OnColapse->Execute (this);  
		}

		// Выбрать схлопнутый узел, если схлопнули...
		if (NodeUnderCursor->Expanded == false)
		{
			t_OnBeforeChange->Execute(this);
			GUITreeNode* m_item = NodeUnderCursor;
			ResetSelection (*Items);
			m_item->Selected = true;
			sel_node = m_item;
			if (!SystemMDown) t_OnChange->Execute (this);
		}
		ClickTime = 0;

	}
}

int GUITreeView::utilGetIndexInNodes (GUITreeNode* node, GUITreeNodes* nodes)
{
	for (int n = 0; n < nodes->GetCount(); n++)
	{
		if (nodes->Get(n) == node) return n;
	}
	return -1;
}

void GUITreeView::MoveSelectionToDown ()
{
	NeedScrolToSelection = true;
	TimeFromLastPressed = 0.0f;
	GUITreeNode* tn = GetSelectedNode();


	if (!tn) return;

	if ((tn->Expanded) && (tn->Childs.GetCount() > 0))
	{
		t_OnBeforeChange->Execute(this);
		ResetSelection (*Items);
		tn->Childs[0]->Selected = true;
		sel_node = tn->Childs[0];
		ClickTime = 0;
		if (!SystemMDown) t_OnChange->Execute (this);
		return;
	}

	GUITreeNodes* nodes = Items;
	if (tn->Parent)	nodes = &tn->Parent->Childs;

	// Если выбран последний нод на подуровне
	int index = utilGetIndexInNodes (tn, nodes);
	if ((nodes->GetCount()-1 == index) && (tn->Parent))
	{
		t_OnBeforeChange->Execute(this);
		GUITreeNodes* nodes2 = Items;
		if (tn->Parent->Parent)	nodes2 = &tn->Parent->Parent->Childs;
		int index2 = utilGetIndexInNodes (tn->Parent, nodes2);
		ResetSelection (*Items);
		nodes2->Get(index2+1)->Selected = true;
		sel_node = nodes2->Get(index2+1);
		ClickTime = 0;
		if (!SystemMDown) t_OnChange->Execute (this);
	} else
		{
			t_OnBeforeChange->Execute(this);
			ResetSelection (*Items);
			nodes->Get(index+1)->Selected = true;
			sel_node = nodes->Get(index+1);
			ClickTime = 0;
			if (!SystemMDown) t_OnChange->Execute (this);
		}
}

void GUITreeView::MoveSelectionToUp ()
{
	NeedScrolToSelection = true;
	TimeFromLastPressed = 0.0f;
	GUITreeNode* tn = GetSelectedNode();

	if (!tn) return;

	GUITreeNodes* nodes = Items;
	if (tn->Parent)	nodes = &tn->Parent->Childs;

	int index = utilGetIndexInNodes (tn, nodes);
	if ((index == 0) && (tn->Parent == NULL)) return;
	
	if ((index == 0) && (tn->Parent))
	{
		t_OnBeforeChange->Execute(this);
		GUITreeNodes* nodes2 = Items;
		if (tn->Parent->Parent)	nodes2 = &tn->Parent->Parent->Childs;
		int index2 = utilGetIndexInNodes (tn->Parent, nodes2);
		ResetSelection (*Items);
		nodes2->Get(index2)->Selected = true;
		sel_node = nodes2->Get(index2);
		ClickTime = 0;
		if (!SystemMDown) t_OnChange->Execute (this);
	} else
		{
			if (nodes->Get(index-1)->Expanded == false)
			{
				t_OnBeforeChange->Execute(this);
				ResetSelection (*Items);
				nodes->Get(index-1)->Selected = true;
				sel_node = nodes->Get(index-1);
				ClickTime = 0;
				if (!SystemMDown) t_OnChange->Execute (this);
			} else
			{
				if (nodes->Get(index-1)->Childs.GetCount() > 0)
				{
					t_OnBeforeChange->Execute(this);
					GUITreeNode* need = nodes->Get(index-1)->Childs.Get(nodes->Get(index-1)->Childs.GetCount()-1);
					ResetSelection (*Items);
					need->Selected = true;
					sel_node = need;
					ClickTime = 0;
					if (!SystemMDown) t_OnChange->Execute (this);
				}
			}
		}

}


void GUITreeView::ChangeWidth(int newWidth)
{
	DrawRect.Width = newWidth;


	if (!NeedHorizScroll)
	{
		ClientRect = DrawRect;
		ClientRect.Width -= 19;
	} else
	{
		ClientRect = DrawRect;
		ClientRect.Height -= 19;
		ClientRect.Width -= 19;
	}

	
	//newWidth -= 10;

	sbHoriz->SetDrawRect(GUIRectangle(0, DrawRect.Height-19, ((newWidth-3)-16), 19));
	sbVert->SetDrawRect(GUIRectangle(newWidth-20, 0, 20, ((DrawRect.Height-2)-16)));




}

void GUITreeView::SetFinishingChar (char finishChar)
{
	_finishChar = finishChar;
}

#endif