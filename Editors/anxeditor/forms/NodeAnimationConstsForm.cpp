//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxEditor
//============================================================================================
// NodeAnimationConstsForm
//============================================================================================

#include "NodeAnimationConstsForm.h"
#include "NodeAnimationConstsAttackForm.h"

//============================================================================================

NodeAnimationConstsForm::NodeAnimationConstsForm(AnxOptions & options, GraphNodeAnimation::ConstBlock & _cb) : GUIWindow(null, 0, 0, 800, 110), opt(options), cb(_cb), params(_FL_)
{
	//Устанавливаем окно поверх
	errorForm = null;
	paramsForm = null;
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
	buttonOk->OnMousePressed = (CONTROL_EVENT)&NodeAnimationConstsForm::EvtOk;
	bx += bw + 20;
	buttonCancel = NEW AnxToolButton(null, this, bx, by, bw, 32);
	buttonCancel->Caption = "Cancel";
	buttonCancel->Glyph->Load("AnxEditor\\AnxCancel");
	buttonCancel->Align = GUIAL_Center;
	buttonCancel->OnMousePressed = (CONTROL_EVENT)&NodeAnimationConstsForm::EvtCancel;
	//Список типов
	GUILabel * lb = NEW GUILabel(this, 10, 10, 180, 28);
	lb->Caption = "Selecet const type:";
	lb->Layout = GUILABELLAYOUT_Left;
	forms = NEW GUIComboBox(this, 190, 14, DrawRect.w - 200, 20, 
							(NEW AnxToolButton(null, null, 10, 10, 10, 10))->NoShadow(), 
							NEW AnxToolEdit(null, 10, 10, 100, 20), 
							NEW AnxToolListBox::LB(null, 100, 100));
	for(long i = 0, ei = 0; i < opt.constsForms; i++)
	{		
		forms->ListBox->Items.Add(opt.constsForms[i]->descr);
		if(cb.type == opt.constsForms[i]->descr) ei = i;
	}
	forms->OnSelect = (CONTROL_EVENT)&NodeAnimationConstsForm::EvtSelectType;
	if(opt.constsForms > 0)
	{
		forms->SelectItem(ei);
	}else{
		forms->ListBox->SelectedLine = -1;
	}
}

NodeAnimationConstsForm::~NodeAnimationConstsForm()
{
	for(long i = 0; i < params; i++)
	{
		if(params[i]->edit)
		{
			params[i]->edit->stringValue = null;
			params[i]->edit->floatValue = null;
		}
		delete params[i];
	}
}

//============================================================================================

//Рисование
void NodeAnimationConstsForm::Draw()
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

void _cdecl NodeAnimationConstsForm::EvtSelectType(GUIControl * sender)
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
	if(errorForm) delete errorForm;
	errorForm = null;
	if(paramsForm) delete paramsForm;
	paramsForm = null;
	//Найдём название формы
	string formName = "<Not forms in list>";
	long idx = forms->ListBox->SelectedLine;
	if(idx >= 0)
	{
		formName = forms->ListBox->Items[idx];
		for(long i = 0; i < opt.constsForms; i++)
		{
			if(formName == opt.constsForms[idx]->descr)
			{
				formName = opt.constsForms[idx]->type;
				break;
			}
		}		
		Assert(i < opt.constsForms);
	}
	Assert(idx < opt.constsForms);
	//Создаём новые элементы
	long y = 50;
	if(idx >= 0 && formName == "standart")
	{
		InitStatdartForm(*opt.constsForms[idx], y);
	}else
	if(idx >= 0 && formName == "attack")
	{
		InitAttackForm(*opt.constsForms[idx], y);
	}else{
		errorForm = NEW GUILabel(this, 10, y, 180, 28);
		y += 30;
		errorForm->Caption = "Unknown consts form type: ";
		errorForm->Caption += formName;
		errorForm->Layout = GUILABELLAYOUT_Left;
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

void _cdecl NodeAnimationConstsForm::EvtOk(GUIControl * sender)
{
	//Очищаем массив констант
	for(long i = 0; i < cb.consts; i++) cb.consts[i]->type = GraphNodeAnimation::Const::t_empty;
	//Переносим данные в константы
	for(i = 0; i < params; i++)
	{
		if(i == cb.consts)
		{
			long j = cb.consts.Add(NEW GraphNodeAnimation::Const());
			Assert(j == i);
		}
		cb.consts[i]->name = params[i]->name;
		cb.consts[i]->type = params[i]->type;
		switch(params[i]->type)
		{
		case GraphNodeAnimation::Const::t_string:
			cb.consts[i]->str = params[i]->svalue;
			break;
		case GraphNodeAnimation::Const::t_float:
			cb.consts[i]->flt = params[i]->fvalue;
			break;
		case GraphNodeAnimation::Const::t_blend:
			cb.consts[i]->bld = params[i]->bvalue;
			break;
		default:
			Assert(false);
		}
	}
	cb.type = forms->Edit->Text;
	Application->Close(this);
}

void _cdecl NodeAnimationConstsForm::EvtCancel(GUIControl * sender)
{
	Application->Close(this);
}

void _cdecl NodeAnimationConstsForm::EvtChangeComboValue(GUIControl * sender)
{
	char buf[32];
	for(long i = 0; i < params; i++)
	{
		if(params[i]->combo)
		{
			switch(params[i]->type)
			{
			case GraphNodeAnimation::Const::t_string:
				params[i]->svalue = params[i]->combo->Edit->Text;
				break;
			case GraphNodeAnimation::Const::t_float:
				params[i]->fvalue = (float)atof(params[i]->combo->Edit->Text);
				crt_snprintf(buf, 32, "%g", params[i]->fvalue);
				params[i]->combo->Edit->Text = buf;
				break;
			case GraphNodeAnimation::Const::t_blend:
				params[i]->bvalue = (float)atof(params[i]->combo->Edit->Text);
				crt_snprintf(buf, 32, "%g", params[i]->bvalue);
				params[i]->combo->Edit->Text = buf;
				break;
			default:
				Assert(false);
			}
		}
	}
}

void NodeAnimationConstsForm::InitStatdartForm(AnxOptions::ConstForm & cf, long & y)
{
	Assert(params == 0);
	for(long i = 0; i < cf.params; i++)
	{
		Param * prm = NEW Param();
		prm->label = null;
		prm->edit = null;
		prm->combo = null;		
		//Имя константы
		prm->name = cf.params[i]->name;
		//Тип константы
		switch(cf.params[i]->type)
		{
		case AnxOptions::ConstForm::t_string:
			prm->type = GraphNodeAnimation::Const::t_string;
			break;
		case AnxOptions::ConstForm::t_float:
			prm->type = GraphNodeAnimation::Const::t_float;
			break;
		case AnxOptions::ConstForm::t_blend:
			prm->type = GraphNodeAnimation::Const::t_blend;
			break;
		default:
			Assert(false);
		}
		//Копируем параметры
		if(i < cb.consts)
		{
			prm->svalue = cb.consts[i]->str;
			prm->fvalue = cb.consts[i]->flt;
			prm->bvalue = cb.consts[i]->bld;
		}else{
			prm->svalue = "";
			prm->fvalue = 0.0f;
			prm->bvalue = 0.0f;
		}
		//Описание константы
		bool isAdd = false;
		if(cf.params[i]->info.Len() > 0)
		{
			prm->label = NEW GUILabel(this, 10, y, DrawRect.w - 20, 28);
			prm->label->Caption = cf.params[i]->info;
			prm->label->Layout = GUILABELLAYOUT_Left;
			isAdd = true;
			y += 30;
		}
		//Контрол ввода константы
		if(cf.params[i]->venum == 0)
		{
			prm->edit = NEW AnxToolEdit(this, 10, y + 4, DrawRect.w - 20, 20);
			prm->edit->isLimitedNumber = false;
			switch(prm->type)
			{
			case GraphNodeAnimation::Const::t_string:
				prm->edit->stringValue = &prm->svalue;
				break;
			case GraphNodeAnimation::Const::t_float:
				prm->edit->floatValue = &prm->fvalue;
				break;
			case GraphNodeAnimation::Const::t_blend:
				prm->edit->floatValue = &prm->bvalue;
				break;
			default:
				Assert(false);
			}
			prm->edit->UpdateText();
			isAdd = true;
			y += 30;
		}else{
			//Создаём элемент
			prm->combo = NEW GUIComboBox(this, 10, y + 4, DrawRect.w - 20, 20, 
									(NEW AnxToolButton(null, null, 10, 10, 10, 10))->NoShadow(), 
									NEW AnxToolEdit(null, 10, 10, 100, 20), 
									NEW AnxToolListBox::LB(null, 100, 100));
			prm->combo->OnChange = (CONTROL_EVENT)&NodeAnimationConstsForm::EvtChangeComboValue;
			//Заполняем поля
			long count = cf.params[i]->venum;
			for(long n = 0, ei = 0; n < count; n++)
			{
				//Добавляем строку
				string & str = cf.params[i]->venum[n];
				prm->combo->ListBox->Items.Add(str);
				float tmp;
				//Смотрим какой элемент выбрать
				switch(prm->type)
				{
				case GraphNodeAnimation::Const::t_string:
					if(prm->svalue == str) ei = n;
					break;
				case GraphNodeAnimation::Const::t_float:
					tmp = (float)atof(str);
					if(fabs(prm->fvalue - tmp) < 1e-34f) ei = n;
					break;
				case GraphNodeAnimation::Const::t_blend:
					tmp = (float)atof(str);
					if(fabs(prm->bvalue - tmp) < 1e-34f) ei = n;
					break;
				default:
					Assert(false);
				}
			}
			prm->combo->SelectItem(ei);
			isAdd = true;
			y += 30;
		}
		if(isAdd) y += 4;
		params.Add(prm);
	}
	EvtChangeComboValue(null);
}

void NodeAnimationConstsForm::InitAttackForm(AnxOptions::ConstForm & cf, long & y)
{
	//Параметры
	Assert(params == 0);
	const char * table[] = 
	{
		"angle.start",
		"angle.end",
		"radius.start",
		"radius.end",
	};
	for(long i = 0; i < ARRSIZE(table); i++)
	{
		Param * prm = NEW Param();
		prm->name = table[i];
		prm->label = null;
		prm->edit = null;
		prm->combo = null;
		prm->type = GraphNodeAnimation::Const::t_float;
		prm->svalue.Empty();
		prm->fvalue = 0.0f;
		prm->bvalue = 0.0f;
		if(i < cb.consts)
		{
			prm->fvalue = cb.consts[i]->flt;
		}
		params.Add(prm);
	}
	//Форма элемента
	dword size = 600;
	y += 20;
	NodeAnimationConstsAttackForm * ctrl = NEW NodeAnimationConstsAttackForm(opt, this, GUIPoint((DrawRect.w - size)/2, y), size);
	if(cb.consts <= 0) params[0]->fvalue = -30.0f;
	if(cb.consts <= 1) params[1]->fvalue = 30.0f;
	if(cb.consts <= 2) params[2]->fvalue = 0.6f;
	if(cb.consts <= 3) params[3]->fvalue = 2.0f;
	ctrl->angleStart = &params[0]->fvalue;
	ctrl->angleEnd = &params[1]->fvalue;
	ctrl->radiusStart = &params[2]->fvalue;
	ctrl->radiusEnd = &params[3]->fvalue;
	y += ctrl->GetDrawRect().h;
	Assert(!paramsForm);
	paramsForm = ctrl;
	y += 20;
}



