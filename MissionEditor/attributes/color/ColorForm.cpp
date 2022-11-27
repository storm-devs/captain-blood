#include "ColorForm.h"
#include "ColorAttr.h"



TColorEdit::TColorEdit (int posX, int posY) : GUIColorPicker (posX, posY, COLOREDIT_WINDOWNAME, 0, 0)
{
	
	//bPopupStyle = true;
	bAlwaysOnTop = true;
	bSystemButton = false;
	
  OKButton->Caption = "Accept";
	OKButton->pFont->SetName ("arialcyrsmall");
	OKButton->Glyph->Load ("meditor\\ok");
	OKButton->FlatButton = true;

	CancelButton->Caption = "Cancel";
	CancelButton->pFont->SetName ("arialcyrsmall");
	CancelButton->Glyph->Load ("meditor\\cancel");
	CancelButton->FlatButton = true;


	editR->Flat = true;
	editG->Flat = true;
	editB->Flat = true;
	editA->Flat = true;
	HexValue->Flat = true;
	/*
	lScale =  NEW GUILabel (this, 440, 226, 100, 20);;
	lScale->Caption = "Bright";
	lScale->Layout = GUILABELLAYOUT_Left;
	edtScale = NEW GUIEdit (this, 440, 245, 85, 22);
	edtScale->Text = "1.0";
	*/
	//edtScale->pFont->SetSize (14);


	SetScreenCenter();

}

TColorEdit::~TColorEdit ()
{
}


void _cdecl TColorEdit::OKClick (GUIControl* sender)
{
	DWORD ourColor = CalcCurrentColor ();
	Color nColor = Color (ourColor);
	if (MasterAttrib->GetIsLimit ())
	{
		nColor.Min (MasterAttrib->GetMax());
		nColor.Max (MasterAttrib->GetMin());
	}



//	float eScale = 1.0f;
//	eScale = (float)atof (edtScale->Text.GetBuffer());

//	nColor.r *= eScale;
//	nColor.g *= eScale;
//	nColor.b *= eScale;
//	nColor.a *= eScale;

	MasterAttrib->SetValue (nColor);


	

	GUIColorPicker::OKClick (sender);
}



bool TColorEdit::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{/*
	if (message == GUIMSG_KEYPRESSED)
	{
		int key = (int)lparam;
		if (key == 13)
		{
			OKClick (this);
			return true;
		}
	}*/
	return GUIColorPicker::ProcessMessages (message, lparam, hparam);
}
