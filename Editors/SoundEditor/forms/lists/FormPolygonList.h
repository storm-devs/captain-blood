

#pragma once

#include "FormListBase.h"


class FormPolygonElement : public FormListElement
{
public:
	enum ElementType
	{
		et_sound,
		et_geometry,
	};


public:
	FormPolygonElement(FormListBase & _list, const char * _name, ElementType _type, void * _ptr);
	virtual ~FormPolygonElement();
	//Нарисовать линию
	virtual void Draw(const GUIRectangle & rect);
	//Если параметры линия должна быть выше чем данная то вернуть true
	virtual bool IsLessThen(const FormListElement * el) const;
	//Проверить на совпадение итема
	virtual bool IsThis(const char * str, dword hash, dword len) const;
	//Проверить попадание по префиксу
	virtual bool IsPreficsEqual(const char * pref) const;

public:
	ElementType type;
	ExtNameStr name;
	void * ptr;
	UniqId soundId;
};

class FormPolygonList : public FormListBase
{
public:
	FormPolygonList(GUIControl * parent, GUIRectangle rect);
	virtual ~FormPolygonList();

	//Добавить элемент
	void Add(const char * name, FormPolygonElement::ElementType type, void * ptr);
	//Удалить элемент
	void Del(dword index);
	//Получить количество элементов
	dword GetElementsCount();
	//Получить указатель элемента
	void * GetElementPtr(dword index);
	//Получить тип элемента
	FormPolygonElement::ElementType GetElementType(dword index);
	//Выделен ли итем
	bool GetElementSelect(dword index);
	//Элемент с фокусом
	long GetFocusIndex();
	//Установить фокус
	void SetFocusIndex(dword index);

	//Получить имя добавляемого звука
	const char * GetAddSoundName();
	//Получить имя добавляемой модельки
	const char * GetAddModelName();
	//Получить путь добавляемой модельки
	const char * GetAddModelPath();

public:
	GUIEventHandler onCreateSound;
	GUIEventHandler onCreateModel;
	GUIEventHandler onDeleteItems;

private:
	virtual void DoInitList();
	void _cdecl OnAddSound(GUIControl * sender);
	void _cdecl OnAddSoundOk(GUIControl * sender);
	void _cdecl OnAddModel(GUIControl * sender);
	void _cdecl OnAddModelOk(GUIControl * sender);
	void _cdecl OnDeleteItems(GUIControl * sender);
	void _cdecl OnCheckNameForEmpty(GUIControl * sender);

private:
	array<FormPolygonElement *> elements;
	FormButton * buttonAddSound;
	FormButton * buttonAddModel;
	FormButton * buttonDelete;
	string creationSoundName;
	string creationModelPath;
};


