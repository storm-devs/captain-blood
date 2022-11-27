//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Mission objects
//===========================================================================================================================
// Mission3DSound
//===========================================================================================================================

#include "Mission3DSound.h"

//============================================================================================
//Mission3DSound
//============================================================================================

Mission3DSound::Mission3DSound()
{
	model = null;
}

Mission3DSound::~Mission3DSound()
{
	if(model) model->Release();
}


//============================================================================================

//Инициализировать объект
bool Mission3DSound::Create(MOPReader & reader)
{
	position = reader.Position();
	return CreateSounds(reader, &position);
}

//Получить матрицу объекта
Matrix & Mission3DSound::GetMatrix(Matrix & mtx)
{
	mtx.SetIdentity().pos = position;
	return mtx;
}

//Обработчик команд для объекта
void Mission3DSound::Command(const char * id, dword numParams, const char ** params)
{
	if(string::IsEqual(id, "volume"))
	{
		if(numParams >= 1)
		{
			char * endPtr = null;
			float vol = (float)strtod(params[0], &endPtr);
			UpdateVolumes(Clampf(vol));
		}else{
			LogicDebugError("Can't set new sound volume. Not enought params.");
		}
	}else{
		LogicDebugError("Unknown command: \"%s\"", id);
	}
}

//Инициализировать объект
bool Mission3DSound::EditMode_Create(MOPReader & reader)
{
	//Создаём модельку
	angle = 0.0f;
	model = Geometry().CreateGMX("editor\\sound3d.gmx", &Animation(), &Particles(), &Sound());
	SetUpdate(&Mission3DSound::EditModeDraw, ML_GEOMETRY5);
	Create(reader);
	return true;
}

//Обновить параметры
bool Mission3DSound::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void Mission3DSound::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if(model)
	{
		Vector c = (model->GetLocalBound().vMin + model->GetLocalBound().vMax)*0.5f;
		Vector s = (model->GetLocalBound().vMax - model->GetLocalBound().vMin)*0.5f;
		s.x = s.z = coremax(s.x, s.z);
		min = c - s;
		max = c + s;
		return;
	}
	min = -0.2f;
	max = 0.2f;
}

//Рисование модельки в режиме редактирования
void _cdecl Mission3DSound::EditModeDraw(float dltTime, long level)
{
	Matrix mtx;
	if(model)
	{
		angle += dltTime*0.5f;
		if(angle > 2.0f*PI) angle -= 2.0f*PI;
		model->SetTransform(mtx.Build(Vector(0.0f, angle, 0.0f), position));
		model->Draw();
	}else Render().DrawSphere(GetMatrix(mtx).pos, 0.1f, 0xff707080);
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(Mission3DSound, "3D sound", '1.00', 100, "Position sound\nCommands: volume (0..1)", "Effects")
	MOP_POSITION("Position", Vector(0.0f))
	MISSION_SOUND_PARAMS
MOP_ENDLIST(Mission3DSound)


