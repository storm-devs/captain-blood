#ifndef STRING_EDIT_FORM
#define STRING_EDIT_FORM

#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\templates.h"

#define STRINGEDIT_WINDOWNAME "String"


class StringAttribute;

class TStringEdit : public GUIWindow
{



public:

	StringAttribute* MasterAttrib;

	GUIButton* btnOK;
  GUIButton* btnCancel;
  
 	GUIButton* btnTakeFromStatic;
	GUIButton* btnTakeFromMission;
	GUIButton* btnTakeFromFile;


  GUIEdit* eValue;

  TStringEdit (int posX, int posY);
  ~TStringEdit ();

  void _cdecl OnDataChange (GUIControl* sender);

	void _cdecl OnButtonPressed (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);


	void _cdecl OnSelectFromMission (GUIControl* sender);

	void _cdecl OnSelectFromGMX (GUIControl* sender);

	void _cdecl OnSelectFromFile (GUIControl* sender);

	void _cdecl OnSelectFileAndClose (GUIControl* sender);

	void _cdecl OnCloseSFM (GUIControl* sender);
	void _cdecl OnCloseSFG (GUIControl* sender);




	void KeyPressed(int key, bool bSysKey);
  
};



#endif