//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_NodeState
//============================================================================================


#include "AnxCmd_NodeState.h"
#include "..\graph\GraphNodeAnimation.h"

//============================================================================================

AnxCmd_NodeState::AnxCmd_NodeState(GraphNodeBase * node) : Command(node->opt)
{
	doNode = node;
	//Имя нода
	undoNodeName = redoNodeName = node->name;
	//Сохраним текущее состояние нода
	WriteDataToStream(node, undoData);
}

AnxCmd_NodeState::~AnxCmd_NodeState()
{

}

//============================================================================================

//============================================================================================

bool AnxCmd_NodeState::IsEnable()
{
	return true;
}

bool AnxCmd_NodeState::CanUndo()
{
	return true;
}

void AnxCmd_NodeState::Do()
{
	Assert(doNode);
	undoNodeName = doNode->name;
	WriteDataToStream(doNode, redoData);
	doNode = null;
}

void AnxCmd_NodeState::UnDo()
{
	Assert(opt.project);
	GraphNodeBase * n = opt.project->graph.Find(undoNodeName);
	Assert(n);
	Assert(!n->child);
	ReadDataFromStream(n, undoData);
}

void AnxCmd_NodeState::ReDo()
{
	Assert(opt.project);
	GraphNodeBase * n = opt.project->graph.Find(redoNodeName);
	Assert(n);
	Assert(!n->child);
	ReadDataFromStream(n, redoData);
}

void AnxCmd_NodeState::WriteDataToStream(GraphNodeBase * node, AnxStreamMemory & stream)
{
	//Сохраним текущее состояние нода и прилегающих линков
	stream.Reset();
	//Сохраним клипы нода
	if(node->GetType() == anxnt_anode)
	{
		GraphNodeAnimation * anode = (GraphNodeAnimation *)node;
		array<long> antsInNode(_FL_, 256);
		anode->CollectAntsFromNode(antsInNode);
		long size = antsInNode.Size();
		stream << size;
		for(dword i = 0; i < antsInNode.Size(); i++)
		{
			if(antsInNode[i] >= 0)
			{
				opt.project->clipsRepository.ExportAnt(antsInNode[i], stream);
			}
		}
	}else{
		long size = 0;
		stream << size;
	}
	//Сохраним данные нода
	node->Write(stream);
	/*
	stream << doNode->conectedLinks;
	for(long i = 0; i < node->conectedLinks; i++)
	{
		node->conectedLinks[i]->Write(stream);
	}	*/
	stream.EndOfWrite();
}

void AnxCmd_NodeState::ReadDataFromStream(GraphNodeBase * node, AnxStreamMemory & stream)
{
	stream.ResetPosition();
	opt.project->clipsRepository.BeginTranslates();
	//Востановим клипы нода
	long count = 0;
	stream >> count;
	for(long i = 0; i < count; i++)
	{
		opt.project->clipsRepository.ImportAnt(stream);
	}
	//Востановим данные нода
	node->Read(stream, null);
	/*
	long numLinks = 0;
	stream >> numLinks;
	Assert(node->conectedLinks == numLinks);
	for(long i = 0; i < node->conectedLinks; i++)
	{
		node->conectedLinks[i]->Read(stream);
	}	*/
	opt.project->clipsRepository.EndTranslates();
	stream.ResetPosition();
}

