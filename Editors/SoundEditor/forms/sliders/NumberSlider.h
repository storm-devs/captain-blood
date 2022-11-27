
#pragma once

#include "BaseSlider.h"



class NumberSlider : public BaseSlider
{
	enum Consts
	{
		c_bkg_line_width_in_percents = 20,
		c_mark_line_size_in_percents = 15,
		c_slider_size_in_percents = 84,
	};

public:

	NumberSlider(GUIControl * parent, const GUIRectangle & r);
	virtual ~NumberSlider();

public:
	//Установить фиксированный размер бегунка
	void SetSliderSize(dword size);
	//Установить название слайдера
	void SetName(const char * str);
	//Запретить отрисовку текста
	void DisableDrawText(bool isDisable);
	//Запретить отрисовку шкалы
	void DisableMarkers(bool isDisable);

protected:
	//Рисование
	virtual void Draw();
	//Подписать
	virtual void DrawText(const GUIRectangle & rect, bool isVerticalText);
	//Обновились параметры слайдера
	virtual void OnUpdateParameters();
	//Обновить позицию
	virtual void OnUpdatePosition();

protected:	
	ExtName name;					//Название слайдера
	dword constSliderSize;			//Фиксированный размер бегунка
	array<GUIPoint> markerLines;	//Отрезки маркеров
	GUIPoint sliderPoints[256];		//Точки для рисования бегунка

public:
	ExtNameStr posString;			//Текст значения
	GUIEventHandler onChangeText;	//Обновить текст значения
	bool disableDrawText;			//Запретить отрисовку текста
	bool disableMarkers;			//Запретить отрисовку линий
};



