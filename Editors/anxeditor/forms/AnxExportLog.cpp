//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxExportLog	
//============================================================================================
			

#include "AnxExportLog.h"
#include "..\AnxProject.h"

#define ANXEXPORTLOGSPD	2.0f

//============================================================================================

AnxExportLog::AnxExportLog(AnxOptions & options, array<string> & errs) : GUIWindow(null, 0, 0, options.width, 30), opt(options)
{
	//Устанавливаем окно поверх
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	//Расставляем элементы
	//Подпись
	GUILabel * lb = NEW GUILabel(this, 4, 4, DrawRect.w - 20, 28);
	lb->Caption = "Extort log";
	lb->Layout = GUILABELLAYOUT_Left;
	GUIRectangle logrect(10, 40, DrawRect.w - 20, 28);
	log = NEW AnxToolListBox::LB(this, logrect.w, logrect.h);
	log->SetDrawRect(logrect);
	log->SetClientRect(logrect);
	for(long i = 0; i < errs; i++)
	{
		log->Items.Add(errs[i]);
	}	
	//Кнопка
	AnxToolButton * b = NEW AnxToolButton(null, this, DrawRect.w - 20, 4, 16, 16);
	b->Caption = "X";
	//b->Glyph->Load("AnxEditor\\AnxOk");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&AnxExportLog::EvtClose;
	size = 0.01f;
	dlt = ANXEXPORTLOGSPD;
}

AnxExportLog::~AnxExportLog()
{
}

void AnxExportLog::OnCreate()
{
	log->SetFocus();
}

//============================================================================================

//Рисование
void AnxExportLog::Draw()
{
	//Размер окошка
	GUIRectangle rect = GetDrawRect();
	size += dlt*api->GetDeltaTime();
	Clampfr(size, 0.0f, 1.0f);
	rect.h = long(size*300.0f + 0.5f);
	rect.y = opt.height - rect.h;
	if(rect.h < 2)
	{
		Application->Close(this);
	}
	if(rect.h < 30) rect.h = 30;
	SetDrawRect(rect);
	SetClientRect(rect);
	rect = GetDrawRect();	
	GUIRectangle logrect = log->GetDrawRect();
	logrect.h = rect.h - logrect.y - 10;
	if(logrect.h < 28) logrect.h = 28;
	log->SetDrawRect(logrect);
	log->SetClientRect(logrect);
	//Прямоугольник окна
	ClientToScreen(rect);
	//Рисуем панель
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xefe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	GUIControl::Draw();
}

void _cdecl AnxExportLog::EvtClose(GUIControl * sender)
{	
	dlt = -ANXEXPORTLOGSPD;
}



