#ifndef _XBOX

#include "gui_label.h"
#include "gui_helper.h"


GUILabel::GUILabel (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	//DWORD color;
	//DWORD font_color;
	pFont = NEW GUIFont ("SansSerif");
	Layout = GUILABELLAYOUT_Center;
	
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
	
	
}

GUILabel::~GUILabel ()
{
	delete pFont;
	delete t_OnMouseDown;
	delete t_OnMouseUp;
	
	delete t_OnMouseEnter;
	delete t_OnMouseLeave;

  
//	GUIControl::~GUIControl ();
}

void GUILabel::Draw ()
{
	if (Visible == false) return;

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
	int TextHeight = 0;
	
	if (Caption)
	{
		TextWidth = pFont->GetWidth (Caption);
		TextHeight = pFont->GetHeight ();
	}
	
	GUIRectangle font_rect;
	font_rect.Left = nLeft;
	font_rect.Top = nTop;
	font_rect.Width = TextWidth;
	font_rect.Height = TextHeight;
	GUIPoint font_point;
	GUIHelper::Center (font_point, font_rect, rect);
	
	
	if (Caption)
	{
		if (Layout == GUILABELLAYOUT_Center)
		{
			pFont->Print (font_point.X, font_point.Y, FontColor, Caption);
		}
		
		if (Layout == GUILABELLAYOUT_Left)
		{
			pFont->Print (nLeft, font_point.Y, FontColor, Caption);
		}
		
		if (Layout == GUILABELLAYOUT_Right)
		{
			int need_posX = (nLeft+Width)-TextWidth;
			pFont->Print (need_posX, font_point.Y, FontColor, Caption);
		}
	}
	
	
	
//	cliper.Pop ();
	GUIControl::Draw ();
}


bool GUILabel::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
 	if (Visible == false) return false;

	bool Handled = false;
	
	Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	if (Handled) return Handled;
	
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);
	
	
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
	
	return Handled;
}

void GUILabel::OnMDown (int MouseKey, const GUIPoint& pt)
{
	t_OnMouseDown->Execute (this);
}

void GUILabel::OnMUp (int MouseKey, const GUIPoint& pt)
{
	t_OnMouseUp->Execute (this);
}


void GUILabel::OnMEnter ()
{
	t_OnMouseEnter->Execute (this);
}


void GUILabel::OnMLeave ()
{
	t_OnMouseLeave->Execute (this);
}


#endif