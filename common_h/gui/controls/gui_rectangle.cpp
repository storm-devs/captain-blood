#ifndef _XBOX

#include "gui_rectangle.h"


GUIRectangle::GUIRectangle ()
{
	Left   = 0;
	Top    = 0;
	Width  = 0;
	Height = 0;
}

GUIRectangle::GUIRectangle (int _Width, int _Height)
{
	Left   = 0;
	Top    = 0;
	Width  = _Width;
	Height = _Height;
}

GUIRectangle::GUIRectangle (const GUIPoint & _size)
{
	Left   = 0;
	Top    = 0;
	Width  = _size.X;
	Height = _size.Y;
}


GUIRectangle::GUIRectangle (int _Left, int _Top, int _Width, int _Height)
{
	Left   = _Left;
	Top    = _Top;
	Width  = _Width;
	Height = _Height;
}

GUIRectangle::GUIRectangle (const GUIPoint & _pos, const GUIPoint & _size)
{
	Left   = _pos.X;
	Top    = _pos.Y;
	Width  = _size.X;
	Height = _size.Y;
}

GUIRectangle::~GUIRectangle ()
{
}

bool GUIRectangle::Inside(int pointX, int pointY)
{
	if (pointX < Left) return false;
	if (pointX > (Left+Width-1)) return false;
	
	if (pointY < Top) return false;
	if (pointY > (Top+Height-1)) return false;
	
	return true;
}

bool GUIRectangle::Inside(const GUIPoint & point)
{
	return Inside(point.X, point.Y);
}

GUIRectangle & GUIRectangle::Intersection(const GUIRectangle & r)
{
	long l = x + w;
	long b = y + h;
	long rl = r.x + r.w;
	long rb = r.y + r.h;
	if(x < r.x) x = r.x;
	if(y < r.y) y = r.y;
	if(l > rl) l = rl;
	if(b > rb) b = rb;
	w = l - x;
	h = b - y;
	if(w < 0) w = 0;
	if(h < 0) h = 0;
	return *this;
}

///Присвоить
GUIRectangle & GUIRectangle::operator = (const GUIRectangle & r)
{
	Left   = r.Left;
	Top    = r.Top; 
	Width  = r.Width; 
	Height = r.Height;
	return *this; 
}




#endif