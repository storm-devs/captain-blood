//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// LoadModelForm	
//============================================================================================
			

#include "LoadModelForm.h"
#include "..\AnxProject.h"


//============================================================================================

LoadModelForm::LoadModelForm(AnxOptions & options) : opt(options), GUIWindow(null, 0, 0, 600, 600)
{
	dfo = null;
	AnxToolButton * b;
	//Устанавливаем окно поверх
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	SetScreenCenter();
	//Расставляем элементы
	//Подпись
	GUILabel * lb = NEW GUILabel(this, 10, 10, DrawRect.w - 20, 28);
	lb->Caption = "Model name for viewer";
	lb->Layout = GUILABELLAYOUT_Left;
	editName = NEW AnxToolEdit(this, 10, 40, DrawRect.w - 120, 28);
	editName->Text = opt.project->preveiwModel;
	editName->OnEditUpdate = (CONTROL_EVENT)&LoadModelForm::EvtReloadModel;
	//Просмоторщик
	viewer = NEW AnxAnimationViewer(opt, this, 10, 80, DrawRect.w - 20, 450);	
	//Кнопки
	b = NEW AnxToolButton(null, this, DrawRect.w - 100, 40, 90, 28);
	b->Caption = "Browse";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&LoadModelForm::EvtBrowse;
	long bw = 120;
	long bh = 32;
	long bx = (DrawRect.w - bw*2 - 20)/2;
	long by = DrawRect.h - bh - 10;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Ok";
	b->Glyph->Load("AnxEditor\\AnxOk");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&LoadModelForm::EvtOk;
	bx += bw + 20;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Cancel";
	b->Glyph->Load("AnxEditor\\AnxCancel");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&LoadModelForm::EvtCancel;
}

LoadModelForm::~LoadModelForm()
{
}

void LoadModelForm::OnCreate()
{
	editName->SetFocus();
	editName->SelectText(0, editName->Text.Len());
}

//============================================================================================

//Рисование
void LoadModelForm::Draw()
{
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем панель
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xefe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	GUIControl::Draw();
}

void _cdecl LoadModelForm::EvtOk(GUIControl * sender)
{
	opt.project->preveiwModel = editName->Text;
	Application->Close(this);
}

void _cdecl LoadModelForm::EvtCancel(GUIControl * sender)
{
	Application->Close(this);
}

void _cdecl LoadModelForm::EvtBrowse(GUIControl * sender)
{
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".gmx");
	dfo->FiltersDesc.Add("Geometry file");
	dfo->RefreshFilters();
	dfo->Caption = "Select new model for viewer";
	dfo->StartDirectory = opt.path.gmx;
	dfo->bShowReadOnly = true;
	dfo->OnOK = (CONTROL_EVENT)&LoadModelForm::EvtBrowseOk;
	dfo->OnCancel = (CONTROL_EVENT)&LoadModelForm::EvtBrowseCancel;
	opt.gui_manager->ShowModal(dfo);
}

void _cdecl LoadModelForm::EvtBrowseOk(GUIControl * sender)
{
	Assert(dfo);
	editName->Text = dfo->FileName;
	char * buf = NEW char[1024];
	GetCurrentDirectory(1024, buf); buf[1023] = 0;
	string path = buf;	
	delete buf; buf = null;
	path += "\\resource\\models\\";
	editName->Text.GetRelativePath(path);	
	editName->Text.Lower().AddExtention(".gmx");
	editName->Text.Delete(editName->Text.Len() - 4, 4);
	string gmxpath;
	if(opt.path.gmx != gmxpath.GetFilePath(dfo->FileName))
	{
		opt.path.gmx = gmxpath;
		opt.UpdatePath();
	}
	EvtReloadModel(sender);
	editName->SetFocus();
	editName->SelectText(0, editName->Text.Len());
	dfo = null;
}

void _cdecl LoadModelForm::EvtBrowseCancel(GUIControl * sender)
{
	dfo = null;
}

void _cdecl LoadModelForm::EvtReloadModel(GUIControl * sender)
{
	string mdl = opt.project->preveiwModel;
	opt.project->preveiwModel = editName->Text;
	viewer->SetAnimation(null, 0);
	opt.project->preveiwModel = mdl;
}





