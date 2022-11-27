

#pragma once

#include "..\lists\FormListBase.h"

class FormComboBox;

//Элемент списка
class FormCBLElement : public FormListElement
{
public:
	FormCBLElement(FormListBase & _list);
	virtual ~FormCBLElement();

public:
	//Установить итему текст
	void SetName(const char * str);
	//Установить итему картинку
	void SetImage(GUIImage * img);
	//Установить индекс итема
	void SetIndex(long index);

protected:
	//Нарисовать линию
	virtual void Draw(const GUIRectangle & rect);
	//Если параметры линия должна быть выше чем данная то вернуть true
	virtual bool IsLessThen(const FormListElement * el) const;
	//Проверить на совпадение итема
	virtual bool IsThis(const char * str, dword hash, dword len) const;
	//Проверить попадание по префиксу
	virtual bool IsPreficsEqual(const char * pref) const;
	//Если на данном щелчке можно проводить операции селекта, вернуть true
	virtual bool MouseClick(const GUIRectangle & rect, const GUIPoint & pos);
	//Эвенты
	virtual void Event(const FormListEventData & data);
	//Над линией едет мышиный курсор
	virtual void OnMouseMove(const GUIRectangle & rect, const GUIPoint & pos);
	//Линию выбрали
	virtual void OnSetSelect();
	//Активировать элемент
	virtual void OnAction();

protected:
	ExtNameStr name;
	GUIImage * image;
	long itemIndex;
};

class FormComboBoxList : public FormListBase
{
	friend class FormCBLElement;
public:
	FormComboBoxList(FormComboBox & combo, GUIControl * parent, const GUIRectangle & rect, bool isDisableSort);
	virtual ~FormComboBoxList();

public:
	//Установить заголовок списка
	void SetTitle(GUIImage * icon, const char * text);
	//Установить элементу текст и картинку
	//void SetElement(long index, GUIImage * icon, const char * text);
	//Заблокирован ли автофокус
	bool IsBlocked();

private:
	//Инициализировать лист
	virtual void DoInitList();
	//Лист был обновлён
	virtual void OnListUpdated();
	//Проверить клик за пределами окна
	virtual void MouseDown(int button, const GUIPoint& pt);
	//Был поворот колеса, true подвинуть автоматически фокус
	virtual bool OnWeel(bool isUp);

private:
	//Комбо-бокс, с которым работаем
	FormComboBox & comboBox;
	bool isBlockAutofocus;
	GUIPoint blockMousePosition;
};

class FormComboBoxListModal : public GUIWindow
{
public:
	FormComboBoxListModal(FormComboBox & combo, long itemsCount, const GUIRectangle & rect, bool isDisableSort);
	virtual ~FormComboBoxListModal();

	//Установить заголовок списка
	void SetTitle(GUIImage * icon, const char * text);
	//Установить элементу текст и картинку
	//void SetElement(long index, GUIImage * icon, const char * text);
	//Выделить элемент
	void SelectElement(long index);

private:
	virtual void OnCreate();
	//Рисование
	virtual void Draw();

private:
	FormComboBoxList * list;
};


