#ifndef _XBOX
#include "gui_savedialog.h"


GUIFileSave::GUIFileSave ()
{
	Caption = "Save file";
	btnOK->Caption = " Save ";
}

GUIFileSave::~GUIFileSave ()
{
}

void GUIFileSave::OnCreate ()
{
	GUIFileOpen::OnCreate ();
}

void _cdecl GUIFileSave::btnOKPressed ()
{
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	
	// Если такой файл есть...
	if (fs->IsExist(FileName.GetBuffer()))
	{
		GUIMessageBox* m_box = Application->MessageBox ("Replace existing file ?", "Warning", GUIMB_OKCANCEL, btnOK->FlatButton);
		m_box->bHotKeys = false;
		m_box->OnOK = (CONTROL_EVENT)&GUIFileSave::ReplaceOK;
		m_box->OnCancel = (CONTROL_EVENT)&GUIFileSave::ReplaceCancel;
		return;
	}

	GUIFileOpen::btnOKPressed ();
}

void _cdecl GUIFileSave::ReplaceOK ()
{
	GUIFileOpen::btnOKPressed ();
}

void _cdecl GUIFileSave::ReplaceCancel ()
{

}

#endif