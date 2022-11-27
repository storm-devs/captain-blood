#ifndef _XBOX
#include "..\..\core.h"
#include "..\..\render.h"
#include "gui_button.h"
#include "gui_helper.h"


GUIEventHandler* hack_temp_ptr;


GUIButton::GUIButton (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	bEnabled = true;
	GroupIndex = 0;
	FlatButtonActiveColor = 0;
	FlatButtonPassiveColor = 0;
	FlatButtonPressedColor = 0;

	FlatButton = false;
	Align = GUIAL_Center;
	//Align = GUIAL_Right;
	//Layout = GUIBUTTONLAYOUT_GlyphRight;

	Layout = GUIBUTTONLAYOUT_GlyphLeft;
	MousePressed = false;
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	color = 0xFFD4D0C8;

	line_color1 = 0xFFFFFFFF;
	line_color2 = 0xFF808080;
	line_color3 = 0xFF404040;

	Glyph = NULL;
	DisabledGlyph = NULL;

	IRender* rs = (IRender*)api->GetService("DX9Render");

	Glyph = NEW GUIImage ();
	DisabledGlyph = NEW GUIImage ();



	pFont = NEW GUIFont ("SansSerif");

	ClientRect = DrawRect;

	ClientRect.Left += 2;
	ClientRect.Top += 2;
	ClientRect.Width -= 4;
	ClientRect.Height -= 4;

  FontColor = 0xFF000000;
	DisabledFontColor = FontColor;

	Down = false;
	TempDown = false;

	// user_event_handler = NULL;
	// user_handler_class = NULL;

	t_OnMousePressed = NEW GUIEventHandler;

	t_OnMouseEnter = NEW GUIEventHandler;
	t_OnMouseLeave = NEW GUIEventHandler;
	t_OnMouseDown = NEW GUIEventHandler;
	t_OnMouseClick = NEW GUIEventHandler;

	MD_Time = 0;

}

GUIButton::~GUIButton ()
{
	delete DisabledGlyph;
	delete Glyph;
	delete pFont;

	delete t_OnMouseClick;
	delete t_OnMousePressed;

	delete t_OnMouseEnter;
	delete t_OnMouseLeave;
	delete t_OnMouseDown;


//	GUIControl::~GUIControl ();
}


void GUIButton::Draw ()
{
	if (Visible == false) return;

	MD_Time += api->GetDeltaTime();
	if (MD_Time > 0.07)
	{
		if (TempDown) t_OnMouseDown->Execute (this);
		MD_Time = 0.0f;
	}

//	cliper.Push ();

	//Layout
	//cliper.SetFullScreenRect ();

	GUIRectangle rect = GetClientRect ();
	this->ClientToScreen (rect);
//	cliper.SetRectangle (rect);
	//pFont->Print (0, 0, 0xFFFFFFFF, "AAAAAAAAAARRGH!!!!");


	int Width = DrawRect.Width;
	int Height = DrawRect.Height;

	GUIPoint np;
	np.X = DrawRect.Left;
	np.Y = DrawRect.Top;
	ClientToScreen (np);

	int nLeft = np.X;
	int nTop = np.Y;

	int TextWidth = 0;
	int TextHeight = 0;

	if (!Caption.IsEmpty())
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


	//font_point.Y = 0;
	
	


//	cliper.SetRectangle (GetScreenRect ());
	
	if ((Down) || (TempDown))
	{
		DrawButtonDown (nLeft, nTop, Width, Height);
		// Отжимаем кнопку после того как она нарисовалась...
		if ((Down) && (GroupIndex == 0))
		{
			Down = false;
		}
	} else
	{
		DrawButtonUp (nLeft, nTop, Width, Height);
	}

	int add = 0;
	if ((Down) || (TempDown)) add = 1;

	int font_add_y = 0;
	int font_add_x = 0;

	GUIRectangle big_rect;
	big_rect.Height = TextHeight;
	big_rect.Width = TextWidth;

	GUIImage* DrawedGlyph = Glyph;
	if (!bEnabled) DrawedGlyph = DisabledGlyph;

	if (DrawedGlyph) 
	{
		// Получаем размеры картинки в glyph_rect
		GUIRectangle glyph_rect;
		glyph_rect.Left = nLeft+4+add;
		glyph_rect.Top = nTop+4+add;
		glyph_rect.Width = DrawedGlyph->GetWidth();
		glyph_rect.Height = DrawedGlyph->GetHeight();

		// считаем сколько надо добавить к фонту..
		font_add_y = DrawedGlyph->GetHeight();
		font_add_y += 2;

		// считаем сколько надо добавить к фонту..
		font_add_x = DrawedGlyph->GetWidth ();
		font_add_x += 2;

		// Получаем большой AABB (glyph + font)
		big_rect.Height += font_add_y;
		big_rect.Width += font_add_x;

		// Центруем большой AABB
		GUIPoint fpz;
		GUIHelper::Center (fpz, big_rect, rect);

		if ((Align == GUIAL_Left) && 
			((Layout == GUIBUTTONLAYOUT_GlyphRight) ||
			(Layout == GUIBUTTONLAYOUT_GlyphLeft)))
		{
//			if (Layout == GUIBUTTONLAYOUT_GlyphLeft)
			//{
			int subX = fpz.x - (nLeft+2);
			fpz.x -= subX;
			//}
		}

		if ((Align == GUIAL_Right) &&
			 ((Layout == GUIBUTTONLAYOUT_GlyphRight) ||
			 (Layout == GUIBUTTONLAYOUT_GlyphLeft)))
		{
			if (Layout == GUIBUTTONLAYOUT_GlyphLeft)
			{
			fpz.x = (nLeft+2) + ClientRect.Width - DrawedGlyph->GetWidth ();
			fpz.x-= TextWidth;
			}
			if (Layout == GUIBUTTONLAYOUT_GlyphRight) 
			{
				//fpz.x = (nLeft+2)- TextWidth;
				//- Glyph->GetWidth ()
				int addonX = ClientRect.Width - big_rect.Width;
				fpz.x = (nLeft+2) + addonX - 2;
			}

		}


		// Центруем глиф внутри AABB
		GUIPoint z_pt;
		GUIHelper::Center (z_pt, glyph_rect, rect);



		
		if (!Caption) 
		{
			fpz = z_pt;
			TextHeight = -3;
			TextWidth = -4;
		}

		if (Layout == GUIBUTTONLAYOUT_GlyphTop)
		{
			GUIHelper::DrawSprite (z_pt.X+add, fpz.Y+add, DrawedGlyph->GetWidth(), DrawedGlyph->GetHeight(), DrawedGlyph);
		}

		if (Layout == GUIBUTTONLAYOUT_GlyphBottom)
		{
			GUIHelper::DrawSprite (z_pt.X+add, fpz.Y+add+TextHeight+3, DrawedGlyph->GetWidth(), DrawedGlyph->GetHeight(), DrawedGlyph);
		}

		if (Layout == GUIBUTTONLAYOUT_GlyphLeft)
		{
				GUIHelper::DrawSprite (fpz.X+add, z_pt.Y+add, DrawedGlyph->GetWidth(), DrawedGlyph->GetHeight(), DrawedGlyph);
		}

		if (Layout == GUIBUTTONLAYOUT_GlyphRight)
		{
			GUIHelper::DrawSprite (fpz.X+add+TextWidth+4, z_pt.Y+add, DrawedGlyph->GetWidth(), DrawedGlyph->GetHeight(), DrawedGlyph);
		}

	}

//	cliper.SetRectangle (rect); 

	GUIPoint fp;
	GUIHelper::Center (fp, big_rect, rect);

	//fp.x = 0;
		if ((Align == GUIAL_Left) && 
			((Layout == GUIBUTTONLAYOUT_GlyphRight) ||
			(Layout == GUIBUTTONLAYOUT_GlyphLeft)))
		{
			int subX = fp.x - (nLeft+2);
			fp.x -= subX;
		}

		if ((Align == GUIAL_Right) &&
			((Layout == GUIBUTTONLAYOUT_GlyphRight) ||
			(Layout == GUIBUTTONLAYOUT_GlyphLeft)))
		{
			int addonX = ClientRect.Width - big_rect.Width;
			fp.x = (nLeft+2) + addonX;
		}



	if (DrawedGlyph->IsEmpty()) 
	{
		fp = font_point;
		font_add_y = 0;
		font_add_x = 0;
	}
	//if (Caption) pFont->Print (nLeft+add, nTop+add, font_color, Caption);

	DWORD DrawFontColor = FontColor;
	if (!bEnabled) DrawFontColor = DisabledFontColor;


	if (Layout == GUIBUTTONLAYOUT_GlyphTop)
	{
		if (!Caption.IsEmpty()) pFont->Print (font_point.X+add, fp.Y+add+font_add_y, DrawFontColor, Caption);
	}

	if (Layout == GUIBUTTONLAYOUT_GlyphBottom)
	{
		if (!Caption.IsEmpty()) pFont->Print (font_point.X+add, fp.Y+add, DrawFontColor, Caption);
	}

	if (Layout == GUIBUTTONLAYOUT_GlyphLeft)
	{
		if (!Caption.IsEmpty()) pFont->Print (fp.X+add+font_add_x, font_point.Y+add, DrawFontColor, Caption);
	}

	if (Layout == GUIBUTTONLAYOUT_GlyphRight)
	{
		if (!Caption.IsEmpty()) pFont->Print (fp.X+add, font_point.Y+add, DrawFontColor, Caption);
	}


	/*
	if (Caption) pFont->PrintWidth ((nLeft+6 + (title_height-4) + 2), nTop+5, (Width-55), 0xFFFFFFFF, Caption);
	*/

//	cliper.SetRectangle (GetScreenRect());


//	cliper.Pop ();
	GUIControl::Draw ();

}

void GUIButton::DrawButtonDown (int Left, int Top, int Width, int Height)
{
	if (!bEnabled)
	{
		DrawButtonUp (Left, Top, Width, Height);
		return;
	}

	if (!FlatButton)
	{
		GUIHelper::DrawDownBox (Left, Top, Width, Height);
		return;
	}

	if (FlatButtonPressedColor) GUIHelper::Draw2DRect(Left, Top, Width, Height, FlatButtonPressedColor);
	GUIHelper::Draw2DLine(Left, Top, Left+Width, Top, 0xFFACA899);
	GUIHelper::Draw2DLine(Left, Top, Left, Top+Height, 0xFFACA899);
	GUIHelper::Draw2DLine(Left, Top+Height, Left+Width, Top+Height, 0xFFFFFFFF);
	GUIHelper::Draw2DLine(Left+Width, Top, Left+Width, Top+Height, 0xFFFFFFFF);
}

void GUIButton::DrawButtonUp (int Left, int Top, int Width, int Height)
{
	if (!FlatButton)
	{
		GUIHelper::DrawUpBox (Left, Top, Width, Height);
		return;
	}

	if ((mouse_inside) && (bEnabled))
	{
		if (FlatButtonActiveColor) GUIHelper::Draw2DRect(Left, Top, Width, Height, FlatButtonActiveColor);
		GUIHelper::Draw2DLine(Left, Top, Left+Width, Top, 0xFFFFFFFF);
		GUIHelper::Draw2DLine(Left, Top, Left, Top+Height, 0xFFFFFFFF);
		GUIHelper::Draw2DLine(Left, Top+Height, Left+Width, Top+Height, 0xFFACA899);
		GUIHelper::Draw2DLine(Left+Width, Top, Left+Width, Top+Height, 0xFFACA899);
		return;
	}

	if (FlatButtonPassiveColor) GUIHelper::Draw2DRect(Left, Top, Width, Height, FlatButtonPassiveColor);


}


bool GUIButton::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (Visible == false) return false;
  bool Handled = false;

	Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	if (Handled) return Handled;
	
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);

	if ((message == GUIMSG_LMB_DOWN) && (bEnabled))
	{
		Handled = true;
		OnMDown (message, cursor_pos);
	}

	if ((message == GUIMSG_LMB_UP)  && (bEnabled))
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


void GUIButton::OnMDown (int MouseKey, const GUIPoint& pt)
{
	TempDown = true;
	MousePressed = true;
	t_OnMouseClick->Execute(this);
}

void GUIButton::OnMUp (int MouseKey, const GUIPoint& pt)
{
	GUIRectangle sRect = GetScreenRect ();
	if (GUIHelper::PointInRect(pt, sRect))
	{
		if (TempDown == false) return;
		Down = true;
		OnPressed ();
	}

	TempDown = false;
	MousePressed = false;
}


void GUIButton::OnMEnter ()
{
	if (MousePressed) TempDown = true;
	t_OnMouseEnter->Execute (this);
}

void GUIButton::OnMLeave ()
{
	if (TempDown) TempDown = false;
	t_OnMouseLeave->Execute (this);
}


void GUIButton::MouseUp (int button, const GUIPoint& pt)
{
	MousePressed = false;
	GUIControl::MouseUp (button, pt);
}


void GUIButton::OnPressed ()
{
	//
	if (GroupIndex > 0)
	{
		for (DWORD n = 0; n < GetParent()->Childs.Size(); n++)
		{
			if (GetParent()->Childs[n]->Is("GUIButton"))
			{
				GUIButton* button = (GUIButton*)GetParent()->Childs[n];
				if ((button->GroupIndex == GroupIndex) && (button != this))
				{
					button->Down = false;
				}
			}
		}
	}

	Down = true;
	t_OnMousePressed->Execute (this);
}

void GUIButton::ChangePos (int posX, int posY)
{
	DrawRect.Left = posX;
	DrawRect.Top = posY;

	ClientRect = DrawRect;

	ClientRect.Left += 2;
	ClientRect.Top += 2;
	ClientRect.Width -= 4;
	ClientRect.Height -= 4;
}


/*void GUIButton::SetOnPressed(CONTROL_EVENT event_handler, GUIControl* handler_class)
{
	user_event_handler = event_handler;
	user_handler_class = handler_class;
}
*/

#endif