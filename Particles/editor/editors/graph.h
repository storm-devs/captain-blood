#ifndef GRAPH_EDITOR
#define GRAPH_EDITOR 

#include "base.h"
#include "..\..\..\common_h\gui.h"
#include "..\CustomControls\gui_graphline.h"
#include "..\CustomControls\gui_grapheditor.h"

class DataGraph;

class GraphEditor : public BaseEditor
{
	bool bRelative;
	GUIGraphLine* GraphLineMax;
	GUIGraphLine* GraphLineMin;

	GUIButton* btnMove; 
	GUIButton* btnScale; 
	GUIButton* btnScaleX; 
	GUIButton* btnScaleY; 
	GUIButton* btnMoveLineY; 
	GUIButton* btnScaleLineY; 
	GUIButton* btnScaleLineX; 
	GUIButton* btnMoveLineX; 
	GUIButton* btnCopy; 
	GUIButton* btnPaste; 

	GUIButton* btnCopyMaxToMin; 
	GUIButton* btnCopyMinToMax; 
	GUIButton* btnFindZeroAxis; 

	GUIButton* btnInsertParabols; 
	GUIGraphEditor* pGraphEditor;
	GUICheckBox* PercentGraph;

	DataGraph* EditedGraph;


	void CreateDataForEdit ();

public:


	GraphEditor ();
  virtual ~GraphEditor ();
  
  virtual void Release ();  

  virtual void BeginEdit (GUIControl* form, DataGraph* EditedGraph);
	virtual void EndEdit ();	
	virtual void Apply ();

public:
	//================== обработчики =============================
	virtual void _cdecl btnMovePressed (GUIControl* sender);
	virtual void _cdecl btnScalePressed (GUIControl* sender);
	virtual void _cdecl btnScaleXPressed (GUIControl* sender);
	virtual void _cdecl btnScaleYPressed (GUIControl* sender);
	virtual void _cdecl btnMoveYPressed (GUIControl* sender);
	virtual void _cdecl btnMoveXPressed (GUIControl* sender);
	virtual void _cdecl btnScaleLineXPressed (GUIControl* sender);
	virtual void _cdecl btnScaleLineYPressed (GUIControl* sender);
	virtual void _cdecl btnCopyPressed (GUIControl* sender);
	virtual void _cdecl btnPastePressed (GUIControl* sender);
	virtual void _cdecl btnMin2Max (GUIControl* sender);
	virtual void _cdecl btnMax2Min (GUIControl* sender);
	virtual void _cdecl btnFindZero (GUIControl* sender);
	virtual void _cdecl btnInsertParabolsPressed (GUIControl* sender);
	void _cdecl RelativeChanged (GUIControl* sender);


};


#endif