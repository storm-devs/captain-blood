
#include "ActorTimeLine.h"
#include "..\..\Character\Character.h"
#include "..\..\Character\Components\CharacterLogic.h"

ActorTimeLine::ActorData::ActorData()
{
	chr = null;
	ani = null;
	playerPosition = 0.0f;
	playerAngle = 0.0f;
	//pos = 0.0f;
	//ay = 0.0f;
	currentPosition = 0.0f;
	currentNode = null;		
}

ActorTimeLine::ActorTimeLine() : timeLine(_FL_, 1)
{
	drawCharacter = null;
	teleportToChar = false;

	cur_rot_time = 0.5f;
	rot_time = cur_rot_time;

	actor_init_node = NULL;
}

ActorTimeLine::~ActorTimeLine()
{
}

//Инициализировать объект
bool ActorTimeLine::Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

//Инициализировать объект в режиме редактирования
bool ActorTimeLine::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&ActorTimeLine::Draw, ML_ALPHA3);
	return EditMode_Update(reader);
}

//Обновить параметры в режиме редактирования
bool ActorTimeLine::EditMode_Update(MOPReader & reader)
{
	timeLine.DelAll();
	long count = reader.Array();
	timeLine.AddElements(count);
	for(long i = 0; i < count; i++)
	{
		TimePoint & tp = timeLine[i];
		tp.linkName = reader.String();
		tp.node = reader.String();
		tp.blendTime = reader.Float();
		switch(reader.Enum().c_str()[0])
		{
		case 'W':
			tp.type = t_wait;
			break;
		case 'A':
			tp.type = t_arrive;
			break;
		case 'L':
			tp.type = t_move;
			break;
		case 'S':
			tp.type = t_change;
			break;
		case 'G':
			tp.type = t_goto;
			break;
		}
		tp.time = reader.Float();
		tp.init_pos = reader.Position();
		tp.init_ay = reader.Angles().y;

		//tp.init_transform = Matrix(Vector(0.0f, init_ay, 0.0f), init_pos);
		tp.gotoPoint = (word)reader.Long();
		tp.isEnableGoto = reader.Bool();
		tp.isPreview = reader.Bool();
		tp.event.Init(reader);
	}
	Vector pos = reader.Position();
	Vector rot = reader.Angles();
	Matrix transform(rot, pos);
	isMovePoint = reader.Bool();
	rot_time = reader.Float();
	teleportToChar = reader.Bool();

	for(long i = 0; i < timeLine; i++)
	{
		Matrix init_transform(true);
		timeLine[i].GetInitTransform(init_transform);
		Matrix mtx(init_transform, transform);
		timeLine[i].pos = mtx.pos;
		timeLine[i].ay = mtx.GetAngles().y;
		/*if(timeLine[i].type == t_goto && timeLine[i].gotoPoint >= timeLine)
		{
			LogicDebug("Out of range, point's index: %i", timeLine[i].gotoPoint);
		}*/
	}
	if(!EditMode_IsOn())
	{
		if(reader.Bool())
		{
			SetUpdate(&ActorTimeLine::Draw, ML_ALPHA3);
		}		
	}
	return true;
}

//Указать поддержку заданного типа 
bool ActorTimeLine::IsControllerSupport(const ConstString & controllerClassName)
{
	//if(!controllerClassName) return false;
	//return (strcmp(controllerClassName, "Movie") == 0);
	return true;
}

//Обработчик команд для объекта
void ActorTimeLine::Command(const char * id, dword numParams, const char ** params)
{
	if(!id || !id[0]) return;
	if(numParams < 1)
	{
		LogicDebugError("Command <%s> error. Invalidate parameters...", id);
		return;
	}
	
	long pointIndex = atol(params[0]);

	if(pointIndex < 0 || pointIndex >= timeLine)
	{
		LogicDebugError("Command <%s> error. Invalidate point index: %i not in range [0, %i]...", id, pointIndex, timeLine.Size());
		return;
	}
	if(timeLine[pointIndex].type != t_goto)
	{
		LogicDebugError("Command <%s> error. Can't change state none \"goto\" point...", id, pointIndex, timeLine.Size());
		return;
	}
	if(string::IsEqual(id, "enable"))
	{
		timeLine[pointIndex].isEnableGoto = true;
	}
	else
	if(string::IsEqual(id, "disable"))
	{
		timeLine[pointIndex].isEnableGoto = false;
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".", id);
	}

	if (drawCharacter->logic->IsActor())
	{
		drawCharacter->UpdateActor(0.0f);
	}
}

//Получить следующую точку относительно текущей
long ActorTimeLine::GetNextPoint(long current)
{
	if(!timeLine || current < 0)
	{
		return -1;
	}
	//Увеличиваем текущую позицию
	current++;
	if(current >= timeLine)
	{
		return -1;
	}
	//Проходим все переходы
	while(timeLine[current].type == t_goto)
	{
		if(timeLine[current].isEnableGoto)
		{
			current = timeLine[current].gotoPoint;
		}else{
			current++;
		}
		if(current < 0 || current >= timeLine) return -1;
	}
	return current;
}

//Установить персонажа в исходную позицию
void ActorTimeLine::Reset(ActorData & data, Matrix& mat)
{
	if (teleportToChar)
	{		
		Matrix m;
		timeLine[0].GetInitTransform(m);
		m.Inverse();

		Matrix transform(m,mat);
		data.transform = transform;		
	}

	Goto(data, 0);

	if (data.ani)
	{
		actor_init_node = data.ani->CurrentNode();
	}

	cur_rot_time = rot_time;
}

//Переместить персонажа
void ActorTimeLine::Move(ActorData & data, float dltTime)
{
	if(!timeLine) return;
	if(data.currentPosition < 0.0f) return;
	//Получаем текущую точку
	long currentPointIndex = long(data.currentPosition);
	if(currentPointIndex >= timeLine)
	{
		//Если текущая точка за пределами, то ничего не делаем
		return;
	}
	//Получаем следующую точку
	long nextPointIndex = GetNextPoint(currentPointIndex);
	if(nextPointIndex < 0 || nextPointIndex >= timeLine)
	{
		//Если нет следующей точки то закончили движение и ничего не делаем
		return;
	}
	//Исходеные данные
	TimePoint & currentTimePoint = timeLine[currentPointIndex];
	TimePoint & nextTimePoint = timeLine[nextPointIndex];
	float relativePosition = data.currentPosition - currentPointIndex;
	const char * curAniNode = null;
	float currentWay = 0.0f;
	if(data.ani)
	{
		curAniNode = data.ani->CurrentNode();
		Vector ptmp;
		float atmp = 0.0f;
		data.ani->GetMovement(ptmp);
		currentWay = ptmp.GetLengthXZ();
	}else{
		curAniNode = null;
		currentWay = 0.0f;
	}
	
	Vector fromPos = GetPos(data,currentTimePoint);
	float fromAy = GetAy(data,currentTimePoint);
	
	if(isMovePoint && currentPointIndex == 0)
	{
		fromPos = data.playerPosition;
		fromAy = data.playerAngle;
		cur_rot_time -= dltTime;
		if (cur_rot_time<0.0f) cur_rot_time = 0.0f;
	}
	

	//Обновляем позицию
	if(currentTimePoint.type != t_change)
	{
		if(currentTimePoint.type == t_arrive)
		{
			//Следующая точка
			Vector to = GetPos(data,nextTimePoint);
			float way = (GetPos(data,nextTimePoint) - fromPos).GetLength();
			if(way > 1e-5f)
			{
				relativePosition += currentWay/way;
			}else{
				relativePosition = 10.0f;
			}
		}else{
			relativePosition += dltTime/currentTimePoint.time;
		}
	}
	else
	{
		if(data.ani)
		{
			if(curAniNode != data.currentNode && !actor_init_node)
			{				
				relativePosition = 10.0f;			
			}
		}
		else
		{
			relativePosition = 10.0f;
		}
	}

	if (currentPointIndex == 0 && actor_init_node && currentTimePoint.node.NotEmpty())
	{
		if (string::IsEqual(curAniNode , currentTimePoint.node.c_str()))
		{
			actor_init_node = null;
		}		
	}

	if (currentPointIndex != 0 || !actor_init_node)
	{
		data.currentNode = curAniNode;
	}	

	//Смотрим на обновление
	if(relativePosition >= 1.0f)
	{
		//Перешли на новую точку
		Goto(data, nextPointIndex);
		return;
	}
	else
	{
		data.currentPosition = currentPointIndex + relativePosition;
	}
	//Получаем координаты для данной позиции
	if(currentTimePoint.type != t_wait && currentTimePoint.type != t_change)
	{
		//Позиция
		data.pos.Lerp(fromPos, GetPos(data,nextTimePoint), relativePosition);
		//Угол
		Quaternion newAy(0.0f, fromAy, 0.0f);
		Quaternion nextAy(0.0f, GetAy(data,nextTimePoint), 0.0f);
		Quaternion res;

		if (isMovePoint && currentPointIndex == 0 && rot_time > 0.0f)
		{
			relativePosition = coremax(relativePosition,1.0f - cur_rot_time/rot_time);
		}

		res.SLerp(newAy, nextAy, relativePosition);
		Matrix mtx;
		res.GetMatrix(mtx);
		data.ay = mtx.GetAngles().y;		
	}
	else
	{
		data.pos = GetPos(data,currentTimePoint);
		data.ay = GetAy(data,currentTimePoint);
	}
}

//Перейти на новую точку таймлайна
void ActorTimeLine::Goto(ActorData & data, long pointIndex)
{
	if(!timeLine) return;
	//Определяем точку куда перейти	
	if(pointIndex < 0 || pointIndex >= timeLine)
	{
		data.currentPosition = (float)timeLine;
		return;
	}
	if(timeLine[pointIndex].type == t_goto)
	{
		pointIndex = GetNextPoint(pointIndex);
	}
	//Устанавлеваем текущую точку
	TimePoint & timePoint = timeLine[pointIndex];
	data.currentPosition = (float)pointIndex;	
	data.pos = GetPos(data,timePoint);
	data.ay = GetAy(data,timePoint);
	
	if(timePoint.linkName.NotEmpty())
	{
		if(data.ani)
		{
			if(!data.ani->ActivateLink(timePoint.linkName.c_str(), true))
			{
				LogicDebugError("Actor mode -> link \"%s\" not activate, current node \"%s\"", timePoint.linkName.c_str(), data.ani->CurrentNode());
			}			
		}
		else
		{
			LogicDebugError("Actor mode -> link \"%s\" not activate, actor animation not set", timePoint.linkName.c_str());
		}
	}
	else
	{
		if(timePoint.node.NotEmpty())
		{
			if(data.ani)
			{
				if(!data.ani->Goto(timePoint.node.c_str(), timePoint.blendTime))
				//if(!data.ani->Goto(timePoint.node, coremax(timePoint.blendTime,0.005f)))
				{
					LogicDebugError("Actor mode -> cant move to node \"%s\", node not found", timePoint.node.c_str());
				}
			}
			else
			{
				LogicDebugError("Actor mode -> cant move to node \"%s\", actor animation not set", timePoint.node.c_str());
			}
		}
	}

	LogicDebug("Go to node %i", pointIndex);
	timePoint.event.Activate(Mission(), false, data.chr);
	if(isMovePoint && pointIndex != 0)
	{
		data.playerPosition = GetPos(data,timeLine[0]);
		data.playerAngle = GetAy(data,timeLine[0]);
	}
	if(data.ani)
	{		
		data.currentNode = data.ani->CurrentNode();
	}
}

//Установить персонажа для отрисовки
void ActorTimeLine::SetDrawCharacter(Character * chr)
{
	if(!drawCharacter)
	{
		drawCharacter = chr;
		FindObject(chr->GetObjectID(),drawCharacterPtr);
	}
}

//Рисование точек
void _cdecl ActorTimeLine::Draw(float dltTime, long level)
{
	if(EditMode_IsOn())
	{
		if (!Mission().EditMode_IsAdditionalDraw()) return;
		if(!EditMode_IsSelect())
		{
			return;
		}
	}
	Render().FlushBufferedLines();
	bool isSel = EditMode_IsSelect();
	dword alpha = isSel ? 0xff000000 : 0xc0000000;
	
	if(!drawCharacterPtr.Validate())
	{
		drawCharacter = null;
	}

	for(long i = 0; i < timeLine; i++)
	{
		bool isDrawLine = false;
		dword color = alpha;
		const char * typeName = "Unknown type";
		switch(timeLine[i].type)
		{
		case t_wait:
			color |= 0x000000ff;
			typeName = "Wait time out";
			break;
		case t_arrive:
			color |= 0x0000ff00;
			isDrawLine = true;
			typeName = "Arrive at next point";
			break;
		case t_move:
			color |= 0x0000ffff;
			isDrawLine = true;
			typeName = "Linear time move";
			break;
		case t_change:
			color |= 0x00ff0000;
			isDrawLine = true;
			typeName = "Step by change node";
			break;
		case t_goto:
			color |= 0x00ff00ff;
			typeName = "Goto other point";
			break;
		}
		Render().DrawSphere(timeLine[i].init_pos, 0.1f, color);
		if(i + 1 < timeLine && isDrawLine)
		{
			Render().DrawBufferedLine(timeLine[i].init_pos, color, timeLine[i+1].init_pos, color);
		}
		if(isSel)
		{
			color |= 0xffff7f7f;
			Render().Print(timeLine[i].init_pos + Vector(0.0f, 0.2f, 0.0f), 10.0f, -2.0f, color, "%i", i);
			Render().Print(timeLine[i].init_pos + Vector(0.0f, 0.2f, 0.0f), 10.0f, -1.0f, color, typeName);
			Render().Print(timeLine[i].init_pos + Vector(0.0f, 0.2f, 0.0f), 10.0f, 0.0f, color, "\"%s\"", timeLine[i].linkName.c_str());
			Render().Print(timeLine[i].init_pos + Vector(0.0f, 0.2f, 0.0f), 10.0f, 1.0f, color, "%f sec", timeLine[i].time);
			
			if(drawCharacter && timeLine[i].isPreview)
			{				
				Matrix mtx(true);
				drawCharacter->DrawCharacterInPosition(timeLine[i].GetInitTransform(mtx));
			}
		}
	}
	Render().FlushBufferedLines();
	drawCharacter = null;
}

bool ActorTimeLine::IsTimeLineFinished(ActorData & data)
{
	long currentPointIndex = long(data.currentPosition);
	if (currentPointIndex >= timeLine) return true;

	long nextPointIndex = GetNextPoint(currentPointIndex);	
	if(nextPointIndex < 0 || nextPointIndex >= timeLine) return true;
	
	return false;
}

void   ActorTimeLine::GetTransform(Matrix& mat,ActorData & data,TimePoint& pt)
{
	pt.GetInitTransform(mat);
	//mat = pt.init_transform;
	if (teleportToChar) mat = mat * data.transform;
}

Vector ActorTimeLine::GetPos(ActorData & data,TimePoint& pt)
{
	if (!teleportToChar)
	{
		return pt.pos;
	}

	Matrix mat;
	GetTransform(mat,data,pt);
	
	static Vector pos;
	pos = mat.pos;	

	return pos;
}

float  ActorTimeLine::GetAy(ActorData & data,TimePoint& pt)
{
	if (!teleportToChar)
	{
		return pt.ay;
	}

	Matrix mat;
	GetTransform(mat,data,pt);

	static float ay;
	ay = mat.GetAngles().y;

	return ay;
}

//============================================================================================
//Параметры инициализации
//============================================================================================

const char * ActorTimeLine::comment = 
"Parameters for character's actor mode\n"
" \n"
" Modes:\n"
"    \"Wait time out\" - position no changes, play animation a set time,\n"
"                      after that teleport to next point\n"
"    \"Arrive at next point\" - position move by animation range, no time limited\n"
"                             activate next point when arrive at it\n"
"    \"Linear time move\" - linear move position by time\n"
"    \"Step by change node\" - move to next point when changing animation node\n"
"    \"Goto other point\" - move to set point when active goto flag else move to next point\n"
" \n"
" Commands:\n"
"    Enable index - enable goto for point of type \"Goto other point\"\n"
"    Disable index - disable goto for point of type \"Goto other point\"\n"
" \n"
" Additional:\n"
"    For access from timeline triggers to current character use object name \"*this*\""
" ";



MOP_BEGINLISTCG(ActorTimeLine, "Actor time line", '1.00', 99, ActorTimeLine::comment, "Character objects")
	MOP_ENUMBEG("MovieCtrMode")
		MOP_ENUMELEMENT("Wait time out")
		MOP_ENUMELEMENT("Arrive at next point")
		MOP_ENUMELEMENT("Linear time move")
		MOP_ENUMELEMENT("Step by change node")
		MOP_ENUMELEMENT("Goto other point")
	MOP_ENUMEND
	MOP_ARRAYBEGC("Time line", 1, 1000, "Array of action's times")
		MOP_STRINGC("Link", "", "Animation link name, first priority")	
		MOP_STRINGC("Node", "", "Animation node name, use if no link")
		MOP_FLOATEXC("Blend time", 0.2f, 0.0f, 100.0f, "Blend time for move from node to node withoput link")		
		MOP_ENUM("MovieCtrMode", "Mode")
		MOP_FLOATEX("Time", 3.0f, 0.01f, 1000000.0f)
		MOP_POSITION("Position", Vector(0.0f))
		MOP_ANGLESEX("Angle", Vector(0.0f), Vector(0.0f, -4.0f*PI, 0.0f), Vector(0.0f, 4.0f*PI, 0.0f))
		MOP_LONGEXC("Goto point", 0, 0, 1000, "Point index for \"goto\" mode")
		MOP_BOOLC("Is goto", true, "Goto on/off flag")
		MOP_BOOL("Model preview", false)
		MOP_MISSIONTRIGGER("")
	MOP_ARRAYEND
	MOP_POSITION("Track move", Vector(0.0f))
	MOP_ANGLES("Track rotate", Vector(0.0f))
	MOP_BOOLC("Move first point", false, "Move first time line point to character position")
	MOP_FLOATEXC("Orinet time to second point", 0.5f, 0.01f, 1000000.0f,"Time taken to orinet Actor to second point if first point was moved to character position")
	MOP_BOOLC("Teleport to char", false, "Move all points relative to char that first time line point was in character position")
	MOP_BOOLC("Debug draw", false, "Enable editor draw in game")
MOP_ENDLIST(ActorTimeLine)

