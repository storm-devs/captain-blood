//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// AnxEditor
//===========================================================================================================================
// AnxEditorToolBar	
//============================================================================================

#include "AnxEditorToolBar.h"

#include "..\Commands\AnxCmd_CreateNode.h"
#include "..\Commands\AnxCmd_DelSelectedNodes.h"
#include "..\Commands\AnxCmd_Undo.h"
#include "..\Commands\AnxCmd_Redo.h"
#include "..\Commands\AnxCmd_Copy.h"
#include "..\Commands\AnxCmd_Paste.h"
#include "..\Commands\AnxCmd_Import.h"
#include "..\Commands\AnxCmd_GotoNode.h"
#include "..\Commands\AnxCmd_OpenNodesList.h"
#include "..\Commands\AnxCmd_ExecuteEvent.h"

#include "..\AntFile.h"
#include "..\AnxExporter.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxMenu.h"
#include "..\EditorControls\AnxToolListBox.h"

#include "LoadModelForm.h"
#include "AnxExportLog.h"

//============================================================================================

AnxEditorToolBar::AnxEditorToolBar(AnxOptions & options) : GUIWindow(null, 0, 0, 750, 40), opt(options)
{
	dfo = null;
	dfs = null;
	editHNodeName = null;
	isRecursiveAntsReload = false;
	//Настраиваем окно
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	//Заводим контролы окна
	dword start = 4;
	//Метка с названием текущего редактируемого нода
	label = NEW GUILabel(this, start, 4, 128, 32);
	label->Hint = "Current hierarchy level";
	start += 128 + 8 + 4;
	label->Caption = "Initialize";
	label->pFont->SetName(opt.node.fNameLabel);	
	label->Enabled = false;
	//Кнопка создания проекта
	AnxToolButton * button = NEW AnxToolButton(null, this, start, 4, 32, 32);
	button->Hint = "Create new project";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxNewProject");
	button->OnMousePressed = (CONTROL_EVENT)&AnxEditorToolBar::NewProject;
	button->Align = GUIAL_Center;
	//Кнопка загрузки проекта
	button = NEW AnxToolButton(null, this, start, 4, 32, 32);
	button->Hint = "Open project";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxOpenProject");
	button->OnMousePressed = (CONTROL_EVENT)&AnxEditorToolBar::OpenProject;
	button->Align = GUIAL_Center;
	//Кнопка сохранения проекта
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::SaveProject), this, start, 4, 32, 32);
	button->Hint = "Save project";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxSaveProject");
	button->Align = GUIAL_Center;
	//Кнопка сохранения проекта в новый файл
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::SaveProjectAs), this, start, 4, 32, 32);
	button->Hint = "Save project in new file";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxSaveProjectAs");
	button->Align = GUIAL_Center;
	//Кнопка экспорта проекта
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::ExportProject), this, start, 4, 32, 32);
	button->Hint = "Export project to anx file";
	start += 32 + 8 + 4;
	button->Glyph->Load("AnxEditor\\AnxExportProject");
	//Кнопка импорта проекта
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::ImportProject), this, start, 4, 32, 32);
	button->Hint = "Import into current project other project file";
	start += 32 + 8 + 4;
	button->Glyph->Load("AnxEditor\\AnxImportProject");
	//Кнопка импорта событий
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::ImportEvents), this, start, 4, 32, 32);
	button->Hint = "Import sound events to current project";
	start += 32 + 8 + 4;
	button->Glyph->Load("AnxEditor\\AnxImportEvents");
	//Кнопка импорта ролика
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::ImportMovie), this, start, 4, 32, 32);
	button->Hint = "Import movie to current project";
	start += 32 + 8 + 4;
	button->Glyph->Load("AnxEditor\\AnxImportMovie");
	//Кнопка импорта ролика
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::ReloadAnts), this, start, 4, 32, 32);
	button->Hint = "Reload animation clips from select folder for current level nodes";
	start += 32 + 8 + 4;
	button->Glyph->Load("AnxEditor\\AnxReloadAnts");
	//Кнопка установки мастер-скелета
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::SetMasterSkeleton), this, start, 4, 32, 32);
	button->Hint = "Set new master skeleton into project";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxMasterSkeleton");
	//button->Align = GUIAL_Center;
	//Кнопка установки модельки предпросмотра
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::SetViewerModel), this, start, 4, 32, 32);
	button->Hint = "Set new viewer model to project";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxViewerModel");
/*	//Кнопка установки менеджера партиклов
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::SetParticles), this, start, 4, 32, 32);
	button->Hint = "Set particles for preview particle's events in viewer";
	start += 32 + 8 + 4;
	button->Glyph->Load("AnxEditor\\AnxParticles");
*/

	/*
	//!!! Max -> Хранилище клипов не умеет работать с Undo-redo
	//Кнопка отмены действия
	button = NEW AnxToolButton(NEW AnxCmd_Undo(opt), this, start, 4, 32, 32);
	button->Hint = "Undo";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxUndo");
	//Кнопка повтора действия
	button = NEW AnxToolButton(NEW AnxCmd_Redo(opt), this, start, 4, 32, 32);
	button->Hint = "Redo";
	start += 32 + 8 + 4;
	button->Glyph->Load("AnxEditor\\AnxRedo");
	*/
	//Кнопка копирования в буфер обмена
	button = NEW AnxToolButton(NEW AnxCmd_Copy(opt), this, start, 4, 32, 32);
	button->Hint = "Copy selected nodes with connected links";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxCopy");
	//Кнопка востонавления с буфера обмена
	button = NEW AnxToolButton(NEW AnxCmd_Paste(opt), this, start, 4, 32, 32);
	button->Hint = "Paste selected nodes with connected links";
	start += 32 + 8 + 4;
	button->Glyph->Load("AnxEditor\\AnxPaste");
	//Кнопка создания нода
	button = NEW AnxToolButton(NEW AnxCmd_ExecuteEvent(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::ShowCreateNodesMenu), this, start, 4, 32, 32);
	button->Hint = "Create new node";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxCreateNode");
	buttonCreateNodes = button;
	//Кнопка удаления выделенных нодов
	button = NEW AnxToolButton(NEW AnxCmd_DelSelectedNodes(opt), this, start, 4, 32, 32);
	button->Hint = "Delete node or link";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxDelete");
	//Кнопка быстрого перехода к ноду
	button = NEW AnxToolButton(NEW AnxCmd_OpenNodesList(opt, this, (CONTROL_EVENT)&AnxEditorToolBar::ShowGotoNodesList), this, start, 4, 32, 32);
	button->Hint = "Go to node by name";
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxGotoNode");
	buttonGotoNode = button;
	DrawRect.w = ClientRect.w = start;
	//Кнопка выхода из редактора
	start += 16;
	button = NEW AnxToolButton(null, this, start, 4, 32, 32);
	button->Hint = "Exit from editor without saving project";
	button->OnMousePressed = (CONTROL_EVENT)&AnxEditorToolBar::EvtExitFromEditor;
	start += 32 + 8;
	button->Glyph->Load("AnxEditor\\AnxExit");
	DrawRect.w = ClientRect.w = start;
	saveViewTime = 0.0f;
}

AnxEditorToolBar::~AnxEditorToolBar()
{
}

//============================================================================================

//Рисование
void AnxEditorToolBar::Draw()
{
	if(opt.IsShowInfo())
	{
		return;
	}
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем панель
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0x8fe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	rect = label->GetScreenRect();
	ClientToScreen(rect);
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xcfe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0x8f000000);
	GUIControl::Draw();
	//Обновляем состояние контролов
	label->Caption = opt.project ? opt.project->graph.current->name : "Project not created";
	//Рисуем сообщение об сохранении
	if(saveViewTime > 0.0f)
	{
		long w = 300;
		long h = 100;
		long x = (opt.width - w)/2;
		long y = (opt.height - h)/2;
		Color ac(1.0f);
		if(saveViewTime > 0.9f)
		{
			ac.a = 1.0f - (saveViewTime - 0.9f)/0.1f;
		}
		if(saveViewTime < 0.2f)
		{
			ac.a = saveViewTime/0.2f;
		}
		GUIHelper::Draw2DRectAlpha(x + 4, y + 4, w, h, (ac*Color((dword)0x1f000000)));
		GUIHelper::Draw2DRectAlpha(x, y, w, h, (ac*Color((dword)0xcfe0e0f0)));
		GUIHelper::DrawLinesBox(x, y, w, h, (ac*Color((dword)0x8f000000)));
		if(opt.fontBig)
		{
			const char * c = "Project saved";
			x = (opt.width - opt.fontBig->GetWidth(c))/2;
			y = (opt.height - opt.fontBig->GetHeight())/2;
			opt.fontBig->Print(x, y, (ac*Color((dword)0xff000000)), c);
		}
		saveViewTime -= api->GetDeltaTime()*2.0f;		
	}
}

//Сообщения
bool AnxEditorToolBar::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	switch(message)
	{
	case GUIMSG_LMB_DBLCLICK:
		if(opt.project && opt.project->graph.current->parent)
		{
			GUIPoint cursor_pos;
			GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
			GUIRectangle rect = label->GetDrawRect();
			label->ClientToScreen(rect);
			if(rect.Inside(cursor_pos))
			{
				if(!editHNodeName)
				{
					GUIWindow * wnd = NEW GUIWindow(this, rect.x + 4, rect.y + (rect.h - 24)/2, 345, 24);
					wnd->bAlwaysOnTop = true;
					wnd->bPopupStyle = true;
					wnd->UpdatePopupStyle();
					editHNodeName = NEW AnxToolEdit(wnd, 0, 0, wnd->GetDrawRect().w, wnd->GetDrawRect().h);
					editHNodeName->Hint = "Input new name for current hierarchy node";
					opt.gui_manager->ShowModal(wnd);
				}
				editHNodeName->Text = opt.project->graph.current->name;				
				editHNodeName->OnEditUpdate = (CONTROL_EVENT)&AnxEditorToolBar::UpdateHNodeName;
				editHNodeName->SetFocus();
				editHNodeName->SelectText(0, editHNodeName->Text.Len());
			}
			return true;
		}
		break;
	}
	return GUIWindow::ProcessMessages(message, lparam, hparam);
}

void _cdecl AnxEditorToolBar::UpdateHNodeName(GUIControl * sender)
{
	Assert(opt.project);
	Assert(opt.project->graph.current->parent);
	Assert(editHNodeName);
	if(opt.project->CheckUniqueNodeName(editHNodeName->Text))
	{
		opt.project->graph.current->name = editHNodeName->Text;
	}
	opt.gui_manager->Close((GUIWindow *)editHNodeName->GetParent());
	editHNodeName = null;
}

//============================================================================================

//Создать новый проект
void _cdecl AnxEditorToolBar::NewProject(GUIControl * sender)
{
	if(opt.project)
	{
		opt.gui_manager->MessageBox("Create new project?", "To be or not to be...", GUIMB_YESNO)->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::NewProjectCreateYes;
	}else{
		opt.project = NEW AnxProject(opt);
		SetMasterSkeleton(this);
	}
}

void _cdecl AnxEditorToolBar::NewProjectCreateYes(GUIControl * sender)
{
	SaveProject(sender);
	if(!dfs)
	{
		NewProjectSaveCancel(this);
	}else{
		dfs->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::NewProjectSaveOk;
		dfs->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::NewProjectSaveCancel;
	}
}

void _cdecl AnxEditorToolBar::NewProjectSaveOk(GUIControl * sender)
{
	SaveProject(dfs->FileName);
	dfs = null;
	NewProjectSaveCancel(sender);
}

void _cdecl AnxEditorToolBar::NewProjectSaveCancel(GUIControl * sender)
{
	dfs = null;
	//Удаляем проект и заводим новый
	delete opt.project;
	opt.project = null;
	opt.project = NEW AnxProject(opt);
	SetMasterSkeleton(this);
}

//Загрузить проект
void _cdecl AnxEditorToolBar::OpenProject(GUIControl * sender)
{
	if(opt.project)
	{
		//SaveProject(sender);
		if(!dfs)
		{
			OpenProjectProcessDialog(this);
		}else{
			dfs->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::OpenProjectSaveCurrent;
			dfs->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::OpenProjectProcessDialog;
		}		
	}else{
		OpenProjectProcessDialog(this);
	}
}

void _cdecl AnxEditorToolBar::OpenProjectSaveCurrent(GUIControl * sender)
{
	SaveProject(dfs->FileName);
	dfs = null;
	OpenProjectProcessDialog(this);
}

void _cdecl AnxEditorToolBar::OpenProjectProcessDialog(GUIControl * sender)
{
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".anp");
	dfo->FiltersDesc.Add("Animation project file");
	dfo->RefreshFilters();
	dfo->Caption = "Select animation project file for open it";
	dfo->StartDirectory = opt.path.project;
	dfo->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::OpenProjectProcess;
	dfo->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::OpenProjectCancel;
	opt.gui_manager->ShowModal(dfo);
}

void _cdecl AnxEditorToolBar::OpenProjectProcess(GUIControl * sender)
{
	//Формируем имя файла
	Assert(dfo);
	string file = dfo->FileName;
	file.Lower().AddExtention(".anp");
	string pjpath;
	if(opt.path.project != pjpath.GetFilePath(file))
	{
		opt.path.project = pjpath;
		opt.UpdatePath();
	}
	//Открываем файл
	string error;
	AnxStreamFile stream(false);
	if(!stream.Open(file))
	{
		error = "File not opened:\n    ";
		error += file;
		opt.gui_manager->MessageBox(error, "Open file error", GUIMB_OK);
		return;
	}
	//Пересоздаём проект
	if(opt.project) delete opt.project;
	opt.project = null;
	opt.project = NEW AnxProject(opt);
	//Загружаем проект
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(stream.ReadID() != ' PNA') throw "Invalidate file id";
		opt.project->Read(stream);
		opt.project->path = file;
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		if(!err) err = "";
		error = "File not opened:\n    ";
		error += file;
		error += " \nError:\n    ";
		error += err;
		opt.gui_manager->MessageBox(error, "Read file error", GUIMB_OK);
		delete opt.project;
		opt.project = null;
	}catch(...){
		error = "File not opened:\n    ";
		error += file;
		opt.gui_manager->MessageBox(error, "Read file error", GUIMB_OK);
		delete opt.project;
		opt.project = null;
	}
#endif
}

void _cdecl AnxEditorToolBar::OpenProjectCancel(GUIControl * sender)
{
	dfo = null;
}

//Сохранить проект
void _cdecl AnxEditorToolBar::SaveProject(GUIControl * sender)
{
	Assert(opt.project);
	dfs = null;
	if(opt.project->path.Len())
	{
		SaveProject(opt.project->path);
	}else{
		SaveProjectAs(this);
	}
}

void _cdecl AnxEditorToolBar::SaveProjectAs(GUIControl * sender)
{
	dfs = NEW GUIFileSave();
	dfs->Filters.Clear();
	dfs->Filters.Add(".anp");
	dfs->FiltersDesc.Add("Animation project file");
	dfs->RefreshFilters();
	dfs->Caption = "Select project file name for save it";
	dfs->StartDirectory = opt.path.project;
	dfs->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::SaveProjectOk;
	dfs->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::SaveProjectCancel;
	opt.gui_manager->ShowModal(dfs);
}

void _cdecl AnxEditorToolBar::SaveProjectOk(GUIControl * sender)
{
	SaveProject(dfs->FileName);
	dfs = null;
}

void AnxEditorToolBar::SaveProject(const char * name)
{
	//Имя файла
	string file = name;
	file.Lower().AddExtention(".anp");
	string pjpath;
	if(opt.path.project != pjpath.GetFilePath(file))
	{
		opt.path.project = pjpath;
		opt.UpdatePath();
	}
	//Переименовываем старый файл проекта
	string backFile = file;
	backFile.AddExtention(".bak");
	DeleteFile(backFile);
	MoveFile(file, backFile);
	//Сохраняем файл
	string error;
	AnxStreamFile stream(true);
	if(!stream.Open(file))
	{
		error = "File not saved: ";
		error += file;
		opt.gui_manager->MessageBox(error, "Open file error", GUIMB_OK);
		return;
	}
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		stream.WriteID(' PNA');
		opt.project->Write(stream);
		opt.project->path = file;
		file.GetFilePath(opt.path.project);
		saveViewTime = 1.0f;
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		if(!err) err = "";
		error = "Project not saved:\n    ";
		error += file;
		error += " \nError:\n    ";
		error += err;
		opt.gui_manager->MessageBox(error, "Write file error", GUIMB_OK);
	}catch(...){
		error = "Project not saved:\n    ";
		error += file;
		opt.gui_manager->MessageBox(error, "Write file error", GUIMB_OK);
	}
#endif
	dfs = null;
}

void _cdecl AnxEditorToolBar::SaveProjectCancel(GUIControl * sender)
{
	dfs = null;
}

//Выгрузить проеки в anx
void _cdecl AnxEditorToolBar::ExportProject(GUIControl * sender)
{
	dfs = NEW GUIFileSave();
	dfs->Filters.Clear();
	dfs->Filters.Add(".anx");
	dfs->FiltersDesc.Add("Animation extended file");
	dfs->RefreshFilters();
	dfs->Caption = "Export animation project file to engine";
	dfs->StartDirectory = opt.path.anx;
	dfs->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::ExportProjectOk;
	dfs->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::ExportProjectCancel;
	opt.gui_manager->ShowModal(dfs);
}

void _cdecl AnxEditorToolBar::ExportProjectOk(GUIControl * sender)
{
	//Формируем имя файла
	Assert(dfs);
	string file = dfs->FileName;
	file.Lower().AddExtention(".anx");
	dfs = null;
	string anxpath;
	if(opt.path.anx != anxpath.GetFilePath(file))
	{
		opt.path.anx = anxpath;
		opt.UpdatePath();
	}
	//Открываем файл
	string error;
	AnxExporter exp(opt);
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(exp.Export())
		{
			IFileService * fs = (IFileService *)api->GetService("FileService");
			Assert(fs);
			if(!fs->SaveData(file.c_str(), exp.GetData(), exp.GetSize()))
			{
				error = "Project not exported:\n    ";
				error += file;
				opt.gui_manager->MessageBox(error, "Open file error", GUIMB_OK);
				return;
			}
		}
		if(exp.error > 0)
		{
			opt.gui_manager->ShowModal(NEW AnxExportLog(opt, exp.error));
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		if(!err) err = "";
		error = "Project not exported:\n    ";
		error += file;
		error += " \nError:\n    ";
		error += err;
		opt.gui_manager->MessageBox(error, "Write file error", GUIMB_OK);
	}catch(...){
		error = "Project not exported:\n    ";
		error += file;
		opt.gui_manager->MessageBox(error, "Write file error", GUIMB_OK);
	}
#endif
}

void _cdecl AnxEditorToolBar::ExportProjectCancel(GUIControl * sender)
{
	dfs = null;
}

//Импортировать другой проект
void _cdecl AnxEditorToolBar::ImportProject(GUIControl * sender)
{
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".anp");
	dfo->FiltersDesc.Add("Animation project file");
	dfo->RefreshFilters();
	dfo->Caption = "Select animation project file for import it";
	dfo->StartDirectory = opt.path.project;
	dfo->bShowReadOnly = true;
	dfo->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::ImportProjectOk;
	dfo->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::ImportProjectCancel;
	opt.gui_manager->ShowModal(dfo);
}

void _cdecl AnxEditorToolBar::ImportProjectOk(GUIControl * sender)
{
	//Формируем имя файла
	Assert(dfo);
	const char * gerr = null;
	string file = dfo->FileName;
	file.Lower().AddExtention(".anp");
	string pjpath;
	if(opt.path.project != pjpath.GetFilePath(file))
	{
		opt.path.project = pjpath;
		opt.UpdatePath();
	}
	//Открываем файл
	string error;
	AnxStreamFile stream(false);
	if(!stream.Open(file))
	{
		error = "File not opened:\n    ";
		error += file;
		opt.gui_manager->MessageBox(error, "Open file error", GUIMB_OK);
		return;
	}
	//Загружаем проект
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(stream.ReadID() != ' PNA') throw "Invalidate file id";
		gerr = opt.project->Import(stream);
		if(!gerr)
		{
			//opt.project->path = file;
			opt.project->Execute(NEW AnxCmd_Import(opt));
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		if(!err) err = "";
		error = "File not opened:\n    ";
		error += file;
		error += " \nError:\n    ";
		error += err;
		opt.gui_manager->MessageBox(error, "Read file error", GUIMB_OK);
		delete opt.project;
		opt.project = null;
	}catch(...){
		error = "File not opened:\n    ";
		error += file;
		opt.gui_manager->MessageBox(error, "Read file error", GUIMB_OK);
		delete opt.project;
		opt.project = null;
	}
#endif
	if(gerr)
	{
		error = "File not opened:\n    ";
		error += file;
		error += " \nError:\n    ";
		error += gerr;
		opt.gui_manager->MessageBox(error, "Import file error", GUIMB_OK);
	}
}

void _cdecl AnxEditorToolBar::ImportProjectCancel(GUIControl * sender)
{
	dfo = null;
}

//Импортировать звуковые события
void _cdecl AnxEditorToolBar::ImportEvents(GUIControl * sender)
{
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".ini");
	dfo->FiltersDesc.Add("Ini file");
	dfo->RefreshFilters();
	dfo->Caption = "Select ini file with sound events for import to project";
	dfo->StartDirectory = opt.path.events;
	dfo->bShowReadOnly = false;
	dfo->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::ImportEventsOk;
	dfo->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::ImportEventsCancel;
	opt.gui_manager->ShowModal(dfo);
}

void _cdecl AnxEditorToolBar::ImportEventsOk(GUIControl * sender)
{
	//Формируем имя файла
	Assert(dfo);
	const char * gerr = null;
	string file = dfo->FileName;
	file.Lower().AddExtention(".ini");
	string evtpath;
	if(opt.path.project != evtpath.GetFilePath(file))
	{
		opt.path.events = evtpath;
		opt.UpdatePath();
	}
	//Импортируем
	Assert(opt.project);
	array<string> errors(_FL_);
	opt.project->ImportEvents(file, errors);
	if(errors.Size() > 0)
	{
		opt.gui_manager->ShowModal(NEW AnxExportLog(opt, errors));
	}
}

void _cdecl AnxEditorToolBar::ImportEventsCancel(GUIControl * sender)
{
	dfo = null;
}

//Импортировать ролик
void _cdecl AnxEditorToolBar::ImportMovie(GUIControl * sender)
{
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".cutscene");
	dfo->FiltersDesc.Add("Cutscene file");
	dfo->RefreshFilters();
	dfo->Caption = "Select ini file with movie acts for import to project";
	dfo->StartDirectory = opt.path.events;
	dfo->bShowReadOnly = false;
	dfo->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::ImportMovieOk;
	dfo->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::ImportMovieCancel;
	opt.gui_manager->ShowModal(dfo);
}

void _cdecl AnxEditorToolBar::ImportMovieOk(GUIControl * sender)
{
	//Формируем имя файла
	Assert(dfo);
	const char * gerr = null;
	string file = dfo->FileName;
	file.Lower().AddExtention(".cutscene");
	string evtpath;
	if(opt.path.project != evtpath.GetFilePath(file))
	{
		opt.path.events = evtpath;
		opt.UpdatePath();
	}
	//Импортируем
	Assert(opt.project);
	array<string> errors(_FL_);
	opt.project->ImportMovie(file, errors);
	if(errors.Size() > 0)
	{
		opt.gui_manager->ShowModal(NEW AnxExportLog(opt, errors));
	}
}

void _cdecl AnxEditorToolBar::ImportMovieCancel(GUIControl * sender)
{
	dfo = null;
}

//Перезагрузить анимационные клипы
void _cdecl AnxEditorToolBar::ReloadAnts(GUIControl * sender)
{
	isRecursiveAntsReload = true;
	GUIMessageBox * mb = opt.gui_manager->MessageBox("Do graph recursive reload ants from current level?", "Select reload mode", GUIMB_YESNO);
	mb->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::ReloadAntsIsRecursive;
	mb->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::ReloadAntsIsNotRecursive;
}

void _cdecl AnxEditorToolBar::ReloadAntsIsRecursive(GUIControl * sender)
{
	isRecursiveAntsReload = true;
	ReloadAntsSelPath(sender);
}

void _cdecl AnxEditorToolBar::ReloadAntsIsNotRecursive(GUIControl * sender)
{
	isRecursiveAntsReload = false;
	ReloadAntsSelPath(sender);
}

void _cdecl AnxEditorToolBar::ReloadAntsSelPath(GUIControl * sender)
{
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".ant");
	dfo->FiltersDesc.Add("Animation temporary file");
	dfo->RefreshFilters();
	dfo->Caption = "Select any ant file for set path to reload operation";
	dfo->StartDirectory = opt.path.ant;
	dfo->bShowReadOnly = true;
	dfo->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::ReloadAntsOk;
	dfo->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::ReloadAntsCancel;
	opt.gui_manager->ShowModal(dfo);	
}

void _cdecl AnxEditorToolBar::ReloadAntsOk(GUIControl * sender)
{
	//Получаем путь
	string file = dfo->FileName;
	string path;
	path.GetFilePath(file);
	//Перезагружаем клипы
	array<string> errors(_FL_);
	opt.project->ReloadAnts(path.c_str(), isRecursiveAntsReload, errors);
	if(errors.Size() > 0)
	{
		opt.gui_manager->ShowModal(NEW AnxExportLog(opt, errors));
	}
}

void _cdecl AnxEditorToolBar::ReloadAntsCancel(GUIControl * sender)
{
	dfo = null;
}

//Установить мастер-скелет
void _cdecl AnxEditorToolBar::SetMasterSkeleton(GUIControl * sender)
{
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".ant");
	dfo->FiltersDesc.Add("Animation temporary file");
	dfo->RefreshFilters();
	dfo->Caption = "Select ant file for extract master skeleton";
	dfo->StartDirectory = opt.path.ant;
	dfo->bShowReadOnly = true;
	dfo->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::LoadMasterSkeletonOk;
	dfo->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::LoadMasterSkeletonCancel;
	opt.gui_manager->ShowModal(dfo);
}

void _cdecl AnxEditorToolBar::LoadMasterSkeletonOk(GUIControl * sender)
{
	Assert(opt.project);
	Assert(dfo);
	AntFile * ant = NEW AntFile(opt);
	MasterSkeleton * ms = null;
	string file = dfo->FileName;
	file.Lower().AddExtention(".ant");
	string fpath;
	if(opt.path.ant != fpath.GetFilePath(file))
	{
		opt.path.ant = fpath;
		opt.UpdatePath();
	}
	if(ant->Load(file))
	{
		ms = ant->CreateMasterSkeleton();
	}else{
		string error = "Ant file not loaded:\n    ";
		error += file;
		error += " \nError:\n    ";
		error += ant->loadError ? ant->loadError : "<unknow>";
		opt.gui_manager->MessageBox(error, "Read file error", GUIMB_OK);
		if(opt.project)
		{
			delete opt.project;
			opt.project = null;
		}
		return;
	}
	dfo = null;
	delete ant; ant = null;
	if(!ms)
	{
		if(!opt.project->skeleton)
		{
			opt.gui_manager->MessageBox("Project not created - not loaded master skeleton", "Error", GUIMB_OK);
			delete opt.project; opt.project = null;
			return;
		}else{
			opt.gui_manager->MessageBox("Not loaded new master skeleton", "Error", GUIMB_OK);
			return;
		}		
		return;
	}
	if(opt.project->skeleton) delete opt.project->skeleton;
	opt.project->skeleton = ms;
	for(AntFile * af = opt.firstAnt; af; af = af->next) af->ApplyMasterSkeleton();
}

void _cdecl AnxEditorToolBar::LoadMasterSkeletonCancel(GUIControl * sender)
{
	Assert(opt.project);
	Assert(dfo);
	dfo = null;
	if(!opt.project->skeleton)
	{
		opt.gui_manager->MessageBox("Project not created - not selected master skeleton", "Error", GUIMB_OK);
		delete opt.project; opt.project = null;
		return;
	}
	for(AntFile * af = opt.firstAnt; af; af = af->next) af->ApplyMasterSkeleton();
}

//Установить модельку для предпросмотра
void _cdecl AnxEditorToolBar::SetViewerModel(GUIControl * sender)
{
	opt.gui_manager->ShowModal(NEW LoadModelForm(opt));
}

//Установить партиклы
void _cdecl AnxEditorToolBar::SetParticles(GUIControl * sender)
{
	if(!opt.project) return;
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".prj");
	dfo->FiltersDesc.Add("Particles project file");
	dfo->RefreshFilters();
	dfo->Caption = "Select particles project file for preview particles events in viewer";
	dfo->StartDirectory = opt.path.prt;
	dfo->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::SetParticlesOk;
	dfo->OnCancel = (CONTROL_EVENT)&AnxEditorToolBar::SetParticlesCancel;
	opt.gui_manager->ShowModal(dfo);
}

void _cdecl AnxEditorToolBar::SetParticlesOk(GUIControl * sender)
{
	opt.project->preveiwParticles.Empty();
	opt.project->preveiwParticles.GetFileTitle(dfo->FileName);
	opt.project->UpdateParticles();
	opt.path.prt = dfo->FileName;
	opt.UpdatePath();
	dfo = null;
}

void _cdecl AnxEditorToolBar::SetParticlesCancel(GUIControl * sender)
{
	dfo = null;
}

//Показать меню нодов
void _cdecl AnxEditorToolBar::ShowCreateNodesMenu(GUIControl * sender)
{
	GUIRectangle r = buttonCreateNodes->Rect();
	AnxMenu * menu = NEW AnxMenu(r.x, r.y + r.h + 2, 192);
	Application->ShowModal(menu);
	menu->AddItem(NEW AnxCmd_CreateNode(anxnt_anode, opt), "AnxEditor\\AnxClipsNode", "Animation node", "Animation node can play animatoin clips");
	menu->AddItem(NEW AnxCmd_CreateNode(anxnt_gnode, opt), "AnxEditor\\AnxGroupNode", "Group node", "Group node include many links directed to other nodes");
	menu->AddItem(NEW AnxCmd_CreateNode(anxnt_hnode, opt), "AnxEditor\\AnxHierarchyNode", "Hierarchy node", "Hierarchy node include other nodes as folder");
	menu->AddItem(NEW AnxCmd_CreateNode(anxnt_inode, opt), "AnxEditor\\AnxInLinkNode", "Input node", "Input node get input links connected to hierarchy node");
	menu->AddItem(NEW AnxCmd_CreateNode(anxnt_onode, opt), "AnxEditor\\AnxOutLinkNode", "Output node", "Output node get output links connected to hierarchy node");
}

//Показать список нодов
void _cdecl AnxEditorToolBar::ShowGotoNodesList(GUIControl * sender)
{
	if(!opt.project) return;
	GUIRectangle r = buttonGotoNode->Rect();
	AnxToolListBox * lb = NEW AnxToolListBox(r.x, r.y + r.h + 2, 256, 256);
	lb->FillNodes(opt.project->graph.root);
	lb->List()->OnMouseDown = (CONTROL_EVENT)&AnxEditorToolBar::SelectGotoNode;
	Application->ShowModal(lb);
}

//Перейти на нод
void _cdecl AnxEditorToolBar::SelectGotoNode(GUIControl * sender)
{
	Assert(opt.project);
	AnxToolListBox * lb = (AnxToolListBox *)sender->GetParent();
	Assert(lb);
	if(lb->List()->SelectedLine < 0) return;
	opt.project->Execute(NEW AnxCmd_GotoNode(opt, lb->List()->Items[lb->List()->SelectedLine]));
	lb->Close(sender);
}

//Выйти из редактора
void _cdecl AnxEditorToolBar::EvtExitFromEditor(GUIControl * sender)
{
	opt.gui_manager->MessageBox("Realy exit from editor?", "Saving project before exit!", GUIMB_YESNO)->OnOK = (CONTROL_EVENT)&AnxEditorToolBar::EvtExitFromEditorOk;
}

void _cdecl AnxEditorToolBar::EvtExitFromEditorOk(GUIControl * sender)
{
	api->Exit();
}

