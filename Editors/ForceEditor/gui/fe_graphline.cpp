#ifndef _XBOX

#include "fe_graphline.h"




GUIGraphLine::GUIGraphLine (const Vector& begin, const Vector& end) : Points(_FL_)
{
	bNegative = false;
	LastMinimalPoint = -1;
	bActive = false;
	SelectedPoint = -1;
	color = 0xFFFF0000;

	Points.Add(begin);
	Points.Add(end);
}
	
GUIGraphLine::~GUIGraphLine ()
{
	Points.DelAll();
}

void GUIGraphLine::systemClear ()
{
	Points.DelAll();
}

void GUIGraphLine::Insert (int num, const Vector& newpoint)
{
	//Vector newVec = newpoint;
	//Points.Insert(num, newVec);
	Points.Insert(newpoint, num);
}

void GUIGraphLine::Add (const Vector& newpoint)
{
	//*(Points.Add()) = newpoint;
	Points.Add(newpoint);
}

int GUIGraphLine::GetCount ()
{
	return Points.Size();
}

void GUIGraphLine::Zero ()
{
	int count = Points.Size();
	for (int n = 1; n < count-1; n++)
	{
		Points.DelIndex(1);
	}

	count = Points.Size();
	for ( n = 0; n < count; n++)
	{
		Points[n].y = 0.0f;
	}
}

void GUIGraphLine::Remove (int num)
{
	int total = GetCount();
//	if (total <= 2) return;
	if (num == 0) return;
//	if (num == (total-1)) return;
	Points.DelIndex(num);
}

const Vector& GUIGraphLine::GetPoint (int num)
{
	return Points[num];
}

void GUIGraphLine::Change (int num, const Vector& newpoint)
{
	Points[num] = newpoint;
}

void GUIGraphLine::Clear ()
{
	Points.DelAll();
}

void GUIGraphLine::Copy (GUIGraphLine* src)
{
	Clear ();
	for (int n = 0; n < src->GetCount(); n++)
	{
		Vector srcpt = src->GetPoint(n);
		Add (srcpt);
	}
}

void GUIGraphLine::SetSize (DWORD count)
{
	Points.DelAll();
	for (DWORD n = 0; n < count; n++)	Points.Add();
}

#endif