#pragma once

#include "BaseGUIElement.h"

class GUITable : public MissionObject
{
	struct Item
	{
		float x;
		float y;

		int elem;
	};

	struct Element
	{
		union Panel
		{
			const char *name;
			BaseGUIElement *p;
		}
		panel;			// контейнер для элемента

		struct Number
		{
			const char *name;
			ICoreStorageFloat *p;
		}
		number;			// переменная из базы игры


		struct Activity
		{
			const char *name;
			ICoreStorageFloat * p;
		}
		activity;			// переменная из базы игры


		int val;

		int item;

		float from_x;
		float from_y;

		float to_x;
		float to_y;

		float time;

		union Widget
		{
			const char *name;
			class GUIWidget *p;
		}
		widget;			// виджет для отображения значения переменной
	};

public:

	 GUITable();
	~GUITable();

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

	void UpdateActivity();

private:

	void _cdecl Update(float dltTime, long level);

private:

	float delay;

	array<Item>	   items;
	array<Element> elems;

	bool show;

};
