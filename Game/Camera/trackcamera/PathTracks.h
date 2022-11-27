//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// PathTracks	
//============================================================================================

#ifndef _PathTracks_h_
#define _PathTracks_h_

#include "..\..\..\common_h\mission\Mission.h"
#include "CameraTracksFile.h"

class PathTracks  
{
public:

#pragma pack(push, 1)
	struct TPoint
	{
		Vector p1, p2;
		Vector pt1, pt2;
		Quaternion q;
		Quaternion q2;
	};
#pragma pack(pop)

//--------------------------------------------------------------------------------------------
public:
	PathTracks();
	virtual ~PathTracks();

//--------------------------------------------------------------------------------------------
public:
	//Загрузить трек в PathTracks
	//bool LoadCTF(const char * fileName);
//	bool LoadANT(IFileService * fs, const char * fileName);

	//Нарисовать трек
	void Draw(IRender & render);
	//Найти новую позицию для камеры
	bool Find(const Vector & p, long & index, Vector & cp, Quaternion & cq);

	//Получить точку трека
	bool GetPoint(float index, Vector & cp, Quaternion & cq);

	//Удалить все точки
	void ClearTrack();
	//Добавить точку в трек
	void AddPoint(TPoint pt);
	//Получить число точек в треке
	long GetNumPoints() { return (long)points.Size(); };
//--------------------------------------------------------------------------------------------
private:
	//Найти коэфициент для данного участка трека
	float Test(const Vector & p, long i);
	//Получить интерполированую позицию камеры
	void GetLerpCamPos(long i1, long i2, const Vector & p, Vector & cp, Quaternion & cq, float k);


	Vector Offset;
//--------------------------------------------------------------------------------------------
public:
	array<TPoint> points;

	void SetOffset(Vector _Offset) { Offset = _Offset; };
};

#endif

