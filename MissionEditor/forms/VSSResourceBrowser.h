#ifndef VSS_RESOURCE_BROWSER
#define VSS_RESOURCE_BROWSER

#include "..\..\common_h\gui.h"
#include "..\SourceSafe/VSSHelper.h"

class TVSSResourceBrowser : public GUIWindow
{

	string strVSSUserName;
	

public:

	
	GUIButton* btnOK;
	GUIButton* btnCancel;

	GUITreeView* TreeView1;
  
  
	TVSSResourceBrowser (const char* szUserName);
	~TVSSResourceBrowser ();

	virtual void OnCreate ();

	void _cdecl onOKPressed (GUIControl* sender);
	void _cdecl onCancelPressed (GUIControl* sender);

	void KeyPressed(int Key, bool bSysKey);



	//void AddRecursive (GUITreeNodes* pNodes, SourceSafeItem &root);


  
};



#endif