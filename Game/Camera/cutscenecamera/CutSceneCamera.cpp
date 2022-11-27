#include "CutSceneCamera.h"



void CutSceneCamera::VectorLerpDouble(Vector& dst, Vector& v1, Vector& v2, double kBlend)
{
	dst.x = (float)((double)v1.x + ((double)v2.x - (double)v1.x)*kBlend);
	dst.y = (float)((double)v1.y + ((double)v2.y - (double)v1.y)*kBlend);
	dst.z = (float)((double)v1.z + ((double)v2.z - (double)v1.z)*kBlend);
}



CutSceneCamera::CutSceneCamera() : Tracks (_FL_, 1)
{
	fov = 1.0f + PI*0.5f;
	model = null;
	isPreview = false;


	fNeededDist=2.5f;
	fMinAngle=10;
	fMaxAngle=25;

	fMinSpeedAngle=80;
	fMaxSpeedAngle=120;

	MissionObject::Activate(false);

	AutoZoom = false;	

	model = null;
	fSpeed = 0.0f;

	ActiveTrack = 0;

	cameraPosition = 0.0f;

	offset = 0.0f;	
}

CutSceneCamera::~CutSceneCamera()
{
}

void CutSceneCamera::InitParams()
{	
	fSpeed = 0.0f;

	ActiveTrack = 0;

	cameraPosition = 0.0f;

	BaseCamera::InitParams();
}

//Обновить параметры
bool CutSceneCamera::CameraPreCreate(MOPReader & reader)
{
	CreateTrackData(reader);

	StartTrack(0);

	return true;
}

void CutSceneCamera::GetTrackInfo (Vector& pos, Vector& look_to,  Vector& up, float time)
{
	Track* track = &Tracks[ActiveTrack];
	float k = (time - track->BakedKnot[iCurKnot].Time)/
			  (track->BakedKnot[iCurKnot+1].Time - track->BakedKnot[iCurKnot].Time);

	Vector v;
	pos = v.Lerp(track->BakedKnot[iCurKnot].pos,track->BakedKnot[iCurKnot+1].pos,k);
	look_to = v.Lerp(track->BakedKnot[iCurKnot].look_to,track->BakedKnot[iCurKnot+1].look_to,k);
	up = v.Lerp(track->BakedKnot[iCurKnot].vUp,track->BakedKnot[iCurKnot+1].vUp,k);
}

void CutSceneCamera::EndTrack(bool restart)
{
	Track* track = &Tracks[ActiveTrack];	

	if (!EditMode_IsOn()) Tracks[ActiveTrack].event.Activate(Mission(),true);

	if (restart)
	{				
		iCurKnot = 0;
		cameraPosition = 0;

		fSpeed = Tracks[ActiveTrack].Knots[1].lenght/Tracks[ActiveTrack].Knots[1].Time;
	}
}	

void CutSceneCamera::ProcessCamera(float dltTime, bool restart)
{
	if (fWaitTime>0.0f)
	{
		fWaitTime -= dltTime;

		if (fWaitTime<0.0f)
		{
			fWaitTime = 0.0f;
		}
	}
	else
	if (iCurKnot < (int)Tracks[ActiveTrack].BakedKnot.Last())
	{		
		float speed = (float)GetCurrentSpeed(dltTime);

		//speed = 1.0f;
		cameraPosition += dltTime * speed;

		Track* track = &Tracks[ActiveTrack];

		while (cameraPosition>track->BakedKnot[iCurKnot+1].Time && fWaitTime==0.0f)
		{				
			iCurKnot++;

			fWaitTime = track->BakedKnot[iCurKnot].WaitTime;

			if (iCurKnot == (int)track->BakedKnot.Last())
			{
				if (!EditMode_IsOn()) Tracks[ActiveTrack].event.Activate(Mission(),true);

				if (restart)
				{				
					iCurKnot = 0;
					cameraPosition = 0;

					fSpeed = Tracks[ActiveTrack].Knots[1].lenght/Tracks[ActiveTrack].Knots[1].Time;
				}

				break;
			}
		}		
	}
	
	Vector pos,look_to,up;

	if (iCurKnot < (int)Tracks[ActiveTrack].BakedKnot.Last())
	{
		GetTrackInfo (pos, look_to, up, cameraPosition);
	}
	else
	{
		pos = Tracks[ActiveTrack].BakedKnot[iCurKnot].pos;
		look_to = Tracks[ActiveTrack].BakedKnot[iCurKnot].look_to;
		up = Tracks[ActiveTrack].BakedKnot[iCurKnot].vUp;
	}

	vLookFrom = pos + offset;
	vLookTo = look_to + offset;
	vUp = up;

	//Render().Print(50,10,0xffffffff,"cam = %4.3f knot = %i speed = %4.3f",cameraPosition,iCurKnot,fSpeed);
}

//Работа
void CutSceneCamera::WorkUpdate(float dltTime)
{	
	ProcessCamera(dltTime,false);
}

//Обновить состояние для предпросмотра
void CutSceneCamera::SelectedUpdate(float dltTime)
{
	ProcessCamera(dltTime,true);
}

//Отрисовка в режиме редактирования
void CutSceneCamera::SelectedDraw(float dltTime)
{
	//if (!Mission().EditMode_IsAdditionalDraw()) return;

	for (dword trackIdx = 0; trackIdx < Tracks.Size(); trackIdx++)
	{
		for (dword i = 0; i < Tracks[trackIdx].Knots.Size(); i++)
		{
			Vector vTangent = ((Tracks[trackIdx].Knots[i].pos - Tracks[trackIdx].Knots[i].tangent) * 0.3f) + Tracks[trackIdx].Knots[i].pos;
			Vector vBinormal = ((Tracks[trackIdx].Knots[i].pos - Tracks[trackIdx].Knots[i].binormal) * 0.3f) + Tracks[trackIdx].Knots[i].pos;

			Render().Print(Tracks[trackIdx].Knots[i].pos, 30.0f, 2.0f, Tracks[trackIdx].trackColor, "%d", i);
			Render().DrawSphere(Tracks[trackIdx].Knots[i].pos, 0.09f, Tracks[trackIdx].trackColor);
			Render().DrawSphere(vTangent, 0.02f, 0xFF00FF00);
			Render().DrawSphere(vBinormal, 0.02f, 0xFFFF0000);

			Render().DrawLine(vBinormal, 0xFFFFFF00, vTangent, 0xFFFFFF00);


			Matrix mtx;
			mtx.BuildView(Tracks[trackIdx].Knots[i].pos, Tracks[trackIdx].Knots[i].look_to, Vector(0.0f, 1.0f, 0.0f));
			mtx.Inverse();

			if (model)
			{
				Matrix mScale;
				mScale.BuildScale(Vector(0.6f));
				mtx = mScale * mtx;
				model->SetTransform(mtx);
				model->Draw();
			} else
			{
				Vector boxExtents = Vector (0.03f, 0.03f, 0.1f);
				Render().DrawSolidBox(-boxExtents, boxExtents, mtx);
			}
		}

		for (int k = 0; k < (int)Tracks[trackIdx].BakedKnot.Size()-1; k++)
		{
			Render().DrawLine(Tracks[trackIdx].BakedKnot[k].pos, 0xFFFFFFFF,
							  Tracks[trackIdx].BakedKnot[k+1].pos, Tracks[trackIdx].trackColor);			

			//Render().DrawLine(Tracks[trackIdx].BakedKnot[k].look_to, 0xFFFFFFFF,
			//				  Tracks[trackIdx].BakedKnot[k+1].look_to, Tracks[trackIdx].trackColor);			

			Render().DrawLine(Tracks[trackIdx].BakedKnot[k].pos, 0xFFFFFFFF,
							  Tracks[trackIdx].BakedKnot[k].look_to, Tracks[trackIdx].trackColor);
		}
		Render().DrawLine(Tracks[trackIdx].BakedKnot.LastE().pos, 0xFFFFFFFF,
						  Tracks[trackIdx].BakedKnot.LastE().look_to, Tracks[trackIdx].trackColor);
	}
	
	Render().DrawVector( Tracks[ActiveTrack].BakedKnot[iCurKnot].pos,
		Tracks[ActiveTrack].BakedKnot[iCurKnot].pos + (Tracks[ActiveTrack].BakedKnot[iCurKnot].look_to-Tracks[ActiveTrack].BakedKnot[iCurKnot].pos)*4.f,
		0xFFFFFFFF );

	BaseCamera::SelectedDraw(dltTime);	
}



Vector CutSceneCamera::CubicBezier(Vector p0, Vector p1, Vector p2, Vector p3, float mu)
{
	Vector a,b,c,p;

	c = 3.0f * (p1 - p0);
	b = 3.0f * (p2 - p1) - c;
	a = p3 - p0 - c - b;

	p = a * mu * mu * mu + b * mu * mu + c * mu + p0;

	return p;
}

void CutSceneCamera::CalcPoint(Vector& pos,int track_index, int point_index, float mu, bool parity)
{
	Track* track = &Tracks[track_index];

	if (parity)
	{
		pos = CubicBezier(track->Knots[point_index].pos, track->Knots[point_index].binormal, track->Knots[point_index+1].binormal, track->Knots[point_index+1].pos, mu);
	}
	else
	{
		pos = CubicBezier(track->Knots[point_index].pos, track->Knots[point_index].tangent, track->Knots[point_index+1].tangent, track->Knots[point_index+1].pos, mu);
	}
}

void CutSceneCamera::CalcTarget(const Vector& pos, Vector& target,int track_index, int point_index, float mu, bool parity)
{
	Track* track = &Tracks[track_index];

	if (parity)
	{
		target = CubicBezier(track->Knots[point_index].look_to, track->Knots[point_index].binormal_lt, track->Knots[point_index+1].binormal_lt, track->Knots[point_index+1].look_to, mu);
	}
	else
	{
		target = CubicBezier(track->Knots[point_index].look_to, track->Knots[point_index].tangent_lt, track->Knots[point_index+1].tangent_lt, track->Knots[point_index+1].look_to, mu);
	}
}

void CutSceneCamera::CalcTargetByAng(Vector& target, const Vector& pos, int track_index, int point_index, float mu)
{
	Track* track = &Tracks[track_index];

	Vector delta = track->Knots[point_index+1].vang - track->Knots[point_index].vang;
	if( delta.x > PI ) delta.x = delta.x - PIm2;
	else if( delta.x < -PI ) delta.x = delta.x + PIm2;
	if( delta.y > PI ) delta.y = delta.y - PIm2;
	else if( delta.y < -PI ) delta.y = delta.y + PIm2;
	if( delta.z > PI ) delta.z = delta.z - PIm2;
	else if( delta.z < -PI ) delta.z = delta.z + PIm2;

	Vector ang = track->Knots[point_index].vang + delta * mu;
	//ang.Lerp( track->Knots[point_index].vang, track->Knots[point_index+1].vang, mu );
	target = pos + Matrix(ang).vz * 0.25f;
}

void CutSceneCamera::CreateTrackData (MOPReader & reader)
{
	Tracks.DelAll();

	long TracksCount = reader.Array();
	Tracks.AddElements(TracksCount);
	for (long i = 0; i < TracksCount; i++)
	{
		Track* track = &Tracks[i];

		//Позиция трека
		Vector pos = reader.Position();
		Vector ang = reader.Angles();

		track->matWorld.Build(ang, pos);

		track->trackColor = reader.Colors();

		track->bRotateZ = reader.Bool();

		track->accel = reader.Float();

		track->useLinearRot = reader.Bool();

		track->event.Init(reader);



		long ArraySize = reader.Array();
		track->Knots.AddElements(ArraySize);
		for (long n = 0; n < ArraySize; n++)
		{
			Knot & knot = track->Knots[n];

			knot.pos = Vector (reader.Position() * track->matWorld);

			Matrix mat = Matrix (reader.Angles());			

			Vector tangent = (mat * track->matWorld).vz;
			tangent *= reader.Float();

			knot.tangent = knot.pos + tangent;
			
			knot.binormal = knot.pos - tangent;// + (knot.pos-knot.tangent);

			//mat = Matrix (reader.Angles());
			knot.vang = reader.Angles();
			mat = Matrix (knot.vang);

			knot.normal = (mat * track->matWorld).vy;

			Vector delta = ((mat * track->matWorld).vz * 0.25f);
			knot.look_to = knot.pos + delta;

			knot.binormal_lt = knot.binormal + delta;
			knot.tangent_lt = knot.tangent + delta;

			knot.Time = reader.Float();
			knot.WaitTime = reader.Float();

			//track->Knots.Add(knot);
		}

		
		float time = 0;
		BakedKnot bakedKnot;

		// добавляем стартовую точку трека
		bakedKnot.Time=0;
		bakedKnot.pos = track->Knots[0].pos;
		bakedKnot.look_to = track->Knots[0].look_to;
		bakedKnot.vUp = track->Knots[0].normal;
		bakedKnot.WaitTime = track->Knots[0].WaitTime;
		track->BakedKnot.Add(bakedKnot);		

		for (long n = 0; n < ArraySize-1; n++)
		{	
			float segment_time = track->Knots[n+1].Time;

			// счиатем дистанцию на сегменте
			float delta = 1.0f / (float)(CutSceneCamera_KnotNumSegments+1);
			Vector vPrev = track->Knots[n].pos;
			Vector vNext = vPrev;
			track->Knots[n+1].lenght = 0;
			for (float t = delta; t <1.0f; t+= delta)
			{
				CalcPoint(vNext,Tracks.Size()-1, n, t, (n & 1));
				track->Knots[n+1].lenght += (vNext-vPrev).GetLength();
				vPrev = vNext;
			}
			track->Knots[n+1].lenght += (track->Knots[n+1].pos-vPrev).GetLength();

			float segment_delta = 0.0333f;
			float segment_delta2pow = segment_delta * segment_delta;
			float segment_time_div = 1.f / segment_time;

			// бъем отрезок трека на мелкие отрезки
			if (track->Knots[n+1].lenght > segment_delta)
			{
				vNext = vPrev = track->Knots[n].pos;
				float step = 0.f;
				float len = 0.0f;
				// пока не вышли за диапазон сегмента
				while( len+segment_delta*1.5f < track->Knots[n+1].lenght && step < 1.f )
				{
					// находим следущую точку на заданной дистанции от предыдущей
					float dist2pow = 0.0f;
					while (dist2pow<segment_delta2pow && step<1.f)
					{
						step+=0.001f;
						CalcPoint(vNext,Tracks.Size()-1, n, step, (n & 1));
						dist2pow = ~(vNext-vPrev);
					}
					len += sqrt( dist2pow );

					// ставим позицию этой точки и время в ней
					bakedKnot.pos = vNext;
					bakedKnot.Time= time + len/track->Knots[n+1].lenght * segment_time;
					// коэффициент времени (тот же что и для вычисления реалтайм позиции камеры)
					//float fKtime = track->useLinearRot ? ((bakedKnot.Time - time) * segment_time_div) : step;
					// расчитаем точку на которую будет смотреть камера в найденной позиции (исходя из временного коэффициента)
					if (track->useLinearRot)
					{
						float fKtime = (bakedKnot.Time - time) * segment_time_div;
						CalcTargetByAng(bakedKnot.look_to, bakedKnot.pos, Tracks.Size()-1, n, fKtime);
					}
					else
					{
						CalcTarget(bakedKnot.pos,bakedKnot.look_to,Tracks.Size()-1, n, step, (n & 1));
					}
					// нормаль  в этой точке (вектор направления по Y)
					bakedKnot.vUp.Lerp(track->Knots[n].normal, track->Knots[n+1].normal, step);
					// никакого ожидания (мы внутри сегмента)
					bakedKnot.WaitTime = 0.0f;

					// добавляем данные трека в буфер и переходим к следущей точке
					track->BakedKnot.Add(bakedKnot);
					vPrev = vNext;
				}
			}
			else
			{
				float step = 0.075f;

				for (float t = step; t<1.0f; t+=step)
				{
					CalcPoint(vNext,Tracks.Size()-1, n, t, (n & 1));

					bakedKnot.Time= time + t * segment_time;

					if( track->useLinearRot )
						CalcTargetByAng(bakedKnot.look_to, vNext, Tracks.Size()-1, n, t);
					else
						CalcTarget(vNext,bakedKnot.look_to,Tracks.Size()-1, n, t, (n & 1));

					bakedKnot.vUp.Lerp(track->Knots[n].normal, track->Knots[n+1].normal, t);
					bakedKnot.WaitTime = 0.0f;
					track->BakedKnot.Add(bakedKnot);
				}
			}

			// добавляем финальную точку сегмента
			time += segment_time;
			bakedKnot.Time=time;
			bakedKnot.pos = track->Knots[n+1].pos;
			bakedKnot.look_to = track->Knots[n+1].look_to;
			bakedKnot.vUp = track->Knots[n+1].normal;
			bakedKnot.WaitTime = track->Knots[n+1].WaitTime;
			track->BakedKnot.Add(bakedKnot);
		}
	}
}



void CutSceneCamera::StartTrack (int TrackIndex)
{
	ActiveTrack = TrackIndex;	
	fWaitTime = Tracks[ActiveTrack].Knots[0].WaitTime;
	cameraPosition = 0.0f;
	
	iCurKnot = 0;

	fSpeed = Tracks[ActiveTrack].Knots[1].lenght/Tracks[ActiveTrack].Knots[1].Time;

	Tracks[ActiveTrack].event.Reset();
}


//Активировать/деактивировать объект
void CutSceneCamera::Activate(bool isActive)
{
	BaseCamera::Activate(isActive);

	if (isActive)
	{
		fWaitTime = Tracks[ActiveTrack].Knots[0].WaitTime;

		//снимаем с паузы и ставим в начало
		cameraPosition = 0.0f;

		StartTrack(ActiveTrack);
	}
}



void CutSceneCamera::GetCameraPosition(Vector & position)
{
	position = vLookFrom;
}

bool CutSceneCamera::GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up)
{
	if(!BaseCamera::GetCameraTarget(position, target_pos, up))
	{
		target_pos = vLookTo;

		target_pos = target_pos * offset_matrix;
	}
	
	if (Tracks[ActiveTrack].bRotateZ)
	{
		up = vUp;
	}
	else
	{
		up  = Vector(0.0f, 1.0f, 0.0f);
	}	

	return true;
}



float CutSceneCamera::GetCurrentSpeed (float dltTime)
{
	Track* track = &Tracks[ActiveTrack];

	float dist = (track->BakedKnot[iCurKnot+1].pos - track->BakedKnot[iCurKnot].pos).GetLength();

	if (dist<0.05f)
	{
		return 1.0f;
	}

	float fNeedSpeed = dist / (track->BakedKnot[iCurKnot+1].Time - track->BakedKnot[iCurKnot].Time);	
	
	float accel = track->accel * fabs(fNeedSpeed - fSpeed);	

	if (fNeedSpeed > fSpeed)
	{
		fSpeed += (accel * dltTime);
		if (fSpeed > fNeedSpeed) fSpeed = fNeedSpeed;
	}
	else
	{
		fSpeed -= (accel * dltTime);
		if (fSpeed < fNeedSpeed) fSpeed = fNeedSpeed;
	}

	return fSpeed/fNeedSpeed;
}

//Обработчик команд для объекта
void CutSceneCamera::Command(const char * id, dword numParams, const char ** params)
{
	if(!id) return;

	if(string::IsEqual(id, "SeaFatality Start"))
	{		
		if(numParams > 1)
		{
			MOSafePointer mo;
			FindObject(ConstString(params[0]), mo);

			if (mo.Ptr())
			{
				offset = mo.Ptr()->GetMatrix(Matrix()).pos;
				LogicDebug("Sea Fatality: make fatality on \"%s\"", params[0]);
			}
			else
			{
				offset = 0.0f;
				LogicDebug("Sea Fatality: \"%s\" not founded", params[0]);
			}			
		}
		else
		{
			offset = 0.0f;			
			LogicDebug("Sea Fatality: invalid parametrs");
		}
	}
	else
	if(string::IsEqual(id, "SeaFatality End"))
	{
		offset = 0.0f;
		LogicDebug("Sea Fatality: invalid parametrs");
	}

	BaseCamera::Command(id, numParams, params);
}

bool CutSceneCamera::CheckCommand(const char * id)
{
	if(!id) return false;

	if(string::IsEqual(id, "SeaFatality Start"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "SeaFatality End"))
	{
		return true;
	}

	return BaseCamera::CheckCommand(id);
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(CutSceneCamera, "Cut scenes camera", '1.00', 1000, CAMERA_COMMENT("Cut scenes camera\n\n"), "Cameras")

	MOP_ARRAYBEG("Tracks", 1, 100000)

		MOP_POSITIONC("Camera track position", Vector(0.0f), "Change track positions")
		MOP_ANGLESC("Camera track angles", Vector(0.0f), "Rotate track")
		MOP_COLOR("Track color", Color(0xFFFF0000L));
		
		MOP_BOOL("RotateZ", false)

		MOP_FLOATEX("Accel", 75.0f,0.001f,1000.0f)

		MOP_BOOL("Use Linear Rot", false)

		MOP_MISSIONTRIGGER("End")

		MOP_ARRAYBEG("Track", 2, 10000000);
			MOP_POSITION("Position", Vector (0.0f));
			MOP_ANGLES("Tangent rotate", Vector (0.0f));
			MOP_FLOAT("Tangent len", 2.5f);
			MOP_ANGLES("Rotation", Vector (0.0f));
			MOP_FLOATEX("Segment time in seconds", 1.0f, 0.0001f, 10000000.0f);
			MOP_FLOATEX("Wait time", 0.0f, 0.0001f, 10000000.0f);
		MOP_ARRAYEND;
	MOP_ARRAYEND;

	MOP_CAM_STD

MOP_ENDLIST(CutSceneCamera)