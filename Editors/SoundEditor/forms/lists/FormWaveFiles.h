//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormWaveFiles
//============================================================================================

#pragma once

#include "FormListBase.h"

//Элемент списка
class FormLWFElement : public FormListElement
{
public:
	enum Consts
	{
		c_buttonsarea = 56,
		c_wavearea = 80,
		c_timearea = 96,
		c_namearea = 256,
	};


public:
	FormLWFElement(FormListBase & _list, bool isFld, bool isDots, const char * path);
	virtual ~FormLWFElement();

public:
	//Можно ли войти в эту папку
	void CheckEnterTo();
	//Получить полный путь
	const char * GetFullPath();
	//Получить имя
	const char * GetName();
	//Файл ли
	bool IsFile();

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
	string fullPath;
	string nameData;
	ExtName name;
	bool isFolder;
	bool isUp;
};

class FormWaveFiles : public FormListBase
{
public:
	FormWaveFiles(FormWaveFilesOptions & opt, GUIControl * parent, const GUIRectangle & rect);
	virtual ~FormWaveFiles();

private:
	//Инициализировать лист
	virtual void DoInitList();
	//Подняться по иерархии
	virtual void DoUpByHerarchy();
	//Опуститься по иерархии ниже
	virtual void DoEnterTo(FormListElement * element);
	//Лист был обновлён
	virtual void OnListUpdated();
	//Получить нажатую кнопку
	virtual void OnKeyPressed(dword code, dword isSys);
	//Пришёл клавиатурный фокус
	virtual void OnKbFocus();

	//Заполнить лист для текущей папки
	void FillList();

private:
	//Импортировать выделенные файлы в проект
	void ImportFiles();
	//
	void _cdecl OnImport(GUIControl* sender);


private:
	FormWaveFilesOptions & formOptions;
	FormButton * buttonImport;
};









