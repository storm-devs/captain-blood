#ifndef _XBOX

#include "gui_edit.h"
#include "gui_helper.h"
#include "..\gui_imanager.h"


static char cTempBuffer[16384];


// Сделать скролл...
GUIEdit::GUIEdit (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	PressedInside = false;
	Flat = false;
  need_accept_message = false;
	bAlreadyPoped = false;
	ChangeCursor = true;
	//DWORD color;
	//DWORD font_color;
	
	same_keys_pressed = 0;
	
	need_key_event = 0;
	
	ScrollOffset = 0;
	
	CursorPosition = 0;
	nSelStart = 0;
	nSelLength = 0;
	
	sCursor = true;
	
	
	pFont = NEW GUIFont ("SansSerif");
	
	key_pressed_time = 0;
	BlinkTime = 0;
	lastkeypressed = 0;
	
	CursorInside = false;
	
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;
	
	
	FontColor = 0xFF000000;
	SelectionColor = 0xFF404040;
	
	t_OnMouseDown = NEW GUIEventHandler;
	t_OnMouseUp = NEW GUIEventHandler;
	
	t_OnMouseEnter = NEW GUIEventHandler;
	t_OnMouseLeave = NEW GUIEventHandler;
	
	t_OnChange = NEW GUIEventHandler;
	
	t_OnAccept = NEW GUIEventHandler;

	t_OnAdvNumChange = NEW GUIEventHandler;
	
	
}

GUIEdit::~GUIEdit ()
{
	GetMainControl ()->Application->SetKeyboardFocus (NULL);
	
	if (ChangeCursor)
	{
		if (!bAlreadyPoped)	GetMainControl ()->Application->GetCursor ()->Pop ();
		bAlreadyPoped = true;
	}

	delete pFont;
	delete t_OnMouseDown;
	delete t_OnMouseUp;
	
	delete t_OnMouseEnter;
	delete t_OnMouseLeave;
	
	delete t_OnChange;
	
	delete t_OnAccept;

	delete t_OnAdvNumChange;
	
//	GUIControl::~GUIControl ();
}

void GUIEdit::Draw ()
{
	if (Visible == false) return;
	
	
	float dt = api->GetDeltaTime();
	key_pressed_time += dt;
	BlinkTime += dt;
	
	if (BlinkTime > 0.5)
	{
		BlinkTime = 0;
		sCursor = !sCursor;
	}
	
	
	
	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	
	
	GUIRectangle rect = GetClientRect ();
	this->ClientToScreen (rect);
	//	cliper.SetRectangle (rect);
	
	
	
	GUIPoint np;
	np.X = DrawRect.Left;
	np.Y = DrawRect.Top;
	ClientToScreen (np);
	
	int nLeft = np.X;
	int nTop = np.Y;
	
	int TextWidth = 0;
	int TextHeight = pFont->GetHeight ();
	
	if (!Text.IsEmpty ())
	{
		TextWidth = pFont->GetWidth (Text);
		
	}
	
	GUIRectangle font_rect;
	font_rect.Left = nLeft;
	font_rect.Top = nTop;
	font_rect.Width = TextWidth;
	font_rect.Height = TextHeight;
	GUIPoint font_point;
	GUIHelper::Center (font_point, font_rect, rect);
	
	
	
	DrawEditBox (nLeft, nTop, DrawRect.Width, DrawRect.Height);
	
	cliper.Push ();
	
	
	GUIRectangle rect2 = rect;
	rect2.Left += 2;
	rect2.Top += 2;
	rect2.Width -= 4;
	rect2.Height -= 4;
	cliper.SetRectangle (rect2);
	
	
	int subAddX = 0;
	if (!Text.IsEmpty())
	{
		GUIHelper::DrawSprite (nLeft+3-ScrollOffset, font_point.Y, Image.GetWidth(), Image.GetHeight(), &Image);
		subAddX = Image.GetWidth ()+2;
		
		pFont->Print (nLeft+3-ScrollOffset+subAddX, font_point.Y, FontColor, "%s", Text);
	}
	
	
	
	//cliper.SetFullScreenRect ();
	
	if (nSelLength != 0)
	{
		int qSelForm = nSelStart;
		int qSelTo = nSelStart+nSelLength;
		
		if (qSelTo < qSelForm) 
		{
			int tmp = qSelForm;
			qSelForm = qSelTo;
			qSelTo = tmp;
		}
		
		if (qSelForm < 0) qSelForm = 0;
		if (qSelTo > (int)Text.Len()) qSelTo = Text.Len();
		
		crt_strncpy (cTempBuffer, 16384, Text, 16383);
		cTempBuffer[qSelTo] = 0;
		int cursor_pos_width = pFont->GetWidth (cTempBuffer+qSelForm);
		
		crt_strncpy (cTempBuffer, 16384, Text, 16383);
		cTempBuffer[qSelTo] = 0;
		
		
		int cursor_pos_big = pFont->GetWidth (cTempBuffer);
		int cursor_pos_from = (cursor_pos_big - cursor_pos_width);
		
		crt_strncpy (cTempBuffer, 16384, Text, 16383);
		cTempBuffer[qSelForm] = 0;
		int cursor_pos_from2 = pFont->GetWidth (cTempBuffer);
		
		if ((cursor_pos_from2 < cursor_pos_from) && (nSelLength > 0))
		{
			cursor_pos_from = cursor_pos_from2;
		}
		
		
		
		
		GUIHelper::Draw2DRect2 ( nLeft+6+cursor_pos_from-ScrollOffset, 
			font_point.Y, 
			(cursor_pos_width), 
			pFont->GetHeight(), 
			SelectionColor);
	}
	
	
	if ((CursorInside) && (sCursor))
	{
		crt_strncpy (cTempBuffer, 16384, Text, 16383);
		cTempBuffer[CursorPosition] = 0;
		int cursor_pos = pFont->GetWidth (cTempBuffer);
		int subAddX = Image.GetWidth ()+2;
		cursor_pos+=subAddX;
		
		GUIHelper::DrawVertLine (font_point.Y, pFont->GetHeight(), nLeft+4+cursor_pos-ScrollOffset, 0xFF000000);
	}
	
	
	cliper.Pop ();
	GUIControl::Draw ();
}

void GUIEdit::DrawEditBox (long nLeft, long nTop, long Width, long Height)
{
	if (!Flat)
	{
		GUIHelper::DrawEditBox (nLeft, nTop, Width, Height);
		return;
	}

	GUIHelper::Draw2DRect(nLeft, nTop, Width, Height, 0xFFFFFFFF);
	GUIHelper::DrawWireRect(nLeft, nTop, Width, Height, 0xFF000000);
}


bool GUIEdit::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (Visible == false) return false;
	
	bool Handled = false;
	
	Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	if (Handled) return Handled;
	
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);
	
	if ((message == GUIMSG_KEYPRESSED) && (CursorInside))
  {
    need_accept_message = false;
		Handled = true;
		OnKPressed ((int)lparam, hparam);
    CalcScrollOffset ();
    if (need_key_event) t_OnChange->Execute (this);
    if (need_accept_message) t_OnAccept->Execute (this);
	}
	
	
	if (message == GUIMSG_LMB_DOWN) 
	{
		Handled = true;
		OnMDown (message, cursor_pos);
	}
	
	if (message == GUIMSG_LMB_UP) 
	{
		Handled = true;
		OnMUp (message, cursor_pos);
	}
	
	if (message == GUIMSG_MOUSEENTER) 
	{
		Handled = true;
		OnMEnter ();
		mouse_inside = true;
	}
	
	if (message == GUIMSG_MOUSELEAVE) 
	{
		Handled = true;
		OnMLeave ();
		mouse_inside = false;
	}
	
	
	if (message == GUIMSG_LOSTKEYBOARDFOCUS) 
	{
		mouse_inside = false;
		CursorInside = false;
		nSelLength = 0;
	}
	
	
	return Handled;
}

void GUIEdit::OnMDown (int MouseKey, const GUIPoint& pt)
{
	t_OnMouseDown->Execute (this);
	
	// Сделали элемент активным...
	CursorInside = true;
	PressedInside = true;
	CursorPosition = 0;
	
	GUIControl* g_ctl = GetMainControl ();
	g_ctl->Application->SetKeyboardFocus (this);
	
	GUIPoint g_point = pt;
	this->ScreenToClient (g_point);
	int pX = g_point.X;
	pX -=4;
	
	crt_strncpy (cTempBuffer, 16384, Text, 16383);
	
	int subAddX = Image.GetWidth ()+2;
	for (unsigned long u=0; u < Text.Len(); u++)
	{
		char save = cTempBuffer[u];
		cTempBuffer[u] = 0;
		int cursor_pos_width = pFont->GetWidth (cTempBuffer);
		cursor_pos_width += subAddX;
		if (cursor_pos_width > pX) break;
		cTempBuffer[u] = save;
	}
	
	
	CursorPosition = u;
}

void GUIEdit::OnMUp (int MouseKey, const GUIPoint& pt)
{
	t_OnMouseUp->Execute (this);
}


void GUIEdit::OnMEnter ()
{
	if (ChangeCursor)
	{
		GetMainControl ()->Application->GetCursor ()->Push ();
		GetMainControl ()->Application->GetCursor ()->SetCursor ("textcursor", -8);
		bAlreadyPoped = false;
	}
	t_OnMouseEnter->Execute (this);
}


void GUIEdit::OnMLeave ()
{
	if (ChangeCursor)
	{
		if (!bAlreadyPoped)	GetMainControl ()->Application->GetCursor ()->Pop ();
		bAlreadyPoped = true;
	}
	t_OnMouseLeave->Execute (this);
}


void GUIEdit::OnKPressed (int key, dword hparam)
{
	if (key <= 4) return;
	need_key_event = true;
	
	if (!CursorInside) 
	{
		need_key_event = false;
		return;
	}
	
	int ShiftState = 0;
	int ControlState = 0;
	if (GetAsyncKeyState(VK_SHIFT) < 0) 
	{
		ShiftState = 1;
	}
	
	if (GetAsyncKeyState(VK_CONTROL) < 0) 
	{
		ControlState = 1;
	}
	

	if (hparam == 1)
	{
	
		if ((key == 36) && (!ShiftState)) /* HOME */
		{
			nSelLength = 0;
			CursorPosition = 0;
			need_key_event = false;
			return;
		}
		
		if ((key == 35) && (!ShiftState)) /* END */
		{
			nSelLength = 0;
			CursorPosition = Text.Len ();
			need_key_event = false;
			return;
		}
		
		
		if ((key == 36) && (ShiftState)) /* HOME */
		{
			if (nSelLength == 0) nSelStart = CursorPosition;
			CursorPosition = 0;
			nSelLength = CursorPosition - nSelStart;
			need_key_event = false;
			return;
		}
		
		if ((key == 35) && (ShiftState)) /* END */
		{
			if (nSelLength == 0) nSelStart = CursorPosition;
			CursorPosition = Text.Len ();
			nSelLength = CursorPosition - nSelStart;
			need_key_event = false;
			return;
		}
		
		
		
		/* CTRL + INSERT */
		if (((key == 45) && (ControlState) && (!ShiftState)) ||
			((key == 'C') && (!ShiftState) && (ControlState)))
		{
			int qSelForm = nSelStart;
			int qSelTo = nSelStart+nSelLength;
			
			if (qSelTo < qSelForm) 
			{
				int tmp = qSelForm;
				qSelForm = qSelTo;
				qSelTo = tmp;
			}
			
			if (qSelForm < 0) qSelForm = 0;
			if (qSelTo > (int)Text.Len()) qSelTo = Text.Len();
			
			crt_strncpy (cTempBuffer, 16384, Text, 16383);
			cTempBuffer[qSelTo] = 0;
			
			GUIHelper::SetClipboardText (cTempBuffer+qSelForm, (HWND)api->Storage().GetLong("system.hwnd"));
			return;
		}
		
		// SHIFT - INSERT
		if (((key == 45) && (ShiftState) && (!ControlState)) ||
				((key == 'V') && (!ShiftState) && (ControlState)))
		{
			DeleteSelected ();
			GUIHelper::GetClipboardText (cTempBuffer, (HWND)api->Storage().GetLong("system.hwnd"));
			Text.Insert (CursorPosition, cTempBuffer);
			return;
		}
		
		
		
		
		if ((key == VK_DELETE) && (ShiftState) && (!ControlState)) /* SHIFT + DELETE */
		{
			int qSelForm = nSelStart;
			int qSelTo = nSelStart+nSelLength;
			
			if (qSelTo < qSelForm) 
			{
				int tmp = qSelForm;
				qSelForm = qSelTo;
				qSelTo = tmp;
			}
			
			if (qSelForm < 0) qSelForm = 0;
			if (qSelTo > (int)Text.Len()) qSelTo = Text.Len();
			
			crt_strncpy (cTempBuffer, 16384, Text, 16383);
			cTempBuffer[qSelTo] = 0;
			
			
			GUIHelper::SetClipboardText (cTempBuffer+qSelForm, (HWND)api->Storage().GetLong("system.hwnd"));
			
			DeleteSelected ();
			return;
		}
		
		
		if ((key == VK_LEFT) && (!ShiftState))
		{
			CursorPosition--;
			if (CursorPosition < 0 ) CursorPosition = 0;
			
			nSelLength = 0;
			sCursor = true;
			return;
			
		}
		
		if ((key == VK_RIGHT) && (!ShiftState))
		{
			CursorPosition++;
			if (CursorPosition > (int)Text.Len() ) CursorPosition = Text.Len();
			
			nSelLength = 0;
			sCursor = true;
			return;
		}
		
		
		if ((key == VK_LEFT) && (ShiftState))
		{
			CursorPosition--;
			if (CursorPosition < 0 ) 
			{
				CursorPosition = 0;
			} else
			{
				if (nSelLength == 0) nSelStart = (CursorPosition+1);
				nSelLength--;
			}
			sCursor = true;
			need_key_event = false;
			return;
		}
		
		
		if ((key == VK_RIGHT) && (ShiftState))
		{
			CursorPosition++;
			if (CursorPosition > (int)Text.Len() ) 
			{
				CursorPosition = Text.Len();
			} else
			{
				if (nSelLength == 0) nSelStart = (CursorPosition-1);
				nSelLength++;
			}
			
			sCursor = true;
			need_key_event = false;
			return;
		}
		
		
		if (key == VK_DELETE  && nSelLength != 0)
		{
			DeleteSelected ();
			return;
		}
		
		if (key == VK_DELETE)
		{
			if ((CursorPosition) < (int)Text.Len())
			{
				Text.Delete (CursorPosition, 1);
				return;
			}
			need_key_event = false;
			return;
		}
	} else
		{
			if (key == VK_RETURN)
			{
				need_accept_message = true;
				return;
			}


			if (key == VK_BACK  && nSelLength != 0)
			{
				DeleteSelected ();
				return;
			}

			if (key == VK_BACK)
			{
				if (CursorPosition > 0)
				{
					Text.Delete ((CursorPosition-1), 1);
					CursorPosition--;
					return;
				}
				need_key_event = false;
				return;
			}



			DeleteSelected ();
			char szText[2];
			szText[0] = (char)key;
			szText[1] = 0;

			if (key >= 32)
			{
				Text.Insert (CursorPosition, szText);
				CursorPosition++;
			}

			lastkeypressed = key;
		}

}

void GUIEdit::CursorToEnd()
{
	CursorPosition = Text.Len();
}


//VK_BACK

void GUIEdit::DeleteSelected ()
{
	if (nSelLength != 0)
	{
		int qSelForm = nSelStart;
		int qSelTo = nSelStart+nSelLength;
		
		if (qSelTo < qSelForm) 
		{
			int tmp = qSelForm;
			qSelForm = qSelTo;
			qSelTo = tmp;
		}
		
		Text.Delete (qSelForm, (qSelTo-qSelForm));
		nSelLength = 0;
		CursorPosition = qSelForm;
	}
	
}


void GUIEdit::CalcScrollOffset ()
{
	
	
	crt_strncpy (cTempBuffer, 16384, Text, 16383);
	cTempBuffer[CursorPosition] = 0;
	int cursor_pos = pFont->GetWidth (cTempBuffer);
	cursor_pos=(cursor_pos+3-ScrollOffset);
	
	GUIRectangle g_rect = GetClientRect ();
	
	g_rect.Left = 2;
	g_rect.Width -= 6;
	if (g_rect.Width < 0) g_rect.Width = 0;
	int delta = 0;
	if (cursor_pos < g_rect.Left) delta = cursor_pos-g_rect.Left;
	if (cursor_pos > (g_rect.Left+g_rect.Width)) delta = cursor_pos-(g_rect.Left+g_rect.Width);
	//if (delta != 0) _asm int 3;
	ScrollOffset += delta;
	
}

void GUIEdit::SelectText (int From, int Length)
{
	nSelStart = From;
	nSelLength = Length;
	CursorPosition = Length;
}
/*
void GUIEdit::DragDrop (int MouseX, int MouseY, GUIControl* dragfrom)
{
	if (ChangeCursor)
	{
		GetMainControl ()->Application->GetCursor ()->Pop ();
		bAlreadyPoped = true;
	}

}
*/

void GUIEdit::MouseUp (int button, const GUIPoint& pt)
{
	PressedInside = false;
}

void GUIEdit::MouseMove (int button, const GUIPoint& pt)
{
	if (!PressedInside) return;
	t_OnAdvNumChange->Execute(this);
}

#endif