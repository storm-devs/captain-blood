#pragma once

#include "BaseGUIElement.h"

class BaseGUITable : public MissionObject
{
	struct Element
	{
		Element();

		struct Widget
		{
			const char *name; 
			class GUIWidget *p;
		} widget;			// виджет для отображения значения переменной

		struct Number
		{
			const char *name;
			ICoreStorageFloat *p;
		} number;			// переменная из базы игры
		
		struct Title
		{
			const char *name; 
			MissionObject *p;
		} title;			// объект, появляющийся при активации элемента

		float delay;	// задержка перед активацией элемента

		bool rel;		// используется относительный путь для переменной
		bool play;		// отыгрывать анимацию набора значения
	};

public:

	 BaseGUITable();
	~BaseGUITable();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void InitParams		(MOPReader &reader);

	void PostCreate();

	void Show(bool isShow);

	void Restart();

	void Command(const char *id, dword numParams, const char **params);

private:

	void SetTable(const char *diff);

private:

	void _cdecl Update(float dltTime, long level);

private:
	ConstString tickSoundName;
	bool tickPeriodic;
	float tickCooldown, tickPeriodicTime;
	ConstString tickEndSoundName;
	ISound * tickSound;

	array<Element> elems;

	MissionTrigger complete;

	bool show;

	int index;	// индекс текущего элемента

	float time;

	bool wait;	// ждем, пока не отыграется анимация текущего элемента

	bool skip;	// скипаем все анимации и задержки, моментально выводим всю таблицу

	const char *menu_control;

	long menu;	// контрол для скипания анимации таблицы

	ISound *sound; float fadeOut;

};
