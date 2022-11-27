

#pragma once

#include "..\..\SndOptions.h"

class BaseSlider;
class FormButton;

class FormScrollBar : public GUIControl
{
	enum State
	{
		s_none,
		s_dec_first,
		s_inc_first,
		s_no_first,
		s_dec,		
		s_inc,
	};

public:
	FormScrollBar(GUIControl * parent, const GUIRectangle & r);
	virtual ~FormScrollBar();

	//Установить диапазон в котором работает слайдер
	void SetRange(long min, long max);
	//Установить размер страницы (влияет на размер бегунка)
	void SetPageSize(long size);
	//Устоновить шаг для перемещения по кнопкам
	void SetButtonStep(long step);

	//Установить позицию
	void SetPosition(long pos);
	//Получить текущую позицию
	long GetPosition();

	//Установить режим автоматического скрывания если размер страницы больше или равен диапазону
	void SetAutoHideMode(bool isEnable);

	//Установить обработчик изменения позиции
	void SetUpdateHandler(GUIControl * listener, CONTROL_EVENT evt);


protected:
	//Нарисовать кнопки
	void Draw();
	//Нарисовать значёк на уменьшающей кнопке
	void _cdecl DrawDec(FormButton * button, const GUIRectangle & rect);
	//Нарисовать значёк на увеличивающей кнопке
	void _cdecl DrawInc(FormButton * button, const GUIRectangle & rect);
	//Обработать нажатие на уменьшающую кнопку
	void _cdecl Dec(GUIControl* sender);
	//Продолжить уменьшение
	void DecContinue();
	//Обработать нажатие на увеличивающую кнопку
	void _cdecl Inc(GUIControl* sender);
	//Продолжить уменьшение
	void IncContinue();

private:
	BaseSlider * slider;	//Слайдер
	FormButton * btDec;		//Уменьшение значения
	FormButton * btInc;		//Увеличение значения
	long buttonStep;		//Шаг на который смещаеться позиция при нажатие на кнопку
	State state;			//Состояние
	float timer;			//Текущий счётчик повторений
	GUIPoint decArrow[4];	//Стрелка для уменьшающей кнопки
	GUIPoint incArrow[4];	//Стрелка для увеличивающей кнопки
	static Vector base[4];	//Базовая нормалитзованная стрелка справа на лево
};

















