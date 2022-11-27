//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormWaveSelector
//============================================================================================

#pragma once

#include "FormListBase.h"

//Элемент списка
class FormLWSElement : public FormListElement
{
public:
	enum Consts
	{
		c_buttonsize = 16,
		c_buttonspace = 4,
		c_buttonborder = 24,
		c_buttonsarea = (c_buttonsize + c_buttonspace)*2 + c_buttonborder*2,
		c_wavearea = 80,
		c_timearea = 80,
		c_namearea = 256,
		

		evt_restoreforcus = 1,
	};

public:
	FormLWSElement(FormListBase & _list, const UniqId & w, const UniqId & f);
	virtual ~FormLWSElement();

	bool IsThisFolder(const UniqId & fld);
	const UniqId & GetWave();
	const UniqId & GetFolder();

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
	ProjectWave * wave;
	bool waveIsDefect;
	UniqId waveId;
	UniqId folder;
	float playHighlight;
	float stopHighlight;
	char nameData[256];
};

class FormWaveSelector : public FormListBase
{
public:
	FormWaveSelector(FormWaveSelectorOptions & opt, GUIControl * parent, const GUIRectangle & rect);
	virtual ~FormWaveSelector();

	//Инициализировать лист
	virtual void DoInitList();
	//Подняться по иерархии
	virtual void DoUpByHerarchy();
	//Опуститься по иерархии ниже
	virtual void DoEnterTo(FormListElement * element);
	//Лист был обновлён
	virtual void OnListUpdated();
	//Пришёл клавиатурный фокус
	virtual void OnKbFocus();

	//Заполнить лист для текущей папки
	void FillList();

	//Получить текущую папку
	const UniqId & GetCurrentFolder();


	//Получить волну в фокусе
	const UniqId & GetFocusWave();
	//Получить папку в фокусе
	const UniqId & GetFocusFolder();
	//Можно ли что-то сделать с выделенным элементом
	bool IsCanProcess();
	//Получить выделенные элементы
	void GetSelectElements(array<UniqId> & elements);

private:
	FormWaveSelectorOptions & formOptions;
};









