//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// LinkForm	
//============================================================================================
			

#include "LinkForm.h"
#include "..\Graph\GraphNodeInOut.h"

//============================================================================================
//LinkFormPanel
//============================================================================================

LinkFormPanel::LinkFormPanel(GraphLink * _link, bool isFake) : GUIWindow(null, 0, 0, 600, 100), inOutNames(_FL_)
{
	Assert(_link);
	Assert(_link->from);
	Assert(_link->to);
	link = _link;
	//Команда отмены
	linkState = NEW AnxCmd_LinkState(link);
	//Создаём окно формы линка
	if(!isFake)
	{
		DrawRect.w = ClientRect.w = 680;
		form = NEW LinkForm(this, link->opt, link->data, GUIPoint(0, 40));
		form->eventOk.SetHandler(this, (CONTROL_EVENT)&LinkFormPanel::EvtOk);
		form->eventCancel.SetHandler(this, (CONTROL_EVENT)&LinkFormPanel::EvtCancel);
		//Устанавливаем новую высоту
		DrawRect.h = ClientRect.h = form->GetDrawRect().h + 40;		
		editFrom = null;
		editTo = null;
	}else{
		form = null;
		//Имена нодов
		long y = 50;
		bool isFields = false;
		if(link->from->GetType() == anxnt_hnode)
		{
			isFields = true;
			GUILabel * lb = NEW GUILabel(this, 10, y, DrawRect.w - 20, 28);
			lb->Caption = "From output node in hierarchy node [";
			lb->Caption += link->from->name;
			lb->Caption += "] :";
			lb->Layout = GUILABELLAYOUT_Left;
			editFrom = NEW AnxToolEdit(this, 10, y + 30, DrawRect.w - 60, 20);
			editFrom->stringValue = &link->fromONode;
			editFrom->UpdateText();
			AnxToolButton * b = NEW AnxToolButton(null, this, DrawRect.w - 41, y + 26, 31, 28);
			b->Glyph->Load("AnxEditor\\AnxSelNode");
			b->Align = GUIAL_Center;
			b->OnMousePressed = (CONTROL_EVENT)&LinkFormPanel::EvtSelFromNode;
			y += 70;
		}
		if(link->to->GetType() == anxnt_hnode)
		{
			isFields = true;
			GUILabel * lb = NEW GUILabel(this, 10, y, DrawRect.w - 20, 28);
			lb->Caption = "To input node in hierarchy node [";
			lb->Caption += link->to->name;
			lb->Caption += "] :";
			lb->Layout = GUILABELLAYOUT_Left;
			editTo = NEW AnxToolEdit(this, 10, y + 30, DrawRect.w - 60, 20);
			editTo->stringValue = &link->toINode;
			editTo->UpdateText();
			AnxToolButton * b = NEW AnxToolButton(null, this, DrawRect.w - 41, y + 26, 31, 28);
			b->Glyph->Load("AnxEditor\\AnxSelNode");
			b->Align = GUIAL_Center;
			b->OnMousePressed = (CONTROL_EVENT)&LinkFormPanel::EvtSelToNode;
			y += 70;
		}
		if(!isFields)
		{
			GUILabel * lb = NEW GUILabel(this, 10, y, DrawRect.w - 20, 28);
			lb->Caption = "This link not include properties...";
			lb->Layout = GUILABELLAYOUT_Left;
			y += 50;
		}
		//Устанавливаем новую высоту
		DrawRect.h = ClientRect.h = y + 50;
		if(isFields)
		{
			//Кнопки
			long bw = 120;
			long bh = 32;
			long bx = (DrawRect.w - bw*2 - 20)/2;
			long by = DrawRect.h - bh - 10;
			AnxToolButton * b = NEW AnxToolButton(null, this, bx, by, bw, 32);
			b->Caption = "Ok";
			b->Glyph->Load("AnxEditor\\AnxOk");
			b->Align = GUIAL_Center;
			b->OnMousePressed = (CONTROL_EVENT)&LinkFormPanel::EvtOk;
			bx += bw + 20;
			b = NEW AnxToolButton(null, this, bx, by, bw, 32);
			b->Caption = "Cancel";
			b->Glyph->Load("AnxEditor\\AnxCancel");
			b->Align = GUIAL_Center;
			b->OnMousePressed = (CONTROL_EVENT)&LinkFormPanel::EvtCancel;
		}else{
			//Кнопки
			long bw = 120;
			long bh = 32;
			long bx = (DrawRect.w - bw)/2;
			long by = DrawRect.h - bh - 10;
			AnxToolButton * b = NEW AnxToolButton(null, this, bx, by, bw, 32);
			b->Caption = "Ok";
			b->Glyph->Load("AnxEditor\\AnxOk");
			b->Align = GUIAL_Center;
			b->OnMousePressed = (CONTROL_EVENT)&LinkFormPanel::EvtCancel;
		}
	}
	//Расчитываем позицию
	SetScreenCenter();
	//Устанавливаем окно поверх
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	//Расставляем элементы
	//Ноды
	lb = NEW GUILabel(this, 10, 10, DrawRect.w - 20, 28);
	lb->Caption = "Link connection from node [";
	lb->Caption += link->from->name;
	lb->Caption += "] to node [";
	lb->Caption += link->to->name;
	lb->Caption += "]";
	lb->Layout = GUILABELLAYOUT_Left;
}

LinkFormPanel::~LinkFormPanel()
{
	if(linkState) delete linkState;
	linkState = null;
}

//============================================================================================

//Рисование
void LinkFormPanel::Draw()
{
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем панель
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xefe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	GUIRectangle lbr = lb->GetDrawRect();
	lb->ClientToScreen(lbr);
	GUIHelper::Draw2DRectAlpha(lbr.x - 3, lbr.y - 3, lbr.w + 6, lbr.h + 6, 0x3f0000ff);
	GUIControl::Draw();
}

void _cdecl LinkFormPanel::EvtOk(GUIControl * sender)
{
	Assert(linkState);
	Assert(link->opt.project);
	link->opt.project->Execute(linkState);
	linkState = null;
	link->opt.gui_manager->Close(this);
}

void _cdecl LinkFormPanel::EvtCancel(GUIControl * sender)
{
	Assert(linkState);
	linkState->UnDo();
	link->opt.gui_manager->Close(this);
}

void _cdecl LinkFormPanel::EvtSelFromNode(GUIControl * sender)
{
	MakeSelFromToNodeList(sender, link->from, anxnt_onode, (CONTROL_EVENT)&LinkFormPanel::EvtSetSelFromNode);
}

void _cdecl LinkFormPanel::EvtSetSelFromNode(GUIControl * sender)
{
	Assert(link->opt.project);
	Assert(editFrom);
	AnxToolListBox * lb = (AnxToolListBox *)sender->GetParent();
	Assert(lb);
	if(lb->List()->SelectedLine < 0 || lb->List()->SelectedLine >= inOutNames) return;
	editFrom->Text = inOutNames[lb->List()->SelectedLine];
	editFrom->AcceptText(this);
	lb->Close(sender);
}

void _cdecl LinkFormPanel::EvtSelToNode(GUIControl * sender)
{
	MakeSelFromToNodeList(sender, link->to, anxnt_inode, (CONTROL_EVENT)&LinkFormPanel::EvtSetSelToNode);
}

void _cdecl LinkFormPanel::EvtSetSelToNode(GUIControl * sender)
{
	Assert(link->opt.project);
	Assert(editFrom);
	AnxToolListBox * lb = (AnxToolListBox *)sender->GetParent();
	Assert(lb);
	if(lb->List()->SelectedLine < 0 || lb->List()->SelectedLine >= inOutNames) return;
	editTo->Text = inOutNames[lb->List()->SelectedLine];
	editTo->AcceptText(this);
	lb->Close(sender);
}

void LinkFormPanel::MakeSelFromToNodeList(GUIControl * sender, GraphNodeBase * findNode, AnxNodeTypes type, CONTROL_EVENT event)
{
	//Позиция листа
	GUIRectangle rect = ((LinkFormPanel *)sender)->DrawRect;
	sender->ClientToScreen(rect);
	rect.x += rect.w - 256;
	rect.y += rect.h;
	rect.w = 256;
	rect.h = 256;
	if(rect.x < 20) rect.x = 20;
	if(rect.y < 20) rect.y = 20;
	if(rect.x + rect.w > link->opt.width - 20) rect.x = link->opt.width - rect.w - 20;
	if(rect.y + rect.h > link->opt.height - 20) rect.y = link->opt.height - rect.h - 20;
	//Лист
	AnxToolListBox * lb = NEW AnxToolListBox(rect.x, rect.y, rect.w, rect.h);
	lb->List()->OnMouseDown = (CONTROL_EVENT)event;
	//Заполняем нодами
	lb->FillNodes(findNode, false, type);
	//Заменяем имена нодов именами линков
	inOutNames.Empty();
	for(long i = 0; i < lb->List()->Items.Size(); )
	{
		const char * name = lb->List()->Items[i];
		GraphNodeInOut * node = (GraphNodeInOut *)link->opt.project->graph.Find(name);
		Assert(node);
		Assert(node->GetType() == anxnt_inode || node->GetType() == anxnt_onode);
		//Ищем среди добавленных
		for(long j = 0; j < inOutNames; j++)
		{
			if(inOutNames[j] == node->link) break;
		}
		if(!node->link.Len() || j < inOutNames)
		{
			lb->List()->Items.Delete(i);
			continue;
		}
		lb->List()->Items[i] += "[";
		lb->List()->Items[i] += node->link;
		lb->List()->Items[i] += "]";
		inOutNames[inOutNames.Add()] = node->link;
		i++;
	}	
	//Выводим лист
	Application->ShowModal(lb);
}

//============================================================================================
//LinkFormGroup
//============================================================================================

LinkFormGroup::LinkFormGroup(AnxOptions & options, GUIPoint point, LinkData & _data, string & toNode) : GUIWindow(null, 0, 0, 1000, 100)
{
	//Создаём окно формы линка
	form = NEW LinkForm(this, options, _data, GUIPoint(0, 40));
	//Устанавливаем высоту
	DrawRect.h = form->GetDrawRect().h + 40;
	//Расчитываем позицию
	SetScreenCenter();
	//Устанавливаем окно поверх
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	//Расставляем элементы
	//Ноды
	lb = NEW GUILabel(this, 10, 10, DrawRect.w - 20, 28);
	lb->Caption = "Link connection to node [";
	lb->Caption += toNode;
	lb->Caption += "]";
	lb->Layout = GUILABELLAYOUT_Left;
}

LinkFormGroup::~LinkFormGroup()
{

}

//============================================================================================

//Рисование
void LinkFormGroup::Draw()
{
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем панель
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xefe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	GUIRectangle lbr = lb->GetDrawRect();
	lb->ClientToScreen(lbr);
	GUIHelper::Draw2DRectAlpha(lbr.x - 3, lbr.y - 3, lbr.w + 6, lbr.h + 6, 0x3f0000ff);
	GUIControl::Draw();
}


//============================================================================================
//LinkForm
//============================================================================================

LinkForm::LinkForm(GUIWindow * parent, AnxOptions & options, LinkData & _data, GUIPoint pos) : GUIControl(parent), data(_data), opt(options)
{
	AnxToolEdit * tmpEdit;
	//Позиция и размеры
	DrawRect.pos = pos;
	DrawRect.size = GUIPoint(680, 410);
	ClientRect = DrawRect;
	//Название
	GUILabel * lb = NEW GUILabel(this, 10, 10, 340, 28);
	lb->Caption = "Link name (animation command):";
	lb->Layout = GUILABELLAYOUT_Left;	
	editName = NEW GUIComboBox(this, 10, 40, 300, 20, 
							(NEW AnxToolButton(null, null, 10, 10, 10, 10))->NoShadow(), 
							NEW AnxToolEdit(null, 10, 10, 100, 20), 
							NEW AnxToolListBox::LB(null, 100, 800));
	GUIRectangle lrect;
	lrect = editName->ListBox->GetDrawRect();
	lrect.h = 600;
	editName->ListBox->SetDrawRect(lrect);
	editName->ListBox->SetClientRect(lrect);
	editName->bUserCanTypeText = true;
	editName->Edit->Text = data.name;
	editName->ListBox->Items.Add("");
	for(long n = 0; n < opt.linkCommands; n++)
	{
		editName->ListBox->Items.Add(opt.linkCommands[n]);
	}
	//Установить дефолтовый линк
	flgDef = NEW GUICheckBox(this, 360, 40, 200, 20);
	flgDef->Caption = "Default link";
	flgDef->ImageChecked->Load("AnxEditor\\Anxscheckbox_ch");
	flgDef->ImageNormal->Load("AnxEditor\\Anxscheckbox_uch");
	flgDef->Checked = data.defaultLink;	
	flgDef->OnChange = (CONTROL_EVENT)&LinkForm::EvtChangeDef;
	//Время блендинга
	lb = NEW GUILabel(this, 10, 90, 100, 28);
	lb->Caption = "Blend time:";
	lb->Layout = GUILABELLAYOUT_Left;
	tmpEdit = NEW AnxToolEdit(this, 120, 94, 100, 20);
	tmpEdit->floatValue = &data.blendTime;
	tmpEdit->min = 0.0f;
	tmpEdit->max = 1000.0f;
	tmpEdit->UpdateText();
	//Синхронизация
	lb = NEW GUILabel(this, 360, 90, 210, 28);
	lb->Caption = "Synchronization position:";
	lb->Layout = GUILABELLAYOUT_Left;
	flgSync = NEW GUICheckBox(this, 520, 94, 20, 20);
	flgSync->Caption = "";
	flgSync->ImageChecked->Load("AnxEditor\\Anxcheckbox_ch");
	flgSync->ImageNormal->Load("AnxEditor\\Anxcheckbox_uch");
	flgSync->Checked = data.isSync;
	flgSync->OnChange = (CONTROL_EVENT)&LinkForm::EvtChangeSync;
	editSync = NEW AnxToolEdit(this, 350 + 210, 94, 100, 20);
	editSync->floatValue = &data.syncPos;
	editSync->min = 0.0f;
	editSync->max = 100.0f;
	editSync->UpdateText();
	editSync->Visible = data.isSync;
	//Установим области активизации линка
	long x = 0;
	long y = 150;
	lb = NEW GUILabel(this, x + 10, y, 210, 28);
	lb->Caption = "Activate link frames ranges:";
	lb->Layout = GUILABELLAYOUT_Left;
	y += 40;
	for(long i = 0; i < 1; i++, y += 30)
	{
		lb = NEW GUILabel(this, x + 10, y, 40, 28);
		lb->Caption = "From:";
		lb->Layout = GUILABELLAYOUT_Left;
		tmpEdit = NEW AnxToolEdit(this, x + 70, y + 4, 100, 20);
		tmpEdit->longValue = &data.aranges[0];
		tmpEdit->min = 0.0f;
		tmpEdit->max = 100000.0f;
		tmpEdit->UpdateText();
		editAct[0] = tmpEdit;
		lb = NEW GUILabel(this, x + 180, y, 40, 28);
		lb->Caption = "to:";
		lb->Layout = GUILABELLAYOUT_Left;
		tmpEdit = NEW AnxToolEdit(this, x + 210, y + 4, 100, 20);
		tmpEdit->longValue = &data.aranges[1];
		tmpEdit->min = 0.0f;
		tmpEdit->max = 100000.0f;
		tmpEdit->UpdateText();
		editAct[1] = tmpEdit;
		if(data.aranges[0] > data.aranges[1])
		{
			editAct[0]->isAccept = false;
			editAct[1]->isAccept = false;
		}
	}
	//Установим области активизации линка
	x = 350;
	y = 150;
	lb = NEW GUILabel(this, x + 10, y, 210, 28);
	lb->Caption = "Move from link frames ranges:";
	lb->Layout = GUILABELLAYOUT_Left;
	for(i = 0, y += 40; i < 1; i++, y += 30)
	{
		lb = NEW GUILabel(this, x + 10, y, 40, 28);
		lb->Caption = "From:";
		lb->Layout = GUILABELLAYOUT_Left;
		tmpEdit = NEW AnxToolEdit(this, x + 70, y + 4, 100, 20);
		tmpEdit->longValue = &data.mranges[0];
		tmpEdit->min = 0.0f;
		tmpEdit->max = 100000.0f;
		tmpEdit->UpdateText();
		editMove[0] = tmpEdit;
		lb = NEW GUILabel(this, x + 180, y, 40, 28);
		lb->Caption = "to:";
		lb->Layout = GUILABELLAYOUT_Left;
		tmpEdit = NEW AnxToolEdit(this, x + 210, y + 4, 100, 20);
		tmpEdit->longValue = &data.mranges[1];
		tmpEdit->min = 0.0f;
		tmpEdit->max = 100000.0f;
		tmpEdit->UpdateText();
		editMove[1] = tmpEdit;
		if(data.mranges[0] > data.mranges[1])
		{
			editMove[0]->isAccept = false;
			editMove[1]->isAccept = false;
		}
	}
	//Кнопки
	long bw = 120;
	long bh = 32;
	long bx = (DrawRect.w - bw*2 - 20)/2;
	long by = DrawRect.h - bh - 10;
	AnxToolButton * b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Ok";
	b->Glyph->Load("AnxEditor\\AnxOk");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&LinkForm::EvtOk;
	bx += bw + 20;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Cancel";
	b->Glyph->Load("AnxEditor\\AnxCancel");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&LinkForm::EvtCancel;
}

LinkForm::~LinkForm()
{

}

void LinkForm::OnCreate()
{
	editName->SetFocus();
	editName->Edit->SelectText(0, editName->Edit->Text.Len());
}

void LinkForm::Draw()
{
	
	GUIControl::Draw();
}

//============================================================================================

void _cdecl LinkForm::EvtOk(GUIControl * sender)
{
	//Проверяем диапазоны
	for(long i = 0; i < 1; i++)
	{
		if(data.aranges[0] > data.aranges[1])
		{
			editAct[0]->isAccept = editAct[1]->isAccept = false;
		}
		if(data.mranges[0] > data.mranges[1])
		{
			editMove[0]->isAccept = editMove[1]->isAccept = false;
		}
	}
	for(long i = 0; i < 1; i++)
	{
		if(!editAct[0]->isAccept || !editAct[1]->isAccept) return;
		if(!editMove[0]->isAccept || !editMove[1]->isAccept) return;
	}
	//Сохраняем параметры
	data.name = editName->Edit->Text;
	//Сообщаем, что всё отредактировали
	eventOk.Execute(this);
	GUIWindow * win = (GUIWindow *)GetParent();
	win->Application->Close(win);
}

void _cdecl LinkForm::EvtCancel(GUIControl * sender)
{
	eventCancel.Execute(this);
	GUIWindow * win = (GUIWindow *)GetParent();
	win->Application->Close(win);
}

void _cdecl LinkForm::EvtChangeSync(GUIControl * sender)
{
	data.isSync = flgSync->Checked;
	editSync->Visible = data.isSync;	
}

void _cdecl LinkForm::EvtChangeDef(GUIControl * sender)
{
	data.defaultLink = flgDef->Checked;
}




