//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeInOut
//============================================================================================

#ifndef _GraphNodeInOut_h_
#define _GraphNodeInOut_h_

#include "GraphNodeBase.h"

class GraphNodeInOut : public GraphNodeBase
{
//--------------------------------------------------------------------------------------------
public:
	GraphNodeInOut(AnxOptions & options) : GraphNodeBase(options){};
	virtual ~GraphNodeInOut(){};

	//Записать данные в поток
	virtual void Write(AnxStream & stream)
	{
		GraphNodeBase::Write(stream);
		stream.WriteVersion(1);
		stream << link;
	}

	//Прочитать данные из потока
	virtual void Read(AnxStream & stream, GraphNodeBase * importTo)
	{
		GraphNodeBase::Read(stream, importTo);
		if(importTo != this)
		{		
			if(stream.ReadVersion() != 1) throw "Invalidate version GraphNodeInOut data";
			stream >> link;
		}
	}

	//Нормальный линк подходит к ноду или фейковый
	virtual bool IsFakeLink()
	{
		return false;
	}

//--------------------------------------------------------------------------------------------
public:
	string link;
};

#endif

