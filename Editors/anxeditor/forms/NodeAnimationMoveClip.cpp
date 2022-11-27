//============================================================================================
// Spirenkov Maxim, 2007
//============================================================================================
// AnxEditor
//============================================================================================
// NodeAnimationMoveClip
//============================================================================================

#include "NodeAnimationMoveClip.h"

//============================================================================================

NodeAnimationMoveClip::NodeAnimationMoveClip(AnxOptions & options, AntFile & _ant) : GUIWindow(null, 0, 0, 400, 350), opt(options), ant(_ant)
{
	t_OnOK_Sender = null;
	frame = 0;
	pos = ang = 0.0f;
	t_OnOK = NEW GUIEventHandler;
	//Устанавливаем окно поверх
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	SetScreenCenter();
	//Кнопки выхода
	long bw = 120;
	long bh = 32;
	long bx = (DrawRect.w - bw*2 - 20)/2;
	long by = DrawRect.h - bh - 10;
	buttonOk = NEW AnxToolButton(null, this, bx, by, bw, 32);
	buttonOk->Caption = "Ok";
	buttonOk->Glyph->Load("AnxEditor\\AnxOk");
	buttonOk->Align = GUIAL_Center;
	buttonOk->OnMousePressed = (CONTROL_EVENT)&NodeAnimationMoveClip::EvtOk;
	bx += bw + 20;
	buttonCancel = NEW AnxToolButton(null, this, bx, by, bw, 32);
	buttonCancel->Caption = "Cancel";
	buttonCancel->Glyph->Load("AnxEditor\\AnxCancel");
	buttonCancel->Align = GUIAL_Center;
	buttonCancel->OnMousePressed = (CONTROL_EVENT)&NodeAnimationMoveClip::EvtCancel;
	//Заголовок
	GUILabel * lb = NEW GUILabel(this, 10, 10, 250, 28);
	lb->Caption = "Move clip to new position (for pivot use first root bone)";
	//Требуемый кадр
	int y = 62;
	lb->Layout = GUILABELLAYOUT_Left;
	lb = NEW GUILabel(this, 30, y, 170, 28);
	lb->Caption = "Pivot bone frame:";
	lb->Layout = GUILABELLAYOUT_Left;
	frameNumber = NEW AnxToolEdit(this, 170, y + 2, 50, 24);
	frameNumber->Text = "0";
	frameNumber->longValue = &frame;
	frameNumber->isLimitedNumber = true;
	frameNumber->min = 0.0f;
	frameNumber->max = ant.clipNumFrames - 1.0f;
	if(frameNumber->max < frameNumber->min)
	{
		frameNumber->max = frameNumber->min;
	}
	//
	AnxToolButton * b = NEW AnxToolButton(null, this, 240, y + 2, 80, 24);
	b->Caption = "Read frame";
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeAnimationMoveClip::EvtReadBoneTransform;
	b->Hint = "Read current frame transformation from root bone";
	//Позиция
	y += 55;
	lb = NEW GUILabel(this, 30, y, 250, 28);
	lb->Caption = "New pivot bone position:";
	lb->Layout = GUILABELLAYOUT_Left;
	y += 36;
	lb = NEW GUILabel(this, 50, y, 25, 28);
	lb->Caption = "X:";
	lb->Layout = GUILABELLAYOUT_Left;
	posX = NEW AnxToolEdit(this, 75, y + 2, 50, 24);
	posX->Text = "0.0";
	posX->floatValue = &pos.x;
	posX->isLimitedNumber = false;
	lb = NEW GUILabel(this, 150, y, 25, 28);
	lb->Caption = "Y:";
	lb->Layout = GUILABELLAYOUT_Left;
	posY = NEW AnxToolEdit(this, 175, y + 2, 50, 24);
	posY->Text = "0.0";
	posY->floatValue = &pos.y;
	posY->isLimitedNumber = false;
	lb = NEW GUILabel(this, 250, y, 25, 28);
	lb->Caption = "Z:";
	lb->Layout = GUILABELLAYOUT_Left;
	posZ = NEW AnxToolEdit(this, 275, y + 2, 50, 24);
	posZ->Text = "0.0";
	posZ->floatValue = &pos.z;
	posZ->isLimitedNumber = false;
	//Углы
	y += 50;
	lb = NEW GUILabel(this, 30, y, 250, 28);
	lb->Caption = "New pivot bone orientation:";
	lb->Layout = GUILABELLAYOUT_Left;
	y += 36;
	lb = NEW GUILabel(this, 50, y, 25, 28);
	lb->Caption = "Ax:";
	lb->Layout = GUILABELLAYOUT_Left;
	angX = NEW AnxToolEdit(this, 75, y + 2, 50, 24);
	angX->Text = "0.0";
	angX->floatValue = &ang.x;
	angX->isLimitedNumber = false;
	lb = NEW GUILabel(this, 150, y, 25, 28);
	lb->Caption = "Ay:";
	lb->Layout = GUILABELLAYOUT_Left;
	angY = NEW AnxToolEdit(this, 175, y + 2, 50, 24);
	angY->Text = "0.0";
	angY->floatValue = &ang.y;
	angY->isLimitedNumber = false;
	lb = NEW GUILabel(this, 250, y, 25, 28);
	lb->Caption = "Az:";
	lb->Layout = GUILABELLAYOUT_Left;
	angZ = NEW AnxToolEdit(this, 275, y + 2, 50, 24);
	angZ->Text = "0.0";
	angZ->floatValue = &ang.z;
	angZ->isLimitedNumber = false;
	EvtReadBoneTransform(this);
}

NodeAnimationMoveClip::~NodeAnimationMoveClip()
{
	frameNumber->longValue = null;
	posX->floatValue = null;
	posY->floatValue = null;
	posZ->floatValue = null;
	angX->floatValue = null;
	angY->floatValue = null;
	angZ->floatValue = null;
	delete t_OnOK;
}

//============================================================================================

//Рисование
void NodeAnimationMoveClip::Draw()
{
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем панель
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xefe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	GUIControl::Draw();
}

void _cdecl NodeAnimationMoveClip::EvtOk(GUIControl * sender)
{
	if(!frameNumber->isAccept)
	{
		frameNumber->SetFocus();
		return;
	}
	if(!posX->isAccept)
	{
		posX->SetFocus();
		return;
	}
	if(!posY->isAccept)
	{
		posY->SetFocus();
		return;
	}
	if(!posZ->isAccept)
	{
		posZ->SetFocus();
		return;
	}
	if(!angX->isAccept)
	{
		angX->SetFocus();
		return;
	}
	if(!angY->isAccept)
	{
		angY->SetFocus();
		return;
	}
	if(!angZ->isAccept)
	{
		angZ->SetFocus();
		return;
	}
	Matrix mtx(ang*(PI/180.0f), pos);
	ant.MoveTrackTo(frame, mtx);
	Application->Close(this);
	t_OnOK->Execute(t_OnOK_Sender);
}

void _cdecl NodeAnimationMoveClip::EvtCancel(GUIControl * sender)
{
	Application->Close(this);
}

void _cdecl NodeAnimationMoveClip::EvtReadBoneTransform(GUIControl * sender)
{
	if(!frameNumber->isAccept)
	{
		frameNumber->SetFocus();
		return;
	}
	Matrix mtx;
	ant.GetFrame(frame, mtx);
	pos = mtx.pos;	
	ang = mtx.GetAngles();
	posX->UpdateText();
	posY->UpdateText();
	posZ->UpdateText();
	angX->UpdateText();
	angY->UpdateText();
	angZ->UpdateText();
}

