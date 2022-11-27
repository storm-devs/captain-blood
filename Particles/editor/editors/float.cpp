#ifndef _XBOX

#include "float.h"
#include "..\..\system\datasource\datafloat.h"

FloatEditor::FloatEditor ()
{
  EditedFloat = NULL;
  
	pLabel = NULL;
	pEdit = NULL;
}

FloatEditor::~FloatEditor ()
{
}
  
void FloatEditor::Release ()
{
 delete this;
}  

void FloatEditor::BeginEdit (GUIControl* form, DataFloat* EditedFloat)
{
	this->EditedFloat = EditedFloat;
	pLabel = NEW GUILabel (form, 10, 10, 300, 20);
	pLabel->Caption = EditedFloat->GetName ();
	pEdit = NEW GUIEdit (form, 10, 30, 300, 20);
	pEdit->Text.Format("%3.2f", EditedFloat->GetValue());
}

void FloatEditor::EndEdit ()
{
	delete pEdit;
	pEdit = NULL;
	delete pLabel;
	pLabel = NULL;
}

void FloatEditor::Apply ()
{
	EditedFloat->SetValue ((float)atof (pEdit->Text.GetBuffer()));
}

#endif