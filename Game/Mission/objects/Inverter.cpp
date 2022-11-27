//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Inverter	
//============================================================================================
			

#include "Inverter.h"



//============================================================================================

Inverter::Inverter() : objects(_FL_),
						commands(_FL_)
{
}

Inverter::~Inverter()
{
}


//============================================================================================


//Инициализировать объект
bool Inverter::Create(MOPReader & reader)
{
	objects.DelAll();
	commands.DelAll();
	isDown = false;
	target.Reset();
	ignoreDistance = 5.0f;
	if(!DetectorObject::Create(reader)) return false;	
	//Получим идентификатор объекта
	targetID = reader.String();
	//Получим позицию
	Vector pos = reader.Position();
	//Получим направление
	Vector ang = reader.Angles();
	mtx.Build(ang, pos);
	//Плоскость
	plane.normal = mtx.vz;
	plane.Move(pos);
	//Матрица преобразования
	imtx = mtx;
	imtx.Inverse();
	//Получим размеры
	width05 = reader.Float()*0.5f;
	height05 = reader.Float()*0.5f;
	//Состояние
	Activate(reader.Bool());
	long count = reader.Array();
	bool state = false;
	for(long i = 0; i < count; i++)
	{
		Element & elm = objects[objects.Add()];
		elm.pointer.Reset();
		elm.objectId = reader.String();
		elm.forward.changeShow = reader.TripleLogic(state);
		elm.forward.show = state;
		elm.back.changeShow = reader.TripleLogic(state);
		elm.back.show = state;
		elm.forward.changeActive = reader.TripleLogic(state);
		elm.forward.active = state;
		elm.back.changeActive = reader.TripleLogic(state);
		elm.back.active = state;
		elm.commandsIndex = commands;
		elm.commandsCount = reader.Array();
		commands.AddElements(elm.commandsCount*2);
		for(long j = 0; j < elm.commandsCount*2; j++)
		{
			Command & cmd = commands[elm.commandsIndex + j];
			cmd.command = reader.String().c_str();
			cmd.paramsCount = reader.Array();
			Assert(cmd.paramsCount <= 8);
			for(long k = 0; k < 8; k++)
			{
				if(k < cmd.paramsCount)
				{
					cmd.params[k] = reader.String().c_str();
				}else{
					cmd.params[k] = null;
				}				
			}
		}
		UpdateObject(elm);
	}
	//Сбрасывать ли состояние после срабатывания
	autoReset = true;
	//Одно или двух сторонний
	isDoubleSide = true;
	//
	ignoreDistance = reader.Float();
	//Ищим объект среди существующих
	return UpdateTarget();
}

//Активация детектора
void Inverter::ActivateDetector(const char * initiatorID)
{
	if(!EditMode_IsOn())
	{
		long isBack;
		if(plane*targetPos < 0.0f)
		{
			LogicDebug("Set forward states");
			isBack = 0;
		}else{
			LogicDebug("Set back states");
			isBack = 1;
		}
		for(long i = 0; i < objects; i++)
		{
			Element & elm = objects[i];
			if(UpdateObject(elm))
			{
				LittleTrigger & trg = isBack ? elm.back : elm.forward;
				if(trg.changeShow)
				{
					elm.pointer.Ptr()->Show(trg.show);
				}
				if(trg.changeActive)
				{
					elm.pointer.Ptr()->Activate(trg.active);
				}
				for(long j = 0; j < elm.commandsCount; j++)
				{
					Command & cmd = commands[elm.commandsIndex + j*2 + isBack];
					elm.pointer.Ptr()->Command(cmd.command, cmd.paramsCount, cmd.params);
				}
			}
		}	
	}
}

//Обновить цель
bool Inverter::UpdateObject(Element & elm)
{
	if(elm.pointer.Validate())
	{
		return true;
	}	
	return FindObject(elm.objectId, elm.pointer);
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(Inverter, "Inverter", '1.00', 0x0fffffff, "Detector invert states if mission object intersection it.", "Logic")
	MOP_STRING("Object id", "Player")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))
	MOP_FLOATEX("Width", 1.0f, 0.01f, 1000000.0f)
	MOP_FLOATEX("Height", 1.0f, 0.01f, 1000000.0f)
	MOP_BOOL("Active", true)
	MOP_ARRAYBEG("Objects", 0, 100)
		MOP_STRING("Object id", "")
		MOP_ENUMTL("Show forward")
		MOP_ENUMTL("Show back")
		MOP_ENUMTL("Activate forward")
		MOP_ENUMTL("Activate back")
		MOP_ARRAYBEG("Commands", 0, 100)
			MOP_STRINGC("Forward", "", "Forward direction command")
			MOP_ARRAYBEG("Forward params", 0, 8)
				MOP_STRING("Param", "")
			MOP_ARRAYEND
			MOP_STRINGC("Back", "", "Back direction command")
			MOP_ARRAYBEG("Back params", 0, 8)
				MOP_STRING("Param", "")
			MOP_ARRAYEND
		MOP_ARRAYEND
	MOP_ARRAYEND
	MOP_FLOATEXC("Ignore distance", 5.0f, 0.1f, 1000000.0f, "If length of target path per frame more then this distance, detector ignore that moving")
MOP_ENDLIST(Inverter)





