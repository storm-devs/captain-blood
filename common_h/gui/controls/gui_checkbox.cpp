#ifndef _XBOX
#include "..\..\core.h"
#include "..\..\render.h"
#include "gui_checkbox.h"
#include "gui_helper.h"




GUICheckBox::GUICheckBox (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ImageNormal = NEW GUIImage ();
	ImageChecked = NEW GUIImage ();
	
	pFont = NEW GUIFont ("SansSerif");
	
	ClientRect = DrawRect;
	ClientRect.Left += 2;
	ClientRect.Top += 2;
	ClientRect.Width -= 4;
	ClientRect.Height -= 4;
	
  FontColor = 0xFF000000;
	
	Checked = false;
	
	MD_Time = 0.0f;
	
	t_OnChange = NEW GUIEventHandler;
	
}

GUICheckBox::~GUICheckBox ()
{
	
	delete ImageNormal;
	delete ImageChecked;
	
	delete pFont;

	delete t_OnChange;
	
//	GUIControl::~GUIControl ();
}


void GUICheckBox::Draw ()
{
	if (Visible == false) return;

	MD_Time += api->GetDeltaTime();
	
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
	
	
	
	
	
	int width = ImageNormal->GetWidth();
	
	if (!Checked)
	{
		GUIHelper::DrawSprite (nLeft+3, np.Y+3, width, ImageNormal->GetHeight(), ImageNormal);
	} else
	{
		GUIHelper::DrawSprite (nLeft+3, np.Y+3, width, ImageChecked->GetHeight(), ImageChecked);
	}
	
	
	if (Caption) pFont->Print (nLeft + width + 6, np.Y, FontColor, Caption);
	
	
//	cliper.Pop ();
	GUIControl::Draw ();
	
}


bool GUICheckBox::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
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
	}
	
	if (message == GUIMSG_MOUSEENTER) 
	{
		Handled = true;
	}
	
	if (message == GUIMSG_MOUSELEAVE) 
	{
		Handled = true;
	}
	
	return Handled;
}


void GUICheckBox::OnMDown (int MouseKey, const GUIPoint& pt)
{
	if (MD_Time < 0.15f ) return;
	
	GUIRectangle sRect = GetScreenRect ();
//	sRect.Width = 20;
	if (GUIHelper::PointInRect(pt, sRect))
	{
		Checked = !Checked;
		MD_Time = 0.0f;
		t_OnChange->Execute (this);
	}
	
	
}

#endif