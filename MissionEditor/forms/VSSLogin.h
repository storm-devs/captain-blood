#ifndef VSS_LOGIN_WINDOW
#define VSS_LOGIN_WINDOW

#include "..\..\common_h\gui.h"

class TVSSLogin : public GUIWindow
{
	

public:

	bool ExitByOK;
	
	GUIButton* btnOK;
	GUIButton* btnCancel;
  
	GUILabel* pDesc;
  GUIEdit* pText;

  
	TVSSLogin ();
	~TVSSLogin ();

	virtual void OnCreate ();

	void _cdecl onOKPressed (GUIControl* sender);
	void _cdecl onCancelPressed (GUIControl* sender);

	void KeyPressed(int Key, bool bSysKey);





  
};



#endif