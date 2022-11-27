//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Anx editor
//============================================================================================
// AnxExporter
//============================================================================================

#ifndef _AnxExporter_h_
#define _AnxExporter_h_

#include "AnxBase.h"
#include "Graph\GraphNodeAnimation.h"
#include "Graph\GraphNodeGroup.h"
#include "Graph\GraphNodeHierarchy.h"
#include "Graph\GraphNodeInOut.h"


class GraphNodeAnimation;
class LinkData;
class GraphNodeBase;

class AnxExporter
{
	enum ExportConsts
	{
		ec_max_bones_hashtable = 4096,
		ec_max_names_hashtable = 8192,
	};


	struct NodeInfo
	{
		GraphNodeAnimation * node;
		dword nameIndex;
		dword clipsIndex;
		dword clipsCount;
		dword linksIndex; 
		dword linksCount;
		long defLink;
		dword constsIndex; 
		dword constsCount;
			
	};

	struct ClipInfo
	{
		AntFile * clip;
		dword frames;
		float p;
		dword eventsIndex;
		dword eventsCount;
		bool isMovement;
		bool isGlobalPos;
	};

	struct LinkInfo
	{
		long fromNode;
		long toNode;
		const char * toNodeName;
		long nameIndex;
		const LinkData * data;
		long arange[2];
		long mrange[2];
	};

	struct ConstInfo
	{
		dword nameIndex;
		AnxConst::Type type;
		long strIndex;
		float fvalue;
	};

	struct EventInfo
	{
		long nameIndex;
		dword frame;
		dword paramsIndex;
		dword paramsCount;
		bool isNoBuffered;
	};

	struct StringInfo
	{
		dword hash;
		dword len;
		long index;
		long next;
		dword flags;
	};

	struct BoneInfo
	{
		long parent;
		dword nameIndex;
		dword hash;
		dword nameShortIndex;
		dword hashShort;
	};

	struct HashCalculator
	{
		struct Desc
		{
			dword * table;
			dword mask;
			dword maxChain;
			float weight;
		};


		HashCalculator(dword maxSize);		
		void AddHash(dword hash);
		dword GetOptimalMask(dword totalSize);

	private:
		float WeightFunc(float bestForZero, float oneValue, float curValue);

	private:
		array<Desc> descs;
		array<dword> buffer;		
		dword count;
	};



//--------------------------------------------------------------------------------------------
public:
	AnxExporter(AnxOptions & options);
	~AnxExporter();

//--------------------------------------------------------------------------------------------
public:
	//Выгрузить анимацию
	bool Export();
	//Выгрузить анимацию для предпросмотра клипа
	bool PreviewExport(GraphNodeAnimation::Clip & clip, bool applyPosition, bool globalPos, bool isLoop);
	//Получить указатель на буфер данных
	const byte * GetData();
	//Получить размер данных
	dword GetSize();


//--------------------------------------------------------------------------------------------
//Сбор информации с текущего проекта
//--------------------------------------------------------------------------------------------
private:
	//Очистить все буфера
	void Reset();
	//Рекурсивная выгрузка
	void ExportTreeProcess(GraphNodeBase * node);
	//Добавить строку в таблицу строк
	dword AddString(const char * str, AnxFndName::Flags flag);
	//Добавить запись в таблицу строк
	void AddStringRecord(long index, AnxFndName::Flags flag);
	//Найти строку среди добавленных
	long FindString(const char * str);
	//Сохраняем мастер-скелет
	void AddMasterSkeleton();
	//Добавить к ноду клип
	float AddClip(GraphNodeAnimation * gnode, long index);
	//Добавить к клипу событие
	void AddEvent(GraphNodeAnimation::Event & evt, dword frame);
	//Добавить к ноду константу
	void AddConst(GraphNodeAnimation::ConstBlock & cst);
	//Добавить к ноду линк
	void AddLinkToNode(long from, GraphLink * link, LinkData * data);
	//Спуститься по иерархии
	void AddLinkToGroupNode(long from, GraphNodeGroup * group);
	//Спуститься по иерархии
	void AddLinkToNodeHierarchyDown(long from, GraphLink * link);
	//Подняться по иерархии
	void AddLinkToNodeHierarchyUp(long from, GraphLink * link, LinkData * data);
	//Сохранить данные найденного клипа в буфере, для последующего добавления
	void AddLinkDataToBuffer(long from, const LinkData & link, const char * toNode, bool isEnableLoopLink);
	//Генерим реальный список линков
	void PostProcessLinksList();
	//Проверить диапазон линка
	void CorrectLinkRange(NodeInfo & ni, LinkInfo & li, long & from, long & to);
	//Добавить имена костей в таблицу имён
	void AddBones();

//--------------------------------------------------------------------------------------------
//Функции конвертации данных в нужный формат
//--------------------------------------------------------------------------------------------
private:
	//Выделить буфер на максимальный возможный объём файла
	void AllocBuffer();
	//Заполнить заголовки и разметить память
	void FillHeaders();
	//Заполнить масив костей
	void FillBones();
	//Заполнить ноды
	void FillNodes();
	//Заполнить линки
	void FillLinks();
	//Заполнить константы
	void FillConsts();
	//Заполнить события с параметрами
	void FillEvents();
	//Заполнить таблицу поиска
	void FillFndTable();
	//Заполнить таблицу клипов, треков и выгрузить клипы
	void ExportClips();
	//Копировать таблицу строк
	void CopyStrings();


//--------------------------------------------------------------------------------------------
public:
	AnxOptions & opt;
	array<string> error;			//Ошибки выгрузки
	string err;						//Накопление ошибки
	//Промежуточный формат с собранными данными
	array<BoneInfo> bones;			//Косточки мастер скелета
	array<NodeInfo> nodes;			//Ноды
	array<ClipInfo> clips;			//Клипы
	array<LinkInfo> links;			//Линки
	array<ConstInfo> consts;		//Константы
	array<EventInfo> events;		//Cобытия
	array<dword> evtParams;			//Параметры событий
	array<char> strTable;			//Таблица символов
	array<StringInfo> strTableStr;	//Описание строк
	long startNode;					//Индекс стартового нода	
	long entryStrings[4096];		//Входная таблица поиска строк
	bool isCritError;				//Есть критические ошибки
	//Конечный формат
	byte * buffer;					//Не выравненый буфер
	byte * data;					//Данные
	dword totalSize;				//Размер данных
	AnxHeader * header;				//Заголовок данных
	const char * stringsBase;		//Базовый адрес таблицы строк
	byte * tracksData;				//Данные треков
};

#endif

