//****************************************************************
#ifndef PROJECT_BROWSER
#define PROJECT_BROWSER

#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\particles.h"

class IFileService;

class TProjectBrowser : public GUIWindow
{
	IFileService* pFS;
	GUIComboBox* pPackName;
	GUIEdit* pTextureNameField;
	GUIListBox* pProjectList;
	GUIListBox* pAvailableSystemsList;
	GUIButton* pOpenTexture;
	GUIButton* pNewSystem;
	GUIButton* pAddSystem;
	GUIButton* pRemoveSystem;
	//GUIButton* pUpdateGeomCache;
	GUIButton* pCloseBrowser;

	//array<string> UsedGeomNames;


public:

	bool UserSelectSystemToOpen;
	string CurrentSystemName;
	string CurrentProjectName;
	string CurrentProjectNameShort;

	
	TProjectBrowser ();
	~TProjectBrowser ();

	virtual void Draw ();


//=================================================
public:
	void BuildProjectList ();
	void _cdecl OnSelectProject (GUIControl* sender);
	void _cdecl OnCreateNewPackInList (GUIControl* sender);
	void _cdecl OnPackIsChange (GUIControl* sender);
	
	void _cdecl OnAddSystem (GUIControl* sender);
	void _cdecl OnSelectTexture (GUIControl* sender);
	void _cdecl TextureSelected (GUIControl* sender);
	void _cdecl AddSystemClosed (GUIControl* sender);

	virtual void _cdecl OnRemoveSystem (GUIControl* sender);

	virtual void _cdecl OnSystemDblClick (GUIControl* sender);

	//void _cdecl UpdateGeomCacheInfo (GUIControl* sender);
	//void _cdecl UpdateGeomCacheInfoReal (GUIControl* sender);

	void _cdecl ExitFromBrowser (GUIControl* sender);
	

	
  
};



#endif