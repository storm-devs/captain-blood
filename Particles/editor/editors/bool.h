#ifndef BOOL_EDITOR
#define BOOL_EDITOR

#include "base.h"
#include "..\..\..\common_h\gui.h"


class DataBool;

class BoolEditor : public BaseEditor
{
	GUICheckBox* cbValue;
  DataBool* EditedBool;


public:


	BoolEditor ();
  virtual ~BoolEditor ();
  
  virtual void Release ();  

	virtual void BeginEdit (GUIControl* form, DataBool* EditedBool);
	virtual void EndEdit ();
	virtual void Apply ();



};





#endif
