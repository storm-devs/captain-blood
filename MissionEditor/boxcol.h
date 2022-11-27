#ifndef _SLOW_BOX_COLLIDER_H_
#define _SLOW_BOX_COLLIDER_H_

#include "..\common_h\math3d.h"

class SlowBoxCollider
{
	struct IndexTriangle
	{
		int a;
		int b;
		int c;
	};

	Vector BoxMin;
	Vector BoxMax;
	Matrix transform;


	bool dsRayTriOverlapUseDistance (const Vector &v0, const Vector &v1, const Vector &v2, float &tri_dist, const Vector& v_from, const Vector& v_to);


public:

	Vector IntersectionPoint;

 SlowBoxCollider ();
 ~SlowBoxCollider ();

 	

 void Init (const Vector& BoxMin, const Vector& BoxMax, const Matrix& transform);

 float RayTrace (const Vector& from, const Vector& to);


};


#endif