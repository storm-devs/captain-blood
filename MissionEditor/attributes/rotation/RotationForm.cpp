
#include "rotationform.h"
#include "rotationattr.h"
#include "..\strutil.h"
#include "..\..\missioneditor.h"
#include "..\..\fast_atof.h"

#define FREE_TIME 0.2f

#define WINDOW_WIDTH  178
#define WINDOW_HEIGHT 65

extern IRender* pRS;


extern BaseAttribute* pEditableNode;


void TRotationEdit::OnCreate()
{
}


TRotationEdit::TRotationEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bAlwaysOnTop = true;
	bPopupStyle = true;
	bSystemButton = false;

  Caption = ROTATIONEDIT_WINDOWNAME;


	btnCamera = NEW GUIButton (this, 150, 3, 24, 24);
	btnCamera->Glyph->Load ("meditor\\camcenter");
	btnCamera->OnMouseDown = (CONTROL_EVENT)&TRotationEdit::OnButtonPressed;
	btnCamera->FlatButton = true;

  eValueX = NEW GUIEdit (this, 6, 5, 45, 19);
	eValueX->OnAccept = (CONTROL_EVENT)&TRotationEdit::OnDataChange;
	eValueX->Flat = true;
	eValueX->pFont->SetName("arialcyrsmall");
  
	eValueY = NEW GUIEdit (this, 54, 5, 45, 19);
	eValueY->OnAccept = (CONTROL_EVENT)&TRotationEdit::OnDataChange;
	eValueY->Flat = true;
	eValueY->pFont->SetName("arialcyrsmall");
  
	eValueZ = NEW GUIEdit (this, 102, 5, 45, 19);
	eValueZ->OnAccept = (CONTROL_EVENT)&TRotationEdit::OnDataChange;
	eValueZ->Flat = true;
	eValueZ->pFont->SetName("arialcyrsmall");

/*
	lDescription = NEW GUILabel (this, 2, 0, 230, 14);
	lDescription->Caption = "long edit";
	lDescription->Layout = GUILABELLAYOUT_Left;
	lDescription->pFont->SetSize (14);
*/
/*	
	lMin = NEW GUILabel (this, 6, 45, 70, 19);
	lMin->Layout = GUILABELLAYOUT_Left;
	lMin->pFont->SetSize (12);

	lMax = NEW GUILabel (this, 6, 56, 70, 19);
	lMax->Layout = GUILABELLAYOUT_Left;
	lMax->pFont->SetSize (12);
*/

	btnOK = NEW GUIButton (this, 9,30, 78, 24);
	btnOK->Glyph->Load ("meditor\\ok");
	btnOK->Caption = "Accept";
	btnOK->pFont->SetName("arialcyrsmall");
	btnOK->FlatButton = true;

	btnCancel = NEW GUIButton (this, 95,30, 78, 24);
	btnCancel->Glyph->Load ("meditor\\cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->pFont->SetName("arialcyrsmall");
	btnCancel->FlatButton = true;

	btnOK->OnMousePressed = (CONTROL_EVENT)&TRotationEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TRotationEdit::OnButtonCancel;

//  cbValue->OnChange = 
}

TRotationEdit::~TRotationEdit ()
{
/*
	delete btnOK;
	delete btnCancel;
*/

	delete btnCamera;

  delete eValueX;
	delete eValueY;
	delete eValueZ;

/*
	delete lMin;
	delete lMax;
*/
}


void _cdecl TRotationEdit::OnDataChange (GUIControl* sender)
{
	if (MasterAttrib->GetIsLimit ())
	{
		float needValueX = fast_atof (eValueX->Text.GetBuffer ());
		float needValueY = fast_atof (eValueY->Text.GetBuffer ());
		float needValueZ = fast_atof (eValueZ->Text.GetBuffer ());

		needValueX = Deg2Rad (needValueX);
		needValueY = Deg2Rad (needValueY);
		needValueZ = Deg2Rad (needValueZ);

		if (needValueX < MasterAttrib->GetMin ().x)  needValueX = MasterAttrib->GetMin ().x;
		if (needValueX > MasterAttrib->GetMax ().x)  needValueX = MasterAttrib->GetMax ().x;

		if (needValueY < MasterAttrib->GetMin ().y)  needValueY = MasterAttrib->GetMin ().y;
		if (needValueY > MasterAttrib->GetMax ().y)  needValueY = MasterAttrib->GetMax ().y;
		
		if (needValueZ < MasterAttrib->GetMin ().z)  needValueZ = MasterAttrib->GetMin ().z;
		if (needValueZ > MasterAttrib->GetMax ().z)  needValueZ = MasterAttrib->GetMax ().z;

		eValueX->Text = FloatToStr (Rad2Deg(needValueX));
		eValueY->Text = FloatToStr (Rad2Deg(needValueY));
		eValueZ->Text = FloatToStr (Rad2Deg(needValueZ));
	}}



void _cdecl TRotationEdit::OnButtonPressed (GUIControl* sender)
{
	Matrix mView = pRS->GetView ();
	mView.Inverse ();


	Vector ang(0.0f);
	Vector xz = Vector(mView.vz.x, 0.0f, mView.vz.z);
	ang.y = xz.GetAY();
	Vector yxz = Vector(mView.vz.y, 0.0f, xz.GetLengthXZ());
	ang.x = -yxz.GetAY();

  
	ang.x = Rad2Deg(ang.x);
	ang.y = Rad2Deg(ang.y);
	ang.z = Rad2Deg(ang.z);

// clamp angles
	int cX = (int)ang.x / 360;
	int cY = (int)ang.y / 360;
	int cZ = (int)ang.z / 360;
	float ostX = (ang.x  - (cX * 360));
	float ostY = (ang.y  - (cY * 360));
	float ostZ = (ang.z  - (cZ * 360));
	if (ostX < 0) ostX = 360.0f + ostX;
	if (ostY < 0) ostY = 360.0f + ostY;
	if (ostZ < 0) ostZ = 360.0f + ostZ;
// clamp angles

	eValueX->Text = FloatToStr (ostX);
	eValueY->Text = FloatToStr (ostY);
	eValueZ->Text = FloatToStr (ostZ);

	OnDataChange (NULL);


}

void _cdecl TRotationEdit::OnButtonOK (GUIControl* sender)
{
	OnDataChange (this);
	float needValueX = fast_atof (eValueX->Text.GetBuffer ());
	float needValueY = fast_atof (eValueY->Text.GetBuffer ());
	float needValueZ = fast_atof (eValueZ->Text.GetBuffer ());

	needValueX = Deg2Rad (needValueX);
	needValueY = Deg2Rad (needValueY);
	needValueZ = Deg2Rad (needValueZ);


	Vector needVal = Vector (needValueX, needValueY, needValueZ);
	MasterAttrib->SetValue (needVal);

	Close (this);
}

void _cdecl TRotationEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}

void TRotationEdit::Draw ()
{
	GUIWindow::Draw ();
}



void TRotationEdit::SetVector (Vector v)
{
	eValueX->Text = FloatToStr (v.x);
	eValueY->Text = FloatToStr (v.y);
	eValueZ->Text = FloatToStr (v.z);
	OnDataChange (NULL);
}




void TRotationEdit::KeyPressed(int Key, bool bSysKey)
{
  if (!bSysKey) return;

	if (Key == 13)
	{
		OnButtonOK (this);
	}
}

