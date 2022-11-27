#ifndef _XBOX
#include "gui_combobox.h"




GUIComboBox::GUIComboBox (GUIControl* pParent, int Left, int Top, int Width, int Height, GUIButton * userButton, GUIEdit * userEdit, GUIListBox * userList) : GUIControl (pParent)
{
	GUIRectangle rect;
	bUserCanTypeText = false;
	
	rect.x = Width-Height;
	rect.y = 0;
	rect.w = Height-1;
	rect.h = Height;
	if(!userButton)
	{
		Button = NEW GUIButton (this, rect.x, rect.y, rect.w, rect.h);
	}else{
		Button = userButton;
		AddChild (Button);
		//rect.pos += ClientRect.pos;
		Button->SetDrawRect(rect);
		Button->SetClientRect(rect);
	} 
	Button->Glyph->Load ("down");
	Button->OnMousePressed = (CONTROL_EVENT)&GUIComboBox::ButtonPressed;
	
	rect.x = 0;
	rect.y = 0;
	rect.w = Width-Height;
	rect.h = Height;
	if(!userEdit)
	{
		Edit = NEW GUIEdit (this, rect.x, rect.y, rect.w-1, rect.h);
	}else{
		Edit = userEdit;
		AddChild (userEdit);
		//rect.pos += ClientRect.pos;
		Edit->SetDrawRect(rect);
		Edit->SetClientRect(rect);
	}
	Edit->OnChange = (CONTROL_EVENT)&GUIComboBox::OnContentsChange;
	Edit->ChangeCursor = false;
	
	rect.x = 0;
	rect.y = Height+2;
	rect.w = Width;
	rect.h = 100;	
	if(!userList)
	{
		ListBox = NEW GUIListBox (this, rect.x, rect.y, rect.w, rect.h);
	}else{
		ListBox = userList;
		AddChild (userList);
		//rect.pos += ClientRect.pos;
		ListBox->SetDrawRect(rect);
		ListBox->SetClientRect(rect);
	}
	ListBox->AutoSelectMode = true;
	ListBox->Visible = false;
	ListBox->OnMouseDown = (CONTROL_EVENT)&GUIComboBox::ChangeElement;
	//ListBox->OnChange = (CONTROL_EVENT)ChangeElement;
	
	
	//
	/*
	Edit->Text = "Text1";
	ListBox->Items.Add ("Item1");
	ListBox->Items.Add ("Item2");
	ListBox->Items.Add ("Item3");
	ListBox->Items.Add ("Item4");
	ListBox->Items.Add ("Item5");
	ListBox->Items.Add ("Item6");
	ListBox->Items.Add ("Item7");
	ListBox->Items.Add ("Item8");
	ListBox->Items.Add ("Item9");
	ListBox->Items.Add ("Item10");
	ListBox->Items.Add ("Item11");
	ListBox->Items.Add ("Item12");
	ListBox->Items.Add ("Item13");
	*/
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;
	
	OriginalRect = DrawRect;
	
	
	t_OnChange = NEW GUIEventHandler;

	t_OnSelect = NEW GUIEventHandler;
	
	
}

GUIComboBox::~GUIComboBox ()
{
	delete t_OnSelect;
	delete t_OnChange;
	delete Button;
	delete Edit;
	delete ListBox;
}



bool GUIComboBox::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (Visible == false) return false;
	
	bool Handled = false;
	
	
	if (message == GUIMSG_LMB_DOWN)
	{
		if (bUserCanTypeText)
		{
			Handled = GUIControl::ProcessMessages (message, lparam, hparam);
		} else
		{
			// Смотрим не попали ли мы мышой в EditBox
			// Если попали, нужно раскрыть список	
			GUIPoint cursor_pos;
			GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);
			
			GUIRectangle gr = Edit->GetDrawRect ();
			Edit->ClientToScreen (gr);
			if (GUIHelper::PointInRect(cursor_pos, gr))
			{
				GUIRectangle lb = Button->GetDrawRect ();
				Button->ClientToScreen (lb);
				GUIPoint cpos;
				cpos.X = lb.Left + 8;
				cpos.Y = lb.Top + 8;
				
				
				// Перенаправляем на кнопку
				Button->OnMDown (message, cpos);
				Button->OnMUp (GUIMSG_LMB_UP, cpos);
				
				Handled = true;
				
			} else
			{
				Handled = GUIControl::ProcessMessages (message, lparam, hparam);
			}
			
			
		}
	} else
	{
		Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	}
	
	
	return Handled;
	
}


void GUIComboBox::Draw ()
{
	Edit->ChangeCursor = bUserCanTypeText;
	GUIControl::Draw ();
}

void _cdecl GUIComboBox::ButtonPressed (GUIControl* sender)
{
	ListBox->Visible = !ListBox->Visible;
	if (ListBox->Visible)
	{
		bAlwaysOnTop = true;
		GetMainControl ()->Application->SetActiveComboBox(this);
	} else
	{
		bAlwaysOnTop = false;
		GetMainControl ()->Application->SetActiveComboBox(NULL);
	}
	
	
	if (ListBox->Visible == false)
	{
		DrawRect = OriginalRect;
		ClientRect = DrawRect;
	} else
	{
		DrawRect = OriginalRect;
		DrawRect.Height += ListBox->GetDrawRect().Height;
		ClientRect = DrawRect;
	}
}

void _cdecl GUIComboBox::ChangeElement (GUIControl* sender)
{
	t_OnSelect->Execute(this);
	if (ListBox->SelectedLine != -1)
	{
		Edit->Text = ListBox->Items[ListBox->SelectedLine];
		
		int cline = ListBox->SelectedLine;
		int tlines = ListBox->Images.Size();
		
		if ((cline >= 0) && (cline < tlines))
		{
			Edit->Image.Load (ListBox->Images[cline].GetName());
		} else
		{
			Edit->Image.Delete ();
		}
		
		
		OnContentsChange (this);
	}
	
	if (ListBox->Visible)	ButtonPressed (sender);
	
	/*
	GUIControl* ctrl = GetMainControl ();
	ctrl->Application->SetDelay2NextMouseEvent (0.3f);
	*/
}

void _cdecl GUIComboBox::OnContentsChange (GUIControl* sender)
{
	t_OnChange->Execute (this);
}

void GUIComboBox::SelectItem (int num)
{
	ListBox->SelectedLine = num;

	if (num < 0 || ListBox->Items.Size() <= 0)
	{
		Edit->Image.Delete();
		Edit->Text = "";
		t_OnSelect->Execute (this); 
		t_OnChange->Execute (this);
		return;

	}
		
	string a = ListBox->Items[num];
	bool bNeedImageLoad = true;
	if (((DWORD)num >= ListBox->Images.Size()) || (num < 0)) bNeedImageLoad = false;
	
	Edit->Text = a;

	if (bNeedImageLoad)
	{
		GUIImage* n = &ListBox->Images[num];
		Edit->Image.Load (n->GetName ());
	}

	t_OnSelect->Execute (this); 
	t_OnChange->Execute (this);
}


void GUIComboBox::MakeFlat (bool Flat)
{
	Button->FlatButton = Flat;
	ListBox->MakeFlat(true);
	Edit->Flat = Flat;
}
#endif