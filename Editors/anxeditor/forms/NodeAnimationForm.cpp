//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// NodeAnimationForm	
//============================================================================================
			

#include "NodeAnimationForm.h"
#include "..\EditorControls\AnxToolListBox.h"
#include "..\commands\AnxCmd_CopyPasteEvent.h"
#include "..\AntFile.h"
#include "..\AnxExporter.h"
#include "NodeAnimationEventsForm.h"
#include "NodeAnimationConstsForm.h"
#include "NodeAnimationMoveClip.h"


//============================================================================================

NodeAnimationForm::NodeAnimationForm(GraphNodeAnimation * _node) : GUIWindow(null, 0, 0, 1016, 760)
{
	AnxToolButton * b;
	Assert(_node);
	Assert(_node->opt.project->graph.current);
	node = _node;
	dfo = null;
	selectedClip = -1;
	selectEvent = -1;
	vQP = vPP = vSP = 0.0f;
	//Данные для undo
	nodeState = NEW AnxCmd_NodeState(node);
	//Устанавливаем окно поверх
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	SetScreenCenter();
	//Расставляем элементы
	//Подпись
	GUILabel * lb = NEW GUILabel(this, 10, 10, DrawRect.w - 20, 28);
	lb->Caption = "Node name";
	lb->Layout = GUILABELLAYOUT_Left;
	editName = NEW AnxToolEdit(this, 10, 40, DrawRect.w - 20, 28);
	editName->Text = node->name;
	editName->Hint = "Unique node text identificator";
	//Параметры клипа
	long basey = 90;
	//Редактирование вероятности
	lb = NEW GUILabel(this, 10, basey + 210, 200, 28);
	lb->Caption = "Selecting weight:";
	lb->Layout = GUILABELLAYOUT_Left;
	editProb = NEW AnxToolEdit(this, 200, basey + 214, 100, 24);
	editProb->Text = "0";
	editProb->Hint = "Probability's weight of selected clip at start node play";
	//Редактирование FPS
	lb = NEW GUILabel(this, 10, basey + 240, 200, 28);
	lb->Caption = "Frames per second:";
	lb->Layout = GUILABELLAYOUT_Left;
	editFPS = NEW AnxToolEdit(this, 200, basey + 244, 100, 24);
	editFPS->Text = "0";
	editFPS->OnEditUpdate = (CONTROL_EVENT)&NodeAnimationForm::EvtUpdateFPS;
	editFPS->Hint = "Play speed of current clip";
	//Редактирование минимального кадра
	lb = NEW GUILabel(this, 10, basey + 270, 200, 28);
	lb->Caption = "Start clip frame:";
	lb->Layout = GUILABELLAYOUT_Left;
	editMin = NEW AnxToolEdit(this, 200, basey + 274, 100, 24);
	editMin->Text = "0";
	editMin->OnEditUpdate = (CONTROL_EVENT)&NodeAnimationForm::EvtUpdateMinFrame;
	editMin->Hint = "Exported clip's data is start from this frame";
	//Редактирование максимального кадра
	lb = NEW GUILabel(this, 10, basey + 300, 200, 28);
	lb->Caption = "End clip frame:";
	lb->Layout = GUILABELLAYOUT_Left;
	editMax = NEW AnxToolEdit(this, 200, basey + 304, 100, 24);
	editMax->Text = "0";
	editMax->OnEditUpdate = (CONTROL_EVENT)&NodeAnimationForm::EvtUpdateMaxFrame;
	editMax->Hint = "Exported clip's data is ended on this frame";
	//Точность кватернионов
	lb = NEW GUILabel(this, 10, basey + 340, 200, 28);
	lb->Caption = "QP:";
	lb->Layout = GUILABELLAYOUT_Left;
	editQP = NEW AnxToolEdit(this, 40, basey + 344, 70, 24);
	editQP->Text = "0";
	editQP->OnEditUpdate = (CONTROL_EVENT)&NodeAnimationForm::EvtUpdatePrecisions;
	editQP->Hint = "Quaternions (angles) compression precision";
	//Точность позиций
	lb = NEW GUILabel(this, 120, basey + 340, 200, 28);
	lb->Caption = "PP:";
	lb->Layout = GUILABELLAYOUT_Left;
	editPP = NEW AnxToolEdit(this, 150, basey + 344, 70, 24);
	editPP->Text = "0";
	editPP->OnEditUpdate = (CONTROL_EVENT)&NodeAnimationForm::EvtUpdatePrecisions;
	editPP->Hint = "Positions compression precision";
	//Точность масштабирования
	lb = NEW GUILabel(this, 230, basey + 340, 200, 28);
	lb->Caption = "SP:";
	lb->Layout = GUILABELLAYOUT_Left;
	editSP = NEW AnxToolEdit(this, 260, basey + 344, 70, 24);
	editSP->Text = "0";
	editSP->OnEditUpdate = (CONTROL_EVENT)&NodeAnimationForm::EvtUpdatePrecisions;
	editSP->Hint = "Scale compression precision (Now is not used)";
	//Список событий
	lb = NEW GUILabel(this, 10, basey + 370, 150, 28);
	lb->Caption = "Clip events:";
	lb->Layout = GUILABELLAYOUT_Left;
	events = NEW GUIComboBox(this, 10, basey + 400, 320, 20, 
							(NEW AnxToolButton(null, null, 10, 10, 10, 10))->NoShadow(), 
							NEW AnxToolEdit(null, 10, 10, 100, 20), 
							NEW AnxToolListBox::LB(null, 100, 100));
	events->ListBox->SelectedLine = 0;
	events->OnChange = (CONTROL_EVENT)&NodeAnimationForm::EvtEventInfoChangle;
	events->bUserCanTypeText = true;
	events->OnSelect = (CONTROL_EVENT)&NodeAnimationForm::EvtSelectEvent;
	events->Edit->Hint = "List of events for current clip";
	//
	b = NEW AnxToolButton(null, this, 10, basey + 430, 120, 24);
	b->Caption = "Add new event";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtAddNewEvent;
	b->Hint = "Addition new event for current clip";
	//
	b = NEW AnxToolButton(null, this, 10 + 10 + 120, basey + 430, 120, 24);
	b->Caption = "Delete event";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtDeleteEvent;
	b->Hint = "Delete selected event from current clip";
	//
	b = NEW AnxToolButton(NEW AnxCmd_CopyPasteEvent(_node->opt, this, true), this, 10 + 10 + 120*2 + 10, basey + 430, 24, 24);
	b->Caption = "";
	b->Align = GUIAL_Center;
	b->Glyph->Load("AnxEditor\\AnxCopy");
	b->Hint = "Copy select event to event's buffer";
	//
	b = NEW AnxToolButton(NEW AnxCmd_CopyPasteEvent(_node->opt, this, false), this, 10 + 10 + 120*2 + 10 + 10 + 24, basey + 430, 24, 24);
	b->Caption = "";
	b->Align = GUIAL_Center;
	b->Glyph->Load("AnxEditor\\AnxPaste");
	b->Hint = "Paste event to current clip from event's buffer";
	//Редактирование кадра события
	lb = NEW GUILabel(this, 10, basey + 470, 200, 28);
	lb->Caption = "Event frame:";
	lb->Layout = GUILABELLAYOUT_Left;
	editEvt = NEW AnxToolEdit(this, 130, basey + 474, 100, 24);
	editEvt->Text = "0";
	editEvt->Hint = "Number of frame by activate event";
	editEvt->OnEditUpdate = (CONTROL_EVENT)&NodeAnimationForm::EvtEventFrameUpdate;
	//Параметры
	b = NEW AnxToolButton(null, this, 250, basey + 474, 80, 24);
	b->Caption = "Params";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtShowEventParams;
	b->Hint = "Edit event's parameters";
	//Параметры нода
	//Флаги
	flgLoop = NEW GUICheckBox(this, 10, 120, 120, 20);
	flgLoop->Caption = "Loop play";
	flgLoop->ImageChecked->Load("AnxEditor\\Anxcheckbox_ch");
	flgLoop->ImageNormal->Load("AnxEditor\\Anxcheckbox_uch");
	flgLoop->Checked = node->isLoop;
	flgLoop->OnChange = (CONTROL_EVENT)&NodeAnimationForm::EvtShowChangeCheck;
	flgLoop->Hint = "Animation of node can play is looping";
	flgChange = NEW GUICheckBox(this, 150, 120, 130, 20);
	flgChange->Caption = "Change clip";
	flgChange->ImageChecked->Load("AnxEditor\\Anxcheckbox_ch");
	flgChange->ImageNormal->Load("AnxEditor\\Anxcheckbox_uch");
	flgChange->Visible = node->isLoop;
	flgChange->Checked = node->isChange;
	flgChange->Hint = "When next loop turn, current clip will be changed";	
	//Флаг смещения рутовой кости
	flgMovement = NEW GUICheckBox(this, 10, 140, 250, 20);
	flgMovement->Caption = "Character movement";
	flgMovement->ImageChecked->Load("AnxEditor\\Anxcheckbox_ch");
	flgMovement->ImageNormal->Load("AnxEditor\\Anxcheckbox_uch");
	flgMovement->Checked = node->isMovement;
	flgMovement->OnChange = (CONTROL_EVENT)&NodeAnimationForm::EvtChangeMovement;
	flgMovement->Hint = "Read position from root bone and affect is it for use later";
	//Флаг разрешающий использование глобальной позиции
	flgGlobalPos = NEW GUICheckBox(this, 10, 180, 250, 20);
	flgGlobalPos->Caption = "Global positions";
	flgGlobalPos->ImageChecked->Load("AnxEditor\\Anxcheckbox_ch");
	flgGlobalPos->ImageNormal->Load("AnxEditor\\Anxcheckbox_uch");
	flgGlobalPos->Checked = node->isGlobalPos;
	flgGlobalPos->OnChange = (CONTROL_EVENT)&NodeAnimationForm::EvtChangeGlobalPos;
	flgGlobalPos->Hint = "Enable use global position tracks in node clips";
	//Установить нод стартовым
	flgStart = NEW GUICheckBox(this, 14, 90, 200, 20);
	flgStart->Caption = "Start node";
	flgStart->ImageChecked->Load("AnxEditor\\Anxscheckbox_ch");
	flgStart->ImageNormal->Load("AnxEditor\\Anxscheckbox_uch");
	flgStart->Checked = node->opt.project->startNode == node;
	flgStart->OnChange = (CONTROL_EVENT)&NodeAnimationForm::EvtChangeStartNode;
	flgStart->Hint = "After load, animation start from start node";
	//Усстоновить нод стоповым
	flgStop = NEW GUICheckBox(this, 214, 90, 200, 20);
	flgStop->Caption = "Stop node";
	flgStop->ImageChecked->Load("AnxEditor\\Anxscheckbox_ch");
	flgStop->ImageNormal->Load("AnxEditor\\Anxscheckbox_uch");
	flgStop->Checked = node->opt.project->stopNode == node;
	flgStop->OnChange = (CONTROL_EVENT)&NodeAnimationForm::EvtChangeStopNode;
	flgStop->Hint = "When animation comming to this node, its set to pause";
	//Список клипов
	lb = NEW GUILabel(this, 10, basey + 110, 150, 28);
	lb->Caption = "Node clips:";
	lb->Layout = GUILABELLAYOUT_Left;
	clips = NEW GUIComboBox(this, 10, basey + 140, 320, 20, 
							(NEW AnxToolButton(null, null, 10, 10, 10, 10))->NoShadow(), 
							NEW AnxToolEdit(null, 10, 10, 100, 20), 
							NEW AnxToolListBox::LB(null, 100, 100));
	clips->ListBox->SelectedLine = 0;
	clips->OnChange = (CONTROL_EVENT)&NodeAnimationForm::EvtClipNameChangle;
	clips->OnSelect = (CONTROL_EVENT)&NodeAnimationForm::EvtSelectClip;
	clips->bUserCanTypeText = true;
	clips->Edit->Hint = "Select current clip for edit";
	//
	b = NEW AnxToolButton(null, this, 10, basey + 170, 100, 24);
	b->Caption = "Add clip";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtAddNewClip;
	b->Hint = "Addition new clip to node";
	//
	b = NEW AnxToolButton(null, this, 120, basey + 170, 100, 24);
	b->Caption = "Reload clip";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtReloadClip;
	b->Hint = "Reload curren clip in node whith safe current events";
	//
	b = NEW AnxToolButton(null, this, 330 - 100, basey + 170, 100, 24);
	b->Caption = "Delete clip";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtDeleteClip;
	b->Hint = "Delete current clip from node";
	//
	b = NEW AnxToolButton(null, this, 330 - 95, basey + 106, 24, 24);
	b->Glyph->Load("AnxEditor\\AnxMoveClip");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtMoveClip;
	b->Hint = "Move current clip new position";
	//
	b = NEW AnxToolButton(null, this, 330 - 60, basey + 106, 24, 24);
	b->Glyph->Load("AnxEditor\\AnxReverse");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtReverseClip;
	b->Hint = "Reverse current clip";
	//
	b = NEW AnxToolButton(null, this, 330 - 25, basey + 106, 24, 24);
	b->Glyph->Load("AnxEditor\\AnxReduce");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtReduceClip;
	b->Hint = "Reduce current clip by 2";
	//Список констант
	lb = NEW GUILabel(this, 10, basey + 520, 150, 28);
	lb->Caption = "Node constatns:";
	lb->Layout = GUILABELLAYOUT_Left;
	consts = NEW GUIComboBox(this, 10, basey + 550, 320, 20, 
							(NEW AnxToolButton(null, null, 10, 10, 10, 10))->NoShadow(), 
							NEW AnxToolEdit(null, 10, 10, 100, 20), 
							NEW AnxToolListBox::LB(null, 100, 100));
	consts->ListBox->SelectedLine = 0;
	consts->OnChange = (CONTROL_EVENT)&NodeAnimationForm::EvtConstNameChangle;
	consts->OnSelect = (CONTROL_EVENT)&NodeAnimationForm::EvtSelectConst;
	consts->bUserCanTypeText = true;
	consts->Edit->Hint = "Select current constant on node";
	UpdateConstsList();
	//
	b = NEW AnxToolButton(null, this, 10, basey + 580, 150, 24);
	b->Caption = "Add new const";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtAddNewConst;
	b->Hint = "Addition new constant to node";
	//
	b = NEW AnxToolButton(null, this, 330 - 150, basey + 580, 150, 24);
	b->Caption = "Delete const";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtDeleteConst;
	b->Hint = "Delete current constant from node";
	//
	b = NEW AnxToolButton(null, this, 10, basey + 620, 320, 24);
	b->Caption = "Edit node constant";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtEditConst;
	b->Hint = "Change constant parameters";
	//Кнопки выхода
	long bw = 120;
	long bh = 32;
	long bx = (DrawRect.w - bw*2 - 20)/2;
	long by = DrawRect.h - bh - 10;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Ok";
	b->Glyph->Load("AnxEditor\\AnxOk");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtOk;
	bx += bw + 20;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Cancel";
	b->Glyph->Load("AnxEditor\\AnxCancel");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationForm::EvtCancel;
	viewer = NEW AnxAnimationViewer(node->opt, this, 360, 80, DrawRect.w - 370, 520);
	viewer->ShowControlPanel();
}

NodeAnimationForm::~NodeAnimationForm()
{
	if(nodeState) delete nodeState;
	nodeState = null;
	if(dfo) dfo->Close(dfo);
	dfo = null;
}

void NodeAnimationForm::OnCreate()
{
	editName->SetFocus();
	editName->SelectText(0, editName->Text.Len());
	UpdateClipsList();
}

//============================================================================================

//Рисование
void NodeAnimationForm::Draw()
{
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем панель
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xefe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	GUIControl::Draw();
	//Рисуем маркер эвента
	if(clips->ListBox->SelectedLine >= 0 && events->ListBox->SelectedLine >= 0)
	{
		GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];
		if(clip.events > 0
			 && events->ListBox->SelectedLine < clip.events
			  && events->ListBox->SelectedLine >= 0)
		{
			dword frame = clip.events[events->ListBox->SelectedLine]->frame;
			if(viewer)
			{
				long y, h;
				long x = viewer->GetFramePosition(frame, y, h);
				GUIHelper::Draw2DLine(x, y, x, y + h, 0xffff0000);
			}
		}
	}
}

void _cdecl NodeAnimationForm::EvtOk(GUIControl * sender)
{
	if(node->name != editName->Text)
	{		
		Assert(node->opt.project);
		if(!node->opt.project->CheckUniqueNodeName(editName->Text))
		{
			editName->FontColor = 0xffff0000;
			return;
		}
		node->name = editName->Text;
	}else{
		node->name = editName->Text;
	}
	node->isLoop = flgLoop->Checked;
	node->isChange = flgChange->Checked;
	Application->Close(this);
	node->opt.project->Execute(nodeState);
	nodeState = null;
}

void _cdecl NodeAnimationForm::EvtCancel(GUIControl * sender)
{
	Application->Close(this);
	nodeState->UnDo();
}

void _cdecl NodeAnimationForm::EvtClipNameChangle(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{
		node->clips[clips->ListBox->SelectedLine]->data.name = clips->Edit->Text;
		clips->ListBox->Items[clips->ListBox->SelectedLine] = clips->Edit->Text;
	}else{
		clips->Edit->Text.Empty();
	}
}

void _cdecl NodeAnimationForm::EvtAddNewClip(GUIControl * sender)
{
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".ant");
	dfo->FiltersDesc.Add("Animation temporary file");
	dfo->RefreshFilters();
	dfo->Caption = "Select ant file for load new clip";
	dfo->StartDirectory = node->opt.path.ant;
	dfo->bShowReadOnly = true;
	dfo->OnOK = (CONTROL_EVENT)&NodeAnimationForm::EvtAddNewClipOk;
	dfo->OnCancel = (CONTROL_EVENT)&NodeAnimationForm::EvtAddNewClipCancel;
	node->opt.gui_manager->ShowModal(dfo);
}

void _cdecl NodeAnimationForm::EvtAddNewClipOk(GUIControl * sender)
{
	Assert(node->opt.project);
	Assert(dfo);
	GraphNodeAnimation::Clip * clip = NEW GraphNodeAnimation::Clip(node->opt);
	string file = dfo->FileName;
	file.Lower().AddExtention(".ant");
	string antpath;
	if(node->opt.path.ant != antpath.GetFilePath(file))
	{
		node->opt.path.ant = antpath;
		node->opt.UpdatePath();
	}
	if(clip->data.Load(file))
	{
		//Загрузили - добавляем
		clip->data.ApplyMasterSkeleton();
		node->clips.Add(clip);
		UpdateClipsList();
		clips->SelectItem(node->clips - 1);
		EvtSelectClip(this);
	}else{
		string error = "Ant file not loaded:\n    ";
		error += file;
		error += " \nError:\n    ";
		error += clip->data.loadError ? clip->data.loadError : "<unknow>";
		node->opt.gui_manager->MessageBox(error, "Read file error", GUIMB_OK);
		delete clip;
	}
	dfo = null;
}

void _cdecl NodeAnimationForm::EvtAddNewClipCancel(GUIControl * sender)
{
	Assert(node->opt.project);
	Assert(dfo);
	dfo = null;
}

void _cdecl NodeAnimationForm::EvtReloadClip(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine < 0) return;
	dfo = NEW GUIFileOpen();
	dfo->Filters.Clear();
	dfo->Filters.Add(".ant");
	dfo->FiltersDesc.Add("Animation temporary file");
	dfo->RefreshFilters();
	dfo->Caption = "Select ant file for reload current clip";
	dfo->StartDirectory = node->opt.path.ant;
	dfo->bShowReadOnly = true;
	dfo->OnOK = (CONTROL_EVENT)&NodeAnimationForm::EvtReloadClipOk;
	dfo->OnCancel = (CONTROL_EVENT)&NodeAnimationForm::EvtReloadClipCancel;
	node->opt.gui_manager->ShowModal(dfo);
}

void _cdecl NodeAnimationForm::EvtReloadClipOk(GUIControl * sender)
{
	Assert(node->opt.project);
	Assert(dfo);
	GraphNodeAnimation::Clip * clip = NEW GraphNodeAnimation::Clip(node->opt);
	string file = dfo->FileName;
	file.Lower().AddExtention(".ant");
	string antpath;
	if(node->opt.path.ant != antpath.GetFilePath(file))
	{
		node->opt.path.ant = antpath;
		node->opt.UpdatePath();
	}
	if(clip->data.Load(file))
	{
		//Загрузили - заменяем
		clip->data.ApplyMasterSkeleton();
		GraphNodeAnimation::Clip * oldClip = node->clips[clips->ListBox->SelectedLine];
		node->clips[clips->ListBox->SelectedLine] = clip;
		clip->ExtractInfo(*oldClip, node->isMovement);
		delete oldClip;
		UpdateClipsList();
		EvtSelectClip(this);
	}else{
		string error = "Ant file not loaded:\n    ";
		error += file;
		error += " \nError:\n    ";
		error += clip->data.loadError ? clip->data.loadError : "<unknow>";
		node->opt.gui_manager->MessageBox(error, "Read file error", GUIMB_OK);
		delete clip;
	}
	dfo = null;
}

void _cdecl NodeAnimationForm::EvtReloadClipCancel(GUIControl * sender)
{
	Assert(node->opt.project);
	Assert(dfo);
	dfo = null;
}


void _cdecl NodeAnimationForm::EvtDeleteClip(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{
		long sl = clips->ListBox->SelectedLine;
		delete node->clips[clips->ListBox->SelectedLine];
		node->clips.DelIndex(clips->ListBox->SelectedLine);
		if(sl >= node->clips) sl = node->clips - 1;
		UpdateClipsList();
		clips->SelectItem(sl);
		EvtSelectClip(this);
	}
}

void _cdecl NodeAnimationForm::EvtMoveClip(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{
		AntFile & ant = node->clips[clips->ListBox->SelectedLine]->data;
		NodeAnimationMoveClip * nodeForm = NEW NodeAnimationMoveClip(node->opt, ant);
		nodeForm->t_OnOK_Sender = this;
		nodeForm->t_OnOK->SetHandler(this, (CONTROL_EVENT)&NodeAnimationForm::EvtSelectClip);
		node->opt.gui_manager->ShowModal(nodeForm);
	}
}

void _cdecl NodeAnimationForm::EvtReverseClip(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{		
		node->clips[clips->ListBox->SelectedLine]->data.Reverse();
		EvtSelectClip(this);		
	}
}

void _cdecl NodeAnimationForm::EvtReduceClip(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{
		GraphNodeAnimation::Clip * clip = node->clips[clips->ListBox->SelectedLine];
		clip->data.Reduce();
		for(long i = 0; i < clip->events; i++)
		{
			clip->events[i]->frame /= 2;
		}
		EvtSelectClip(this);
	}
}

void _cdecl NodeAnimationForm::EvtSelectClip(GUIControl * sender)
{
	if(sender != this && clips->ListBox->SelectedLine == selectedClip) return;
	selectEvent = -1;
	//Сбрасываем состояние контролов
	editProb->Text = "0";
	editProb->floatValue = null;
	editFPS->Text = "0";
	editFPS->floatValue = null;
	editMin->Text = "0";
	editMin->longValue = null;
	editMax->Text = "0";
	editMax->longValue = null;
	editEvt->Text = "0";
	editQP->floatValue = null;
	editQP->Text = "0";
	editPP->floatValue = null;
	editPP->Text = "0";
	editSP->floatValue = null;
	editSP->Text = "0";
	//Устанавливаем новый клип
	selectedClip = clips->ListBox->SelectedLine;
	if(clips->ListBox->SelectedLine >= 0)
	{
		byte * data = null;
		dword size = 0;
		try{
			AnxExporter exporter(node->opt);
			Assert(node->clips[clips->ListBox->SelectedLine]);
			if(exporter.PreviewExport(*node->clips[clips->ListBox->SelectedLine], node->isMovement, node->isGlobalPos, flgLoop->Checked))
			{
				size = exporter.GetSize();
				data = NEW byte[size + 16];
				memcpy(AlignPtr(data), exporter.GetData(), size);
			}
		}catch(...){
			if(data) delete data;
			data = null;
			size = 0;
		}
		AntFile & ant = node->clips[clips->ListBox->SelectedLine]->data;
		if(data && size > 4)
		{
			viewer->SetAnimation(data, size);
			ant.UseAnt();
			viewer->clipSize = ant.clipSize;
			viewer->clipSRate = float(ant.GetAntDataSize())/float(ant.clipSize);
			viewer->clipFrames = ant.numFrames;
			viewer->asignedBones = ant.bone;
			viewer->totalBones = ant.numBones;
			viewer->globalPosBones = ant.globalPosBones;
			editProb->floatValue = &ant.probability;
			editProb->min = 0.00001f;
			editProb->max = 1000.0f;
			editProb->UpdateText();
			editFPS->floatValue = &ant.fps;
			editFPS->min = 0.001f;
			editFPS->max = 48.0f;
			editFPS->UpdateText();
			minFrame = ant.clipStartFrame;
			maxFrame = ant.clipStartFrame + ant.clipNumFrames - 1;
			maxFrames = ant.numFrames - 1;
			editMin->longValue = &minFrame;
			editMin->min = 0;
			editMin->max = float(ant.numFrames - 1);
			editMin->UpdateText();
			editMax->longValue = &maxFrame;
			editMax->min = 0;
			editMax->max = float(ant.numFrames - 1);
			editMax->UpdateText();
			vQP = ant.quaternionError;
			editQP->floatValue = &vQP;
			editQP->min = 0.0f;
			editQP->max = 0.1f;
			editQP->UpdateText();
			vPP = ant.positionError;
			editPP->floatValue = &vPP;
			editPP->min = 0.0f;
			editPP->max = 10.0f;
			editPP->UpdateText();
			vSP = ant.scaleError;
			editSP->floatValue = &vSP;
			editSP->min = 0.0f;
			editSP->max = 0.1f;
			editSP->UpdateText();
			ant.FreeAnt();
		}else{
			viewer->SetAnimation(null, 0);
		}
	}else{
		viewer->SetAnimation(null, 0);
	}
	UpdateEventsList();
}

void _cdecl NodeAnimationForm::EvtUpdateFPS(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{
		AntFile & ant = node->clips[clips->ListBox->SelectedLine]->data;
		viewer->SetNewFPS(ant.fps);
		if(ant.fps > 15.0f)
		{
			node->opt.gui_manager->MessageBox("Are you sure about this FPS?!", " O.O", GUIMB_YESNO)->OnCancel = (CONTROL_EVENT)&NodeAnimationForm::EvtUpdateSkipFPSChange;
		}
	}
}

void _cdecl NodeAnimationForm::EvtUpdateSkipFPSChange(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{
		AntFile & ant = node->clips[clips->ListBox->SelectedLine]->data;
		ant.fps = 15.0f;
		viewer->SetNewFPS(ant.fps);
		editFPS->UpdateText();
	}
}

void _cdecl NodeAnimationForm::EvtUpdateMinFrame(GUIControl * sender)
{
	if(minFrame < 0) minFrame = 0;
	if(minFrame > maxFrame) minFrame = maxFrame;
	if(clips->ListBox->SelectedLine >= 0)
	{
		AntFile & ant = node->clips[clips->ListBox->SelectedLine]->data;
		ant.clipStartFrame = minFrame;
		ant.clipNumFrames = maxFrame - minFrame + 1;
		EvtSelectClip(this);
	}
}

void _cdecl NodeAnimationForm::EvtUpdateMaxFrame(GUIControl * sender)
{
	if(maxFrame < minFrame) maxFrame = minFrame;
	if(maxFrame > maxFrames) maxFrame = maxFrames;
	if(clips->ListBox->SelectedLine >= 0)
	{
		AntFile & ant = node->clips[clips->ListBox->SelectedLine]->data;
		ant.clipStartFrame = minFrame;
		ant.clipNumFrames = maxFrame - minFrame + 1;
		EvtSelectClip(this);
	}
}

void _cdecl NodeAnimationForm::EvtUpdatePrecisions(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{
		AntFile & ant = node->clips[clips->ListBox->SelectedLine]->data;
		ant.quaternionError = vQP;
		ant.positionError = vPP;
		ant.scaleError = vSP;
		EvtSelectClip(this);
	}
}

void _cdecl NodeAnimationForm::EvtAddNewEvent(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine < 0) return;
	GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];
	GraphNodeAnimation::Event * evt = NEW GraphNodeAnimation::Event();
	evt->frame = 0;
	for(long c = 0; c < clip.events + 1; c++)
	{
		evt->info = "Event ";
		evt->info += c;
		for(long i = 0; i < clip.events; i++)
		{
			if(evt->info == clip.events[i]->info) break;
		}
		if(i == clip.events) break;
	}
	clip.events.Add(evt);
	UpdateEventsList();
	events->SelectItem(clip.events - 1);
}

void _cdecl NodeAnimationForm::EvtDeleteEvent(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine < 0) return;
	if(events->ListBox->SelectedLine < 0) return;
	GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];
	clip.events.DelIndex(events->ListBox->SelectedLine);
	long sl = events->ListBox->SelectedLine;
	UpdateEventsList();
	if(sl >= clip.events) sl = clip.events - 1;
	events->SelectItem(sl);
}

void _cdecl NodeAnimationForm::EvtSelectEvent(GUIControl * sender)
{
	if(events->ListBox->SelectedLine == selectEvent) return;
	editEvt->longValue = null;
	editEvt->Text = "0";
	if(clips->ListBox->SelectedLine < 0){ selectEvent = -1; return; }
	//Устанавливаем новый клип
	selectEvent = events->ListBox->SelectedLine;
	if(events->ListBox->SelectedLine >= 0)
	{
		GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];
		editEvt->longValue = (long *)&clip.events[events->ListBox->SelectedLine]->frame;
		editEvt->min = 0;
		editEvt->max = float(clip.data.GetNumFrames(node->isMovement) - 1);
		float fv = float(*editEvt->longValue);
		editEvt->UpdateText();		
		if(fv < editEvt->min || fv > editEvt->max)
		{
			editEvt->ChangeText(editEvt);
		}
		//editEvt->AcceptText(editEvt);
	}else{
		editEvt->Text = "0";
		events->Edit->Text.Empty();
	}
}

void _cdecl NodeAnimationForm::EvtEventFrameUpdate(GUIControl * sender)
{
	static bool noUpdate = false;
	if(noUpdate || events->ListBox->SelectedLine < 0) return;
	long idx = events->ListBox->SelectedLine;
	noUpdate = true;
	EvtSelectClip(this);
	events->SelectItem(idx);
	noUpdate = false;
}

void _cdecl NodeAnimationForm::EvtEventInfoChangle(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine >= 0)
	{
		if(events->ListBox->SelectedLine >= 0)
		{
			GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];
			events->ListBox->Items[events->ListBox->SelectedLine] = events->Edit->Text;
			clip.events[events->ListBox->SelectedLine]->info = events->Edit->Text;
		}else{
			events->Edit->Text.Empty();
		}
	}else{
		clips->Edit->Text.Empty();
		events->Edit->Text.Empty();
	}
}

void NodeAnimationForm::UpdateClipsList()
{
	clips->ListBox->Items.Clear();
	clips->Edit->Text.Empty();
	for(long i = 0; i < node->clips; i++)
	{
		clips->ListBox->Items.Add(node->clips[i]->data.name);
	}
	if(node->clips > 0) clips->SelectItem(0);
}

//Обновить список событий
void NodeAnimationForm::UpdateEventsList()
{
	events->ListBox->Items.Clear();
	events->Edit->Text.Empty();
	if(clips->ListBox->SelectedLine >= 0)
	{
		GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];
		for(long i = 0; i < clip.events; i++)
		{
			events->ListBox->Items.Add(clip.events[i]->info);
		}
		if(clip.events > 0) events->SelectItem(0);
		if(events->ListBox->SelectedLine > clip.events)
		{
			events->ListBox->SelectedLine = clip.events - 1;
		}
	}
}

void _cdecl NodeAnimationForm::EvtShowEventParams(GUIControl * sender)
{
	if(clips->ListBox->SelectedLine < 0) return;
	if(events->ListBox->SelectedLine < 0) return;
	GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];
	long sl = events->ListBox->SelectedLine;
	node->opt.gui_manager->ShowModal(NEW NodeAnimationEventsForm(node->opt, *clip.events[sl]));
}


//==============================================================================================
//Flags
//==============================================================================================

void _cdecl NodeAnimationForm::EvtShowChangeCheck(GUIControl * sender)
{
	flgChange->Visible = flgLoop->Checked;
	long selEvent = events->ListBox->SelectedLine;
	selectedClip = -1;
	EvtSelectClip(this);
	selectEvent = -1;
	events->SelectItem(selEvent);
}

void _cdecl NodeAnimationForm::EvtChangeStartNode(GUIControl * sender)
{
	flgStart->Checked = true;
	node->opt.project->startNode = node;
	if(node->opt.project->stopNode == node)
	{
		flgStop->Checked = false;
		node->opt.project->stopNode = null;
	}
}

void _cdecl NodeAnimationForm::EvtChangeStopNode(GUIControl * sender)
{
	flgStop->Checked = true;
	node->opt.project->stopNode = node;
	if(node->opt.project->startNode == node)
	{
		flgStart->Checked = false;
		node->opt.project->startNode = null;
	}
}

void _cdecl NodeAnimationForm::EvtChangeMovement(GUIControl * sender)
{
	node->isMovement = flgMovement->Checked;
	long selEvent = events->ListBox->SelectedLine;
	selectedClip = -1;
	EvtSelectClip(this);
	selectEvent = -1;
	events->SelectItem(selEvent);
}

void _cdecl NodeAnimationForm::EvtChangeGlobalPos(GUIControl * sender)
{
	node->isGlobalPos = flgGlobalPos->Checked;
	long selEvent = events->ListBox->SelectedLine;
	selectedClip = -1;
	EvtSelectClip(this);
	selectEvent = -1;
	events->SelectItem(selEvent);
}

//==============================================================================================
//Const
//==============================================================================================

void _cdecl NodeAnimationForm::EvtConstNameChangle(GUIControl * sender)
{
	if(consts->ListBox->SelectedLine >= 0)
	{
		GraphNodeAnimation::ConstBlock & cb = *node->consts[consts->ListBox->SelectedLine];
		cb.name = consts->Edit->Text;
		consts->ListBox->Items[consts->ListBox->SelectedLine] = consts->Edit->Text;
	}else{
		consts->Edit->Text.Empty();
	}
}

void _cdecl NodeAnimationForm::EvtSelectConst(GUIControl * sender)
{
	
}

void _cdecl NodeAnimationForm::EvtAddNewConst(GUIControl * sender)
{
	GraphNodeAnimation::ConstBlock * cb = NEW GraphNodeAnimation::ConstBlock();
	for(long c = 0; c < node->consts + 1; c++)
	{		
		cb->name = "Constant ";
		cb->name += c;
		for(long i = 0; i < node->consts; i++)
		{
			if(cb->name == node->consts[i]->name) break;
		}
		if(i == node->consts) break;
	}
	node->consts.Add(cb);
	UpdateConstsList();
	consts->SelectItem(node->consts - 1);
}

void _cdecl NodeAnimationForm::EvtDeleteConst(GUIControl * sender)
{
	if(consts->ListBox->SelectedLine >= 0)
	{
		node->consts.DelIndex(consts->ListBox->SelectedLine);
		long sl = consts->ListBox->SelectedLine;
		if(sl >= node->consts) sl = node->consts - 1;
		UpdateConstsList();
		consts->SelectItem(sl);
	}else{
		if(node->consts)
		{
			UpdateConstsList();
		}
	}
}

void _cdecl NodeAnimationForm::EvtEditConst(GUIControl * sender)
{
	if(consts->ListBox->SelectedLine >= 0)
	{
		GraphNodeAnimation::ConstBlock & cb = *node->consts[consts->ListBox->SelectedLine];
		node->opt.gui_manager->ShowModal(NEW NodeAnimationConstsForm(node->opt, cb));
	}else{
		UpdateConstsList();
	}
}

void NodeAnimationForm::UpdateConstsList()
{
	consts->ListBox->Items.Clear();
	consts->Edit->Text.Empty();
	for(long i = 0; i < node->consts; i++)
	{
		consts->ListBox->Items.Add(node->consts[i]->name);
	}
	if(node->consts > 0) consts->SelectItem(0);
}

bool NodeAnimationForm::IsEnableEventCopy()
{
	//Управляем кнопками копирования эвентов
	if(clips->ListBox->SelectedLine >= 0 && events->ListBox->SelectedLine >= 0)
	{
		return true;
	}
	return false;
}

bool NodeAnimationForm::CopyEvent(AnxStream & stream)
{
	if(clips->ListBox->SelectedLine < 0 || events->ListBox->SelectedLine < 0)
	{
		return false;
	}
	GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];		
	GraphNodeAnimation::Event * evt = clip.events[events->ListBox->SelectedLine];
	evt->Write(stream);
	return true;
}

void NodeAnimationForm::PasteEvent(AnxStream & stream)
{
	if(clips->ListBox->SelectedLine < 0) return;
	GraphNodeAnimation::Clip & clip = *node->clips[clips->ListBox->SelectedLine];
	GraphNodeAnimation::Event * evt = NEW GraphNodeAnimation::Event();
	evt->Read(stream);
	clip.events.Add(evt);
	UpdateEventsList();
	events->SelectItem(clip.events - 1);	
}