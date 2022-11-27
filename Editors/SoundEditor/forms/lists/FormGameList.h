#pragma once

#include "FormListBase.h"

class FormGameListElement : public FormListElement
{
public:
	enum State
	{
		s_ok,
		s_cantexport,	//Звук в текущем проекте не экспортиться
		s_notinbank,	//Нет данного звука в банке
	};

public:
	FormGameListElement(FormListBase &owner, ProjectSound * sound, State state);

public:
	//Получить идентификатор звука
	const UniqId & GetId();
	//Установить состояние
	void SetState(State state);

private:
	//Нарисовать линию
	virtual void Draw(const GUIRectangle &rect);
	//Если линия должна быть выше чем в параметре то вернуть true
	virtual bool IsLessThen(const FormListElement *el) const;
	//Проверить на совпадение итема
	virtual bool IsThis(const char *str, dword hash, dword len) const;
	//Проверить попадание по префиксу
	virtual bool IsPreficsEqual(const char *pref) const;

private:
	ExtNameStr name;
	dword color;
	const GUIImage * image;
	UniqId id;
};

class FormGameList : public FormListBase
{
public:
	FormGameList(GUIControl * parent, GUIRectangle & rect);
	~FormGameList();

	//Получить текущий редактируемый звук
	const UniqId & GetFocusElement();

private:
	//Инициализировать лист
	virtual void DoInitList();
	//Рисование
	virtual void Draw();
	//Обновить звук
	FormGameListElement::State UpdateSound(ProjectSound * sound, bool isCheckIds);
	//Изменить состояние звука
	void ChangeState(const UniqId & id, FormGameListElement::State state);

private:
	dword updatePointer;
};
