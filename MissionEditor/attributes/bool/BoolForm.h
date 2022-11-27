#ifndef BOOLEAN_EDIT_FORM
#define BOOLEAN_EDIT_FORM

#include "..\..\..\common_h\gui.h"
class BoolAttribute;

#define BOOLEDIT_WINDOWNAME "Boolean"

class TBoolEdit : public GUIWindow
{
	
public:

	BoolAttribute* MasterAttrib;

	GUIButton* btnOK;
	GUIButton* btnCancel;

  GUICheckBox* cbValue;

  TBoolEdit (int posX, int posY);
  ~TBoolEdit ();

  void _cdecl OnDataChange (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);

	bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);

  
};



#endif