//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_ChangeHrcNode
//============================================================================================


#include "AnxCmd_ChangeHrcNode.h"

//============================================================================================

AnxCmd_ChangeHrcNode::AnxCmd_ChangeHrcNode(const char * goto_node, AnxOptions & options) : Command(options)
{
	Assert(opt.project->graph.current);
	fromNode = opt.project->graph.current->name;
	toNode = goto_node;
}

AnxCmd_ChangeHrcNode::~AnxCmd_ChangeHrcNode()
{

}


//============================================================================================

bool AnxCmd_ChangeHrcNode::IsEnable()
{
	return true;
}

bool AnxCmd_ChangeHrcNode::CanUndo()
{
	return true;
}

void AnxCmd_ChangeHrcNode::Do()
{
	ReDo();
}

void AnxCmd_ChangeHrcNode::UnDo()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	GraphNodeBase * from = opt.project->graph.Find(fromNode);
	GraphNodeBase * to = opt.project->graph.Find(toNode);
	Assert(from);
	Assert(to);
	opt.project->graph.current->DeselectAllChilds();
	opt.project->graph.current = from;
}

void AnxCmd_ChangeHrcNode::ReDo()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	GraphNodeBase * from = opt.project->graph.Find(fromNode);
	GraphNodeBase * to = opt.project->graph.Find(toNode);
	Assert(from);
	Assert(to);
	opt.project->graph.current->DeselectAllChilds();
	opt.project->graph.current = to;
}



