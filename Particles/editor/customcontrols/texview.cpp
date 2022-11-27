#ifndef _XBOX

#include "texview.h"


	
TTexPanel::TTexPanel (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIPanel (pParent, Left, Top, Width, Height)
{
	RMB_Pressed = false;
  t_LMB_Pressed = NEW GUIEventHandler;
}

TTexPanel::~TTexPanel ()
{
	delete t_LMB_Pressed;
}


bool TTexPanel::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (message == GUIMSG_RMB_DOWN)	RMB_Pressed = true;
	if (message == GUIMSG_LMB_DOWN)	t_LMB_Pressed->Execute(this);

	return GUIPanel::ProcessMessages( message, lparam, hparam);
}

void TTexPanel::MouseUp(int button, const GUIPoint& pt)
{
	if (button == GUIMSG_RMB_UP) RMB_Pressed = false;
}


#endif