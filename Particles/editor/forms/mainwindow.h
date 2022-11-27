//****************************************************************
#ifndef MAIN_WINDOW
#define MAIN_WINDOW

#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\gmx.h"
#include "..\..\..\common_h\render.h"
#include "..\..\..\common_h\particles.h"
#include "..\customcontrols\gui_viewport.h"
#include "..\customcontrols\gui_grapheditor.h"
#include "..\..\icommon\types.h"
#include "..\..\system\datasource\datasource.h"



class BaseEditor;


class TMainWindow : public GUIWindow
{
	float fTimeAfterRestart;
	bool bLowFPS;
	bool bShowAxis;
	IBaseTexture* pBackgroundTexture;
	float SaveOKTime;

	GUITreeNode* TempNodeToAdd;
	string TempLongFileName;
	string TempSystemName;
	string TempEmitterName;
	BaseEditor* NowEdited;

	int ViewPortWidth;
	int ViewPortHeight;

	GUIButton* btnNewParticleSystem; 
	GUIButton* btnOpenParticleSystem; 
	GUIButton* btnSaveParticleSystem; 
	GUIButton* btnSaveAsParticleSystem;
	
	GUIButton* btnCreatePointEmitter; 
	GUIButton* btnCreateBoxEmitter; 
	GUIButton* btnCreateSphereEmitter; 
	GUIButton* btnCreateLineEmitter; 
	GUIButton* btnCreateModelEmitter; 

	GUIButton* btnDelete; 
	GUIButton* btnCreateBillBoardParticle; 
	GUIButton* btnCreateModelParticle; 


	GUIButton* btnShowGrid; 
	GUIButton* btnShowDirection; 
	GUIButton* btnRenameObject; 
	GUIButton* btnEmulateLOWFps; 
	GUIButton* btnSelectAmbientColor; 
	GUIButton* btnSelectDirectionColor; 
	

	bool SystemIsPaused;
	bool SystemIsSlowMode;
	GUIButton* btnRestartSystem; 
	GUIButton* btnPauseSystem; 
	GUIButton* btnSlowMotionSystem; 

	GUIButton* btnSelectBackgroundTexture; 


	GUIButton* btnSimpleMode; 
	GUIButton* btnMoveMode; 
	GUIButton* btnRotateMode; 
	GUIComboBox* bGizmoMode;
	
	GUIButton* btnLoadGeom; 
	GUIButton* btnResetTransform; 
	IGMXScene* pGeom;


	GUIPanel* Panel1;
	GUIViewPort* ViewPort1;
	GUITreeView* TreeView1;


	bool bDrawGrid;


	static bool FuncCompare (GUITreeNode* const &a1, GUITreeNode* const &a2);


	int PositionInUndoBuffer;
	DWORD MaxUndoPos;
	array<MemFile*> UndoEntry;
	void InitUndoSystem(DWORD UndoSteps);
	void ShutdownUndoSystem ();
	void ClearUndoSystem ();
	void Undo ();
	void Redo ();
	void CreateUndo (bool KillSameUndos = true);
	void SetUndoButtonsState ();
	bool UndoSelectNode;


	int GZM_move_index;
	int GZM_rotate_index;



public:



	TMainWindow ();
	~TMainWindow ();

	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);

	virtual void Draw ();

	virtual void _cdecl DrawViewport (GUIControl* sender);

	virtual void DrawGrid ();

	virtual void MouseMove (int button, const GUIPoint& pt);
	virtual void MouseUp (int button, const GUIPoint& pt);


public:

	//Строит дерево для просмотра, систем, емиттеров и партиклов
	void BuildBrowserTree ();
	void AddFieldListToTree (GUITreeNodes* Parent, FieldList* pParticleInfo);

	


public:

//Обработчики =========================================================
	virtual void _cdecl ShowHideParticle (GUIControl* sender);
	virtual void _cdecl TreeViewChangeNode (GUIControl* sender);
	virtual void _cdecl RenameObjectPressed (GUIControl* sender);
	virtual void _cdecl EmitterRename (GUIControl* sender);
	virtual void _cdecl ParticleRename (GUIControl* sender);
	virtual void _cdecl SavePressed (GUIControl* sender);
	virtual void _cdecl UndoPressed (GUIControl* sender);
	virtual void _cdecl RedoPressed (GUIControl* sender);

	virtual void _cdecl CreatePointEmitterPressed (GUIControl* sender);
	virtual void _cdecl CreateBillboardPressed (GUIControl* sender);
	virtual void _cdecl CreateModelPressed (GUIControl* sender);


	virtual void _cdecl RealCreatePointEmitter (GUIControl* sender);
	virtual void _cdecl RealCreateBillboard (GUIControl* sender);
	virtual void _cdecl RealCreateModel (GUIControl* sender);

	virtual void _cdecl RestartSystem (GUIControl* sender);


	virtual void _cdecl DeleteObjectPressed (GUIControl* sender);

	virtual void _cdecl OpenNewSystemPressed (GUIControl* sender);
	virtual void _cdecl OnCloseManager (GUIControl* sender);

	virtual void _cdecl OnCreateNewSystem (GUIControl* sender);
	virtual void _cdecl OnCreateNewSystemReal (GUIControl* sender);

	
	virtual void _cdecl RealCreateEmptySystem (GUIControl* sender);

	//virtual void _cdecl AddToCurrentProject (GUIControl* sender);

	
	virtual void _cdecl SaveAsPressed (GUIControl* sender);
	virtual void _cdecl RealSaveAs (GUIControl* sender);

	void _cdecl RealLoadSystem (GUIControl* sender);

	virtual void _cdecl PausePressed (GUIControl* sender);
	virtual void _cdecl SlowMotionPressed (GUIControl* sender);


	virtual void _cdecl SelectBackgroundPressed (GUIControl* sender);
	virtual void _cdecl LoadBackgroundTexture (GUIControl* sender);

	virtual void _cdecl btnShowDirPressed (GUIControl* sender);
	virtual void _cdecl btnShowGridPressed (GUIControl* sender);


	virtual void _cdecl AmbientSelect (GUIControl* sender);
	virtual void _cdecl DirectionalSelect (GUIControl* sender);
	
	void _cdecl ApplyBackgroundColor (GUIControl* sender);
	void _cdecl ApplyDirectionalColor (GUIControl* sender);
	


	



	virtual void _cdecl btnSimplePressed (GUIControl* sender);
	virtual void _cdecl btnMovePressed (GUIControl* sender);
	virtual void _cdecl btnRotatePressed (GUIControl* sender);
	virtual void _cdecl btnGizmoTypeChanged (GUIControl* sender);
	virtual void _cdecl btnLoadGMXPressed (GUIControl* sender);
	virtual void _cdecl btnResetTransformPressed (GUIControl* sender);


	void _cdecl LoadGMXFile (GUIControl* sender);
	
	

	
	

	void KeyPressed(int Key, bool bSysKey);

private:

	void LoadToEditor (const char* pSysName);
	void BoldBranch (GUITreeNode* pNode);
	void UnBoldBranch (GUITreeNode* pNode);


	void ApplyChangeSystem (bool NeedCreateUndo = true);


	struct NodeInfo
	{
		bool Expanded;
		array<bool> Childs;
	};
	array<NodeInfo> ExpandedNode;

	void SaveTreeExpansion (MemFile* pFile);
	void RestoreTreeExpansion (MemFile* pFile);

	float TimeFromLastAutoSave;
	int AutoSaveIndex;
	int MaxAutoSaveSlots;
	void Autosave ();

public:

	void SaveGizmoMatrix ();
 
};



#endif