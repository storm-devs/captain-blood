

#pragma once

#include "..\SndOptions.h"


class FormControlBase : public GUIControl
{
public:
	FormControlBase(GUIControl * parent, const GUIRectangle & rect);
	virtual ~FormControlBase();

private:
	//Рисование
	virtual void Draw();
	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	//Нажатие кнопки мыши
	bool MouseDownL(const GUIPoint& pt);
	//Нажатие правой кнопкой мыши
	bool MouseDownR(const GUIPoint& pt);
	//Нажатие кнопки мыши
	void MouseUp(int button, const GUIPoint& pt);
	//Перемещение мыши
	void MouseMove(int button, const GUIPoint& pt);	
	//Дельта от колеса
	void MouseWeel(const GUIPoint& pt, long delta);


public:
	//Рисование
	virtual void OnDraw(const GUIRectangle & screenRect){};
	//Начало перемещения, возвранить true, если начинать тащить. elementPivot графический центр элемента
	virtual bool OnDragBegin(const GUIPoint & mousePos, GUIPoint & elementPivot){ return false; };
	//Перемещение текущего элемента в данную точку
	virtual void OnDragMoveTo(const GUIPoint & elementPosition){};
	//Прекратить операцию перетаскивания
	virtual void OnDragEnd(){};
	//Щелчёк левой кнопкой мыши
	virtual void OnMouseLeftClick(const GUIPoint & mousePos){};
	//Щелчёк правой кнопкой мыши
	virtual void OnMouseRightClick(const GUIPoint & mousePos){};
	//Дельта от колеса
	virtual void OnMouseWeel(const GUIPoint & mousePos, long delta){};

private:	
	GUIPoint deltaDrag;		//Смещение от центра при таскании
	bool isBlockWeel;		//1 раз за кадр приходит
};