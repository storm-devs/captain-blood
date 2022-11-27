//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GroupNodeListElement	
//============================================================================================
			

#include "GroupNodeListElement.h"
#include "NodeGroupForm.h"
#include "LinkForm.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolListBox.h"

//============================================================================================

class GNLENodeForm : public GUIWindow
{
public:
	GNLENodeForm(AnxOptions & options, GUIPoint point, string & _nodeName);
	virtual void OnCreate();
	virtual void Draw();
	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtSelNode(GUIControl * sender);
	void _cdecl EvtSetSelNode(GUIControl * sender);
private:
	AnxOptions & opt;
	string & nodeName;
	GUIEdit * editName;
};

//============================================================================================

GroupNodeListElement::GroupNodeListElement(GUIControl * p, AnxOptions & options, NodeGroupForm * _nf) : GUIControl(p), opt(options)
{
	nf = _nf;
	AnxToolButton * b = NEW AnxToolButton(null, this, 310, 20, 20, 20);
	b->Glyph->Load("AnxEditor\\AnxDelItem");
	b->OnMousePressed = (CONTROL_EVENT)&GroupNodeListElement::EvtDel;
	mouseDown = false;
	deleteMe = false;
}

GroupNodeListElement::~GroupNodeListElement()
{

}


//============================================================================================

//Рисование
void GroupNodeListElement::Draw()
{
	cliper.SetRectangle(((GroupNodeListElement *)GetParent())->cliper.GetRectangle());
	bool linkProcess = false;
	bool nodeProcess = false;
	GUIRectangle rect = DrawRect;
	ClientToScreen(rect);
	//Получаем позицию курсора мыши
	GUIPoint cursor;
	opt.gui_manager->GetCursor(cursor.x, cursor.y);
	cursor -= rect.pos;
	//Треугольник линка
	GUIPoint p[4];
	p[0] = rect.pos + GUIPoint(50, 30);
	p[1] = rect.pos + GUIPoint(70, 40);
	p[2] = rect.pos + GUIPoint(50, 50);
	p[3] = p[0];
	GUIHelper::Draw2DLine(rect.x + 30, rect.y + 40, rect.x + 150, rect.y + 40, 0xffffffff);
	dword color = opt.link.defBkg;
	if(cursor.x >= 50 && cursor.x <= 70 && cursor.y >= 30 && cursor.y <= 50)
	{
		linkProcess = mouseDown;
		color = opt.link.defSelBkg;
	}
	GUIHelper::DrawPolygon(p, 3, color);
	GUIHelper::DrawLines(p, 4, opt.link.defFrm);
	//Нод
	color = opt.node.defBkg;
	if(cursor.x >= 150 && cursor.x <= 300 && cursor.y >= 10 && cursor.y <= 50)
	{
		nodeProcess = mouseDown;
		color = opt.node.defSelBkg;
	}
	GUIHelper::Draw2DRect(rect.x + 150, rect.y + 10, 150, 40, color);
	GUIHelper::DrawLinesBox(rect.x + 150, rect.y + 10, 150, 40, opt.node.defFrm);
	if(opt.node.fontLabel)
	{	
		//Команда линка
		cliper.Push();
		cliper.SetInsRectangle(GUIRectangle(rect.x + 10, rect.y, 130, 40));
		opt.stmp = "[";
		opt.stmp += data.name;
		opt.stmp += "]";
		float x = (float)opt.node.fontLabel->GetWidth(opt.stmp);
		float y = (float)opt.node.fontLabel->GetHeight();
		x = 50 - x*0.5f; if(x < 15) x = 15; x += rect.x;
		y = rect.y + 10 + (20 - y)*0.5f;
		opt.node.fontLabel->Print((int)x, (int)y, 0xff000000, opt.stmp);
		cliper.Pop();
		//Подпись нода
		cliper.Push();
		cliper.SetInsRectangle(GUIRectangle(rect.x + 150, rect.y + 10, 150, 40));
		x = (float)opt.node.fontLabel->GetWidth(nodeName);
		y = (float)opt.node.fontLabel->GetHeight();
		x = rect.x + 150 + (150 - x)*0.5f;
		y = rect.y + 10 + (40 - y)*0.5f;
		opt.node.fontLabel->Print((int)x, (int)y, 0xffffffff, nodeName);
		cliper.Pop();
	}
	//Разделительная черта
	GUIHelper::Draw2DLine(rect.x + 10, rect.y + rect.h, rect.x + rect.w - 10, rect.y + rect.h, 0xff000000);
	GUIControl::Draw();
	//Удаление
	if(deleteMe)
	{
		nf->EvtDeleteItemFromList(this);
		return;
	}
	//Сбросим состояние мыши
	mouseDown = false;
	//Процессируем если что нужно
	if(linkProcess)
	{
		LinkFormGroup * f = NEW LinkFormGroup(opt, rect.pos + rect.size/2, data, nodeName);
		opt.gui_manager->ShowModal(f);
	}else
	if(nodeProcess)
	{
		GNLENodeForm * f = NEW GNLENodeForm(opt, rect.pos + rect.size/2, nodeName);
		opt.gui_manager->ShowModal(f);
	}
}

//Сообщения
bool GroupNodeListElement::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	if(message == GUIMSG_LMB_DBLCLICK) mouseDown = true;
	return GUIControl::ProcessMessages(message, lparam, hparam);
}

//Сообщение об удалении
void _cdecl GroupNodeListElement::EvtDel(GUIControl * sender)
{
	deleteMe = true;
}

//============================================================================================
//GNLENodeForm
//============================================================================================

GNLENodeForm::GNLENodeForm(AnxOptions & options, GUIPoint point, string & _nodeName) : GUIWindow(null, point.x, point.y, 500, 150), opt(options), nodeName(_nodeName)
{
	//Положение окна
	DrawRect.pos -= DrawRect.size/2;
	if(DrawRect.x < 20) DrawRect.x = 20;
	if(DrawRect.y < 20) DrawRect.y = 20;
	if(DrawRect.x + DrawRect.w >= opt.width - 20) DrawRect.x = opt.width - DrawRect.w - 20;
	if(DrawRect.y + DrawRect.h >= opt.height - 20) DrawRect.y = opt.height - DrawRect.h - 20;
	bPopupStyle = true;
	UpdatePopupStyle();
	//Элементы
	GUILabel * lb = NEW GUILabel(this, 10, 10, DrawRect.w - 20, 28);
	lb->Caption = "Node name";
	lb->Layout = GUILABELLAYOUT_Left;
	editName = NEW AnxToolEdit(this, 10, 40, DrawRect.w - 20 - 28 - 10, 28);
	editName->Text = _nodeName;
	//Кнопки
	long bw = 120;
	long bh = 32;
	long bx = (DrawRect.w - bw*2 - 20)/2;
	long by = DrawRect.h - bh - 10;
	AnxToolButton * b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Ok";
	b->Glyph->Load("AnxEditor\\AnxOk");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&GNLENodeForm::EvtOk;
	bx += bw + 20;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Cancel";
	b->Glyph->Load("AnxEditor\\AnxCancel");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&GNLENodeForm::EvtCancel;
	b = NEW AnxToolButton(null, this, DrawRect.w - 28 - 15, 40, 31, 28);
	b->Glyph->Load("AnxEditor\\AnxSelNode");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&GNLENodeForm::EvtSelNode;
}

void GNLENodeForm::OnCreate()
{
	editName->SetFocus();
}

void GNLENodeForm::Draw()
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

void _cdecl GNLENodeForm::EvtOk(GUIControl * sender)
{
	nodeName = editName->Text;
	Application->Close(this);
}

void _cdecl GNLENodeForm::EvtCancel(GUIControl * sender)
{
	Application->Close(this);
}

void _cdecl GNLENodeForm::EvtSelNode(GUIControl * sender)
{
	GUIRectangle rect = ((GNLENodeForm *)sender)->DrawRect;
	((GNLENodeForm *)sender)->ClientToScreen(rect);
	rect.x += rect.w - 256;
	rect.y += rect.h;
	rect.w = 256;
	rect.h = 256;
	if(rect.x < 20) rect.x = 20;
	if(rect.y < 20) rect.y = 20;
	if(rect.x + rect.w > opt.width - 20) rect.x = opt.width - rect.w - 20;
	if(rect.y + rect.h > opt.height - 20) rect.y = opt.height - rect.h - 20;
	AnxToolListBox * lb = NEW AnxToolListBox(rect.x, rect.y, rect.w, rect.h);
	lb->FillNodes(opt.project->graph.root, true, anxnt_anode);
	lb->FillNodes(opt.project->graph.root, true, anxnt_gnode);
	lb->List()->OnMouseDown = (CONTROL_EVENT)&GNLENodeForm::EvtSetSelNode;
	Application->ShowModal(lb);
}

void _cdecl GNLENodeForm::EvtSetSelNode(GUIControl * sender)
{
	Assert(opt.project);
	AnxToolListBox * lb = (AnxToolListBox *)sender->GetParent();
	Assert(lb);
	if(lb->List()->SelectedLine < 0) return;
	editName->Text = lb->List()->Items[lb->List()->SelectedLine];
	lb->Close(sender);
}





