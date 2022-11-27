#ifndef PATHFINDER_MISSION_OBJECT
#define PATHFINDER_MISSION_OBJECT

//***************************************************************************************************
//
// Объект миссии - граф для поиска путей, дизайнер может родив такой объект создать граф с путями
// А программист может запросить у этого объекта путь в графе из точки А в точку Б
//
// ищет используя алгоритмы:
// 
//  (A*  "А-звездочка") GeneratePath_AStar  - хорош для равномерного графа быстрый
//  (Алгоритм Дийкстры) GeneratePath_Dijkstra - для любых графов но более медленный
//
//***************************************************************************************************


#include "..\..\..\Common_h\AI\aiPathFinder.h"

#define WAY_POINT_GROUP GroupId('W','P','S','S')

class PathFinder;
class VPath : public IPath
{
friend class PathFinder;
friend class PathService;
public:
	VPath(bool bUsed = false);
	
	void Use();
	bool IsUsed() const;
	void SetPath(const Vector & from, const Vector & to, const Vector & add, MissionObject * pathFinder, dword phys_mask, bool ally);
	MissionObject * GetObject();

	// Высвобождает связи и сам путь
	virtual bool Release();

	// возвратить начальную точку отрезка пути
	virtual const Vector & Pnt0() const;
	// возвратить конечную точку отрезка пути
	virtual const Vector & Pnt1() const;
	// возвратить текущую точку где находится 
	virtual const Vector & Curr() const;
	// возвратить точку откуда начали искать путь
	virtual const Vector & From() const;
	// возвратить точку до куда начали искать путь
	virtual const Vector & To() const;

	// true если путь закончился
	virtual bool IsDone() const;
	// возвращает true при первом вызове после того как путь стартовал
	// потом сбрасывает флажок и возвращает false
	virtual bool IsStarted();
	// возвращает true если путь стартовал и готов к использованию
	virtual bool IsReady() const; 
	// возвращает true если если мы находимся на последнем отрезке
	virtual bool IsLast() const;
	// возвращает true если путь непроходимый
	virtual bool IsImpassable() const; 

	// перейти к следующему отрезку, если есть такая возможность
	// pnt1 становится pnt0, выбирается новый pnt1
	// возвращает true - если новый отрезок нашелся, false - если тупик или конец пути
	// эта функция сбрасывает указатель текущего положения в начальную точку отрезка
	virtual bool Next();

	// Перемещает указатель по пути на расстояние fDistance
	// возвращает true - если прошел дистанцию и есть еще куда идти, false - если дошел до конца или уперся в тупик
	virtual bool Move(float fDistance);

	// возвращает кол-во точек в пути
	virtual int GetNumPoints() const;

	void Start();

	// Отрисовка дебажной инфы
	void Draw(dword color = 0xFFFFFFFF);


private:
	array<dword>	vpath;
	PathFinder		* m_PathFinder;
	bool			m_bUsed, m_bReady, m_bStarted, m_bImpassable, m_bDone, m_ally;
	Vector			m_from, m_to, m_traceadd;
	dword			m_physMask;
	Vector			v1, v2;			// v0 - первая точка отрезка пути, v1 - вторая точка отрезка пути
	Vector			curr;			// текущая точка пути между v0 и v1
	dword			c1, c2;			// p1 - стартовая точка пути, p2 - конечная, c1 - текущая точка пути, c2 - следующая
};

class PathService : public Service
{
public:
	PathService();
	virtual ~PathService();

	// Инициализация сервиса
	virtual bool Init();
	//Исполнение в начале кадра
	virtual void StartFrame(float dltTime);

	// Создает новый или возвращает свободный объект пути
	VPath * GetFreePath();
	// освобождает все пути связанные с этим объектом
	void ReleasePaths(MissionObject * PathFinder);
	// добавляем непроходимую связь
	void AddImpassableLink(MissionObject * PathFinder, dword c1, dword c2);

private:
	struct ImpassableLink
	{
		MissionObject	* pathFinder;
		dword			c1, c2;
	};

	// массив всех путей
	array<VPath*> aPaths;
	// массив путей ждущих когда их запустят
	array<VPath*> aStarts;

	long impassableIndex;
	// массив непроходимых путей
	array<ImpassableLink> aImpassableLinks;
};

class PathFinder : public aiPathFinder
{
public:
	enum RaycastResult
	{
		rr_ok = 0,
		rr_patch_hit,
		rr_ally_hit,
	};

private:
friend class VPath;
friend class PathService;

	struct PathPoint;

	struct PathGroup
	{
		const char * Name; // Имя группы
		bool bActive; //Активна группа или нет
		bool bIgnoreDefAutoThreshold;
		float fAutoThreshold;
	};

	struct LinkPoint
	{
		LinkPoint(PathPoint	* point) { pnt = point; distance = 0.0f; busy = 0; bDesignBlocked = false; bDynamicBlocked = false;}
		LinkPoint()	{ pnt = null; distance = 0.0f; busy = 0; bDesignBlocked = false; bDynamicBlocked = false; }

		bool IsBlocked() const { return bDesignBlocked || bDynamicBlocked; }

		PathPoint * pnt;
		float distance;		// расстояние от папы до сюда
		dword busy;
		bool bDesignBlocked;
		bool bDynamicBlocked;
	};

	struct PathPoint
	{
		Vector Position; // Позиция точки
		long blockedLinks;
		bool bDeleted;
		bool bUsed;
		bool bInOpen;
		bool bInAutoThreshold;
		long MasterGroup; // Какой группе принадлежит
		long pointindex;

		float h, g;
		PathPoint * p;

		array<LinkPoint> Links; // Список линков куда можно из этой точки попасть

		PathPoint() : Links(_FL_, 5)
		{
			/*Position = Vector (0.0f);
			blockedLinks = 0;
			bDeleted = false;
			bUsed = false;
			bInOpen = false;
			bInAutoThreshold = true;
			MasterGroup = 0;*/
		}

		bool HaveLinkTo (PathPoint * pnt) const
		{
			for (dword i = 0; i < Links.Size(); i++)
				if (Links[i].pnt == pnt) return true;
			return false;
		}
	};

	array<PathPoint> Graph;
	array<PathGroup> Groups;
	
	bool bDraw;
	float fWeldThreshold;

	bool bShowID;

	bool  bShowAutoThreshold;
	float fAutoThreshold;

	Vector vFinalPoint;

private:
	array<dword>	aOpen;
	PathService		* m_PathService;

	void BuildGraph(MOPReader & reader);

	// возвращает указатель на линк из точки с1 в с2
	LinkPoint * GetLinkPointer(dword c1, dword c2);

	// возвращает путь из точек, построенный по A* с учетом загруженности путей и непроходимостей
	bool GetPath_AStar(const Vector & from, const Vector & to, array<dword> & path);
	// трейсит на проходимость по 
	bool PathTrace(const Vector & from, const Vector & to, const Vector & add, dword physmask, bool ally);
	// Проверяет связь на проходимость и релизит ее если стала проходимой
	// Возращает true если связь стала проходимой
	bool CheckImpassableLink(dword c1, dword c2);
	// повышает счетчик загруженности связи
	void FreeLink(dword c1, dword c2);
	// понижает счетчик загруженности связи
	void CaptureLink(dword c1, dword c2);
	// возвращает true если raycast встретил объект
	RaycastResult ImpassableRaycast(const Vector & from, const Vector & to, bool ally);
	// проверяют отрезок на пересечение с объектами которые загораживают путь
	RaycastResult ImpassableRaycast(dword c1, dword c2, bool ally);
	// Проверяет связь на проходимость, и если она непроходима, то добавляет ее в сервис на обработку
	// Возвращает true если связь непроходима
	bool TryImpassableLink(dword c1, dword c2, bool ally);
	// блокирует все связи в этой точке и с этой точкой
	void BlockGraphPoint(dword c);

public:
	//Конструктор - деструктор
	PathFinder();
	virtual ~PathFinder();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	void ShowDebug(bool show);
	void _cdecl Draw(float fDeltaTime, long level);

	virtual dword GetGraphSize() const;
	virtual const Vector & GetPointPos(dword dwPointIdx) const;
	virtual bool IsPointDeleted(dword dwPointIdx) const;
	virtual dword GetLinksSize(dword dwPointIdx) const;
	virtual dword GetLink(dword dwPointIdx, dword dwLinkIdx) const;

	// возращает номер ближайшей точки к pos
	int GetNearestPoint(const Vector & pos) const;

	// создает объект с отложенным стартом, через который происходит проход по пути, с проверкой на проходимость
	IPath * GetPath(const Vector & from, const Vector & to, const Vector & trace_add, dword phys_mask, bool ally);
	// Проверяет точку графа на попадание внутрь физ объекта, если попадает то блокирует связи этой точки и возвращает true
	bool CheckGraphPoint(dword c);

	MO_IS_FUNCTION(PathFinder, MissionObject);
};

#endif