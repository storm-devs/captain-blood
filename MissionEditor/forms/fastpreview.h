#ifndef PREVIEW_WINDOW
#define PREVIEW_WINDOW

#include "..\..\common_h\gui.h"
#include "..\missioneditor.h"

class BaseAttribute;

class TFastPreviewWindow : public GUIWindow
{
	

	void AddBaseAttr (BaseAttribute* pBase, int Level);

public:

	GUIButton* btnOK;
  
	GUIListBox* pAttrTable;

  
	TFastPreviewWindow (MissionEditor::tCreatedMO* pObject);
	~TFastPreviewWindow ();

	void _cdecl onOKPressed (GUIControl* sender);

	void Draw ();

  
};



#endif