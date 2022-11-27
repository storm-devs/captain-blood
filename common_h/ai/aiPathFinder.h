#ifndef AIPATHFINDER_MISSION_OBJECT
#define AIPATHFINDER_MISSION_OBJECT

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


#include "..\Mission.h"
#include "aiPath.h"

class IPathFinder : public MissionObject
{
public:
	virtual ~IPathFinder() {};

	// создает объект с отложенным стартом, через который происходит проход по пути, с проверкой на проходимость
	virtual IPath * GetPath(const Vector & from, const Vector & to, const Vector & trace_add, dword phys_mask, bool ally) = 0;

	// Кусок для дебажной инфы
	virtual dword GetGraphSize() const = 0;
	virtual const Vector & GetPointPos(dword dwPointIdx) const = 0;
	virtual bool IsPointDeleted(dword dwPointIdx) const = 0;
	virtual dword GetLinksSize(dword dwPointIdx) const = 0;
	virtual dword GetLink(dword dwPointIdx, dword dwLinkIdx) const = 0;
};

class aiPathFinder : public IPathFinder//MissionObject
{
public:
	virtual ~aiPathFinder() {};

};



#endif