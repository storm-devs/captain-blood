//*
//****************************************************************
#ifndef PANEL_WINDOW
#define PANEL_WINDOW

#include "..\..\common_h\gui.h"
#include "..\attributes\baseattr.h"
#include "my_viewport.h"
#include "resourceselect.h"

class TPanelWindowHandler;

#define SAVEDOK_SHOWTIME 0.8f



class TPanelWindow : public GUIWindow
{

	TViewPort* TPanel1;	
public:

	float MouseNotMovedTime;
	float SaveOKTime;
	float TimeFromLastAutoSave;
	int AutoSaveIndex;
	int MaxAutoSaveSlots;
	void Autosave ();
	void ForceAutosave ();



	GUILabel* PanelMessage;
	GUIButton* ApplyButton;
	GUIButton* CancelButton;

	GUIPanel* ToolsPanel;
	GUIButton* btnCreateNewMission; 
	GUIButton* btnCreateEvent; 
	GUIButton* btnCreateNew; 
	GUIButton* btnExportToEngine; 
	GUIButton* btnImportMission; 
	GUIButton* btnPlayGame; 
	GUIButton* btnCreateFolder; 
	GUIButton* btnDeleteSelected; 
	GUIButton* btnSaveMission; 
	GUIButton* btnLoadMission; 
	GUIButton* btnRenameObjects; 

/*	GUIButton* btnAddToVSS; 
	GUIButton* btnLoadMissionFromVSS; 
	GUIButton* btnCheckOut; 
	GUIButton* btnCheckIn; 
	GUIButton* btnUndoCheckOut; 
	GUIButton* btnGetLatestVersion; 
*/
	GUIButton* btnMoveObjects; 
	GUIButton* btnRotateObjects; 
	GUIButton* btnSelectObjects; 


	GUIEdit* MissionName;
	GUILabel* LabelMissionName;

	GUIButton* btnConvertTexturesInFolder; 
	GUIButton* btnCreatePKXFromFolder; 
	GUIButton* btnChangeBackgroundColor; 
	
	GUIButton* btnImportMissionsList;

	GUICheckBox* chckAdditionalDraw;


	void OnCreate();
	
	TPanelWindow ();
  ~TPanelWindow ();


	


	void _cdecl DefferedLoad (GUIControl* sender);
	void _cdecl DefferedImport (GUIControl* sender);
	
	void _cdecl btnCreateObjectPressed (GUIControl* sender);

	void _cdecl ChangeAdditionalDraw (GUIControl* sender);


	

	void _cdecl RunGamePressed (GUIControl* sender);
	void _cdecl DeleteSelectedPressed (GUIControl* sender);
	void _cdecl ExportPressed (GUIControl* sender);
	void _cdecl SaveMissionPressed (GUIControl* sender);
	void _cdecl ImportPressed (GUIControl* sender);
	void _cdecl Export (GUIControl* sender);
	void _cdecl LoadMissionPressed (GUIControl* sender);
	void _cdecl CreateFolderPressed (GUIControl* sender);

	void _cdecl CreateMissionPressed (GUIControl* sender);
	void _cdecl RealCreateMission (GUIControl* sender);
	void _cdecl CreateEmptyMission (GUIControl* sender);

	void _cdecl InsertArrayItemPressed (GUIControl* sender);
	void _cdecl RealInsertArrayItem (GUIControl* sender);

	

 
// recall
	void _cdecl RunGame (GUIControl* sender);
	void _cdecl Save (GUIControl* sender);
	void _cdecl Load (GUIControl* sender);
	void _cdecl Import (GUIControl* sender);
	void _cdecl CreateFolder (GUIControl* sender);

	void _cdecl RenameObjectPressed (GUIControl* sender);
	void _cdecl RenameObject (GUIControl* sender);



	


	virtual void Draw ();
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);


	virtual void KeyPressed(int Key, bool bSysKey);

	void RecalcWay (GUITreeNode* object);


	void _cdecl btnMoveObjectsPressed (GUIControl* sender); 
	void _cdecl btnRotateObjectsPressed (GUIControl* sender); 
	void _cdecl btnSelectObjectsPressed (GUIControl* sender); 

	void _cdecl RealDeleteSelected (GUIControl* sender);

	void _cdecl ApplyChanges (GUIControl* sender);
	void _cdecl CancelChanges (GUIControl* sender);

	GUITreeNode* FindUpperArrayItem (GUITreeNode* m_node);


	void _cdecl MissionNameIsChange (GUIControl* sender);
	void _cdecl ApplyBackgroundColor (GUIControl* sender);

	void _cdecl btnSelectBackground (GUIControl* sender);

	void _cdecl btnImportMissionsListAction (GUIControl* sender);

	

	
	void _cdecl btnCreatePKX (GUIControl* sender);
	void _cdecl createPKXEvent (GUIControl* sender);

	void _cdecl btnConvertTextures (GUIControl* sender);
	void _cdecl convertTexturesEvent (GUIControl* sender);



	void RecursiveDelete (GUITreeNode* sNode);

/*
	void _cdecl OpenFromVSS (GUIControl* sender);
	void _cdecl VSSLoginEntered (GUIControl* sender);


	void _cdecl VSS_Add_pressed (GUIControl* sender);
	void _cdecl VSS_CheckIn_pressed (GUIControl* sender);
	void _cdecl VSS_CheckOut_pressed (GUIControl* sender);
	void _cdecl VSS_UndoCheckOut_pressed (GUIControl* sender);
	void _cdecl VSS_GetLatest_pressed (GUIControl* sender);


	void _cdecl VSS_UndoCheckOut (GUIControl* sender);

	
	void SetThisMissionIsSourceControlFolder (bool bVal);
	*/

	void StartLoad (const char* szMissName);

	//void VSSAfterLoad(const char* szMissionName);

	void SaveProcess (const char* szFileName, bool autosave);


	bool HaveReadOnlyChilds (GUITreeNode * node);

	bool InsideReadOnlyFolder (GUITreeNode * node);


};



#endif