

#include "MirrorPath.h"
#include "FileService.h"


MirrorPath::MirrorPath(const char * _from, const char * _on, const char * _realPathFrom, const char * _realPathOn, const char * _cppFileName, long _cppFileLine)
{
	node = null;
	from = _from;
	on = _on;
	pathFrom = _realPathFrom;
	pathOn = _realPathOn;
	refCount = 1;
#ifndef STOP_DEBUG
	cppFileName = _cppFileName;
	cppFileLine = _cppFileLine;
#endif
}

MirrorPath::~MirrorPath()
{
}

//Удалить объект, сообщив об ошибке
void MirrorPath::ErrorRelease()
{
#ifndef STOP_DEBUG
	api->Trace("FileService error: IMirrorPath not released (file: %s, line: %i)", cppFileName, cppFileLine);
#endif
	delete this;
}

//Этот ли объект
bool MirrorPath::IsThis(const string & _from, const string & _on)
{
	if(pathFrom == _from)
	{
		if(pathOn == _on)
		{
			return true;
		}
	}
	return false;
}

//Увеличить счётчик объектов
void MirrorPath::AddRefCount()
{
	refCount++;
}

//Удалить объект, закончив отражать путь
void MirrorPath::Release()
{
	{
		SingleExClassThread(FileService::object)
		refCount--;
		if(refCount > 0)
		{
			return;
		}
		FileService::object->DeleteMirrorPath(this);
	}	
	delete this;
}


//Получить путь который отражается
const char * MirrorPath::From()
{
	return from;
}

//Получить путь на который отражается
const char * MirrorPath::On()
{
	return on;
}

//Установить нод к которому привязан объект
void MirrorPath::SetNode(void * _node)
{
	node = _node;
}

//Получить нод к которому привязан объект
void * MirrorPath::GetNode()
{
	return node;
}

