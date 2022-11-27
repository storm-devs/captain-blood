
//****************************************************************
#ifndef POSITION_EDIT_WINDOW
#define POSITION_EDIT_WINDOW

#include "..\..\common_h\gui.h"

class TNewFolder : public GUIWindow
{
	
public:


	GUIButton* btnOK;
	GUIButton* btnCancel;
  
	GUILabel* pDesc;
  GUIEdit* pDirName;

  
	TNewFolder ();
	~TNewFolder ();

	virtual void OnCreate ();

	void _cdecl onOKPressed (GUIControl* sender);
	void _cdecl onCancelPressed (GUIControl* sender);



	void KeyPressed(int Key, bool bSysKey);


  
};



#endif