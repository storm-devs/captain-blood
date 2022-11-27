
#include "RespawnGizmo.h"
#include "..\Arbiter\CharactersArbiter.h"
#include "..\..\Character\Character.h"
#include "..\..\Character\Components\CharacterPhysics.h"
#include "..\..\Character\Components\CharacterLogic.h"

RespawnGizmo::EnumElement RespawnGizmo::enumElements[] = 
{
	{ConstString("Character"), MG_CHARACTER},
	{ConstString("Ship"), MG_SHIP},
};

RespawnGizmo::RespawnGizmo() : 
	points(_FL_),
	list(_FL_),
	watchList(_FL_),
	bestPoints(_FL_, 16)
{
	skipIfNoActive = false;
	group = null;
}

RespawnGizmo::~RespawnGizmo()
{
	if(group)
	{
		group->Release();
		group = null;
	}
}

//Инициализировать объект
bool RespawnGizmo::Create(MOPReader & reader)
{
	list.Empty();
	points.Empty();
	points.AddElements(reader.Array());
	for(long i = 0; i < points; i++)
	{
		points[i] = reader.Position();
	}
	watchList.Empty();
	watchList.AddElements(reader.Array());
	for(long i = 0; i < watchList; i++)
	{
		WatchElement & we = watchList[i];
		ConstString id = reader.String();				
		if(!FindObject(id,we.mo))
		{
			LogicDebugError("Mission object \"%s\" not found...", id.c_str());
		}		
	}
	bool isAct = reader.Bool();
	skipIfNoActive = reader.Bool();
	watchEnable = reader.Bool();
	ConstString enDesc = reader.Enum();
	for(dword i = 0; i < ARRSIZE(enumElements); i++)
	{
		if(enDesc == enumElements[i].desc)
		{
			group = &GroupIterator(enumElements[i].id, _FL_);
			break;
		}
	}	
	Assert(group);
	Activate(isAct);
	return true;
}

//Обработчик команд для объекта
void RespawnGizmo::Command(const char * id, dword numParams, const char ** params)
{
	if(!id) id = "";
	if(string::IsEqual(id, "respawn"))
	{
		if(!IsActive() && skipIfNoActive)
		{
			LogicDebugError("Object no active, skip command <respawn>");
			return;
		}
		if(numParams > 0)
		{
			MOSafePointer obj;			
			FindObject(ConstString(params[0]), obj);

			if(obj.Ptr())
			{
				bool inlist = false;

				for (int i=0;i<(int)list.Size();i++)
				{
					if (list[i]==obj.Ptr())
					{
						inlist = true;

						break;
					}
				}

				if (!inlist)
				{									
					list.Add(obj.Ptr());
					Activate(IsActive());
					LogicDebug("Command <respawn>. Character \"%s\" put to list", params[0]);
				}
			}
			else
			{
				LogicDebugError("Character \"%s\" not found", params[0]);
			}
		}
		else
		{
			LogicDebugError("Not set character ID fo respawn");
		}

	}else
	if(string::IsEqual(id, "reset"))
	{
		list.Empty();
		LogicDebug("Command <reset>. Respawn list be cleared");
	}else
	if(string::IsEqual(id, "enable"))
	{
		watchEnable = true;
		SetUpdate(&RespawnGizmo::Watch, ML_EXECUTE1);
	}else
	if(string::IsEqual(id, "disable"))
	{
		watchEnable = false;
		DelUpdate(&RespawnGizmo::Watch);
	}else{
		LogicDebugError("Unknown command \"%s\".", id);
	}
}

//Инициализировать объект в режиме редактирования
bool RespawnGizmo::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&RespawnGizmo::Draw, ML_ALPHA3);
	return EditMode_Update(reader);
}

//Обновить параметры в режиме редактирования
bool RespawnGizmo::EditMode_Update(MOPReader & reader)
{
	points.Empty();
	list.Empty();
	tmpMtx.SetIdentity();
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void RespawnGizmo::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}

//Активировать/деактивировать объект
void RespawnGizmo::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(IsActive())
	{
		if(list > 0)
		{
			SetUpdate(&RespawnGizmo::Work, ML_EXECUTE2);
		}
		if(watchEnable)
		{
			SetUpdate(&RespawnGizmo::Watch, ML_EXECUTE1);
		}else{
			DelUpdate(&RespawnGizmo::Watch);
		}
		LogicDebug("Activate");
	}else{
		DelUpdate(&RespawnGizmo::Work);
		DelUpdate(&RespawnGizmo::Watch);
		LogicDebug("Deactivate");
	}
}

//Получить матрицу объекта
Matrix & RespawnGizmo::GetMatrix(Matrix & mtx)
{
	return (mtx = tmpMtx);
}

//Работа логики
void _cdecl RespawnGizmo::Work(float dltTime, long level)
{
	//Если список пуст, неработаем дальше
	if(list.IsEmpty())
	{
		DelUpdate(&RespawnGizmo::Work);
		return;
	}

	bestPoints.Empty();

	//Получим радиус и центр сферы, описывающей игрока
	MissionObject * respObject = list[0];
	Vector min(0.0f), max(0.0f);
	respObject->GetBox(min, max);	
	Vector center = (min + max)*0.5f;
	float radius = (max - min).GetLength()*0.5f;
	//Получим игрока
	MissionObject * player = Mission().Player();
	Vector playerPos = player ? player->GetMatrix(Matrix()).pos : 0.0f;
	//Плоскости камеры
	const Plane * plane = Render().GetFrustum();
	//Перебираем точки ища невидимые, ближнии и свободные
	long index = -1;
	float bestDistToPlayer2;
	for(long i = 0; i < points; i++)
	{
		//Проверим на необходимость тестировать данную точку
		const Vector & pnt = points[i];
		float distToPlayer2 = ~(playerPos - pnt);
		if(!player)
		{
			distToPlayer2 = 0.0f;
		}
		if(index >= 0)
		{
			if(distToPlayer2 >= bestDistToPlayer2)
			{
				continue;
			}
		}
		//Проверим на видимость точку
		Matrix mtx;
		//Позиция респауна
		if(player)
		{
			mtx.vz = (playerPos - pnt).GetXZ();
			if(mtx.vz.NormalizeXZ() > 1e-5f)
			{
				mtx.vy = Vector(0.0f, 1.0f, 0.0f);
				mtx.vx = mtx.vy ^ mtx.vz;
			}else{
				continue;
			}
		}
		mtx.pos = pnt;
		Vector worldCenter = mtx*center;
		//Перебераем плоскости фрустума
		for(long j = 0; j < 4; j++)
		{
			if(plane[j].Dist(worldCenter) < -radius) break;
		}
		if(j >= 4) continue;
		//Точка невидна, проверим на свободность
		for(group->Reset(); !group->IsDone(); group->Next())
		{
			MissionObject * mo = group->Get();
			//С собой ничего не тестируем
			if(mo == respObject) continue;
			//Невидимых пропускаем
			if(!mo->IsShow()) continue;
			//Описывающий объект ящик
			Matrix tmtx;
			group->Get()->GetMatrix(tmtx);
			Vector tmin(0.0f), tmax(0.0f);
			group->Get()->GetBox(tmin, tmax);
			//Посмотрим пересекается ли сфера с ящиком ориентированым по осям
			if(Box::OverlapsBoxSphere(tmtx, ((tmax - tmin)*0.5f).Abs(), worldCenter, radius))
			{
				break;
			}
		}
		if(group->IsDone())
		{
			BestPoint bp;
			bp.index = i;
			bp.distance = distToPlayer2;
			bp.mtx = mtx;
			bestPoints.Add(bp);
		
			index = i;
			bestDistToPlayer2 = distToPlayer2;
			tmpMtx = mtx;
		}
	}

	if (!bestPoints.IsEmpty())
	{
		bestPoints.QSort(BestPoint::QSort);

		// выбираем точку на средней дальности, с небольшим рандомом
		int index = int(bestPoints.Len() * RRnd(0.25f, 0.6f));
		tmpMtx = bestPoints[index].mtx;

		//Респауним
		const char * pnt[1];
		pnt[0] = GetObjectID().c_str();
		list[0]->Command("respawn", 1, pnt);
		//Удалим отреспавленого
		list.DelIndex(0);
		tmpMtx.SetIdentity();
	}
}

//Наблюдение за списком персонажей
void _cdecl RespawnGizmo::Watch(float dltTime, long level)
{
	for(long i = 0; i < watchList; i++)
	{
		WatchElement & we = watchList[i];
		if(we.mo.Validate())
		{
			if(we.mo.Ptr()->IsDead())
			{
				const char * params[1];
				params[0] = we.mo.Ptr()->GetObjectID().c_str();
				Command("respawn", 1, params);
			}
		}
	}
}

//Рисование точек
void _cdecl RespawnGizmo::Draw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;
	if(!EditMode_IsSelect())
	{
		return;
	}
	for(long i = 0; i < points; i++)
	{
		//Плоскости камеры
		const Plane * plane = Render().GetFrustum();
		for(long j = 0; j < 4; j++)
		{
			if(plane[j].Dist(points[i]) < -0.3f)
			{
				break;
			}
		}
		if(j >= 4)
		{
			Render().DrawSphere(points[i], 0.3f, 0x8fffff00);
			continue;
		}
		Render().DrawSphere(points[i], 0.3f, 0x8fff00ff);
	}
}

//Пересоздать объект
void RespawnGizmo::Restart()
{
	ReCreate();
}

//============================================================================================
//Параметры инициализации
//============================================================================================

const char * RespawnGizmo::comment = 
"Respawn characters at nearest of frustum camera points...\n"
"\n"
"Commands list:\n"
"----------------------------------------\n"
"  Add character to respawn list\n"
"----------------------------------------\n"
"    command: respawn\n"
"    param: character id\n"
" \n"
"----------------------------------------\n"
"  Clear respawn list\n"
"----------------------------------------\n"
"    command: reset\n"
"----------------------------------------\n"
"  Enable watch\n"
"----------------------------------------\n"
"    command: enable\n"
" \n"
"----------------------------------------\n"
"  Disable watch\n"
"----------------------------------------\n"
"    command: disable\n"
" \n"
" ";


MOP_BEGINLISTCG(RespawnGizmo, "Respawn gizmo", '1.00', 1000, RespawnGizmo::comment, "Character objects")
	MOP_ENUMBEG("Groups")
		for(dword i = 0; i < ARRSIZE(RespawnGizmo::enumElements); i++)
		{
			MOP_ENUMELEMENT(RespawnGizmo::enumElements[i].desc.c_str())
		}
	MOP_ENUMEND
	MOP_ARRAYBEG("Respawn points", 0, 100)
		MOP_POSITION("Point", Vector(0.0f))
	MOP_ARRAYEND
	MOP_ARRAYBEG("Watch characters", 0, 100)
		MOP_STRING("Character", "")
	MOP_ARRAYEND
	MOP_BOOL("Active", true)
	MOP_BOOLC("Skip", false, "Skip respawn command, when gizmo not active")
	MOP_BOOLC("Enable watch", false, "Watch alive characters from watch list")
	MOP_ENUM("Groups", "Respawn")
MOP_ENDLIST(RespawnGizmo)

