/*

//============================================================================================
// Spirenkov Maxim
//============================================================================================
// AniCamera	
//============================================================================================
			

#include "AniCamera.h"
#include "..\..\..\AnimationService\anxeditor\ant.h"

//============================================================================================

AniCamera::AniCamera() : events(_FL_)
{
	fov = 1.0f + PI*0.5f;
	model = null;
	isPreview = false;


	fNeededDist=2.5f;
	fMinAngle=10;
	fMaxAngle=25;

	fMinSpeedAngle=80;
	fMaxSpeedAngle=120;

	AutoZoom = false;

	track = null;
	numPoints = 0;
	curPosition = 0.0f;
	speed = 0.0f;
	teleDist = 0.0f;
	teleCosAng = 0.0f;

	init_fov = fov;
}

AniCamera::~AniCamera()
{
	if(track) delete track;
	target.Reset();
	DelUpdate(&AniCamera::DrawFrame);
}

void AniCamera::InitParams()
{
	if(track) delete track;
	track = null;

	numPoints = 0;
	curPosition = 0.0f;
	speed = 0.0f;
	teleDist = 0.0f;	
	teleCosAng = 0.0f;
	
	BaseCamera::InitParams();
}

//============================================================================================

//Получить позицию камеры
void AniCamera::GetCameraPosition(Vector & position)
{
	if(track)
	{
		long p1, p2;
		float k = GetCurrentPosition(p1, p2);
		position.Lerp(track[p1].p, track[p2].p, k);
	}else{
		position = 0.0f;
	}
}

//Получить точку наблюдения камеры
bool AniCamera::GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up)
{
	if(BaseCamera::GetCameraTarget(position, target_pos, up)) return true;
	if(track)
	{
		long p1, p2;
		float k = GetCurrentPosition(p1, p2);
		Quaternion ang;
		ang.SLerp(track[p1].q, track[p2].q, k);
		Matrix camRot;
		ang.GetMatrix(camRot);
		camRot.vz = -camRot.vz;		
		target_pos = position + offset_matrix.MulNormal(camRot.vz);
		up = camRot.vy;
		return true;
	}
	target_pos = position + Vector(0.0f, 0.0f, 1.0f);
	return false;
}


//Получить текущую позицию камеру
inline float AniCamera::GetCurrentPosition(long & i1, long & i2)
{
	Assert(track);
	float p = curPosition;
	if(p < 0.0f) p = 0.0f;
	if(p > float(numPoints - 1)) p = float(numPoints - 1);
	i1 = long(p);
	i2 = i1 + 1;
	if(i2 > numPoints - 1) i2 = numPoints - 1;
	float k = p - i1;
	if(teleDist > 0.0f)
	{
		if(~(track[i1].p - track[i2].p) > teleDist*teleDist)
		{
			if(k < 0.5f)
			{
				k = 0.0f;
			}else{
				k = 1.0f;
			}
		}
	}
	if(teleCosAng > -5.0f)
	{
		Matrix m1, m2;
		track[i1].q.GetMatrix(m1);
		track[i2].q.GetMatrix(m2);
		if((m1.vz | m2.vz) < teleCosAng)
		{
			if(k < 0.5f)
			{
				k = 0.0f;
			}else{
				k = 1.0f;
			}
		}
	}
	return k;
}

//Прочитать параметры идущие перед стандартным
bool AniCamera::CameraPreCreate(MOPReader & reader)
{
	//Очищаем данные
	event.Empty();
	//Зачитаем трек
	string path = "Resource\\Cameras\\";
	path += reader.String().c_str();
	//path.AddExtention(".ant");
	
	if(track) delete track;
	
	track = null;
	numPoints = 0;
	
	if(!LoadTrackANT(path))
	{
		//if(!LoadTrackCTF(path))
		{		
			delete track;
			track = null;
		}
	}

	//Позиция трека
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	world.Build(ang, pos);


	Matrix mTemp;
	for (long n = 0; n < numPoints; n++)
	{
		track[n].p = track[n].p * world;

		
		track[n].q.GetMatrix(mTemp);
		mTemp = mTemp * world;

		track[n].q.Set(mTemp);
		
	}


	//Скорость
	speed = reader.Float();
	//Пороговая дистанция телепорта
	teleDist = reader.Float();
	//Пороговый угол телепорта
	teleCosAng = reader.Float();
	if(teleCosAng > 0.0f)
	{
		teleCosAng = cosf(Deg2Rad(teleCosAng));
	}else{
		teleCosAng = -10.0f;
	}
	return true;
}

//Прочитать параметры идущие после стандартных
bool AniCamera::CameraPostCreate(MOPReader & reader)
{
	//Событие
	event.Init(reader);
	long count = reader.Array();
	events.AddElements(count);
	for(long i = 0; i < count; i++)
	{
		events[i].frame = reader.Long();
		events[i].trigger.Init(reader);
	}	
	if(reader.Bool())
	{
		SetUpdate(&AniCamera::DrawFrame, ML_GUI5);
	}
	return track != null;
}


//Сбросить состояние камеры в исходное
void AniCamera::Reset()
{
	BaseCamera::Reset();
	
	curPosition = 0.0f;
	event.Reset();
}

//Обновить состояние камеры
void AniCamera::WorkUpdate(float dltTime)
{
	//Двигаемся
	long prev = long(curPosition);
	curPosition += speed*dltTime;
	bool isEnd = false;
	if(curPosition >= float(numPoints - 1))
	{
		isEnd = true;
		curPosition = float(numPoints - 1);
	}
	long next = long(curPosition);
	if(prev != next)
	{
		for(long i = prev + 1; i <= next; i++)
		{
			for(long j = 0; j < events; j++)
			{
				if(events[j].frame == i)
				{
					events[j].trigger.Activate(Mission(), false);
				}
			}
		}
	}
	if(isEnd)
	{
		Activate(false);
		event.Activate(Mission());
	}
}

//Обновить состояние для предпросмотра
void AniCamera::SelectedUpdate(float dltTime)
{
	curPosition += speed*dltTime;
	if(curPosition >= float(numPoints - 1))
	{
		curPosition = 0;
	}
}


//Нарисовать дополнительную информацию нри селекте
void AniCamera::SelectedDraw(float dltTime)
{
	Render().FlushBufferedLines();
	
	for(long i = 0; i < numPoints - 1; i++)
	{
		Render().DrawBufferedLine(track[i].p, 0xffff0000, track[i + 1].p, 0xffffff00, false);
	}
	
	Render().FlushBufferedLines();
	
	BaseCamera::SelectedDraw(dltTime);
}

/*
//Загрузить трек
inline bool AniCamera::LoadTrackCTF(const char * name)
{
	//Файловый сервис
	IFileService * fileServ = (IFileService *)api->GetService("FileService");
	if(!fileServ)
	{
		api->Trace("FileService not created!");
		return false;
	}
	//Загружаем файл в память
	Assert(track == null);
	Assert(numPoints == 0);
	void * data = null;
	dword size = 0;
	data = fileServ->Load(name, &size);
	if(!data)
	{
		api->Trace("Camera tracks file %s not loaded...", name);
		return false;
	}
	//Проверяем заголовок
	long nPoints = ((CameraTracksHeader *)data)->numPoints;
	if(((CameraTracksHeader *)data)->id != CameraTracksID || nPoints == 0)
	{
		api->Trace("Camera tracks file %s is invalidate...", name);
		delete data;
		return false;
	}
	if(((CameraTracksHeader *)data)->ver != CameraTracksVer)
	{
		api->Trace("Camera tracks file %s have incorrect version...", name);
		delete data;
		return false;
	}
	//Проверяем размеры файла
	if(size < sizeof(CameraTracksHeader) + nPoints*sizeof(CameraTracksPoint))
	{
		api->Trace("Camera tracks file %s is invalidate...", name);
		delete data;
		return false;
	}
	//Сохраняем данные
	CameraTracksPoint * trk = (CameraTracksPoint *)((byte *)data + sizeof(CameraTracksHeader));
	numPoints = nPoints;
	track = NEW Point[numPoints];
	for(long i = 0; i < numPoints; i++)
	{
		track[i].q.x = trk[i].qx;
		track[i].q.y = trk[i].qy;
		track[i].q.z = trk[i].qz;
		track[i].q.w = trk[i].qw;
		track[i].p.x = trk[i].px;
		track[i].p.y = trk[i].py;
		track[i].p.z = trk[i].pz;
	}
	
	delete data;
	return true;
}*/
/*
inline bool AniCamera::LoadTrackANT(const char * name)
{
	ILoadBuffer * loadBuffer = Files().LoadData(name, _FL_);

	if (!loadBuffer)
	{
		api->Trace("Camera tracks file %s not loaded...", name);
		return false;
	}

	AntFileHeader* pHeader;

	pHeader = (AntFileHeader*)loadBuffer->Buffer();
	XSwizzleULong(pHeader->id);
	XSwizzleULong(pHeader->ver);
	XSwizzleULong(pHeader->bonesCount);
	XSwizzleULong(pHeader->framesCount);
	XSwizzleULong(pHeader->stringsTableSize);
	XSwizzleFloat(pHeader->defaultFPS);

	//Проверяем заголовок		
	if (pHeader->id != ANTFILE_ID || pHeader->bonesCount == 0 ||
		pHeader->framesCount == 0)
	{
		api->Trace("Camera tracks file %s is invalidate...", name);
		loadBuffer->Release();

		return false;
	}

	if (pHeader->ver != ANTFILE_VER)
	{
		api->Trace("Camera tracks file %s have incorrect version...", name);
		loadBuffer->Release();

		return false;
	}

	AntFileBone * pBoneTable = (AntFileBone*)(loadBuffer->Buffer() + sizeof(AntFileHeader) + pHeader->stringsTableSize);

	AntFileTrackElement * pFramesTable = (AntFileTrackElement*) (loadBuffer->Buffer() + sizeof(AntFileHeader) +
		pHeader->stringsTableSize + pHeader->bonesCount*sizeof(AntFileBone));

	int iBoneCameraIndex=-1;

	for (unsigned int i=0;i<pHeader->bonesCount;i++)
	{
		const char* BoneName = (const char*)loadBuffer->Buffer() + sizeof(AntFileHeader) + SwizzleDWord(pBoneTable[i].nameIndex);

		if (string::EqualPostfix(BoneName,"camera"))
		{
			iBoneCameraIndex = i;
		}
	}

	if (iBoneCameraIndex == -1)
	{
		api->Trace("Can't find Camera Bone in ant file %s", name);
		loadBuffer->Release();
		return false;
	}


	numPoints = pHeader->framesCount;
	track = NEW Point[numPoints];

	for(long i = 0; i < numPoints; i++)
	{
		AntFileTrackElement & el = pFramesTable[i+iBoneCameraIndex*pHeader->framesCount];

		track[i].p.x = SwizzleFloat(el.px);
		track[i].p.y = SwizzleFloat(el.py);
		track[i].p.z = SwizzleFloat(el.pz);

		track[i].q.x = SwizzleFloat(el.qx);
		track[i].q.y = SwizzleFloat(el.qy);
		track[i].q.z = SwizzleFloat(el.qz);
		track[i].q.w = SwizzleFloat(el.qw);
	}	

	loadBuffer->Release();

	return true;
}


//Нарисовать дополнительную информацию нри селекте
void AniCamera::DrawFrame()
{
	IFont * font = Render().GetSystemFont();
	if(!font) return;
	long i1, i2;
	GetCurrentPosition(i1, i2);
	float h = font->GetHeight();
	font->SetHeight(100.0f);
	font->Print(20.0f, 100.0f, "%i", i1);
	font->SetHeight(h);
	font->Release();
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(AniCamera, "Animation camera", '1.00', 1000, CAMERA_COMMENT("Animation camera\n\n"), "Cameras")
	MOP_STRINGC("Camera track file", "Camera", "File whith track data (*.ant, *.ctf)")
	MOP_POSITIONC("Camera track position", Vector(0.0f), "Change track positions")
	MOP_ANGLESC("Camera track angles", Vector(0.0f), "Rotate track")
	MOP_FLOATEXC("Camera move speed", 10.0f, 0.0001f, 1000.0f, "Frames per second")
	MOP_FLOATEXC("Teleport distance", 0.0f, 0.0f, 100.0f, "If 0, then not work. Else if step distance above it, camera is teleported to next point")
	MOP_FLOATEXC("Teleport angle", 0.0f, 0.0f, 180.0f, "If 0, then not work. Else if step angle above it, camera is teleported to next point")
	MOP_CAM_STD
	MOP_MISSIONTRIGGER("")
	MOP_ARRAYBEG("Frame triggers", 0, 100)
		MOP_LONGEX("Frame", 0, 0, 100000)
		MOP_MISSIONTRIGGER("")
	MOP_ARRAYEND
	MOP_BOOL("Show frame", false)
MOP_ENDLIST(AniCamera)

*/