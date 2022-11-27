//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_GotoNode
//============================================================================================


#include "AnxCmd_GotoNode.h"
#include "..\Graph\GraphNodeBase.h"

//============================================================================================

AnxCmd_GotoNode::AnxCmd_GotoNode(AnxOptions & options, const char * _gotoNode) : Command(options)
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	GraphNodeBase * n = opt.project->graph.Find(_gotoNode);
	Assert(n);
	Assert(n->parent);
	gotoNode = n->name;
	baseGoto = n->parent->base;
	fromNode = opt.project->graph.current->name;
	baseNode = opt.project->graph.current->base;
}

AnxCmd_GotoNode::~AnxCmd_GotoNode()
{

}


//============================================================================================

bool AnxCmd_GotoNode::IsEnable()
{
	return true;
}

bool AnxCmd_GotoNode::CanUndo()
{
	return true;
}

void AnxCmd_GotoNode::Do()
{
	ReDo();
}

void AnxCmd_GotoNode::UnDo()
{
	GraphNodeBase * n = opt.project->graph.Find(fromNode);
	Assert(n);
	opt.project->graph.current = n;
	opt.project->graph.current->DeselectAllChilds();
	n->base = baseNode;
}

void AnxCmd_GotoNode::ReDo()
{
	GraphNodeBase * n = opt.project->graph.Find(gotoNode);
	Assert(n);
	Assert(n->parent);
	opt.project->graph.current = n->parent;
	opt.project->graph.current->DeselectAllChilds();
	n->parent->base = GUIPoint(opt.width, opt.height)/2 - n->rect.pos - n->rect.size/2;
	opt.project->graph.current->child.Del(n);
	opt.project->graph.current->child.Add(n);
}




