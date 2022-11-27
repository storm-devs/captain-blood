#include "ForceEditorMainWindow.h"

//#include "..\common_h\gui_controls.h"

#include "..\..\common_h\FileService.h"
#include "..\..\common_h\Controls.h"

#include "..\..\System\Controls\Force.h"

extern IFileService		*storage;
extern IControlsService *controls;
extern IGUIManager		*application;
extern IRender			*render;

IForce *force = NULL;

extern string forcePath;

ForceEditorMainWindow:: ForceEditorMainWindow() : GUIWindow(NULL,0,0,0,0)
{
	const RENDERVIEWPORT &vp = render->GetViewport();

	int cx = vp.Width;
	int cy = vp.Height;

	SetWidth (cx);
	SetHeight(cy);

	bMovable = false;
	bPopupStyle = true;
	bAlwaysOnTop = true;

	graph = NEW GraphEditor(this,5,5 + 32 + 4,cx - 10,cy - 10 - 32 - 4);

	graph->HorizontalAxeDesc = "time";
	graph->VerticalAxeDesc	 = "force";

	Vector end(3.0f,0.0f,0.0f);

	graph->Lines->Add(ls = NEW GUIGraphLine(0.0f,end));
	graph->Lines->Add(rs = NEW GUIGraphLine(0.0f,end));

	graph->Lines->GetLine(1)->color = 0xff0000ff;

	graph->FitViewWidth ( 0.1f,4.0f);
	graph->FitViewHeight(-0.1f,1.1f);

	GUIButton *b;

	int xoff = 5;

	b = NEW GUIButton(this,xoff,5,32,32);
	b->Glyph->Load ("meditor\\big_new");
	b->FontColor = 0xFF000000;
	b->Hint = "Create new effect (Ctrl+N)";
	b->FlatButton = true;
	b->FlatButtonPressedColor = 0xFFFFFFFF;
	b->OnMousePressed = (CONTROL_EVENT)&ForceEditorMainWindow::InitEffect;

	xoff += 32 + 10;

	b = NEW GUIButton(this,xoff,5,32,32);
	b->Glyph->Load("meditor\\big_fileopen");
	b->FontColor = 0xFF000000;
	b->OnMousePressed = (CONTROL_EVENT)&ForceEditorMainWindow::LoadEffect;
	b->Hint = "Load effect from file (Ctrl+O)";
	b->FlatButton = true;
	b->FlatButtonPressedColor = 0xFFFFFFFF;

	xoff += 32 + 2;

	b = NEW GUIButton (this,xoff,5,32,32);
	b->Glyph->Load ("meditor\\save");
	b->FontColor = 0xFF000000;
	b->OnMousePressed = (CONTROL_EVENT)&ForceEditorMainWindow::SaveEffect;
	b->Hint = "Save effect to file (Ctrl+S)";
	b->FlatButton = true;
	b->FlatButtonPressedColor = 0xFFFFFFFF;

	xoff += 32 + 10;

	b = NEW GUIButton (this,xoff,5,32,32);
	b->Glyph->Load ("meditor\\run_game");
	b->FontColor = 0xFF000000;
	b->OnMousePressed = (CONTROL_EVENT)&ForceEditorMainWindow::PlayEffect;
	b->Hint = "Play effect (F1)";
	b->FlatButton = true;
	b->FlatButtonPressedColor = 0xFFFFFFFF;

	xoff += 32 + 13;

	caption = NEW GUILabel(this,xoff,5,400,32);
	caption->Caption = "Force editor";
	caption->Layout = GUILABELLAYOUT_Left;

	b = NEW GUIButton (this,cx - 32 - 5,5,32,32);
//	b->Glyph->Load ("close");
	b->FontColor = 0xFF000000;
	b->OnMousePressed = (CONTROL_EVENT)&ForceEditorMainWindow::Exit;
	b->Hint = "Quit the editor (Alt+F4)";
	b->FlatButton = true;
	b->FlatButtonPressedColor = 0xFFFFFFFF;

	edit = NEW GUIEdit(this,cx - 32 - 5 - 5 - 100,5,100,32);

	edit->Flat = true;
	edit->Text = "3.0";
	edit->OnAccept = (CONTROL_EVENT)&ForceEditorMainWindow::Time;

	GUILabel *label = NEW GUILabel(this,cx - 32 - 5 - 5 - 100 - 115,5,100,32);

	label->Caption = "Effect time:";
	label->Layout = GUILABELLAYOUT_Right;
}

ForceEditorMainWindow::~ForceEditorMainWindow()
{
	if( ls ) delete ls;
	if( rs ) delete rs;
}

void _cdecl ForceEditorMainWindow::InitEffect(GUIControl* sender)
{
	Vector end(3.0f,0.0f,0.0f);

	ls->Clear(); ls->Add(0.0f); ls->Add(end);
	rs->Clear(); rs->Add(0.0f); rs->Add(end);

	caption->Caption = "Force editor";
}

void _cdecl ForceEditorMainWindow::LoadEffect(GUIControl* sender)
{
	GUIFileOpen* fo = NEW GUIFileOpen();

	fo->Filters.Add(".ffe");
	fo->FiltersDesc.Add("Force feedback effect");
	fo->OnOK = (CONTROL_EVENT)&ForceEditorMainWindow::Load;
//	fo->SetDirectory(forcePath.c_str());
	Application->ShowModal(fo);
	fo->Filter->SelectItem(1);
	fo->MakeFlat(true);
}

void _cdecl ForceEditorMainWindow::Load(GUIControl* sender)
{
	GUIFileOpen* dialog = (GUIFileOpen*)sender;
	const char *name = dialog->FileName.c_str();

//	IFile *file = storage->OpenFile(dialog->FileName.c_str(),file_open_existing,_FL_);
	IFile *file;

	if( strchr(name,'.'))
	{
		file = storage->OpenFile(name,file_open_existing,_FL_);
	}
	else
	{
		string s(name); s += ".ffe";
		file = storage->OpenFile(s.c_str(),file_open_existing,_FL_);
	}
	Assert(file)

	ls->Clear();
	rs->Clear();

	Head h;

	file->Read(h,h.Size());

	Data d(h);

	file->Read(d,d.Size());

	for( int i = 0 ; i < h.ln ; i++ )
		ls->Add(d.lp[i]);

	for( int i = 0 ; i < h.rn ; i++ )
		rs->Add(d.rp[i]);

	file->Release();
//	;
	d.Release();

	char b[10]; sprintf_s(b,10,"%.1f",ls->GetPoint(ls->GetCount() - 1).x);

	edit->Text = b;

	caption->Caption = "Force editor - " + dialog->FileName;
}

void _cdecl ForceEditorMainWindow::SaveEffect(GUIControl* sender)
{
	GUIFileSave* fo = NEW GUIFileSave();

	fo->Filters.Add(".ffe");
	fo->FiltersDesc.Add("Force feedback effect");
	fo->OnOK = (CONTROL_EVENT)&ForceEditorMainWindow::Save;
//	fo->SetDirectory(forcePath.c_str());
	Application->ShowModal(fo);
	fo->Filter->SelectItem(1);
	fo->MakeFlat(true);
}

void _cdecl ForceEditorMainWindow::Save(GUIControl* sender)
{
	GUIFileSave* dialog = (GUIFileSave*)sender;
	const char *name = dialog->FileName.c_str();

//	IFile *file = storage->OpenFile(dialog->FileName.c_str(),file_create_always,_FL_);
	IFile *file;

	if( strchr(name,'.'))
	{
		file = storage->OpenFile(name,file_create_always,_FL_);
	}
	else
	{
		string s(name); s += ".ffe";
		file = storage->OpenFile(s.c_str(),file_create_always,_FL_);
	}
	Assert(file)

	int ln = ls->GetCount();
	int rn = rs->GetCount();

	Head h(ln,rn);
/*	Data d(h);

	for( int i = 0 ; i < ln ; i++ )
		d.lp[i] = ls->GetPoint(i);

	for( int i = 0 ; i < rn ; i++ )
		d.rp[i] = rs->GetPoint(i);*/

	file->Write(h,h.Size());
//	file->Write(d,d.Size());

	array<Vector> &ld = ls->GetPoints();
	array<Vector> &rd = rs->GetPoints();

	if( ld[0].x < 0.0f )
		ld[0].x = 0.0f;
	if( rd[0].x < 0.0f )
		rd[0].x = 0.0f;

	for( int i = 0 ; i < ld ; i++ )
	{
		if( ld[i].y < 0.0f )
			ld[i].y = 0.0f;
	}

	for( int i = 0 ; i < rd ; i++ )
	{
		if( rd[i].y < 0.0f )
			rd[i].y = 0.0f;
	}

	file->Write(ld.GetBuffer(),ln*sizeof(Vector));
	file->Write(rd.GetBuffer(),rn*sizeof(Vector));

	file->Release();
	
//	d.Release();

	caption->Caption = "Force editor - " + dialog->FileName;
}

void _cdecl ForceEditorMainWindow::PlayEffect(GUIControl* sender)
{
	if( force )
		force->Release();

/*	array<Vector> ld(_FL_);
	array<Vector> rd(_FL_);

	int ln = ls->GetCount();
	int rn = rs->GetCount();

	ld.Reserve(ln);
	rd.Reserve(rn);

	for( int i = 0 ; i < ln ; i++ )
		ld.Add(ls->GetPoint(i));

	for( int i = 0 ; i < rn ; i++ )
		rd.Add(rs->GetPoint(i));*/

//	force = controls->CreateForce(ld,rd);
	force = controls->CreateForce(ls->GetPoints(),rs->GetPoints());
	Assert(force)

	force->Play();
}

void ResizeLine(GUIGraphLine *line, float t)
{
	for( int i = line->GetCount() - 1 ; i >= 0 ; i-- )
	{
		const Vector &cur = line->GetPoint(i);

		if( cur.x < t )
		{
			line->Add(Vector(t,cur.y,0.0f));
			break;
		}

		if( cur.x > t )
		{
			const Vector &pre = line->GetPoint(i - 1);

			if( pre.x < t )
			{
				Vector v; v.Lerp(pre,cur,(t - pre.x)/(cur.x - pre.x)); v.x = t;

				line->Change(i,v);
				break;
			}
			else
				line->Remove(i);
		}
		else
			break;
	}
}

void _cdecl ForceEditorMainWindow::Time(GUIControl* sender)
{
	float t = (float)atof(edit->Text.c_str());

	if( t < 0.0f )
		t = -t;

	ResizeLine(ls,t);
	ResizeLine(rs,t);
}

void _cdecl ForceEditorMainWindow::Exit(GUIControl* sender)
{
	api->Exit();
}
