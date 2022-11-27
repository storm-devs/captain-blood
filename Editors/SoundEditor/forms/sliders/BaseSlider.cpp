

#include "BaseSlider.h"


BaseSlider::BaseSlider(GUIControl * parent, const GUIRectangle & r) : GUIControl(parent)
{
	SetClientRect(r);
	SetDrawRect(r);
	minValue = 0;
	maxValue = 100000;
	current = 0;
	pageSize = 5000;
	isAutoHide = true;
	isVertical = true;
	isSnapSlider = false;	
	isInverse = false;
	isRestoreInitial = false;
	position = 0.0f;
	minPosition = 0;
	dltPosition = 1;
	centerStatic = 0;
	sliderSize = 1;
	sliderStatic = 1;
	sliderPosition = 0;
	deltaDrag = 0;
	
}

BaseSlider::~BaseSlider()
{
	if(options->dndControl == this)
	{
		options->dndControl = null;
	}
}

//Установить диапазон в котором работает слайдер
void BaseSlider::SetRange(long min, long max)
{
	if(min <= max)
	{
		minValue = min;
		maxValue = max;
	}else{
		minValue = max;
		maxValue = min;
	}
	SetPosition(current);
	SetInitialPosition(initPosition);
	OnUpdateParameters();
}

//Установить размер страницы (влияет на размер бегунка)
void BaseSlider::SetPageSize(long size)
{
	pageSize = size;
	if(pageSize < 1) pageSize = 1;
	OnUpdateParameters();
}

//Установить позицию
void BaseSlider::SetPosition(long pos, bool enableRestoreInitial)
{
	current = pos;
	if(current < minValue) current = minValue;
	if(current > maxValue) current = maxValue;
	if(enableRestoreInitial)
	{
		SetInitialPosition(current);
		SetRestore(true);
	}
	OnUpdateParameters();
}

//Установить инициальную позицию
void BaseSlider::SetInitialPosition(long pos)
{
	initPosition = pos;
	if(initPosition < minValue) initPosition = minValue;
	if(initPosition > maxValue) initPosition = maxValue;
}

//Получить текущую позицию
long BaseSlider::GetPosition()
{
	return current;
}

//Установить режим автоматического скрывания если размер страницы больше или равен диапазону
void BaseSlider::SetAutoHideMode(bool isEnable)
{
	isAutoHide = isEnable;
	OnUpdateParameters();
}

//Установить вертикальную ориентацию
void BaseSlider::SetVertical()
{
	isVertical = true;
	OnUpdateParameters();
}

//Установить горизонтальную ориентацию
void BaseSlider::SetHorisontal()
{
	isVertical = false;
	OnUpdateParameters();
}

//Установить привязку бегунка к странице
void BaseSlider::SetSnapSlider(bool isEnable)
{
	isSnapSlider = isEnable;
	OnUpdateParameters();
}

//Установить инвертированную связь значения и бегунка
void BaseSlider::SetInverse(bool isEnable)
{
	isInverse = isEnable;
	OnUpdateParameters();
}

//Разрешить востонавливать значение по правой кнопке мыши
void BaseSlider::SetRestore(bool isEnable)
{
	isRestoreInitial = isEnable;
}

//Рисование
void BaseSlider::Draw()
{
	if(!Visible)
	{
		if(options->dndControl == this)
		{
			options->dndControl = null;
		}
		return;
	}
	if(!Enabled)
	{
		if(options->dndControl == this)
		{
			options->dndControl = null;
		}
	}
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	dword bcolor = Enabled ? options->black2Bkg[0] : options->black2Bkg[5];
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->black2Bkg[Enabled ? 5 : 7]);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, bcolor);
	CalcSliderRect(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg2White[Enabled ? 4 : 1]);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, bcolor);
}

//Сообщения
bool BaseSlider::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	if(!Visible || !Enabled)
	{
		if(options->dndControl == this)
		{
			options->dndControl = null;
		}
		return true;
	}
	if(message == GUIMSG_LMB_DOWN)
	{
		GUIPoint cursor_pos;
		GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
		if(MouseDown(GUIPoint(cursor_pos)))
		{
			return true;
		}		
	}
	if(message == GUIMSG_RMB_DOWN)
	{
		GUIPoint cursor_pos;
		GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
		if(RMouseDown(GUIPoint(cursor_pos)))
		{
			return true;
		}		
	}
	return GUIControl::ProcessMessages(message, lparam, hparam);
}

//Нажатие кнопки мыши
bool BaseSlider::MouseDown(const GUIPoint& pt)
{
	if(!Visible || !Enabled) return false;
	if(options->dndControl)
	{
		return false;
	}
	GUIRectangle r = GetClientRect();	
	ClientToScreen(r);
	if(!r.Inside(pt))
	{
		return false;
	}
	CalcSliderRect(r);
	if(!r.Inside(pt))
	{		
		deltaDrag = 0;
		MoveSliderTo(pt);
		CalcSliderRect(r);
	}
	deltaDrag.x = pt.x - (r.x + r.w/2);
	deltaDrag.y = pt.y - (r.y + r.h/2);
	options->dndControl = this;
	return true;
}

//Нажатие кнопки мыши
void BaseSlider::MouseUp(int button, const GUIPoint& pt)
{
	if(!Visible || !Enabled) return;
	if(options->dndControl != this)
	{
		return;
	}
	MoveSliderTo(pt);
	options->dndControl = null;
}

//Перемещение мыши
void BaseSlider::MouseMove(int button, const GUIPoint& pt)
{
	if(!Visible || !Enabled) return;
	if(options->dndControl != this)
	{
		return;
	}
	MoveSliderTo(pt);
}

//Нажатие правой кнопкой мыши
bool BaseSlider::RMouseDown(const GUIPoint& pt)
{
	if(!Visible || !Enabled) return false;
	if(!isRestoreInitial)
	{
		return false;
	}
	if(options->dndControl)
	{
		return false;
	}
	GUIRectangle r = GetClientRect();	
	ClientToScreen(r);
	if(!r.Inside(pt))
	{
		return false;
	}
	CalcSliderRect(r);
	if(r.Inside(pt))
	{
		RestoreInitial();
		return true;
	}
	return false;
}

//Правый шелчёк на бегунке
void BaseSlider::RestoreInitial()
{
	SetPosition(initPosition);
}

//Обновились параметры слайдера
void BaseSlider::OnUpdateParameters()
{
	long delta = maxValue - minValue;
	if(delta > 0)
	{
		position = Clampf(float(current - minValue)/float(delta));
		if(isInverse)
		{
			position = 1.0f - position;
		}
	}else{
		position = 0.0f;
	}
	//Размер бара без коррекции размером бегунка
	GUIRectangle r = GetDrawRect();
	if(isVertical)
	{
		minPosition = 0;
		dltPosition = r.h;
		centerStatic = r.w/2;
		sliderStatic = r.w;
	}else{
		minPosition = 0;
		dltPosition = r.w;
		centerStatic = r.h/2;
		sliderStatic = r.h;
	}
	//Размер бегунка
	if(delta > 0)
	{
		sliderSize = pageSize*dltPosition/delta;
	}else{
		sliderSize = dltPosition;
	}	
	if(sliderSize < 5) sliderSize = 5;
	if(sliderSize > dltPosition) sliderSize = dltPosition;
	if(pageSize > 0 && maxValue > minValue)
	{
		sliderStep = float(pageSize)/float(maxValue - minValue);
	}
	//Корректируем длинну и начало отрезка перемещения
	minPosition = sliderSize/2;
	dltPosition -= sliderSize;
	//Проверяем видимость
	if(isAutoHide)
	{
		if(pageSize < maxValue - minValue)
		{
			Visible = true;
			Enabled = true;
		}else{
			Visible = false;
			Enabled = false;
		}
	}
	OnUpdatePosition();
}

//Получить прямоугольник бегунка
void BaseSlider::CalcSliderRect(GUIRectangle & r)
{
	r = GetDrawRect();
	ClientToScreen(r);
	if(isVertical)
	{
		r.x += centerStatic - sliderStatic/2;
		r.y += sliderPosition - sliderSize/2;
		r.h = sliderSize;
	}else{
		r.x += sliderPosition - sliderSize/2;
		r.w = sliderSize;
		r.y += centerStatic - sliderStatic/2;
	}
}

//Переместить бегунок в ближнюю точку к заданной
void BaseSlider::MoveSliderTo(const GUIPoint & pt)
{
	GUIPoint pnt;
	pnt.x = pt.x - deltaDrag.x;
	pnt.y = pt.y - deltaDrag.y;
	GUIRectangle r;
	r = GetDrawRect();
	ClientToScreen(r);
	long pos = 0;
	if(isVertical)
	{
		pos = pnt.y - (r.y + minPosition);
	}else{
		pos = pnt.x - (r.x + minPosition);
	}
	if(pos > dltPosition) pos = dltPosition;
	if(pos < 0) pos = 0;	
	position = Clampf(float(pos)/float(dltPosition));	
	if(!isInverse)
	{
		current = long((maxValue - minValue)*GetSnapPosition() + 0.5f + minValue);
	}else{
		current = long((maxValue - minValue)*(1.0f - GetSnapPosition()) + 0.5f + minValue);
	}
	OnUpdatePosition();
}

//Получить приклееную позицию
float BaseSlider::GetSnapPosition()
{
	float pos = position;
	if(isSnapSlider && sliderStep > 0.0f)
	{
		pos = long(position/sliderStep + 0.5f)*sliderStep;
	}
	return pos;
}

//Обновить позицию
void BaseSlider::OnUpdatePosition()
{
	sliderPosition = long(minPosition + dltPosition*GetSnapPosition());
	onUpdate.Execute(this);
}




