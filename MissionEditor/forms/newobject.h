//*
//****************************************************************
#ifndef NEW_OBJECT_WINDOW
#define NEW_OBJECT_WINDOW

#include "..\..\common_h\gui.h"
#include "..\missioneditor.h"
#include "..\..\common_h\mission.h"


class TNewObjectWindow : public GUIWindow
{
	
  //MissionEditor::tAvailableMO* pMissionObject;

	array<string> Groups;

	void AddGroup (const char* szGroup);

public:
	
	//GUIComboBox* AvailObj;
	GUIListBox* ObjectsList;
	GUIListBox* GroupList;
	GUIEdit* txtObjectName;
  GUIButton* btnOK; 
	GUIButton* btnCancel; 

	//GUITreeView* TreeView1;
	
	TNewObjectWindow ();
	~TNewObjectWindow ();

	GUITreeNode* FindFirstFolder (GUITreeNode* m_node);

	void _cdecl OnAcceptNewObjectName (GUIControl* sender);

	void _cdecl SelectionChange (GUIControl* sender);

	void _cdecl btnCreatePressed (GUIControl* sender);
	void _cdecl btnCancelPressed (GUIControl* sender);

	void _cdecl UpdateTree (GUIControl* sender);

	void _cdecl NameChanged (GUIControl* sender);

	void _cdecl MissionObjectSelected (GUIControl* sender);

	void _cdecl ObjectShowHint (GUIControl* sender);


	

	void _cdecl TNewObjectWindow::GroupChanged (GUIControl* sender);

	


	void OnCreate ();

	void KeyPressed(int Key, bool bSysKey);


};



#endif