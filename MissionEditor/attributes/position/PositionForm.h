#ifndef POSITION_EDIT_FORM
#define POSITION_EDIT_FORM

#include "..\..\..\common_h\gui.h"

#define POSITIONEDIT_WINDOWNAME "Position"

class PositionAttribute;


class TPositionEdit : public GUIWindow
{

	float lTimePressed;
	int cIndex;

	void SelectIndex (int i);
public:

	PositionAttribute* MasterAttrib;

	GUIButton* btnOK;
	GUIButton* btnCancel;


	GUIButton* btnCamera;

  GUIEdit* eValueX;
	GUIEdit* eValueY;
	GUIEdit* eValueZ;


/*
	GUILabel* lMin;
	GUILabel* lMax;
*/

  TPositionEdit (int posX, int posY);
  ~TPositionEdit ();

	virtual void Draw ();
  void _cdecl OnDataChange (GUIControl* sender);

	void _cdecl OnButtonPressed (GUIControl* sender);

	void _cdecl OnButtonOK (GUIControl* sender);
	void _cdecl OnButtonCancel (GUIControl* sender);

	void SetVector (Vector v);




	virtual void OnCreate() ;


	virtual void KeyPressed(int Key, bool bSysKey);


  
};



#endif