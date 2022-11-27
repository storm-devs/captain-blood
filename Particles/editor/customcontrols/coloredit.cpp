#ifndef _XBOX

#include "coloredit.h"
#include "strutil.h"

#define RAMKACOLOR 0xFF000000

GUIColorEditor::GUIColorEditor (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIPanel (pParent, Left, Top, Width, Height), Points(_FL_)
{
	mbBorned = false;
	pFont = NEW GUIFont ("arialcyrsmall");
	EditedLowest = false;
	EditedIndex = -1;
	DragedIndex = -1;
/*
	ColorPoint cPoint;
	cPoint.pos = 0.0f;
	cPoint.max = Color (0xFFFFFFFFL);
	cPoint.min = Color (0xFFFFFFFFL);
	Points.Add(cPoint);

	cPoint.pos = 1.0f;
	cPoint.max = Color (0xFFFFFFFFL);
	cPoint.min = Color (0xFFFFFFFFL);
	Points.Add(cPoint);
*/
}

GUIColorEditor::~GUIColorEditor ()
{
	if (pFont) delete pFont;
	pFont = NULL;
}

void _cdecl GUIColorEditor::OnClearYes (GUIControl* sender)
{
	Points.DelAll();

	ColorPoint pt;
	pt.max = Color (1.0f, 1.0f, 1.0f, 1.0f);
	pt.min = Color (1.0f, 1.0f, 1.0f, 1.0f);
	pt.pos = 0.0f;
	Points.Add(pt);
	pt.pos = 1.0f;
	Points.Add(pt);
	mbBorned = false;
}

void _cdecl GUIColorEditor::OnClearNo (GUIControl* sender)
{
	mbBorned = false;
}


void GUIColorEditor::Draw ()
{
	GUIRectangle rect = GetScreenRect();


	if ((GetAsyncKeyState(VK_DELETE) < 0) && (!mbBorned))
	{
		GUIMessageBox* mb = GetMainControl()->Application->MessageBox("Clear color graph ?", "Warning", GUIMB_YESNO);
		mb->OnOK = (CONTROL_EVENT)&GUIColorEditor::OnClearYes;
		mb->OnCancel = (CONTROL_EVENT)&GUIColorEditor::OnClearNo;
		mbBorned = true;
	}

	//GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, 0xFFFFFFFF);

	for (DWORD n = 0; n < Points.Size()-1; n++)
	{
		int FromX = (int)((float)(rect.w) * Points[n].pos);
		int ToX = (int)((float)(rect.w) * Points[n+1].pos);
		int Width = ToX - FromX;
		DWORD c1 = Points[n].max.GetDword();
		DWORD c2 = Points[n].min.GetDword();
		DWORD c3 = Points[n+1].max.GetDword();
		DWORD c4 = Points[n+1].min.GetDword();

		GUIHelper::DrawColorRect(rect.x + FromX, rect.y, Width, rect.h, c3, c1, c4, c2);
	}

	//GUIPanel::Draw();

	for (n = 0; n < Points.Size(); n++)
	{
		int FromX = (int)((float)(rect.w-6) * Points[n].pos);

		if (n == DragedIndex)
		{
			pFont->Print(rect.x + FromX-10, rect.y-pFont->GetHeight()-2, 0xFF000000, "%3.2f%%", Points[n].pos*100.0f);
		}

		GUIHelper::Draw2DRect(rect.x + FromX-1, rect.y-1, 8, 8, 0xFFFFFFFF);
		GUIHelper::Draw2DRect(rect.x + FromX, rect.y, 6, 6, RAMKACOLOR);
		GUIHelper::Draw2DRect(rect.x + FromX+1, rect.y+1, 4, 4, Points[n].max.GetDword());
		
		GUIHelper::Draw2DRect(rect.x + FromX-1, rect.y+rect.h-7, 8, 8, 0xFFFFFFFF);
		GUIHelper::Draw2DRect(rect.x + FromX, rect.y+rect.h-6, 6, 6, RAMKACOLOR);
		GUIHelper::Draw2DRect(rect.x + FromX+1, rect.y+rect.h-5, 4, 4, Points[n].min.GetDword());
	}

}

bool GUIColorEditor::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (message == GUIMSG_LMB_DOWN)
	{
		MouseLMBPressed ();
		return true;
	}

	if (message == GUIMSG_LMB_DBLCLICK)
	{
		MouseLMBDBLClick ();
		return true;
	}

	return GUIPanel::ProcessMessages(message, lparam, hparam);
}

void GUIColorEditor::MouseLMBPressed ()
{
	bool Temp;
	GUIPoint mouse_pos = Application->GetCursor()->GetPosition();
	int PointIndex = GetPointIndex (mouse_pos, Temp);

	if (PointIndex == -1) return;

	if (GetAsyncKeyState(VK_CONTROL) < 0)
	{
		//Крайние не удалять
		if ((PointIndex > 0) && (PointIndex < (int)Points.Size() - 1))
		{
			Points.Extract(PointIndex);
			return;
		}
	}

	if ((PointIndex > 0) && (PointIndex < (int)Points.Size() - 1))
	{
		DragedIndex = PointIndex;	
		mouse_hotspot = mouse_pos;
	} else
	{
		DragedIndex = -1;
	}


}

void GUIColorEditor::MouseLMBDBLClick ()
{
	GUIPoint mouse_pos = Application->GetCursor()->GetPosition();
  EditedLowest = false;
	EditedIndex = GetPointIndex (mouse_pos, EditedLowest);

	if (EditedIndex >= 0)
	{
		GUIColorPicker* clr_picker = NEW GUIColorPicker (100, 100, "Select color", 0, 0);
		Color edited_color = Points[EditedIndex].max;
		if (EditedLowest) edited_color = Points[EditedIndex].min;
		clr_picker->editA->Text = IntToStr (255);
		clr_picker->editR->Text = IntToStr ((int)(edited_color.r * 255.0f));
		clr_picker->editG->Text = IntToStr ((int)(edited_color.g * 255.0f));
		clr_picker->editB->Text = IntToStr ((int)(edited_color.b * 255.0f));
		clr_picker->TextIsChanged (NULL);

		clr_picker->OnApply = (CONTROL_EVENT)&GUIColorEditor::ColorIsChange;
		Application->ShowModal(clr_picker);
		return;
	}


	GUIRectangle rect = GetScreenRect();

	int len = mouse_pos.x - rect.x;
	float pos = (float)len / (float)(rect.w - 6);

	for (DWORD n = 0; n < Points.Size() - 1; n++)
	{
		if (Points[n].pos < pos && Points[n+1].pos > pos)
		{
			NewIndex = n+1;;
			NewPos = pos;

			GUIColorPicker* clr_picker = NEW GUIColorPicker (100, 100, "Select color for new point", 0, 0);
			clr_picker->OnApply = (CONTROL_EVENT)&GUIColorEditor::CreateNewPoint;
			Application->ShowModal(clr_picker);


			break;
		}
	}

}

int GUIColorEditor::GetPointIndex (const GUIPoint& point, bool& minpointpressed)
{
	GUIPoint pt = point;
	//ScreenToClient(pt);
	GUIRectangle rect = GetScreenRect();
	for (DWORD n = 0; n < Points.Size(); n++)
	{
		int FromX = (int)((float)(rect.w-6) * Points[n].pos);

		GUIRectangle color_point;
		color_point.x = rect.x + FromX-8;
		color_point.y = rect.y-8;
		color_point.h = 16;
		color_point.w = 16;

		if (color_point.Inside(pt))
		{
			minpointpressed = false;
			return n;
		}

		color_point.y = rect.y+rect.h-6;

		if (color_point.Inside(pt))
		{
			minpointpressed = true;
			return n;
		}
	}

	return -1;
}


void _cdecl GUIColorEditor::ColorIsChange (GUIControl* sender)
{
	GUIColorPicker* picker = (GUIColorPicker*)sender;
	Color newColor(picker->SelectedColor);

	if (EditedLowest) 
		Points[EditedIndex].min = newColor;
	else
		Points[EditedIndex].max = newColor;

}

void _cdecl GUIColorEditor::CreateNewPoint (GUIControl* sender)
{
	GUIColorPicker* picker = (GUIColorPicker*)sender;
	Color newColor(picker->SelectedColor);

	ColorPoint cPoint;
	cPoint.pos = NewPos;
	cPoint.max = newColor;
	cPoint.min = newColor;
	Points.Insert(cPoint, NewIndex);


}


void GUIColorEditor::MouseUp (int button, const GUIPoint& pt)
{
	DragedIndex = -1;
}

void GUIColorEditor::MouseMove (int button, const GUIPoint& pt)
{
	if (DragedIndex < 0) return;

	GUIRectangle rect = GetScreenRect();
	float one_percent = (float)(rect.w - 6) / 100.0f;
	GUIPoint mouse_pos = Application->GetCursor()->GetPosition();
	float delta = (float)mouse_hotspot.x - (float)mouse_pos.x;
	float percentmoved = delta / one_percent;

	float oldpos = Points[DragedIndex].pos;
	Points[DragedIndex].pos -= (percentmoved * 0.01f); 

	if (Points[DragedIndex].pos <= Points[DragedIndex-1].pos + 0.008f)
		Points[DragedIndex].pos = oldpos;

	if (Points[DragedIndex].pos >= Points[DragedIndex+1].pos - 0.008f)
		Points[DragedIndex].pos = oldpos;

	mouse_hotspot = mouse_pos;
	
}

#endif