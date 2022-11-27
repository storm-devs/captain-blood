#ifndef _XBOX

#include "..\..\core.h"
#include "..\..\render.h"
#include "gui_window.h"
#include "gui_helper.h"
#include "..\gui_imanager.h"







GUIWindow::GUIWindow (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	dwWindowBackColor = 0xFFD4D0C8;
	bMovable = true;
	DrawBackground = true;
	bPopupStyle = false;
	bSystemButton = true;
	IsActive = false; 
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	
	title_height = 17;
	
	draged = false;
	
	icon = NULL;
	
	
	IRender* rs = (IRender*)api->GetService("DX9Render");
	
	icon = NEW GUIImage ("icon");
	
	
	
	pFont = NEW GUIFont ("SansSerif");
	//, (float)(title_height-2)
	
	
	
	close_button = NEW GUIButton (this, (Width-title_height-4), 
																	-title_height-1, 
																	title_height-2, 
																	title_height-2);
	
	close_button->Glyph->Load ("close");
	//close_button->glyph->Load ("icon");
	
	
	
	
	close_button->OnMousePressed = (CONTROL_EVENT)&GUIMessageBox::Close;
	
	//close_button->t_OnMousePressed->SetHandler (this, (CONTROL_EVENT)OnCloseClik);
	
	
	
	
	
	
	ClientRect = DrawRect;
	ClientRect.Top += (title_height+6);
	ClientRect.Height -= (title_height+9);


	t_OnClose = NEW GUIEventHandler;

	t_OnBeforeClose = NEW  GUIEventHandler;
	
	
	
}

GUIWindow::~GUIWindow ()
{
	
	//delete close_button;
	if (pFont) delete pFont;
  delete icon;
  delete t_OnClose;
	delete t_OnBeforeClose;
	
	//GUIControl::~GUIControl ();
}


void GUIWindow::Draw ()
{
 if (bPopupStyle)
 {
	ClientRect = DrawRect;
 } else
	 {
		ClientRect = DrawRect;
		ClientRect.Top += (title_height+6);
		ClientRect.Height -= (title_height+9);
	 }

	if (bSystemButton)
	{
		close_button->Visible = true;
	} else
		{
			close_button->Visible = false;
		}

	if (Visible == false) return;



	if (IsActive)
	{
		//title_color1 = 0xFF6A240A;
		//title_color2 = 0xFFF0CAA6;
		
		title_color1 = 0xFF0A246A;
		title_color2 = 0xFFA6CAF0;
		
	} else
	{
		title_color1 = 0xFF808080;
		title_color2 = 0xFFC0C0C0;
		
	}
	
//	cliper.Push ();
//	cliper.SetRectangle (GetScreenRect ());
	
	
	GUIRectangle r;
	if (parent) r= parent->GetClientRect ();
	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	int nLeft = DrawRect.Left;
	nLeft+= r.Left;
	int nTop = DrawRect.Top;
	nTop+= r.Top;
	
	
	if (DrawBackground)
	{
		GUIHelper::DrawBigUpBox (nLeft, nTop, Width, Height, dwWindowBackColor);
	} else
		{
		  GUIHelper::Draw2DRect(nLeft, nTop, Width, title_height+6, dwWindowBackColor);
		  GUIHelper::DrawUpBorder(nLeft, nTop, Width, Height);
		}
	
	
	GUIRectangle toprectangle;
	
	toprectangle.Left = nLeft+4;
	toprectangle.Top = nTop+4;
	toprectangle.Width = Width-8;
	toprectangle.Height = title_height;
//	cliper.SetRectangle (toprectangle);
	
	
	if (!bPopupStyle) 
	{
		GUIHelper::Draw2DRectHorizGradient (nLeft+4, nTop+4, Width-8, title_height, title_color1, title_color2);
		if (icon) GUIHelper::DrawSprite (nLeft+6, nTop+6, (title_height-4), (title_height-4), icon);

		if (IsActive)
		{
			title_caption_color = 0xFFFFFFFF;
		} else
		{
			title_caption_color = 0xFFD4D0C8;
		}
		
		
		
		if (Caption) pFont->PrintWidth ((nLeft+6 + (title_height-4) + 2), nTop+5, (Width-55), title_caption_color, Caption);

	
	// GUIRectangle b_rect = close_button->GetDrawRect ();
	// b_rect.Left = (DrawRect.Left+Width-8-title_height-6);
	//close_button->SetDrawRect (b_rect);
	
		close_button->Visible = true;
		if (bSystemButton) close_button->Draw ();
	}
	
	
	GUIRectangle window_rectangle = GetScreenRect ();
//	cliper.SetFullScreenRect ();
//	cliper.SetRectangle (window_rectangle);
	
//	cliper.Pop ();
	close_button->Visible = false;
	GUIControl::Draw ();
	
	
}


bool GUIWindow::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{

 if (!Enabled) return false;
 if (bPopupStyle)
 {
	ClientRect = DrawRect;
 } else
	 {
		ClientRect = DrawRect;
		ClientRect.Top += (title_height+6);
		ClientRect.Height -= (title_height+9);
	 }


	if (bSystemButton)
	{
		close_button->Visible = true;
	} else
		{
			close_button->Visible = false;
		}

	if (bPopupStyle) close_button->Visible = false;

 	if (Visible == false) return false;

	bool Handled = false;
	
	Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	
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
	
	if (message == GUIMSG_MOUSEMOVE) 
	{
		Handled = true;
		OnMMove (cursor_pos);
	}



	return Handled;
}




void GUIWindow::OnMDown (int MouseKey, const GUIPoint& pt)
{
	if (!bMovable) return;
	GUIRectangle sRect = GetScreenRect ();
	sRect.Left += 8;
	sRect.Top += 4;
	sRect.Width -= 16;
	sRect.Height = title_height-2;
	
	if (GUIHelper::PointInRect(pt, sRect) && (!bPopupStyle))
	{
		draged = true;
		m_pressed = pt;
	}
	
}

void GUIWindow::OnMUp (int MouseKey, const GUIPoint& pt)
{
	draged = false;
	
}


void GUIWindow::OnMEnter ()
{
}

void GUIWindow::OnMLeave ()
{
}


void GUIWindow::OnMMove (const GUIPoint& pt)
{
	
}


void GUIWindow::MouseMove (int button, const GUIPoint& pt)
{
	if (draged)
	{
		int dX  = m_pressed.X - pt.X;
		int dY  = m_pressed.Y - pt.Y;
		
		DrawRect.Left = DrawRect.Left - dX;
		DrawRect.Top = DrawRect.Top - dY;
		ClientRect = DrawRect;
		ClientRect.Top += (title_height+6);
		ClientRect.Height -= (title_height+9);
		
		m_pressed = pt;
	}
	
	GUIControl::MouseMove (button, pt);
}

bool GUIWindow::BeforeClose ()
{
	return true;
}

void _cdecl GUIWindow::Close (GUIControl* Sender)
{
	bCloseCanceled = false;
	t_OnBeforeClose->Execute(this);

	if (bCloseCanceled) return;

	if (!BeforeClose ()) return;
	Application->DestroyGUIWindow (this);
}

void GUIWindow::Show ()
{
	Application->Show (this);
}

void GUIWindow::ShowModal ()
{
	Application->ShowModal (this);
}

void GUIWindow::SetPosition (int pX, int pY)
{
	DrawRect.Left = pX;
	DrawRect.Top = pY;
	ClientRect = DrawRect;
	ClientRect.Top += (title_height+6);
  ClientRect.Height -= (title_height+9);
  UpdatePopupStyle ();
}

void GUIWindow::SetWidth (int width)
{
	DrawRect.Width = width;
	ClientRect = DrawRect;
	ClientRect.Top += (title_height+6);
	ClientRect.Height -= (title_height+9);
	
	GUIRectangle old_rec = close_button->GetDrawRect ();
	old_rec.Left = (width-title_height-4);
	close_button->SetDrawRect (old_rec);
	
	GUIRectangle CRect = old_rec;
	CRect.Left += 2;
	CRect.Top += 2;
	CRect.Width -= 4;
	CRect.Height -= 4;
	
	close_button->SetClientRect (CRect);
  UpdatePopupStyle ();	
	
}

void GUIWindow::SetHeight (int height)
{
	DrawRect.Height = height;
	ClientRect = DrawRect;
	ClientRect.Top += (title_height+6);
  ClientRect.Height -= (title_height+9);
  UpdatePopupStyle ();  

}


void GUIWindow::SetScreenCenter ()
{
	IRender* rs = (IRender*)api->GetService("DX9Render");
	
	int ScreenX = rs->GetScreenInfo3D().dwWidth;
	int ScreenY = rs->GetScreenInfo3D().dwHeight;
	int newpX = (ScreenX-DrawRect.Width) / 2;
	int newpY = (ScreenY-DrawRect.Height) / 2;

	SetPosition (newpX, newpY);
}


void GUIWindow::UpdatePopupStyle ()
{
 if (bPopupStyle)
 {
	ClientRect = DrawRect;
	close_button->Visible = false;
 } else
	 {
		ClientRect = DrawRect;
		ClientRect.Top += (title_height+6);
		ClientRect.Height -= (title_height+9);
	 }


}

void GUIWindow::SetAllocID (int id)
{
	//Caption.Format("ID - %d", id);
	GUIControl::SetAllocID (id);
}

#endif