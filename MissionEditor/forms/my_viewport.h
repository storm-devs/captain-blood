
#ifndef MY_VIEWPORT
#define MY_VIEWPORT

#include "..\..\common_h\gui.h"

class TViewPort : public GUIPanel
{
	
public:

	int initialX;
	int initialWidth;
	bool bActive;
	bool isFly;

	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
public:

	TViewPort (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~TViewPort ();
	
	virtual void Draw ();

	void KeyPressed (int key, bool bSysKey);


	void MouseUp (int button, const GUIPoint& pt);

  
};



#endif