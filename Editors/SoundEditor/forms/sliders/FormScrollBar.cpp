
#include "FormScrollBar.h"
#include "..\sliders\BaseSlider.h"
#include "..\FormButton.h"


//Базовая нормалитзованная стрелка справа на лево
Vector FormScrollBar::base[4] = {Vector(-0.15f,  0.0f, 0.0f), 
								 Vector(-0.3f,  0.3f, 0.0f),
								 Vector( 0.2f,  0.0f, 0.0f),
								 Vector(-0.3f, -0.3f, 0.0f)};


FormScrollBar::FormScrollBar(GUIControl * parent, const GUIRectangle & r) : GUIControl(parent)
{
	buttonStep = 1000;
	state = s_none;
	SetClientRect(r);
	SetDrawRect(r);
	GUIRectangle sr;
	if(r.w >= r.h)
	{
		slider = NEW BaseSlider(this, GUIRectangle(r.h + 1, 0, r.w - 2*r.h - 2, r.h));
		slider->SetHorisontal();
		btDec = NEW FormButton(this, GUIRectangle(0, 0, r.h, r.h));		
		btInc = NEW FormButton(this, GUIRectangle(r.w - r.h, 0, r.h, r.h));		
		for(long i = 0; i < 4; i++)
		{
			incArrow[i].x = long(base[i].x*r.h);
			incArrow[i].y = long(base[i].y*r.h);
			decArrow[i].x = -incArrow[i].x;
			decArrow[i].y = incArrow[i].y;
		}
	}else{
		slider = NEW BaseSlider(this, GUIRectangle(0, r.w + 1, r.w, r.h - 2*r.w - 2));
		slider->SetVertical();
		btDec = NEW FormButton(this, GUIRectangle(0, 0, r.w, r.w));
		btInc = NEW FormButton(this, GUIRectangle(0, r.h - r.w, r.w, r.w));
		for(long i = 0; i < 4; i++)
		{
			incArrow[i].x = long(base[i].y*r.w);
			incArrow[i].y = long(base[i].x*r.w);
			decArrow[i].x = incArrow[i].x;
			decArrow[i].y = -incArrow[i].y;
		}
	}
	btDec->onDown.SetHandler(this, (CONTROL_EVENT)&FormScrollBar::Dec);
	btDec->SetDrawGlyph(this, &FormScrollBar::DrawDec);
	btInc->onDown.SetHandler(this, (CONTROL_EVENT)&FormScrollBar::Inc);
	btInc->SetDrawGlyph(this, &FormScrollBar::DrawInc);
}

FormScrollBar::~FormScrollBar()
{
}

//Установить диапазон в котором работает слайдер
void FormScrollBar::SetRange(long min, long max)
{
	slider->SetRange(min, max);
}

//Установить размер страницы (влияет на размер бегунка)
void FormScrollBar::SetPageSize(long size)
{
	slider->SetPageSize(size);
}

//Устоновить шаг для перемещения по кнопкам
void FormScrollBar::SetButtonStep(long step)
{
	buttonStep = step;
	if(buttonStep < 0) buttonStep = -buttonStep;
}

//Установить позицию
void FormScrollBar::SetPosition(long pos)
{
	slider->SetPosition(pos);
}

//Получить текущую позицию
long FormScrollBar::GetPosition()
{
	return slider->GetPosition();
}

//Установить режим автоматического скрывания если размер страницы больше или равен диапазону
void FormScrollBar::SetAutoHideMode(bool isEnable)
{
	slider->SetAutoHideMode(isEnable);
}

//Установить обработчик изменения позиции
void FormScrollBar::SetUpdateHandler(GUIControl * listener, CONTROL_EVENT evt)
{
	slider->onUpdate.SetHandler(listener, evt);
}

void FormScrollBar::Draw()
{	
	if(!slider->Visible)
	{
		return;
	}	
	if(state != s_none)
	{
		timer += api->GetNoScaleDeltaTime();
		if(state < s_no_first)
		{
			if(timer >= 0.5f)
			{
				if(state == s_dec_first)
				{
					state = s_dec;
					DecContinue();
				}else{
					state = s_inc;
					IncContinue();
				}
			}
		}else{
			if(timer >= 0.05f)
			{
				if(state == s_dec)
				{
					DecContinue();
				}else{
					IncContinue();
				}
			}
		}
	}
	GUIControl::Draw();
	//Рисуем рамку
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
	
}

//Нарисовать значёк на уменьшающей кнопке
void _cdecl FormScrollBar::DrawDec(FormButton * button, const GUIRectangle & rect)
{
	GUIHelper::DrawPolygon(decArrow, 4, options->black2Bkg[3], rect.x + rect.w/2, rect.y + rect.h/2);
}

//Нарисовать значёк на увеличивающей кнопке
void _cdecl FormScrollBar::DrawInc(FormButton * button, const GUIRectangle & rect)
{
	GUIHelper::DrawPolygon(incArrow, 4, options->black2Bkg[3], rect.x + rect.w/2, rect.y + rect.h/2);
}

//Обработать нажатие на уменьшающую кнопку
void _cdecl FormScrollBar::Dec(GUIControl* sender)
{
	state = s_dec_first;
	DecContinue();
}

//Продолжить уменьшение
void FormScrollBar::DecContinue()
{
	if(btDec->IsPressed())
	{
		timer = 0.0f;
		slider->SetPosition(slider->GetPosition() - buttonStep);		
	}else{
		state = s_none;
	}
}

//Обработать нажатие на уменьшающую кнопку
void _cdecl FormScrollBar::Inc(GUIControl* sender)
{
	state = s_inc_first;
	IncContinue();
}

//Продолжить уменьшение
void FormScrollBar::IncContinue()
{
	if(btInc->IsPressed())
	{
		timer = 0.0f;
		slider->SetPosition(slider->GetPosition() + buttonStep);
	}else{
		state = s_none;
	}
}

