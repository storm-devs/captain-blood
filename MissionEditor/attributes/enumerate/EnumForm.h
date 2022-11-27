#ifndef ENUM_EDIT_FORM
#define ENUM_EDIT_FORM

#include "..\..\..\common_h\gui.h"
class EnumAttribute;

#define ENUMEDIT_WINDOWNAME "Enumerate"

class TEnumEdit : public GUIWindow
{
	
public:

	EnumAttribute* MasterAttrib;

	GUIButton* btnOK;
	GUIButton* btnCancel;

  GUIComboBox* cbValue;

  TEnumEdit (int posX, int posY);
  ~TEnumEdit ();

  void _cdecl OnDataChange (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);

	bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
  
};



#endif