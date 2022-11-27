#ifndef SELECTFROMMISSION_WINDOW
#define SELECTFROMMISSION_WINDOW

#include "..\..\..\common_h\gui.h"

class TSSFMission : public GUIWindow
{



public:


	
	GUIButton* btnOK; 
	GUIButton* btnCANCEL; 
	GUIListBox* ListBox1;
	
	TSSFMission (int posX, int posY);
	~TSSFMission ();

	static bool TSSFMission::FuncCompare (const string& s1, const string& s2);

	void _cdecl OKPressed (GUIControl* sender);
	void _cdecl CANCELPressed (GUIControl* sender);

  
};



#endif