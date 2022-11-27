#ifndef MAIN_WINDOW
#define MAIN_WINDOW

#include "..\..\common_h\gui.h"
#include "..\attributes\baseattr.h"

extern IMission* miss;

class TMainWindow : public GUIWindow
{
	
float WDontPressedTime;
float EDontPressedTime;


public:

	static bool TMainWindow::FuncCompare (GUITreeNode* const &a1, GUITreeNode* const &a2);
	static int TMainWindow::NeedToSortChilds(GUITreeNode* const &a1);


	GUITreeView* TreeView1;
	
	TMainWindow (GUIControl* parent = NULL, int posX = 0, int posY = 0, int width = -1, int height = -1);
	~TMainWindow ();

  void _cdecl btnCreatePressed (GUIControl* sender);
	void _cdecl btnCreatePathPressed (GUIControl* sender);




	void _cdecl SelectionChange (GUIControl* sender);
	void _cdecl UpdateTree (GUIControl* sender);


	void SortTree();



	void _cdecl WhenDrop (GUIControl* sender);
	void _cdecl WhenCopy (GUIControl* sender);





	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);


	virtual void OnCreate();

	virtual void Draw();

	void _cdecl CreateDefaultFolders (GUIControl* sender);

	void _cdecl TreeShowHint (GUIControl* sender);

	void _cdecl SelectEvent (GUIControl* sender);
	void _cdecl BeforeSelectEvent (GUIControl* sender);

	
	void _cdecl BeforeDropItem (GUIControl* sender);
	void _cdecl FastObjectPropView (GUIControl* sender);

	


	GUITreeNode* FindObjectNode (GUITreeNode* c_node);


	void ShowOrHideFolder(GUITreeNode* node);
	void ShowOrHideFolderProcess(GUITreeNode* node, bool Hide);


	void Resize (dword dwNewWidth);



	void DeleteAllReadOnlyObjectsRecursive (GUITreeNodes* items);
	void DeleteAllReadOnlyObjects ();


	void RefreshImportedMissions ();

  
};



#endif