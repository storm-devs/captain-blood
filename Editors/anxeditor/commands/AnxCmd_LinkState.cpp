//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_LinkState
//============================================================================================


#include "AnxCmd_LinkState.h"

//============================================================================================

AnxCmd_LinkState::AnxCmd_LinkState(GraphLink * link) : Command(link->opt)
{
	doLink = link;
	//Адресаты
	fromNode = link->from->name;
	toNode = link->to->name;
	//Сохраним текущее состояние нода
	link->Write(undoData);
	undoData.EndOfWrite();
}

AnxCmd_LinkState::~AnxCmd_LinkState()
{

}

//============================================================================================

//============================================================================================

bool AnxCmd_LinkState::IsEnable()
{
	return true;
}

bool AnxCmd_LinkState::CanUndo()
{
	return true;
}

void AnxCmd_LinkState::Do()
{
	Assert(doLink);
	doLink->Write(redoData);
	redoData.EndOfWrite();
	doLink = null;
}

void AnxCmd_LinkState::UnDo()
{
	GraphLink * l = GetLink();
	l->Read(undoData);
	undoData.ResetPosition();
}

void AnxCmd_LinkState::ReDo()
{
	GraphLink * l = GetLink();
	l->Read(redoData);
	redoData.ResetPosition();
}

GraphLink * AnxCmd_LinkState::GetLink()
{
	Assert(opt.project);
	GraphNodeBase * from = opt.project->graph.Find(fromNode);
	Assert(from);
	Assert(from->parent);
	GraphNodeBase * to = opt.project->graph.Find(toNode);
	Assert(to);	
	Assert(to->parent);
	Assert(from->parent == to->parent);
	long i = from->parent->links.FindLink(from, to);
	Assert(i >= 0);
	GraphLink * l = from->parent->links.links[i];
	Assert(l);
	return l;
}

