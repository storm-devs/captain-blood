//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxEditor
//============================================================================================
// NodeAnimationEventsForm
//============================================================================================

#include "NodeAnimationEventsForm.h"

//============================================================================================

NodeAnimationEventsForm::NodeAnimationEventsForm(AnxOptions & options, GraphNodeAnimation::Event & _event) : GUIWindow(null, 0, 0, 800, 110), opt(options), event(_event), params(_FL_)
{
	//Устанавливаем окно поверх
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	SetScreenCenter();
	//Кнопки выхода
	long bw = 120;
	long bh = 32;
	long bx = (DrawRect.w - bw*2 - 20)/2;
	long by = DrawRect.h - bh - 10;
	buttonOk = NEW AnxToolButton(null, this, bx, by, bw, 32);
	buttonOk->Caption = "Ok";
	buttonOk->Glyph->Load("AnxEditor\\AnxOk");
	buttonOk->Align = GUIAL_Center;
	buttonOk->OnMousePressed = (CONTROL_EVENT)&NodeAnimationEventsForm::EvtOk;
	bx += bw + 20;
	buttonCancel = NEW AnxToolButton(null, this, bx, by, bw, 32);
	buttonCancel->Caption = "Cancel";
	buttonCancel->Glyph->Load("AnxEditor\\AnxCancel");
	buttonCancel->Align = GUIAL_Center;
	buttonCancel->OnMousePressed = (CONTROL_EVENT)&NodeAnimationEventsForm::EvtCancel;
	//Список типов
	GUILabel * lb = NEW GUILabel(this, 10, 10, 180, 28);
	lb->Caption = "Selecet event type:";
	lb->Layout = GUILABELLAYOUT_Left;
	forms = NEW GUIComboBox(this, 190, 14, DrawRect.w - 200, 20, 
							(NEW AnxToolButton(null, null, 10, 10, 10, 10))->NoShadow(), 
							NEW AnxToolEdit(null, 10, 10, 100, 20), 
							NEW AnxToolListBox::LB(null, 100, 100));
	for(long i = 0, ei = 0; i < opt.eventForms; i++)
	{		
		forms->ListBox->Items.Add(opt.eventForms[i]->descr);
		if(event.name == opt.eventForms[i]->name) ei = i;
	}
	forms->OnSelect = (CONTROL_EVENT)&NodeAnimationEventsForm::EvtSelectType;
	if(opt.eventForms > 0)
	{
		forms->SelectItem(ei);
	}else{
		forms->ListBox->SelectedLine = -1;
	}
}

NodeAnimationEventsForm::~NodeAnimationEventsForm()
{
	for(long i = 0; i < params; i++)
	{
		if(params[i]->edit) params[i]->edit->stringValue = null;
		delete params[i];
	}
}

//============================================================================================

//Рисование
void NodeAnimationEventsForm::Draw()
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

void _cdecl NodeAnimationEventsForm::EvtSelectType(GUIControl * sender)
{
	//Удаляем старые элементы
	for(long i = 0; i < params; i++)
	{		
		if(params[i]->label) delete params[i]->label;
		if(params[i]->edit)
		{
			params[i]->edit->stringValue = null;
			delete params[i]->edit;
		}
		if(params[i]->combo) delete params[i]->combo;
		delete params[i];
	}
	params.Empty();
	name.Empty();
	//Индекс формы
	long idx = forms->ListBox->SelectedLine;
	if(idx >= opt.eventForms) idx = -1;
	//Создаём новые элементы
	long y = 50;
	if(idx >= 0)
	{		
		name = opt.eventForms[idx]->name;
		for(i = 0; i < opt.eventForms[idx]->params; i++)
		{
			Param * prm = NEW Param();
			prm->label = null;
			prm->edit = null;
			prm->combo = null;
			prm->value = "";
			bool isAdd = false;
			if(opt.eventForms[idx]->params[i]->name.Len() > 0)
			{
				prm->label = NEW GUILabel(this, 10, y, DrawRect.w - 20, 28);
				prm->label->Caption = opt.eventForms[idx]->params[i]->name;
				prm->label->Layout = GUILABELLAYOUT_Left;
				isAdd = true;
				y += 30;
			}
			if(opt.eventForms[idx]->params[i]->enumValue > 0)
			{
				//Создаём элемент
				prm->combo = NEW GUIComboBox(this, 10, y + 4, DrawRect.w - 20, 20, 
										(NEW AnxToolButton(null, null, 10, 10, 10, 10))->NoShadow(), 
										NEW AnxToolEdit(null, 10, 10, 100, 20), 
										NEW AnxToolListBox::LB(null, 100, 100));
				prm->combo->OnChange = (CONTROL_EVENT)&NodeAnimationEventsForm::EvtChangeComboValue;
				//Заполняем поля
				long count = opt.eventForms[idx]->params[i]->enumValue;
				for(long n = 0, ei = 0; n < count; n++)
				{
					string & str = opt.eventForms[idx]->params[i]->enumValue[n];
					prm->combo->ListBox->Items.Add(str);
					if(i < event.params)
					{
						if(*event.params[i] == str)
						{
							ei = n;
						}
					}
				}
				prm->combo->SelectItem(ei);
				prm->value = opt.eventForms[idx]->params[i]->enumValue[ei];
				isAdd = true;
				y += 30;
			}else{
				if(opt.eventForms[idx]->params[i]->value.Len() == 0)
				{
					prm->edit = NEW AnxToolEdit(this, 10, y + 4, DrawRect.w - 20, 20);
					if(i < event.params)
					{
						prm->edit->Text = *event.params[i];
						prm->value = *event.params[i];
					}
					prm->edit->stringValue = &prm->value;
					isAdd = true;
					y += 30;
				}else{
					if(prm->label) delete prm->label; prm->label = null;
					prm->edit = null;
					prm->value = opt.eventForms[idx]->params[i]->value;
					if(i < event.params) prm->value = *event.params[i];
				}
			}
			if(isAdd) y += 4;
			params.Add(prm);
		}
	}
	y += 16;
	//Пересчитываем размеры	
	GUIRectangle rect;
	rect = buttonOk->GetDrawRect(); rect.y = y; buttonOk->SetDrawRect(rect);
	rect = buttonOk->GetClientRect(); rect.y = y; buttonOk->SetClientRect(rect);
	rect = buttonCancel->GetDrawRect(); rect.y = y; buttonCancel->SetDrawRect(rect);
	rect = buttonCancel->GetClientRect(); rect.y = y; buttonCancel->SetClientRect(rect);
	DrawRect.h = y + rect.h + 10;
	ClientRect.h = y + rect.h + 10;
	SetScreenCenter();
}

void _cdecl NodeAnimationEventsForm::EvtOk(GUIControl * sender)
{
	//Сохраняем изменения
	if(name.Len())
	{
		event.name = name;
		for(long i = 0; i < event.params; i++) delete event.params[i];
		event.params.Empty();
		for(i = 0; i < params; i++)
		{
			event.params.Add(NEW string(params[i]->value));
		}
	}
	for(long i = 0, ei = 0; i < opt.eventForms; i++)
	{
		if(event.name == opt.eventForms[i]->name)
		{
			event.noBuffered = opt.eventForms[i]->noBuffered;
		}
	}
	Application->Close(this);
}

void _cdecl NodeAnimationEventsForm::EvtCancel(GUIControl * sender)
{
	Application->Close(this);
}

void _cdecl NodeAnimationEventsForm::EvtChangeComboValue(GUIControl * sender)
{
	for(long i = 0; i < params; i++)
	{
		if(params[i]->combo)
		{
			params[i]->value = params[i]->combo->Edit->Text;			
		}
	}
}

