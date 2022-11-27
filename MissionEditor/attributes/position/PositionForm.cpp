
#include "positionform.h"
#include "PositionAttr.h"
#include "..\strutil.h"
#include "..\..\MissionEditor.h"
#include "..\..\fast_atof.h"


#define FREE_TIME 0.2f
#define WINDOW_WIDTH  178
#define WINDOW_HEIGHT 65

extern IRender* pRS;
extern BaseAttribute* pEditableNode;


void TPositionEdit::OnCreate()
{
	SelectIndex (cIndex);
}

void TPositionEdit::SelectIndex (int i)
{
	if (i == 0)
	{
		eValueX->SelectText(0, eValueX->Text.Size());
		eValueX->SetFocus();
	}
	if (i == 1)
	{
		eValueY->SelectText(0, eValueY->Text.Size());
		eValueY->SetFocus();
	}
	if (i == 2)
	{
		eValueZ->SelectText(0, eValueZ->Text.Size());
		eValueZ->SetFocus();
	}

}

TPositionEdit::TPositionEdit (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	lTimePressed = 100.0f;
	cIndex = 0;
	bAlwaysOnTop = true;
	bPopupStyle = true;
	bSystemButton = false;


  Caption = POSITIONEDIT_WINDOWNAME;

	btnCamera = NEW GUIButton (this, 150, 3, 24, 24);
	btnCamera->Glyph->Load ("meditor\\camcenter");
	btnCamera->OnMouseDown = (CONTROL_EVENT)&TPositionEdit::OnButtonPressed;
	btnCamera->FlatButton = true;

  eValueX = NEW GUIEdit (this, 6, 5, 45, 19);
	eValueX->OnAccept = (CONTROL_EVENT)&TPositionEdit::OnDataChange;
	eValueX->Flat = true;
	eValueX->pFont->SetName("arialcyrsmall");
  
	eValueY = NEW GUIEdit (this, 54, 5, 45, 19);
	eValueY->OnAccept = (CONTROL_EVENT)&TPositionEdit::OnDataChange;
	eValueY->Flat = true;
	eValueY->pFont->SetName("arialcyrsmall");
  
	eValueZ = NEW GUIEdit (this, 102, 5, 45, 19);
	eValueZ->OnAccept = (CONTROL_EVENT)&TPositionEdit::OnDataChange;
	eValueZ->Flat = true;
	eValueZ->pFont->SetName("arialcyrsmall");


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
	btnOK->pFont->SetSize (12);
	btnOK->FlatButton = true;

	btnCancel = NEW GUIButton (this, 95,30, 78, 24);
	btnCancel->Glyph->Load ("meditor\\cancel");
	btnCancel->Caption = "Cancel";
	btnCancel->pFont->SetSize (12);
	btnCancel->FlatButton = true;

	btnOK->OnMousePressed = (CONTROL_EVENT)&TPositionEdit::OnButtonOK;
	btnCancel->OnMousePressed = (CONTROL_EVENT)&TPositionEdit::OnButtonCancel;

//  cbValue->OnChange = 
}

TPositionEdit::~TPositionEdit ()
{
	delete btnOK;
	delete btnCancel;


	delete btnCamera;

  delete eValueX;
	delete eValueY;
	delete eValueZ;


//	delete lMin;
//	delete lMax;

}

void TPositionEdit::Draw ()
{
	lTimePressed += api->GetDeltaTime();
	GUIWindow::Draw ();
}



void _cdecl TPositionEdit::OnButtonPressed (GUIControl* sender)
{
	Matrix mView = pRS->GetView ();
	Vector cam_pos = mView.GetCamPos ();

	eValueX->Text = FloatToStr (cam_pos.x);
	eValueY->Text = FloatToStr (cam_pos.y);
	eValueZ->Text = FloatToStr (cam_pos.z);

	OnDataChange (NULL);
}

void _cdecl TPositionEdit::OnDataChange (GUIControl* sender)
{
	if (MasterAttrib->GetIsLimit ())
	{
		float needValueX = fast_atof(eValueX->Text.GetBuffer ());
		float needValueY = fast_atof(eValueY->Text.GetBuffer ());
		float needValueZ = fast_atof(eValueZ->Text.GetBuffer ());

		if (needValueX < MasterAttrib->GetMin ().x)  needValueX = MasterAttrib->GetMin ().x;
		if (needValueX > MasterAttrib->GetMax ().x)  needValueX = MasterAttrib->GetMax ().x;

		if (needValueY < MasterAttrib->GetMin ().y)  needValueY = MasterAttrib->GetMin ().y;
		if (needValueY > MasterAttrib->GetMax ().y)  needValueY = MasterAttrib->GetMax ().y;
		
		if (needValueZ < MasterAttrib->GetMin ().z)  needValueZ = MasterAttrib->GetMin ().z;
		if (needValueZ > MasterAttrib->GetMax ().z)  needValueZ = MasterAttrib->GetMax ().z;

		eValueX->Text = FloatToStr (needValueX);
		eValueY->Text = FloatToStr (needValueY);
		eValueZ->Text = FloatToStr (needValueZ);
	}
}


void _cdecl TPositionEdit::OnButtonOK (GUIControl* sender)
{
	OnDataChange (this);
	float needValueX = fast_atof (eValueX->Text.GetBuffer ());
	float needValueY = fast_atof (eValueY->Text.GetBuffer ());
	float needValueZ = fast_atof (eValueZ->Text.GetBuffer ());

	Vector needVal = Vector (needValueX, needValueY, needValueZ);
	MasterAttrib->SetValue (needVal);

	Close (this);
}

void _cdecl TPositionEdit::OnButtonCancel (GUIControl* sender)
{
	Close (this);
}

void TPositionEdit::SetVector (Vector v)
{
	eValueX->Text = FloatToStr (v.x);
	eValueY->Text = FloatToStr (v.y);
	eValueZ->Text = FloatToStr (v.z);
	OnDataChange (NULL);
}




void TPositionEdit::KeyPressed(int Key, bool bSysKey)
{
  if (!bSysKey) return;

	bool ShiftState = false;
	if (GetAsyncKeyState(VK_SHIFT) < 0) ShiftState = true;

	
	if (lTimePressed > 0.2f)
	{
		if ((Key == 9) && (!ShiftState))
		{
			cIndex++;
			if (cIndex > 2) cIndex = 0;
			SelectIndex (cIndex);
			lTimePressed = 0.0f;
		}

		if ((Key == 9) && (ShiftState))
		{
			cIndex--;
			if (cIndex < 0) cIndex = 2;
			SelectIndex (cIndex);
			lTimePressed = 0.0f;
		}
	}


	if (Key == 13)
	{
		OnButtonOK (this);
	}
}