#include "..\..\..\common_h\mission\Mission.h"
#include "..\..\..\common_h\IMissionPhysObject.h"
#include "aiPathFinder.h"

CREATE_SERVICE(PathService, 100)

#define INIT_DIJKSTRA_LEN	9999999999999.0f

//dword dw2_total = 0;
//dword dw2_count = 0;

VPath::VPath(bool bUsed) :
	vpath(_FL_, 16)
{
	m_bUsed = bUsed;
	m_bReady = false;
	m_bStarted = false;
	m_bImpassable = false;
	m_PathFinder = null;
	m_physMask = 0;
	c1 = c2 = INVALID_ARRAY_INDEX;
}

bool VPath::IsUsed() const { return m_bUsed; } 
bool VPath::IsReady() const { return m_bReady; } 
bool VPath::IsImpassable() const { return m_bImpassable; }
bool VPath::IsStarted() 
{ 
	bool bFlag = m_bStarted;
	m_bStarted = false;
	return bFlag; 
}
bool VPath::IsDone() const 
{ 
	//if (c1 == INVALID_ARRAY_INDEX && c2 == INVALID_ARRAY_INDEX) return true;
	//if (m_bImpassable) return true;
	return m_bDone;
}

bool VPath::IsLast() const
{
	return ~(v2 - m_to) < 1e-5f;
}

void VPath::Use()
{
	m_bUsed = true;
}

bool VPath::Release()
{
	// релизим путь
	for (long i=0; i<vpath.Len() - 1; i++)
		m_PathFinder->FreeLink(vpath[i], vpath[i + 1]);

	c1 = c2 = INVALID_ARRAY_INDEX;

	m_bUsed = false;
	m_bReady = false;
	m_bStarted = false;
	m_bImpassable = false;
	m_bDone = true;
	m_PathFinder = null;
	m_physMask = 0;

	vpath.Empty();

	return true;
}

MissionObject * VPath::GetObject()
{
	return m_PathFinder;
}

void VPath::SetPath(const Vector & from, const Vector & to, const Vector & trace_add, MissionObject * pathFinder, dword physmask, bool ally)
{
	m_PathFinder = (PathFinder *)pathFinder;
	m_from = from;
	m_to = to;
	m_traceadd = trace_add;
	c1 = c2 = INVALID_ARRAY_INDEX;
	m_bReady = false;
	m_bStarted = false;
	m_bImpassable = false;
	m_physMask = physmask;
	m_bDone = false;
	m_ally = ally;

	vpath.Empty();
}

const Vector & VPath::Pnt0() const { return v1; }
const Vector & VPath::Pnt1() const { return v2; }
const Vector & VPath::Curr() const { return curr; }
const Vector & VPath::From() const { return m_from; }
const Vector & VPath::To() const { return m_to; }

void VPath::Start()
{
	Assert(m_PathFinder);

	// отключаем полный оптимизатор если путь длинный, или разница в высотах сильная
	// если на полном пути что-то мешает идти, то идем по вейпоинтам
	bool bOptimizeOff = true;
	if (~(m_from - m_to) < Sqr(20.0f) && fabsf(m_from.y - m_to.y) < 1.0f)
		bOptimizeOff = m_PathFinder->ImpassableRaycast(m_from, m_to, m_ally) != PathFinder::rr_ok;

	m_bStarted = true;
	m_bReady = true;

	c1 = -1;
	c2 = -1;

	m_bDone = false;

	// оптимизируем весь путь если есть Tracer
	// если на прямом пути ничего нет, то пробуем проверить возможность пройти по прямой по патчу
	if (!bOptimizeOff)
	{
		bool bWholeOptimize = m_PathFinder->PathTrace(m_from, m_to, m_traceadd, m_physMask, m_ally) == false;
		if (bWholeOptimize)
		{
			v1 = curr = m_from;
			v2 = m_to;

			// тестим на боксы физобъектов
			if (m_PathFinder->ImpassableRaycast(v1, v2, m_ally) == PathFinder::rr_ok) return;
			// если непроходимая последняя точка
			m_bDone = true;
			m_bImpassable = true;
			return;
		}
	}

	// получаем полный путь
	if (!m_PathFinder->GetPath_AStar(m_from, m_to, vpath) || vpath.Size() < 2)
	{
		vpath.Empty();
		// нет пути 
		m_bImpassable = true;
		m_bDone = true;
		m_PathFinder->CheckGraphPoint(m_PathFinder->GetNearestPoint(m_from));
		m_PathFinder->CheckGraphPoint(m_PathFinder->GetNearestPoint(m_to));
		return;
	}

	c2 = 0;
	v1 = curr = m_from;
	v2 = m_PathFinder->GetPointPos(vpath[c2]);

	// захватываем путь
	for (long i=0; i<vpath.Len() - 1; i++)
		m_PathFinder->CaptureLink(vpath[i], vpath[i + 1]);

	// оптимизируем начало, если можно пройти от m_from до c2 + 1
	if (!bOptimizeOff)
	{
		Vector v = m_PathFinder->GetPointPos(vpath[1]);
		bool bFirstOptimize = m_PathFinder->PathTrace(m_from, v, m_traceadd, m_physMask, m_ally) == false;

		if (bFirstOptimize)
		{
			c1 = -1;
			c2 = 1;
			v2 = v;

			return;
		}
	}

	// оптимизируем начало, если точка m_from и точка p1 рядом
	/*if (~(m_from - v2) <= Sqr(1.0f))
	{
		Next();
	
		v1 = curr = m_from;
	}*/
}

// перейти к следующему отрезку, если есть такая возможность
bool VPath::Next()
{
	Assert(m_PathFinder);
	if (IsDone()) return false;

	bool bImpassable = false;

	// если это была последняя связь(или рядом с точкой)
	if (~(v2 - m_to) < 0.5f)
	{
		c1 = INVALID_ARRAY_INDEX;
		c2 = INVALID_ARRAY_INDEX;

		m_bDone = true;
		
		return false;
	}

	// переходим на следующую точку
	c1 = c2;
	c2 = (c2 == vpath.Last()) ? -1 : c2 + 1;

	// проверяем две-три следующие связи на предмет проходимости
	// если первая связь не проходимая, то ищем новый путь из c1 в m_to
	long impassableCount = 0;
	bool impassableFirst = false;
	for (long i=c1; i<long(c1+3), i<vpath.Len(); i++)
	{
		if (i == vpath.Last())
		{
			Vector v = m_PathFinder->GetPointPos(vpath[i]);
			if (m_PathFinder->ImpassableRaycast(v, m_to, m_ally) != PathFinder::rr_ok) impassableCount++;
		}
		else
			if (m_PathFinder->TryImpassableLink(vpath[i], vpath[i+1], m_ally) != PathFinder::rr_ok) impassableCount++;
		
		if (i == c1 && impassableCount) impassableFirst = true;
	}

	if (impassableFirst)
	{
		m_bDone = true;
		m_bImpassable = true;
		return false;
	}

	// оптимизируем последнюю точку, если m_to близко от последней точки
	if (c2 == vpath.Last())
	{
		v2 = m_PathFinder->GetPointPos(vpath[c2]);

		if (~(m_to - v2) <= Sqr(1.0f))
		{
			v2 = m_to;
			if (m_PathFinder->ImpassableRaycast(v1, v2, m_ally) == PathFinder::rr_ok) return true;
			// если непроходимая последняя точка
			m_bDone = true;
			m_bImpassable = true;
			return false;
		}
	}

	// если дошли до конечной точки - то идем еще до m_to
	if (c1 == vpath.Last())
	{
		v1 = curr = v2;
		v2 = m_to;

		if (m_PathFinder->ImpassableRaycast(v1, v2, m_ally) == PathFinder::rr_ok) return true;
		// если непроходимая последняя точка
		m_bDone = true;
		m_bImpassable = true;
		return false;
	}
	else
	{
		v1 = curr = m_PathFinder->GetPointPos(vpath[c1]);
		v2 = m_PathFinder->GetPointPos(vpath[c2]);
	}

	return true;
}

// возвращает кол-во точек в пути
int VPath::GetNumPoints() const
{
	return vpath.Len();
}

// Перемещает указатель по пути на расстояние fDistance
bool VPath::Move(float fDistance)
{
	if (IsDone()) return false;

	while (fDistance > 0.0f)
	{
		float cdist = (v2 - curr).GetLength();

		if (cdist < fDistance)
		{
			fDistance -= cdist;
			if (!Next())
			{
				curr = v2;
				return false;
			}
			continue;
		}
		
		curr = curr + (!(v2 - curr)) * fDistance;
		return true;
	}

	return false;
}

void VPath::Draw(dword color)
{
	if (!IsReady()) return;

	IRender * pRS = (IRender *)api->GetService("DX9Render");

	for (long j=0; j<vpath.Len() - 1; j++)
	{
		const Vector & v1 = m_PathFinder->GetPointPos(vpath[j]) + Vector(RRnd(-0.01f, 0.01f), RRnd(-0.01f, 0.01f), RRnd(-0.01f, 0.01f));
		const Vector & v2 = m_PathFinder->GetPointPos(vpath[j + 1]) + Vector(RRnd(-0.01f, 0.01f), RRnd(-0.01f, 0.01f), RRnd(-0.01f, 0.01f));
		pRS->DrawVector(v1, v2, color);
	}
}

PathService::PathService() :
	aPaths(_FL_, 32),
	aStarts(_FL_, 32),
	aImpassableLinks(_FL_, 128)
{
	impassableIndex = 0;
}

PathService::~PathService()
{
	// уничтожаем все объекты пути
	aPaths.DelAllWithPointers();
}

bool PathService::Init()
{
	aPaths.DelAllWithPointers();

	// резервируем несколько путей
	for (long i=0; i<16; i++)
		aPaths.Add(NEW VPath(false));

	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel);

	return true;
}

VPath * PathService::GetFreePath()
{
	for (long i=0; i<aPaths.Len(); i++)
		if (!aPaths[i]->IsUsed()) 
		{
			aPaths[i]->Use();
			aStarts.Add(aPaths[i]);
			return aPaths[i];
		}
	
	VPath * path = NEW VPath(true);
	aPaths.Add(path);
	aStarts.Add(path);
	return path;
}

void PathService::AddImpassableLink(MissionObject * PathFinder, dword c1, dword c2)
{
	ImpassableLink	link;
	link.pathFinder = PathFinder;
	link.c1 = c1;
	link.c2 = c2;

	aImpassableLinks.Add(link);
}

void PathService::StartFrame(float dltTime)
{
	// Стартуем максимум 2 пути за кадр(возможно завязать на время?)
	for (long i=0; i<2; i++)
	{
		if (aStarts.IsEmpty()) break;
		VPath * p = aStarts[0];
		if (p->IsUsed() && !p->IsReady()) 
		{
			ProfileTimer timer;
			p->Start();
			timer.Stop();
			/*api->Trace("------------------");
			api->Trace("Path statistic: ticks = %d", timer.Get32());
			api->Trace("num pnts: %d", p->vpath.Size());
			if (p->vpath.Size() > 2)
				api->Trace("1st pnt: %d, last pnt: %d", p->vpath[0], p->vpath.LastE());
			api->Trace("first jump: %d", p->c1);*/
		}
		aStarts.Extract(0);
	}

	// проверяем максимум 2 непроходимые связи за кадр(возможно завязать на время?)
	for (long i=0; i<2; i++)
	{
		if (aImpassableLinks.IsEmpty()) break;

		impassableIndex = impassableIndex % aImpassableLinks.Len();
		ImpassableLink & il = aImpassableLinks[impassableIndex];
		PathFinder * pf = (PathFinder*)il.pathFinder;
		// если линк освободился, то убираем его
		if (pf->CheckImpassableLink(il.c1, il.c2))
		{
			aImpassableLinks.Extract(impassableIndex);
			continue;
		}
		impassableIndex++;
	}
}

void PathService::ReleasePaths(MissionObject * PathFinder)
{
	// релизим пути связанные с этим объектом
	for (long i=0; i<aPaths.Len(); i++)
		if (aPaths[i]->GetObject() == PathFinder)
			aPaths[i]->Release();

	// убираем непроходимые связи связанные с этим объектом
	for (long i=0; i<aImpassableLinks.Len(); i++)
		if (aImpassableLinks[i].pathFinder == PathFinder)
		{
			aImpassableLinks.ExtractNoShift(i);
			i--;
		}
}

//Конструктор
PathFinder::PathFinder() : 
	Graph(_FL_, 384),
	Groups(_FL_, 1),
	aOpen(_FL_, 32)
{
	bDraw = true;
	bShowID = false;

	m_PathService = null;
}

//Деструктор
PathFinder::~PathFinder()
{	
	m_PathService->ReleasePaths(this);
	m_PathService = null;
}

//Создание объекта
bool PathFinder::Create(MOPReader & reader)
{
	m_PathService = (PathService *)api->GetService("PathService");

	EditMode_Update (reader);

	Registry(WAY_POINT_GROUP);

	return true;
}

//Обновление параметров
bool PathFinder::EditMode_Update(MOPReader & reader)
{
	fWeldThreshold = reader.Float();
	fAutoThreshold = reader.Float();
	
	ProfileTimer timerBuild;
	BuildGraph(reader);
	timerBuild.Stop();
	api->Trace("Waypoints graph build name = %s, time = %d, points = %d", GetObjectID().c_str(), timerBuild.GetTime32(), Graph.Size());

	bDraw = reader.Bool();
	bShowID = reader.Bool();
	bShowAutoThreshold = reader.Bool();

	if (EditMode_IsOn())
	{
		SetUpdate((MOF_UPDATE)&PathFinder::Draw, ML_ALPHA5);
	}
	else
	{
		DelUpdate((MOF_UPDATE)&PathFinder::Draw);
	}

	return true;
}

void PathFinder::ShowDebug(bool show)
{
	bDraw = show;
	if (show)
	{
		SetUpdate((MOF_UPDATE)&PathFinder::Draw, ML_ALPHA5);
	}
	else
	{
		DelUpdate((MOF_UPDATE)&PathFinder::Draw);
	}

}

void _cdecl PathFinder::Draw(float fDeltaTime, long level)
{
#ifndef STOP_DEBUG
	if (!Mission().EditMode_IsAdditionalDraw()) return;
	if (!bDraw) return;
	//if (!EditMode_IsOn()) return;

	const char* id = GetObjectID().c_str();

	for (dword j = 0; j < Graph.Size(); j++)
	{
		if (Graph[j].bDeleted) continue;

		//if (bShowID)
		Render().Print(Graph[j].Position, 7.0f, 1.0f, 0xff00ffff,"%d, %s : Track %i - %i", j, id,Graph[j].MasterGroup,Graph[j].pointindex);

		Render().DrawSphere(Graph[j].Position, 0.1f, 0xFFFFFF00);

		Render().FlushBufferedLines();
		for (dword n = 0; n < Graph[j].Links.Size(); n++)
		{
			LinkPoint & lp = Graph[j].Links[n];

			const Vector & v1 = Graph[j].Position;
			const Vector & v2 = lp.pnt->Position;

			if (lp.IsBlocked())
			{
				Render().DrawBufferedLine(v1,  0xFF1F1F1F, v2, 0xFF1F1F1F);
			}
			else
			{
				if (Graph[j].MasterGroup == lp.pnt->MasterGroup)
					Render().DrawBufferedLine(v1,  0xFFFFFF00, v2, 0xFFFFFF00);
				else
					Render().DrawBufferedLine(v1, 0xFF0000FF, v2, 0xFF0000FF);
			}
			
			Render().Print(Vector().Lerp(v1, v2, 0.5f), 10.0f, 0.0f, 0xFF00FF00, "%d", dword(lp.busy));
		}
		Render().FlushBufferedLines();

		if (bShowAutoThreshold && !Graph[j].bInAutoThreshold)
		{
			if (Groups[Graph[j].MasterGroup].bIgnoreDefAutoThreshold)
			{			
				Render().DrawSphere(Graph[j].Position, Groups[Graph[j].MasterGroup].fAutoThreshold, 0xFF00FF00);
			}
			else
			{
				Render().DrawSphere(Graph[j].Position, fAutoThreshold, 0xFF00FF00);
			}
		}		
	}	
#endif
}

void PathFinder::BuildGraph (MOPReader & reader)
{
	Graph.DelAll();
	Groups.DelAll();

	long PathCount = reader.Array();

	long total = 0;
	Groups.AddElements(PathCount);
	for (long i = 0; i < PathCount; i++)
	{
		PathGroup & group = Groups[i];
		group.Name = reader.String().c_str();
		group.bIgnoreDefAutoThreshold = reader.Bool();
		group.fAutoThreshold = reader.Float();
		
		long PointsCount = reader.Array();

		PathPoint * previous = NULL;
		for (long n = 0; n < PointsCount; n++)
		{
			PathPoint * current = &Graph[Graph.Add()];
			
			/*current->bDeleted = false;
			current->MasterGroup = i; 
			current->pointindex = n;
			current->bInAutoThreshold = reader.Bool();*/

			current->Position = reader.Position();
			current->blockedLinks = 0;
			current->bDeleted = false;
			current->bUsed = false;
			current->bInOpen = false;
			current->bInAutoThreshold = reader.Bool();
			current->MasterGroup = i;
			current->pointindex = n;
			current->h = 0.0f;
			current->g = 0.0f;
			current->p = null;

			previous = current;
		}
	}

	//В одной группе соединяем линками....
	PathPoint* previous = NULL;
	for (dword i = 0; i < Graph.Size(); i++)
	{
		PathPoint * current = &Graph[i];

		if (previous != NULL)
		{
			if (previous->MasterGroup == current->MasterGroup)
			{
				previous->Links.Add(current);
				current->Links.Add(previous);
			}
		}

		previous = current;
	}
	
	//Теперь пути у которых точки совпадают объеденяем...
	for (dword j = 0; j < Graph.Size(); j++)
	{
		for (dword m = j + 1; m < Graph.Size(); m++)
		{
			if (Graph[j].MasterGroup == Graph[m].MasterGroup) continue;
			const Vector & p1 = Graph[j].Position;
			const Vector & p2 = Graph[m].Position;
			//float fDist = Vector(p1 - p2).GetLength();
			if (~(p1 - p2) < Sqr(fWeldThreshold))
			{
				//Переносим линки из одного узла в другой, а другой убиваем...

				//Перебиваем линки указывающие на этот нод...
				PathPoint* node_to_delete = &Graph[m];
				for (dword z = 0; z < Graph[m].Links.Size(); z++)
				{
					PathPoint* neighbour = Graph[m].Links[z].pnt;

					//Смотрим линки соседа...
					for (dword x = 0; x < neighbour->Links.Size(); x++)
					{
						//Если линк был на узел который умрет, меняем его на новый...
						if (neighbour->Links[x].pnt == node_to_delete)
						{
							neighbour->Links[x].pnt = &Graph[j];
						}
					}

					//Добавляем соседа в нужный нам список... 
					Graph[j].Links.Add(neighbour);
				}

				//Удаляем линки из нода... (сам нод убить нельзя, иначе все ссылки побъються... :()
				Graph[m].Links.DelAll();
				Graph[m].bDeleted = true;

				//Graph[j].Links.Add(&Graph[m]);
				//Graph[m].Links.Add(&Graph[j]);
			}
		}
	}

	//Авто соединение точек в зависимости от их расположенеия
	for (dword j = 0; j < Graph.Size(); j++)
	if (!Graph[j].bDeleted && !Graph[j].bInAutoThreshold)
	{
		for (dword m = j + 1; m < Graph.Size(); m++)
		if (!Graph[m].bDeleted && !Graph[m].bInAutoThreshold)
		{
			if (Graph[j].MasterGroup == Graph[m].MasterGroup) continue;

			float fThreshold = fAutoThreshold;
	
			if (Groups[Graph[j].MasterGroup].bIgnoreDefAutoThreshold)
			{
				//fThreshold = coremax( fThreshold,Groups[Graph[j].MasterGroup].fAutoThreshold);
				fThreshold = Groups[Graph[j].MasterGroup].fAutoThreshold;

				if (Groups[Graph[m].MasterGroup].bIgnoreDefAutoThreshold)
				{
					fThreshold = Max(fThreshold, Groups[Graph[m].MasterGroup].fAutoThreshold);
				}
			}
			else
			if (Groups[Graph[m].MasterGroup].bIgnoreDefAutoThreshold)
			{
				fThreshold = Groups[Graph[m].MasterGroup].fAutoThreshold;
			}

			const Vector & p1 = Graph[j].Position;
			const Vector & p2 = Graph[m].Position;
			//float fDist = Vector(p1 - p2).GetLength();
			if (~(p1 - p2) < Sqr(fThreshold))
			{
				//Проверяем, связаны ли ноды
				// более жесткая проверка, чтобы не было дубликатов линков
				if (!Graph[m].HaveLinkTo(&Graph[j])) Graph[m].Links.Add(&Graph[j]);
				if (!Graph[j].HaveLinkTo(&Graph[m])) Graph[j].Links.Add(&Graph[m]);					
			}
		}
	}

	// рассчитываем расстояние от точек к линкам
	for (long i=0; i<Graph; i++)
	{
		const Vector & v1 = Graph[i].Position;

		for (long j=0; j<Graph[i].Links; j++)
		{
			const Vector & v2 = Graph[i].Links[j].pnt->Position;
			Graph[i].Links[j].distance = (v2 - v1).GetLength();
		}
	}

	/*dword total_ = 0;
	for (long i=0; i<Graph.Len(); i++)
	{
		api->Trace("num links = %d", Graph[i].Links.Size());
		total_ += Graph[i].Links.Size();
	}
	api->Trace("total = %d", total_);*/
}

PathFinder::LinkPoint * PathFinder::GetLinkPointer(dword c1, dword c2)
{
	if (c1 == INVALID_ARRAY_INDEX || c2 == INVALID_ARRAY_INDEX) return null;

	for (long i=0; i<Graph[c1].Links.Len(); i++)
		if (c2 == Graph[c1].Links[i].pnt - &Graph[0])
			return &Graph[c1].Links[i];

	return null;
}

// проверяет отрезок на пересечение с объектами которые загораживают путь
PathFinder::RaycastResult PathFinder::ImpassableRaycast(const Vector & from, const Vector & to, bool ally)
{
	// чуть приподымаем над полом
	Vector h = Vector(0.0f, 0.5f, 0.0f);

	Vector h1 = Vector(0.0f, 0.5f, 0.0f);
	Vector h2 = Vector(0.0f, 2.0f, 0.0f);
	
	Vector _from = from;
	Vector _to = to;

	IPhysicsScene::RaycastResult Result;
	if (Physics().Raycast(from + h1, from - h2, phys_mask(phys_character), &Result))
		_from = Result.position;
	if (Physics().Raycast(to + h1, to - h2, phys_mask(phys_character), &Result))
		_to = Result.position;

	_from = _from + h;
	_to = _to + h;

	Vector delta = Vector(0.1f, 0.6f, 0.1f);

	//Max переделал на поиск в группе
	Vector boxMin, boxMax;
	boxMin.Min(_from, _to);
	boxMax.Max(_from, _to);
	//dword collisionsCount = FindObjects(MG_AI_COLLISION, Vector(-1000.0), Vector(1000.0));

	if (ally)
	{
		if (Physics().Raycast(from + h, to + h, phys_mask(phys_ally), &Result))
			return rr_ally_hit;
	}

	dword count = QTFindObjects(MG_AI_COLLISION, boxMin - delta, boxMax + delta);
	for(dword i = 0; i < count; i++)
	{
		IMissionQTObject* fo = QTGetObject(i);
		if(!fo)
		{
			continue;
		}
		const Vector & boxCenter = fo->GetBoxCenter();
		const Vector & boxSize = fo->GetBoxSize();
		Matrix mtx(fo->GetMatrix());
		mtx.pos = mtx*boxCenter;
		if(Box::OverlapsBoxLine(mtx, boxSize * 0.5f, _from, _to))
		{
			return rr_patch_hit;
		}
	}

	return rr_ok;
}

// Проверяет точку графа на попадание внутрь физ объекта, если попадает то блокирует связи этой точки и возвращает true
bool PathFinder::CheckGraphPoint(dword c)
{
	if (c == INVALID_ARRAY_INDEX) return false;
	if (!ImpassableRaycast(Graph[c].Position, Graph[c].Position, false)) return false;
	BlockGraphPoint(c);
	return true;
}

// блокирует все связи в этой точке и с этой точкой
void PathFinder::BlockGraphPoint(dword c)
{
	if (c == INVALID_ARRAY_INDEX) return;

	PathPoint & p1 = Graph[c];
	for (long i=0; i<p1.Links; i++)
	{
		LinkPoint * l1 = &p1.Links[i];
		LinkPoint * l2 = GetLinkPointer(l1->pnt - &Graph[0], c);

		if (!l1->bDynamicBlocked)
		{
			p1.blockedLinks++;
			l1->bDynamicBlocked = true;
		}

		if (!l2->bDynamicBlocked)
		{
			l2->bDynamicBlocked = true;
			l1->pnt->blockedLinks++;
		}
	}
}

// проверяет отрезок на пересечение с объектами которые загораживают путь
PathFinder::RaycastResult PathFinder::ImpassableRaycast(dword c1, dword c2, bool ally)
{
	Assert(c1 != INVALID_ARRAY_INDEX && c2 != INVALID_ARRAY_INDEX);
	return ImpassableRaycast(Graph[c1].Position, Graph[c2].Position, ally);
}

// Проверяет связь на проходимость, и если она непроходима, то добавляет ее в сервис на обработку
// возвращает true если связь непроходима
bool PathFinder::TryImpassableLink(dword c1, dword c2, bool ally)
{
	Assert(c1 != INVALID_ARRAY_INDEX && c2 != INVALID_ARRAY_INDEX);

	LinkPoint * l1 = GetLinkPointer(c1, c2);
	LinkPoint * l2 = GetLinkPointer(c2, c1);

	// хотя бы 1 связь должна быть
	Assert(l1 && l2);

	// если связь уже заблокирована, то выходим
	if (l1->IsBlocked()) return true;
	if (l2->IsBlocked()) return true;

	// если raycast ничего не встретил, то выходим
	RaycastResult result = ImpassableRaycast(Graph[c1].Position, Graph[c2].Position, ally);
	if (result == rr_ok) return false;
	// если это союзник и врезались в коллижен для союзника, то выходим
	if (result == rr_ally_hit) return true;
	
	l1->bDynamicBlocked = true;
	l2->bDynamicBlocked = true;
	// увеличиваем кол-во закрытых линков
	Graph[c1].blockedLinks++;
	Graph[c2].blockedLinks++;

	m_PathService->AddImpassableLink(this, c1, c2);
	return true;
}

// Проверяет связь на проходимость и релизит ее если стала проходимой
// Возращает true если связь стала проходимой
bool PathFinder::CheckImpassableLink(dword c1, dword c2)
{
	Assert(c1 != INVALID_ARRAY_INDEX && c2 != INVALID_ARRAY_INDEX);

	LinkPoint * l1 = GetLinkPointer(c1, c2);
	LinkPoint * l2 = GetLinkPointer(c2, c1);

	// хотя бы 1 связь должна быть
	Assert(l1 && l2);

	// если заблокирован дизайнерами то выходим
	if (l1->bDesignBlocked || l2->bDesignBlocked) return false;

	// если объекты все еще стоят на пути
	if (ImpassableRaycast(c1, c2, false) != rr_ok) return false;

	// путь свободен, делаем связи проходимыми
	l1->bDynamicBlocked = false; 
	l2->bDynamicBlocked = false;

	// уменьшаем кол-во закрытых линков
	Graph[c1].blockedLinks--;
	Graph[c2].blockedLinks--;

	return true;
}

// повышает счетчик загруженности связи
void PathFinder::FreeLink(dword c1, dword c2)
{
	LinkPoint * l1 = GetLinkPointer(c1, c2);	if (l1 && l1->busy) l1->busy--;
	LinkPoint * l2 = GetLinkPointer(c2, c1);	if (l2 && l2->busy) l2->busy--;
}

// понижает счетчик загруженности связи
void PathFinder::CaptureLink(dword c1, dword c2)
{
	LinkPoint * l1 = GetLinkPointer(c1, c2);	if (l1 && l1->busy < 65535) l1->busy++;
	LinkPoint * l2 = GetLinkPointer(c2, c1);	if (l2 && l2->busy < 65535) l2->busy++;
}

// создает объект с отложенным стартом, через который происходит проход по пути, с проверкой на проходимость
IPath * PathFinder::GetPath(const Vector & from, const Vector & to, const Vector & trace_add, dword phys_mask, bool ally)
{
	dword dw1;
	RDTSC_B(dw1);

	VPath * path = m_PathService->GetFreePath();
	if (!path) return null;
	path->SetPath(from, to, trace_add, this, phys_mask, ally);

	RDTSC_E(dw1);
	//api->Trace("Generate path = %d", dw1);

	return path;
}

int PathFinder::GetNearestPoint (const Vector & pos) const
{
	int Index = -1;
	float fMinDistance = 1e+10f;
	for (dword j = 0; j < Graph.Size(); j++)
	{
		const PathPoint & pnt = Graph[j];
		if (pnt.bDeleted) continue;
		if (pnt.blockedLinks == pnt.Links.Len()) continue;

		float fDist = ~(pnt.Position - pos);
		if (fDist < fMinDistance)
		{
			Index = j;
			fMinDistance = fDist;
		}
	}

	return Index;
}

bool PathFinder::PathTrace(const Vector & from, const Vector & to, const Vector & add, dword physmask, bool ally)
{
	if (Physics().Raycast(from + add, to + add, physmask) != null) return true;
	if (Physics().Raycast(to + add, from + add, physmask) != null) return true;
	if (ally)
	{
		if (Physics().Raycast(from + add, to + add, phys_mask(phys_ally)) != null) return true;
		if (Physics().Raycast(to + add, from + add, phys_mask(phys_ally)) != null) return true;
	}
	return false;
}

bool PathFinder::GetPath_AStar(const Vector & from, const Vector & to, array<dword> & path)
{
	path.Empty();

	ProfileTimer timer1;
	// ищем ближайшие точки к from и to, также заполняем значения по умолчанию
	long idx_from = -1, idx_to = -1;
	float minDist1 = 1e+10f, minDist2 = 1e+10f;
	for (dword i = 0; i < Graph.Size(); i++)
	{
		PathPoint & pnt = Graph[i];
		if (pnt.bDeleted) continue;
		if (pnt.blockedLinks == pnt.Links.Len()) continue;

		pnt.bUsed = false;
		pnt.bInOpen = false;
		pnt.g = 0.0f;
		pnt.h = 0.0f;

		float dist = ~(pnt.Position - from);
		if (dist < minDist1) { idx_from = i; minDist1 = dist; }

		dist = ~(pnt.Position - to);
		if (dist < minDist2) { idx_to = i; minDist2 = dist; }
	}
	/*int idx_from = GetNearestPoint(from);*/	if (idx_from < 0) return false;
	/*int idx_to = GetNearestPoint(to);*/		if (idx_to < 0) return false;
	timer1.Stop();

	vFinalPoint = Graph[idx_to].Position;

	aOpen.Empty();
	Graph[idx_from].p = null;
	Graph[idx_from].bInOpen = true;
	aOpen.Add(idx_from);

	ProfileTimer timer3;
	// основной цикл A*
	while (!aOpen.IsEmpty())
	{
		// достаем самый лучший узел (FIX-ME может сделать list или еще чего?)
		float fBest = 1e+10f;
		dword best, best_idx;
		for (long i=0; i<aOpen.Len(); i++) 
		{
			const PathPoint & p = Graph[aOpen[i]];
			if (p.g + p.h >= fBest) continue;
			best = &Graph[aOpen[i]] - &Graph[0];
			fBest = p.g + p.h;
			best_idx = i;
		}

		PathPoint * p1 = &Graph[best];
		
		// удаляем точку из числа обрабатываемых
		p1->bInOpen = false;
		aOpen.ExtractNoShift(best_idx);

		// выходим если попали в последнюю точку(или рядом с ней) 
		if (best == idx_to) // || 
		{
			PathPoint * p = &Graph[best];
			while (p)
			{
				path.Add(p - &Graph[0]);
				p = p->p;
			}
			path.SwapArray();
			timer3.Stop();
			//api->Trace("------------------------------------------------------------");
			//api->Trace("Path statistic: t1 = %d, t2 = %d", timer1.Get32(), timer3.Get32());
			return true;
		}

		// добавляем в список open всех детей лучшего узла
		// если они удовлетворяют условиям
		for (long j=0; j<p1->Links.Len(); j++)
		{
			PathPoint * p2 = p1->Links[j].pnt;

			// скипаем если связь заблокирована
			if (p1->Links[j].IsBlocked()) continue;

			float k = 1.0f + float(p1->Links[j].busy) * 0.2f;
			float newg = p1->g + k * p1->Links[j].distance;//(p1->Position - p2->Position).GetLength();
			
			// если точка уже использовалась и путь до нее был короче - скипаем эту связь
			if ((p2->bUsed || p2->bInOpen) && p2->g <= newg)
				continue;

			p2->p = p1;
			p2->g = newg;
			p2->h = 0.0f;
			p2->bUsed = false;

			// добавляем точку для обработки
			if (!p2->bInOpen)
			{
				p2->bInOpen = true;
				aOpen.Add(p2 - &Graph[0]);
			}
		}
		p1->bUsed = true;
	}

	timer3.Stop();
	//api->Trace("------------------------------------------------------------");
	//api->Trace("Path statistic failed: t1 = %d, t2 = %d", timer1.Get32(), timer3.Get32());

	return false;
}

dword PathFinder::GetGraphSize() const
{
	return Graph.Size();
}

const Vector & PathFinder::GetPointPos(dword dwPointIdx) const
{
	return Graph[dwPointIdx].Position;
}

bool PathFinder::IsPointDeleted(dword dwPointIdx) const
{
	return Graph[dwPointIdx].bDeleted;
}

dword PathFinder::GetLinksSize(dword dwPointIdx) const
{
	return Graph[dwPointIdx].Links.Size();
}

dword PathFinder::GetLink(dword dwPointIdx, dword dwLinkIdx) const
{
	return Graph[dwPointIdx].Links[dwLinkIdx].pnt - &Graph[0];
}


MOP_BEGINLISTG(PathFinder, "WayPoints", '1.00', 100, "Character objects")
	MOP_FLOAT("Weld threshold", 0.5f);
    MOP_FLOAT("Auto threshold", 5.0f);
	MOP_ARRAYBEG("Paths", 0, 500)	// Массив путей
		MOP_STRING("Path name", "")
		MOP_BOOL("Use own threshold", false)
		MOP_FLOAT("Auto threshold", 5.0f);
		MOP_ARRAYBEG("Points", 0, 200)	// Набор точек
			MOP_POSITION("Position", Vector(0.0f))
			MOP_BOOL("Ignore Auto threshold", false)
		MOP_ARRAYEND
	MOP_ARRAYEND
	MOP_BOOL("Show", true)
	MOP_BOOL("ShowID", true)
	MOP_BOOL("Show Auto Threshold", false)
MOP_ENDLIST(PathFinder)

