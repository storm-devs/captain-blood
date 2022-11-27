//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormSoundWaves
//============================================================================================


#pragma once


#include "FormListBase.h"

//Элемент списка
class FormSndWaveElement : public FormListElement
{
public:
	enum Consts
	{
		c_controlspace = 20,
		c_mousesafrarea = 8,
		c_controlbar = 4,
		c_ctrlhspace = 5,
		c_volwidth = 110,
		c_weightwidth = 110,
		c_timespace = 80,

		c_buttonsize = 16,
		c_buttonspace = 4,
		c_buttonsarea = (c_buttonsize + c_buttonspace)*3,

		c_controlarea_base = c_volwidth + c_weightwidth + c_buttonsarea + c_timespace + c_controlspace*4,
		c_shortname = 50,

		c_enter_area = 20,
		


		evt_restore_focus,
		evt_set_new_volume,
		evt_set_new_weight,
	};

	struct RestoreFocus : public FormListEventData
	{
		long waveIndex;
	};

	struct FloatParam : public FormListEventData
	{
		float param;
	};

public:
	FormSndWaveElement(FormListBase & _list, ProjectSound * s, long idx);
	virtual ~FormSndWaveElement();

	//Получить индекс записи в таблице звука
	long GetWaveIndex();
	//Получить идентификатор волны проекта
	const UniqId & GetProjectWaveId();

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

private:
	//Нарисовать бегунок
	void DrawSlider(const GUIRectangle & rect, long x, float normValue, long w, ExtName & label);

protected:
	ProjectSound * sound;
	ProjectWave * pw;
	long waveIndex;
	float playHighlightVol;
	float playHighlight;
	float stopHighlight;
	ExtName name;	
	char nameData[64];
	static long playWaveIndex;
	static ExtName volText;
	static ExtName wgtText;
};

class FormSoundWaves : public FormListBase
{
	enum Consts
	{
		c_waves_list_width = 700,
		c_waves_list_height = 500,
	};

public:
	FormSoundWaves(FormSoundWavesOptions & opt, GUIControl * parent, const GUIRectangle & rect, bool isLightVersion);
	virtual ~FormSoundWaves();

	//Инициализировать лист
	virtual void DoInitList();
	//Лист был обновлён
	virtual void OnListUpdated();
	//Установить новый звук
	void SetNewSound(const UniqId & id);
	//Получить идентификатор текущей волны
	const UniqId & GetProjectWaveIdFormFocus();
	//Короткая версия
	bool IsShortVersion();

private:
	void _cdecl OnWavesAdd(GUIControl* sender);
	void _cdecl OnWavesAddSilence(GUIControl* sender);
	void _cdecl OnChooseWavesOk(GUIControl* sender);
	void _cdecl OnWavesDel(GUIControl* sender);

private:
	FormSoundWavesOptions & formOptions;
	ProjectSound * sound;
	FormButton * buttonAdd;
	FormButton * buttonAddSilence;
	FormButton * buttonDelete;
	bool isShortIems;
};

//Редактирование дополнительного времени
class FormEditTime : public GUIWindow
{
	enum Consts
	{
		c_width = 500,
		c_height = 200,
		c_border = 15,
		c_space = 20,
		c_edit_w = 100,
		c_barheight = 30,
		c_button_width = 130,
		c_button_height = 25,		
	};

public:
	FormEditTime(GUIControl * parent, const GUIPoint & p, ProjectSound * s, long wi);
	virtual ~FormEditTime();

private:
	//Рисование
	virtual void Draw();
	void _cdecl OnOkEdit(GUIControl* sender);
	void _cdecl OnCancelEdit(GUIControl* sender);
	void _cdecl OnEditChange(GUIControl* sender);
	void _cdecl OnEditAccept(GUIControl* sender);


protected:
	FormButton * buttonOk;
	FormButton * buttonCancel;
	FormEdit * edit;
	float waveTime;
	float activeTime;
	ExtName actLabel;
	ProjectSound * sound;
	long waveIndex;
};







