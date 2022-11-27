#ifndef MAIN_WINDOW
#define MAIN_WINDOW

#include "..\..\..\..\common_h\gui.h"
#include "..\..\..\..\common_h\gmx.h"
#include "..\..\..\..\common_h\render.h"
#include "..\..\..\..\common_h\particles.h"
#include "..\customcontrols\gui_viewport.h"
#include "..\customcontrols\gui_secktor.h"
#include "..\customcontrols\gui_radiobutton2.h"
#include "..\customcontrols\gui_edit2.h"

#include "..\..\gui_scrollbar2.h"

class BaseEditor;

class TMainWindow : public GUIWindow
{	
	float SaveOKTime;
	
	int ViewPortWidth;
	int ViewPortHeight;


	GUIButton* btnExit;

	
	GUIButton* btnNewRagDoll; 
	GUIButton* btnOpenRagDoll; 
	GUIButton* btnSaveRagDoll; 
	GUIButton* btnSaveAsRagDoll;
	
	GUIButton* btnExportToBin;

	GUIButton* btnNewEnv;
	GUIButton* btnOpenEnv; 
	GUIButton* btnSaveEnv;
	GUIButton* btnSaveAsEnv;
	
	GUIButton* btnBoneAdd;
	GUIButton* btnBoneDelete; 
	GUIButton* btnLoadGeom;	

	
	GUIButton* btnAddEnvObj;
	GUIButton* btnDelEnvObj; 
	GUIButton* btnLoadEnvGeom;
	GUIButton* btnLoadSceletone;


	GUIButton* btnStartPhisSimulation; 
	GUIButton* btnStopPhisSimulation; 

	GUIButton* btnShowGeom; 
	GUIButton* btnToAttachBoneMode;
	GUIButton* btnShowDebugGeom;

	GUIButton* btnMoveMode;
	GUIButton* btnRotateMode;

//	GUIComboBox* bGizmoMode;

	GUIButton* btnWorld;
	GUIButton* btnLocal;

	GUIViewPort* ViewPort;

	class MyPanel *BottomPanel;

	GUIButton* btnShowSkel;

	GUIButton* btnDrawOrder;
	GUIButton* btnDrawPower;

	GUIButton* btnAutoLook;

	bool drawOrder;

	GUIRadioButton2* bCapsuleBoneType;
	GUIRadioButton2* bBoxBoneType; 

	GUIScrollBar2 *pScrollBoneHeight;
	GUIScrollBar2 *pScrollBoneLenght;
	GUIScrollBar2 *pScrollBoneWidth;

	GUIRadioButton2* bSphericalJointType;
	GUIRadioButton2* bRevoltJointType; 

	GUISecktor* SecktorAnchorA;
	GUISecktor* SecktorAnchorB;

	GUILabel* labelMass;

	GUIEdit2* pBoneMass;

	GUILabel* labelSpring;
	GUILabel* labelDumper;

	GUIEdit2* pBoneSpringA;
	GUIEdit2* pBoneDamperA;
	
	GUILabel* pLabelSpringB;
	GUIEdit2* pBoneSpringB;
	GUILabel* pLabelDamperB;
	GUIEdit2* pBoneDamperB;


	bool autoLook;

	int GZM_move_index;
	int GZM_rotate_index;

	int api_exit;



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
	
	bool ShowGeom;
	bool ShowSkel;

public:

	void UpdateScrolls();

	void LoadGMX (string ModelFileName);

	void LoadEnvGMX (string ModelFileName);

	void LoadSceletonFromAnt(string FileName);

	void MoveBottoms(int dy);

//	Обработчики =========================================================

	virtual void _cdecl OnCreateBone (GUIControl* sender);
	virtual void _cdecl OnDeleteBone (GUIControl* sender);

	virtual void _cdecl OnChangeBoneType (GUIControl* sender);
	virtual void _cdecl OnChangeBoneHeihgt (GUIControl* sender);
	virtual void _cdecl OnChangeBoneLenght (GUIControl* sender);
	virtual void _cdecl OnChangeBoneWidth (GUIControl* sender);
	virtual void _cdecl OnChangeJointType (GUIControl* sender);

	virtual void _cdecl LoadSkeleton (GUIControl* sender);
	virtual	void _cdecl SaveSkeleton (GUIControl* sender);
		
	virtual	void _cdecl ExportRagDoll (GUIControl* sender);

	virtual void _cdecl StartPhisSimulatione (GUIControl* sender);
	virtual void _cdecl StopPhisSimulation (GUIControl* sender);

	virtual void _cdecl OnFCancel (GUIControl* sender);
	
	
	virtual	void _cdecl OnChangePhysParams (GUIControl* sender);
	

	virtual	void _cdecl OnAddEnvObject (GUIControl* sender);
	virtual	void _cdecl OnDeleteEnvObject (GUIControl* sender);
		
	virtual void _cdecl btnLoadEnvGMXPressed (GUIControl* sender);
	virtual void _cdecl LoadEnvGMXFile (GUIControl* sender);
	
	virtual void _cdecl ToAttachBoneMode (GUIControl* sender);
	
	virtual void _cdecl ShowDebugGeom (GUIControl* sender);
		
	virtual void _cdecl OnLoadSceletonFromAnt(GUIControl* sender);
	virtual void _cdecl LoadSceletonFromAntEvent(GUIControl* sender);

	

	

	virtual void _cdecl ExitPressed (GUIControl* sender);


	virtual void _cdecl OnCreateNewRagDoll (GUIControl* sender);
	virtual void _cdecl OnOpenRagDoll (GUIControl* sender);
	virtual void _cdecl SavePressed (GUIControl* sender);
	virtual void _cdecl SaveAsPressed (GUIControl* sender);	
	
	virtual void _cdecl ExportPressed (GUIControl* sender);

	virtual void _cdecl OnCloseManager (GUIControl* sender);

		
	virtual void _cdecl btnMovePressed (GUIControl* sender);
	virtual void _cdecl btnRotatePressed (GUIControl* sender);

//	virtual void _cdecl btnGizmoTypeChanged (GUIControl* sender);

	virtual void _cdecl btnWorldPressed (GUIControl* sender);
	virtual void _cdecl btnLocalPressed (GUIControl* sender);
		
	virtual void _cdecl btnShowGMXModel (GUIControl* sender);	
	virtual void _cdecl btnLoadGMXPressed (GUIControl* sender);	


	virtual void _cdecl LoadGMXFile (GUIControl* sender);
	
	
	virtual void _cdecl ShowSkeletonPressed (GUIControl* sender);

	virtual void _cdecl DrawOrderPressed (GUIControl* sender);
	virtual void _cdecl DrawPowerPressed (GUIControl* sender);

	virtual void _cdecl AutoLookPressed (GUIControl* sender);

	virtual void _cdecl OnCreateNewEnv (GUIControl* sender);
	virtual void _cdecl OnOpenEnv (GUIControl* sender);
	virtual void _cdecl SaveEnvPressed (GUIControl* sender);
	virtual void _cdecl SaveAsEnvPressed (GUIControl* sender);	

	virtual void _cdecl LoadEnv (GUIControl* sender);
	virtual	void _cdecl SaveEnv (GUIControl* sender);

	

	void KeyPressed(int Key, bool bSysKey);	

};



#endif