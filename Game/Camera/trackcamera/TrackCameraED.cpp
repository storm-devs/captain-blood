
#include "TrackCameraED.h"


//Прочитать параметры идущие перед стандартных
void TrackCameraED::ReadTrack(MOPReader & reader)
{
	if(track) delete track;

	track = NEW PathTracks();

	track->ClearTrack();

	long PointsCount = reader.Array();

	for (long i=0;i<PointsCount;i++)
	{
		PathTracks::TPoint pt;
		
		pt.pt1 = reader.Position();
		Vector angels = reader.Angles();
		pt.q = Quaternion(Matrix(-angels.x, angels.y, -angels.z).RotateY(PI));

		pt.pt2 = reader.Position();	
		angels = reader.Angles();
		pt.q2 = Quaternion(Matrix(-angels.x, angels.y, -angels.z).RotateY(PI));
		
		pt.p1 = reader.Position();
		pt.p2 = reader.Position();

		track->AddPoint(pt);
	}

	if(!track->GetNumPoints())
	{
		delete track;
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

MOP_BEGINLISTCG(TrackCameraED, "Tracked camera ED", '1.00', 1000, CAMERA_COMMENT("Tracked camera ED\n\n"),"Cameras")
	
	MOP_ARRAYBEG("Camera Track", 0, 512);
		MOP_POSITION ("TrackPoint", Vector(0.0f));		
		MOP_ANGLES   ("Direction", Vector(0.0f));
		MOP_POSITION ("TrackPoint2", Vector(0.0f));
		MOP_ANGLES   ("Direction2", Vector(0.0f));
		MOP_POSITION ("RailPos1", Vector(0.0f));
		MOP_POSITION ("RailPos2", Vector(0.0f));
	MOP_ARRAYEND	

	MOP_TRACK_CAM
	MOP_CAM_STD

MOP_ENDLIST(TrackCameraED)
