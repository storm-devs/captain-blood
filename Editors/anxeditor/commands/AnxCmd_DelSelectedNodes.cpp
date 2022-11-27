//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_DelSelectedNodes
//============================================================================================


#include "AnxCmd_DelSelectedNodes.h"
#include "..\Graph\GraphNodeBase.h"

//============================================================================================

AnxCmd_DelSelectedNodes::AnxCmd_DelSelectedNodes(AnxOptions & options) : Command(options)
{

}

AnxCmd_DelSelectedNodes::~AnxCmd_DelSelectedNodes()
{

}


//============================================================================================

bool AnxCmd_DelSelectedNodes::IsEnable()
{
	if(!opt.project) return false;
	Assert(opt.project->graph.current);
	GraphNodeBase * n = opt.project->graph.current;
	for(long i = 0; i < n->child; i++)
	{
		if(n->child[i]->select) return true;
	}
	for(i = 0; i < n->links.links; i++)
	{
		if(n->links.links[i]->select) return true;
	}
	return false;
}

bool AnxCmd_DelSelectedNodes::CanUndo()
{
	return true;
}

void AnxCmd_DelSelectedNodes::Do()
{
	ReDo();
}

void AnxCmd_DelSelectedNodes::UnDo()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	GraphNodeBase * n = opt.project->graph.current;
	dword tag = 0;
	do
	{
		data >> tag;
		if(tag == 'Node')
		{
			n->ReadChild(data, false);
		}else
		if(tag == 'Link')
		{
			n->links.ReadLink(data, false);
		}
	}while(tag != 'EOSM');
	data.Reset();
}

void AnxCmd_DelSelectedNodes::ReDo()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	GraphNodeBase * n = opt.project->graph.current;
	data.Reset();
	//Сохраняем ноды, которые удалим
	for(long i = 0; i < n->child; i++)
	{
		if(n->child[i]->select)
		{
			//Сохраняем данные нода
			data << dword('Node');
			n->WriteChild(data, i);
		}
	}
	//Сохраним линки, которые будут удалены
	for(i = 0; i < n->links.links; i++)
	{
		if(n->links.links[i]->from->select ||
			n->links.links[i]->to->select ||
			n->links.links[i]->select)
		{
			//Сохраняем данные линка
			data << dword('Link');
			n->links.WriteLink(data, i);
		}
	}
	data << dword('EOSM');
	data.EndOfWrite();
	//Удаляем ноды
	for(i = 0; i < n->child; i++)
	{
		if(n->child[i]->select)
		{
			//Удаляем нод
			delete n->child[i];
			i = -1;
		}
	}
	//Выделенные линки
	for(i = 0; i < n->links.links; i++)
	{
		if(n->links.links[i]->select)
		{
			//Удаляем линк
			delete n->links.links[i];
			n->links.links.DelIndex(i);
			i = -1;
		}
	}
}

Command * AnxCmd_DelSelectedNodes::CreateThisObject(AnxOptions & opt)
{
	return NEW AnxCmd_DelSelectedNodes(opt);
}

