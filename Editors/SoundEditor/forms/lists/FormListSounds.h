//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormListSounds
//============================================================================================

#pragma once

#include "FormListBase.h"

//Элемент списка
class FormLSndElement : public FormListElement
{
public:
	enum Consts
	{
		c_buttonsarea = 56,
		c_wavesarea = 60,
		c_namearea = 256,
	};

public:
	FormLSndElement(FormListBase & _list, const UniqId & s, const UniqId & f);
	virtual ~FormLSndElement();

	//Получить папку
	const UniqId & GetFolder();
	//Получить звук
	const UniqId & GetSound();

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
	//Активировать элемент
	virtual void OnAction();


protected:	
	ExtName name;
	GUIImage * image;
	UniqId sound;
	UniqId folder;
	char nameData[256];
};

class FormListSounds : public FormListBase
{
public:
	FormListSounds(FormListSoundsOptions & opt, GUIControl * parent, const GUIRectangle & rect);
	virtual ~FormListSounds();


public:
	//Установить новую родительскую папку
	void SetParentFolder(const UniqId & parentFolderId);
	//Получить текущую родительскую папку
	const UniqId & GetParentFolder();
	//Можно ли что-то сделать с выделенным элементом
	bool IsCanProcess();
	//Получить индекс текущего выделенного фокусом звука
	const UniqId & GetFocusSound();
	//Получить индекс текущей выделенной фокусом папки
	const UniqId & GetFocusFolder();
	//Получить список выделеных элементов
	void GetSelectItems(array<UniqId> & selectItems);

private:
	//Инициализировать лист
	virtual void DoInitList();
	//Подняться по иерархии
	virtual void DoUpByHerarchy();
	//Опуститься по иерархии ниже
	virtual void DoEnterTo(FormListElement * element);
	//Лист был обновлён
	virtual void OnListUpdated();

	//Заполнить лист для текущей папки
	void FillList();

private:
	FormListSoundsOptions & formOptions;
};









