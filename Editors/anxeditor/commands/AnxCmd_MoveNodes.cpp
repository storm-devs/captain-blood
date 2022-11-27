//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_MoveNodes
//============================================================================================


#include "AnxCmd_MoveNodes.h"

//============================================================================================

AnxCmd_MoveNodes::AnxCmd_MoveNodes(AnxOptions & options, GUIPoint _delta) : Command(options)
{
	delta = _delta;
	Assert(opt.project);
	Assert(opt.project->graph.current);
	GraphNodeBase * n = opt.project->graph.current;
	for(long i = 0; i < n->child; i++)
	{
		if(n->child[i]->select)
		{
			nodes.Add(n->child[i]->name);
		}
	}
}

AnxCmd_MoveNodes::~AnxCmd_MoveNodes()
{

}


//============================================================================================


bool AnxCmd_MoveNodes::IsEnable()
{
	return true;
}

bool AnxCmd_MoveNodes::CanUndo()
{
	return nodes.Size() > 0;
}

void AnxCmd_MoveNodes::Do()
{
}

void AnxCmd_MoveNodes::UnDo()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	for(long i = 0; i < nodes.Size(); i++)
	{
		GraphNodeBase * n = opt.project->graph.Find(nodes[i]);
		if(n) n->rect.pos -= delta;
	}
}

void AnxCmd_MoveNodes::ReDo()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	for(long i = 0; i < nodes.Size(); i++)
	{
		GraphNodeBase * n = opt.project->graph.Find(nodes[i]);
		if(n) n->rect.pos += delta;
	}
}






