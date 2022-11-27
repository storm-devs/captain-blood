
/*
#ifndef SELECTFROMGMX_WINDOW
#define SELECTFROMGMX_WINDOW

#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\gmx.h"

class TSSFGeometry : public GUIWindow
{

IGMXScene* pScene;

public:


	
	GUIButton* btnOK; 
	GUIButton* btnCANCEL; 
	GUITreeView* TreeView1;
	
	TSSFGeometry (int posX, int posY, IGMXScene* scene);
	~TSSFGeometry ();


	void _cdecl OKPressed (GUIControl* sender);
	void _cdecl CANCELPressed (GUIControl* sender);


	void AddNode (IGMXEntity* pEnt);

	GUITreeNode* FindObject (IGMXEntity* obj, GUITreeNodes* items);

  
};



#endif
*/