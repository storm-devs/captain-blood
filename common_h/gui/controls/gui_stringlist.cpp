#ifndef _XBOX
#include "gui_stringlist.h"


GUIStringList * GUIStringList::listInSort = null;
GUIStringList::CompareFunc GUIStringList::compareFunc = null;

GUIStringList::GUIStringList () : Strings (_FL_, 64),
                                 StringsPull (_FL_, 64)
{
	GlobalDelete = false;
	t_OnClear = NEW GUIEventHandler;  
	t_OnAdd = NEW GUIEventHandler;  
	Clear ();
}

GUIStringList::~GUIStringList ()
{
	GlobalDelete = true;
	Clear ();
  delete t_OnAdd;
	delete t_OnClear;
}

int GUIStringList::Add (const string& s)
{
	StringsPull.Add(s);
	Strings.Add (StringsPull.Last());	
	t_OnAdd->Execute (NULL);
	return (Strings.Size ()-1);
}

void GUIStringList::Clear ()
{
	if (!GlobalDelete) t_OnClear->Execute(NULL);
	Strings.DelAll ();
	StringsPull.DelAll ();
}

int GUIStringList::Size () const
{
	return Strings.Size ();
}

const string& GUIStringList::GetConst (int index)
{
	return StringsPull[Strings[index]];
}

string& GUIStringList::Get(int index)
{
	return StringsPull[Strings[index]];
}

void GUIStringList::Insert (int index, string& s)
{
	dword idx = StringsPull.Add(s);
	Strings.Insert(idx, index);
}

void GUIStringList::Delete (int index)
{
	Assert(Strings.Size() == StringsPull.Size());
	dword idx = Strings[index];
	dword last = StringsPull.Last();
	if(idx != last)
	{
		for(long i = last; i >= 0; i--)
		{
			if(Strings[i] == last)
			{
				Strings[i] = idx;
				break;
			}
		}
		StringsPull[idx] = StringsPull[StringsPull.Last()];
	}
	Strings.DelIndex (index);
	StringsPull.DelIndex(StringsPull.Last());
}

string& GUIStringList::operator[] (int index)
{
	return Get (index);
}


void GUIStringList::Sort (bool (*CompareFunc) (const string& s1, const string& s2))
{
	AssertCoreThread	
	Assert(!listInSort);
	listInSort = this;
	compareFunc = CompareFunc;
	Strings.QSort (&SystemCompare);
	compareFunc = null;
	listInSort = null;
}

bool GUIStringList::SystemCompare(const long& i1, const long& i2)
{
	const string& s1 = listInSort->StringsPull[i1];
	const string& s2 = listInSort->StringsPull[i2];
	return compareFunc(s1, s2);
}

bool GUIStringList::Compare_s1_less_s2(const string& s1, const string& s2)
{
	return s1 < s2;
}

bool GUIStringList::Compare_s1_lessequal_s2(const string& s1, const string& s2)
{
	return s1 <= s2;
}

bool GUIStringList::Compare_s2_less_s1(const string& s1, const string& s2)
{
	return s2 < s1;
}

bool GUIStringList::Compare_s2_lessequal_s1(const string& s1, const string& s2)
{
	return s2 <= s1;
}



#endif