
#pragma once

#include "..\..\SndBase.h"

class FormComboBoxListModal;

class FormComboBox : public GUIControl
{
	struct Item
	{		
		ExtNameStr text;
		GUIImage * image;
		long id;
		UniqId uid;
	};

	friend class FormCBLElement;
	friend class FormComboBoxList;

public:
	FormComboBox(GUIControl * parent, const GUIRectangle & frame, const GUIRectangle & list, bool enableSort = true);
	virtual ~FormComboBox();
	
	//Установить заголовок списка
	void SetTitle(GUIImage * icon, const char * text);
	//Установить текст, когда элемент не выделен
	void SetUnselectText(const char * text);
	//Добавить элемент
	void AddItem(GUIImage * icon, const char * text, long id = -1, const UniqId & uid = UniqId::zeroId);
	//Установить текущий элемент
	bool SelectItem(long index);
	//Установить текущий элемент с данным идентификатором
	bool SelectItemById(long id);
	//Установить текущий элемент с данным ид
	bool SelectItemByUId(const UniqId & uid);
	//Установить текущий элемент с данным текстом
	bool SelectItemByText(const char * text);
	//Получить индекс текущего элемента
	long GetCurrentItemIndex();
	//Получить идентификатор текущего элемента, если нет, возвращает -1
	long GetCurrentItemId();
	//Получить ид текущего элемента
	const UniqId * GetCurrentItemUId();
	//Получить текст текущего элемента
	ExtName * GetCurrentItemText();
	//Удалить все элементы
	void Clear();


private:
	//Рисование
	virtual void Draw();
	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	//Выделить линию из списка
	void SetIndexFromList(long index);
	//Очистить указатель на лист
	void ClearListPtr();



private:
	GUIRectangle listRect;				//Размер и позиция открываемого списка
	array<Item> items;					//Элементы бокса
	string titleText;					//Заголовок списка
	GUIImage * titleImage;				//Картинка заголовка списка
	long currentItem;					//Индекс текущего выбранного элемента
	ExtNameStr unselectedText;			//Текст, который надо писать при отсутствие выделенного элемента
	FormComboBoxListModal * listPanel;	//Открытая панель листа
	bool isChangeItem;					//Был изменён элемент
	bool isDisableSort;					//Запретить сортировку
public:
	GUIEventHandler onChange;			//Событие изменения выбранного элемента
};