//============================================================================================
// Spirenkov Maxim, 2009
//============================================================================================
// FormLabel
//============================================================================================


#pragma once

#include "..\SndOptions.h"

class FormLabel : public GUIControl
{
public:
	enum Consts
	{
		aling_center = 0,								//Выравнивание по центру
		align_left = 0x01,								//Текст с картинкой выравнить по левой границе
		align_right = 0x02,								//Текст с картинкой выравнить по правой границе
		align_text_top = 0x10,							//Текст выравнить по верхней границе
		align_text_bottom = 0x20,						//Текст выравнить по нижней границе
		align_text_image_center = 0x80,					//По вертикали по центру картинки
		align_image_top = 0x100,						//Картинку выравнить по верхней границе
		align_image_bottom = 0x200,						//Картинку выравнить по нижней границе
		align_image_by_text_left = 0,					//Картинка слева от текста
		align_image_by_text_right = 0x10000,			//Картинка справа от текста
		align_debug_frame = 0x100000,					//Нарисовать для отладки чёрную рамку

		align_init = align_left,						//Начальное значение выравнивания

		border_text = 1,								//Пустое пространство вокруг текста
		border_image = 1,								//Пустое пространство вокруг картинки
		image_by_text_space = 2,						//Дополнительное пустое пространство между картинкой и текстом

		image_width = 16,								//Начальная ширина картинки
		image_height = 16,								//Начальная высота картинки
	};

public:
	FormLabel(GUIControl * parent, const GUIRectangle & r);
	virtual ~FormLabel();

protected:
	//Рисование
	virtual void Draw();


public:
	ExtNameStr text;			//Текст
	dword textColor;			//Цвет текста
	dword align;				//Флаги выравнивания
	GUIImage * image;			//Картинка
	long imageW;				//Ширина картинки
	long imageH;				//Высота картинки	
	dword borderText;			//Размер пустой области вогруг текста
	dword borderImage;			//Размер пустой области вогруг картинки
	dword imageByTextSpace;		//Дополнительное пустое пространство между картинкой и текстом
};












