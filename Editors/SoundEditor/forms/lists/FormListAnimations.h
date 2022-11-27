//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormListAnimations
//============================================================================================

#pragma once

#include "FormListBase.h"

class FormButton;

//Элемент списка
class FormLAniElement : public FormListElement
{
public:
	enum Consts
	{
		c_buttonsarea = 30,
		c_image_w = 16,
		c_image_h = 16,
		c_wavesarea = 60,
		c_namearea = 256,
	};

public:
	FormLAniElement(FormListBase & _list, ProjectAnimation * pa, long m);
	virtual ~FormLAniElement();

	//Получить анимацию
	const UniqId & GetAnimation();
	//Получить индекс ролика
	long GetMovie();
	//Изменить стадию ролика
	void DoChangeMovieStage();
	//Обновить состояние стадии ролика
	void UpdateMovieStage();

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
	//На линию был установлен фокус
	virtual void OnSetFocus();
	//Установить картинку для состояния
	void SetStageImage(dword mst);

protected:
	ExtName name;
	GUIImage * image;
	GUIImage * stage;
	UniqId animationId;
	long movie;
	dword mst;
	char buffer[256];
};

class FormListAnimations : public FormListBase
{
	friend class FormLAniElement;
public:
	FormListAnimations(FormListAnimationsOptions & opt, GUIControl * parent, const GUIRectangle & rect);
	virtual ~FormListAnimations();


public:
	//Получить анимацию в фокусе
	const UniqId & GetFocusAnimation();
	//Получить анимацию в фокусе
	long GetFocusMovie();
	//Обновить состояние стадии роликов
	void UpdateMovieStages();

private:
	//Инициализировать лист
	virtual void DoInitList();
	//Подняться по иерархии
	virtual void DoUpByHerarchy();
	//Опуститься по иерархии ниже
	virtual void DoEnterTo(FormListElement * element);
	//Событие изменения фокуса
	virtual void OnChangeFocus();
	//Лист был обновлён
	virtual void OnListUpdated();
	//Подтвердить изменение стадии ролика
	void DoChangeMovieStageAccept(long x, long y);

private:
	void _cdecl DoChangeMovieStageProcess(GUIControl* sender);
	void _cdecl DoAddNewAnimation(GUIControl* sender);
	void _cdecl DoAddNewAnimationProcess(GUIControl* sender);
	void _cdecl DoDelAnimation(GUIControl* sender);
	void _cdecl DoDelAnimationProcess(GUIControl* sender);
	void _cdecl DoExpAnimation(GUIControl* sender);
	void _cdecl DoAddNewMovie(GUIControl* sender);
	void _cdecl DoAddNewMovie_Check(GUIControl* sender);
	void _cdecl DoAddNewMovie_Ok(GUIControl* sender);
	void _cdecl DoCopyMovie(GUIControl* sender);
	void _cdecl DoSetMission(GUIControl* sender);
	void _cdecl DoSetMission_Ok(GUIControl* sender);
	void _cdecl DoRename(GUIControl* sender);
	void _cdecl DoRename_Check(GUIControl* sender);
	void _cdecl DoRename_Ok(GUIControl* sender);
	void _cdecl DoDelMovie(GUIControl* sender);
	void _cdecl DoDelMovie_Ok(GUIControl* sender);

private:
	FormListAnimationsOptions & formOptions;
	FormButton * buttonNew;
	FormButton * buttonCopy;
	FormButton * buttonMission;
	FormButton * buttonRename;	
	FormButton * buttonDel;
	FormButton * buttonExport;
	string misName;
	long makeCopyFrom;
};









