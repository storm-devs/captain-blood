//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// TrackCamera	
//============================================================================================
			

#include "TrackCamera.h"
#include "..\CameraController\CameraController.h"
#include "..\TrackCamera\CameraTracksFile.h"

void TrackCamera::InitParams()
{
	BaseCamera::InitParams();

	pos = 0.0f;
	ang.SetIdentity();
	currentIndex = -1;
	fovFar = fovNear = 1.25f;
	fovNearDist = 10.0f;
	fovFarDist = 50.0f;;

	track = null;

	distFactor = 1.0f;
	previewIndex = 0.0f;
	traceTarget = false;	
	isMoveToPos = false;

	CamBlendK = 0;

	traceTarget = false;
	bNormalize = false;
}

TrackCamera::~TrackCamera()
{
	if(track) delete track;
	track = null;
	if(model) model->Release();
	model = null;
}

bool TrackCamera::GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up)
{		
	Matrix camRot;
	ang.GetMatrix(camRot);
	camRot.vz = -camRot.vz;
	target_pos = position + camRot.vz;
	up = camRot.vy;

	if (bNormalize)
	{
		up = Vector(0.0f,1.0f,0.0f);
	}

	return true;
}

void TrackCamera::ReadTrack(MOPReader & reader)
{
	//Зачитаем трек
	string path = "Resource\\Cameras\\";
	path += reader.String().c_str();

	if(track) delete track;

	track = NEW PathTracks();

//	if(!track->LoadANT(&Files(), path))
	{
//		delete track;
		track = null;
	}

	isMoveToPos = true;

	MissionObject * obj = Mission().Player();

	if(obj)
	{
		Matrix mtx;
		chrPos = obj->GetMatrix(mtx).pos;
	}
}

//Прочитать параметры идущие перед стандартных
bool TrackCamera::CameraPreCreate(MOPReader & reader)
{
	ReadTrack(reader);
	
	//Изменение перспективы
	fovNear = reader.Float()*(PI/180.0f);
	fovNearDist = reader.Float();
	fovFar = reader.Float()*(PI/180.0f);
	fovFarDist = reader.Float();

	if(fovFarDist - fovNearDist < 1e-5f)
	{
		fovFarDist = fovNearDist + 1e-5f;
	}

	distFactor = reader.Float();

	//Режим слежения за целью
	traceTarget = reader.Bool();

	Vector offset = reader.Position();
	
	if (track)
	{
		track->SetOffset(offset);
	}
	

	bNormalize = reader.Bool();

	currentIndex = -1;
	isMoveToPos = true;

	return track != null;
}

void TrackCamera::Reset()
{
	MissionObject * obj = Mission().Player();
	Matrix mtx;
	if(obj)
	{
		obj->GetMatrix(mtx);
		chrPos = mtx.pos;
	}

	if(IsActive())
	{
		if(track)
		{
			track->Find(chrPos, currentIndex, pos, ang);
			if(!obj) MissionObject::Activate(false);
		}
	}
	else
	{			
		currentIndex = -1;
		isMoveToPos = true;
	}
}

//Работа
void TrackCamera::WorkUpdate(float dltTime)
{	
	if (AutoZoom)
	{
		CamBlendK+=dltTime*2.5f;

		if (CamBlendK>1.0f)
		{
			CamBlendK = 1.0f;
		}
	}
	else
	{
		CamBlendK-=dltTime*2.5f;

		if (CamBlendK<0.0f)
		{
			CamBlendK = 0.0f;
		}
	}

	Vector trg, up(0.0f, 1.0f, 0.0f);

	GetCameraPosition(pos);
	
	//Получим матрицу цели
	Matrix mtx;
		
	if(targetID.NotEmpty())
	{
		if(!target.Validate())
		{
			FindObject(targetID,target);
		}
	}
	if(!target.Ptr())
	{		
		Activate(false);

		return;
	}

	target.Ptr()->GetMatrix(mtx);
	
	//Если надо двигать, делаем это
	//if(isMoveToPos)
	{
		chrPos = mtx.pos;
		chrPos.Lerp(chrPos, mtx.pos, Clampf(dltTime*6.0f));
		
		//Найдём соответствующую позицию на треке
		if(track)
		{
			track->Find(chrPos, currentIndex, pos, ang);
		}
		else
		{
			pos = 0.0f;
			ang.SetIdentity();
		}
				
		if(traceTarget)
		{
			Matrix mtmp;
			mtmp.BuildView(chrPos + targetPos, pos, Vector(0.0f, 1.0f, 0.0f));
			mtmp.Inverse();			

			ang.Set(mtmp);
		}
		else
		{
			Matrix mtmp;
			mtmp.BuildView(chrPos + targetPos, pos, Vector(0.0f, 1.0f, 0.0f));
			mtmp.Inverse();

			Quaternion q(mtmp);			

			ang.SLerp(ang, q, CamBlendK);
		}
		
		//Надо ли ещё двигать
		if(~(mtx.pos - chrPos) < 0.005f) isMoveToPos = false;
	}
	/*else
	{
		//Смотрим, начать двигать или телепортнуть камеру
		float d = ~(mtx.pos - chrPos);
		if(d > 0.5f)
		{
			if(d < 3.0f)
			{
				isMoveToPos = true;
			}else{
				chrPos = mtx.pos;
			}
		}
	}*/
	
	//Установим камеру
	float kDist = (pos - mtx.pos).GetLength();
	kDist = (kDist - fovNearDist)/(fovFarDist - fovNearDist);
	
	if(kDist < 0.0f) kDist = 0.0f;
	if(kDist > 1.0f) kDist = 1.0f;
	
	kDist = powf(kDist, distFactor);
	
	fov = fovNear + (fovFar - fovNear)*kDist;	
}

//Нарисовать дополнительную информацию нри селекте
void TrackCamera::SelectedDraw(float dltTime)
{	
	previewIndex += dltTime*1.0f;	

	if(track)
	{
		//Найдём соответствующую позицию на треке
		if(EditMode_IsSelect())
		{
			if(!track->GetPoint(previewIndex, pos, ang))
			{
				previewIndex = 0.0f;
				if(!track->GetPoint(previewIndex, pos, ang))
				{
					pos = 0.0f;
					ang.SetIdentity();
				}
			}
		}
		else
		{
			track->GetPoint(0.0f, pos, ang);
			previewIndex = 0.0f;
		}
	}
	else
	{
		pos = 0.0f;
		ang.SetIdentity();
	}

	//if(isPreview)
	//{
		//Work(dltTime, level);
	//	Render().SetView((Mission().GetSwingMatrix()*GetMatrix(Matrix())).BuildViewFromObject());
	//}

	if (track)
	{			
		track->Draw(Render());		
	}
	
	BaseCamera::SelectedDraw(dltTime);
}

//Получить позицию камеры
void TrackCamera::GetCameraPosition(Vector & position)
{
	position = pos;
}

//Отрисовка в выделеном режиме
void _cdecl TrackCamera::DebugDraw(float dltTime, long level)
{
	EditModeDrawSelected(dltTime, level);	

	RS_SPRITE spr[4];

	float fPosX = -1.0f;
	float fPosY = 1.0f-0.075f;

	spr[0].vPos = Vector (fPosX,      fPosY+0.075f, 0.0f);
	spr[1].vPos = Vector (fPosX+1.0f, fPosY+0.075f, 0.0f);
	spr[2].vPos = Vector (fPosX+1.0f, fPosY, 0.0f);
	spr[3].vPos = Vector (fPosX,      fPosY, 0.0f);

	spr[0].tv = 0.0f;
	spr[0].tu = 0.0f; 
	spr[0].dwColor = 0xAA000000;

	spr[1].tv = 0.0f;
	spr[1].tu = 1.0f; 
	spr[1].dwColor = 0xAA000000;

	spr[2].tv = 1.0f;
	spr[2].tu = 1.0f;  
	spr[2].dwColor = 0xAA000000;

	spr[3].tv = 1.0f;
	spr[3].tu = 0.0f;  	
	spr[3].dwColor = 0xAA000000;

	Render().DrawSprites(NULL,spr, 1);

	Render().Print(0,0,0xff00ff00,"%s : Current Segment is %i",GetObjectID().c_str(),currentIndex);
	Render().DrawSphere(chrPos,0.5f,0xff00ff00);
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(TrackCamera, "Tracked camera", '1.00', 1000, CAMERA_COMMENT("Tracked camera\n\n"),"Cameras")
	MOP_STRINGC("Camera track file", "Camera", "File whith track data (*.ant)")	
	MOP_TRACK_CAM
	MOP_CAM_STD
MOP_ENDLIST(TrackCamera)
