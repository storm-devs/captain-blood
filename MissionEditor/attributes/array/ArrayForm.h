#ifndef ARRAY_EDIT_FORM
#define ARRAY_EDIT_FORM

#include "..\..\..\common_h\gui.h"
class ArrayAttribute;

#define ARRAYEDIT_WINDOWNAME "Array"

class TArrayEdit : public GUIWindow
{
	
public:

	ArrayAttribute* MasterAttrib;

	GUIButton* btnOK;
	GUIButton* btnCancel;

  GUIEdit* eValue;;

  TArrayEdit (int posX, int posY);
  ~TArrayEdit ();


	void OnCreate();
  void _cdecl OnDataChange (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);



	void KeyPressed (int key, bool bSysKey);

  
};



#endif