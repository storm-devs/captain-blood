#ifndef _XBOX

#include "gui_urllabel.h"
#include "gui_helper.h"


GUIURLLabel::GUIURLLabel (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	//DWORD color;
	//DWORD font_color;
	pFont = NEW GUIFont ("SansSerif");
	
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;
	
	
  FontColor = 0xFF000000;
	SelectedFontColor = 0xFF0000FF;
	
	t_OnMouseDown = NEW GUIEventHandler;
	t_OnMouseUp = NEW GUIEventHandler;
	
	t_OnMouseEnter = NEW GUIEventHandler;
	t_OnMouseLeave = NEW GUIEventHandler;
	
	
}

GUIURLLabel::~GUIURLLabel ()
{
	delete pFont;
	
	//GUIControl::~GUIControl ();
}

void GUIURLLabel::Draw ()
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
		if (!mouse_inside) 
		{
			pFont->Print (font_point.X, font_point.Y, FontColor, Caption);
		} else
		{
			
			rect.Height = (TextHeight+10);
//			cliper.SetRectangle (rect);
			
			pFont->Print (font_point.X, font_point.Y, SelectedFontColor, Caption);
			GUIHelper::Draw2DLine (font_point.X, font_point.Y+TextHeight, font_point.X+TextWidth, font_point.Y+TextHeight, SelectedFontColor);
		}
		
	}
	
	
	
//	cliper.Pop ();
	GUIControl::Draw ();
}


bool GUIURLLabel::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
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

void GUIURLLabel::OnMDown (int MouseKey, const GUIPoint& pt)
{
	t_OnMouseDown->Execute (this);
	
	if (Caption) ShellExecute ((HWND)api->Storage().GetLong("system.hwnd"), "open", Caption, NULL, NULL, SW_SHOW );
}

void GUIURLLabel::OnMUp (int MouseKey, const GUIPoint& pt)
{
	t_OnMouseUp->Execute (this);
}


void GUIURLLabel::OnMEnter ()
{
	t_OnMouseEnter->Execute (this);
}


void GUIURLLabel::OnMLeave ()
{
	t_OnMouseLeave->Execute (this);
}



#endif