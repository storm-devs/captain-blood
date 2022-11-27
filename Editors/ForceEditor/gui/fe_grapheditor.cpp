#ifndef _XBOX
//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*  description:
//*
//****************************************************************

#include "fe_grapheditor.h"


#define BACKGROUND_COLOR 0xFFFFFFFF
#define LINE_COLOR 0xFFEBEBEB
#define TENLINE_COLOR 0xFFCDCDCD
#define HUNDREDLINE_COLOR 0xFF999999
#define AXIS_COLOR 0xFF000000
#define FONT_COLOR 0xFF000000
#define GRAPHBORDERS_COLOR 0xFF000000
#define SMALLFONT_COLOR 0xFF000000

#define MAXXPOS 8000.0f  /* Самая большая позиция по оси X */

#define PTSIZE 2 /* Размер рендерящихся вершин */
#define PTHOTSPOT 6 /* Размер вершин для кликанья мышкой*/

#define XOFFSET 30 /* Смещение всего графика по X */
#define YOFFSET 10 /* Смещение всего графика по Y */

#define MIN_DISTANCE_BETWEEN_POINTS 0.01f /* Максимально возможное расстояние между точками*/

#define UNITS_IN_ONE_PIXEL 0.01f /* Сколько единиц в одном пикселе */


char TmpChar8K[8192];


GUIGraphEditor::GUIGraphEditor (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	mbBorned = false;
	SelectedLine = NULL;
	Lines = NEW GUILineList ();

	Mode = GEM_EDIT;
	CtrlState = true;
	NegativeValues = false;
	OffsetX = 10.0f;
	OffsetY = -YOFFSET;


	MouseButtonPressed = false;
	RMouseButtonPressed = false;

	HorizontalAxeDesc = "X";
	VerticalAxeDesc = "Y";

	LockXScale = false;
	LockYScale = false;



	pSmallFont = NEW GUIFont ("arialcyrsmall");
	if (pSmallFont)	pSmallFont->SetSize(9.0f);

	pFont = NEW GUIFont ("arialcyrsmall");


	pRS = (IRender*)api->GetService("DX9Render");	
	
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;

	GridStepX = 1.0f;
	GridStepY = 1.0f;
	ScaleX = 4.5f;
	ScaleY = 4.5f;


}

GUIGraphEditor::~GUIGraphEditor ()
{
	
	if (pFont) delete pFont;
	if (pSmallFont) delete pSmallFont;

	pFont = NULL;
	pSmallFont = NULL;
	


	delete Lines;
}

void GUIGraphEditor::Draw ()
{
	if (Visible == false) return;
	GUIControl::Draw ();

	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	
	GUIRectangle rect = GetDrawRect ();
	this->ClientToScreen (rect);
	

	GUIHelper::DrawDownBorder(rect.x, rect.y , Width, Height);

	cliper.Push();
	GUIRectangle r;
	r = rect;
	r.x += 2;
	r.y += 2;
	r.w = Width - 4;
	r.h = Height - 4;
	cliper.SetRectangle(r);

	//render...
	DrawBackGround ();
	DrawLine ();

	cliper.Pop();

/*	if ((GetAsyncKeyState(VK_DELETE) < 0) && (!mbBorned))
	{
		GUIMessageBox* mb = GetMainControl()->Application->MessageBox("Clear graph ?", "Warning", GUIMB_YESNO);
		mb->OnOK = (CONTROL_EVENT)&GUIGraphEditor::OnClearYes;
		mb->OnCancel = (CONTROL_EVENT)&GUIGraphEditor::OnClearNo;
		mbBorned = true;
	}*/
	if( GetAsyncKeyState(VK_DELETE) < 0 && !mbBorned && SelectedLine )
	{
		GUIMessageBox *mb = GetMainControl()->Application->MessageBox("Delete point?","Warning",GUIMB_YESNO);

		mb->OnOK	 = (CONTROL_EVENT)&GUIGraphEditor::OnDeleteYes;
		mb->OnCancel = (CONTROL_EVENT)&GUIGraphEditor::OnDeleteNo;

		mbBorned = true;
	}


	if ((GetAsyncKeyState(VK_CONTROL) < 0) && (GetAsyncKeyState(VK_ADD) < 0))
	{
		float ScaleFactor = (ScaleY / 450.0f);
		float Dist = 25.0;
		ScaleY += (Dist * ScaleFactor);

	}

	if ((GetAsyncKeyState(VK_CONTROL) < 0) && (GetAsyncKeyState(VK_SUBTRACT) < 0))
	{
		float ScaleFactor = (ScaleY / 450.0f);
		float Dist = -25.0;
		ScaleY += (Dist * ScaleFactor);
		if (ScaleY < 0.1) ScaleY = 0.1f;
	}

}


void _cdecl GUIGraphEditor::OnClearYes (GUIControl* sender)
{
	mbBorned = false;
	for (int n = 0; n < Lines->GetCount(); n++)
	{
		GUIGraphLine* GraphLine = Lines->GetLine(n);
		GraphLine->Zero ();
	}

}

void _cdecl GUIGraphEditor::OnClearNo (GUIControl* sender)
{
	mbBorned = false;
}


void _cdecl GUIGraphEditor::OnDeleteYes(GUIControl *sender)
{
	mbBorned = false;

	if( SelectedLine && SelectedLine->SelectedPoint >= 0 )
	{
		SelectedLine->Remove(SelectedLine->SelectedPoint);
		SelectedLine->SelectedPoint = -1;
	}
}

void _cdecl GUIGraphEditor::OnDeleteNo (GUIControl *sender)
{
	mbBorned = false;
}


bool GUIGraphEditor::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (!Visible) return false;
//	LockXScale = false;
//	LockYScale = false;
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);

	CtrlState = false;
	if (GetAsyncKeyState(VK_CONTROL) < 0) CtrlState = true;
		

	
	if (message == GUIMSG_LMB_DOWN) 
	{
		OnMDown (message, cursor_pos);
		if (CtrlState)
		{
			if (SelectedLine)
				if (SelectedLine->SelectedPoint != -1)
						SelectedLine->Remove(SelectedLine->SelectedPoint);
		}
		
	}

	if (message == GUIMSG_RMB_DOWN) 
	{
		RMouseButtonPressed = true;
		RBegMousePressed = cursor_pos;
		GetMainControl()->Application->GetCursor()->Push();
		GetMainControl()->Application->GetCursor()->SetCursor("hand");
	}
	

/*
	if (message == GUIMSG_KEYPRESSED)
	{
		int Key = (int)lparam;

		if ((Key == VK_DELETE) && (GraphLine->SelectedPoint != -1))
		{
			if (GraphLine->GetCount() > 2)
			{
				GraphLine->Remove(GraphLine->SelectedPoint);
				GraphLine->SelectedPoint = -1;
			}
		}

		if ((Key == VK_ADD) && ((!LockXScale) && (!LockYScale)))
		{
			float ScaleFactor = (ScaleX / 450.0f);
			ScaleX -= ScaleFactor;
			ScaleY = ScaleX;
		}
		if ((Key == VK_SUBTRACT) && ((!LockXScale) && (!LockYScale)))
		{
			float ScaleFactor = (ScaleX / 450.0f);
			ScaleX += ScaleFactor;
			ScaleY = ScaleX;
		}
		if ((Key == 33) && (!LockYScale)) // Page up
		{
			float ScaleFactor = (ScaleY / 450.0f);
			ScaleY -= ScaleFactor;
		}
		if ((Key == 34) && (!LockYScale)) // Page down
		{
			float ScaleFactor = (ScaleY / 450.0f);
			ScaleY += ScaleFactor;
		}
		if ((Key == 36) && (!LockXScale)) // home
		{
			float ScaleFactor = (ScaleX / 450.0f);
			ScaleX -= ScaleFactor;
		}
		if ((Key == 35) && (!LockXScale)) // end
		{
			float ScaleFactor = (ScaleX / 450.0f);
			ScaleX += ScaleFactor;
		}
	}

*/

	return GUIControl::ProcessMessages (message, lparam, hparam);
}


void GUIGraphEditor::DrawBackGround ()
{
	float fScalerY = 10.0f;
	float fScalerX = 10.0f;
	ClampScale ();
	pRS->Clear(0, NULL, CLEAR_STENCIL | CLEAR_TARGET | CLEAR_ZBUFFER, BACKGROUND_COLOR, 1.0f, 0);


	if (ScaleY > 2.0) fScalerY = 1.0f;
	if (ScaleX > 2.0) fScalerX = 1.0f;

	// 0.01 - в 100 пикселях одна единица графа
	float UnitsInOnePixelX = UNITS_IN_ONE_PIXEL * ScaleX;
	float UnitsInOnePixelY = UNITS_IN_ONE_PIXEL * ScaleY;

	GUIRectangle screenrect = DrawRect;
	ClientToScreen(screenrect);
	float Width = (float)DrawRect.w;
	float Height = (float)DrawRect.h;

	int StepXCount = (int)((Width * UnitsInOnePixelX) * fScalerX);

	int BigGrid = 0;
	if (ScaleX > 12) BigGrid = 1;
	if (ScaleX > 145) BigGrid = 2;

	bool ZeroLineIsDrawed = false;
	int ZeroLineY = -1;
	int ZeroLineHeight = -1;
	int ZeroLineX = -1;

	int StartFromX = (int)((-OffsetX * UnitsInOnePixelX) * fScalerX);
	for (int rx = StartFromX; rx <= StartFromX+StepXCount+1;rx++)
	{
		float x = (float)rx;
		if ((BigGrid == 1) && ((rx % 10) != 0)) continue;
		if ((BigGrid == 2) && ((rx % 100) != 0)) continue;
		int xpos = screenrect.x + (int)((x / UnitsInOnePixelX) / fScalerX);
		xpos += XOFFSET;
		DWORD clr = LINE_COLOR;
		if ((rx % 10) == 0) clr = TENLINE_COLOR;
		if ((rx % 100) == 0) clr = HUNDREDLINE_COLOR;
		if (x == 0) clr = AXIS_COLOR;
		xpos += (int)OffsetX;
		if (xpos < screenrect.x + XOFFSET) continue;

		if (clr == AXIS_COLOR)
		{
			ZeroLineIsDrawed = true;
			ZeroLineY = screenrect.y;
			ZeroLineHeight = (int)Height-YOFFSET;
			ZeroLineX = xpos+1;
			//			GUIHelper::DrawVertLine(screenrect.y, (int)Height-YOFFSET,  xpos+1, clr);
		} else
		{
			GUIHelper::DrawVertLine(screenrect.y, (int)Height-YOFFSET,  xpos, clr);

			// Печатаем через 1
			if (!(rx & 1))
			{
				crt_snprintf (TmpChar8K, 7000, "%3.1f", (float)rx / fScalerX);
				int sf_width_div2 = pSmallFont->GetWidth(TmpChar8K) / 2;
				pSmallFont->Print(xpos-sf_width_div2, screenrect.y + (int)Height  -YOFFSET, SMALLFONT_COLOR, TmpChar8K);
			}
		}
	}



	int StepYCount = 0;
	StepYCount = (int)((Height * UnitsInOnePixelY) * fScalerY);
	BigGrid = 0;
	if (ScaleY > 12) BigGrid = 1;
	if (ScaleY > 145) BigGrid = 2;

	int StartFromY = (int)((OffsetY * UnitsInOnePixelY) * fScalerY);
	for (int ry = StartFromY; ry <= StartFromY + StepYCount+1;ry++)
	{
		float y = (float)ry;
		if ((BigGrid == 1) && ((ry % 10) != 0)) continue;
		if ((BigGrid == 2) && ((ry % 100) != 0)) continue;

		DWORD clr = LINE_COLOR;
		if ((ry % 10) == 0) clr = TENLINE_COLOR;
		if ((ry % 100) == 0) clr = HUNDREDLINE_COLOR;
		if (y == 0) clr = AXIS_COLOR;

		int ypos = (screenrect.y+(int)Height) - (int)((y / UnitsInOnePixelY) / fScalerY);
		//ypos /= 100;
		ypos -= 4;
		ypos += (int)OffsetY;

		if (ypos > (screenrect.y+(int)Height)-YOFFSET) continue;
		GUIHelper::DrawHorizLine(screenrect.x+XOFFSET+1, (int)Width,  ypos, clr);

		// Печатаем через 1
		if (!(ry & 1))
		{
			crt_snprintf (TmpChar8K, 7000, "%3.1f", (float)ry / fScalerY);
			int sf_width = pSmallFont->GetWidth(TmpChar8K);
			int sf_height_div2 = pSmallFont->GetHeight() / 2;
			pSmallFont->Print(screenrect.x+XOFFSET-sf_width-3, ypos-sf_height_div2, SMALLFONT_COLOR, TmpChar8K);
			if (clr == AXIS_COLOR)
			{
				pSmallFont->Print(screenrect.x+XOFFSET-sf_width-3+1, ypos-sf_height_div2, SMALLFONT_COLOR, TmpChar8K);
			}

		}

		if (clr == AXIS_COLOR)
		{
			GUIHelper::DrawHorizLine(screenrect.x+XOFFSET+1, (int)Width,  ypos+1, clr);
		}
	}

	if (ZeroLineIsDrawed)
	{
		GUIHelper::DrawVertLine(ZeroLineY, ZeroLineHeight,  ZeroLineX, AXIS_COLOR);
		GUIHelper::DrawVertLine(ZeroLineY, ZeroLineHeight,  ZeroLineX+1, AXIS_COLOR);

		crt_snprintf (TmpChar8K, 7000, "0");
		int sf_width_div2 = pSmallFont->GetWidth(TmpChar8K) / 2;
		pSmallFont->Print(ZeroLineX-sf_width_div2, ZeroLineY + (int)Height  -YOFFSET, SMALLFONT_COLOR, TmpChar8K);
		pSmallFont->Print(ZeroLineX-sf_width_div2+1, ZeroLineY + (int)Height  -YOFFSET, SMALLFONT_COLOR, TmpChar8K);

	}



	GUIHelper::DrawVertLine(screenrect.y, (int)Height-YOFFSET,  screenrect.x+XOFFSET, GRAPHBORDERS_COLOR);
	GUIHelper::DrawHorizLine(screenrect.x+XOFFSET, (int)Width,  screenrect.y+(int)Height-YOFFSET, GRAPHBORDERS_COLOR);


	int HZ_Width = pFont->GetWidth(HorizontalAxeDesc.GetBuffer());
	int HZ_Height = pFont->GetHeight();
	pFont->Print(screenrect.x+XOFFSET+2, screenrect.y, FONT_COLOR, "%s", VerticalAxeDesc.GetBuffer());
	pFont->Print(screenrect.x+(int)Width-HZ_Width-2, screenrect.y+(int)Height-YOFFSET-HZ_Height, FONT_COLOR, "%s", HorizontalAxeDesc.GetBuffer());


	//pSmallFont
	/*

	int DigitSize = pFont->GetWidth("9");
	int FHd2= pFont->GetHeight() / 2;
	int FWd2= DigitSize;
	if (StepX >= 10) FWd2 += DigitSize;
	if (StepX >= 100) FWd2 += DigitSize;
	if (StepX >= 1000) FWd2 += DigitSize;


	int XBeginFrom = -OffsetX * UnitsInOnePixelX;
	if (RStepX <= 0) RStepX = 1;
	if (MaxY == 0) MaxY = 1;
	int DescWidth = pFont->GetWidth(HorizontalAxeDesc.GetBuffer());
	pFont->Print(screenrect.x+(int)Width-DescWidth-8, screenrect.y+(int)Height - (FHd2*4) - 4, FONT_COLOR, "%s", HorizontalAxeDesc.GetBuffer());
	pFont->Print(screenrect.x+(int)Width-FWd2-8, screenrect.y+(int)Height - (FHd2*2) - 4, FONT_COLOR, "%d", RStepX+XBeginFrom);

	int YBeginFrom = OffsetY * UnitsInOnePixelY;
	pFont->Print(screenrect.x+XOFFSET+3, screenrect.y + 4 , FONT_COLOR, "%s", VerticalAxeDesc.GetBuffer());
	pFont->Print(screenrect.x+XOFFSET+3, screenrect.y + 4 + (FHd2*2), FONT_COLOR, "%d", MaxY+YBeginFrom);
	pFont->Print(screenrect.x+XOFFSET+3, screenrect.y+(int)Height - (FHd2*2) - 4, FONT_COLOR, "%d", MinY+YBeginFrom);


	*/	
}


void GUIGraphEditor::DrawLine ()
{
	GUIRectangle screenrect = DrawRect;
	ClientToScreen(screenrect);

	cliper.Push();

	GUIRectangle cliprect = screenrect;
	cliprect.x += XOFFSET;
	cliprect.w -= XOFFSET;
	cliprect.h -= YOFFSET;
	cliper.SetRectangle(cliprect);


	for (int n = 0; n < Lines->GetCount(); n++)
	{
		GUIGraphLine* GraphLine = Lines->GetLine(n);
		GraphLine->bNegative = NegativeValues;
		for (int i = 0; (i < GraphLine->GetCount()-1); i++)
		{
			Vector pt1 = GraphLine->GetPoint(i);
			Vector pt2 = GraphLine->GetPoint(i+1);

			if (!NegativeValues)
			{
				if (pt1.y < 0) pt1.y = 0;
				if (pt2.y < 0) pt2.y = 0;
			}

			TransformPoint (pt1, pt1);
			TransformPoint (pt2, pt2);

			if (pt1.x > screenrect.x + screenrect.w) break;
			if (pt2.x < screenrect.x + XOFFSET) continue;



			GUIHelper::Draw2DLine((int)pt1.x, (int)pt1.y , (int)pt2.x, (int)pt2.y, GraphLine->color);
		//	if (GraphLine->bActive) GUIHelper::Draw2DLine((int)pt1.x, (int)(pt1.y+1) , (int)pt2.x, (int)(pt2.y+1), GraphLine->color);
			if (GraphLine->bActive)
			{
				if( pt2.x - pt1.x > fabsf(pt2.y - pt1.y))
					GUIHelper::Draw2DLine((int)pt1.x, (int)(pt1.y+1) , (int)pt2.x, (int)(pt2.y+1), GraphLine->color);
				else
					GUIHelper::Draw2DLine((int)(pt1.x+1), (int)pt1.y , (int)(pt2.x+1), (int)pt2.y, GraphLine->color);
			}
		}

	//	for ( i = 0; (i < GraphLine->GetCount()-1); i++)
		for ( i = 0; (i < GraphLine->GetCount()  ); i++)
		{
			Vector pt1 = GraphLine->GetPoint(i);

			if (!NegativeValues)
				if (pt1.y < 0) pt1.y = 0;

			TransformPoint (pt1, pt1);
			if (pt1.x > screenrect.x + screenrect.w) break;
			if (pt1.x < screenrect.x + XOFFSET) continue;

			bool bSel = false;
			if (GraphLine->SelectedPoint == i) bSel = true;
			DrawPoint (pt1, GraphLine->color, bSel);
	}

		}


	cliper.Pop();
}

void GUIGraphEditor::DrawPoint (const Vector& pos, DWORD color, bool Selected)
{
	GUIHelper::Draw2DLine ((int)pos.x - PTSIZE, (int)pos.y - PTSIZE, (int)pos.x + PTSIZE, (int)pos.y - PTSIZE, color);
	GUIHelper::Draw2DLine ((int)pos.x - PTSIZE, (int)pos.y + PTSIZE, (int)pos.x + PTSIZE, (int)pos.y + PTSIZE, color);
	GUIHelper::Draw2DLine ((int)pos.x - PTSIZE, (int)pos.y - PTSIZE, (int)pos.x - PTSIZE, (int)pos.y + PTSIZE, color);
	GUIHelper::Draw2DLine ((int)pos.x + PTSIZE, (int)pos.y - PTSIZE, (int)pos.x + PTSIZE, (int)pos.y + PTSIZE, color);

	if (!Selected) return;

	GUIHelper::Draw2DLine ((int)pos.x - PTSIZE-1, (int)pos.y - PTSIZE-1, (int)pos.x + PTSIZE+1, (int)pos.y - PTSIZE-1, color);
	GUIHelper::Draw2DLine ((int)pos.x - PTSIZE-1, (int)pos.y + PTSIZE+1, (int)pos.x + PTSIZE+1, (int)pos.y + PTSIZE+1, color);
	GUIHelper::Draw2DLine ((int)pos.x - PTSIZE-1, (int)pos.y - PTSIZE-1, (int)pos.x - PTSIZE-1, (int)pos.y + PTSIZE+1, color);
	GUIHelper::Draw2DLine ((int)pos.x + PTSIZE+1, (int)pos.y - PTSIZE-1, (int)pos.x + PTSIZE+1, (int)pos.y + PTSIZE+1, color);



}

void GUIGraphEditor::UnTransformPoint (const Vector& from, Vector& to)
{
	Vector pt1 = from;

	GUIRectangle screenrect = DrawRect;
	ClientToScreen(screenrect);

	float UnitsInOnePixelX = UNITS_IN_ONE_PIXEL * ScaleX;
	float UnitsInOnePixelY = UNITS_IN_ONE_PIXEL * ScaleY;



	int Zero = screenrect.y + screenrect.h;
	Zero-=4;

	pt1.x -= OffsetX;
	pt1.y -= OffsetY;

	pt1.y = Zero + pt1.y;
	pt1.x -= XOFFSET;
	pt1.x -= screenrect.x;

	pt1.x *= UnitsInOnePixelX;
	pt1.y *= UnitsInOnePixelY;
	
	to = pt1;
}


void GUIGraphEditor::TransformPoint (const Vector& from, Vector& to)
{
	Vector pt1 = from;
	
	GUIRectangle screenrect = DrawRect;
	ClientToScreen(screenrect);

	float UnitsInOnePixelX = UNITS_IN_ONE_PIXEL * ScaleX;
	float UnitsInOnePixelY = UNITS_IN_ONE_PIXEL * ScaleY;

	
	pt1.x /= UnitsInOnePixelX;
	pt1.y /= UnitsInOnePixelY;
	int Zero = screenrect.y + screenrect.h;
	Zero-=4;
	pt1.x += screenrect.x;
	pt1.x += XOFFSET;
	pt1.y = Zero - pt1.y;

	pt1.x += OffsetX;
	pt1.y += OffsetY;

	to = pt1;
}


void GUIGraphEditor::OnMDown (int MouseKey, const GUIPoint& pt)
{
	Lines->SortByActive ();
	BegMousePressed = pt;
	MouseButtonPressed = true;

	if ((Mode == GEM_EDIT) || (Mode == GEM_MOVELINEY) || (Mode == GEM_SCALELINEX) || (Mode == GEM_SCALELINEY) || (Mode == GEM_MOVELINEX))
	{
		GUIRectangle screenrect = DrawRect;
		ClientToScreen(screenrect);

		Vector graphPt;
		ScreenToGraph (Vector ((float)pt.x, (float)(pt.y-(YOFFSET-3)), 0.0f), graphPt);


		for (int n = 0; n < Lines->GetCount(); n++)
		{
			GUIGraphLine* GraphLine = Lines->GetLine(n);
			GraphLine->LastMinimalPoint = -1;

		//	for (int i = 0; (i < GraphLine->GetCount()-1); i++)
			for (int i = 0; (i < GraphLine->GetCount()  ); i++)
			{
				Vector pt1 = GraphLine->GetPoint(i);

				if (pt1.x < graphPt.x)
				{
					GraphLine->LastMinimalPoint = i;
				}

				if (!NegativeValues)
					if (pt1.y < 0) pt1.y = 0;

				TransformPoint (pt1, pt1);
				if (pt1.x > screenrect.x + screenrect.w) break;
				if (pt1.x < screenrect.x + XOFFSET) continue;
				
				if ((pt.x > (pt1.x - PTHOTSPOT)) && (pt.x < (pt1.x + PTHOTSPOT)))
				{
					if ((pt.y > (pt1.y - PTHOTSPOT)) && (pt.y < (pt1.y + PTHOTSPOT)))
					{
						// Попали куда нужно...
						if (SelectedLine) SelectedLine->bActive = false;
						SelectedLine = GraphLine;
						SelectedLine->bActive = true;
						SelectedLine->SelectedPoint = i;
						Lines->SortByActive ();

						if (!NegativeValues)
						{
							Vector real_pt = GraphLine->GetPoint(i);
							if (real_pt.y < 0) real_pt.y = 0;
							GraphLine->Change(i, real_pt);
						}

						

						return;
					}
				}
			}
			

				
		}
		

		if ((SelectedLine) && (Mode == GEM_EDIT))
		{
			if (SelectedLine->LastMinimalPoint >= 0)
			{
				/*if (!NegativeValues)
					if (graphPt.y < 0.0f) graphPt.y = 0.0f;
				*/
				if (!CtrlState)
				{
					SelectedLine->Insert((SelectedLine->LastMinimalPoint+1), graphPt);
					SelectedLine->SelectedPoint = (SelectedLine->LastMinimalPoint+1);
				} else
						{
							SelectedLine->SelectedPoint = SelectedLine->LastMinimalPoint;
						}
			} else
				{
					SelectedLine->SelectedPoint = -1;
				}
		}

	} // Mode == GEM_EDIT
	
	
}


void GUIGraphEditor::MouseUp (int button, const GUIPoint& pt)
{
	if (button == GUIMSG_LMB_UP)	MouseButtonPressed = false;
	if (button == GUIMSG_RMB_UP)
	{
		RMouseButtonPressed = false;
		GetMainControl()->Application->GetCursor()->Pop();
	}
	
}

void GUIGraphEditor::MouseMove (int button, const GUIPoint& pt)
{
	if (MouseButtonPressed)
	{
		float dX = (float)(BegMousePressed.x - pt.x);
		float dY = (float)(BegMousePressed.y - pt.y);

		// Режим редактирования...
		if ((Mode == GEM_EDIT) && (SelectedLine))
		{
			if (SelectedLine->SelectedPoint == -1) return;

			dX *= UNITS_IN_ONE_PIXEL;
			dY *= UNITS_IN_ONE_PIXEL;
			dX *= ScaleX;
			dY *= ScaleY;

			Vector cpos = SelectedLine->GetPoint(SelectedLine->SelectedPoint);
			Vector savedpos = cpos;
			
		//	if (SelectedLine->SelectedPoint > 0)	cpos.x -= dX;
			if (SelectedLine->SelectedPoint > 0 && SelectedLine->SelectedPoint < SelectedLine->GetCount() - 1)
													cpos.x -= dX;
			cpos.y += dY;
			if (cpos.x < 0) cpos.x = 0;
			if (cpos.x > MAXXPOS) cpos.x = MAXXPOS;

			if( cpos.y < 0.0f )
				cpos.y = 0.0f;
			if( cpos.y > 1.0f )
				cpos.y = 1.0f;

			if (SelectedLine->SelectedPoint >= 1)
			{
				Vector minpos = SelectedLine->GetPoint((SelectedLine->SelectedPoint-1));
				if (cpos.x < (minpos.x + MIN_DISTANCE_BETWEEN_POINTS))
				{
					cpos.x = (minpos.x + MIN_DISTANCE_BETWEEN_POINTS);
				}
			}

			int LastPoint = (SelectedLine->GetCount()-1);
			if (SelectedLine->SelectedPoint < LastPoint)
			{
				Vector maxpos = SelectedLine->GetPoint((SelectedLine->SelectedPoint+1));
				if (cpos.x > (maxpos.x - MIN_DISTANCE_BETWEEN_POINTS))
				{
					cpos.x = (maxpos.x - MIN_DISTANCE_BETWEEN_POINTS);
				}
			}

/*
			if (!NegativeValues) 
				if (cpos.y < 0) cpos.y = 0;
*/
			SelectedLine->Change(SelectedLine->SelectedPoint, cpos);
		}

		if (Mode == GEM_SCALE)
		{
			float ScaleFactorX = (ScaleX / 450.0f);
			float ScaleFactorY = (ScaleY / 450.0f);
			float ScaleFactor = ScaleFactorX+ScaleFactorY;

			float Dist = dX + dY;
			//Dist *= 0.05f;
			ScaleX += (Dist * ScaleFactor);
			ScaleY += (Dist * ScaleFactor);

		}
		if (Mode == GEM_SCALEX)
		{
			float ScaleFactor = (ScaleX / 450.0f);

			float Dist = dX + dY;
			//Dist *= 0.05f;
			ScaleX += (Dist * ScaleFactor);
		}
		if (Mode == GEM_SCALEY)
		{
			float ScaleFactor = (ScaleY / 450.0f);

			float Dist = dX + dY;
			//Dist *= 0.05f;
			ScaleY += (Dist * ScaleFactor);
		}

		if ((Mode == GEM_MOVELINEY) && (SelectedLine))
		{
			for (int n = 0; n < SelectedLine->GetCount(); n++)
			{
				float UnitsInOnePixelY = UNITS_IN_ONE_PIXEL * ScaleY;
				Vector pt = SelectedLine->GetPoint(n);
				pt.y += (dY * UnitsInOnePixelY);
				SelectedLine->Change(n, pt);
			}
		}

		if ((Mode == GEM_MOVELINEX) && (SelectedLine))
		{
			for (int n = 1; n < (SelectedLine->GetCount()-1); n++)
			{
				float UnitsInOnePixelX = UNITS_IN_ONE_PIXEL * ScaleX;
				Vector pt = SelectedLine->GetPoint(n);
				pt.x -= (dX * UnitsInOnePixelX);

				Vector pt_prev = SelectedLine->GetPoint(n-1);
				if ((pt_prev.x + MIN_DISTANCE_BETWEEN_POINTS) > pt.x)
				{
					pt.x = (pt_prev.x + MIN_DISTANCE_BETWEEN_POINTS);
				}
				SelectedLine->Change(n, pt);
			}
		}

		if ((Mode == GEM_SCALELINEY) && (SelectedLine))
		{
			float MaxY = -99999999.0f;
			float MinY = 99999999.0f;
			for (int n = 0; n < SelectedLine->GetCount(); n++)
			{
				Vector pt = SelectedLine->GetPoint(n);
				if (pt.y < MinY) MinY = pt.y;
				if (pt.y > MaxY) MaxY = pt.y;
			}
			float MidY = MinY + ((MaxY - MinY) * 0.5f);

			for ( n = 0; n < SelectedLine->GetCount(); n++)
			{
				float UnitsInOnePixelY = UNITS_IN_ONE_PIXEL * ScaleY;
				Vector pt = SelectedLine->GetPoint(n);

				float ScaleK  = 0.98f;
				if (dX < 0) ScaleK = 1.02f;
				
				pt.y -= MidY;
				pt.y *= ScaleK;
				pt.y += MidY;
				SelectedLine->Change(n, pt);
			}
		}

		if ((Mode == GEM_SCALELINEX) && (SelectedLine))
		{
			/*
			float MaxX = -99999999.0f;
			float MinX = 99999999.0f;
			for (int n = 1; n < SelectedLine->GetCount()-1; n++)
			{
				Vector pt = SelectedLine->GetPoint(n);
				if (pt.x < MinX) MinX = pt.x;
				if (pt.x > MaxX) MaxX = pt.x;
			}
			float MidX = MinX + ((MaxX - MinX) * 0.5f);
*/
			for (int n = 1; n < SelectedLine->GetCount()-1; n++)
			{
				float UnitsInOnePixelX = UNITS_IN_ONE_PIXEL * ScaleX;
				Vector pt = SelectedLine->GetPoint(n);

				float ScaleK  = 0.98f;
				if (dX < 0) ScaleK = 1.02f;
				
				//pt.x -= MidX;
				pt.x *= ScaleK;
				//pt.x += MidX;

				Vector pt_prev = SelectedLine->GetPoint(n-1);
				if ((pt_prev.x + MIN_DISTANCE_BETWEEN_POINTS) > pt.x)
				{
					pt.x = (pt_prev.x + MIN_DISTANCE_BETWEEN_POINTS);
				}

				SelectedLine->Change(n, pt);
			}
		}



		BegMousePressed = pt;
	}

	if (RMouseButtonPressed)
	{
		float dX = (float)(RBegMousePressed.x - pt.x);
		float dY = (float)(RBegMousePressed.y - pt.y);
		RBegMousePressed = pt;
		OffsetX -= dX;
		OffsetY -= dY;

		if (OffsetX > 32) OffsetX = 32;
	}

	
}

void GUIGraphEditor::ScreenToGraph (const Vector& fromr, Vector& to)
{
	Vector from = fromr;
	GUIRectangle screenrect = DrawRect;
	ClientToScreen(screenrect);

	float realX = from.x - screenrect.x;
	float realY = from.y - screenrect.y;

	realY = screenrect.h - realY;


	realX -= XOFFSET;
	realY -= YOFFSET;

	float UnitsInOnePixelX = UNITS_IN_ONE_PIXEL * ScaleX;
	float UnitsInOnePixelY = UNITS_IN_ONE_PIXEL * ScaleY;

	realX -= OffsetX;
	realY += OffsetY;


	realX *= UnitsInOnePixelX;
	realY *= UnitsInOnePixelY;


	


	to.x = realX;
	to.y = realY;
	to.z = 0.0f;
	
}


void GUIGraphEditor::FitViewWidth (float Min, float Max)
{
	float Interval = fabsf(Max - Min);
	GUIRectangle d_rect = GetDrawRect();
	float UnitsInGraph = ((d_rect.w-(XOFFSET*2)) * UNITS_IN_ONE_PIXEL);
	ScaleX = Interval / UnitsInGraph;
	ClampScale ();
	float UnitsInOnePixelX = UNITS_IN_ONE_PIXEL * ScaleX;

	OffsetX = Min / UnitsInOnePixelX;
}

void GUIGraphEditor::FitViewHeight (float Min, float Max)
{
	if (Min < 0.0f && Max < 0.0f) Max = 0.0f;
	if (Min > 0.0f && Max > 0.0f) Min = 0.0f;
	float Interval = fabsf(Max - Min);
	GUIRectangle d_rect = GetDrawRect();
	float UnitsInGraph = ((d_rect.h-(YOFFSET*2)) * UNITS_IN_ONE_PIXEL);
	ScaleY = Interval / UnitsInGraph;
	ClampScale ();
	float UnitsInOnePixelY = UNITS_IN_ONE_PIXEL * ScaleY;

	OffsetY = (Min-(UnitsInOnePixelY*10)) / UnitsInOnePixelY;
		//(UnitsInOnePixelY/ 10.0f );
}


float GUIGraphEditor::GetMaxValueY ()
{
	float MaxY = -9999999999.0f;
	for (int n = 0; n < Lines->GetCount(); n++)
	{
		GUIGraphLine* GraphLine = Lines->GetLine(n);
		for (int i = 0; (i < GraphLine->GetCount()-1); i++)
		{
			Vector pt1 = GraphLine->GetPoint(i);
			if (pt1.y > MaxY) MaxY = pt1.y;
		}
	}

	return MaxY;
}

float GUIGraphEditor::GetMinValueY ()
{
	float MinY = 9999999999.0f;
	for (int n = 0; n < Lines->GetCount(); n++)
	{
		GUIGraphLine* GraphLine = Lines->GetLine(n);
		for (int i = 0; (i < GraphLine->GetCount()-1); i++)
		{
			Vector pt1 = GraphLine->GetPoint(i);
			if (pt1.y < MinY) MinY = pt1.y;
		}
	}

	return MinY;

}

void GUIGraphEditor::ClampScale ()
{
	if (ScaleX < 0.009f) ScaleX = 0.009f;
	if (ScaleY < 0.009f) ScaleY = 0.009f;
	if (ScaleX > 1345.0f) ScaleX = 1345.0f;
	if (ScaleY > 1345.0f) ScaleY = 1345.0f;
}


#endif