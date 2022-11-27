//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeAnimation	
//============================================================================================

#ifndef _GraphNodeAnimation_h_
#define _GraphNodeAnimation_h_

#include "GraphNodeBase.h"
#include "..\\AntFile.h"

class GraphNodeAnimation : public GraphNodeBase
{
public:
	//Событие
	struct Event
	{
		Event();
		~Event();
		
		//Записать данные в поток
		virtual void Write(AnxStream & stream);
		//Прочитать данные из потока
		virtual void Read(AnxStream & stream, dword version = 5);

		string info;				//Описание события
		string name;				//Имя события
		dword frame;				//Кадр срабатывания
		array<string *> params;		//Параметры события
		bool noBuffered;			//Не буферезированный эвент
	};

	//Клип
	struct Clip
	{
		Clip(AnxOptions & options);
		~Clip();
		//Удалить событие по имени
		void DeleteEventsByName(const char * name);

		AntFile data;				//Данные клипа
		array<Event *> events;		//События данного клипа

		void ExtractInfo(Clip & source, bool isMovment);
	};

	//Константа
	struct Const
	{
		Const();
		enum Type
		{
			t_empty = 0,			//Не используемая константа
			t_string,				//Строка
			t_float,				//float
			t_blend,				//blend float
			t_max
		};
		string name;				//Имя константы
		string str;					//Строка
		float flt;					//float
		float bld;					//blend float
		Type type;					//Тип константы
	};

	//Блок констант
	struct ConstBlock
	{
		ConstBlock();
		~ConstBlock();
		//Записать данные в поток
		virtual void Write(AnxStream & stream);
		//Прочитать данные из потока
		virtual void Read(AnxStream & stream);

		string name;				//Имя блока
		string type;				//Тип блока
		//Константы
		array<Const *> consts;		//Константы блока
	};


//--------------------------------------------------------------------------------------------
public:
	GraphNodeAnimation(AnxOptions & options);
	virtual ~GraphNodeAnimation();

//--------------------------------------------------------------------------------------------
public:
	//Нарисовать нод
	virtual void Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper);

	//Активировать
	virtual void Activate();

	//Записать данные в поток
	virtual void Write(AnxStream & stream);
	//Прочитать данные из потока
	virtual void Read(AnxStream & stream, GraphNodeBase * importTo);

	//Собрать список антов с выделеных нодов, которые надо сохранить
	virtual void CollectAntsFromNode(array<long> & tbl);

	//Получить тип нода
	virtual AnxNodeTypes GetType();

	//Удалить событие по имени
	void DeleteEventsByName(const char * name);


//--------------------------------------------------------------------------------------------
public:
	//Клипы анимации в ноде
	array<Clip *> clips;
	//Блоки констант
	array<ConstBlock *> consts;
	//Флаги
	bool isLoop;
	bool isChange;
	bool isMovement;
	bool isGlobalPos;
};

#endif

