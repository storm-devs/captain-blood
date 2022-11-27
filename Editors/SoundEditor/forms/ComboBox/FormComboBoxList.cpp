

#include "FormComboBoxList.h"
#include "FormComboBox.h"


FormCBLElement::FormCBLElement(FormListBase & _list) : FormListElement(_list)
{
	image = null;
	itemIndex = -1;
}

FormCBLElement::~FormCBLElement()
{
}

//Установить итему текст
void FormCBLElement::SetName(const char * str)
{
	name.SetString(str);
}

//Установить итему картинку
void FormCBLElement::SetImage(GUIImage * img)
{
	image = img;
}

//Установить индекс итема
void FormCBLElement::SetIndex(long index)
{
	itemIndex = index;
}

//Нарисовать линию
void FormCBLElement::Draw(const GUIRectangle & rect)
{
	//Фон выделения	и фокус
	DrawCursor(rect);
	DrawSelect(rect);
	if(image)
	{
		//Иконка объекта
		DrawIcon(rect, image);
		//Текст
		DrawText(rect, &name, true);
	}else{
		//Текст
		DrawText(rect, &name, false);
	}
	//GUIHelper::DrawWireRect(rect.x, rect.y, rect.w, rect.h, options->black2Bkg[0]);
}

//Если параметры линия должна быть выше чем данная то вернуть true
bool FormCBLElement::IsLessThen(const FormListElement * el) const
{
	FormCBLElement & el2 = *(FormCBLElement *)el;
	const char * name1 = name.str;
	const char * name2 = el2.name.str;
	if(string::Compare(name1, name2) >= 0)
	{
		return false;
	}
	return true;
}

//Проверить на совпадение итема
bool FormCBLElement::IsThis(const char * str, dword hash, dword len) const
{
	if(hash == name.hash && len == name.len)
	{
		if(string::IsEqual(str, name.str))
		{
			return true;
		}
	}
	return false;
}

//Проверить попадание по префиксу
bool FormCBLElement::IsPreficsEqual(const char * pref) const
{
	return string::EqualPrefix(name.str, pref);
}

//Если на данном щелчке можно проводить операции селекта, вернуть true
bool FormCBLElement::MouseClick(const GUIRectangle & rect, const GUIPoint & pos)
{
	OnAction();
	return true;
}

//Эвенты
void FormCBLElement::Event(const FormListEventData & data)
{
}

//Над линией едет мышиный курсор
void FormCBLElement::OnMouseMove(const GUIRectangle & rect, const GUIPoint & pos)
{
	if(!IsFocus())
	{
		if(!((FormComboBoxList &)List()).IsBlocked())
		{
			SetFocus();
		}
	}	
}

//Линию выбрали
void FormCBLElement::OnSetSelect()
{
	isSelect = false;
}

//Активировать элемент
void FormCBLElement::OnAction()
{
	//Был выбран пункт списка
	((FormComboBoxList &)List()).comboBox.SetIndexFromList(itemIndex);
	options->gui_manager->Close((GUIWindow *)List().GetParent());
}

FormComboBoxList::FormComboBoxList(FormComboBox & combo, GUIControl * parent, const GUIRectangle & rect, bool isDisableSort) : 
FormListBase(parent, rect),
comboBox(combo)
{
	DisableSort(isDisableSort);
	SetFrame(true);
	InitList();
}

FormComboBoxList::~FormComboBoxList()
{
	comboBox.ClearListPtr();
}

//Установить заголовок списка
void FormComboBoxList::SetTitle(GUIImage * icon, const char * text)
{
	SetCaption(text, icon);
}

//Заблокирован ли автофокус
bool FormComboBoxList::IsBlocked()
{
	if(isBlockAutofocus)
	{
		int x = blockMousePosition.x;
		int y = blockMousePosition.y;
		options->gui_manager->GetCursor(x, y);
		int dx = abs(x - blockMousePosition.x);
		int dy = abs(y - blockMousePosition.y);
		int dmax = coremax(dx, dy);
		if(dmax < 10)
		{
			return true;
		}
	}
	return false;
}

//Инициализировать лист
void FormComboBoxList::DoInitList()
{
	for(dword i = 0; i < comboBox.items.Size(); i++)
	{
		FormCBLElement * el = NEW FormCBLElement(*this);
		el->SetName(comboBox.items[i].text.str);
		el->SetImage(comboBox.items[i].image);
		el->SetIndex(i);
		AddElement(el);
	}	
}

//Лист был обновлён
void FormComboBoxList::OnListUpdated()
{	
}

//Проверить клик за пределами окна
void FormComboBoxList::MouseDown(int button, const GUIPoint& pt)
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	if(!r.Inside(pt))
	{
		options->gui_manager->Close((GUIWindow *)GetParent());	
	}
}

//Был поворот колеса, true подвинуть автоматически фокус
bool FormComboBoxList::OnWeel(bool isUp)
{
	//Блокируем перемещение фокуса на заданную дистанцию
	isBlockAutofocus = true;	
	options->gui_manager->GetCursor(blockMousePosition.x, blockMousePosition.y);
	return true;
}


FormComboBoxListModal::FormComboBoxListModal(FormComboBox & combo, long itemsCount, const GUIRectangle & rect, bool isDisableSort) : GUIWindow(&combo, rect.x, rect.y, rect.w, rect.h)
{
	SetDrawRect(rect);
	SetClientRect(rect);
	//GUIWindow::Draw();
	bPopupStyle = true;
	bSystemButton = false;	
	bSystemButton = false;
	bMovable = false;
	bAlwaysOnTop = true;
	list = NEW FormComboBoxList(combo, this, GUIRectangle(0, 0, rect.w, rect.h), isDisableSort);
	UpdatePopupStyle();
	GUIWindow::Draw();
}

FormComboBoxListModal::~FormComboBoxListModal()
{
}

void FormComboBoxListModal::OnCreate()
{
	options->kbFocus = list;
	options->gui_manager->SetKeyboardFocus(list);
}

//Установить заголовок списка
void FormComboBoxListModal::SetTitle(GUIImage * icon, const char * text)
{
	list->SetTitle(icon, text);
}

//Выделить элемент
void FormComboBoxListModal::SelectElement(long index)
{
	list->SetFocus(index);
}

//Рисование
void FormComboBoxListModal::Draw()
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->black2Bkg[7]);
	GUIControl::Draw();
}
