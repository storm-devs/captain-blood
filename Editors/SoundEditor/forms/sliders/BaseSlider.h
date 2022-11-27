//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// BaseSlider
//============================================================================================


#pragma once

#include "..\..\SndOptions.h"


class BaseSlider : public GUIControl
{
public:
	BaseSlider(GUIControl * parent, const GUIRectangle & r);
	virtual ~BaseSlider();

public:
	//Установить диапазон в котором работает слайдер
	void SetRange(long min, long max);
	//Установить размер страницы (влияет на размер бегунка)
	void SetPageSize(long size);

	//Установить позицию
	void SetPosition(long pos, bool enableRestoreInitial = false);
	//Установить инициальную позицию
	void SetInitialPosition(long pos);
	//Получить текущую позицию
	long GetPosition();

	//Установить режим автоматического скрывания если размер страницы больше или равен диапазону
	void SetAutoHideMode(bool isEnable);
	//Установить вертикальную ориентацию
	void SetVertical();
	//Установить горизонтальную ориентацию
	void SetHorisontal();
	//Установить привязку бегунка к странице
	void SetSnapSlider(bool isEnable);
	//Установить инвертированную связь значения и бегунка
	void SetInverse(bool isEnable);
	//Разрешить востонавливать значение по правой кнопке мыши
	void SetRestore(bool isEnable);

protected:
	//Рисование
	virtual void Draw();

	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);

	//Нажатие кнопки мыши
	bool MouseDown(const GUIPoint& pt);
	//Нажатие кнопки мыши
	virtual void MouseUp(int button, const GUIPoint& pt);
	//Перемещение мыши
	virtual void MouseMove(int button, const GUIPoint& pt);

	//Нажатие правой кнопкой мыши
	virtual bool RMouseDown(const GUIPoint& pt);
	//Правый шелчёк на бегунке
	virtual void RestoreInitial();
	
/*
	//Начать тянуть
	virtual bool DragBegin();
	//Процесс переноса
	virtual bool DragOver(int MouseX, int MouseY, GUIControl* dragfrom);
	//Бросить
	virtual void DragDrop(int MouseX, int MouseY, GUIControl* dragfrom);
*/
	//Обновились параметры слайдера
	virtual void OnUpdateParameters();

protected:
	//Получить прямоугольник бегунка
	void CalcSliderRect(GUIRectangle & r);
	//Переместить бегунок в ближнюю точку к заданной
	void MoveSliderTo(const GUIPoint & pt);
	//Получить приклееную позицию
	float GetSnapPosition();
	//Обновить позицию
	virtual void OnUpdatePosition();
	
//Задаваемые параметры
protected:
	long minValue;				//Пользовательское минимальное значение
	long maxValue;				//Пользовательское максимальное значение
	long current;				//Пользовательское текущее значение
	long initPosition;			//На какую позицию востонавливать при сбросе
	long pageSize;				//Размер страницы (влияет на размер бегунка)
	bool isAutoHide;			//Скрывать ли слайдер если бегунок в полный размер
	bool isVertical;			//Вертикальный или горизонтальный слайдер
	bool isSnapSlider;			//Приклеивать ли бегунок
	bool isInverse;				//Инвертировать значение
	bool isRestoreInitial;		//Режим востоновления начального значения по правой кнопке

//Вычисляемые параметры
protected:	
	float position;				//Текущая позиция центра бегунка 0..1
	long minPosition;			//Минимальная позиция центра бегунка
	long dltPosition;			//Длинна отрезка по которому перемещаеться центр бегунока
	long centerStatic;			//Вторая статичная координата центра бегунка		
	long sliderSize;			//Плавающий размер бегунка
	long sliderStatic;			//Второй, статичный размер слайдера
	long sliderPosition;		//Текущая позиция слайдера
	float sliderStep;			//Шаг с которым привязываться бегунку
	GUIPoint deltaDrag;			//Смещение от центра при перемещении бегунка
	bool isDrag;				//Процесс перемещения бегунка
	
public:
	GUIEventHandler onUpdate;	//Обновить позицию
};

