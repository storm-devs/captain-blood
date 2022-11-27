//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphLink	
//============================================================================================
			

#include "GraphLink.h"
#include "GraphNodeBase.h"
#include "..\Forms\LinkForm.h"

//============================================================================================

GraphLink::GraphLink(AnxOptions & options, GraphNodeBase * _from, GraphNodeBase * _to) : opt(options)
{
	Assert(_from);
	Assert(_to);
	Assert(_from->parent == _to->parent);
	//Сохраняем ноды
	from = _from;
	to = _to;
	//Регистрируемся в нодах
	from->conectedLinks.Add(this);
	to->conectedLinks.Add(this);
	//Параметры
	fake = _from->IsFakeLink() || _to->IsFakeLink();
	select = false;
}

GraphLink::~GraphLink()
{
	from->conectedLinks.Del(this);
	to->conectedLinks.Del(this);
}


//============================================================================================

//Активировать линк
void GraphLink::Activate()
{
	Assert(opt.gui_manager);
	LinkFormPanel * form = NEW LinkFormPanel(this, fake);
	opt.gui_manager->ShowModal(form);
}

//Записать данные в поток
void GraphLink::Write(AnxStream & stream)
{
	stream.WriteVersion(2);
	stream << fake << select;
	stream << fromONode << toINode;
	data.Write(stream);
}

//Прочитать данные из потока
void GraphLink::Read(AnxStream & stream)
{
	if(stream.ReadVersion() != 2) throw "Invalidate version of link data";
	stream >> fake >> select;
	stream >> fromONode >> toINode;
	data.Read(stream);
}




