//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_CreateLink
//============================================================================================


#include "AnxCmd_CreateLink.h"
#include "..\Graph\GraphNodeBase.h"

//============================================================================================

AnxCmd_CreateLink::AnxCmd_CreateLink(AnxOptions & options, GraphNodeBase * from, GraphNodeBase * to) : Command(options)
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	fromName = from->name;
	toName = to->name;
	currentName = opt.project->graph.current->name;
}

AnxCmd_CreateLink::~AnxCmd_CreateLink()
{

}


//============================================================================================


bool AnxCmd_CreateLink::IsEnable()
{
	return true;
}

bool AnxCmd_CreateLink::CanUndo()
{
	return true;
}

void AnxCmd_CreateLink::Do()
{
	ReDo();
}

void AnxCmd_CreateLink::UnDo()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	Assert(opt.project->graph.current->name == currentName);
	GraphNodeBase * from = opt.project->graph.Find(fromName);
	GraphNodeBase * to = opt.project->graph.Find(toName);
	Assert(from);
	Assert(to);
	Assert(from->parent == to->parent);
	opt.project->graph.current->links.DeleteLink(from, to);
}

void AnxCmd_CreateLink::ReDo()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	Assert(opt.project->graph.current->name == currentName);
	GraphNodeBase * from = opt.project->graph.Find(fromName);
	GraphNodeBase * to = opt.project->graph.Find(toName);
	Assert(from);
	Assert(to);
	Assert(from->parent == to->parent);
	opt.project->graph.current->links.CreateLink(from, to);
}

