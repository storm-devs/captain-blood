//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_CreateNode
//============================================================================================


#include "AnxCmd_CreateNode.h"
#include "..\Graph\GraphNodeBase.h"
#include "..\Graph\GraphNodeAnimation.h"
#include "..\Graph\GraphNodeHierarchy.h"
#include "..\Graph\GraphNodeGroup.h"
#include "..\Graph\GraphNodeIn.h"
#include "..\Graph\GraphNodeOut.h"


//============================================================================================

AnxCmd_CreateNode::AnxCmd_CreateNode(AnxNodeTypes t, AnxOptions & options) : Command(options)
{
	type = t;
}

AnxCmd_CreateNode::~AnxCmd_CreateNode()
{
}

//============================================================================================

bool AnxCmd_CreateNode::IsEnable()
{
	if(!opt.project) return false;
	switch(type)
	{
	case anxnt_anode:
		return true;
	case anxnt_hnode:
		return true;
	case anxnt_gnode:
		return true;
	case anxnt_inode:
	case anxnt_onode:
		Assert(opt.project->graph.current);
		return opt.project->graph.current->parent != null;
	};
	return false;
}

bool AnxCmd_CreateNode::CanUndo()
{
	return true;
}

void AnxCmd_CreateNode::Do()
{
	Assert(opt.project);
	opt.project->GenUniqueNodeName(nodeName);
	ReDo();
}

void AnxCmd_CreateNode::UnDo()
{
	Assert(opt.project);
	GraphNodeBase * n = opt.project->graph.Find(nodeName);
	if(n) delete n;
}

void AnxCmd_CreateNode::ReDo()
{
	Assert(opt.project);
	//Создаём нод
	GraphNodeBase * n = CreateNodeByType(type, opt);
	Assert(n);
	//Настраиваем общие свойства
	n->name = nodeName;
	n->rect.pos = GUIPoint(opt.width/2, opt.height/2) - opt.project->graph.current->base;
	n->rect.pos -= n->rect.size/2;
	long dpos = ((opt.project->numCounter & 15) - 7)*16;
	long npos = ((opt.project->numCounter >> 4) & 3) - 1;
	n->rect.pos += GUIPoint(dpos + npos*40, dpos - npos*38);
	n->parent = opt.project->graph.current;
	opt.project->graph.current->child.Add(n);
}

Command * AnxCmd_CreateNode::CreateThisObject(AnxOptions & opt)
{
	return NEW AnxCmd_CreateNode(type, opt);
}

//Создать объект нода по типу
GraphNodeBase * AnxCmd_CreateNode::CreateNodeByType(AnxNodeTypes type, AnxOptions & options)
{
	switch(type)
	{
	case anxnt_anode:
		return NEW GraphNodeAnimation(options);
	case anxnt_hnode:
		return NEW GraphNodeHierarchy(options);
	case anxnt_gnode:
		return NEW GraphNodeGroup(options);
	case anxnt_inode:
		return NEW GraphNodeIn(options);
	case anxnt_onode:
		return NEW GraphNodeOut(options);
	}
	return null;
}
