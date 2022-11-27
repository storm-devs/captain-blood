#ifndef NEW_STRING_WINDOW
#define NEW_STRING_WINDOW

#include "..\..\..\common_h\gui.h"

class TNewLocStringWindow : public GUIWindow
{
	

public:

	bool ExitByOK;
	
	GUIButton* btnOK;
	GUIButton* btnCancel;
  
	GUILabel* pDesc;
  GUIEdit* pText;

  
	TNewLocStringWindow (const string &curname);
	~TNewLocStringWindow ();

	virtual void OnCreate ();

	void _cdecl onOKPressed (GUIControl* sender);
	void _cdecl onCancelPressed (GUIControl* sender);

	void KeyPressed(int Key, bool bSysKey);





  
};



#endif