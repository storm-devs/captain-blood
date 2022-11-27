//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphManager	
//============================================================================================

#ifndef _GraphManager_h_
#define _GraphManager_h_

#include "..\AnxBase.h"
#include "GraphNodeBase.h"

class GraphManager
{
public:
	//Таблица трансляции имён нодов при импорте
	struct ImportTable
	{
		string oldNodeName;	//Старое название нода
		string newNodeName;	//Новое название нода
	};


//--------------------------------------------------------------------------------------------
public:
	GraphManager(AnxOptions & opt);
	virtual ~GraphManager();
	//Удалить граф
	void Release();


//--------------------------------------------------------------------------------------------
public:
	//Найти по нод имени
	GraphNodeBase * Find(const char * nodeName);
	//Определить, есть ли выделенные ноды
	bool IsHaveSelNodes();

	//Записать данные в поток
	void Write(AnxStream & stream);
	//Прочитать данные из потока
	void Read(AnxStream & stream);
	//Импортировать данные из потока
	void Import(AnxStream & stream, GraphNodeBase * importTo = null);
	//Экспортировать выделенные ноды в поток
	void Export(AnxStream & stream);

	//Удалить эвенты с заданным именем
	void DeleteEventsByName(const char * eventName[], dword count, GraphNodeBase * rootNode = null);
	//Удалить всё из графа
	void Clear();

	//Собрать все ноды в линейный список
	void CollectionNodes(array<GraphNodeBase *> & nodes, GraphNodeBase * rootNode = null);

//--------------------------------------------------------------------------------------------
public:
	//Отрисовка содержимого проекта
	void Draw(IRender * render, const GUIRectangle & rect, GUICliper & clipper);
	//Обрабатываем нажание левой кнопки мыши
	void MouseLeftDown(GUIPoint & pnt, dword flags);
	//Обрабатываем отпускание левой кнопки мыши
	void MouseLeftUp(GUIPoint & pnt, dword flags);
	//Обрабатываем нажание правой кнопки мыши
	void MouseRightDown(GUIPoint & pnt, dword flags);
	//Обрабатываем отпускание правой кнопки мыши
	void MouseRightUp(GUIPoint & pnt, dword flags);
	//Обрабатываем двойной щелчёк мыши
	void MouseDblClick(GUIPoint & pnt, dword flags);
	//Обрабатываем перемещение мышки
	void MouseMove(GUIPoint & pnt, GUIPoint & dpnt, dword flags);


//--------------------------------------------------------------------------------------------
public:
	//Опции
	AnxOptions & options;
	//Весь иерархический граф
	GraphNodeBase * root;
	//Текущая часть
	GraphNodeBase * current;
	//Разрешить двигать выделенные ноды
	bool modeMoveNodes;
	//Разрешить двигать граф
	bool modeMoveGraph;
	GUIPoint moveDelta;
	//Процесс создания линка
	bool modeCreateLink;
	GUIPoint startLinkPoint;
	GUIPoint endLinkPoint;
	GraphNodeBase * from;
	//Таблица импорта
	array<ImportTable> importTable;
	array<string> translateGroupNodes;
};

#endif

