#ifndef STRING_EDITOR
#define STRING_EDITOR

#include "base.h"
#include "..\..\..\common_h\gui.h"


class DataString;

class StringEditor : public BaseEditor
{
  DataString* EditedString;

	GUILabel* pLabel;
	GUIEdit* pEdit;



public:


	StringEditor ();
  virtual ~StringEditor ();
  
  virtual void Release ();  

	virtual void BeginEdit (GUIControl* form, DataString* EditedString);
	virtual void EndEdit ();
	virtual void Apply ();



};





#endif
