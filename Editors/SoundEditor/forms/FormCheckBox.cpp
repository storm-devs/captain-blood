

#include "FormCheckBox.h"


FormCheckBox::FormCheckBox(GUIControl * parent, const GUIPoint & p) : GUIControl(parent)
{
	GUIRectangle r;
	r.pos = p;
	r.w = c_imageSize;
	r.h = c_imageSize;
	SetClientRect(r);
	SetDrawRect(r);
	isChecked = false;
}

FormCheckBox::~FormCheckBox()
{

}

//Установить текст метки
void FormCheckBox::SetText(const char * t)
{
	text.SetString(t);
	GUIRectangle r = GetClientRect();
	r.w = coremax(text.w + c_imageSize + c_labelSpace, c_imageSize);
	r.h = coremax(text.h, c_imageSize);
	SetClientRect(r);
	SetDrawRect(r);
}

//Установить состояние
void FormCheckBox::SetCheck(bool isCheck)
{
	isChecked = isCheck;
	onChange.Execute(this);
}

//Получить состояние
bool FormCheckBox::IsChecked()
{
	return isChecked;
}

//Рисование
void FormCheckBox::Draw()
{
	if(!Visible) return;
	dword color = Enabled ? options->black2Bkg[0] : options->black2Bkg[2];
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	//Квадратик с галкой
	long imgY = r.y + (r.h - c_imageSize)/2;
	GUIHelper::DrawWireRect(r.x + c_rectSpace, imgY + c_rectSpace, c_imageSize - c_rectSpace*2, c_imageSize - c_rectSpace*2, color);
	if(isChecked)
	{
		GUIHelper::Draw2DLine(r.x + 2, imgY + 2, r.x + c_imageSize/2, imgY + c_imageSize, color);
		GUIHelper::Draw2DLine(r.x + c_imageSize, imgY - 4, r.x + c_imageSize/2, imgY + c_imageSize, color);
	}
	if(text.str)
	{
		float x = float(r.x + c_imageSize + c_labelSpace);
		float y = float(r.y + (r.h - text.h)/2);
		options->render->Print(x, y, color, text.str);
	}
	GUIControl::Draw();
}

//Нажатие кнопки мыши
void FormCheckBox::MouseDown(int button, const GUIPoint& pt)
{
	if(!Enabled || !Visible) return;
	GUIRectangle r = GetClientRect();
	ClientToScreen(r);
	if(r.Inside(pt))
	{
		SetCheck(!isChecked);		
	}
}
