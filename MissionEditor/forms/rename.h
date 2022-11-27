//*
//****************************************************************
#ifndef RENAME_WINDOW
#define RENAME_WINDOW

#include "..\..\common_h\gui.h"

class TRenameWindow : public GUIWindow
{
	

public:

	bool ExitByOK;
	
	GUIButton* btnOK;
	GUIButton* btnCancel;
  
	GUILabel* pDesc;
  GUIEdit* pText;

  
	TRenameWindow (const string &curname);
	~TRenameWindow ();

	virtual void OnCreate ();

	void _cdecl onOKPressed (GUIControl* sender);
	void _cdecl onCancelPressed (GUIControl* sender);

	void KeyPressed(int Key, bool bSysKey);





  
};



#endif