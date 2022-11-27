#ifndef FLOAT_EDITOR
#define FLOAT_EDITOR

#include "base.h"
#include "..\..\..\common_h\gui.h"


class DataFloat;

class FloatEditor : public BaseEditor
{
  DataFloat* EditedFloat;
  
	GUILabel* pLabel;
	GUIEdit* pEdit;
  


public:


	FloatEditor ();
  virtual ~FloatEditor ();
  
  virtual void Release ();  

	virtual void BeginEdit (GUIControl* form, DataFloat* EditedFloat);
	virtual void EndEdit ();
	virtual void Apply ();



};





#endif
