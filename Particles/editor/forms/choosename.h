//****************************************************************
#ifndef CHOOSE_NAME_WINDOW
#define CHOOSE_NAME_WINDOW

#include "..\..\..\common_h\gui.h"

class TChooseWindow : public GUIWindow
{
	

public:

	bool ExitByOK;
	
	GUIButton* btnOK;
	GUIButton* btnCancel;
  
	GUILabel* pDesc;
  GUIEdit* pText;

  
	TChooseWindow (const string &curname);
	~TChooseWindow ();

	virtual void OnCreate ();

	void _cdecl onOKPressed (GUIControl* sender);
	void _cdecl onCancelPressed (GUIControl* sender);

	void KeyPressed(int Key);





  
};



#endif