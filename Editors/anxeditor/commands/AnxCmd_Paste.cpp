//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_Paste
//============================================================================================


#include "AnxCmd_Paste.h"
#include "..\Graph\GraphNodeBase.h"


//============================================================================================

AnxCmd_Paste::AnxCmd_Paste(AnxOptions & options) : Command(options), nodes(_FL_)
{
}

AnxCmd_Paste::~AnxCmd_Paste()
{
}

//============================================================================================

bool AnxCmd_Paste::IsEnable()
{
	if(!opt.project) return false;
	if(opt.project->graphCopyBuffer.GetDataSize() > 0) return true;
	return false;
}

bool AnxCmd_Paste::CanUndo()
{
	return true;
}

void AnxCmd_Paste::Do()
{
	Assert(opt.project);
	Assert(opt.project->graphCopyBuffer.GetDataSize() > 0);
	Assert(opt.project->graph.current);
	//Импортируем ноды из буфера
	opt.project->graphCopyBuffer.ResetPosition();
	opt.project->graph.Import(opt.project->graphCopyBuffer, opt.project->graph.current);
	//Сохраняем данные в буфере для операций анду-реду
	SaveSelectedNodes();
}

void AnxCmd_Paste::UnDo()
{
	//Удаляем ноды
	Assert(opt.project);
	GraphNodeBase * cur = opt.project->graph.current;
	Assert(cur);
	for(long i = 0; i < nodes; i++)
	{
		GraphNodeBase * node = cur->Find(nodes[i]);
		if(node) delete node;
	}
}

void AnxCmd_Paste::ReDo()
{
	Assert(opt.project);
	GraphNodeBase * cur = opt.project->graph.current;
	Assert(cur);
	data.ResetPosition();
	opt.project->graph.Import(data, opt.project->graph.current);
}

Command * AnxCmd_Paste::CreateThisObject(AnxOptions & opt)
{  
	return NEW AnxCmd_Paste(opt);
}

void AnxCmd_Paste::SaveSelectedNodes()
{
	Assert(opt.project);
	GraphNodeBase * cur = opt.project->graph.current;
	Assert(cur);
	//Сохраняем имена всех выделенных нодов	
	for(long i = 0; i < cur->child; i++)
	{
		if(cur->child[i]->select)
		{
			nodes.Add(cur->child[i]->name);
		}
	}
	//Сохраняем все выделенные ноды
	data.Reset();
	opt.project->graph.Export(data);
	data.EndOfWrite();
}


