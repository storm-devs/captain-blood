#ifndef _XBOX
#ifndef GUI_STRINGLIST
#define GUI_STRINGLIST

#include "..\..\Templates\string.h"



extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnAdd \
t_OnAdd->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

#define OnClear \
t_OnClear->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)



class GUIStringList
{
	bool GlobalDelete;
	
	array<long> Strings;
	array<string> StringsPull;

	typedef bool (*CompareFunc) (const string& s1, const string& s2);

	static GUIStringList * listInSort;
	static CompareFunc compareFunc;

	static bool SystemCompare(const long& i1, const long& i2);

public:
	static bool Compare_s1_less_s2(const string& s1, const string& s2);
	static bool Compare_s1_lessequal_s2(const string& s1, const string& s2);
	static bool Compare_s2_less_s1(const string& s1, const string& s2);
	static bool Compare_s2_lessequal_s1(const string& s1, const string& s2);

public:
	
	GUIStringList ();
	~GUIStringList ();
	
	int Add (const string& s);
	
	void Clear ();
	
	int Size () const;
	
	string& Get(int index);
	
	void Insert (int index, string& s);
	
	void Delete (int index);
	
	string& operator[] (int index);
	const string& GetConst (int index);
	
	
	GUIEventHandler* t_OnAdd;  
	GUIEventHandler* t_OnClear;  


	void Sort (bool (*CompareFunc) (const string& s1, const string& s2));
};



#endif

#endif