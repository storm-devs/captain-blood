#ifndef _XBOX
#ifndef FILEOPEN_DIALOG
#define FILEOPEN_DIALOG

#include "gui_window.h"


extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnOK \
t_OnOK->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

#define OnChangeExtension \
t_OnChangeExtension->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)




// Для навешивания обработчика приравниванием :)
#define OnCancel \
t_OnCancel->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


class GUIFileOpen : public GUIWindow
{

string StartFromDrive;
string CurrentDrive;


GUIStringList CurrentDrivesPaths;


public:

	string StartDirectory;
	string FileName;

	bool bShowReadOnly;

	GUIStringList Filters;
	GUIStringList FiltersDesc;

	GUIButton* btnOK; 
	GUIButton* btnCancel; 

	GUIEdit* edtFileName;

	GUIComboBox* Filter;

	GUITreeView* DirList;

	GUIListBox* Files;

	GUIComboBox* Drives;
	
	GUIFileOpen ();
	~GUIFileOpen ();

	virtual void _cdecl DriveIsChange ();

	virtual void _cdecl TreeExpand ();
	virtual void _cdecl TreeColapse ();

	virtual void _cdecl TreeSelectionChange ();

	virtual void _cdecl FileSelectionChange ();

	

	virtual void RefreshDirs (const string& str);


	virtual void GetFilesAndDirs (string& path, GUIStringList& files, GUIStringList& dirs, const char* Mask="*.*", bool ExitOnFolder = false);

	virtual void GetFullNodeProcess (GUITreeNode* node);
	virtual const char* GetFullNodeName (GUITreeNode* node);


	virtual const char* GetCurrentMask ();

	virtual void _cdecl SetCurrentdirectory (const char* dir = NULL);

	virtual void _cdecl btnOKPressed ();
	virtual void _cdecl btnCancelPressed ();

	// Путь должен быть корректным не иметь лишних "\\"
	// И заканчиваться "\\"
	virtual void SetDirectory (const char* FullPath);

	virtual void _cdecl WhenFindItem (GUIControl* item);


	virtual void _cdecl filenameChanged (GUIControl* item);


	virtual void _cdecl FilterChanged (GUIControl* item);

	virtual void _cdecl onBeforeHintShow (GUIControl* item);
	virtual void _cdecl onDirBeforeHintShow (GUIControl* item);

	


	virtual int GetDriveNum (const char* Drive);

	virtual void RefreshFilters ();

	GUIEventHandler* t_OnOK;

	GUIEventHandler* t_OnCancel;


	GUIEventHandler* t_OnChangeExtension;


	virtual void OnCreate ();


	virtual void MakeFlat (bool Flat);


	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	virtual void KeyPressed(int Key, bool bSysKey);


	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIFileOpen") == 0) return true;
		return GUIWindow::Is (className);
	}

 
};



#endif

#endif