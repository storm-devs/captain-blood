//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// NodeInOutForm	
//============================================================================================
			

#include "NodeInOutForm.h"
#include "..\AnxProject.h"


//============================================================================================

NodeInOutForm::NodeInOutForm(GraphNodeInOut * _node) : GUIWindow(null, 0, 0, 600, 200)
{
	Assert(_node);
	Assert(_node->opt.project->graph.current);
	node = _node;
	//Данные для undo
	nodeState = NEW AnxCmd_NodeState(node);
	//Расчитываем позицию
	GUIPoint pos = node->rect.pos + node->opt.project->graph.current->base;
	pos += (node->rect.size - DrawRect.size)/2;
	if(pos.x < 20) pos.x = 20;
	if(pos.y < 20) pos.y = 20;
	if(pos.x + DrawRect.w > long(node->opt.width) - 20) pos.x = node->opt.width - 20 - DrawRect.w;
	if(pos.y + DrawRect.h > long(node->opt.height) - 20) pos.y = node->opt.height - 20 - DrawRect.h;
	SetPosition(pos.x, pos.y);
	//Устанавливаем окно поверх
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
	//Расставляем элементы
	//Подпись
	GUILabel * lb = NEW GUILabel(this, 10, 10, DrawRect.w - 20, 28);
	lb->Caption = "Node name";
	lb->Layout = GUILABELLAYOUT_Left;
	editName = NEW AnxToolEdit(this, 10, 40, DrawRect.w - 20, 28);
	editName->Text = node->name;
	lb = NEW GUILabel(this, 10, 80, DrawRect.w - 20, 28);
	lb->Caption = "Connection link name";
	lb->Layout = GUILABELLAYOUT_Left;
	editLink = NEW AnxToolEdit(this, 10, 110, DrawRect.w - 20, 28);
	editLink->Text = node->link;
	//Кнопки
	long bw = 120;
	long bh = 32;
	long bx = (DrawRect.w - bw*2 - 20)/2;
	long by = DrawRect.h - bh - 10;
	AnxToolButton * b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Ok";
	b->Glyph->Load("AnxEditor\\AnxOk");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeInOutForm::EvtOk;
	bx += bw + 20;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Cancel";
	b->Glyph->Load("AnxEditor\\AnxCancel");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeInOutForm::EvtCancel;
}

NodeInOutForm::~NodeInOutForm()
{
	if(nodeState) delete nodeState;
	nodeState = null;
}

void NodeInOutForm::OnCreate()
{
	editLink->SetFocus();
	editName->SelectText(0, editName->Text.Len());
}

//============================================================================================

//Рисование
void NodeInOutForm::Draw()
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

void _cdecl NodeInOutForm::EvtOk(GUIControl * sender)
{
	if(node->name != editName->Text)
	{		
		Assert(node->opt.project);
		if(!node->opt.project->CheckUniqueNodeName(editName->Text))
		{
			editName->t_OnChange->Execute(this);
			return;
		}
		node->name = editName->Text;
	}else{
		node->name = editName->Text;
	}
	node->link = editLink->Text;
	Application->Close(this);
	node->opt.project->Execute(nodeState);
	nodeState = null;
}

void _cdecl NodeInOutForm::EvtCancel(GUIControl * sender)
{
	Application->Close(this);
	nodeState->UnDo();
}


