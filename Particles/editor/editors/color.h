#ifndef COLOR_EDITOR
#define COLOR_EDITOR

#include "base.h"
#include "..\..\..\common_h\gui.h"
#include "..\CustomControls\coloredit.h"

class DataColor;

class ColorEditor : public BaseEditor
{
	DataColor* EditedColor;
	GUIColorEditor* pEditor;

public:


	ColorEditor ();
  virtual ~ColorEditor ();
  
  virtual void Release ();  

	virtual void BeginEdit (GUIControl* form, DataColor* EditedColor);
	virtual void EndEdit ();
	virtual void Apply ();



};





#endif
