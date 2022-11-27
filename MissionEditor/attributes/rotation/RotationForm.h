#ifndef ROTATION_EDIT_FORM
#define ROTATION_EDIT_FORM

#include "..\..\..\common_h\gui.h"

#define ROTATIONEDIT_WINDOWNAME "Rotation"


class RotationAttribute;

class TRotationEdit : public GUIWindow
{

public:

	RotationAttribute* MasterAttrib;

	GUIButton* btnOK;
	GUIButton* btnCancel;


	GUIButton* btnCamera;

  GUIEdit* eValueX;
	GUIEdit* eValueY;
	GUIEdit* eValueZ;

//	GUILabel* lDescription;
/*
	GUILabel* lMin;
	GUILabel* lMax;
*/

  TRotationEdit (int posX, int posY);
  ~TRotationEdit ();

  void _cdecl OnDataChange (GUIControl* sender);

	void _cdecl OnButtonPressed (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);

	virtual void Draw ();

	void SetVector (Vector v);



	virtual void OnCreate();

	virtual void KeyPressed(int Key, bool bSysKey);	


  
};



#endif