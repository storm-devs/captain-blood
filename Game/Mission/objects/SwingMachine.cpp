//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// SwingMachine	
//============================================================================================
			

#include "SwingMachine.h"

//============================================================================================

SwingMachine::SwingMachine()
{
	angX = angZ = 0.0f;
	spdX = spdZ = 0.0f;
	ampX = ampZ = 0.0f;
}

SwingMachine::~SwingMachine()
{
	IMission * mis = &Mission();
	if(mis) mis->SetSwingParams(Vector(0.0f));
}

//============================================================================================

//Инициализировать объект
bool SwingMachine::Create(MOPReader & reader)
{
	spdX = reader.Float()*(PI/180.0f);
	spdZ = reader.Float()*(PI/180.0f);
	ampX = reader.Float()*(PI/180.0f);
	ampZ = reader.Float()*(PI/180.0f);
	matrix.pos = reader.Position();
	isGlobalSet = reader.Bool();
	Activate(reader.Bool());
	return true;
}

//Активировать/деактивировать объект
void SwingMachine::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(!EditMode_IsOn())
	{
		if(IsActive())
		{
			SetUpdate(&SwingMachine::Work, ML_SCENE_START - 1);
		}else{
			DelUpdate(&SwingMachine::Work);
			if(isGlobalSet)
			{
				Mission().SetSwingParams(Vector(0.0f));
			}else{
				matrix.SetIdentity3x3();
			}			
		}
	}
}

void _cdecl SwingMachine::Work(float dltTime, long level)
{
	angX += spdX*dltTime;
	angZ += spdZ*dltTime;
	if(angX > PI*2.0f) angX -= PI*2.0f;
	if(angZ > PI*2.0f) angZ -= PI*2.0f;
	Vector ang(ampX*sinf(angX), 0.0f, ampZ*sinf(angZ));
	if(isGlobalSet)
	{
		Mission().SetSwingParams(ang);
	}else{
		matrix.Build(ang, matrix.pos);
	}
}


//Получить матрицу объекта
Matrix & SwingMachine::GetMatrix(Matrix & mtx)
{
	if(isGlobalSet)
	{
		mtx.SetIdentity();
		return mtx;
	}
	return mtx = matrix;
}


//============================================================================================

//Инициализировать объект
bool SwingMachine::EditMode_Create(MOPReader & reader)
{
	Create(reader);
	SetUpdate(&SwingMachine::EditMode_Work, ML_GEOMETRY1);
	return true;
}

//Инициализировать объект
bool SwingMachine::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

void _cdecl SwingMachine::EditMode_Work(float dltTime, long level)
{
	if(!EditMode_IsSelect())
	{
		Mission().SetSwingParams(Vector(0.0f));
	}else{
		Work(dltTime, level);
	}
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(SwingMachine, "Swing machine", '1.00', 0x0, "Swing world", "Effects")
	MOP_FLOATEXC("Swing temp around X axis", 20.0f, 0.0f, 10000.0f, "Angle_X = Amplitude_X*sin(time*#bTemp_X@b), #bTemp_X@b in degrees per second")
	MOP_FLOATEXC("Swing temp around Z axis", 15.0f, 0.0f, 10000.0f, "Angle_Z = Amplitude_Z*sin(time*#bTemp_Z@b), #bTemp_Z@b in degrees per second")
	MOP_FLOATEXC("Swing amplitude by X axis", 2.0f, 0.0f, 10000.0f, "Angle_X = #bAmplitude_X@b*sin(time*temp), #bAmplitude_X@b in degrees")
	MOP_FLOATEXC("Swing amplitude by Z axis", 6.0f, 0.0f, 10000.0f, "Angle_Z = #bAmplitude_Z@b*sin(time*temp), #bAmplitude_Z@b in degrees")
	MOP_POSITIONC("Local position", Vector(0.0f), "Position for local mode");
	MOP_BOOLC("Global swing", true, "Swing all mission or just connection objects")
	MOP_BOOLC("Active", true, "Active object in start mission time")	
MOP_ENDLIST(SwingMachine)

