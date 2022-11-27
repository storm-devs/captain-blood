#ifndef LONG_EDIT_FORM
#define LONG_EDIT_FORM

#include "..\..\..\common_h\gui.h"

#define LONGEDIT_WINDOWNAME "Long"

class LongAttribute;

class TLongEdit : public GUIWindow
{
	
public:

	LongAttribute* MasterAttrib;

	GUIButton* btnOK;
	GUIButton* btnCancel;


  GUIEdit* eValue;

  TLongEdit (int posX, int posY);
  ~TLongEdit ();

	void OnCreate();
  void _cdecl OnDataChange (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);

	bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);

	void KeyPressed(int key, bool bSysKey);


  
};



#endif