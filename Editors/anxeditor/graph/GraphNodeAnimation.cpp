//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeAnimation	
//============================================================================================
			

#include "GraphNodeAnimation.h"
#include "..\Forms\NodeAnimationForm.h"

//============================================================================================

GraphNodeAnimation::GraphNodeAnimation(AnxOptions & options) : GraphNodeBase(options),
																clips(_FL_),
																consts(_FL_)
{
	isLoop = true;
	isChange = true;
	isMovement = false;
	isGlobalPos = true;
	if(!opt.project->startNode) opt.project->startNode = this;
}

GraphNodeAnimation::~GraphNodeAnimation()
{
	if(opt.project->startNode == this) opt.project->startNode = null;
	if(opt.project->stopNode == this) opt.project->stopNode = null;
	for(long i = 0; i < clips; i++) delete clips[i];
	clips.Empty();
	for(i = 0; i < consts; i++) delete consts[i];
	consts.Empty();
}

//============================================================================================

//Нарисовать нод
void GraphNodeAnimation::Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper)
{
	GraphNodeBase::Draw(pos, render, clipper);
	bool isShowInfo = opt.IsShowInfo();
	if((opt.project->startNode == this || opt.project->stopNode == this) && !isShowInfo)
	{
		static Matrix identity;
		Vector point[10];
		point[0].x = pos.x + rect.pos.x + 8.0f;
		point[0].y = pos.y + rect.pos.y + 8.0f;
		point[0].z = 0.0f;
		for(long i = 0; i <= 8; i++)
		{
			point[i + 1].x = point[0].x + 3.0f*sinf(i*2.0f*PI/8.0f);
			point[i + 1].y = point[0].y + 3.0f*cosf(i*2.0f*PI/8.0f);
			point[i + 1].z = 0.0f;
		}
		float fScrX = float(render->GetScreenInfo3D().dwWidth)*0.5f;
		float fScrY = float(render->GetScreenInfo3D().dwHeight)*0.5f;
		for(i = 0; i < 10; i++)
		{
			point[i].x = point[i].x/fScrX - 1.0f;
			point[i].y = 1.0f - point[i].y/fScrY;
		}
		long color = opt.project->startNode == this ? 0xff00ffff : 0xffff0000;
		render->DrawPolygon(point, 10, Color(color), identity, "GUIBase");
		for(i = 1; i < 9; i++)
		{
			render->DrawLine(point[i], 0xffffffff, point[i + 1], 0xffffffff, false, "GUIBase");
		}
	}
	if(isShowInfo)
	{
		clipper.SetFullScreenRect();
		for(long i = 0; i < clips; i++)
		{
			char buffer[256];
			crt_snprintf(buffer, sizeof(buffer), "%i: %u-%u", i + 1, clips[i]->data.clipStartFrame, clips[i]->data.clipStartFrame + clips[i]->data.clipNumFrames - 1);
			int width = opt.node.fontLabel->GetWidth(buffer);
			int height = opt.node.fontLabel->GetHeight();
			long x = pos.x + rect.pos.x- height*2;
			long y = pos.y + rect.pos.y - height/2 + (height + 2)*i;
			GUIHelper::Draw2DRect(x - 1, y - 1, width + 2, height + 2, 0xc0000000);
			opt.node.fontLabel->Print(x, y, 0xffff6060, buffer);
		}
	}else{
		float fps = 0.0f;
		for(long i = 0; i < clips; i++)
		{
			if(fps < clips[i]->data.fps)
			{
				fps = clips[i]->data.fps;
			}
		}
		if(fps > 15.0f)
		{
			clipper.SetFullScreenRect();
			const char * fpsMessage = "Hi fps: %3.1f";
			if(fps >= 20)
			{
				if(fps <= 30)
				{
					fpsMessage = "Ultra-hi fps: %3.1f";
				}else{
					fpsMessage = "Impossible fps: %3.1f !";
				}				
			}
			char buffer[256];
			crt_snprintf(buffer, sizeof(buffer), fpsMessage, fps);
			int width = opt.node.fontLabel->GetWidth(buffer);
			int height = opt.node.fontLabel->GetHeight();
			long x = pos.x + rect.pos.x + rect.size.x - width*3/4;
			long y = pos.y + rect.pos.y - height/2;
			GUIHelper::Draw2DRect(x - 1, y - 1, width + 2, height + 2, 0xff000000);
			opt.node.fontLabel->Print(x, y, 0xffff0000, buffer);
		}
	}
}

//Активировать
void GraphNodeAnimation::Activate()
{
	Assert(opt.gui_manager);
 	NodeAnimationForm * form = NEW NodeAnimationForm(this);
	opt.gui_manager->ShowModal(form);
}

//Записать данные в поток
void GraphNodeAnimation::Write(AnxStream & stream)
{
	GraphNodeBase::Write(stream);
	stream.WriteVersion(5);
	//Сохраняем клипы
	stream << clips;
	for(long i = 0; i < clips; i++)
	{
		clips[i]->data.Write(stream);
		stream << clips[i]->events;
		for(long j = 0; j < clips[i]->events; j++)
		{
			clips[i]->events[j]->Write(stream);
			/*
			stream << clips[i]->events[j]->info;
			stream << clips[i]->events[j]->name;
			stream << clips[i]->events[j]->frame;
			stream << clips[i]->events[j]->params;
			for(long k = 0; k < clips[i]->events[j]->params; k++)
			{
				stream << *clips[i]->events[j]->params[k];
			}
			stream << clips[i]->events[j]->noBuffered;
			*/
		}
	}
	//Константы
	stream << consts;
	for(i = 0; i < consts; i++)
	{
		consts[i]->Write(stream);
	}
	//Флажки
	bool isRotate = false;
	stream << isLoop << isChange << isMovement << isRotate;
	stream << isGlobalPos;
}

//Прочитать данные из потока
void GraphNodeAnimation::Read(AnxStream & stream, GraphNodeBase * importTo)
{
	GraphNodeBase::Read(stream, importTo);
	if(importTo == this) return;
	for(long i = 0; i < clips; i++) delete clips[i];
	clips.Empty();
	for(i = 0; i < consts; i++) delete consts[i];
	consts.Empty();
	dword version = stream.ReadVersion();
	if(version < 3 || version > 5) throw "Invalidate version of animation node data";
	Clip * clip = null;
	Event * evt = null;
	ConstBlock * cnt = null;
	string * str = null;
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		long count = 0;
		stream >> count;
		for(i = 0; i < count; i++)
		{
			clip = NEW Clip(opt);
			clip->data.Read(stream);
			long numevents = 0;
			stream >> numevents;
			clip->events.Reserve(numevents);
			for(long j = 0; j < numevents; j++)
			{
				evt = NEW Event();
				evt->Read(stream, version);

				/*
				stream >> event->info;
				stream >> event->name;
				stream >> event->frame;
				long numParams = 0;
				stream >> numParams;
				for(long k = 0; k < numParams; k++)
				{
					str = NEW string();
					stream >> *str;
					event->params.Add(str);
					str = null;
				}
				if(version > 4)
				{
					stream >> event->noBuffered;
				}else{
					event->noBuffered = false;
				}
				*/

				for(long i = 0; i < opt.eventForms; i++)
				{
					if(opt.eventForms[i]->name == evt->name)
					{
						evt->noBuffered = opt.eventForms[i]->noBuffered;
						break;
					}
				}
				clip->events.Add(evt);
				evt = null;
			}
			clips.Add(clip);
			clip = null;
		}
		count = 0;
		stream >> count;
		consts.Reserve(count);
		for(i = 0; i < count; i++)
		{
			cnt = NEW ConstBlock();
			cnt->Read(stream);
			consts.Add(cnt);
			cnt = null;
		}
		bool isRotate = false;
		stream >> isLoop >> isChange >> isMovement >> isRotate;
		if(version > 3)
		{
			stream >> isGlobalPos;
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		if(clip) delete clip;
		clip = null;
		if(evt) delete evt;
		evt = null;
		if(str) delete str;
		str = null;
		if(cnt) delete cnt;
		cnt = null;
		throw err;
	}catch(...){
		if(clip) delete clip;
		clip = null;
		if(evt) delete evt;
		evt = null;
		if(str) delete str;
		str = null;
		if(cnt) delete cnt;
		cnt = null;
		throw "Unknown error: GraphNodeAnimation::Read";
	}
#endif
	Assert(!clip);
	Assert(!evt);
	Assert(!str);
	if(importTo)
	{
		for(long i = 0; i < clips; i++)
		{
			clips[i]->data.ApplyMasterSkeleton();
		}
	}
}

//Собрать список антов с выделеных нодов, которые надо сохранить
void GraphNodeAnimation::CollectAntsFromNode(array<long> & tbl)
{
	for(dword i = 0; i < clips.Size(); i++)
	{
		long index = clips[i]->data.pantIndex;
		if(index >= 0)
		{
			for(dword j = 0; j < tbl.Size(); j++)
			{
				if(tbl[j] == index)
				{
					break;
				}
			}
			if(j >= tbl.Size())
			{
				tbl.Add(index);
			}
		}
	}
}

//Получить тип нода
AnxNodeTypes GraphNodeAnimation::GetType()
{
	return anxnt_anode;
}

//Удалить событие по имени
void GraphNodeAnimation::DeleteEventsByName(const char * name)
{
	for(dword i = 0; i < clips.Size(); i++)
	{
		clips[i]->DeleteEventsByName(name);
	}
}

//============================================================================================

GraphNodeAnimation::Event::Event() : params(_FL_)
{
	frame = 0;
	noBuffered = false;
}

GraphNodeAnimation::Event::~Event()
{
	for(long i = 0; i < params; i++) delete params[i];
	params.Empty();
}

//Записать данные в поток
void GraphNodeAnimation::Event::Write(AnxStream & stream)
{
	stream << info;
	stream << name;
	stream << frame;
	stream << params;
	for(long k = 0; k < params; k++)
	{
		stream << *params[k];
	}
	stream << noBuffered;

}

//Прочитать данные из потока
void GraphNodeAnimation::Event::Read(AnxStream & stream, dword version)
{
	stream >> info;
	stream >> name;
	stream >> frame;
	long numParams = 0;
	stream >> numParams;
	params.AddElements(numParams);
	for(long k = 0; k < numParams; k++)
	{
		params[k] = NEW string();
		stream >> *params[k];
	}
	if(version > 4)
	{
		stream >> noBuffered;
	}else{
		noBuffered = false;
	}
}

//============================================================================================

GraphNodeAnimation::Clip::Clip(AnxOptions & options) : data(options),
														events(_FL_)
{
}

GraphNodeAnimation::Clip::~Clip()
{
	for(long i = 0; i < events; i++) delete events[i];
}

//Удалить событие по имени
void GraphNodeAnimation::Clip::DeleteEventsByName(const char * name)
{
	for(dword i = 0; i < events.Size(); )
	{
		if(events[i]->name == name)
		{
			delete events[i];
			events.DelIndex(i);
		}else{
			i++;
		}
	}
}

void GraphNodeAnimation::Clip::ExtractInfo(Clip & source, bool isMovment)
{
	for(long i = 0; i < source.events; i++)
	{
		events.Add(source.events[i]);
	}
	source.events.Empty();
	//Параметры клипа	
	data.clipStartFrame = source.data.clipStartFrame;
	data.clipNumFrames = source.data.clipNumFrames;
	data.fps = source.data.fps;
	data.probability = source.data.probability;	
	dword numFrames = data.GetAntFrames(isMovment);
	if(data.clipStartFrame >= numFrames)
	{
		data.clipStartFrame = numFrames - 1;
	}
	dword afterLastFrame = data.clipStartFrame + data.clipNumFrames;
	if(afterLastFrame > numFrames)
	{
		data.clipNumFrames = numFrames - data.clipStartFrame;
	}
}

//============================================================================================


GraphNodeAnimation::Const::Const()
{
	name.Empty();
	str.Empty();
	flt = 0.0f;
	bld = 0.0f;
	type = t_empty;
}

GraphNodeAnimation::ConstBlock::ConstBlock() : consts(_FL_)
{
}

GraphNodeAnimation::ConstBlock::~ConstBlock()
{
	for(long i = 0; i < consts; i++)
	{
		delete consts[i];
	}
	consts.Empty();
}

//Записать данные в поток
void GraphNodeAnimation::ConstBlock::Write(AnxStream & stream)
{
	stream.EnterWriteSection("Consts block -> ");
	stream.WriteVersion(1);
	stream << name;
	stream << type;
	stream << long(consts);
	for(long i = 0; i < consts; i++)
	{
		stream << consts[i]->name;
		stream << consts[i]->str;
		stream << consts[i]->flt;
		stream << consts[i]->bld;
		stream << byte(consts[i]->type);
	}
	stream.ExitWriteSection();
}

//Прочитать данные из потока
void GraphNodeAnimation::ConstBlock::Read(AnxStream & stream)
{
	string sid;
	stream.EnterReadSection(sid);
	Const * csnt = null;
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Consts block -> ") throw "Invalidate consts block section id";
		if(stream.ReadVersion() != 1) throw "Invalidate consts block section version";
		stream >> name;
		stream >> type;
		long count = 0;
		stream >> count;
		for(long i = 0; i < count; i++)
		{
			csnt = NEW Const();
			stream >> csnt->name;
			stream >> csnt->str;
			stream >> csnt->flt;
			stream >> csnt->bld;
			byte t;
			stream >> t;
			if(t >= Const::t_max) throw "Invalidate consts block section data";
			csnt->type = (Const::Type)t;
			consts.Add(csnt);
			csnt = null;
		}
	#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor links section: IORead link error: %s", err ? err : "<no info>");
	}catch(...){
		api->Trace("AnxEditor links section: IORead link unknow error.");
	}
#endif
	if(csnt) delete csnt;
	csnt = null;
	stream.ExitReadSection();
}
