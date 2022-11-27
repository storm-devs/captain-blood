//****************************************************************
#ifndef TEX_VIEW
#define TEX_VIEW

#include "..\..\..\common_h\gui.h"

extern GUIEventHandler* hack_temp_ptr;


#define LMB_Pressed \
	t_LMB_Pressed->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)



class TTexPanel : public GUIPanel
{

public:
	
	bool RMB_Pressed;
	TTexPanel (GUIControl* pParent, int Left, int Top, int Width, int Height);
	~TTexPanel ();


	bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);

	void MouseUp(int button, const GUIPoint& pt);


	GUIEventHandler* t_LMB_Pressed;

  
};



#endif