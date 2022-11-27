

#include "FormLabel.h"


FormLabel::FormLabel(GUIControl * parent, const GUIRectangle & r) : GUIControl(parent)
{
	SetClientRect(r);
	SetDrawRect(r);
	textColor = options->colorTextLo;
	align = align_init;
	image = null;
	imageW = image_width;
	imageH = image_height;
	borderText = border_text;
	borderImage = border_image;
	imageByTextSpace = image_by_text_space;
}

FormLabel::~FormLabel()
{
}

//Рисование
void FormLabel::Draw()
{
	if(!Visible) return;
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	if(align & align_debug_frame)
	{
		GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, 0xff000000);
	}
	//Размеры
	long wt = text.w + borderText + borderText;
	long ht = text.h + borderText + borderText;
	long wi = imageW + borderImage + borderImage;
	long hi = imageH + borderImage + borderImage;
	if(!image)
	{
		wi = 0;
		hi = 0;
	}
	//Позиция по горизонтали
	long x;
	if(align & align_left)
	{
		x = r.x;
	}else
	if(align & align_right)
	{
		x = r.x + r.w - wt - wi - imageByTextSpace;
	}else{
		x = r.x + (r.w - wt - wi - imageByTextSpace)/2;
	}
	long xt, xi;
	if(align & image_by_text_space)
	{
		xt = x + borderText;
		xi = x +  wt + imageByTextSpace;
	}else{
		xi = x + borderImage;
		xt = x +  wi + imageByTextSpace;
	}
	//Позиция по вертикали
	long yi, yt;
	if(align & align_image_top)
	{
		yi = r.y;
	}else
	if(align & align_image_bottom)
	{
		yi = r.y + r.h - hi;
	}else{
		yi = r.y + (r.h - hi)/2;
	}
	if(align & align_text_top)
	{
		yt = r.y;
	}else
	if(align & align_text_bottom)
	{
		yt = r.y + r.h - ht;
	}else
	if(align & align_text_image_center)
	{
		yt = yi + (hi - ht)/2;
	}else{
		yt = r.y + (r.h - ht)/2;
	}
	if(image)
	{
		GUIHelper::DrawSprite(xi, yi, imageW, imageH, image);
	}
	if(text.str)
	{
		dword color = textColor;
		if(!Enabled)
		{
			color = (color & 0x00ffffff) | 0x70000000;
		}
		options->render->Print(float(xt), float(yt), color, text.str);
	}
}