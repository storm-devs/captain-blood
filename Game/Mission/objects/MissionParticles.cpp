//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Mission objects
//============================================================================================
// MissionParticles	
//============================================================================================
			

#include "MissionParticles.h"


//============================================================================================

MissionParticles::MissionParticles()
{
	ps = null;
	model = null;
	noSwing = false;
}

MissionParticles::~MissionParticles()
{
	if(model) model->Release();
	if(ps) ps->Release();
}


//============================================================================================

//Инициализировать объект
bool MissionParticles::Create(MOPReader & reader)
{
	objectPtr.Reset();
	ConstString n = reader.String();
	if(name != n)
	{
		name = n;
		if(ps){ ps->Release(); ps = null; }
		ps = Particles().CreateParticleSystem(name.c_str());
	}	
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	float scale = reader.Float();
	float timeScale = reader.Float();
	matrix.Build(ang, pos);
	if(ps)
	{	
		ps->Teleport(matrix);
		ps->Restart(rand());		
		ps->AutoDelete(false);
		ps->SetScale(scale);
		ps->SetTimeScale(timeScale);
	}
	connectToObject = reader.String();	
	needUpdate = connectToObject.NotEmpty();
	needUpdate |= CreateSounds(reader, &pos);
	noSwing = reader.Bool();
	needUpdate |= noSwing;
	if(noSwing && ps)
	{
		ps->SetLocalMode(true);
	}
	return true;
}

//Показать/скрыть объект
void MissionParticles::Show(bool isShow)
{
	LogicDebug(isShow ? "Show" : "Hide");
	Activate(isShow);
	MissionSoundBase::Show(isShow);
}

//Активировать/деактивировать объект
void MissionParticles::Activate(bool isActive)
{
	MissionSoundBase::Activate(isActive);
	if(ps)
	{		
		ps->Restart(rand());
		ps->PauseEmission(!isActive);
	}
	if(needUpdate)
	{
		if(isActive)
		{
			SetUpdate(&MissionParticles::Work, ML_EXECUTE1);
		}else{
			DelUpdate(&MissionParticles::Work);
		}
	}else{
		DelUpdate(&MissionParticles::Work);
	}
}

#ifndef MIS_STOP_EDIT_FUNCS
//Перевести объект в спящий режим
void MissionParticles::EditMode_Sleep(bool isSleep)
{
	MissionObject::EditMode_Sleep(isSleep);
	if(EditMode_IsSleep() || !EditMode_IsVisible())
	{
		if(ps)
		{
			ps->Restart(rand());
			ps->PauseEmission(true);
		}
		bool curState = IsActive();
		Activate(false);
		MissionObject::Activate(curState);
	}else{
		Activate(IsActive());
	}
}


//Показать или скрыть объект в редакторе
void MissionParticles::EditMode_Visible(bool isVisible)
{
	MissionObject::EditMode_Visible(isVisible);
	EditMode_Sleep(EditMode_IsSleep());
}

#endif


//Получить матрицу объекта
Matrix & MissionParticles::GetMatrix(Matrix & mtx)
{
	if(connectToObject.NotEmpty())
	{
		if(objectPtr.Validate())
		{
			mtx = matrix*objectPtr.Ptr()->GetMatrix(Matrix());
			return mtx;
		}else{
			if(FindObject(connectToObject, objectPtr))
			{
				mtx = matrix*objectPtr.Ptr()->GetMatrix(Matrix());
				return mtx;
			}
		}
	}
	//Компенсируем свинг, если активна опция
	if(noSwing)
	{
		//Видовая матрица с включёной свинг матрицей
		Matrix view = Render().GetView();
		//Чистая видовая матрица
		Matrix realView = Mission().GetInverseSwingMatrix()*Matrix(view).Inverse();
		realView.Inverse();
		//Разносная матрица
		Matrix diff = Matrix(Matrix(realView), Matrix(view).Inverse());
		mtx.EqMultiply(matrix, diff);
		return mtx;
	}
	return mtx = matrix;
}

//Инициализировать объект
bool MissionParticles::EditMode_Create(MOPReader & reader)
{
	IGMXScene * m = model;
	model = Geometry().CreateGMX("editor\\particles.gmx", &Mission().Animation() , &Mission().Particles(), &Mission().Sound());
	if(m)
	{
		m->Release();
	}
	Create(reader);
	SetUpdate(&MissionParticles::EditModeWork, ML_GEOMETRY5);
	return true;
}

//Обновить параметры
bool MissionParticles::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void MissionParticles::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if(model)
	{
		min = model->GetLocalBound().vMin;
		max = model->GetLocalBound().vMax;
		return;
	}
	min = -0.2f;
	max = 0.2f;
}


//Работа партиклов в режиме игры
void _cdecl MissionParticles::Work(float dltTime, long level)
{
	Matrix mtx;
	GetMatrix(mtx);
	UpdatePosition(mtx.pos);
	if(ps)
	{
		ps->SetTransform(mtx);		
	}
	Update(dltTime);
}

//Работа детектора в режиме редактирования
void _cdecl MissionParticles::EditModeWork(float dltTime, long level)
{
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	if(ps && !ps->IsAlive() && EditMode_IsSelect()) ps->Restart(rand());
	Matrix mtx;	
	GetMatrix(mtx);
	if(model)
	{
		model->SetTransform(mtx);
		model->Draw();
	}else Render().DrawSphere(mtx.pos, 0.1f, ps ? 0x80808080 : 0x80800000);
	
}

//Обработчик команд для объекта
void MissionParticles::Command(const char * id, dword numParams, const char ** params)
{
	if(!id || !id[0]) return;

	if(string::IsEqual(id, "pause") && ps)
	{
		ps->PauseEmission(true);
		return;
	}

	if(string::IsEqual(id, "play") && ps)
	{
		ps->PauseEmission(false);
		return;
	}

	if(string::IsEqual(id, "teleport"))
	{
		if(numParams < 1) return;
		MOSafePointer obj;
		if(FindObject(ConstString(params[0]), obj))
		{
			//Получаем конечные матрицы
			Matrix mtx, mtxCur;
			GetMatrix(mtxCur);
			obj.Ptr()->GetMatrix(mtx);
			Matrix mobj(Matrix(matrix).Inverse(), mtxCur);
			matrix = mtx*mobj.Inverse();
			Work(0.0f, 0);
			LogicDebug("Teleport to mission object \"%s\"", obj.Ptr()->GetObjectID().c_str());
		}else{
			LogicDebugError("Can't to mission object \"%s\", object not found...", params[0]);
		}
	}else{
		LogicDebugError("Unknown command \"%s\"", id);
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================


const char * MissionParticles::comment =
"Representation particle system in mission\n"
" \n"
"Commands list:\n"
"----------------------------------------\n"
"  Teleport geometry to position of some\n"
"  mission object\n"
"----------------------------------------\n"
"    command: teleport\n"
"    parm: name of mission object\n"
" \n"
"    command: pause\n"
" \n"
"    command: play\n"
" \n"
" ";


MOP_BEGINLISTCG(MissionParticles, "Particles system", '1.00', 0x0fffffff, MissionParticles::comment, "Effects")
	MOP_STRING("Paticles name", "particles")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))
	MOP_FLOATEX("Scale", 1.0f, 0.001f, 10000000.0f)
	MOP_FLOATEX("Time scale", 1.0f, 0.001f, 10000000.0f)
	MOP_STRING("Connect to object", "")
	MISSION_SOUND_PARAMS
	MOP_BOOLC("No swing", false, "No swing particles in swing machine")
MOP_ENDLIST(MissionParticles)
