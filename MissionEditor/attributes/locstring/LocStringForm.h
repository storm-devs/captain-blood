#ifndef LOCALIZED_STRING_EDIT_FORM
#define LOCALIZED_STRING_EDIT_FORM

#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\templates.h"

#define STRINGEDIT_WINDOWNAME "String"


class LocStringAttribute;

class TLocStringEdit : public GUIWindow
{

	long currentValue;


	bool bLocked;

public:

	LocStringAttribute* MasterAttrib;

	GUIButton* btnOK;
  GUIButton* btnCancel;
  
	GUIButton* btnTakeFromTranslatedString;


	GUIEdit* TextValue;


  TLocStringEdit (int posX, int posY);
  ~TLocStringEdit ();


	void _cdecl OnButtonPressed (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);


	void _cdecl OnSelectFromLocalText (GUIControl* sender);
	void _cdecl OnSelectFromLocalTextClose (GUIControl* sender);

	void KeyPressed(int key, bool bSysKey);
  
};



#endif