


#include "NumberSlider.h"




NumberSlider::NumberSlider(GUIControl * parent, const GUIRectangle & r) : BaseSlider(parent, r), markerLines(_FL_)
{
	constSliderSize = 16;
	isAutoHide = false;
	disableDrawText = false;
	disableMarkers = false;
}

NumberSlider::~NumberSlider()
{

}

//Установить фиксированный размер бегунка
void NumberSlider::SetSliderSize(dword size)
{
	constSliderSize = size;
	OnUpdateParameters();
}

//Установить название слайдера
void NumberSlider::SetName(const char * str)
{
	Caption = str; 
	name.Init(Caption.c_str());
}

//Запретить отрисовку текста
void NumberSlider::DisableDrawText(bool isDisable)
{
	disableDrawText = isDisable;
}

//Запретить отрисовку шкалы
void NumberSlider::DisableMarkers(bool isDisable)
{
	disableMarkers = isDisable;
	OnUpdateParameters();
}

//Рисование
void NumberSlider::Draw()
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
	//Прямоугольник контрола
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	GUIRectangle r;
	//Рисуем линейку-подложку
	if(isVertical)
	{
		long w = c_bkg_line_width_in_percents*rect.w/100;
		if(w < 2) w = 2;
		r.x = rect.x + (rect.w - w)/2;
		r.y = rect.y;
		r.w = w;
		r.h = rect.h;
	}else{
		long h = c_bkg_line_width_in_percents*rect.h/100;
		if(h < 2) h = 2;
		r.x = rect.x;
		r.y = rect.y + (rect.h - h)/2;
		r.w = rect.w;
		r.h = h;
	}
	dword colorFrame = Enabled ? options->black2Bkg[0] : options->black2Bkg[4];
	dword colorBkg = Enabled ? options->black2Bkg[4] : options->black2Bkg[7];	
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, colorBkg);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, colorFrame);
	//Рисуем шкалу
	if(markerLines.Size() > 1)
	{
		GUIHelper::Draw2DLines(markerLines.GetBuffer(), markerLines.Size(), colorFrame, rect.x, rect.y);
	}
	//Рисуем бегунок
	GUIPoint center = rect.pos;
	if(isVertical)
	{
		center.x += centerStatic;
		center.y += sliderPosition;
	}else{
		center.x += sliderPosition;
		center.y += centerStatic;
	}
	GUIHelper::DrawPolygon(sliderPoints, 21, options->black2Bkg[6], center.x, center.y);
	GUIHelper::DrawLines(sliderPoints + 1, 20, colorFrame, center.x, center.y);
	if(!disableDrawText)
	{
		DrawText(rect, isVertical);
	}
}

//Подписать
void NumberSlider::DrawText(const GUIRectangle & rect, bool isVerticalText)
{
	//Пишем надписи	
	if(name.len > 0)
	{
		if(isVerticalText)
		{
			float x = rect.x + (rect.w - name.w)*0.5f;
			float y = rect.y - name.h - 5.0f;
			options->render->Print(x, y, options->colorTextLo, name.str);
		}else{
			float x = rect.x - name.w - 5.0f;
			float y = rect.y + (rect.h - name.h)*0.5f;
			options->render->Print(x, y, options->colorTextLo, name.str);
		}
	}
	if(posString.len > 0)
	{
		if(isVerticalText)
		{
			float x = rect.x + (rect.w - posString.w)*0.5f;
			float y = rect.y + rect.h + 5.0f;
			options->render->Print(x, y, options->colorTextLo, posString.str);
		}else{
			float x = rect.x + rect.w + 5.0f;
			float y = rect.y + (rect.h - posString.h)*0.5f;
			options->render->Print(x, y, options->colorTextLo, posString.str);
		}
	}
}

//Обновились параметры слайдера
void NumberSlider::OnUpdateParameters()
{
	//Посчитаем параметры по умолчанию
	BaseSlider::OnUpdateParameters();
	//Скорректируем под новый бегунок
	if(constSliderSize > 0)
	{
		minPosition = minPosition - sliderSize/2 + constSliderSize/2;
		dltPosition = dltPosition + sliderSize - constSliderSize;
		sliderSize = constSliderSize;
		sliderStatic = (sliderStatic*c_slider_size_in_percents)/100;
	}
	//Заполняем данные для вертикального бегунка
	sliderPoints[0].x = 0;
	sliderPoints[0].y = 0;
	sliderPoints[1].x = 0;
	sliderPoints[1].y = -(sliderSize/2 - 1);
	sliderPoints[2].x = sliderStatic/2 - 2;
	sliderPoints[2].y = -(sliderSize/2);
	sliderPoints[3].x = sliderStatic/2;
	sliderPoints[3].y = -(sliderSize/2 - 2);
	sliderPoints[4].x = sliderStatic/2;
	sliderPoints[4].y = -(sliderSize/2 - 2);
	sliderPoints[5].x = sliderStatic/2 - 2;
	sliderPoints[5].y = 0;
	for(long i = 0; i < 5; i++)
	{
		sliderPoints[i + 6].x = sliderPoints[5 - i].x;
		sliderPoints[i + 6].y = -sliderPoints[5 - i].y;
	}
	for(long i = 0; i < 10; i++)
	{
		sliderPoints[i + 11].x = -sliderPoints[10 - i].x;
		sliderPoints[i + 11].y = sliderPoints[10 - i].y;
	}
	if(!isVertical)
	{
		for(long i = 0; i < 21; i++)
		{
			long x = -sliderPoints[i].y;
			long y = sliderPoints[i].x;
			sliderPoints[i].x = x;
			sliderPoints[i].y = y;
		}
	}
	//Заполняем шкалу
	markerLines.Empty();
	long markersCount = (maxValue - minValue)/pageSize + 1;
	if(markersCount > 1 && !disableMarkers)
	{
		GUIRectangle rect = GetDrawRect();
		long mark_line_size = 0;
		if(isVertical)
		{
			mark_line_size = (rect.w*c_mark_line_size_in_percents)/100;
			if(markersCount > rect.h/2) markersCount = rect.h/2;
		}else{
			mark_line_size = (rect.h*c_mark_line_size_in_percents)/100;
			if(markersCount > rect.w/2) markersCount = rect.w/2;
		}
		if(markersCount < 2) markersCount = 2;
		markerLines.AddElements(markersCount*4);
		for(long i = 0; i < markersCount; i++)
		{
			//Маркеры
			GUIPoint & p1 = markerLines[i*4 + 0];
			GUIPoint & p2 = markerLines[i*4 + 1];
			GUIPoint & p3 = markerLines[i*4 + 2];
			GUIPoint & p4 = markerLines[i*4 + 3];
			long pos = minPosition + (i*dltPosition)/(markersCount - 1);
			if(isVertical)
			{
				p1.x = 0; p1.y = pos;
				p2.x = mark_line_size; p2.y = pos;
				p3.x = rect.w - mark_line_size; p3.y = pos;
				p4.x = rect.w; p4.y = pos;
			}else{
				p1.x = pos; p1.y = 0;
				p2.x = pos; p2.y = mark_line_size;
				p3.x = pos; p3.y = rect.h - mark_line_size;
				p4.x = pos; p4.y = rect.h;
			}
		}
	}
	OnUpdatePosition();
}

//Обновить позицию
void NumberSlider::OnUpdatePosition()
{
	BaseSlider::OnUpdatePosition();
	if(!onChangeText.GetClass())
	{
		posString.data.Empty();
		posString.data += current;
		posString.Init(posString.data.c_str());		
	}else{
		onChangeText.Execute(this);
	}
}

