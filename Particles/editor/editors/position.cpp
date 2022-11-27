#ifndef _XBOX

#include "position.h"
#include "..\..\system\datasource\dataposition.h"


PositionEditor::PositionEditor ()
{
	EditedPos = NULL;

	pLabelX = NULL;
	pEditX = NULL;
	pLabelY = NULL;
	pEditY = NULL;
	pLabelZ = NULL;
	pEditZ = NULL;
}

PositionEditor::~PositionEditor ()
{
}
  
void PositionEditor::Release ()
{
	delete this;
}

void PositionEditor::BeginEdit (GUIControl* form, DataPosition* EditedPos)
{
	this->EditedPos = EditedPos;
	pLabelX = NEW GUILabel (form, 10, 10, 100, 20);
	pLabelX->Caption = "X";
	pEditX = NEW GUIEdit (form, 10, 30, 100, 20);
	pEditX->Text.Format("%3.2f", EditedPos->GetValue().x);

	pLabelY = NEW GUILabel (form, 120, 10, 100, 20);
	pLabelY->Caption = "Y";
	pEditY = NEW GUIEdit (form, 120, 30, 100, 20);
	pEditY->Text.Format("%3.2f", EditedPos->GetValue().y);


	pLabelZ = NEW GUILabel (form, 230, 10, 100, 20);
	pLabelZ->Caption = "Z";
	pEditZ = NEW GUIEdit (form, 230, 30, 100, 20);
	pEditZ->Text.Format("%3.2f", EditedPos->GetValue().z);


}

void PositionEditor::EndEdit ()
{
	delete pLabelX;
	pLabelX = NULL;
	delete pEditX;
	pEditX = NULL;
	delete pLabelY;
	pLabelY = NULL;
	delete pEditY;
	pEditY = NULL;
	delete pLabelZ;
	pLabelZ = NULL;
	delete pEditZ;
	pEditZ = NULL;
}

void PositionEditor::Apply ()
{
	Vector val;
	val.x = (float)atof (pEditX->Text.GetBuffer());
	val.y = (float)atof (pEditY->Text.GetBuffer());
	val.z = (float)atof (pEditZ->Text.GetBuffer());
	EditedPos->SetValue (val);
}


#endif