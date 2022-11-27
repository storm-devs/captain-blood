#ifndef _XBOX
#include "gui_colorpicker.h"
#include "gui_helper.h"

	
	


GUIColorPicker::GUIColorPicker (int X, int Y, const char* caption, int AddWidth, int AddHeight) : GUIWindow (NULL, X, Y, (535+AddWidth), (310+AddHeight))
{
	t_OnApply = NEW GUIEventHandler;
	t_OnCancel = NEW GUIEventHandler;
	t_OnChange = NEW GUIEventHandler;
	

	Caption = caption;

  OKButton = NEW GUIButton (this, 440, 10, 80, 24);
	OKButton->Caption = "OK";
	OKButton->OnMousePressed = (CONTROL_EVENT)&GUIColorPicker::OKClick;
	

	CancelButton = NEW GUIButton (this, 440, 40, 80, 24);
	CancelButton->Caption = "Cancel";
	CancelButton->OnMousePressed = (CONTROL_EVENT)&GUIColorPicker::CloseClick;

	Target = NEW GUIImage;
	Target->Load ("target");


//	Colorizer = NEW GUIImage;
//	Colorizer->Load ("colors");

	Arrows = NEW GUIImage;
	Arrows->Load ("arrows");

	posX = 258.0f;
	posY = 0.0f;

	MouseP = false;
	MouseS = false;
	MouseA = false;

	MainColor = 0xFFFF0000;

	MainColorPosition = 0;
	AlphaPosition = 0;

	SelectedColor = 0xFFFF0000;


	HexValue = NEW GUIEdit (this, 440, 205, 85, 22);
  HexValue->pFont->SetName("arialcyrsmall");
	HexValue->OnAccept = (CONTROL_EVENT)&GUIColorPicker::HexTextIsChanged;


	editR = NEW GUIEdit (this, 440, 70, 45, 22);
  editR->pFont->SetName("arialcyrsmall");
	editR->OnAccept = (CONTROL_EVENT)&GUIColorPicker::TextIsChanged;


	editG = NEW GUIEdit (this, 440, 100, 45, 22);
  editG->pFont->SetName("arialcyrsmall");
	editG->OnAccept = (CONTROL_EVENT)&GUIColorPicker::TextIsChanged;

	editB = NEW GUIEdit (this, 440, 130, 45, 22);
  editB->pFont->SetName("arialcyrsmall");
	editB->OnAccept = (CONTROL_EVENT)&GUIColorPicker::TextIsChanged;

	editA = NEW GUIEdit (this, 440, 160, 45, 22);
  editA->pFont->SetName("arialcyrsmall");
	editA->OnAccept = (CONTROL_EVENT)&GUIColorPicker::TextIsChanged;

	HDRPower = NEW GUIEdit (this, 440, 245, 45, 22);
	HDRPower->pFont->SetName("arialcyrsmall");
	HDRPower->Visible = false;
	//HDRPower->OnAccept = (CONTROL_EVENT)TextIsChanged;

	labelHDR = NEW GUILabel (this, 480, 245, 45, 22);
	labelHDR->Caption = "HDR";
	labelHDR->Visible = false;

	



	labelR = NEW GUILabel (this, 475, 70, 45, 22);
	labelG = NEW GUILabel (this, 475, 100, 45, 22);
	labelB = NEW GUILabel (this, 475, 130, 45, 22);
	labelA = NEW GUILabel (this, 475, 160, 45, 22);
	labelHex = NEW GUILabel (this, 435, 185, 45, 22);

	labelR->Caption = "R";
	labelG->Caption = "G";
	labelB->Caption = "B";
	labelA->Caption = "A";
	labelHex->Caption = "Hex";

	UpdateTextValues ();


}

GUIColorPicker::~GUIColorPicker ()
{

	delete t_OnApply;
	delete t_OnCancel;
	delete t_OnChange;

	delete Target;
  delete Arrows;
  
	delete OKButton;
	delete CancelButton;


	delete labelR;
	delete labelG;
	delete labelB;
	delete labelA;
	delete labelHex;

	delete editR;
	delete editG;
	delete editB;
	delete editA;
	delete HexValue;

  

}

//----------------------------------------------------------------	

void _cdecl GUIColorPicker::OKClick (GUIControl* sender)
{
  SelectedColor = CalcCurrentColor ();
	t_OnApply->Execute(this);
	Close (this);
}

//----------------------------------------------------------------

void _cdecl GUIColorPicker::CloseClick (GUIControl* sender)
{
	//UpdateTextValues ();
	t_OnCancel->Execute(this);
	Close (this);
}

//----------------------------------------------------------------

void GUIColorPicker::Draw ()
{
	if (Visible == false) return;
CalcMainColor ();

GUIWindow::Draw ();

GUIRectangle r = GetClientRect ();
ClientToScreen (r);

GUIHelper::DrawDownBorder (r.Left+8, r.Top+8,260+2, 260+2);

GUIHelper::DrawColorRect ( r.Left+10, r.Top+10, 258, 258, MainColor, 0xFFFFFFFF, 
													                 0xFF000000, 0xFF000000);

cliper.Push ();

GUIRectangle clip_rect;
clip_rect.Left = r.Left+10;
clip_rect.Top = r.Top+10;
clip_rect.Width = 258;
clip_rect.Height = 258;

cliper.SetRectangle (clip_rect);

int halfX = Target->GetWidth() / 2;
int halfY = Target->GetHeight() / 2;

GUIHelper::DrawSprite ((int)(r.Left+10+posX-halfX), (int)(r.Top+10+posY-halfY), Target->GetWidth(), Target->GetHeight(), Target);


cliper.Pop ();

DWORD clr = CalcCurrentColor ();

//HexValue->Text.Format ("0x%08X", clr);

GUIHelper::DrawDownBorder (r.Left+318+40, r.Top+8,74, 84);
GUIHelper::Draw2DRect (r.Left+320+40, r.Top+10, 70, 80, clr);

BYTE a = (BYTE)(clr >> 24);
DWORD a_color = ((a << 24) | (a << 16) | (a << 8) | (a));
GUIHelper::DrawDownBorder (r.Left+318+40, r.Top+8+88,74, 84);
GUIHelper::Draw2DRect (r.Left+320+40, r.Top+10+88, 70, 80, a_color);

GUIHelper::DrawDownBorder (r.Left+318+40, r.Top+8+(88*2),74, 84);
GUIHelper::Draw2DRectAlpha (r.Left+320+40, r.Top+10+(88*2), 70, 80, clr);


GUIHelper::DrawDownBorder (r.Left+282, r.Top+9-2, 16+4, 258+4);
GUIHelper::DrawColorPickerRect (r.Left+284, r.Top+9, 16, 258);
GUIHelper::DrawSprite (r.Left+270+6, (int)(r.Top+9+MainColorPosition-7), Arrows->GetWidth(), Arrows->GetHeight(), Arrows);

//-------------------
GUIHelper::DrawDownBorder (r.Left+282+40, r.Top+9-2, 16+4, 258+4);
GUIHelper::DrawAlphaRect (r.Left+284+40, r.Top+9, 16, 258);
GUIHelper::DrawSprite (r.Left+270+6+40, r.Top+9+AlphaPosition-7, Arrows->GetWidth(), Arrows->GetHeight(), Arrows);
//-------------------





}


void GUIColorPicker::OnMDown (int MouseKey, const GUIPoint& pt)
{
	
	if (MouseP)
	{
		GUIRectangle r = GetClientRect ();
		ClientToScreen (r);

		GUIRectangle cRect;
		cRect.Left = r.Left+10;
		cRect.Top = r.Top+10;
		cRect.Width = 258;
		cRect.Height = 258;

		posX = (float)(pt.X - cRect.Left);
		posY = (float)(pt.Y - cRect.Top);

		if (posX < 0) posX = 0.0f;
		if (posY < 0) posY = 0.0f;
		if (posX > cRect.Width) posX = (float)cRect.Width;
		if (posY > cRect.Height) posY = (float)cRect.Height;

		UpdateTextValues ();

	}


	if (MouseS)
	{
		GUIRectangle r = GetClientRect ();
		ClientToScreen (r);

		GUIRectangle sRect = r;
		sRect.Left+=284;
		sRect.Top+=9;
		sRect.Width = 16;
		sRect.Height = 258;
		MainColorPosition = (float)(pt.Y - sRect.Top);
		if (MainColorPosition < 0) MainColorPosition = 0.0f;
		if (MainColorPosition > sRect.Height) MainColorPosition = (float)sRect.Height;

		UpdateTextValues ();

	}

	if (MouseA)
	{
		GUIRectangle r = GetClientRect ();
		ClientToScreen (r);

		GUIRectangle sRect = r;
		sRect.Left+=284+40;
		sRect.Top+=9;
		sRect.Width = 16;
		sRect.Height = 258;
		AlphaPosition = pt.Y - sRect.Top;
		if (AlphaPosition < 0) AlphaPosition = 0;
		if (AlphaPosition > sRect.Height) AlphaPosition = sRect.Height;

		UpdateTextValues ();

	}

}


void GUIColorPicker::MouseUp (int button, const GUIPoint& pt)
{
	GUIWindow::MouseUp (button, pt);
	MouseP = false;
	MouseS = false;
	MouseA = false;
}

bool GUIColorPicker::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
 	if (Visible == false) return false;
  bool Handled = false;
	
	bool Handled2 = GUIWindow::ProcessMessages (message, lparam, hparam);
	


	
	
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);
	
		
	
	if (message == GUIMSG_LMB_DOWN) 
	{
		GUIWindow::OnMDown (message, cursor_pos);
		GUIRectangle r = GetClientRect ();
		ClientToScreen (r);

//		r.Left+284, r.Top+9, 16, 258
		GUIRectangle sRect = r;
		sRect.Left+=284;
		sRect.Top+=9;
		sRect.Width = 16;
		sRect.Height = 258;
		if (GUIHelper::PointInRect (cursor_pos, sRect)) MouseS = true;

		sRect = r;
		sRect.Left+=284+40;
		sRect.Top+=9;
		sRect.Width = 16;
		sRect.Height = 258;
		if (GUIHelper::PointInRect (cursor_pos, sRect)) MouseA = true;



		GUIRectangle cRect = r;
		cRect.Left += 10;
		cRect.Top += 10;
		cRect.Width = 258;
		cRect.Height = 258;




		if (GUIHelper::PointInRect (cursor_pos, cRect))	MouseP = true;
		Handled = true;
		OnMDown (message, cursor_pos);
	}
	
	if ((message == GUIMSG_MOUSEMOVE)  && (MouseP))
	{
		Handled = true;
		OnMDown (message, cursor_pos);
	}

	
	if (Handled2) return Handled2;
	return Handled;
}


DWORD GUIColorPicker::CalcCurrentColor ()
{
  Color rColor1 = Color(MainColor);
  Color rColor2 = Color((DWORD)0xFFFFFFFF);
  Color rColor3 = Color((DWORD)0xFF000000);
  Color rColor4 = Color((DWORD)0xFF000000);

	Color xCol1;
	xCol1.Lerp (rColor1, rColor3, ((float)posY / 258.0f));

	Color xCol2;
	xCol2.Lerp (rColor2, rColor4, ((float)posY / 258.0f));

	Color reColor1;
	reColor1.Lerp (xCol2, xCol1, ((float)posX / 258.0f));

	Color aColor1 = Color((DWORD)0xFFFFFFFF);
	Color aColor2 = Color((DWORD)0x00000000);
	Color aCol1;
	aCol1.Lerp (aColor1, aColor2, ((float)AlphaPosition / 258.0f));



	reColor1.a = aCol1.r;

	return reColor1.GetDword ();
}


void GUIColorPicker::MouseMove (int button, const GUIPoint& pt)
{
	GUIWindow::MouseMove (button, pt);
	OnMDown (button, pt);
}

void GUIColorPicker::CalcMainColor ()
{
	
//	MainColorPosition++;
//	if (MainColorPosition > 258) MainColorPosition = 0;

	MainColor = GUIHelper::GetColorPickerColor (258, MainColorPosition);
}

void GUIColorPicker::UpdateTextValues ()
{
		DWORD clr = CalcCurrentColor ();
		HexValue->Text.Format ("0x%08X", clr);

		BYTE a = (BYTE)((clr >> 24) & 0xFF);
		editA->Text.Format ("%d", a);

		BYTE r = (BYTE)((clr >> 16) & 0xFF);
		editR->Text.Format ("%d", r);

		BYTE g = (BYTE)((clr >> 8) & 0xFF);
		editG->Text.Format ("%d", g);

		BYTE b = (BYTE)((clr) & 0xFF);
		editB->Text.Format ("%d", b);

		SelectedColor = CalcCurrentColor ();
		t_OnChange->Execute(this);

}


void _cdecl GUIColorPicker::HexTextIsChanged (GUIControl* sender)
{
  DWORD dwColor = 0;
	crt_sscanf (HexValue->Text.GetBuffer (), "0x%08X", &dwColor);

	BYTE tA = (BYTE)((dwColor >> 24) & 0xFF);
	BYTE tR = (BYTE)((dwColor >> 16) & 0xFF);
	BYTE tG = (BYTE)((dwColor >> 8) & 0xFF);
	BYTE tB = (BYTE)(dwColor & 0xFF);

	ProcessTextIsChanged (tR, tG, tB, tA);

//	UpdateTextValues ();
	editA->Text.Format ("%d", tA);
	editR->Text.Format ("%d", tR);
	editG->Text.Format ("%d", tG);
	editB->Text.Format ("%d", tB);

	t_OnChange->Execute(this);


}


void _cdecl GUIColorPicker::TextIsChanged (GUIControl* sender)
{
	int R = atoi (editR->Text.GetBuffer ());
	int G = atoi (editG->Text.GetBuffer ());
	int B = atoi (editB->Text.GetBuffer ());
	int A = atoi (editA->Text.GetBuffer ());

	ProcessTextIsChanged (R, G, B, A);

	DWORD clr = ((A << 24) | (R << 16) | (G << 8) | B);
	HexValue->Text.Format ("0x%08X", clr);

	t_OnChange->Execute(this);


//	UpdateTextValues ();
}

void GUIColorPicker::ProcessTextIsChanged (BYTE _R, BYTE _G, BYTE _B, BYTE _A)
{
//-------------- A

	float k = (float)_A / 255.0f;
	k = 1.0f - k;
	AlphaPosition = (int)(k * 258.0f);

//-------------- RGB

	// Если все цвета равны, то это оттенок серого (вариант 1)
	if ((_R == _B) && (_B == _G))
	{
		posX = 0.0f;

		float k = (float)_R / 255.0f;
		k = 1.0f - k;
		posY = k * 258.0f;
		return;
	}

	//Если два компонента равны, чистый цвет (вариант 2) 
	if ((_R == _G) || (_R == _B) || (_G == _B))
	{
		//R - главный цвет
		if ((_R > _B) && (_R > _G))
		{
			MainColorPosition = 0;
			posX = 258;
			float k = (float)_R / 255.0f;
			k = 1.0f - k;
			posY = k * 258.0f;

			k = (float)_G / (float)_R;
			k = 1.0f - k;
			posX = k * 258.0f;

			return;
		}
		//B - главный цвет (2)
		if ((_B > _R) && (_B > _G))
		{
			MainColorPosition = 43*2;
			posX = 258;
			float k = (float)_B / 255.0f;
			k = 1.0f - k;
			posY = k * 258.0f;

			k = (float)_G / (float)_B;
			k = 1.0f - k;
			posX = k * 258.0f;

			return;
		}
		//G - главный цвет (4)
		if ((_G > _R) && (_G > _B))
		{
			MainColorPosition = 43*4;
			posX = 258;
			float k = (float)_G / 255.0f;
			k = 1.0f - k;
			posY = k * 258.0f;

			k = (float)_R / (float)_G;
			k = 1.0f - k;
			posX = k * 258.0f;

			return;
		}

		// R && G - главный цвет (5)
		if (_R == _G)
		{
			MainColorPosition = 43*5;
			posX = 258;
			float k = (float)_G / 255.0f;
			k = 1.0f - k;
			posY = k * 258.0f;

			k = (float)_B / (float)_G;
			k = 1.0f - k;
			posX = k * 258.0f;

			return;
		}

		// R && B - главный цвет (1)
		if (_R == _B)
		{
			MainColorPosition = 43*1;
			posX = 258;
			float k = (float)_R / 255.0f;
			k = 1.0f - k;
			posY = k * 258.0f;

			k = (float)_G / (float)_R;
			k = 1.0f - k;
			posX = k * 258.0f;

			return;
		}
		
		// G && B - главный цвет (3)
		if (_G == _B)
		{
			MainColorPosition = 43*3;
			posX = 258;
			float k = (float)_G / 255.0f;
			k = 1.0f - k;
			posY = k * 258.0f;

			k = (float)_R / (float)_B;
			k = 1.0f - k;
			posX = k * 258.0f;

			return;
		}

		return;
	}


//Если все компоненты разные, надо найти два наиболее
//сильных цвета, выбрать их на колорайзере и потом подобрать
// з-й цвет...

// R + G (B - small)
if ((_B < _R) && (_B < _G))
{
	// Среди двух оставшися _R имеет приоритет
	if (_R > _G)
	{
		CalculateColorPositions (_R, _G, _B, 6, true);
		return;
	} else
		{
		// Среди двух оставшися _G имеет приоритет
		  CalculateColorPositions (_G, _R, _B, 4);
			return;
		}
}

// B + G (R - small)
if ((_R < _B) && (_R < _G))
{
	// Среди двух оставшися _B имеет приоритет
	if (_B > _G)
	{
		CalculateColorPositions (_B, _G, _R, 2);
		return;
	} else
		{
		// Среди двух оставшися _G имеет приоритет
		  CalculateColorPositions (_G, _B, _R, 4, true);
			return;
		}
}

// R + B (G - small)
if ((_G < _B) && (_G < _R))
{
	// Среди двух оставшися _R имеет приоритет
	if (_R > _B)
	{
		CalculateColorPositions (_R, _B, _G, 0);
		return;
	} else
		{
		// Среди двух оставшися _B имеет приоритет
		  CalculateColorPositions (_B, _R, _G, 2, true);
			return;
		}
}




}


void GUIColorPicker::CalculateColorPositions (BYTE c1, BYTE c2, BYTE c3, int ColorSegment, bool StepUp)
{
	float multipler = 1.0f;
  if (StepUp) multipler = -1.0f;

	float C1 = (float)c1 / 255.0f;
	float C2 = (float)c2 / 255.0f;
	float C3 = (float)c3 / 255.0f;
	Assert (C1 >= C2);
	Assert (C2 >= C3);

	float KZ = ((C2 - C3) / (C1 - C3));
	float KX = (C3 / C1);
	float KY = C1;

	KZ = (KZ * multipler);
	MainColorPosition = (43.0f*((float)ColorSegment+KZ));
	posX = (1.0f - KX) * 258.0f;
	posY = (1.0f - KY) * 258.0f;
}

#endif