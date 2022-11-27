#ifndef _XBOX

#include "gui_point.h"


GUIPoint::GUIPoint ()
{
	X   = 0;
	Y   = 0;
}

GUIPoint::GUIPoint (int _X, int _Y)
{
	X   = _X;
	Y   = _Y;
}

GUIPoint::~GUIPoint ()
{
}

///Присвоить
GUIPoint & GUIPoint::operator = (const GUIPoint & r)
{
	X   = r.X;
	Y   = r.Y; 
	return *this; 
}

GUIPoint & GUIPoint::operator = (long v)
{
	X = Y = v;
	return *this; 
}

GUIPoint & GUIPoint::operator += (const GUIPoint & p)
{
	X   += p.X;
	Y   += p.Y; 
	return *this; 
}

GUIPoint & GUIPoint::operator -= (const GUIPoint & p)
{
	X   -= p.X;
	Y   -= p.Y; 
	return *this; 
}

GUIPoint & GUIPoint::operator *= (const GUIPoint & p)
{
	X   *= p.X;
	Y   *= p.Y; 
	return *this; 
}

GUIPoint & GUIPoint::operator /= (const GUIPoint & p)
{
	X   /= p.X;
	Y   /= p.Y; 
	return *this; 
}

GUIPoint & GUIPoint::operator *= (long v)
{
	X   *= v;
	Y   *= v; 
	return *this; 
}

GUIPoint & GUIPoint::operator /= (long v)
{
	X   /= v;
	Y   /= v; 
	return *this; 
}

GUIPoint GUIPoint::operator + (const GUIPoint & p)
{
	GUIPoint t = *this;
	t.X += p.X;
	t.Y += p.Y;
	return t;
}

GUIPoint GUIPoint::operator - (const GUIPoint & p)
{
	GUIPoint t = *this;
	t.X -= p.X;
	t.Y -= p.Y;
	return t;
}

GUIPoint GUIPoint::operator * (const GUIPoint & p)
{
	GUIPoint t = *this;
	t.X *= p.X;
	t.Y *= p.Y;
	return t;
}

GUIPoint GUIPoint::operator / (const GUIPoint & p)
{
	GUIPoint t = *this;
	t.X /= p.X;
	t.Y /= p.Y;
	return t;
}

GUIPoint GUIPoint::operator * (long v)
{
	GUIPoint t = *this;
	t.X *= v;
	t.Y *= v;
	return t;
}

GUIPoint GUIPoint::operator / (long v)
{
	GUIPoint t = *this;
	t.X /= v;
	t.Y /= v;
	return t;
}


#endif