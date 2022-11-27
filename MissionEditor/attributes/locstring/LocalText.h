#ifndef LOCALIZED_TEXT_EDITOR
#define LOCALIZED_TEXT_EDITOR

#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\templates.h"
#include "..\..\..\common_h\LocStrings.h"



class TLocTextEditor : public GUIWindow
{

public:

	GUIListBox* listBox;


	//GUIButton* addString;
	//GUIButton* delString;


	GUIButton* btnOK;
	GUIButton* btnCancel;




	TLocTextEditor (int posX, int posY);
	~TLocTextEditor ();


	void BuildLocString();


	void _cdecl OKPressed (GUIControl* sender);
	void _cdecl CancelPressed (GUIControl* sender);
	void _cdecl strAdd (GUIControl* sender);
	void _cdecl strDel (GUIControl* sender);

	void _cdecl strDelConfirm (GUIControl* sender);
	void _cdecl strAddOk (GUIControl* sender);


};



#endif