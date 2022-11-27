#ifndef _XBOX

#include "gui_memo.h"
#include "gui_helper.h"
#include "..\gui_imanager.h"


static char cTempBuffer16K[16384];
static char cTempBuffer64K[65535];



GUIMemo::GUIMemo (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	
	//DWORD color;
	//DWORD font_color;
	
	cur_line = 0;
	
	same_keys_pressed = 0;
	
	need_key_event = 0;
	
	ScrollOffset = 0;
	
	OffsetY = 0;
	
	CursorPosition = 0;
	nSelStart = 0;
	nSelLength = 0;
	
	sCursor = true;
	
	
	//*****************[ Откуда - докуда selection ]**************
	ResetMultiline ();
	//***********************************
	
	
	pFont = NEW GUIFont ("Courier");
	
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
	
	t_OnMouseDown = NEW GUIEventHandler;
	t_OnMouseUp = NEW GUIEventHandler;
	
	t_OnMouseEnter = NEW GUIEventHandler;
	t_OnMouseLeave = NEW GUIEventHandler;
	
	t_OnChange = NEW GUIEventHandler;
	
	Lines.Clear ();
	
	
	
	ClientRect = DrawRect;
	ClientRect.Height -= 19;
	ClientRect.Width -= 19;
	sbHoriz = NEW GUIScrollBar (this, GUISBKIND_Horizontal, 0, Height-19, ((Width-3)-16), 19);
	
	sbVert = NEW GUIScrollBar (this, GUISBKIND_Vertical, Width-20, 0, 20, ((Height-2)-16));
	sbVert->Position = 0;
	sbHoriz->Position = 0;
	
	
	sbHoriz->OnChange = (CONTROL_EVENT)&GUIMemo::sbHorizChange;
	sbVert->OnChange = (CONTROL_EVENT)&GUIMemo::sbVertChange;
	
	Lines.OnAdd = (CONTROL_EVENT)&GUIMemo::OnStrAdd;
	
	
}

GUIMemo::~GUIMemo ()
{
	GetMainControl ()->Application->GetCursor ()->Pop ();

  delete t_OnMouseDown;
	delete t_OnMouseUp;
	
	delete t_OnMouseEnter;
	delete t_OnMouseLeave;

  delete t_OnChange;
	delete pFont;
	
	delete sbHoriz;
	delete sbVert;
	
//	GUIControl::~GUIControl ();
}

void GUIMemo::Draw ()
{
	if (Visible == false) return;

	string& Text = Lines[cur_line];
	
	
	
	//SelectFromY = 0;
	//SelectSizeY = 0;
	//MultiLine = false;
	/*IRender* rs = (IRender*)api->CreateService("DX9Render");
	rs->Print (100, 50, 0xFFFFFFFF, "Multiline - %d",   MultiLine);
	rs->Print (100, 70, 0xFFFFFFFF, "SelectFromY - %d", SelectFromY);
	rs->Print (100, 90, 0xFFFFFFFF, "SelectSizeY - %d", SelectSizeY);
	rs->Print (100, 110, 0xFFFFFFFF, "nSelStart - %d",  nSelStart);
	rs->Print (100, 130, 0xFFFFFFFF, "nSelLength - %d", nSelLength);
	rs->Print (100, 150, 0xFFFFFFFF, "nSelEnd - %d", nSelEnd);
	
	*/
	
	
	//if (MultiLine) FontColor = 0xFFFF0000; else FontColor = 0xFF00FF00; 
	
	float dt = api->GetDeltaTime();
	key_pressed_time += dt;
	BlinkTime += dt;
	
	if (BlinkTime > 0.5)
	{
		BlinkTime = 0;
		sCursor = !sCursor;
	}
	
	
//	cliper.Push ();
	
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
	
	
//	cliper.Push ();
//	cliper.SetFullScreenRect ();
	GUIHelper::DrawDownBorder (nLeft, nTop, DrawRect.Width, DrawRect.Height);
//	cliper.Pop ();
	GUIHelper::DrawEditBox (nLeft, nTop, DrawRect.Width, DrawRect.Height-19, false);
	
	GUIRectangle rect2 = rect;
	rect2.Left += 2;
	rect2.Top += 2;
	rect2.Width -= 4;
	rect2.Height -= 4;
//	cliper.SetRectangle (rect2);
	
	// Уводим печать на верх...
	font_point.Y = nTop-OffsetY;
	

  cliper.Push ();
	cliper.SetRectangle (rect2);

	for (int u=0;u< Lines.Size();u++)
	{
		//pFont->Print (nLeft+3-ScrollOffset, font_point.Y+(TextHeight*u), FontColor, "%s", Text);
		string& cl = Lines[u];
		int line_height = font_point.Y+(TextHeight*u);
		if (((line_height) >= nTop) && (line_height+19 < (nTop+Height-TextHeight)))
		{
			if (!cl.IsEmpty()) pFont->Print (nLeft+3-ScrollOffset, font_point.Y+(TextHeight*u), FontColor, "%s", cl.GetBuffer());
		}
	}
	

	cliper.Pop ();
	//if (!Text.IsEmpty())
	//{
		//     pFont->Print (nLeft+3-ScrollOffset, font_point.Y, FontColor, "%s", Text);
	// }
	
	
	
	//cliper.SetFullScreenRect ();
	
	// Подсветить мултивыделение...
	if (MultiLine)
	{
		/*
		rs->Print (100, 50, 0xFFFFFFFF, "Multiline - %d",   MultiLine);
		rs->Print (100, 70, 0xFFFFFFFF, "SelectFromY - %d", SelectFromY);
		rs->Print (100, 90, 0xFFFFFFFF, "SelectSizeY - %d", SelectSizeY);
		rs->Print (100, 110, 0xFFFFFFFF, "nSelStart - %d",  nSelStart);
		rs->Print (100, 130, 0xFFFFFFFF, "nSelLength - %d", nSelLength);
		rs->Print (100, 150, 0xFFFFFFFF, "nSelEnd - %d", nSelEnd);
		*/
		int qSelFormY = SelectFromY;
		int qSelToY = SelectFromY+SelectSizeY;
		
		if (qSelToY < qSelFormY) 
		{
			int tmp = qSelFormY;
			qSelFormY = qSelToY;
			qSelToY = tmp;
		}
		
		
		int full_lines = (qSelToY-qSelFormY-1);
		if (full_lines > 0)
		{
			int hght = pFont->GetHeight ();
			for (int y=(qSelFormY+1); y < qSelToY; y++)
			{
				int wdth = pFont->GetWidth (Lines[y]);
				GUIHelper::Draw2DRect2 ( nLeft+3-ScrollOffset, 
				font_point.Y+(y*hght), 
				(wdth), hght, 
				0xFF404040);
				
			}
		}
		
		// Нужно дорисовать начало и окончание выделения...
		int bSelStart = nSelStart;
		int bSelEnd = nSelEnd;
		
		if (SelectSizeY < 0)
		{
			bSelStart = nSelEnd;
			bSelEnd = nSelStart;
		}
		
		
		crt_strncpy (cTempBuffer16K, 16384, Lines[qSelFormY], 16383);
		int AfterCursor = pFont->GetWidth (cTempBuffer16K+bSelStart);
		int TotalWidth = pFont->GetWidth (cTempBuffer16K);
		int StartFrom = (TotalWidth-AfterCursor);
		GUIHelper::Draw2DRect2 ( nLeft+3-ScrollOffset+StartFrom, 
		font_point.Y+(qSelFormY*pFont->GetHeight()), 
		(TotalWidth-StartFrom), pFont->GetHeight(), 
		0xFF404040);
		
		
		crt_strncpy (cTempBuffer16K, 16384, Lines[qSelToY], 16383);
		AfterCursor = pFont->GetWidth (cTempBuffer16K+bSelEnd);
		TotalWidth = pFont->GetWidth (cTempBuffer16K);
		StartFrom = (TotalWidth-AfterCursor);
		
		GUIHelper::Draw2DRect2 ( nLeft+3-ScrollOffset, 
		font_point.Y+(qSelToY*pFont->GetHeight()), 
		(StartFrom), pFont->GetHeight(), 
		0xFF404040);
		
		
	}
	
	if ((nSelLength != 0) && (!MultiLine))
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
		
		crt_strncpy (cTempBuffer16K, 16384, Text, 16383);
		cTempBuffer16K[qSelTo] = 0;
		int cursor_pos_width = pFont->GetWidth (cTempBuffer16K+qSelForm);
		
		crt_strncpy (cTempBuffer16K, 16384, Text, 16383);
		cTempBuffer16K[qSelTo] = 0;
		
		
		int cursor_pos_big = pFont->GetWidth (cTempBuffer16K);
		int cursor_pos_from = (cursor_pos_big - cursor_pos_width);
		
		crt_strncpy (cTempBuffer16K, 16384, Text, 16383);
		cTempBuffer16K[qSelForm] = 0;
		int cursor_pos_from2 = pFont->GetWidth (cTempBuffer16K);
		
		if ((cursor_pos_from2 < cursor_pos_from) && (nSelLength > 0))
		{
			cursor_pos_from = cursor_pos_from2;
		}
		
		
		
		
		GUIHelper::Draw2DRect2 ( nLeft+3+cursor_pos_from-ScrollOffset, 
		font_point.Y+(cur_line*TextHeight), 
		(cursor_pos_width), 
		pFont->GetHeight(), 
		0xFF404040);
	}
	
	
	if ((CursorInside) && (sCursor))
	{
		crt_strncpy (cTempBuffer16K, 16384, Text, 16383);
		cTempBuffer16K[CursorPosition] = 0;
		int cursor_pos = pFont->GetWidth (cTempBuffer16K);
		
		GUIHelper::DrawVertLine (font_point.Y+(cur_line*TextHeight), pFont->GetHeight(), nLeft+3+cursor_pos-ScrollOffset, 0xFF000000);
	}
	
	
//	cliper.Pop ();
	GUIControl::Draw ();
}


bool GUIMemo::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
 	if (Visible == false) return false;

	bool Handled = false;
	
	Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	if (Handled) return Handled;
	
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);
	
	if ((message == GUIMSG_KEYPRESSED) && (CursorInside))
	{
		Handled = true;
		OnKPressed ((int)lparam);
		CalcScrollOffset ();
		UpdateScrollBars ();
		if (need_key_event) t_OnChange->Execute (this);
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
		ResetMultiline ();
	}
	
	
	return Handled;
}

void GUIMemo::OnMDown (int MouseKey, const GUIPoint& pt)
{
	GUIPoint g_point = pt;
	this->ScreenToClient (g_point);
	
	
	int fntHeight = pFont->GetHeight ();
	int CurY = -OffsetY;
	bool usd = false;
	for (int n = 0; n < Lines.Size(); n++)
	{
		if ((g_point.Y >= CurY) && (g_point.Y < (CurY+fntHeight)))
		{
			cur_line = n; 
			usd = true;
			break;
		}
		
		CurY+=fntHeight;
	}
	
	
	if (!usd) 
	{
		cur_line = (Lines.Size()-1);
	}
	
	
	string& Text = Lines[cur_line];
	
	t_OnMouseDown->Execute (this);
	
	// Сделали элемент активным...
	CursorInside = true;
	CursorPosition = 0;
	
	GUIControl* g_ctl = GetMainControl ();
	g_ctl->Application->SetKeyboardFocus (this);
	
	int pX = g_point.X;
	pX -=4;
	
	crt_strncpy (cTempBuffer16K, 16384, Text, 16383);
	
	for (unsigned long u=0; u < Text.Len(); u++)
	{
		char save = cTempBuffer16K[u];
		cTempBuffer16K[u] = 0;
		int cursor_pos_width = pFont->GetWidth (cTempBuffer16K);
		if (cursor_pos_width > pX) break;
		cTempBuffer16K[u] = save;
	}
	
	
	CursorPosition = u;
	
	ResetMultiline ();
	nSelLength = 0;
	
	UpdateScrollBars ();
}

void GUIMemo::OnMUp (int MouseKey, const GUIPoint& pt)
{
	t_OnMouseUp->Execute (this);
}


void GUIMemo::OnMEnter ()
{
	GetMainControl ()->Application->GetCursor ()->Push ();
	GetMainControl ()->Application->GetCursor ()->SetCursor ("textcursor", -8);
	t_OnMouseEnter->Execute (this);
}


void GUIMemo::OnMLeave ()
{
	GetMainControl ()->Application->GetCursor ()->Pop ();
	t_OnMouseLeave->Execute (this);
}


void GUIMemo::OnKPressed (int key)
{
//FIX ME !
/*
	string& Text = Lines[cur_line];
	
	// 13 - Enter
	//40 - Down
	//38 - Up
	
	need_key_event = true;
	
	if ((key < 32) || (key > 90)) 
	{
		if ( (key != VK_LEFT) && 
		(key != VK_RIGHT) && 
		(key != VK_DELETE) && 
		(key != VK_BACK) && 
		(key != 190)  && 
		(key != 191)  && 
		(key != 186)  &&   
		(key != 222)  &&    
		(key != 219)  &&    
		(key != 221)  &&    
		(key != 189)  &&    
		(key != 187)  &&    
		(key != 220)  &&    
		(key != 192)  &&    
		(key != 13)  &&    

		(key != 188) )   
		{
			need_key_event = false;
			return;
		}
	}
	
	
	if ((key >= 33) && (key < 48)) 
	{
		if ((key != VK_LEFT) && 
		(key != VK_RIGHT) && 
		(key != VK_DELETE) && 
		(key != 36)  &&    
		(key != 35)  &&    
		(key != 45)  &&    
		(key != 40)  &&    
		(key != 38)  &&    
		(key != 33)  &&    
		(key != 34)  &&    

		(key != VK_BACK) )   
		{
			need_key_event = false;
			return;
		}
	}
	
	if ((key >= 58) && (key <= 64)) 
	{
		need_key_event = false;
		return;
	}
	
	
	if (lastkeypressed == key)
	{
		if (same_keys_pressed == 0)
		{
			if (key_pressed_time <= 0.35)
			{
				need_key_event = false;
				return;
			}
			
		}
		
		if (key_pressed_time <= 0.14) 
		{
			need_key_event = false;
			return;
		}
		same_keys_pressed++;
	} else
	{
		same_keys_pressed = 0;
	}
	
	if (key_pressed_time <= 0.08) 
	{
		need_key_event = false;
		return;
	}
	
	key_pressed_time = 0;
	
	char temptxt[2];
	temptxt[0] = (char)key;
	temptxt[1] = 0x0;
	
	
	
	
	if (!CursorInside) 
	{
		need_key_event = false;
		return;
	}
	
	sCursor = true;
	
	
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
	
	if (key == 33) 
	{
		cur_line -= sbVert->PageSize;
		if (cur_line < 0) cur_line = 0;
		CalcScrollOffset ();
		UpdateScrollBars ();
		return;
	}

	if (key == 34) 
	{
		cur_line += sbVert->PageSize;
		if (cur_line > (Lines.Size()-1)) cur_line = (Lines.Size()-1);
		CalcScrollOffset ();
		UpdateScrollBars ();

		return;
	}
	
	if (key == 13) 
	{
		nSelStart = CursorPosition;
		nSelLength = Text.Len () - nSelStart;
		
		string new_str;
		
		if (nSelLength > 0)
		{
			new_str.Format("%s", Text.GetBuffer() + nSelStart);
		}
		else
		{
			new_str = "";
		}
		
		DeleteSelected ();
		
		cur_line++;
		Lines.Insert (cur_line, new_str);
		
		CursorPosition = 0;
		
		return;
	}
	
	if ((key == 40) && (!ShiftState))
	{
		if (cur_line >= (int)(Lines.Size()-1)) return;
		cur_line++;
		
		if (CursorPosition > (int)Lines[cur_line].Len() ) CursorPosition = Lines[cur_line].Len();
		nSelLength = 0;
		ResetMultiline ();
		
		return;
	}
	
	if ((key == 38) && (!ShiftState))
	{
		if (cur_line <= 0) return;
		cur_line--;
		
		if (CursorPosition > (int)Lines[cur_line].Len() ) CursorPosition = Lines[cur_line].Len();
		nSelLength = 0;
		ResetMultiline ();
		
		return;
	}
	
	
	if ((key == 40) && (ShiftState)) 
	{
		if (cur_line >= (int)(Lines.Size()-1)) return;
		cur_line++;
		
		if (CursorPosition > (int)Lines[cur_line].Len() ) CursorPosition = Lines[cur_line].Len();
		//nSelLength = 0;
		
		if (SelectSizeY == 0 ) 
		{
			SelectFromY = cur_line-1;
			MultiLine = true;
		}
		
		SelectSizeY++;
		if (SelectSizeY == 0 ) ResetMultiline ();
		nSelEnd = CursorPosition;
		
		if (nSelLength == 0) nSelStart = CursorPosition;
		
		return;
	}
	
	if ((key == 38) && (ShiftState)) 
	{
		if (cur_line <= 0) return;
		cur_line--;
		
		if (CursorPosition > (int)Lines[cur_line].Len() ) CursorPosition = Lines[cur_line].Len();
		//nSelLength = 0;
		//MultiLine = true;
		if (SelectSizeY == 0 ) 
		{
			SelectFromY = cur_line+1;
			MultiLine = true;
		}
		
		SelectSizeY--;
		if (SelectSizeY == 0 ) ResetMultiline ();
		nSelEnd = CursorPosition;
		
		if (nSelLength == 0) nSelStart = CursorPosition;
		
		return;
	}
	
	
	
	if ((key == VK_BACK) && (CursorPosition == 0))
	{
		if (cur_line > 0)
		{
			int cp = Lines[(cur_line-1)].Len();
			Lines[(cur_line-1)] += Text;
			Lines.Delete (cur_line);
			
			CursorPosition = cp;
			cur_line--;
			//Text.Insert (0, "DELETE<-");
		}
		return;
	}
	
	if ((key == VK_DELETE) && (CursorPosition == (int)Text.Len()))
	{
		if (cur_line < (int)(Lines.Size()-1)) 
		{
			Text += Lines[(cur_line+1)];
			Lines.Delete ((cur_line+1));
		}
		return;
	}
	
	
	if ((key == 36) && (ControlState))
	{
		nSelLength = 0;
		ResetMultiline ();
		CursorPosition = 0;
		need_key_event = false;
		cur_line = 0;
		return;
	}
	
	if ((key == 35) && (ControlState))
	{
		cur_line = Lines.GetCount ();
		cur_line--;
		nSelLength = 0;
		ResetMultiline ();
		CursorPosition = Lines[cur_line].Len ();
		need_key_event = false;
		return;
	}
	
	if ((key == 36) && (!ShiftState))
	{
		nSelLength = 0;
		ResetMultiline ();
		CursorPosition = 0;
		need_key_event = false;
		return;
	}
	
	if ((key == 35) && (!ShiftState)) 
	{
		nSelLength = 0;
		ResetMultiline ();
		CursorPosition = Text.Len ();
		need_key_event = false;
		return;
	}
	
	
	if ((key == 36) && (ShiftState))
	{
		if (nSelLength == 0) nSelStart = CursorPosition;
		CursorPosition = 0;
		nSelLength = CursorPosition - nSelStart;
		need_key_event = false;
		return;
	}
	
	if ((key == 35) && (ShiftState)) 
	{
		if (nSelLength == 0) nSelStart = CursorPosition;
		CursorPosition = Text.Len ();
		nSelLength = CursorPosition - nSelStart;
		need_key_event = false;
		return;
	}
	
	
	
	if ((key == 45) && (ControlState) && (!ShiftState))
	{
		CopyToClipboard ();
		return;
	}
	
	if ((key == 45) && (ShiftState) && (!ControlState)) 
	{
		PasteFromClipboard ();
		return;
	}
	
	if ((key == VK_DELETE) && (ShiftState) && (!ControlState)) 
	{
		DeleteToClipboard ();
		return;
	}
	
	
	if ((key == VK_LEFT) && (!ShiftState))
	{
		CursorPosition--;
		if (CursorPosition < 0 ) CursorPosition = 0;
		
		nSelLength = 0;
		ResetMultiline ();
		return;
		
	}
	
	if ((key == VK_RIGHT) && (!ShiftState))
	{
		CursorPosition++;
		if (CursorPosition > (int)Text.Len() ) CursorPosition = Text.Len();
		
		nSelLength = 0;
		ResetMultiline ();
		return;
	}
	
	if (ShiftState) 
	{
		_strupr (temptxt);
	} else
	{
		_strlwr (temptxt);
	}
	
	
	if ((key == 190)&& (!ShiftState)) temptxt[0] = '.';
	if ((key == 188)&& (!ShiftState)) temptxt[0] = ',';
	if ((key == 190)&& (ShiftState)) temptxt[0] = '>';
	if ((key == 188)&& (ShiftState)) temptxt[0] = '<';
	if ((key == 191)&& (!ShiftState)) temptxt[0] = '/';
	if ((key == 191)&& (ShiftState)) temptxt[0] = '?';
	
	if ((key == 186)&& (!ShiftState)) temptxt[0] = ';';
	if ((key == 186)&& (ShiftState)) temptxt[0] = ':';
	
	if ((key == 222)&& (!ShiftState)) temptxt[0] = 0x27;
	if ((key == 222)&& (ShiftState)) temptxt[0] = 0x22;
	
	if ((key == 219)&& (!ShiftState)) temptxt[0] = '[';
	if ((key == 219)&& (ShiftState)) temptxt[0] = '{';
		
		if ((key == 221)&& (!ShiftState)) temptxt[0] = ']';
	if ((key == 221)&& (ShiftState)) temptxt[0] = '}';
	
	if ((key == 189)&& (!ShiftState)) temptxt[0] = '-';
	if ((key == 189)&& (ShiftState)) temptxt[0] = '_';
	
	if ((key == 187)&& (!ShiftState)) temptxt[0] = '=';
	if ((key == 187)&& (ShiftState)) temptxt[0] = '+';
	
	if ((key == 220)&& (!ShiftState)) temptxt[0] = 0x5C;
	if ((key == 220)&& (ShiftState)) temptxt[0] = '|';
	
	if ((key == '1')&& (!ShiftState)) temptxt[0] = '1';
	if ((key == '1')&& (ShiftState)) temptxt[0] = '!';
	
	if ((key == '2')&& (!ShiftState)) temptxt[0] = '2';
	if ((key == '2')&& (ShiftState)) temptxt[0] = '@';
	
	if ((key == '3')&& (!ShiftState)) temptxt[0] = '3';
	if ((key == '3')&& (ShiftState)) temptxt[0] = '#';
	
	if ((key == '4')&& (!ShiftState)) temptxt[0] = '4';
	if ((key == '4')&& (ShiftState)) temptxt[0] = '$';
	
	if ((key == '5')&& (!ShiftState)) temptxt[0] = '5';
	if ((key == '5')&& (ShiftState)) temptxt[0] = '%';
	
	if ((key == '6')&& (!ShiftState)) temptxt[0] = '6';
	if ((key == '6')&& (ShiftState)) temptxt[0] = '^';
	
	if ((key == '7')&& (!ShiftState)) temptxt[0] = '7';
	if ((key == '7')&& (ShiftState)) temptxt[0] = '&';
	
	if ((key == '8')&& (!ShiftState)) temptxt[0] = '8';
	if ((key == '8')&& (ShiftState)) temptxt[0] = '*';
	
	if ((key == '9')&& (!ShiftState)) temptxt[0] = '9';
	if ((key == '9')&& (ShiftState)) temptxt[0] = '(';
	
	if ((key == '0')&& (!ShiftState)) temptxt[0] = '0';
	if ((key == '0')&& (ShiftState)) temptxt[0] = ')';
	
	if ((key == 192)&& (!ShiftState)) temptxt[0] = '`';
	if ((key == 192)&& (ShiftState)) temptxt[0] = '~';
	
	
	
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
		
		nSelEnd = CursorPosition;
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
		
		nSelEnd = CursorPosition;
		return;
	}
	
	
	if (((key == VK_BACK) || (key == VK_DELETE)) && (nSelLength != 0))
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
	
	//  nSelStart = 0;
	//nSelLength = 0;
	
	
	
	DeleteSelected ();
	Text.Insert (CursorPosition, temptxt);
	CursorPosition++;
	
	
	lastkeypressed = key;
*/
}


//VK_BACK



void GUIMemo::CalcScrollOffset ()
{
	string& Text = Lines[cur_line];
	
	crt_strncpy (cTempBuffer16K, 16384, Text, 16383);
	cTempBuffer16K[CursorPosition] = 0;
	int cursor_pos = pFont->GetWidth (cTempBuffer16K);
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
	
	CalcYOffset ();
}


void GUIMemo::CalcYOffset ()
{
	int fHeight = pFont->GetHeight ();
	int cursorYpos = ((cur_line * fHeight) - OffsetY) + fHeight;
	
	GUIRectangle c_rect = GetClientRect ();
	
	if (cursorYpos > (c_rect.Height))
	{
		OffsetY = (cursorYpos+OffsetY) - (c_rect.Height);
	}
	
	if ((cursorYpos-fHeight) < 0)
	{
		OffsetY += (cursorYpos-fHeight);
	}
	
	int tmp = (int)((float)((float)OffsetY  / (float)fHeight) + 0.5f);
	OffsetY = tmp * fHeight;

}



void GUIMemo::ResetMultiline ()
{
	SelectFromY = 0;
	SelectSizeY = 0;
	MultiLine = false;
	nSelEnd = 0;
}




void GUIMemo::PasteFromClipboard ()
{
	DeleteSelected ();
	
	string saved_str = Lines[cur_line];
	Lines.Delete (cur_line);
	
	crt_strncpy (cTempBuffer16K, 16384, saved_str, 16383);
	char oldsim = cTempBuffer16K[CursorPosition];
	cTempBuffer16K[CursorPosition] = 0x0;
	
	string s_begin = cTempBuffer16K;
	
	cTempBuffer16K[CursorPosition] = oldsim;
	
	string s_final = (cTempBuffer16K+CursorPosition);
	
	
	
	GUIHelper::GetClipboardText (cTempBuffer64K, (HWND)api->Storage().GetLong("system.hwnd"));
	
	
	int r_num = strlen (cTempBuffer64K);
	
	int last_begin = 0;
	int first = true;
	for (int n = 0; n < (r_num-1); n++)
	{
		if ((cTempBuffer64K[n+0] == 0x0D) &&
		(cTempBuffer64K[n+1] == 0x0A))
		{
			// copy...
			cTempBuffer64K[n+0] = 0x00;
			cTempBuffer64K[n+1] = 0x00;
			
			string temp1;
			if (first)
			{
				temp1 = s_begin+ string(cTempBuffer64K+last_begin);
			} else
			{
				temp1 = (cTempBuffer64K+last_begin);
			}
			Lines.Insert (cur_line, temp1);
			cur_line++;
			
			n+=2;
			last_begin = n;
			first = false;
		}
	}
	
	// Остался невставленный текст...
	if ((last_begin < (r_num-1)) || (s_final.Len() > 0 ))
	{
		// Вставим его
		string temp2 = string(cTempBuffer64K+last_begin) + s_final;
		Lines.Insert (cur_line, temp2);
		
		CursorPosition = temp2.Len () - s_final.Len ();
	} 
	
	return;
	
	
	
	
	
}





void GUIMemo::DeleteSelected ()
{
	if (!MultiLine)
	{
		
		string& Text = Lines[cur_line];
		
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
			ResetMultiline ();
			CursorPosition = qSelForm;
		}
		
		return;
	}
	
	// Multiline....
	int qSelFormY = SelectFromY;
	int qSelToY = SelectFromY+SelectSizeY;
	
	if (qSelToY < qSelFormY) 
	{
		int tmp = qSelFormY;
		qSelFormY = qSelToY;
		qSelToY = tmp;
	}
	
	
	int bSelStart = nSelStart;
	int bSelEnd = nSelEnd;
	
	if (SelectSizeY < 0)
	{
		bSelStart = nSelEnd;
		bSelEnd = nSelStart;
	}
	
	Lines[qSelFormY].Delete (bSelStart, Lines[qSelFormY].Len()-bSelStart);
	Lines[qSelToY].Delete (0, bSelEnd);
	
	CursorPosition = Lines[qSelFormY].Len ();
	cur_line = qSelFormY;
	Lines[qSelFormY] += Lines[qSelToY];
	
	int full_lines = (qSelToY-qSelFormY-1);
	if (full_lines > 0)
	{
		for (int y=(qSelFormY+1); y < qSelToY; y++)
		{
			Lines.Delete (y); 
		}
		
	}
	
	Lines.Delete (qSelFormY+1); 
	nSelLength = 0;
	ResetMultiline ();
	
	
}



//------------ Копирование в буфер обмена

void GUIMemo::CopyToClipboard ()
{
	if (!MultiLine)
	{
		string& Text = Lines[cur_line];
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
		
		crt_strncpy (cTempBuffer16K, 16384, Text, 16383);
		cTempBuffer16K[qSelTo] = 0;
		
		GUIHelper::SetClipboardText (cTempBuffer16K+qSelForm, (HWND)api->Storage().GetLong("system.hwnd"));
		return;
	}
	
	// Multiline....
	// Multiline....
	int qSelFormY = SelectFromY;
	int qSelToY = SelectFromY+SelectSizeY;
	
	if (qSelToY < qSelFormY) 
	{
		int tmp = qSelFormY;
		qSelFormY = qSelToY;
		qSelToY = tmp;
	}
	
	
	int bSelStart = nSelStart;
	int bSelEnd = nSelEnd;
	
	if (SelectSizeY < 0)
	{
		bSelStart = nSelEnd;
		bSelEnd = nSelStart;
	}
	
	crt_snprintf (cTempBuffer64K, 65534, "%s\r\n", (Lines[qSelFormY].GetBuffer() + bSelStart));
	
	int full_lines = (qSelToY-qSelFormY-1);
	if (full_lines > 0)
	{
		for (int y=(qSelFormY+1); y < qSelToY; y++)
		{
			crt_strcat (cTempBuffer64K, 65535, Lines[y].GetBuffer()); 
			crt_strcat (cTempBuffer64K, 65535, "\r\n");
		}
		
	}
	
	
	crt_strncpy (cTempBuffer16K, 16384, Lines[qSelToY], 16383);
	cTempBuffer16K[bSelEnd] = 0;
	
	crt_strcat (cTempBuffer64K, 65534, cTempBuffer16K);
	
	GUIHelper::SetClipboardText (cTempBuffer64K, (HWND)api->Storage().GetLong("system.hwnd"));
	
	
	
	
}


//------------ Удаление в буфер обмена
void GUIMemo::DeleteToClipboard ()
{
	CopyToClipboard ();
	DeleteSelected ();
}


void GUIMemo::UpdateScrollBars ()
{
	
	//! Находим максимальное кол-во символов по ширине
	int MaxWidth = 0;
	for (int n = 0; n < Lines.Size(); n++)
	{
		int t_len = Lines[n].Len ();
		if (t_len > MaxWidth) MaxWidth = t_len;
	}
	
	int MaxHeight = Lines.Size ();
	
	GUIRectangle rect = GetClientRect ();
	this->ClientToScreen (rect);
	rect.Left += 2;
	rect.Top += 2;
	rect.Width -=4;
	rect.Height -=4;



	int symbY = rect.Height / pFont->GetHeight ();
	int symbX = rect.Width / pFont->GetWidth ("W");
	sbHoriz->PageSize = symbX;
	sbVert->PageSize = symbY;
	

	sbHoriz->Min = 0;
	sbHoriz->Max = MaxWidth;
	sbHoriz->Position = CursorPosition;
	
	sbVert->Min = 0;
	sbVert->Max = (Lines.Size () - 1);
	if (cur_line > (Lines.Size () - 1)) cur_line = (Lines.Size () - 1);
	sbVert->Position = cur_line;
	
}

//sbHoriz->OnChange = (CONTROL_EVENT)sbHorizChange;
//sbVert->OnChange = (CONTROL_EVENT)sbVertChange;


void _cdecl GUIMemo::sbHorizChange (GUIControl* sender)
{
	//CursorPosition = sbHoriz->Position;
	//sCursor = true;
	//UpdateScrollBars ();
	//CalcScrollOffset ();
	
	ScrollOffset = sbHoriz->Position * pFont->GetWidth ("W");
}

void _cdecl GUIMemo::sbVertChange (GUIControl* sender)
{
	//cur_line = sbVert->Position;
	//sCursor = true;
	//UpdateScrollBars ();


	int ofs_pos = (sbVert->Position - sbVert->PageSize);
	if (ofs_pos < 0) ofs_pos = 0;
	OffsetY = ofs_pos * pFont->GetHeight ();
	//CalcScrollOffset ();
}

void _cdecl GUIMemo::OnStrAdd (GUIControl* sender)
{
	if (Lines.Size() > 0)
	{
		UpdateScrollBars ();
		//CalcScrollOffset ();
	}
}

#endif