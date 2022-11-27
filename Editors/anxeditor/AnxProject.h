//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxProject
//============================================================================================

#ifndef _AnxProject_h_
#define _AnxProject_h_

#include "AnxBase.h"
#include "Graph\GraphManager.h"
#include "MasterSkeleton.h"
#include "ClipsRepository.h"

class Command;


class AnxProject
{
	struct TmpSaveGraphParams
	{
		TmpSaveGraphParams() : events(_FL_)
		{
		};

		~TmpSaveGraphParams();

		string nodeName;
		array<void *> events;
		GUIPoint pos;
		bool isGlobalPos;
		bool isMovment;
		bool isLoop;
	};


//--------------------------------------------------------------------------------------------
public:
	AnxProject(AnxOptions & opt);
	virtual ~AnxProject();

//--------------------------------------------------------------------------------------------
public:
	//Отрисовка содержимого проекта
	void Draw(IRender * render, const GUIRectangle & rect, GUICliper & clipper);

	//Исполнить команду
	void Execute(Command * cmd);
	

	//Создать уникальное имя для нода
	void GenUniqueNodeName(string & uname);
	//Проверить имя нода на уникальность
	bool CheckUniqueNodeName(const string & uname);

	//Записать данные в поток
	void Write(AnxStream & stream);
	//Прочитать данные из потока
	void Read(AnxStream & stream);
	//Импортировать данные из потока
	const char * Import(AnxStream & stream);
	//Импортировать звуковые события, замещая текущие
	void ImportEvents(const char * iniFilePath, array<string> & errors);
	//Импортировать ролик
	void ImportMovie(const char * iniFilePath, array<string> & errors);
	//Перезагрузить анимационные клипы
	void ReloadAnts(const char * path, bool isRecursive, array<string> & errors);


	//Создать объект нода по типу
	GraphNodeBase * CreateNodeByType(AnxNodeTypes type) const;

	//Сбросить позицию камеры
	void ResetCameraPosition();
	//Имя пиртиклов было изменено
	void UpdateParticles();


//--------------------------------------------------------------------------------------------
public:
	GraphManager graph;
	MasterSkeleton * skeleton;

public:
	AnxOptions & options;
	array<Command *> commands;
	long maxUndo;
	long cmdPointer;
	dword numCounter;
	//Имя модельки для просмотра
	string preveiwModel;
	//Имя проекта партиклов для предпросмотра
	string preveiwParticles;
	//Позиция камеры в просмоторщике
	Vector camAng, camPos;
	//Стартовый нод
	GraphNodeBase * startNode;
	//Стоповый нод
	GraphNodeBase * stopNode;
	//Имя проекта c путём
	string path;
	//Буфер для копирования графа
	AnxStreamMemory graphCopyBuffer;
	//Буфер для копирования эвентов
	AnxStreamMemory eventCopyBuffer;
	//Глобальное хранилище для клипов
	ClipsRepository clipsRepository;
};

#endif

