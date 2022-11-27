//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeGroup	
//============================================================================================

#ifndef _GraphNodeGroup_h_
#define _GraphNodeGroup_h_

#include "GraphNodeBase.h"

class GraphNodeGroup : public GraphNodeBase
{
public:
	struct VirtualLink
	{
		LinkData linkData;
		string toNodeName;
	};

//--------------------------------------------------------------------------------------------
public:
	GraphNodeGroup(AnxOptions & options);
	virtual ~GraphNodeGroup();

//--------------------------------------------------------------------------------------------
public:
	//Нарисовать нод
	virtual void Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper);
	//Проверить поподание в нод
	virtual bool Inside(const GUIPoint & pnt);
	//Активировать
	virtual void Activate();

	//Можно отводить линк
	virtual bool CanFromLink();
	//Нормальный линк подходит к ноду или фейковый
	virtual bool IsFakeLink();

	//Записать данные в поток
	virtual void Write(AnxStream & stream);
	//Прочитать данные из потока
	virtual void Read(AnxStream & stream, GraphNodeBase * importTo);
	//Ретранслировать данные при импорте
	virtual void ImportTranslate();

	//Получить тип нода
	virtual AnxNodeTypes GetType();

//--------------------------------------------------------------------------------------------
public:
	array<VirtualLink *> vlink;
	bool isLoopLinks;
};

#endif

