//*
//****************************************************************
#ifndef RESOURCE_SELECTOR_WINDOW
#define RESOURCE_SELECTOR_WINDOW

#include "..\..\common_h\gui.h"
#include "..\..\common_h\gmx.h"
#include "..\..\common_h\render.h"
#include "..\..\common_h\particles.h"

#define OnSelect \
	t_OnSelect->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)


class IFinder;

class TResourceSelectorWindow : public GUIWindow
{
	struct ExtensionInfo
	{
		string ext;
		string image;
	};

	array <ExtensionInfo> ExtInfo;

	array <string> SubstratedPath;

	GUIButton* btnOK;
	GUIButton* btnCANCEL;


	GUIPanel* Preview;
	GUITreeView* TreeView1;

	string previewname;
	IBaseTexture* PreviewTexture;
	IGMXScene* PreviewScene;

	IParticleManager* pParticleManager;
	IAnimationScene * anims;
	IParticleSystem* PreviewSystem;

	float angle;

	dword dwWinWidth;
	dword dwWinHeight;
	
public:

	string ObjectName;

	TResourceSelectorWindow ();
	~TResourceSelectorWindow ();


	ExtensionInfo* GetExtInfo (const char* ext);
	void ReadExtInfo ();

	void AddScanDir (const char* dirName);

	void SubstractPath (string& fullname);
	void ScanForResources ();

	void _cdecl OnSelectResource (GUIControl* sender);
	void _cdecl OnOpenResource (GUIControl* sender);

	static bool TResourceSelectorWindow::FuncCompare (GUITreeNode* const &a1, GUITreeNode* const &a2);

	GUIEventHandler* t_OnSelect;  

	void BeginPreview ();

	void _cdecl RenderPreview (GUIControl* sender);

	void ExpandAndGoToParent (GUITreeNode* node);

};



#endif