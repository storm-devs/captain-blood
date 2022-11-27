#include "boxcol.h"
#include "..\common_h\render.h"

#define LOCAL_EPSILON 0.000001f
 

SlowBoxCollider::SlowBoxCollider ()
{
	BoxMin = Vector(0.0f);
	BoxMax = Vector(0.0f);
	transform = Matrix ();
}

SlowBoxCollider::~SlowBoxCollider ()
{
}


void SlowBoxCollider::Init (const Vector& BoxMin, const Vector& BoxMax, const Matrix& transform)
{
	this->BoxMin = BoxMin;
	this->BoxMax = BoxMax;
	this->transform = transform;

}


bool SlowBoxCollider::dsRayTriOverlapUseDistance (const Vector &v0, const Vector &v1, const Vector &v2, float &tri_dist, const Vector& v_from, const Vector& v_to)
{
//-------- make math ray------------
	Vector direction;
	Vector origin;
	float max_dist;
	Vector r =  (v_to - v_from );
	max_dist = sqrtf (r.x * r.x + r.y * r.y + r.z * r.z);
	origin = v_from;
	direction = !r;
	//-------- make math ray------------


	// Find vectors for two edges sharing vert0
	Vector edge1 = v1 - v0;
	Vector edge2 = v2 - v0;
	
	//------ Учитываем, что луч может быть ограничен по длинне
	
	
	Vector plane = edge2 ^ edge1;
	plane = !plane;
	double plane_dist = -(v0 | plane);
	double Denom = plane | direction;
	if(Denom>-LOCAL_EPSILON && Denom<LOCAL_EPSILON) return false;
	double ttrid = -((plane_dist + (plane | origin )) / Denom);
	tri_dist = (float)ttrid;
	//if (tri_dist < 0 || tri_dist > max_dist) return false;
	
	
	//---------------------
	
	
	// Begin calculating determinant - also used to calculate U parameter
	Vector pvec = direction^edge2;
	
	// If determinant is near zero, ray lies in plane of triangle
	double det = edge1|pvec;
	
	// the non-culling branch
	if(det>-LOCAL_EPSILON && det<LOCAL_EPSILON) return false;
	double inv_det = 1.0f / det;
	
	// Calculate distance from vert0 to ray origin
	Vector tvec = origin - v0;
	
	// Calculate U parameter and test bounds
	double mU = (tvec|pvec) * inv_det;
	if (mU < 0.0 || mU > 1.0) return false;
	
	// prepare to test V parameter
	Vector qvec = tvec^edge1;
	
	// Calculate V parameter and test bounds
	double mV = (direction|qvec) * inv_det;
	if (mV < 0.0 || (mU+mV) > 1.0) return false;
	
	// Calculate distance
	
	/*
	float mDistance = (edge2|qvec) * inv_det;
	*/
	//tri_dist = (edge2|qvec) * inv_det;
	//if (tri_dist < 0 || tri_dist > distance) return false;
	
	return true;
	

	
}



float SlowBoxCollider::RayTrace (const Vector& from, const Vector& to)
{
	IntersectionPoint = Vector (0.0f);
	Vector v[8];
	v[0] = Vector (BoxMax.x, BoxMax.y, BoxMax.z);
	v[1] = Vector (BoxMax.x, BoxMax.y, BoxMin.z);
	v[2] = Vector (BoxMin.x, BoxMax.y, BoxMax.z);
	v[3] = Vector (BoxMin.x, BoxMax.y, BoxMin.z);

	v[4] = Vector (BoxMax.x, BoxMin.y, BoxMax.z);
	v[5] = Vector (BoxMax.x, BoxMin.y, BoxMin.z);
	v[6] = Vector (BoxMin.x, BoxMin.y, BoxMax.z);
	v[7] = Vector (BoxMin.x, BoxMin.y, BoxMin.z);

	IndexTriangle Cube[12];
	Cube[0].a = 1; Cube[0].b = 4; Cube[0].c = 3;
	Cube[1].a = 1; Cube[1].b = 2; Cube[1].c = 4;
	Cube[2].a = 5; Cube[2].b = 2; Cube[2].c = 1;
	Cube[3].a = 5; Cube[3].b = 6; Cube[3].c = 2;
	Cube[4].a = 7; Cube[4].b = 5; Cube[4].c = 8;
	Cube[5].a = 5; Cube[5].b = 6; Cube[5].c = 8;
	Cube[6].a = 7; Cube[6].b = 1; Cube[6].c = 3;
	Cube[7].a = 7; Cube[7].b = 5; Cube[7].c = 1;
	Cube[8].a = 8; Cube[8].b = 4; Cube[8].c = 2;
	Cube[9].a = 8; Cube[9].b = 2; Cube[9].c = 6;
	Cube[10].a = 7; Cube[10].b = 3; Cube[10].c = 4;
	Cube[11].a = 7; Cube[11].b = 4; Cube[11].c = 8;


	for (int i = 0; i < 8; i++)
	{
		v[i] = v[i] * transform;
	}

	for (i = 0; i < 12; i++)
	{
		Cube[i].a = Cube[i].a - 1;
		Cube[i].b = Cube[i].b - 1;
		Cube[i].c = Cube[i].c - 1;
	}


	float MinDistance = -1.0f;
	float TriDist = 0.0f;

	IRender* pRS = (IRender*)api->GetService("DX9Render");
	for (int n = 0; n < 12; n++)
	{
		//pRS->DrawBufferedLine(v[Cube[n].a], 0xFFFF0000, v[Cube[n].b], 0xFFFF0000);
		//pRS->DrawBufferedLine(v[Cube[n].b], 0xFFFF0000, v[Cube[n].c], 0xFFFF0000);
		//pRS->DrawBufferedLine(v[Cube[n].c], 0xFFFF0000, v[Cube[n].a], 0xFFFF0000);

		
		bool res = dsRayTriOverlapUseDistance (v[Cube[n].a], v[Cube[n].b], v[Cube[n].c], TriDist, from, to);
		

		Vector direction;
		Vector origin;
		float max_dist;
		Vector r =  (to - from );
		max_dist = sqrtf (r.x * r.x + r.y * r.y + r.z * r.z);
		origin = from;
		direction = !r;

		if (res)
		{
			if (MinDistance < 0)
			{
				MinDistance = TriDist;
				IntersectionPoint = origin + (direction * MinDistance);
			}	else
				{
					if (TriDist < MinDistance)
					{
						MinDistance = TriDist;
						IntersectionPoint = origin + (direction * MinDistance);
					}
				}
		}
	}

	return MinDistance;
}