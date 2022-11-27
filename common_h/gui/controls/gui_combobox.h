#ifndef _XBOX
#ifndef GUI_COMBOBOX
#define GUI_COMBOBOX

#include "gui_control.h"
#include "gui_font.h"

#include "gui_events.h"
#include "gui_edit.h"
#include "gui_button.h"
#include "gui_listbox.h"

#include "..\..\Templates\string.h"

// Для навешивания обработчика приравниванием :)
#define OnChange \
t_OnChange->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


#define OnSelect \
t_OnSelect->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


class GUIComboBox : public GUIControl
{
protected:



	GUIRectangle OriginalRect;
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
public:


	bool bUserCanTypeText;

	GUIButton* Button;
	GUIEdit* Edit;
	GUIListBox* ListBox;

	
	GUIComboBox (GUIControl* pParent, int Left, int Top, int Width, int Height, GUIButton * userButton = null, GUIEdit * userEdit = null, GUIListBox * userList = null);
	
	virtual ~GUIComboBox ();
	
	virtual void Draw ();

	void _cdecl ButtonPressed (GUIControl* sender);

	void _cdecl ChangeElement (GUIControl* sender);

	GUIEventHandler* t_OnChange;  

	GUIEventHandler* t_OnSelect;  

	void _cdecl OnContentsChange (GUIControl* sender);

	virtual void SelectItem (int num);

	virtual void MakeFlat (bool Flat);


	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIComboBox") == 0) return true;
		return GUIControl::Is (className);
	}

};



#endif

#endif