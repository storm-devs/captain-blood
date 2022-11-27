#ifndef _XBOX

#include "bool.h"
#include "..\..\system\datasource\databool.h"

BoolEditor::BoolEditor ()
{
	cbValue = NULL;
	EditedBool = NULL;
}

BoolEditor::~BoolEditor ()
{
}
  
void BoolEditor::Release ()
{
	delete this;
}

void BoolEditor::BeginEdit (GUIControl* form, DataBool* EditedBool)
{
	this->EditedBool = EditedBool;
	cbValue = NEW GUICheckBox (form, 10, 10, 100, 20);
	cbValue->ImageChecked->Load ("checked");
	cbValue->ImageNormal->Load ("normal");
	cbValue->Caption = EditedBool->GetName();
	cbValue->Checked = EditedBool->GetValue();
}

void BoolEditor::EndEdit ()
{
	delete cbValue;
	cbValue = NULL;
}

void BoolEditor::Apply ()
{
	EditedBool->SetValue(cbValue->Checked);
}


#endif