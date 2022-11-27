#ifndef POSITION_EDITOR
#define POSITION_EDITOR

#include "base.h"
#include "..\..\..\common_h\gui.h"


class DataPosition;

class PositionEditor : public BaseEditor
{
  DataPosition* EditedPos;

	GUILabel* pLabelX;
	GUIEdit* pEditX;
	GUILabel* pLabelY;
	GUIEdit* pEditY;
	GUILabel* pLabelZ;
	GUIEdit* pEditZ;
  

public:


	PositionEditor ();
  virtual ~PositionEditor ();
  
  virtual void Release ();  

  virtual void BeginEdit (GUIControl* form, DataPosition* EditedPos);
	virtual void EndEdit ();
	virtual void Apply ();



};





#endif
