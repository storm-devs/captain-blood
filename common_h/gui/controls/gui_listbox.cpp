#ifndef _XBOX
#include "gui_listbox.h"


//GUIScrollBar* sbVert;
//int SelectedLine;
//GUIStringList Items;
//DWORD FontColor;

GUIListBox::GUIListBox (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent),
                                                                                         Images (_FL_, 32)
{
	SearchString = "";
	bCaptureKeyboard = true;
	AutoSelectMode = false;
	OffsetY = 0;
	
	SelectedLine = -1;
	
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;
	ClientRect.Width -= 19;
	
	
	Items.Clear ();
	FontColor = 0xFF000000;
	sbVert = NEW GUIScrollBar (this, GUISBKIND_Vertical, Width-19, 0, 20, (Height));
	sbVert->OnChange = (CONTROL_EVENT)&GUIListBox::sbVertChange;
	sbVert->Min = -1;
	sbVert->Max = 0;
	sbVert->PageSize = 6;

	SelectColor = 0xFF404040;
	
	pFont = NEW GUIFont ("Courier");
	
	Items.OnAdd = (CONTROL_EVENT)&GUIListBox::OnStrAdd;
	Items.OnClear = (CONTROL_EVENT)&GUIListBox::ListBeforeClear;
	
	CursorInside = false;
	
	key_pressed_time = 0;
	
	t_OnMouseDown = NEW GUIEventHandler;
	t_OnMouseUp = NEW GUIEventHandler;
	
	t_OnMouseEnter = NEW GUIEventHandler;
	t_OnMouseLeave = NEW GUIEventHandler;

	t_OnChange = NEW GUIEventHandler;

	
	t_OnMouseDBLClick = NEW GUIEventHandler;


	m_nMouseWeelUpCounter = m_nMouseWeelDownCounter = 0;
	
}

GUIListBox::~GUIListBox ()
{
	delete pFont;
  delete sbVert;
  
	delete t_OnMouseDBLClick;
	delete t_OnMouseDown;
	delete t_OnMouseUp;
	
	delete t_OnMouseEnter;
	delete t_OnMouseLeave;

	delete t_OnChange;
  
}


bool GUIListBox::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
 	if (Visible == false) return false;

	bool Handled = false;
	
	Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	if (Handled) return Handled;
	
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);

	if ((message == GUIMSG_MOUSEMOVE) && (AutoSelectMode == true) &&
		!(sbVert && sbVert->IsMouseMoveNowProcessing()) )
	{
		Handled = true;
		OnMDown (message, cursor_pos);
	}
	
	if (message == GUIMSG_WHEEL_UP)
	{
		m_nMouseWeelUpCounter++;
		if( m_nMouseWeelUpCounter % 20 == 1 )
		{
			SelectedLine = SelectedLine - 1;
			CalcYOffset ();
			UpdateScrollBars ();
		}
	}

	if (message == GUIMSG_WHEEL_DOWN)
	{
		m_nMouseWeelDownCounter++;
		if( m_nMouseWeelDownCounter % 20 == 1 )
		{
			SelectedLine = SelectedLine + 1;
			CalcYOffset ();
			UpdateScrollBars ();
		}
	}

	if (message == GUIMSG_LMB_DBLCLICK)
	{
		Handled = true;
		t_OnMouseDBLClick->Execute(this);
	}
	
	if (message == GUIMSG_LMB_DOWN) 
	{
		Handled = true;
		OnMDown (message, cursor_pos);
		t_OnMouseDown->Execute (this);
	}
	
	if (message == GUIMSG_LMB_UP) 
	{
		Handled = true;
		OnMUp (message, cursor_pos);
	}
	
	
	
	if ((message == GUIMSG_KEYPRESSED) && (CursorInside))
	{
		Handled = true;
		OnKPressed ((int)lparam, hparam);
		CalcYOffset ();
		UpdateScrollBars ();
		//t_OnChange->Execute (this);
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
	}
	
	
	return Handled;
	
}

void GUIListBox::CalcYOffset ()
{
	if (SelectedLine < 0) SelectedLine = 0;
	int fHeight = pFont->GetHeight ();
	int cursorYpos = ((SelectedLine * fHeight) - OffsetY) + fHeight;

	
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

	if (OffsetY < 0) OffsetY = 0;
}

void GUIListBox::UpdateScrollBars ()
{
	int MaxHeight = Items.Size ();
	int CharHeight = pFont->GetHeight ();
	
	GUIRectangle gr = GetClientRect();
	float ScreenHeight = (float)gr.Height / (float)CharHeight;
	
	float kY = ScreenHeight / (float)MaxHeight;
	if (kY > 1.0f) kY = 1.0f;
	
	// kY - сколько процентов по высоте
//	sbVert->sbSize = kY;
	
	sbVert->Min = 0;
	sbVert->Max = (Items.Size () - 1);
	if (SelectedLine > sbVert->Max) SelectedLine = sbVert->Max;
	sbVert->Position = SelectedLine;

	GUIRectangle rect = GetClientRect ();
	this->ClientToScreen (rect);
	rect.Left += 2;
	rect.Top += 2;
	rect.Width -=4;
	rect.Height -=4;

	int symbY = rect.Height / pFont->GetHeight ();
	sbVert->PageSize = symbY;

}


void GUIListBox::Draw ()
{
	if (Visible == false) return;

	if (key_pressed_time > 2.5)
	{
		SearchString = "";
	}

	m_nMouseWeelUpCounter = m_nMouseWeelDownCounter = 0;


	float dt = api->GetDeltaTime();
	key_pressed_time += dt;
	
	string& Text = string ("");
	if (Items.Size () > 0 && SelectedLine >= 0)
	{
	Text = Items[SelectedLine];
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
	
	DrawListBox (nLeft, nTop, DrawRect.Width, DrawRect.Height);
	
	GUIRectangle rect2 = rect;
	rect2.Left += 2;
	rect2.Top += 2;
	rect2.Width -= 4;
	rect2.Height -= 4;
	cliper.Push ();
	cliper.SetRectangle (rect2);
	
	
	// Уводим печать на верх...
	font_point.Y = nTop-OffsetY;

	for (int u=0;u< Items.Size();u++)
	{
		//pFont->Print (nLeft+3-ScrollOffset, font_point.Y+(TextHeight*u), FontColor, "%s", Text);
		string& cl = Items[u];
		int line_height = font_point.Y+(TextHeight*u);
		
		if (((line_height) >= nTop) && (line_height+19 < (nTop+Height+TextHeight)))
		{
			//
			//
			int subAddX = 0;
			if ((u >= 0) && (u < (int)Images.Size()))
			{
				GUIHelper::DrawSprite (nLeft+3, font_point.Y+(TextHeight*u), Images[u].GetWidth(), Images[u].GetHeight(), &Images[u]);
				subAddX = Images[u].GetWidth ()+2;
			}

			if (!cl.IsEmpty())
			{
				if (cl.GetBuffer())
				{
					pFont->Print (nLeft+3+subAddX, font_point.Y+(TextHeight*u), FontColor, "%s", cl.GetBuffer());
				}
			}
		}
	}
	


	DWORD dwPrintColor = 0x5000FF00;
	if (SearchString.IsEmpty()) dwPrintColor = SelectColor;
	GUIHelper::Draw2DRect2 ( nLeft+3, font_point.Y+(SelectedLine*pFont->GetHeight()), 
														Width-24, pFont->GetHeight(), dwPrintColor);
	
	cliper.Pop ();
	

	if (!SearchString.IsEmpty())
	{
		//GUIHelper::Draw2DRect(pX-1, pY-1, Width+2, Height+2, 0xFF000000);
		//GUIHelper::Draw2DRect(pX, pY, Width, Height, 0xFFFFFFE1);

		GUIHelper::Draw2DRect ( nLeft+3+4-1, 3-1+font_point.Y+((SelectedLine+1)*pFont->GetHeight()), 
			Width-24+2-7, pFont->GetHeight()+2, 0xFF000000);


		GUIHelper::Draw2DRect ( nLeft+3+4, 3+font_point.Y+((SelectedLine+1)*pFont->GetHeight()), 
			Width-24-7, pFont->GetHeight(), 0xFFFFFFE1);

		pFont->Print(nLeft+3+4, 3+font_point.Y+((SelectedLine+1)*pFont->GetHeight()), 0xFF0000FF, "%s", SearchString.c_str());
	}
	
	
//	cliper.Pop ();
	GUIControl::Draw ();
	
}

void GUIListBox::DrawListBox (long nLeft, long nTop, long Width, long Height)
{
	GUIHelper::DrawEditBox2 (nLeft, nTop, Width, Height);
}

void _cdecl GUIListBox::sbVertChange (GUIControl* sender)
{
	//SelectedLine = sbVert->Position;

	t_OnChange->Execute (this);

	//if (SelectedLine <= 0) SelectedLine = 0;
	//if (SelectedLine >= (int)(Items.GetCount()-1)) SelectedLine = (int)(Items.GetCount()-1);
	//if (Items.GetCount() == 0)
	//{
//		SelectedLine = 0;
//	}

	//UpdateScrollBars ();
	//CalcYOffset ();

	int ofs_pos = (sbVert->Position - sbVert->PageSize)+1;
	if (ofs_pos < 0) ofs_pos = 0;


	OffsetY = ofs_pos * pFont->GetHeight ();
}

void _cdecl GUIListBox::OnStrAdd (GUIControl* sender)
{
	if (Items.Size() > 0)
	{
		UpdateScrollBars ();
		CalcYOffset ();
	}
	
}


void GUIListBox::OnMDown (int MouseKey, const GUIPoint& pt)
{
	GUIPoint g_point = pt;
	this->ScreenToClient (g_point);
	
	
	int fntHeight = pFont->GetHeight ();
	int CurY = -OffsetY;
	bool usd = false;
	for (int n = 0; n < Items.Size(); n++)
	{
		if ((g_point.Y >= CurY) && (g_point.Y < (CurY+fntHeight)))
		{
			if ((g_point.X >= 0) && (g_point.X < DrawRect.Width-19))
			{
			SearchString = "";
			SelectedLine = n; 
			usd = true;
			break;
			}
		}
		
		CurY+=fntHeight;
	}
	
	
	if ((!usd) && (AutoSelectMode == false)) SelectedLine = (Items.Size()-1);
	t_OnChange->Execute (this);
	
	
	//t_OnMouseDown->Execute (this);
	
	// Сделали элемент активным...
	CursorInside = true;
	
	if (bCaptureKeyboard)
	{
		GUIControl* g_ctl = GetMainControl ();
		g_ctl->Application->SetKeyboardFocus (this);
	}
	
	UpdateScrollBars ();
}

void GUIListBox::OnMUp (int MouseKey, const GUIPoint& pt)
{
	//  t_OnMouseUp->Execute (this);
}


void GUIListBox::OnMEnter ()
{
	//t_OnMouseEnter->Execute (this);
}


void GUIListBox::OnMLeave ()
{
	//t_OnMouseLeave->Execute (this);
}


void GUIListBox::OnKPressed (int key, dword hparam)
{
	if (key >= 32 && hparam == 0)
	{
		if (key_pressed_time > 0.9)
		{
			SearchString = "";
		}

		key_pressed_time = 0;


		char szText[2];
		szText[0] = (char)key;
		szText[1] = 0;
		SearchString += szText;

		//Изменили быстро поиск :))))

		for (int j = 0; j < Items.Size(); j++)
		{
			string a1 = Items[j];
			string a2 = SearchString;
			a1.Lower();
			a2.Lower();
			
			const char* szRes = crt_strstr (a1.c_str(), a2.c_str());
			if (szRes == a1.c_str())
			{
				SelectedLine = j;
				return;
			}
		}

		return;
	}



	if (key_pressed_time <= 0.15) /* повтор клавиши */
	{
		return;
	}
	
	if ((key != 38) && (key != 40)) return;
	
	key_pressed_time = 0;
	
	if (key == 38) /* UP */
	{
		if (SelectedLine <= 0) return;
		SelectedLine--;
		t_OnChange->Execute (this);
	}
	
	
	if (key == 40)/* DOWN */
	{
		if (SelectedLine >= (int)(Items.Size()-1)) return;
		SelectedLine++;
		t_OnChange->Execute (this);
		return;
	}
	
	
	
}

void _cdecl GUIListBox::ListBeforeClear (GUIControl* sender)
{
	sbVert->Position = 0;
	sbVert->Max = 0;
	sbVert->Min = -1;
	SelectedLine = -1;
	sbVertChange (NULL);
}

void GUIListBox::SetDrawRect (const GUIRectangle& rect)
{
	DrawRect = rect;

	GUIRectangle r;
	r.x = rect.w-19;
	r.y = 0;
	r.w = 20;
	r.h = rect.h;
	sbVert->SetDrawRect (r);

}


void GUIListBox::MakeFlat (bool Flat)
{
	sbVert->MakeFlat (Flat);
//	sbHoriz->MakeFlat (Flat);
}

int GUIListBox::GetElementNumAtPos(const GUIPoint& pt)
{
	GUIPoint np;
	np.X = DrawRect.Left;
	np.Y = DrawRect.Top;
	ClientToScreen (np);

	if ((pt.x < np.X) || (pt.y > (np.X + DrawRect.w))) return -1;

	int printFrom = np.Y-OffsetY;
	int TextHeight = pFont->GetHeight ();

	for (int u = 0; u < Items.Size();u++)
	{
		int curY = printFrom + (TextHeight*u);
		if ((pt.y >= curY) && (pt.y <= (curY+TextHeight)))
		{
			if ((pt.x-4) <= np.X + pFont->GetWidth(Items[u]))		return u;
		}
	}
	
	return -1;
}

#endif