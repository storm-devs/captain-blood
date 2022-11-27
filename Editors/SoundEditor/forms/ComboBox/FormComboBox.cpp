


#include "FormComboBox.h"
#include "..\..\SndOptions.h"
#include "FormComboBoxList.h"



FormComboBox::FormComboBox(GUIControl * parent, const GUIRectangle & frame, const GUIRectangle & list, bool enableSort)	: GUIControl(parent),
																											items(_FL_, 256)
{
	listPanel = null;
	listRect = list;
	listRect.h = FormListBase::FindListHeight(listRect.h);
	listRect.x -= frame.x;
	listRect.y -= frame.y;
	SetClientRect(frame);
	SetDrawRect(frame);
	currentItem = -1;
	unselectedText.SetString(null);
	titleImage = null;
	isChangeItem = false;
	isDisableSort = !enableSort;
}

FormComboBox::~FormComboBox()
{
	
}

//Установить заголовок списка
void FormComboBox::SetTitle(GUIImage * icon, const char * text)
{
	titleImage = icon;
	titleText = text;
}

//Установить текст, когда элемент не выделен
void FormComboBox::SetUnselectText(const char * text)
{
	unselectedText.SetString(text);
}

//Добавить элемент
void FormComboBox::AddItem(GUIImage * icon, const char * text, long id, const UniqId & uid)
{
	Item & item = items[items.Add()];
	item.image = icon;
	item.text.SetString(text);
	item.id = id;
	item.uid = uid;
}

//Установить текущий итем
bool FormComboBox::SelectItem(long index)
{	
	if(index >= 0 && index < items)
	{
		if(currentItem != index)
		{
			isChangeItem = true;
			currentItem = index;
		}		
		return true;
	}
	if(currentItem >= 0)
	{
		isChangeItem = true;
		currentItem = -1;
	}	
	return false;	
}

//Установить текущий элемент с данным идентификатором
bool FormComboBox::SelectItemById(long id)
{
	for(long i = 0; i < items; i++)
	{
		if(items[i].id == id)
		{
			return SelectItem(i);
		}
	}
	SelectItem(-1);
	return false;
}


//Установить текущий элемент с данным ид
bool FormComboBox::SelectItemByUId(const UniqId & uid)
{
	for(long i = 0; i < items; i++)
	{
		if(items[i].uid == uid)
		{
			return SelectItem(i);
		}
	}
	SelectItem(-1);
	return false;
}

//Установить текущий элемент с данным текстом
bool FormComboBox::SelectItemByText(const char * text)
{
	if(!text)
	{
		SelectItem(-1);
		return false;
	}
	dword len = 0;
	dword hash = string::HashNoCase(text, len);
	for(long i = 0; i < items; i++)
	{
		ExtNameStr & name = items[i].text;
		if(name.hash == hash && name.len == len)
		{
			if(string::IsEqual(name.str, text))
			{
				return SelectItem(i);
			}
		}
	}
	SelectItem(-1);
	return false;
}

//Получить индекс текущего элемента
long FormComboBox::GetCurrentItemIndex()
{
	return currentItem;
}

//Получить идентификатор текущего элемента, если нет, возвращает -1
long FormComboBox::GetCurrentItemId()
{
	if(currentItem >= 0 && currentItem < items)
	{
		return items[currentItem].id;
	}
	return -1;
}

//Получить ид текущего элемента
const UniqId * FormComboBox::GetCurrentItemUId()
{
	if(currentItem >= 0 && currentItem < items)
	{
		return &items[currentItem].uid;
	}
	return null;
}

//Получить текст текущего элемента
ExtName * FormComboBox::GetCurrentItemText()
{
	if(currentItem >= 0 && currentItem < items)
	{
		return &items[currentItem].text;
	}
	return null;
}

//Удалить все элементы
void FormComboBox::Clear()
{
	currentItem = -1;
	items.DelAll();
	if(listPanel)
	{
		options->gui_manager->Close(listPanel);
		listPanel = null;
	}
	Assert(!listPanel);
}

//Рисование
void FormComboBox::Draw()
{
	if(!Visible) return;	
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	dword colorBkg = options->bkg;
	dword colorFrame = options->black2Bkg[6];
	if(!options->dndControl && !listPanel)
	{
		if(options->IsHighlight(this))
		{
			colorBkg = options->bkg2White[2];
			colorFrame = options->black2Bkg[0];
		}
	}
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, colorBkg);
	if(currentItem >= 0)
	{
		Item & item = items[currentItem];
		long x = r.x + 5;
		long y = r.y + (r.h - item.text.h)/2;
		options->render->Print(float(x), float(y), options->colorTextLo, item.text.str);
	}else{
		if(unselectedText.str)
		{
			long x = r.x + 5;
			long y = r.y + (r.h - unselectedText.h)/2;
			options->render->Print(float(x), float(y), options->colorTextLo | 0x00ff0000, unselectedText.str);
		}
	}
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, colorFrame);
	if(Caption.Len() > 0)
	{
		float h = options->uiFont->GetHeight();
		options->render->Print(float(r.x), float(r.y - h - 3), options->colorTextLo, Caption.c_str());
	}
	if(isChangeItem)
	{		
		onChange.Execute(this);
		isChangeItem = false;
	}
}

//Сообщения
bool FormComboBox::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	if(message == GUIMSG_LMB_DOWN && !options->dndControl)
	{
		GUIPoint cursor_pos;
		GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
		//Проверяем на попадание
		GUIRectangle r = GetClientRect();
		ClientToScreen(r);
		if(r.Inside(cursor_pos))
		{
			//Попали в контрол, открываем список
			options->kbFocus = null;
			listPanel = NEW FormComboBoxListModal(*this, items.Size(), listRect, isDisableSort);
			listPanel->SetTitle(titleImage, titleText.c_str());
			listPanel->SelectElement(currentItem);
			options->gui_manager->ShowModal(listPanel);			
		}
	}
	return GUIControl::ProcessMessages(message, lparam, hparam);
}

//Выделить линию из списка
void FormComboBox::SetIndexFromList(long index)
{
	ClearListPtr();
	SelectItem(index);
	isChangeItem = true;
}

//Очистить указатель на лист
void FormComboBox::ClearListPtr()
{
	listPanel = null;
}
