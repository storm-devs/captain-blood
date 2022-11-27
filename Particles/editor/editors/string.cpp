#ifndef _XBOX

#include "string.h"
#include "..\..\system\datasource\datastring.h"


StringEditor::StringEditor ()
{
	EditedString = NULL;

	pLabel = NULL;
	pEdit = NULL;
}

StringEditor::~StringEditor ()
{
}
  
void StringEditor::Release ()
{
	delete this;
}


void StringEditor::BeginEdit (GUIControl* form, DataString* EditedString)
{
	this->EditedString = EditedString;

	pLabel = NEW GUILabel (form, 10, 10, 300, 20);
	pLabel->Caption = EditedString->GetName ();
	pEdit = NEW GUIEdit (form, 10, 30, 300, 20);
	pEdit->Text = EditedString->GetValue();
}

void StringEditor::EndEdit ()
{
	delete pLabel;
	pLabel = NULL;
	delete pEdit;
	pEdit = NULL;
}

void StringEditor::Apply ()
{
	EditedString->SetValue(pEdit->Text.GetBuffer());
}


#endif