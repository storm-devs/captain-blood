#ifndef COLOR_EDIT_FORM
#define COLOR_EDIT_FORM

#include "..\..\..\common_h\gui.h"
class ColorAttribute;

#define COLOREDIT_WINDOWNAME "Color"

class TColorEdit : public GUIColorPicker
{
	
public:


//	GUILabel* lScale;
//	GUIEdit* edtScale;


	ColorAttribute* MasterAttrib;

  TColorEdit (int posX, int posY);
  ~TColorEdit ();

	virtual void _cdecl OKClick (GUIControl* sender);


	bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);


  
};



#endif