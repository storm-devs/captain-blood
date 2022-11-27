//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormListNodes
//============================================================================================

#pragma once

#include "FormListBase.h"

class FormButton;

//Элемент списка
class FormLNodeElement : public FormListElement
{
public:
	FormLNodeElement(FormListBase & _list, ProjectAnimation * pa, long n, string & tmp);
	virtual ~FormLNodeElement();

	//Получить индекс нода
	long GetNode();
	//Получить имя
	const char * GetName();

protected:
	//Нарисовать линию
	virtual void Draw(const GUIRectangle & rect);
	//Если параметры линия должна быть выше чем данная то вернуть true
	virtual bool IsLessThen(const FormListElement * el) const;
	//Проверить на совпадение итема
	virtual bool IsThis(const char * str, dword hash, dword len) const;
	//Проверить попадание по префиксу
	virtual bool IsPreficsEqual(const char * pref) const;
	//Активировать элемент
	virtual void OnAction();


protected:
	ExtName name;
	long node;
	bool notValidForUse;
	char buffer[128];
};

class FormListNodes : public FormListBase
{
	friend class FormLNodeElement;
public:
	FormListNodes(FormListAnimationsOptions & opt, GUIControl * parent, const GUIRectangle & rect);
	virtual ~FormListNodes();


public:
	//Получить анимацию в фокусе
	long GetFocusNode();
	//Разрешить - запретить
	void SetEnabled(bool isEnable);

public:
	GUIEventHandler onAddNode;		//Событие добавления нода

private:
	//Инициализировать лист
	virtual void DoInitList();
	//Событие изменения фокуса
	virtual void OnChangeFocus();
	//Лист был обновлён
	virtual void OnListUpdated();

private:
	void _cdecl DoAddNewNode(GUIControl* sender);

private:
	FormListAnimationsOptions & formOptions;
	FormButton * buttonNew;
	bool isSaveFocus;
};









