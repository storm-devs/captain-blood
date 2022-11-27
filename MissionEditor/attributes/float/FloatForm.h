#ifndef FLOAT_EDIT_FORM
#define FLOAT_EDIT_FORM

#include "..\..\..\common_h\gui.h"

#define FLOATEDIT_WINDOWNAME "Float"

class FloatAttribute;

class TFloatEdit : public GUIWindow
{
	
public:


	void OnCreate();
	void KeyPressed(int key, bool bSysKey);
	
	FloatAttribute* MasterAttrib;

	GUIButton* btnOK;
	GUIButton* btnCancel;

  GUIEdit* eValue;

  TFloatEdit (int posX, int posY);
  ~TFloatEdit ();

  void _cdecl OnDataChange (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);


	bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);


  
};



#endif