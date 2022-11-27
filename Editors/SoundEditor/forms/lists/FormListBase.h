//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormListBase
//============================================================================================

#pragma once

#include "..\..\SndOptions.h"
#include "..\FormOptions.h"

#include "..\FormButton.h"
#include "..\FormEdit.h"


class FormListBase;
class FormScrollBar;

//Данные для эвента списка
struct FormListEventData
{
	dword id;
};

//Элемент списка
class FormListElement
{
	friend class FormListBase;
public:
	FormListElement(FormListBase & _list);
	virtual ~FormListElement();

public:
	//Установить на линию фокус
	void SetFocus();
	//Выделена ли линия
	bool IsSelect() const;
	//Здесь ли фокус
	bool IsFocus() const;
	//Получить индекс линии
	long GetIndex() const;
	//Получить список которому принадлежит линия
	FormListBase & List();
		
protected:
	//Нарисовать линию
	virtual void Draw(const GUIRectangle & rect) = null;
	//Если параметры линия должна быть выше чем данная то вернуть true
	virtual bool IsLessThen(const FormListElement * el) const = null;
	//Проверить на совпадение итема
	virtual bool IsThis(const char * str, dword hash, dword len) const = null;
	//Проверить попадание по префиксу
	virtual bool IsPreficsEqual(const char * pref) const = null;
	//Если на данном щелчке можно проводить операции селекта, вернуть true
	virtual bool MouseClick(const GUIRectangle & rect, const GUIPoint & pos);
	//Эвенты
	virtual void Event(const FormListEventData & data);
	//Над линией едет мышиный курсор
	virtual void OnMouseMove(const GUIRectangle & rect, const GUIPoint & pos);
	//На линию был установлен фокус
	virtual void OnSetFocus();
	//С линии убрали фокус
	virtual void OnLostFocus();
	//Линию выбрали
	virtual void OnSetSelect();
	//Линия потеряла выбор
	virtual void OnLostSelect();
	//Активировать элемент
	virtual void OnAction();

protected:
	//Разослать эвент остальным всем элементам списка
	void SendEvent(const FormListEventData & data);
	//Разослать эвент остальным всем элементам списка
	void SendEvent(dword id);
	//На следующем кадре подняться выше по иерархии
	void UpByHerarchy();
	//На следующем кадре перейти в этот элемент
	void Enter();
	//В клавиатурном фокусе ли лист
	bool IsKbFoces();
	//Нарисовать курсор
	void DrawCursor(const GUIRectangle & rect);
	//Нарисовать выделение
	void DrawSelect(const GUIRectangle & rect);
	//Нарисовать иконку
	void DrawIcon(const GUIRectangle & rect, const GUIImage * image);
	//Вывести текст
	void DrawText(const GUIRectangle & rect, const ExtName * text, bool isImageSpace = true, dword color = options->colorTextLo);

protected:
	bool isSelect;
private:
	long index;
	FormListBase & list;
};

//Базовай класс списка
class FormListBase : public GUIControl
{
	friend class FormListElement;
public:
	enum Consts
	{
		spellingWaitTimeInMs = 900,					//Время ожидания нажатия следующей клавиши в миллисекундах
		c_path_leftspace_no_image = sgc_header_left_space,
		c_path_height = sgc_header_h,
		c_path_image_size = sgc_header_image_size,
		c_path_leftspace = sgc_header_left_space + c_path_image_size + sgc_header_controls_space,
		c_line_height = 22,
		c_line_leftspace = c_path_leftspace,
		c_line_image_size = sgc_header_image_size,
		c_scroll_width = 20,
		c_border = 4,
		c_space = 10,
	};

private:
	//Строка в списке
	struct ListLine
	{
		GUIRectangle rect;
		FormListElement * element;
	};

public:
	FormListBase(GUIControl * parent, GUIRectangle rect, bool _disableAutoDelete = false);
	virtual ~FormListBase();

	//Найти реальную высоту списка исходя из данной высоты
	static long FindListHeight(dword srcHeight, long * count = null);

public:
	//Инициализировать лист
	void InitList();
	//Получить количество строк
	long GetNumLines();
	//Получить линию
	FormListElement * GetLine(long line);
	//Установить режим для рамки
	void SetFrame(bool isDraw);
	//Установить фокус
	void SetFocus(long line);
	//Установить фокус на нужном элементе
	void SetFocus(const char * name);
	//Переместить фокус и проследить чтобы он был видн
	void MoveFocus(long delta);
	//Получить индекс элемента на котором фокус
	long GetFocus();
	//Установить/снять выделение
	void SetSelect(long from, long to, bool isSelect);
	//Получить количество выделенных
	dword GetSelectCount();
	//Узнать выделена ли строка
	bool IsSelect(long line);
	//Получить список выделенных
	void FillSelectList(array<FormListElement *> & selected);

public:
	//Добавить кнопку к списку и откоректировать прямоугольник для следующей
	FormButton * ButtonAdd();
	//Удалить кнопки (очистить смещение отведёное для кнопок)
	void ButtonsClear();
	
protected:
	//Инициализировать лист
	virtual void DoInitList() = null;
	//Подняться по иерархии
	virtual void DoUpByHerarchy();
	//Опуститься по иерархии ниже
	virtual void DoEnterTo(FormListElement * element);
	//Лист был обновлён
	virtual void OnListUpdated();
	//Получить нажатую кнопку
	virtual void OnKeyPressed(dword code, dword isSys);
	//Событие изменения фокуса
	virtual void OnChangeFocus();
	//Пришёл клавиатурный фокус
	virtual void OnKbFocus();
	//Был поворот колеса, true подвинуть автоматически фокус
	virtual bool OnWeel(bool isUp);

protected:
	//Рисование
	virtual void Draw();
	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	//Добавить элемент в список
	void AddElement(FormListElement * el);
	//Разослать эвент всем элементам списка
	void SendEvent(const FormListEventData & data);
	//Установить новый путь
	void SetCaption(const char * str, GUIImage * image);	
	//Запретить сортировку
	void DisableSort(bool isDisable);

private:
	//Попытаться установить фокус на строку префикса
	bool TrySetFocus(const char * prefix);
	//Очистить список
	void SystemClearList(FormListElement * exludeThat);
	//Обновить список
	void SystemPrepareList();
	//Функция сравнения для сортировки
	static bool SystemListLineSort(FormListElement * const & v1, FormListElement * const & v2);
	//Обновляем отображаемый список
	void SystemUpdateDrawList();
	//Проверить попадание фокуса в видимую область
	void SystemCheckFocus();
	//На следующем кадре подняться выше по иерархии
	void SystemUpByHerarchy();
	//На следующем кадре перейти в папку
	void SystemEnterTo(FormListElement * element);

public:
	GUIEventHandler onChangeFocus;		//Событие изменения фокуса

private:
	ExtName caption;					//Текущий путь
	string captionData;					//Данные строки пути преобразованной под размер
	GUIImage * imagePath;				//Текущая картинка для пути
	array<FormListElement *> elements;	//Список элементов
	array<ListLine> list;				//Видимые элементы	
	long currentPos;					//Текущая отображаемая позиция
	long focusPosition;					//Элемент на котором фокус
	dword lineWidthWoScroll;			//Ширина строк без скрол бара
	dword lineWidthWScroll;				//Ширина строк c скрол баром
	bool isDrawFrame;					//Рисовать рамку для списка
	bool weelBlocker;					//Только 1 раз за кадр обрабатываем колесо
	bool needMoveUH;					//Вверх по иерархии
	bool disableAutoDelete;				//Не удалять итемы
	bool isDisableSort;					//Запретить сортировку
	FormListElement * needMoveDH;		//Вниз по иерархии
	FormScrollBar * scrollBar;			//Полоска прокручивания
	dword buttonsWidth;					//Область занимаемая кнопками в заголовке
	char spellingBuffer[16];			//Буфер для поиска по набору текста
	float spellingTimer;				//Таймер, сбрасывающий буфер поиска
	ExtNameStr spellingDraw;			//Строка для рисования
};

//Выделена ли линия
__forceinline bool FormListElement::IsSelect() const
{
	return isSelect;
}

//Здесь ли фокус
__forceinline bool FormListElement::IsFocus() const
{
	return (list.focusPosition == index);
}

//Получить индекс линии
__forceinline long FormListElement::GetIndex() const
{
	return index;
}

//Получить список которому принадлежит линия
__forceinline FormListBase & FormListElement::List()
{
	return list;
}

//Получить количество строк
__forceinline long FormListBase::GetNumLines()
{
	return elements;
}

//Получить линию
__forceinline FormListElement * FormListBase::GetLine(long line)
{
	if(line < 0 || line >= elements)
	{
		return null;
	}
	return elements[line];
}

//Получить индекс элемента на котором фокус
__forceinline long FormListBase::GetFocus()
{
	return focusPosition;
}

//Узнать выделена ли строка
__forceinline bool FormListBase::IsSelect(long line)
{
	if(line < 0 || line >= elements)
	{
		return false;
	}
	return elements[line]->IsSelect();
}



