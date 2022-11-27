//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// PathTracks	
//============================================================================================
			

#include "PathTracks.h"
//#include "..\..\..\AnimationService\anxeditor\ant.h"

PathTracks::PathTracks(): points(_FL_)
{
	points.Empty();
	Offset = 0.0f;
}

PathTracks::~PathTracks()
{
	ClearTrack();
}

/*
//Загрузить трек в PathTracks
bool PathTracks::LoadCTF(const char * fileName)
{
	ClearTrack();

	//Загружаем файл в память
	void * data = null;
	dword size = 0;
	IFileService * fileServ = (IFileService *)api->GetService("FileService");
	
	if(!fileServ)
	{
		api->Trace("FileService not created!");
		return false;
	}
	
	data = fileServ->Load(fileName, &size);
	if(!data)
	{
		api->Trace("Camera tracks file %s not loaded...", fileName);
		return false;
	}
	
	//Проверяем заголовок
	long nPoints = ((CameraTracksHeader *)data)->numPoints;
	
	if(((CameraTracksHeader *)data)->id != CameraTracksID || nPoints == 0)
	{
		api->Trace("Camera tracks file %s is invalidate...", fileName);
		delete data;
		return false;
	}
	
	if(((CameraTracksHeader *)data)->ver != CameraTracksVer)
	{
		api->Trace("Camera tracks file %s have incorrect version...", fileName);
		delete data;
		return false;
	}
	
	//Проверяем размеры файла
	if(size < sizeof(CameraTracksHeader) + nPoints*sizeof(CameraTracksPoint))
	{
		api->Trace("Camera tracks file %s is invalidate...", fileName);
		delete data;
		return false;
	}
	
	CameraTracksPoint* TracksPoints = (CameraTracksPoint*)((byte *)data + sizeof(CameraTracksHeader));
		
	for(long i = 0; i < nPoints; i++)
	{
		TPoint pt;

		
		pt.p1.x=TracksPoints[i].p1x;
		pt.p1.y=TracksPoints[i].p1y;
		pt.p1.z=TracksPoints[i].p1z;

		pt.p2.x=TracksPoints[i].p2x;
		pt.p2.y=TracksPoints[i].p2y;
		pt.p2.z=TracksPoints[i].p2z;

		pt.p.x=TracksPoints[i].px;
		pt.p.y=TracksPoints[i].py;
		pt.p.z=TracksPoints[i].pz;

		pt.q.x=TracksPoints[i].qx;
		pt.q.y=TracksPoints[i].qy;
		pt.q.z=TracksPoints[i].qz;
		pt.q.w=TracksPoints[i].qw;

		AddPoint(pt);
	}

	delete data;

	return true;
}*/
/*
//Загрузить трек в PathTracks
bool PathTracks::LoadANT(IFileService * fs, const char * fileName)
{
	ClearTrack();

	Assert(fs);
	ILoadBuffer * loadBuffer = fs->LoadData(fileName, _FL_);

	if (!loadBuffer)
	{
		api->Trace("Camera tracks file %s not loaded...", fileName);
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
		api->Trace("Camera tracks file %s is invalidate...", fileName);
		loadBuffer->Release();

		return false;
	}

	if (pHeader->ver != ANTFILE_VER)
	{
		api->Trace("Camera tracks file %s have incorrect version...", fileName);
		loadBuffer->Release();

		return false;
	}

	AntFileBone * pBoneTable = (AntFileBone*)((byte*)loadBuffer->Buffer() + sizeof(AntFileHeader) + pHeader->stringsTableSize);

	AntFileTrackElement * pFramesTable = (AntFileTrackElement*) ((byte*)loadBuffer->Buffer() + sizeof(AntFileHeader) +
		pHeader->stringsTableSize + pHeader->bonesCount*sizeof(AntFileBone));


	int iBoneCharacter1Index=-1;
	int iBoneCharacter2Index=-1;
	int iBoneCameraIndex=-1;
	int iBoneCameraIndex2=-1;

	string str;

	for (unsigned int i=0;i<pHeader->bonesCount;i++)
	{
		byte* pNamePointer = (byte*)loadBuffer->Buffer() + sizeof(AntFileHeader) + SwizzleDWord(pBoneTable[i].nameIndex);

		char* BoneName = (char*)pNamePointer;

		if (str.EqualPostfix(BoneName,"character1"))
		{
			iBoneCharacter1Index = i;
		}
		else
			if (str.EqualPostfix(BoneName,"character2"))
			{
				iBoneCharacter2Index = i;
			}
			else
				if (str.EqualPostfix(BoneName,"camera"))
				{
					iBoneCameraIndex = i;
				}
				else
					if (str.EqualPostfix(BoneName,"camera1"))
					{
						iBoneCameraIndex2 = i;
					}
	}

	if (iBoneCharacter1Index == -1)
	{
		api->Trace("Can't find Character1 Bone", fileName);
		loadBuffer->Release();
		return false;
	}

	if (iBoneCharacter2Index == -1)
	{
		api->Trace("Can't find Character2 Bone", fileName);
		loadBuffer->Release();
		return false;
	}

	if (iBoneCameraIndex == -1)
	{
		api->Trace("Can't find Camera Bone", fileName);
		loadBuffer->Release();
		return false;
	}

	for (unsigned int i=0;i<pHeader->framesCount;i++)
	{
		TPoint pt;


		AntFileTrackElement & el1 = pFramesTable[i+iBoneCharacter1Index*pHeader->framesCount];
		pt.p1.x=SwizzleFloat(el1.px);
		pt.p1.y=SwizzleFloat(el1.py);
		pt.p1.z=SwizzleFloat(el1.pz);

		AntFileTrackElement & el2 = pFramesTable[i+iBoneCharacter2Index*pHeader->framesCount];
		pt.p2.x=SwizzleFloat(el2.px);
		pt.p2.y=SwizzleFloat(el2.py);
		pt.p2.z=SwizzleFloat(el2.pz);

		AntFileTrackElement & el = pFramesTable[i+iBoneCameraIndex*pHeader->framesCount];
		pt.pt1.x=SwizzleFloat(el.px);
		pt.pt1.y=SwizzleFloat(el.py);
		pt.pt1.z=SwizzleFloat(el.pz);

		pt.q.x=SwizzleFloat(el.qx);
		pt.q.y=SwizzleFloat(el.qy);
		pt.q.z=SwizzleFloat(el.qz);
		pt.q.w=SwizzleFloat(el.qw);

		if (iBoneCameraIndex2!=-1)
		{
			AntFileTrackElement & el3 = pFramesTable[i+iBoneCameraIndex2*pHeader->framesCount];

			pt.pt2.x=SwizzleFloat(el3.px);
			pt.pt2.y=SwizzleFloat(el3.py);
			pt.pt2.z=SwizzleFloat(el3.pz);

			pt.q2.x=SwizzleFloat(el.qx);
			pt.q2.y=SwizzleFloat(el.qy);
			pt.q2.z=SwizzleFloat(el.qz);
			pt.q2.w=SwizzleFloat(el.qw);
		}
		else
		{
			pt.pt2.x=pt.pt1.x;
			pt.pt2.y=pt.pt1.y;
			pt.pt2.z=pt.pt1.z;

			pt.q2.x=pt.q.x;
			pt.q2.y=pt.q.y;
			pt.q2.z=pt.q.z;
			pt.q2.w=pt.q.w;
		}		

		AddPoint(pt);
	}

	loadBuffer->Release();

	return true;
}
*/
//Нарисовать трек
void PathTracks::Draw(IRender & render)
{
	if (points.Size()==0) return;

	render.FlushBufferedLines();
	
	for(dword i = 0; i < points.Size() - 1; i++)
	{
		render.DrawBufferedLine(points[i].p1 + Offset, 0xff00ff00, points[i + 1].p1 + Offset, 0xff00ff00, false);
		render.DrawBufferedLine(points[i].p2 + Offset, 0xff00ff00, points[i + 1].p2 + Offset, 0xff00ff00, false);		
		render.DrawBufferedLine(points[i].p1 + Offset, 0xff00ffff, points[  i  ].p2 + Offset, 0xff00ffff, false);

		render.DrawBufferedLine(points[i].pt1 + Offset, 0xffff0000, points[i + 1].pt1 + Offset, 0xffff0000, false);
		render.DrawBufferedLine(points[i].pt2 + Offset, 0xffff0000, points[i + 1].pt2 + Offset, 0xffff0000, false);		
		render.DrawBufferedLine(points[i].pt1 + Offset, 0xffffff00, points[  i  ].pt2 + Offset, 0xffffff00, false);

		render.DrawBufferedLine(points[i].p1 + Offset, 0xffffffff, points[i].pt1 + Offset, 0xffffffff, false);
		render.DrawBufferedLine(points[i].p2 + Offset, 0xffffffff, points[i].pt2 + Offset, 0xffffffff, false);
	}

	render.DrawBufferedLine(points[points.Size() - 1].p1 + Offset, 0xff00ffff, points[points.Size() - 1].p2 + Offset, 0xff00ffff, false);
	
	render.DrawBufferedLine(points[points.Size() - 1].pt1 + Offset, 0xffffff00, points[points.Size() - 1].pt2 + Offset, 0xffffff00, false);

	render.FlushBufferedLines();
}

//Найти новую позицию для камеры
bool PathTracks::Find(const Vector & p, long & index, Vector & cp, Quaternion & cq)
{
	if(index >= 0)
	{
		//Ищем от текущей позиции
		if(index >= (long)points.Size() - 1) index = points.Size() - 1;
		
		for(dword i = 0; i < points.Size() - 1; i++)
		{
			long i1 = index + i;
			long i2 = index - i;
			if(i1 < (long)points.Size() - 1)
			{
				float k = Test(p, i1);
				if(k >= 0.0f && k < 1.0f)
				{
					index = i1;
					GetLerpCamPos(i1, i1 + 1, p, cp, cq, k);
					return true;
				}
			}
			if(i2 >= 0 && i1 != i2)
			{
				float k = Test(p, i2);
				if(k >= 0.0f && k < 1.0f)
				{
					index = i2;
					GetLerpCamPos(i2, i2 + 1, p, cp, cq, k);
					return true;
				}
			}
		}
	}
	else
	{
		//Ищем по всему треку наиближайший подходящий элемент
		float dist;
		Line line;
		for(long i = 0, idx = -1; i < (long)points.Size() - 1; i++)
		{
			float k = Test(p, i);
			if(k >= 0.0f)
			{
				line.p1 = (points[i].p1 + Offset).GetXZ();
				line.p2 = (points[i].p2 + Offset).GetXZ();
				float d = line.DistanceToLine(p);
				if(idx < 0 || dist > d)
				{
					dist = d;
					idx = i;
				}
			}
		}
		
		if(idx >= 0)
		{
			index = idx;
			return Find(p, index, cp, cq);
		}
	}

	return false;
}


//Получить точку трека
bool PathTracks::GetPoint(float index, Vector & cp, Quaternion & cq)
{	
	if(index < 0 || index >= points.Size()) return false;
	long i1 = long(index);
	long i2 = i1 + 1;
	
	if (i1 >= (long)points.Size()) i1 = points.Size() - 1;
	if (i2 >= (long)points.Size()) i2 = points.Size() - 1;

	Vector cp1, cp2;
	Quaternion cq1, cq2;

	cp1.Lerp(points[i1].pt1 + Offset, points[i2].pt1 + Offset, index - i1);
	cp2.Lerp(points[i1].pt2 + Offset, points[i2].pt2 + Offset, index - i1);
	cq1.SLerp(points[i1].q, points[i2].q, index - i1);
	cq2.SLerp(points[i1].q2, points[i2].q2, index - i1);

	cp.Lerp(cp1, cp2, 0.5f);
	cq.SLerp(cq1, cq2, 0.5f);
	
	return true;
}

//Удалить все точки
void PathTracks::ClearTrack()
{
	points.DelAll();
}

//Добавить точку в трек
void PathTracks::AddPoint(TPoint pt)
{
	points.Add(pt);
}

//Найти коэфициент для данного участка трека
inline float PathTracks::Test(const Vector & p, long i)
{
	Vector s1 = points[i].p1 + Offset;
	Vector e1 = points[i + 1].p1 + Offset;
	Vector s2 = points[i].p2 + Offset;
	Vector e2 = points[i + 1].p2 + Offset;
	
	float c2 = s1.z - e1.z;
	float c4 = e2.x - s2.x - e1.x + s1.x;
	float c6 = e2.z - s2.z - e1.z + s1.z;
	float c8 = s1.x - e1.x;
	float c1 = p.z - s1.z;	
	float c3 = s2.x - s1.x;	
	float c5 = s2.z - s1.z;	
	float c7 = p.x - s1.x;
	float a = c2*c4 - c6*c8;
	float b = c1*c4 + c2*c3 - c5*c8 - c6*c7;
	float c = c1*c3 - c5*c7;
	float k;
	
	if(fabs(a) >= 0.001f)
	{
		float D = b*b - 4*a*c;
		if(D < 0.0f) return -1.0f;
		D = sqrtf(D);
		float k1 = (-b + D)/(2*a);
		float k2 = (-b - D)/(2*a);	
		k = -1.0f;
		if(k1 >= 0.0f && k1 <= 1.0f) k = k1;
		if(k2 >= 0.0f && k2 <= 1.0f) k = k2;		
	}
	else
	{
		if(fabs(b) < 0.001f) return -1.0f;
		k = -c/b;
	}

	//Проверим попадание в трек
	if(k < 0.0f) return -1.0f;
	
	Vector p1, p2;
	p1.Lerp(s1, e1, k);
	p2.Lerp(s2, e2, k);
	Vector t = p2 - p1;
	
	if(((p - p1) | t) < 0.0f) return -1.0f;
	if(((p2 - p) | t) < 0.0f) return -1.0f;
	
	return k;
}

//Получить интерполированую позицию камеры
inline void PathTracks::GetLerpCamPos(long i1, long i2, const Vector & p, Vector & cp, Quaternion & cq, float k)
{
	Vector cp1, cp2;
	Quaternion cq1, cq2;
	//Найдём коэфициент
	cp1.Lerp(points[i1].p1 + Offset, points[i2].p1 + Offset, k);
	cp2.Lerp(points[i1].p2 + Offset, points[i2].p2 + Offset, k);
	float t = (p - cp1).GetLength()/(cp2 - cp1).GetLength();
	t = Clampf(t);
	//Проинтерполируем камеру по 2м осям
	cp1.Lerp(points[i1].pt1 + Offset, points[i2].pt1 + Offset, k);
	cp2.Lerp(points[i1].pt2 + Offset, points[i2].pt2 + Offset, k);
	cq1.SLerp(points[i1].q, points[i2].q, k);
	cq2.SLerp(points[i1].q2, points[i2].q2, k);
	cp.Lerp(cp1, cp2, t);
	cq.SLerp(cq1, cq2, t);
}