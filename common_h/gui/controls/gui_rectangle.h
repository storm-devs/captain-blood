#ifndef _XBOX
#ifndef GUI_RECTANGLE
#define GUI_RECTANGLE

#include "gui_point.h"

class GUIRectangle
{
public:

	union
	{
		struct
		{
			int Left;
			int Top;
			int Width;
			int Height;
		};
		struct
		{
			int x;
			int y;
			int w;
			int h;
		};
		struct
		{
			GUIPoint pos;
			GUIPoint size;
		};
	};
	
	
	GUIRectangle ();
	GUIRectangle (int _Width, int _Height);
	GUIRectangle (const GUIPoint & _size);
	GUIRectangle (int _Left, int _Top, int _Width, int _Height);
	GUIRectangle (const GUIPoint & _pos, const GUIPoint & _size);
	~GUIRectangle ();

	bool Inside(int pointX, int pointY);
	bool Inside(const GUIPoint & point);

	GUIRectangle & Intersection(const GUIRectangle & r);
	
	///Присвоить
	GUIRectangle & operator = (const GUIRectangle & r);
	
};


#endif

#endif