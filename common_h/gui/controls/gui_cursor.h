#ifndef _XBOX
#ifndef GUI_MOUSE_CURSOR
#define GUI_MOUSE_CURSOR


#include "gui_point.h"
#include "gui_image.h"


class IControls;

class GUICursor
{
	//POINT last_cursor_pos;
	
	struct SavedCursor
	{
		string Name;
		int OfsX;
		int OfsY;
	};

	array<SavedCursor> Cursors;
	
	GUIPoint position;
	
	int cOffsetX;
	int cOffsetY;

	
	string CurrentCursor;
	GUIImage* cursor;

	IControls * ctrl;
	
public:
	
	float DeltaX;
	float DeltaY;

	GUICursor (const char* FileName);
	~GUICursor ();
	
	void Update ();
	
	void Draw (); 
	
	
	const GUIPoint& GetPosition ();
	void SetPosition (const GUIPoint& pos);


	void Push ();
	void Pop ();

	void SetCursor (const char* name, int offsetY = 0, int offsetX= 0);

	GUIImage* GetImage ();

	int GetOffsetX ();
	int GetOffsetY ();
	
};


#endif

#endif