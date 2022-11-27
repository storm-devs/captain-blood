//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*  description: Oriented Bounding Box
//*
//*  usage: external
//****************************************************************


#ifndef _ORIENTED_BOUNDING_BOX_
#define _ORIENTED_BOUNDING_BOX_

#include "..\Math3D.h"



class GMXOBB
{

public:
	
	// Положение коробки
	Matrix mWorld;
	// Размерность коробки 
	Vector vExtents;
	
	
	GMXOBB ()
	{
		vExtents = Vector(0.0f);
	}
	GMXOBB (const Vector& _vExtents, const Matrix& _mWorld)
	{
		vExtents = _vExtents;
		mWorld = _mWorld;
	}

	GMXOBB (const Vector& _vCenter, const Vector& _vExtents, const Matrix& _mRotation)
	{
		vExtents = _vExtents;
		mWorld = _mRotation;
		mWorld.pos = _vCenter;
	}
	
	GMXOBB (const Vector& _vCenter, const Vector& _vExtents)
	{
		Create(_vCenter,  _vExtents);
	}
	
	
	~GMXOBB () {}


	//Получить трансформированную матрицей коробку
	__forceinline GMXOBB Transform (const Matrix& m) const
	{
		GMXOBB tbox = *this;

		Vector vOldCenter = mWorld.pos;
		
		tbox.mWorld = mWorld * m;
		return tbox;
	}

	//Получить трансформированную матрицей коробку
	__forceinline void TransformInplace (const Matrix& m)
	{
		mWorld = mWorld * m;
	}



	__forceinline void Create (const Vector& _vCenter, const Vector& _vExtents)
	{
		vExtents = _vExtents;
		mWorld.pos = _vCenter;
	}

	//! Установить из минимума и максимума 
	__forceinline void SetFromMinMax (const Vector &vMin, const Vector &vMax)
	{
		vExtents.x = ((vMax.x - vMin.x) * 0.5f);
		vExtents.y = ((vMax.y - vMin.y) * 0.5f);
		vExtents.z = ((vMax.z - vMin.z) * 0.5f);
		mWorld.pos = vMin + vExtents;
	}

	//! Получить размерность
	__forceinline const Vector& GetExtents () const
	{
		return vExtents;
	}

	//! Получить центр 
	__forceinline const Vector& GetCenter () const
	{
		return mWorld.pos;
	}

	//! Установить размерность
	__forceinline void SetExtents (const Vector& _vExtents)
	{
		vExtents = _vExtents;
	}

	//! Установить центр 
	__forceinline void SetCenter (const Vector& _vCenter)
	{
		mWorld.pos = _vCenter;
	}

	//! Получить мин. и макс.
	__forceinline void GetMinMax (Vector &vMin, Vector &vMax) const
	{
		vMin = mWorld.pos - vExtents;
		vMax = mWorld.pos + vExtents;
	}

	//! Установить вращение
	__forceinline void SetRotation (const Matrix& mRotation)
	{
		Vector SavedPos = mWorld.pos;
		mWorld = mRotation;
		mWorld.pos = SavedPos;
	}

	//! Получить трансформацию 
	__forceinline const Matrix& GetTransform () const
	{
		return mWorld;
	}

	//! Установить трансформацию
	__forceinline void SetTransform (const Matrix& mTransf)
	{
		mWorld = mTransf;
	}


	//! получить углы OBB в мировой системе координат...
	__forceinline void GetCornerPoints (Vector* pPointsArray)
	{
		pPointsArray[0].x = vExtents.x;
		pPointsArray[0].y = vExtents.y;
		pPointsArray[0].z = vExtents.z;

		pPointsArray[1].x = -vExtents.x;
		pPointsArray[1].y = vExtents.y;
		pPointsArray[1].z = vExtents.z;

		pPointsArray[2].x = -vExtents.x;
		pPointsArray[2].y = vExtents.y;
		pPointsArray[2].z = -vExtents.z;

		pPointsArray[3].x = vExtents.x;
		pPointsArray[3].y = vExtents.y;
		pPointsArray[3].z = -vExtents.z;

		pPointsArray[4].x = vExtents.x;
		pPointsArray[4].y = -vExtents.y;
		pPointsArray[4].z = vExtents.z;

		pPointsArray[5].x = -vExtents.x;
		pPointsArray[5].y = -vExtents.y;
		pPointsArray[5].z = vExtents.z;

		pPointsArray[6].x = -vExtents.x;
		pPointsArray[6].y = -vExtents.y;
		pPointsArray[6].z = -vExtents.z;

		pPointsArray[7].x = vExtents.x;
		pPointsArray[7].y = -vExtents.y;
		pPointsArray[7].z = -vExtents.z;

		for (int i = 0; i < 8; i++)
		{
			pPointsArray[i] = mWorld.MulVertex(pPointsArray[i]);
		}
	}
};

#endif