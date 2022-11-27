#ifndef _XBOX
#ifndef GUI_CLIPER
#define GUI_CLIPER


#include "..\..\render.h"

#include "gui_rectangle.h"


class GUICliper
{
	
	GUIRectangle r;
	
	array <GUIRectangle> stack;
	
public:
	
	GUICliper ();
	~GUICliper ();
	
	void SetRectangle (const GUIRectangle& rect);
	const GUIRectangle& GetRectangle ();
	void SetInsRectangle (const GUIRectangle& rect);
	
	void SetFullScreenRect ();
	
	
	void Push ();
	void Pop ();
	
	
	
};



#endif

#endif