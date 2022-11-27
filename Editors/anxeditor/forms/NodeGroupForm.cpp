//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================
//	
// 
//===========================================================================================================================
// NodeGroupForm	
//============================================================================================
			

#include "NodeGroupForm.h"
#include "GroupNodeListElement.h"

//============================================================================================

NodeGroupForm::NodeGroupForm(GraphNodeGroup * _node) : GUIWindow(null, 0, 0, 400, 600)
{
	Assert(_node);
	Assert(_node->opt.project->graph.current);
	node = _node;
	//Данные для undo
	nodeState = NEW AnxCmd_NodeState(node);
	//Расчитываем позицию
	SetScreenCenter();
	//Устанавливаем окно поверх
	bPopupStyle = true;
	UpdatePopupStyle();
	//bAlwaysOnTop = true;
	//Расставляем элементы
	//Подпись
	GUILabel * lb = NEW GUILabel(this, 10, 10, DrawRect.w - 20, 28);
	lb->Caption = "Node name";
	lb->Layout = GUILABELLAYOUT_Left;
	editName = NEW AnxToolEdit(this, 10, 40, DrawRect.w - 20, 28);
	editName->Text = node->name;
	//Ралочка, разрешающая циклить линки
	flgLoop = NEW GUICheckBox(this, 10, 90, 320, 20);
	flgLoop->Caption = "Loop links in this group node";
	flgLoop->ImageChecked->Load("AnxEditor\\Anxcheckbox_ch");
	flgLoop->ImageNormal->Load("AnxEditor\\Anxcheckbox_uch");
	flgLoop->Checked = node->isLoopLinks;
	flgLoop->OnChange = (CONTROL_EVENT)&NodeGroupForm::EvtChangeCheckLoop;
	flgLoop->Hint = "If links loop enable then link\ncan connected to source node";
	//Список линков на ноды
	list = NEW AnxItemsList(this, 10, 120, DrawRect.w - 20, DrawRect.h - 120 - 50, 60);
	AnxToolButton * b = NEW AnxToolButton(null, list, 0, 0, 300, 30);
	b->Caption = "Add new link no node";
	b->Glyph->Load("AnxEditor\\AnxCreateItem");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeGroupForm::EvtAddNewItemToList;
	list->items.Add(b);
	for(long i = 0; i < node->vlink; i++)
	{
		GroupNodeListElement * li = NEW GroupNodeListElement(list, node->opt, this);
		li->data = node->vlink[i]->linkData;
		li->nodeName = node->vlink[i]->toNodeName;
		list->items.Add(li);
	}
	//Кнопки
	long bw = 120;
	long bh = 32;
	long bx = (DrawRect.w - bw*2 - 20)/2;
	long by = DrawRect.h - bh - 10;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Ok";
	b->Glyph->Load("AnxEditor\\AnxOk");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeGroupForm::EvtOk;
	bx += bw + 20;
	b = NEW AnxToolButton(null, this, bx, by, bw, 32);
	b->Caption = "Cancel";
	b->Glyph->Load("AnxEditor\\AnxCancel");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&NodeGroupForm::EvtCancel;
}

NodeGroupForm::~NodeGroupForm()
{
	if(nodeState) delete nodeState;
	nodeState = null;
}

void NodeGroupForm::OnCreate()
{
	editName->SetFocus();
	editName->SelectText(0, editName->Text.Len());
}

//============================================================================================


//Рисование
void NodeGroupForm::Draw()
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

void _cdecl NodeGroupForm::EvtOk(GUIControl * sender)
{
	//Имя нода
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
	//Сохраняем параметры нода
	for(long i = 0; i < node->vlink; i++) delete node->vlink[i];
	node->vlink.Empty();
	for(i = 1; i < list->items; i++)
	{
		GraphNodeGroup::VirtualLink * vlink = NEW GraphNodeGroup::VirtualLink();
		vlink->linkData = ((GroupNodeListElement *)list->items[i])->data;
		vlink->toNodeName = ((GroupNodeListElement *)list->items[i])->nodeName;
		node->vlink.Add(vlink);
	}
	//Закрываем окно
	Application->Close(this);
	node->opt.project->Execute(nodeState);
	nodeState = null;
}

void _cdecl NodeGroupForm::EvtCancel(GUIControl * sender)
{
	Application->Close(this);
	nodeState->UnDo();
}

void _cdecl NodeGroupForm::EvtAddNewItemToList(GUIControl * sender)
{
	list->items.Add(NEW GroupNodeListElement(list, node->opt, this));
}

void _cdecl NodeGroupForm::EvtDeleteItemFromList(GUIControl * sender)
{
	list->items.Del(sender);
	delete sender;
}

void _cdecl NodeGroupForm::EvtChangeCheckLoop(GUIControl * sender)
{
	node->isLoopLinks = flgLoop->Checked;
}

