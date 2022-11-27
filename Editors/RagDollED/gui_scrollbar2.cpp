#ifndef _XBOX
#include "gui_scrollbar2.h"

class GUIButton2 : public GUIButton
{
	bool inside;

public:

	GUIButton2(GUIControl *pParent, int Left, int Top, int Width, int Height)
		: GUIButton(pParent,Left,Top,Width,Height)
	{
		inside = false;
	}

	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
	{
		if(!Visible )
			return false;

		switch( message )
		{
			case GUIMSG_MOUSEENTER:
				inside = true;
				break;
		
			case GUIMSG_MOUSELEAVE:
				inside = false;
				break;
		}

		return GUIButton::ProcessMessages(message,lparam,hparam);
	}

	virtual void DrawButtonUp(int Left, int Top, int Width, int Height)
	{
		if(!FlatButton )
		{
			GUIHelper::DrawUpBox(Left,Top,Width,Height);

			return;
		}

		if( inside )
		{
			if( FlatButtonActiveColor  ) GUIHelper::Draw2DRect(Left,Top,Width,Height,FlatButtonActiveColor);
		}
		else
		{
			if( FlatButtonPassiveColor ) GUIHelper::Draw2DRect(Left,Top,Width,Height,FlatButtonPassiveColor);
		}

		if(/* inside &&*/ bEnabled )
		{
			GUIHelper::Draw2DLine(Left,Top,Left + Width,Top,0xFFFFFFFF);
			GUIHelper::Draw2DLine(Left,Top,Left,Top + Height,0xFFFFFFFF);
			GUIHelper::Draw2DLine(Left,Top + Height,Left + Width,Top + Height,0xFFACA899);
			GUIHelper::Draw2DLine(Left + Width,Top,Left + Width,Top + Height,0xFFACA899);
		}
	}

};

GUIScrollBar2::GUIScrollBar2 (GUIControl* pParent, GUIScrollBarKind sbKind, int Left, int Top, int Width, int Height) : GUIControl (pParent)
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
		button_less = NEW GUIButton2(this, 2, 2, 15, 15);
		button_less->Glyph->Load ("left");
		button_great = NEW GUIButton2(this, (Width-16)-1, 2, 15, 15);
		button_great->Glyph->Load ("right");
	} else
	{
		button_less = NEW GUIButton2(this, 3, 2, 15, 15);
		button_less->Glyph->Load ("up");
		button_great = NEW GUIButton2(this, 3, Height-17, 15, 15);
		button_great->Glyph->Load ("down");
	}
	
	
	button_great->OnMouseDown = (CONTROL_EVENT)&GUIScrollBar2::MoreButtonDown;
	//OnMouseUp
	button_less->OnMouseDown = (CONTROL_EVENT)&GUIScrollBar2::LessButtonDown;
	
	
	t_OnChange = NEW GUIEventHandler;

	x_off = 0;
}

GUIScrollBar2::~GUIScrollBar2 ()
{
	delete button_less;
  delete button_great;
  
  delete t_OnChange;
	
}

void GUIScrollBar2::Draw ()
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
	
//	GUIHelper::DrawScrollBarBox (nLeft, nTop, Width, Height);
	GUIHelper::Draw2DRect(nLeft + 2,nTop + 2,Width - 3,Height - 3,0xfff3f3e6);
	
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

	//	DrawSlider(rect);
	}
	else
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

	//	DrawSlider(rect);
	}

	button_less->Draw();
	button_great->Draw();

	DrawSlider(rect);
}

bool GUIScrollBar2::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
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

void _cdecl GUIScrollBar2::MoreButtonDown (GUIControl* sender)
{
	Position++;
	if (Position > Max) Position = Max;
	
	t_OnChange->Execute (this);
}

void _cdecl GUIScrollBar2::LessButtonDown (GUIControl* sender)
{
	Position--;
	if (Position < Min) Position = Min;
	
	t_OnChange->Execute (this);
}



void GUIScrollBar2::OnMDown(int MouseKey, const GUIPoint &pt)
{
	mouseisdown = true;

	GUIPoint p(pt.x,pt.y);

	if( slider_rect.Inside(p))
	{
		x_off = p.x - slider_rect.x - 1;
	}
	else
	{
		x_off = slider_rect.w/2;
	}

	MakeScroll(p);
}


void GUIScrollBar2::MouseUp (int button, const GUIPoint& pt)
{
	x_off = 0;

	mouseisdown = false;
}



void GUIScrollBar2::MakeScroll (const GUIPoint &pt)
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
			int position = pt.X - r.Left - x_off;
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


void GUIScrollBar2::ChangeSize (int Width, int Height)
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


void GUIScrollBar2::SetDrawRect (const GUIRectangle& rect)
{
	DrawRect = rect;
	ChangeSize (rect.w, rect.h);
}


void GUIScrollBar2::MakeFlat (bool enable)
{
	Color pas(0xffd4d0c8);
	Color act(pas);

	act.r *= 1.05f;
	act.g *= 1.05f;
	act.b *= 1.05f;

	Flat = enable;
	button_less->FlatButton = enable;
	button_less->FlatButtonActiveColor = act;
	button_less->FlatButtonPressedColor = act;
	button_less->FlatButtonPassiveColor = pas;
	
	button_great->FlatButton = enable;
	button_great->FlatButtonActiveColor = act;
	button_great->FlatButtonPressedColor = act;
	button_great->FlatButtonPassiveColor = pas;
}


void GUIScrollBar2::MouseMove( int button, const GUIPoint& pt)
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

void GUIScrollBar2::DrawSlider(const GUIRectangle &rect)
{
	int Left = slider_rect.x;
	int Top = slider_rect.y;
	int Width = slider_rect.w;
	int Height = slider_rect.h;

	if(!Flat)
	{
		GUIHelper::DrawUpBox(Left,Top,Width,Height);
		return;
	}

	Color col(0xffd4d0c8);

	if(!MouseInsideSlider)
	{
		GUIHelper::Draw2DRect(Left,Top,Width,Height,col);

		col.r *= 0.9f;
		col.g *= 0.9f;
		col.b *= 0.9f;
	}
	else
	{
		col.r *= 1.05f;
		col.g *= 1.05f;
		col.b *= 1.05f;

		GUIHelper::Draw2DRect(Left,Top,Width,Height,col);
	}

	GUIHelper::Draw2DLine(Left,Top,Left + Width,Top,0xFFFFFFFF);
	GUIHelper::Draw2DLine(Left,Top,Left,Top + Height,0xFFFFFFFF);
	GUIHelper::Draw2DLine(Left,Top + Height,Left + Width,Top + Height,0xFFACA899);
	GUIHelper::Draw2DLine(Left + Width,Top,Left + Width,Top + Height,0xFFACA899);
}

void GUIScrollBar2::OwnerRescaled(int x, int y, int width, int height)
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