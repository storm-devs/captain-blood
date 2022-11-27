//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormButton
//============================================================================================


#pragma once

#include "..\SndOptions.h"

class FormButton : public GUIControl
{
public:
	typedef void (_cdecl GUIControl::*DrawGlyph)(FormButton * button, const GUIRectangle & rect);
	

public:
	FormButton(GUIControl * parent, const GUIRectangle & r);
	virtual ~FormButton();

	//Установить отрисовщик изображения на кнопке
	template<class T> inline void SetDrawGlyph(GUIControl * glyphObject, T drawGlyphFunc)
	{
		glyphFunc = (DrawGlyph)drawGlyphFunc;
		glyphControl = glyphObject;
	};

	//Получить состояние кнопки
	bool IsPressed();

protected:
	//Рисование
	virtual void Draw();

	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	//Нажатие кнопки мыши
	virtual void MouseUp(int button, const GUIPoint& pt);

protected:
	//Событие нажатия кнопки
	virtual void OnDown();
	//Событие отпускания кнопки
	virtual void OnUp();

public:
	ExtNameStr text;			//Текст на кнопке
	GUIImage * image;			//Картинка на кнопке
	dword imageW;				//Ширина картинки
	dword imageH;				//Высота картинка
	GUIEventHandler onDown;		//Событие нажатия кнопки
	GUIEventHandler onUp;		//Событие отпускания кнопки

private:
	DrawGlyph glyphFunc;		//Внешний элемент рисуемый на кнопке
	GUIControl * glyphControl;	//Объект рисующий на кнопке
};












