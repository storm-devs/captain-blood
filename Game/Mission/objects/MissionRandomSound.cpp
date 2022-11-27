

#include "MissionRandomSound.h"




MissionRandomSound::MissionRandomSound() : volumes(_FL_)
{	
}

MissionRandomSound::~MissionRandomSound()
{
}


//Инициализировать объект
bool MissionRandomSound::Create(MOPReader & reader)
{
	volumes.DelAll();
	volumes.AddElements(reader.Array());
	for(long i = 0; i < volumes; i++)
	{
		Vector pos = reader.Position();
		Vector ang = reader.Angles();
		volumes[i].mtx.Build(ang, pos);
		volumes[i].size = reader.Position();
	}
	return CreateSounds(reader, &Vector(0.0f));
}

//Инициализировать объект
bool MissionRandomSound::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&MissionRandomSound::EditModeDraw, ML_GEOMETRY5);
	Create(reader);
	return true;
}

//Обновить параметры
bool MissionRandomSound::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Рисование модельки в режиме редактирования
void _cdecl MissionRandomSound::EditModeDraw(float dltTime, long level)
{
	if(!EditMode_IsSelect()) return;
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	for(long i = 0; i < volumes; i++)
	{
		SoundVolume & sv = volumes[i];
		Render().DrawBox(-sv.size, sv.size, sv.mtx, 0xff00ffff);
	}
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		if(!se.sound) continue;
		if(se.states & s_stoped) continue;
		if(!se.sound->IsPlay()) continue;
		Render().DrawSphere(((ISound3D *)se.sound)->GetPosition(), 0.7f, 0xff0000ff);
	}
}

//Работа
void _cdecl MissionRandomSound::Work(float dltTime, long level)
{
	if(EditMode_IsOn() && !EditMode_IsSelect())
	{
		return;
	}
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		if(!se.sound) continue;
		if(se.states & s_stoped) continue;
		if(se.sound->IsPlay()) continue;
		se.waitTime -= dltTime;
		if(se.waitTime > 0.0f) continue;
		if((se.states & s_periodical) == 0)
		{
			se.states |= s_stoped;
		}
		se.waitTime = se.minTime + Rnd(se.maxTime - se.minTime);
		((ISound3D *)se.sound)->SetPosition(GetRandomPosition());
		se.sound->Play();
	}
}

//Найти произвольную позицию
inline Vector MissionRandomSound::GetRandomPosition()
{
	long index = (long)Rnd((float)volumes.Size());
	if(index >= volumes) index = volumes - 1;
	SoundVolume & sv = volumes[index];
	return sv.mtx*Vector().Rand(-sv.size, sv.size);
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(MissionRandomSound, "Random sound", '1.00', 100, "Sound play at random positions", "Effects")
	MOP_ARRAYBEG("Boxes", 1, 100)
		MOP_POSITION("Position", Vector(0.0f))
		MOP_ANGLES("Angles", Vector(0.0f))
		MOP_POSITIONEX("Size", Vector(1.0f), Vector(0.0f), Vector(1000.0f))
	MOP_ARRAYEND
	MISSION_SOUND_PARAMS
MOP_ENDLIST(MissionRandomSound)





