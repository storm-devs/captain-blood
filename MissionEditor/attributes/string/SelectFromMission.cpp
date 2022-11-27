#include "SelectFromMission.h"
#include "..\..\missioneditor.h"

#define WINDOW_WIDTH  320
#define WINDOW_HEIGHT 270
 
extern MissionEditor* sMission;
extern string my_val;
        
bool TSSFMission::FuncCompare (const string& s1, const string& s2)
{
	if (s1 < s2) return true;
	return false;
}


TSSFMission::TSSFMission (int posX, int posY) : GUIWindow (NULL, posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	bPopupStyle = true;
	bAlwaysOnTop = true;
	Caption = "Select object";
  ListBox1 = NEW GUIListBox (this, 5, 5, 300, 230);
	ListBox1->FontColor = 0xFF000000;
	ListBox1->pFont->SetName ("arialcyrsmall");
	ListBox1->Items.Clear ();
	ListBox1->MakeFlat (true);


	//btnCANCEL; 
	btnOK = NEW GUIButton (this, 5, 240, 100, 24);
	btnOK->Glyph->Load ("ok");
	btnOK->Caption = "Select";
	btnOK->FontColor = 0xFF000000;
	btnOK->pFont->SetName ("arialcyrsmall");
  btnOK->OnMousePressed = (CONTROL_EVENT)&TSSFMission::OKPressed;
	btnOK->FlatButton = true;

	btnCANCEL = NEW GUIButton (this, 116, 240, 100, 24);
	btnCANCEL->Glyph->Load ("cancel");
	btnCANCEL->Caption = "cancel";
	btnCANCEL->FontColor = 0xFF000000;
	btnCANCEL->pFont->SetName ("arialcyrsmall");
  btnCANCEL->OnMousePressed = (CONTROL_EVENT)&TSSFMission::CANCELPressed;
	btnCANCEL->FlatButton = true;




// Заполняем объектами миссии...
  int count = sMission->GetCreatedMissionObjectsCount();
	for (int n =0; n < count; n++)
	{
		MOSafePointer pMo = sMission->GetCreatedMissionObjectByIndex(n);
		ListBox1->Items.Add (pMo.Ptr()->GetObjectID ().c_str());
	}


	ListBox1->Items.Sort (FuncCompare);
}

TSSFMission::~TSSFMission ()
{
	delete btnOK;
	delete btnCANCEL;
}


void _cdecl TSSFMission::OKPressed (GUIControl* sender)
{
	
	int s_line = ListBox1->SelectedLine;;
	if (s_line >= 0)
	{
		my_val = ListBox1->Items[s_line];
	} else
		{
		 my_val = "";
		}
	Close (this);
}

void _cdecl TSSFMission::CANCELPressed (GUIControl* sender)
{
	Close (this);
}

