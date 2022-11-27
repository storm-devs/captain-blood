//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphLinks	
//============================================================================================

#ifndef _GraphLinks_h_
#define _GraphLinks_h_

#include "GraphLink.h"

class GraphLinks
{
//--------------------------------------------------------------------------------------------
public:
	GraphLinks(AnxOptions & options);
	virtual ~GraphLinks();

//--------------------------------------------------------------------------------------------
public:
	//Нарисовать линки
	void Draw(const GUIPoint & pos, IRender * render);

	//Создать линк
	long CreateLink(GraphNodeBase * from, GraphNodeBase * to);
	//Удалить линк
	void DeleteLink(GraphNodeBase * from, GraphNodeBase * to);	
	//Найти линк
	long FindLink(GraphNodeBase * from, GraphNodeBase * to);
	//Найти линк по позиции
	long FindLink(const GUIPoint & pos);
	//Выделить линк
	void SelectLink(long index, bool deselectCurrent = false, bool deselectAll = true);

	//Записать данные в поток
	void Write(AnxStream & stream);
	//Прочитать данные из потока
	void Read(AnxStream & stream, bool importMode);

	//Записать данные в поток линки идущие к селекченым нодам
	void WriteSelection(AnxStream & stream);

	//Записать данные в поток
	void WriteLink(AnxStream & stream, long index);
	//Прочитать данные из потока
	void ReadLink(AnxStream & stream, bool importMode);

	//Транслирование имени нода при импо
	void ImportTranslate(string & nodeName);

//--------------------------------------------------------------------------------------------
public:
	//Опции
	AnxOptions & opt;
	//Линки
	array<GraphLink *> links;
};

#endif

