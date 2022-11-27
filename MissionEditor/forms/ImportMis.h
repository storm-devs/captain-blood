#ifndef IMPORTED_MISSIONS_LIST_EDITOR
#define IMPORTED_MISSIONS_LIST_EDITOR

#include "..\..\common_h\gui.h"
#include "..\..\common_h\templates.h"
#include "..\..\common_h\LocStrings.h"



class TImportedMistEditor : public GUIWindow
{

public:

	GUIListBox* listBox;


	GUIButton* btnOK;
	GUIButton* btnCancel;




	TImportedMistEditor (int posX, int posY);
	~TImportedMistEditor ();



	void _cdecl OKPressed (GUIControl* sender);
	void _cdecl CancelPressed (GUIControl* sender);
	void _cdecl ClosePressed (GUIControl* sender);
	void _cdecl NewMissionSelected (GUIControl* sender);



};



#endif