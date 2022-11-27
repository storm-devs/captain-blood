#ifndef _XBOX

#include "fe_linelist.h"

//	GUIList<GUIGraphLine*> Lines;

bool Compare (GUIGraphLine* const& line1, GUIGraphLine* const& line2)
{
	if (line1->bActive) return false;
	if (line2->bActive) return true;
	return false;
}

GUILineList::GUILineList () : Lines (_FL_, 64)
{
}
	
GUILineList::~GUILineList ()
{
}

void GUILineList::Add (GUIGraphLine* line)
{
	Lines.Add(line);
}

int GUILineList::GetCount ()
{
	return Lines.Size();
}

void GUILineList::Remove (int num)
{
	Lines.DelIndex(num);
}

GUIGraphLine* GUILineList::GetLine (int num)
{
	return Lines[num];
}


void GUILineList::SortByActive ()
{
	Lines.QSort(Compare);
}


#endif