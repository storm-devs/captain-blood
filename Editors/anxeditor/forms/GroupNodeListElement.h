//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GroupNodeListElement	
//============================================================================================

#ifndef _GroupNodeListElement_h_
#define _GroupNodeListElement_h_

#include "..\AnxBase.h"
#include "..\Graph\LinkData.h"

class NodeGroupForm;

class GroupNodeListElement : public GUIControl
{
//--------------------------------------------------------------------------------------------
public:
	GroupNodeListElement(GUIControl * p, AnxOptions & options, NodeGroupForm * _nf);
	virtual ~GroupNodeListElement();

//--------------------------------------------------------------------------------------------
public:
	//Рисование
	virtual void Draw();
	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	//Сообщение об удалении
	void _cdecl EvtDel(GUIControl * sender);

//--------------------------------------------------------------------------------------------
public:
	AnxOptions & opt;
	LinkData data;
	string nodeName;
	bool mouseDown;
	bool deleteMe;
	NodeGroupForm * nf;
};

#endif

