//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeBase	
//============================================================================================

#ifndef _GraphNodeBase_h_
#define _GraphNodeBase_h_

#include "..\AnxBase.h"
#include "GraphLinks.h"

enum AnxGraphKeyStates
{
	agks_shift = 1,
	agks_ctrl = 2,
	agks_alt = 4,
	agks_space = 8,
};

class GraphManager;

class GraphNodeBase  
{
	friend GraphManager;
//--------------------------------------------------------------------------------------------
public:
	GraphNodeBase(AnxOptions & options);
	virtual ~GraphNodeBase();

//--------------------------------------------------------------------------------------------
public:
	//Нарисовать детей
	virtual void DrawChilds(GUIPoint pos, IRender * render, GUICliper & clipper);
	//Нарисовать нод
	virtual void Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper);
	//Найти нод среди детей
	virtual GraphNodeBase * ChildByPosition(const GUIPoint & pnt);
	//Снять выделение со всех детей
	void DeselectAllChilds();

	//Активировать
	virtual void Activate(){};

	//Можно отводить линк
	virtual bool CanFromLink();
	//Можно присоединять линк
	virtual bool CanToLink();
	//Нормальный линк подходит к ноду или фейковый
	virtual bool IsFakeLink();

	//Записать данные в поток
	virtual void Write(AnxStream & stream);
	//Прочитать данные из потока
	virtual void Read(AnxStream & stream, GraphNodeBase * importTo);
	//Ретранслировать данные при импорте
	virtual void ImportTranslate();
	
	//Собрать список антов с выделеных нодов, которые надо сохранить
	virtual void CollectAntsFromNode(array<long> & tbl);

	//Собрать список антов с выделеных нодов, которые надо сохранить
	void CollectSelectionAnts(array<long> & tbl);
	//Сохранить выделенные ноды и линки в потоке под фиктивным нодом
	long WriteSelection(AnxStream & stream);

	//Записать данные дочернего нода в поток
	void WriteChild(AnxStream & stream, long index);
	//Прочитать данные дочернего нода из потока
	void ReadChild(AnxStream & stream, bool importMode);

	//Получить тип нода
	virtual AnxNodeTypes GetType();

//--------------------------------------------------------------------------------------------
public:

	//Проверить поподание в нод
	virtual bool Inside(const GUIPoint & pnt);
	//Найти по нод имени
	GraphNodeBase * Find(const char * nodeName);
	//Виден ли данный нод (по rect)
	bool IsView(const GUIPoint & pos);

public:
	AnxOptions & opt;					//Опции	
	GraphNodeBase * parent;				//Родительский нод
	array<GraphNodeBase *> child;		//Дочернии ноды
	string name;						//Название нода	
	GUIRectangle rect;					//Позиция и размер нода
	GUIPoint base;						//Позиция листа
	bool select;						//Выделение	
	GraphLinks links;					//Линки в текущем ноде	
	array<GraphLink *> conectedLinks;	//Линки, подключённые к ноду
};

#endif

