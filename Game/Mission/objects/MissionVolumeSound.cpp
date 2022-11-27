

#include "MissionVolumeSound.h"



MissionVolumeSound::MissionVolumeSound()
{
	radius = 0.0f;
	fadeDist2 = 0.0f;
	kFadeDist = 0.0f;
	flags = f_init;
	debug = null;
}

MissionVolumeSound::~MissionVolumeSound()
{
	DELETE(debug);
}

//Инициализировать объект
bool MissionVolumeSound::Create(MOPReader & reader)
{
	flags = f_init;
	const char * figure = reader.Enum().c_str();
	Vector pos = reader.Position();
	connectToObject = reader.String();
	Vector ang = reader.Angles();	
	size = reader.Position()*0.5f;
	radius = reader.Float();
	if(figure[0] == 'B' || figure[0] == 'b')
	{
		matrix.Build(ang, pos);	
		radius = -1.0f;
	}else{
		matrix.SetIdentity();
		matrix.pos = pos;
		size = radius;
		flags |= f_isSphere;
	}
	bool is3D = (reader.Enum().c_str()[0] == '3');
	fadeDist2 = reader.Float();	
	if(reader.Bool()) flags |= f_isManageByDist;
	if(is3D)
	{
		flags |= f_is3D;
		fadeDist2 *= fadeDist2;
	}else{
		Assert(fadeDist2 > 0.0f);
		kFadeDist = 1.0f/fadeDist2;
		fadeDist2 *= fadeDist2;
		flags |= f_isManageByDist;
	}
	bool res = CreateSounds(reader, is3D ? &matrix.pos : null);
	if(reader.Bool() && !EditMode_IsOn())
	{
		debug = NEW Debug();
		memset(debug, 0, sizeof(Debug));
		SetUpdate(&MissionVolumeSound::EditModeDraw, ML_POSTEFFECTS);
	}else{
		DELETE(debug);
	}
	return res;
}

//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
void MissionVolumeSound::PostCreate()
{
	FindObject(connectToObject, objectPtr);
}

//Получить матрицу объекта
Matrix & MissionVolumeSound::GetMatrix(Matrix & mtx)
{
	Sound().GetListenerMatrix(mtx);
	Vector pos = mtx.pos;
	MissionObject * cmo = objectPtr.Ptr();
	if((flags & f_isSphere) == 0)
	{		
		if(!cmo)
		{
			mtx = matrix;
		}else{
			cmo->GetMatrix(mtx);
			mtx = Matrix(matrix, mtx);
		}
		Vector local = mtx.MulVertexByInverse(pos);
		local.Max(local, -size);
		local.Min(local, size);
		mtx.pos = mtx.MulVertex(local);
	}else{
		Vector dir = mtx.pos - matrix.pos;
		if(cmo)
		{
			cmo->GetMatrix(mtx);
			dir -= mtx.pos;
		}		
		dir.ClampLength(radius);
		mtx = matrix;
		mtx.pos += dir;
	}
	return mtx;
}

//Инициализировать объект
bool MissionVolumeSound::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&MissionVolumeSound::EditModeDraw, ML_POSTEFFECTS - 1);
	Create(reader);
	return true;
}

//Обновить параметры
bool MissionVolumeSound::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void MissionVolumeSound::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	Matrix mtx;
	MissionObject * cmo = null;
	if(objectPtr.Validate())
	{
		cmo = objectPtr.Ptr();
	}	
	if(cmo)
	{
		cmo->GetMatrix(mtx);
		mtx = Matrix(matrix, mtx);
	}else{
		mtx = matrix;
	}
	Matrix m;
	GetMatrix(m);
	min = m.MulVertexByInverse(-size*mtx);
	max = m.MulVertexByInverse(size*mtx);

}

//Рисование модельки в режиме редактирования
void _cdecl MissionVolumeSound::EditModeDraw(float dltTime, long level)
{
	if(EditMode_IsOn())
	{
		if(!EditMode_IsSelect()) return;
		if(!EditMode_IsVisible()) return;
	}
	objectPtr.Reset();
	if(connectToObject.NotEmpty())
	{
		FindObject(connectToObject, objectPtr);
	}
	Matrix mtx;
	bool isConnectActive = true;
	if(!objectPtr.Ptr())
	{
		mtx = matrix;
	}else{
		objectPtr.Ptr()->GetMatrix(mtx);
		mtx = Matrix(matrix, mtx);
		isConnectActive = objectPtr.Ptr()->IsShow();
	}
	if(flags & f_isSphere)
	{
		Render().DrawSphereGizmo(mtx.pos, radius, 0xff8080ff, 0xff8080ff);
		if(flags & f_isManageByDist)
		{
			Render().DrawSphereGizmo(mtx.pos, radius + sqrtf(fadeDist2), 0xff808080, 0xff808080);
		}		
	}else{
		if(!EditMode_IsOn())
		{
			Render().DrawBox(-size, size, mtx, 0xff8080ff);
		}
		if(flags & f_isManageByDist)
		{
			float addSize = sqrtf(fadeDist2);
			Render().DrawBox(-size - Vector(addSize), size + Vector(addSize), mtx, 0xff8080ff);
		}
	}
	if(IsActive())
	{
		Vector pos = GetMatrix(Matrix()).pos;
		dword color = 0xff6060ff;
		if(debug)
		{
			pos = debug->sndPos;
			if(isConnectActive)
			{
				Render().DrawLine(debug->sndPos, color, debug->listPos, 0xffffffff);
			}
			if(debug->curVolume <= 1e-6f) color = 0xff606060;
		}
		Render().DrawSphere(pos, 0.2f, color);
		Render().Print(pos, 1000.0f, -1.1f, 0xffc0c0ff, "%s", GetObjectID().c_str());
		if(debug)
		{
			if(isConnectActive)
			{
				Render().Print(pos, 1000.0f, 0.0f, 0xffc0c0ff, "dist = %f", debug->dist);
				if(fadeDist2 > 0.0f)
				{
					Render().Print(pos, 1000.0f, 1.1f, 0xffc0c0ff, "vol = %f", debug->curVolume);
				}
			}else{
				Render().Print(pos, 1000.0f, 0.0f, 0xffc0c0c0, "Parent object: %s isn't visible", connectToObject.c_str());
			}
		}
	}else{
		Render().Print(mtx.pos, 1000.0f, -0.6f, 0xffc0c0ff, "%s", GetObjectID().c_str());
		Render().Print(mtx.pos, 1000.0f, 0.6f, 0xffc0c0ff, "not active");
	}
}

//Работа
void _cdecl MissionVolumeSound::Work(float dltTime, long level)
{
	if(objectPtr.Ptr())
	{
		if(!objectPtr.Ptr()->IsShow())
		{
			Fadeout();
			return;
		}
	}
	Vector pos = GetMatrix(Matrix()).pos;
	if((flags & f_isManageByDist) == 0)
	{
		UpdatePosition(pos);
		Update(dltTime);
		if(debug)
		{
			Matrix mtx;
			Sound().GetListenerMatrix(mtx);
			float dist2 = ~(mtx.pos - pos);
			debug->curVolume = 0.0f;
			debug->dist = sqrtf(dist2);
			debug->sndPos = pos;
			debug->listPos = mtx.pos;
		}
	}else{		
		Matrix mtx;
		Sound().GetListenerMatrix(mtx);
		float dist2 = ~(mtx.pos - pos);
		float vol = 0.0f;
		if(dist2 < fadeDist2)
		{
			if((flags & f_is3D) == 0)
			{
				vol = 1.0f - sqrtf(dist2)*kFadeDist;
				vol *= vol;
				UpdateVolumes(vol);
			}else{
				UpdatePosition(pos);
			}
			Update(dltTime);
		}else{
			UpdatePosition(pos);
			Fadeout();
		}
		if(debug)
		{
			debug->curVolume = vol;
			debug->dist = sqrtf(dist2);
			debug->sndPos = pos;
			debug->listPos = mtx.pos;
		}
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTG(MissionVolumeSound, "Volume sound", '1.00', 100, "Effects")
	MOP_ENUMBEG("Geometry")
		MOP_ENUMELEMENT("Box")
		MOP_ENUMELEMENT("Shpere")
	MOP_ENUMEND
	MOP_ENUMBEG("Mode")
		MOP_ENUMELEMENT("3D")
		MOP_ENUMELEMENT("stereo")
	MOP_ENUMEND

	MOP_ENUM("Geometry", "Figure")
	MOP_POSITIONC("Position", Vector(0.0f), "Figuge position in world")
	MOP_STRING("Connect to object", "")
	MOP_GROUPBEG("Box params")
		MOP_ANGLESC("Angles", Vector(0.0f), "Box orientation in world")
		MOP_POSITIONEXC("Size", Vector(1.0f), Vector(0.1f), Vector(10000.0f), "Box size")
	MOP_GROUPEND()
	MOP_GROUPBEG("Shpere params")
		MOP_FLOATEXC("Radius", 1.0f, 0.1f, 10000.0f, "Shpere radius")
	MOP_GROUPEND()
	MOP_ENUM("Mode", "Sound is")	
	MOP_FLOATEXC("Fade distance", 10.0, 0.1f, 1000.0f, "Inside box stereo sound play with maximem volume,\noutside box volume of stereo sound fade in that distance")
	MOP_BOOLC("Manage 3D", false, "Manage 3D sound by fade distance")
	MISSION_SOUND_PARAMS
	MOP_BOOL("Debug", false)
MOP_ENDLIST(MissionVolumeSound)



