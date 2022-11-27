#ifndef _cut_scene_Camera_h_
#define _cut_scene_Camera_h_

#include "..\..\..\common_h\mission\Mission.h"
#include "..\TrackCamera\CameraTracksFile.h"
#include "..\camera_inc.h"

class CutSceneCamera : public BaseCamera
{

//--------------------------------------------------------------------------------------------
public:
	CutSceneCamera();
	virtual ~CutSceneCamera();

	virtual void InitParams();

//--------------------------------------------------------------------------------------------
public:

	//Обновить параметры
	virtual bool CameraPreCreate(MOPReader & reader);

	virtual void GetCameraPosition(Vector & position);

	virtual bool GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	//Работа
	virtual void WorkUpdate(float dltTime);

	//Обновить состояние для предпросмотра
	virtual void SelectedUpdate(float dltTime);

	virtual void SelectedDraw(float dltTime);

	bool CheckMOCondition(MissionObject* mo);

	bool CheckLockTargetList();

	void Command(const char * id, dword numParams, const char ** params);
	bool CheckCommand(const char * id);


//--------------------------------------------------------------------------------------------
private:

	//Установка камеры в режиме редактирования
	void _cdecl EditModeSet(float dltTime, long level);





//--------------------------------------------------------------------------------------------
private:

	struct BakedKnot
	{
		Vector pos;
		Vector look_to;
		Vector vUp;
		float Time;
		float WaitTime;		
	};

	#define CutSceneCamera_KnotNumSegments 20
    

	struct Knot
	{
		Vector pos;
		Vector tangent;
		Vector binormal;

		Vector tangent_lt;
		Vector binormal_lt;

		Vector vang;

		Vector normal;

		Vector look_to;
		float Time;
		float WaitTime;		

		float lenght;		

		Knot()
		{
			Time = 1.0f;
			WaitTime = 0.0f;

			lenght = 0.0f;
		};
	};

	struct Track
	{
		MissionTrigger event;				//Событие
		Matrix matWorld;
		Color trackColor;

		bool  bRotateZ;
		float accel;
		bool  useLinearRot;

		array<Knot> Knots;		
		array<BakedKnot> BakedKnot;		

		Track() : Knots(_FL_, 1), BakedKnot(_FL_)
		{
			accel = 75.0f;
			useLinearRot = false;
		}
	};
	

	Vector offset;

	array<Track> Tracks;

	Vector CubicBezier(Vector p0, Vector p1, Vector p2, Vector p3, float mu);
	void CreateTrackData (MOPReader & reader);
	float GetCurrentSpeed (float dltTime);
	void GetTrackInfo (Vector& pos, Vector& look_to, Vector& up, float time);	
	void StartTrack (int TrackIndex);
	void CalcPoint(Vector& pos,int track_index, int point_index, float mu, bool parity);
	void CalcTarget(const Vector& pos, Vector& target,int track_index, int point_index, float mu, bool parity);
	void CalcTargetByAng(Vector& target, const Vector& pos, int track_index, int point_index, float mu);

	void ProcessCamera(float dltTime, bool restart);

	void EndTrack(bool restart);

	static void VectorLerpDouble(Vector& dst, Vector& v1, Vector& v2, double kBlend);

	float fSpeed;  //Текущая скорость камеры
	int ActiveTrack;		
	
	Vector vLookFrom;
	Vector vLookTo;
	Vector vUp;

	int    iCurKnot;
	float  fWaitTime;
	float  cameraPosition;	
};

#endif

