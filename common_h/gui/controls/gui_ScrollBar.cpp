#ifndef _XBOX
#include "gui_scrollbar.h"

GUIScrollBar::GUIScrollBar (GUIControl* pParent, GUIScrollBarKind sbKind, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	Flat = false;
	MouseInsideSlider = false;
	slider_rect.x = 0;
	slider_rect.y = 0;
	slider_rect.w = 0;
	slider_rect.h = 0;
	PageSize = 16;
	
	Max = -1;
	Min = 0;
	Position = 0;
	
	
	Kind = sbKind;
	
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;
	
	mouseisdown = false;
	
	
	
	if (Kind == GUISBKIND_Horizontal)
	{
		button_less = NEW GUIButton (this, 2, 2, 15, 15);
		button_less->Glyph->Load ("left");
		button_great = NEW GUIButton (this, (Width-16)-1, 2, 15, 15);
		button_great->Glyph->Load ("right");
	} else
	{
		button_less = NEW GUIButton (this, 3, 2, 15, 15);
		button_less->Glyph->Load ("up");
		button_great = NEW GUIButton (this, 3, Height-17, 15, 15);
		button_great->Glyph->Load ("down");
	}
	
	
	button_great->OnMouseDown = (CONTROL_EVENT)&GUIScrollBar::MoreButtonDown;
	//OnMouseUp
	button_less->OnMouseDown = (CONTROL_EVENT)&GUIScrollBar::LessButtonDown;
	
	
	t_OnChange = NEW GUIEventHandler;
}

GUIScrollBar::~GUIScrollBar ()
{
	delete button_less;
	delete button_great;
	delete t_OnChange;
}

void GUIScrollBar::Draw ()
{

	bool bFullBar = false;
	if (PageSize > (Max-Min)) bFullBar = true;

	if (Visible == false) return;



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
	
	
//	cliper.SetFullScreenRect ();
	
	GUIHelper::DrawScrollBarBox (nLeft, nTop, Width, Height);
	
	if (Kind == GUISBKIND_Horizontal)
	{
		
		// С какой позиции в пикселях
		int SliderFrom = (nLeft+18);
		int SliderTo = (nLeft+Width-18);

		// Длинна слайдера в пикселях
		float PathLen = (float)(SliderTo-SliderFrom);
		// Длина слайдера в значениях
		float SliderValLen = (float)(Max - Min);
		// Размер для одного элемента
		float OneSize = PathLen / SliderValLen;

		
		SliderFrom = (nLeft+18);
		SliderTo = (nLeft+Width-18)-(int)(OneSize*PageSize);


		PathLen = (float)(SliderTo-SliderFrom);
		// позиция на слайдере в процентах 0..1
		float oPos = ((float)Position / (float)SliderValLen);

		// позиция слайдера в пикселях
		int SliderPos = SliderFrom  + (int)(PathLen*oPos);
		
		// Ширина слайдера
		float sbWidth = PageSize * OneSize;
		
		slider_rect.x = SliderPos;
		slider_rect.y = nTop+2;
		slider_rect.w = (int)sbWidth;
		slider_rect.h = 15;

		if (bFullBar)
		{
			slider_rect.x = SliderFrom;
			slider_rect.y = nTop+2;
			slider_rect.h = 15;
			slider_rect.w = (int)Width-18-15-3;
		}


		DrawSlider ();
		
	} else
	{
		
		int SliderFrom = (nTop+18);
		int SliderTo = (nTop+Height-18);
		
		// Длинна слайдера в пикселях
		float PathLen = (float)(SliderTo-SliderFrom);

		// Длина слайдера в значениях
		float SliderValLen = (float)(Max - Min);
		// Размер для одного элемента
		float OneSize = PathLen / SliderValLen;
		
		SliderFrom = (nTop+18);
		SliderTo = (nTop+Height-18)-(int)(OneSize*PageSize);



		PathLen = (float)(SliderTo-SliderFrom);
		// позиция на слайдере в процентах 0..1
		float oPos = ((float)Position / (float)SliderValLen);

		// позиция слайдера в пикселях
		int SliderPos = SliderFrom  + (int)(PathLen*oPos);
		
		// Высота слайдера
		float sbHeight = PageSize * OneSize;
		
		slider_rect.x = nLeft+3;
		slider_rect.y = SliderPos;
		slider_rect.w = 15;
		slider_rect.h = (int)sbHeight;

		if (bFullBar)
		{
			slider_rect.x = nLeft+3;
			slider_rect.y = SliderFrom;
			slider_rect.w = 15;
			slider_rect.h = (int)Height-18-15-3;
		}

		
		DrawSlider ();
	}

	button_less->Draw ();
	button_great->Draw ();

}


bool GUIScrollBar::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
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
	
	if (message == GUIMSG_LMB_DBLCLICK)
	{
		return true;
	}

	return Handled;
}

void _cdecl GUIScrollBar::MoreButtonDown (GUIControl* sender)
{
	Position++;
	if (Position > Max) Position = Max;
	
	t_OnChange->Execute (this);
}

void _cdecl GUIScrollBar::LessButtonDown (GUIControl* sender)
{
	Position--;
	if (Position < Min) Position = Min;
	
	t_OnChange->Execute (this);
}


void GUIScrollBar::OnMDown (int MouseKey, const GUIPoint& pt)
{
  mouseisdown = true;
	MakeScroll (pt);
}


void GUIScrollBar::MouseUp (int button, const GUIPoint& pt)
{
	mouseisdown = false;
}



void GUIScrollBar::MakeScroll (const GUIPoint& pt)
{
	if (!mouseisdown) return;

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

	if (Kind == GUISBKIND_Horizontal)
	{
		// С какой позиции в пикселях
		int SliderFrom = (nLeft+18);
		int SliderTo = (nLeft+Width-18);

		// Длинна слайдера в пикселях
		float PathLen = (float)(SliderTo-SliderFrom);
		// Длина слайдера в значениях
		float SliderValLen = (float)(Max - Min);
		// Размер для одного элемента
		float OneSize = PathLen / SliderValLen;

		SliderFrom = (nLeft+18);
		SliderTo = (nLeft+Width-18)-(int)(OneSize*PageSize);

		// Получили с какого пиксела по какой у нас скролбар
		GUIRectangle r;
		r.Left = SliderFrom; 
		r.Width = SliderTo-SliderFrom+(int)(OneSize*PageSize);
		r.Top = 0;
		r.Height = 9999;

		if (GUIHelper::PointInRect(pt, r))
		{
			PathLen = (float)(SliderTo-SliderFrom);
			int position = pt.X - r.Left;
			PathLen = (float)(SliderTo-SliderFrom);
			float percent = ((float)position / PathLen);
			if (percent < 0.0f) percent = 0.0f;
			if (percent > 1.0f) percent = 1.0f;

			Position = (int)((float)SliderValLen * percent);

			t_OnChange->Execute (this);
			return;
		}
	} else
		{
			int SliderFrom = (nTop+18);
			int SliderTo = (nTop+Height-18);
			
			// Длинна слайдера в пикселях
			float PathLen = (float)(SliderTo-SliderFrom);
			// Длина слайдера в значениях
			float SliderValLen = (float)(Max - Min);
			// Размер для одного элемента
			float OneSize = PathLen / SliderValLen;
			
//			SliderFrom = (nTop+18);
//			SliderTo = (nTop+Height-18)-(int)(OneSize*PageSize)+19;

			// Получили с какого пиксела по какой у нас скролбар
			GUIRectangle r;
			r.Left = 0; 
			r.Width = 9999;
			r.Top = SliderFrom;
			r.Height = SliderTo-SliderFrom+(int)(OneSize*PageSize)+19;

			if (GUIHelper::PointInRect(pt, r))
			{
				PathLen = (float)(SliderTo-SliderFrom);
				int position = pt.Y - r.Top;
				PathLen = (float)(SliderTo-SliderFrom);
				float percent = ((float)position / PathLen);
				if (percent < 0.0f) percent = 0.0f;
				if (percent > 1.0f) percent = 1.0f;

				Position = (int)((float)SliderValLen * percent);

				t_OnChange->Execute (this);
				return;
			}



		}


}


void GUIScrollBar::ChangeSize (int Width, int Height)
{
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;

	if (Kind == GUISBKIND_Horizontal)
	{
		button_great->ChangePos ((Width-16)-1, 2);
	} else
	{
		button_great->ChangePos (3, Height-17);
	}

}


void GUIScrollBar::SetDrawRect (const GUIRectangle& rect)
{
	DrawRect = rect;
	ChangeSize (rect.w, rect.h);
}


void GUIScrollBar::MakeFlat (bool enable)
{
	Flat = enable;
	button_less->FlatButton = enable;
	button_less->FlatButtonActiveColor = 0xFFD4D0C8;
	button_less->FlatButtonPassiveColor = 0xFFC5C5C5;
	
	button_great->FlatButton = enable;
	button_great->FlatButtonActiveColor = 0xFFD4D0C8;
	button_great->FlatButtonPressedColor = 0xFFD4D0C8;
	button_great->FlatButtonPassiveColor = 0xFFC5C5C5;

}


void GUIScrollBar::MouseMove( int button, const GUIPoint& pt)
{
	MakeScroll (pt);

	if (slider_rect.Inside(pt))
	{
		MouseInsideSlider = true;
	} else
		{
			MouseInsideSlider = false;
		}
}

void GUIScrollBar::DrawSlider ()
{
	if (!Flat)
	{
		GUIHelper::DrawUpBox (slider_rect.x, slider_rect.y, slider_rect.w, slider_rect.h);
		return;
	}

	dword dwColor = 0xFFC5C5C5;


	if (!MouseInsideSlider)
	{
		GUIHelper::Draw2DRect(slider_rect.x, slider_rect.y, slider_rect.w, slider_rect.h, dwColor);
	} else
		{
			int Left = slider_rect.x;
			int Top = slider_rect.y;
			int Width = slider_rect.w;
			int Height = slider_rect.h;
			GUIHelper::Draw2DRect(Left, Top, Width, Height, dwColor);
			GUIHelper::Draw2DLine(Left, Top, Left+Width, Top, 0xFFFFFFFF);
			GUIHelper::Draw2DLine(Left, Top, Left, Top+Height, 0xFFFFFFFF);
			GUIHelper::Draw2DLine(Left, Top+Height, Left+Width, Top+Height, 0xFFACA899);		
			GUIHelper::Draw2DLine(Left+Width, Top, Left+Width, Top+Height, 0xFFACA899);
		}
	
}

void GUIScrollBar::OwnerRescaled(int x, int y, int width, int height)
{
	DrawRect.Left = x;
	DrawRect.Top = y;
	DrawRect.Width = width;
	DrawRect.Height = height;

	ClientRect = DrawRect;


	if (Kind == GUISBKIND_Horizontal)
	{
		button_less->ChangePos(2, 2);
		button_great->ChangePos((DrawRect.Width-16)-1, 2);
	} else
	{
		button_less->ChangePos(3, 2);
		button_great->ChangePos(3, (DrawRect.Height-17));
	}

}

#endif