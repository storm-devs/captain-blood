//============================================================================================
// Spirenkov Maxim, 2009
//============================================================================================
// FormCheckBox
//============================================================================================


#pragma once

#include "..\SndOptions.h"

class FormCheckBox : public GUIControl
{
	enum Consts
	{
		c_imageSize = 16,
		c_rectSpace = 3,
		c_labelSpace = 4,
	};

public:
	FormCheckBox(GUIControl * parent, const GUIPoint & p);
	virtual ~FormCheckBox();

	//Установить текст метки
	void SetText(const char * t);
	//Установить состояние
	void SetCheck(bool isCheck);
	//Получить состояние
	bool IsChecked();

protected:
	//Рисование
	virtual void Draw();
	//Нажатие кнопки мыши
	virtual void MouseDown(int button, const GUIPoint& pt);

public:
	GUIEventHandler onChange;	//Событие изменения состояния

private:
	ExtNameStr text;			//Текст
	bool isChecked;				//Состояние
};

