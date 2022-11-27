//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Sphere
//============================================================================================

#ifndef _Sphere_h_
#define _Sphere_h_


#include "Vector.h"
#include "Vector4.h"


///Класс представления шара в 3D пространстве
class Sphere
{
public:
	union
	{
		struct
		{
			union
			{
				struct
				{
					///Позиция по X
					float x;
					///Позиция по Y
					float y;
					///Позиция по Z
					float z;
				};
				union
				{
					struct
					{
						///Позиция
						Vector p;
					};
					struct
					{
						///Позиция
						Vector pos;
					};
				};
			};
			union
			{
				///Радиус
				float r;
				///Радиус
				float radius;
			};
		};
		struct
		{
			///Представление в виде Vector4
			Vector4 v4;
		};
	};


//-----------------------------------------------------------
//Утилитные
//-----------------------------------------------------------
public:
	//Empty constructor
	Sphere();
	//Точка в сфере
	bool mathcall Intersection(const Vector & p);
	//Проверить пересечение отрезка и сферы
	bool mathcall Intersection(const Vector & src, const Vector & dst);
	//Проверить пересечение луча и сферы
	bool mathcall Intersection(const Vector & orig, const Vector & normdir, float * res);
	//Проверить пересечение сферы и сферы
	bool mathcall Intersection(const Sphere & sph);
	
	//Установить сферу в точку с 0 радиусом
	void mathcall Reset(const Vector & p);
	//Включить в описывающую сферу точку
	void mathcall AddPoint(const Vector & p);


	//Проверить пересечение луча и сферы
	static bool mathcall Intersection(const Vector & orig, const Vector & normdir, const Vector & pos, float r, float * res);
	//Проверить пересечение сферы с AABB
	static bool	mathcall Intersection(const Vector & boxMin, const Vector & boxMax, const Vector & pos, float r);

};


//Empty constructor
mathinline Sphere::Sphere() {}

//===========================================================
//Утилитные
//===========================================================

//Точка в сфере
mathinline bool mathcall Sphere::Intersection(const Vector & p)
{
	return ~(pos - p) <= radius*radius;
}

//Проверить пересечение отрезка и сферы или нахождение внутри сферы
mathinline bool mathcall Sphere::Intersection(const Vector & src, const Vector & dst)
{
	Vector dir = dst - src;
	float len = dir.Normalize();
	if(len > 1e-10f)
	{
		float dist;
		if(!Intersection(src, dir, pos, r, &dist)) return false;
		if(dist >= 0.0f)
		{
			return (dist <= len);
		}
		dir = -dir;
		if(!Intersection(dst, dir, pos, r, &dist)) return false;
		if(dist >= 0.0f)
		{
			return (dist <= len);
		}
		return ~(src - pos) <= radius*radius;
	}
	return ~(pos - src) <= radius*radius;
}

//Проверить пересечение луча и сферы
mathinline bool mathcall Sphere::Intersection(const Vector & orig, const Vector & normdir, float * res)
{
	return Intersection(orig, normdir, pos, r, res);
}

//Проверить пересечение сферы и сферы
mathinline bool mathcall Sphere::Intersection(const Sphere & sph)
{
	return (~(p - sph.p) <= (r + sph.r)*(r + sph.r));
}

//Установить сферу в точку с 0 радиусом
mathinline void mathcall Sphere::Reset(const Vector & p)
{
	pos = p;
	r = 0.0f;
}

//Включить в описывающую сферу точку
mathinline void mathcall Sphere::AddPoint(const Vector & p)
{
	//Вектор из точки к центру
	float dx = pos.x - p.x;
	float dy = pos.y - p.y;
	float dz = pos.z - p.z;
	float len = dx*dx + dy*dy + dz*dz;
	if(len <= r*r) return;
	len = sqrtf(len);
	//Новый радиус
	r = (len + r)*0.5f;
	//Новая позиция
	len = r/len;
	pos.x = p.x + dx*len;
	pos.y = p.y + dy*len;
	pos.z = p.z + dz*len;
}

//Проверить пересечение луча и сферы
mathinline bool mathcall Sphere::Intersection(const Vector & orig, const Vector & normdir, const Vector & pos, float r, float * res)
{
	Vector toCenter = pos - orig;
	float distToOrtoPlane = normdir | toCenter;
	float distFromOrtoPlaneToSphere2 = r*r - (~toCenter - distToOrtoPlane*distToOrtoPlane);
	if(distFromOrtoPlaneToSphere2 < 0.0f) return false;
	if(res)
	{
		*res = distToOrtoPlane - sqrtf(distFromOrtoPlaneToSphere2);
	}
	return true;	
}

//Проверить пересечение сферы с AABB
mathinline bool mathcall Sphere::Intersection(const Vector & boxMin, const Vector & boxMax, const Vector & pos, float r)
{
	float dr = 0.0f;
	if(pos.x < boxMin.x)
	{
		float d = boxMin.x - pos.x;
		if(d > r) return false;
		dr += d*d;
	}else
	if(pos.x > boxMax.x)
	{
		float d = pos.x - boxMax.x;
		if(d > r) return false;
		dr += d*d;
	}
	if(pos.y < boxMin.y)
	{
		float d = boxMin.y - pos.y;
		if(d > r) return false;
		dr += d*d;
	}else
	if(pos.y > boxMax.y)
	{
		float d = pos.y - boxMax.y;
		if(d > r) return false;
		dr += d*d;
	}
	if(pos.z < boxMin.z)
	{
		float d = boxMin.z - pos.z;
		if(d > r) return false;
		dr += d*d;
	}else
	if(pos.z > boxMax.z)
	{
		float d = pos.z - boxMax.z;
		if(d > r) return false;
		dr += d*d;
	}
	if(dr > r*r)
	{
		return false;
	}
	return true;
}


#endif
