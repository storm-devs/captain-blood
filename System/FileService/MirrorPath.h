
#ifndef _MirrorPath_h_
#define _MirrorPath_h_

#include "..\..\common_h\FileService.h"

//Зеркальный путь
class MirrorPath : public IMirrorPath
{
public:
	MirrorPath(const char * _from, const char * _on, const char * _realPathFrom, const char * _realPathOn, const char * _cppFileName, long _cppFileLine);
	virtual ~MirrorPath();

	//Удалить объект, сообщив об ошибке
	void ErrorRelease();

	//Этот ли объект
	bool IsThis(const string & _from, const string & _on);
	//Увеличить счётчик объектов
	void AddRefCount();

public:
	//Удалить объект, закончив отражать путь
	virtual void Release();
	//Получить путь который отражается
	virtual const char * From();
	//Получить путь на который отражается
	virtual const char * On();

	//Получить полный нормальизованый путь
	const char * GetPath();

	//Установить нод к которому привязан объект
	void SetNode(void * _node);
	//Получить нод к которому привязан объект
	void * GetNode();

private:
	string from;
	string on;
	string pathFrom;
	string pathOn;
	void * node;
	long refCount;
#ifndef STOP_DEBUG
	const char * cppFileName;
	long cppFileLine;
#endif
};


//Получить полный нормальизованый путь
__forceinline const char * MirrorPath::GetPath()
{
	return pathFrom;
}

#endif

